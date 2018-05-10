#include "ContribInc.h"

#include "kuromoji/JapaneseTokenizer.h"

#include "CharTermAttribute.h"
#include "OffsetAttribute.h"
#include "PositionIncrementAttribute.h"
#include "PositionLengthAttribute.h"
#include "RollingCharBuffer.h"
#include "UTF8Stream.h"
#include "UnicodeUtils.h"
#include "kuromoji/GraphvizFormatter.h"
#include "kuromoji/Token.h"
#include "kuromoji/dict/ConnectionCosts.h"
#include "kuromoji/dict/TokenInfoDictionary.h"
#include "kuromoji/dict/UnknownDictionary.h"
#include "kuromoji/dict/UserDictionary.h"
#include "kuromoji/tokenattributes/BaseFormAttribute.h"
#include "kuromoji/tokenattributes/InflectionAttribute.h"
#include "kuromoji/tokenattributes/PartOfSpeechAttribute.h"
#include "kuromoji/tokenattributes/ReadingAttribute.h"

#include <thread>

namespace Lucene {
namespace Analysis {
namespace Ja {

const JapaneseTokenizer::Mode JapaneseTokenizer::DEFAULT_MODE = Mode::SEARCH;

const std::map<JapaneseTokenizer::Type, String> JapaneseTokenizer::TYPE_STRING_MAP = {
	{Type::KNOWN, L"Known"}, {Type::UNKNOWN, L"Unknown"}, {Type::USER, L"User"}};

JapaneseTokenizer::JapaneseTokenizer(ReaderPtr input, Dict::UserDictionaryPtr userDictionary, bool discardPunctuation, Mode mode)
	: Tokenizer(input)
{
	_buffer = newLucene<RollingCharBuffer>();

	_inflectionAtt = addAttribute<TokenAttributes::InflectionAttribute>();
	_readingAtt = addAttribute<TokenAttributes::ReadingAttribute>();
	_posAtt = addAttribute<TokenAttributes::PartOfSpeechAttribute>();
	_basicFormAtt = addAttribute<TokenAttributes::BaseFormAttribute>();
	_posLengthAtt = addAttribute<PositionLengthAttribute>();
	_posIncAtt = addAttribute<PositionIncrementAttribute>();
	_offsetAtt = addAttribute<OffsetAttribute>();
	_termAtt = addAttribute<CharTermAttribute>();

	_dictionary = Dict::TokenInfoDictionary::getInstance();
	_fst = _dictionary->getFST();
	_unkDictionary = Dict::UnknownDictionary::getInstance();
	_characterDefinition = _unkDictionary->getCharacterDefinition();
	_userDictionary = userDictionary;
	_costs = Dict::ConnectionCosts::getInstance();
	_fstReader = _fst->getBytesReader(0);

	if (_userDictionary != nullptr)
	{
		_userFST = _userDictionary->getFST();
		_userFSTReader = _userFST->getBytesReader(0);
	}
	else
	{
		_userFST.reset();
		_userFSTReader.reset();
	}

	_discardPunctuation = discardPunctuation;
	switch (mode)
	{
		case Mode::SEARCH:
			_searchMode = true;
			_extendedMode = false;
			_outputCompounds = true;
			break;
		case Mode::EXTENDED:
			_searchMode = true;
			_extendedMode = true;
			_outputCompounds = false;
			break;
		default:
			_searchMode = false;
			_extendedMode = false;
			_outputCompounds = false;
			break;
	}

	_buffer->reset(input);

	resetState();

	_dictionaryMap.insert({Type::KNOWN, _dictionary});
	_dictionaryMap.insert({Type::UNKNOWN, _unkDictionary});
	_dictionaryMap.insert({Type::USER, _userDictionary});
}

void JapaneseTokenizer::setGraphvizFormatter(GraphvizFormatterPtr dotOut)
{
	this->_dotOut = dotOut;
}

void JapaneseTokenizer::reset(const ReaderPtr& input)
{
	Tokenizer::reset(input);
	_buffer->reset(input);
}

void JapaneseTokenizer::reset()
{
	Tokenizer::reset();
	resetState();
}

void JapaneseTokenizer::resetState()
{
	_positions->reset();
	_unknownWordEndIndex = -1;
	_pos = 0;
	_end = false;
	_lastBackTracePos = 0;
	_lastTokenPos = -1;
	_pending.clear();

	// Add BOS:
	_positions->get(0)->add(0, 0, -1, -1, -1, Type::KNOWN);
}

void JapaneseTokenizer::end()
{
	// Set final offset
	int finalOffset = correctOffset(_pos);
	_offsetAtt->setOffset(finalOffset, finalOffset);
}

int JapaneseTokenizer::computeSecondBestThreshold(int pos, int length)
{
	// TODO: maybe we do something else here, instead of just
	// using the penalty...?  EG we can be more aggressive on
	// when to also test for 2nd best path
	return computePenalty(pos, length);
}

int JapaneseTokenizer::computePenalty(int pos, int length)
{
	if (length > SEARCH_MODE_KANJI_LENGTH)
	{
		bool allKanji = true;
		// check if node consists of only kanji
		const int endPos = pos + length;
		for (int pos2 = pos; pos2 < endPos; pos2++)
		{
			if (!_characterDefinition->isKanji(static_cast<wchar_t>(_buffer->get(pos2))))
			{
				allKanji = false;
				break;
			}
		}
		if (allKanji)
		{
			// Process only Kanji keywords
			return (length - SEARCH_MODE_KANJI_LENGTH) * SEARCH_MODE_KANJI_PENALTY;
		}
		else if (length > SEARCH_MODE_OTHER_LENGTH)
		{
			return (length - SEARCH_MODE_OTHER_LENGTH) * SEARCH_MODE_OTHER_PENALTY;
		}
	}
	return 0;
}

void JapaneseTokenizer::Position::grow()
{
	costs.resize(MiscUtils::getNextSize(1 + count));
	lastRightID.resize(MiscUtils::getNextSize(1 + count));
	backPos.resize(MiscUtils::getNextSize(1 + count));
	backIndex.resize(MiscUtils::getNextSize(1 + count));
	backID.resize(MiscUtils::getNextSize(1 + count));

	// NOTE: sneaky: grow separately because
	// ArrayUtil.grow will otherwise pick a different
	// length than the int[]s we just grew:
	backType.resize(backID.size());
}

void JapaneseTokenizer::Position::growForward()
{
	forwardPos.resize(MiscUtils::getNextSize(1 + forwardCount));
	forwardID.resize(MiscUtils::getNextSize(1 + forwardCount));
	forwardIndex.resize(MiscUtils::getNextSize(1 + forwardCount));

	// NOTE: sneaky: grow separately because
	// ArrayUtil.grow will otherwise pick a different
	// length than the int[]s we just grew:
	forwardType.resize(forwardPos.size());
}

void JapaneseTokenizer::Position::add(int cost, int lastRightID, int backPos, int backIndex, int backID, Type backType)
{
	// NOTE: this isn't quite a true Viterbit search,
	// becase we should check if lastRightID is
	// already present here, and only update if the new
	// cost is less than the current cost, instead of
	// simply appending.  However, that will likely hurt
	// performance (usually we add a lastRightID only once),
	// and it means we actually create the full graph
	// intersection instead of a "normal" Viterbi lattice:
	if (count == costs.size())
	{
		grow();
	}
	this->costs[count] = cost;
	this->lastRightID[count] = lastRightID;
	this->backPos[count] = backPos;
	this->backIndex[count] = backIndex;
	this->backID[count] = backID;
	this->backType[count] = backType;
	count++;
}

void JapaneseTokenizer::Position::addForward(int forwardPos, int forwardIndex, int forwardID, Type forwardType)
{
	if (forwardCount == this->forwardID.size())
	{
		growForward();
	}
	this->forwardPos[forwardCount] = forwardPos;
	this->forwardIndex[forwardCount] = forwardIndex;
	this->forwardID[forwardCount] = forwardID;
	this->forwardType[forwardCount] = forwardType;
	forwardCount++;
}

void JapaneseTokenizer::Position::reset()
{
	count = 0;
	// forwardCount naturally resets after it runs:
	BOOST_ASSERT(forwardCount == 0);
}

void JapaneseTokenizer::add(Dict::DictionaryPtr dict, PositionPtr fromPosData, int endPos, int wordID, Type type, bool addPenalty)
{
	const int wordCost = dict->getWordCost(wordID);
	const int leftID = dict->getLeftId(wordID);
	int leastCost = std::numeric_limits<int>::max();
	int leastIDX = -1;

	BOOST_ASSERT(fromPosData->count > 0);

	for (int idx = 0; idx < fromPosData->count; idx++)
	{
		// Cost is path cost so far, plus word cost (added at
		// end of loop), plus bigram cost:
		const int cost = fromPosData->costs[idx] + _costs->get(fromPosData->lastRightID[idx], leftID);
		if (VERBOSE)
		{
			std::wcout << L"      fromIDX=" << idx << L": cost=" << cost << L" (prevCost=" << fromPosData->costs[idx] << L" wordCost="
					   << wordCost << L" bgCost=" << _costs->get(fromPosData->lastRightID[idx], leftID) << L" leftID=" << leftID
					   << std::endl;
		}
		if (cost < leastCost)
		{
			leastCost = cost;
			leastIDX = idx;
			if (VERBOSE)
			{
				std::wcout << L"        **" << std::endl;
			}
		}
	}

	leastCost += wordCost;

	if (VERBOSE)
	{
		std::wcout << L"      + cost=" << leastCost << L" wordID=" << wordID << L" leftID=" << leftID << L" leastIDX=" << leastIDX
				   << L" toPos=" << endPos << L" toPos.idx=" << _positions->get(endPos)->count << std::endl;
	}

	if ((addPenalty || (!_outputCompounds && _searchMode)) && type != Type::USER)
	{
		const int penalty = computePenalty(fromPosData->pos, endPos - fromPosData->pos);
		if (VERBOSE)
		{
			if (penalty > 0)
			{
				std::wcout << L"        + penalty=" << penalty << L" cost=" << (leastCost << penalty) << std::endl;
			}
		}
		leastCost += penalty;
	}

	// positions.get(endPos).add(leastCost, dict.getRightId(wordID), fromPosData.pos, leastIDX, wordID, type);
	BOOST_ASSERT(leftID == dict->getRightId(wordID));
	_positions->get(endPos)->add(leastCost, leftID, fromPosData->pos, leastIDX, wordID, type);
}

bool JapaneseTokenizer::incrementToken()
{
	// parse() is able to return w/o producing any new
	// tokens, when the tokens it had produced were entirely
	// punctuation.  So we loop here until we get a real
	// token or we end:
	while (_pending.empty())
	{
		if (_end)
		{
			return false;
		}

		// Push Viterbi forward some more:
		parse();
	}

	const auto token = _pending.back();
	_pending.pop_back();

	int position = token->getPosition();
	int length = token->getLength();
	clearAttributes();

	BOOST_ASSERT(length > 0);

	// System.out.println("off=" + token.getOffset() + " len=" + length + " vs " + token.getSurfaceForm().length);
	_termAtt->copyBuffer(token->getSurfaceForm().get(), token->getOffset(), length);
	_offsetAtt->setOffset(correctOffset(position), correctOffset(position + length));
	_basicFormAtt->setToken(token);
	_posAtt->setToken(token);
	_readingAtt->setToken(token);
	_inflectionAtt->setToken(token);

	if (token->getPosition() == _lastTokenPos)
	{
		_posIncAtt->setPositionIncrement(0);
		_posLengthAtt->setPositionLength(token->getPositionLength());
	}
	else
	{
		BOOST_ASSERT(token->getPosition() > _lastTokenPos);
		_posIncAtt->setPositionIncrement(1);
		_posLengthAtt->setPositionLength(1);
	}
	if (VERBOSE)
	{
		std::wcout << std::this_thread::get_id() << L":    incToken: return token=" << token->toString() << std::endl;
	}
	_lastTokenPos = token->getPosition();

	return true;
}

JapaneseTokenizer::WrappedPositionArray::WrappedPositionArray()
{
	for (int i = 0; i < _positions.size(); i++)
	{
		_positions[i] = newLucene<Position>();
	}
}

void JapaneseTokenizer::WrappedPositionArray::reset()
{
	_nextWrite--;
	while (_count > 0)
	{
		if (_nextWrite == -1)
		{
			_nextWrite = _positions.size() - 1;
		}
		_positions[_nextWrite--]->reset();
		_count--;
	}
	_nextWrite = 0;
	_nextPos = 0;
	_count = 0;
}

JapaneseTokenizer::PositionPtr JapaneseTokenizer::WrappedPositionArray::get(int pos)
{
	while (pos >= _nextPos)
	{
		// System.out.println("count=" + count + " vs len=" + positions.length);
		if (_count == _positions.size())
		{
			std::vector<PositionPtr> newPositions = std::vector<PositionPtr>(static_cast<double>(1 + _count) * 1.5);
			// System.out.println("grow positions " + newPositions.length);
			MiscUtils::arrayCopy(_positions.data(), _nextWrite, newPositions.data(), 0, _positions.size() - _nextWrite);
			MiscUtils::arrayCopy(_positions.data(), 0, newPositions.data(), _positions.size() - _nextWrite, _nextWrite);

			for (int i = _positions.size(); i < newPositions.size(); i++)
			{
				newPositions[i] = newLucene<Position>();
			}
			_nextWrite = _positions.size();
			_positions = newPositions;
		}

		if (_nextWrite == _positions.size())
		{
			_nextWrite = 0;
		}

		// Should have already been reset:
		BOOST_ASSERT(_positions[_nextWrite]->count == 0);
		_positions[_nextWrite++]->pos = _nextPos++;
		_count++;
	}

	BOOST_ASSERT(inBounds(pos));

	const int index = getIndex(pos);
	BOOST_ASSERT(_positions[index]->pos == pos);

	return _positions[index];
}

int JapaneseTokenizer::WrappedPositionArray::getNextPos()
{
	return _nextPos;
}

bool JapaneseTokenizer::WrappedPositionArray::inBounds(int pos)
{
	return pos < _nextPos && pos >= _nextPos - _count;
}

int JapaneseTokenizer::WrappedPositionArray::getIndex(int pos)
{
	int index = _nextWrite - (_nextPos - pos);
	if (index < 0)
	{
		index += _positions.size();
	}
	return index;
}

void JapaneseTokenizer::WrappedPositionArray::freeBefore(int pos)
{
	const int toFree = _count - (_nextPos - pos);
	BOOST_ASSERT(toFree >= 0);
	BOOST_ASSERT(toFree <= _count);

	int index = _nextWrite - _count;
	if (index < 0)
	{
		index += _positions.size();
	}

	for (int i = 0; i < toFree; i++)
	{
		if (index == _positions.size())
		{
			index = 0;
		}
		// System.out.println("  fb idx=" + index);
		_positions[index]->reset();
		index++;
	}
	_count -= toFree;
}

void JapaneseTokenizer::parse()
{
	if (VERBOSE)
	{
		std::wcout << L"\nPARSE" << std::endl;
	}

	// Advances over each position (character):
	while (true)
	{
		if (_buffer->get(_pos) == -1)
		{
			// End
			break;
		}

		auto const posData = _positions->get(_pos);
		const bool isFrontier = _positions->getNextPos() == _pos + 1;

		if (posData->count == 0)
		{
			// No arcs arrive here; move to next position:
			_pos++;
			if (VERBOSE)
			{
				std::wcout << L"    no arcs in; skip" << std::endl;
			}
			continue;
		}

		if (_pos > _lastBackTracePos && posData->count == 1 && isFrontier)
		{
			//  if (pos > lastBackTracePos && posData.count == 1 && isFrontier) {
			// We are at a "frontier", and only one node is
			// alive, so whatever the eventual best path is must
			// come through this node.  So we can safely commit
			// to the prefix of the best path at this point:
			backtrace(posData, 0);

			// Re-base cost so we don't risk int overflow:
			posData->costs[0] = 0;

			if (_pending.size() != 0)
			{
				return;
			}
			else
			{
				// This means the backtrace only produced
				// punctuation tokens, so we must keep parsing.
			}
		}

		if (_pos - _lastBackTracePos >= MAX_BACKTRACE_GAP)
		{
			// Safety: if we've buffered too much, force a
			// backtrace now.  We find the least-cost partial
			// path, across all paths, backtrace from it, and
			// then prune all others.  Note that this, in
			// general, can produce the wrong result, if the
			// total bast path did not in fact back trace
			// through this partial best path.  But it's the
			// best we can do... (short of not having a
			// safety!).

			// First pass: find least cost parital path so far,
			// including ending at future positions:
			int leastIDX = -1;
			int leastCost = std::numeric_limits<int>::max();
			PositionPtr leastPosData = nullptr;

			for (int pos2 = _pos; pos2 < _positions->getNextPos(); pos2++)
			{
				auto const posData2 = _positions->get(pos2);
				for (int idx = 0; idx < posData2->count; idx++)
				{
					// System.out.println("    idx=" + idx + " cost=" + cost);
					const int cost = posData2->costs[idx];
					if (cost < leastCost)
					{
						leastCost = cost;
						leastIDX = idx;
						leastPosData = posData2;
					}
				}
			}

			// We will always have at least one live path:
			BOOST_ASSERT(leastIDX != -1);

			// Second pass: prune all but the best path:
			for (int pos2 = _pos; pos2 < _positions->getNextPos(); pos2++)
			{
				auto const posData2 = _positions->get(pos2);
				if (posData2 != leastPosData)
				{
					posData2->reset();
				}
				else
				{
					if (leastIDX != 0)
					{
						posData2->costs[0] = posData2->costs[leastIDX];
						posData2->lastRightID[0] = posData2->lastRightID[leastIDX];
						posData2->backPos[0] = posData2->backPos[leastIDX];
						posData2->backIndex[0] = posData2->backIndex[leastIDX];
						posData2->backID[0] = posData2->backID[leastIDX];
						posData2->backType[0] = posData2->backType[leastIDX];
					}
					posData2->count = 1;
				}
			}

			backtrace(leastPosData, 0);

			// Re-base cost so we don't risk int overflow:
			for (auto i = 0; i < leastPosData->count; i++)
			{
				leastPosData->costs[i] = 0;
			}

			if (_pos != leastPosData->pos)
			{
				// We jumped into a future position:
				BOOST_ASSERT(_pos < leastPosData->pos);
				_pos = leastPosData->pos;
			}

			if (_pending.size() != 0)
			{
				return;
			}
			else
			{
				// This means the backtrace only produced
				// punctuation tokens, so we must keep parsing.
				continue;
			}
		}

		if (VERBOSE)
		{
			std::wcout << L"\n  extend @ pos=" << _pos << L" char=" << static_cast<wchar_t>(_buffer->get(_pos)) << std::endl;
		}

		if (VERBOSE)
		{
			std::wcout << L"    " << posData->count << L" arcs in" << std::endl;
		}

		bool anyMatches = false;

		// First try user dict:
		if (_userFST != nullptr)
		{
			_userFST->getFirstArc(_arc);
			int output = 0;
			for (int posAhead = posData->pos;; posAhead++)
			{
				const int ch = _buffer->get(posAhead);
				if (ch == -1)
				{
					break;
				}
				if (_userFST->findTargetArc(ch, _arc, _arc, posAhead == posData->pos, _userFSTReader) == nullptr)
				{
					break;
				}
				output += GetOutputValue<int>(_arc->output);
				if (_arc->isFinal())
				{
					if (VERBOSE)
					{
						std::wcout << L"    USER word " << _buffer->get(_pos, posAhead - _pos + 1) << L" toPos=" << (posAhead << 1)
								   << std::endl;
					}

					add(_userDictionary, posData, posAhead + 1, output + GetOutputValue<int>(_arc->nextFinalOutput), Type::USER, false);
					anyMatches = true;
				}
			}
		}

		// TODO: we can be more aggressive about user
		// matches?  if we are "under" a user match then don't
		// extend KNOWN/UNKNOWN paths?

		if (!anyMatches)
		{
			// Next, try known dictionary matches
			_fst->getFirstArc(_arc);
			int output = 0;

			for (int posAhead = posData->pos;; posAhead++)
			{
				const int ch = _buffer->get(posAhead);
				if (ch == -1)
				{
					break;
				}
				// System.out.println("    match " + (char) ch + " posAhead=" + posAhead);

				if (_fst->findTargetArc(ch, _arc, _arc, posAhead == posData->pos, _fstReader) == nullptr)
				{
					break;
				}

				output += GetOutputValue<int>(_arc->output);

				// Optimization: for known words that are too-long
				// (compound), we should pre-compute the 2nd
				// best segmentation and store it in the
				// dictionary instead of recomputing it each time a
				// match is found.

				if (_arc->isFinal())
				{
					_dictionary->lookupWordIds(output + GetOutputValue<int>(_arc->nextFinalOutput), _wordIdRef);
					if (VERBOSE)
					{
						std::wcout << L"    KNOWN word " << _buffer->get(_pos, posAhead - _pos + 1) << L" toPos=" << (posAhead + 1) << L" "
								   << _wordIdRef.length << L" wordIDs" << std::endl;
					}
					for (int ofs = 0; ofs < _wordIdRef.length; ofs++)
					{
						add(_dictionary, posData, posAhead + 1, _dictionary->getWordId(_wordIdRef.offset + ofs), Type::KNOWN, false);
						anyMatches = true;
					}
				}
			}
		}

		// In the case of normal mode, it doesn't process unknown word greedily.

		if (!_searchMode && _unknownWordEndIndex > posData->pos)
		{
			_pos++;
			continue;
		}

		const char16_t firstCharacter = static_cast<char16_t>(_buffer->get(_pos));
		if (!anyMatches || _characterDefinition->isInvoke(firstCharacter))
		{
			// Find unknown match:
			const int characterId = _characterDefinition->getCharacterClass(firstCharacter);

			// NOTE: copied from UnknownDictionary.lookup:
			int unknownWordLength;
			if (!_characterDefinition->isGroup(firstCharacter))
			{
				unknownWordLength = 1;
			}
			else
			{
				// Extract unknown word. Characters with the same character class are considered to be part of unknown word
				unknownWordLength = 1;
				for (int posAhead = _pos + 1; unknownWordLength < MAX_UNKNOWN_WORD_LENGTH; posAhead++)
				{
					const int ch = _buffer->get(posAhead);
					if (ch == -1)
					{
						break;
					}
					if (characterId == _characterDefinition->getCharacterClass(static_cast<char16_t>(ch)))
					{
						unknownWordLength++;
					}
					else
					{
						break;
					}
				}
			}

			_unkDictionary->lookupWordIds(characterId, _wordIdRef); // characters in input text are supposed to be the same
			if (VERBOSE)
			{
				std::wcout << L"    UNKNOWN word len=" << unknownWordLength << L" " << _wordIdRef.length << L" wordIDs" << std::endl;
			}
			for (int ofs = 0; ofs < _wordIdRef.length; ofs++)
			{
				add(_unkDictionary, posData, posData->pos + unknownWordLength, _unkDictionary->getWordId(_wordIdRef.offset + ofs),
					Type::UNKNOWN, false);
			}

			_unknownWordEndIndex = posData->pos + unknownWordLength;
		}

		_pos++;
	}

	_end = true;

	if (_pos > 0)
	{
		auto const endPosData = _positions->get(_pos);
		int leastCost = std::numeric_limits<int>::max();
		int leastIDX = -1;
		if (VERBOSE)
		{
			std::wcout << L"  end: " << endPosData->count << L" nodes" << std::endl;
		}
		for (int idx = 0; idx < endPosData->count; idx++)
		{
			// Add EOS cost:
			const int cost = endPosData->costs[idx] + _costs->get(endPosData->lastRightID[idx], 0);
			// System.out.println("    idx=" + idx + " cost=" + cost + " (pathCost=" + endPosData.costs[idx] + " bgCost=" +
			// costs.get(endPosData.lastRightID[idx], 0) + ") backPos=" + endPosData.backPos[idx]);
			if (cost < leastCost)
			{
				leastCost = cost;
				leastIDX = idx;
			}
		}

		backtrace(endPosData, leastIDX);
	}
	else
	{
		// No characters in the input string; return no tokens!
	}
}

void JapaneseTokenizer::pruneAndRescore(int startPos, int endPos, int bestStartIDX)
{
	if (VERBOSE)
	{
		std::wcout << L"  pruneAndRescore startPos=" << startPos << L" endPos=" << endPos << L" bestStartIDX=" << bestStartIDX << std::endl;
	}

	// First pass: walk backwards, building up the forward
	// arcs and pruning inadmissible arcs:
	for (int _pos = endPos; _pos > startPos; _pos--)
	{
		auto const posData = _positions->get(_pos);
		if (VERBOSE)
		{
			std::wcout << L"    back pos=" << _pos << std::endl;
		}
		for (int arcIDX = 0; arcIDX < posData->count; arcIDX++)
		{
			const int backPos = posData->backPos[arcIDX];
			if (backPos >= startPos)
			{
				// Keep this arc:
				// System.out.println("      keep backPos=" + backPos);
				_positions->get(backPos)->addForward(_pos, arcIDX, posData->backID[arcIDX], posData->backType[arcIDX]);
			}
			else
			{
				if (VERBOSE)
				{
					std::wcout << L"      prune" << std::endl;
				}
			}
		}
		if (_pos != startPos)
		{
			posData->count = 0;
		}
	}

	// Second pass: walk forward, re-scoring:
	for (int _pos = startPos; _pos < endPos; _pos++)
	{
		auto const posData = _positions->get(_pos);
		if (VERBOSE)
		{
			std::wcout << L"    forward pos=" << _pos << L" count=" << posData->forwardCount << std::endl;
		}
		if (posData->count == 0)
		{
			// No arcs arrive here...
			if (VERBOSE)
			{
				std::wcout << L"      skip" << std::endl;
			}
			posData->forwardCount = 0;
			continue;
		}

		if (_pos == startPos)
		{
			// On the initial position, only consider the best
			// path so we "force congruence":  the
			// sub-segmentation is "in context" of what the best
			// path (compound token) had matched:
			int rightID;
			if (startPos == 0)
			{
				rightID = 0;
			}
			else
			{
				rightID = getDict(posData->backType[bestStartIDX])->getRightId(posData->backID[bestStartIDX]);
			}

			const int pathCost = posData->costs[bestStartIDX];
			for (int forwardArcIDX = 0; forwardArcIDX < posData->forwardCount; forwardArcIDX++)
			{
				const Type forwardType = posData->forwardType[forwardArcIDX];
				auto const dict2 = getDict(forwardType);
				const int wordID = posData->forwardID[forwardArcIDX];
				const int toPos = posData->forwardPos[forwardArcIDX];
				const int newCost = pathCost + dict2->getWordCost(wordID) + _costs->get(rightID, dict2->getLeftId(wordID)) +
					computePenalty(_pos, toPos - _pos);

				if (VERBOSE)
				{
					std::wcout << L"      + " << TYPE_STRING_MAP.at(forwardType) << L" word " << _buffer->get(_pos, toPos - _pos)
							   << L" toPos=" << toPos << L" cost=" << newCost << L" penalty=" << computePenalty(_pos, toPos - _pos)
							   << L" toPos.idx=" << _positions->get(toPos)->count << std::endl;
				}
				_positions->get(toPos)->add(newCost, dict2->getRightId(wordID), _pos, bestStartIDX, wordID, forwardType);
			}
		}
		else
		{
			// On non-initial positions, we maximize score
			// across all arriving lastRightIDs:
			for (int forwardArcIDX = 0; forwardArcIDX < posData->forwardCount; forwardArcIDX++)
			{
				const Type forwardType = posData->forwardType[forwardArcIDX];
				const int toPos = posData->forwardPos[forwardArcIDX];

				if (VERBOSE)
				{
					std::wcout << L"      + " << TYPE_STRING_MAP.at(forwardType) << L" word " << _buffer->get(_pos, toPos - _pos)
							   << L" toPos=" << toPos << std::endl;
				}

				add(getDict(forwardType), posData, toPos, posData->forwardID[forwardArcIDX], forwardType, true);
			}
		}
		posData->forwardCount = 0;
	}
}

void JapaneseTokenizer::backtrace(PositionPtr endPosData, int const fromIDX)
{
	const int endPos = endPosData->pos;

	if (VERBOSE)
	{
		std::wcout << L"\n  backtrace: endPos=" << endPos << L" pos=" << _pos << L"; " << (_pos - _lastBackTracePos)
				   << L" characters; last=" << _lastBackTracePos << L" cost=" << endPosData->costs[fromIDX] << std::endl;
	}

	const auto fragment = _buffer->get(_lastBackTracePos, endPos - _lastBackTracePos);

	if (_dotOut != nullptr)
	{
		_dotOut->onBacktrace(shared_from_this(), _positions, _lastBackTracePos, endPosData, fromIDX, fragment, _end);
	}

	int pos = endPos;
	int bestIDX = fromIDX;
	Ja::TokenPtr altToken = nullptr;

	// We trace backwards, so this will be the leftWordID of
	// the token after the one we are now on:
	int lastLeftWordID = -1;

	int backCount = 0;

	// TODO: sort of silly to make Token instances here; the
	// back trace has all info needed to generate the
	// token.  So, we could just directly set the attrs,
	// from the backtrace, in incrementToken w/o ever
	// creating Token; we'd have to defer calling freeBefore
	// until after the bactrace was fully "consumed" by
	// incrementToken.

	while (pos > _lastBackTracePos)
	{
		// System.out.println("BT: back pos=" + pos + " bestIDX=" + bestIDX);
		auto const posData = _positions->get(pos);
		BOOST_ASSERT(bestIDX < posData->count);

		int backPos = posData->backPos[bestIDX];
		BOOST_ASSERT(backPos >= _lastBackTracePos);
		int length = pos - backPos;
		Type backType = posData->backType[bestIDX];
		int backID = posData->backID[bestIDX];
		int nextBestIDX = posData->backIndex[bestIDX];

		if (_outputCompounds && _searchMode && altToken == nullptr && backType != Type::USER)
		{
			// In searchMode, if best path had picked a too-long
			// token, we use the "penalty" to compute the allowed
			// max cost of an alternate back-trace.  If we find an
			// alternate back trace with cost below that
			// threshold, we pursue it instead (but also output
			// the long token).
			// System.out.println("    2nd best backPos=" + backPos + " pos=" + pos);

			const int penalty = computeSecondBestThreshold(backPos, pos - backPos);

			if (penalty > 0)
			{
				if (VERBOSE)
				{
					std::wcout << L"  compound=" << _buffer->get(backPos, pos - backPos) << L" backPos=" << backPos << L" pos=" << pos
							   << L" penalty=" << penalty << L" cost=" << posData->costs[bestIDX] << L" bestIDX=" << bestIDX
							   << L" lastLeftID=" << lastLeftWordID << std::endl;
				}

				// Use the penalty to set maxCost on the 2nd best
				// segmentation:
				int maxCost = posData->costs[bestIDX] + penalty;
				if (lastLeftWordID != -1)
				{
					maxCost += _costs->get(getDict(backType)->getRightId(backID), lastLeftWordID);
				}

				// Now, prune all too-long tokens from the graph:
				pruneAndRescore(backPos, pos, posData->backIndex[bestIDX]);

				// Finally, find 2nd best back-trace and resume
				// backtrace there:
				int leastCost = std::numeric_limits<int>::max();
				int leastIDX = -1;
				for (int idx = 0; idx < posData->count; idx++)
				{
					int cost = posData->costs[idx];
					// System.out.println("    idx=" + idx + " prevCost=" + cost);

					if (lastLeftWordID != -1)
					{
						cost += _costs->get(getDict(posData->backType[idx])->getRightId(posData->backID[idx]), lastLeftWordID);
						// System.out.println("      += bgCost=" + costs.get(getDict(posData.backType[idx]).getRightId(posData.backID[idx]),
						// lastLeftWordID) + " -> " + cost);
					}
					// System.out.println("penalty " + posData.backPos[idx] + " to " + pos);
					// cost += computePenalty(posData.backPos[idx], pos - posData.backPos[idx]);
					if (cost < leastCost)
					{
						// System.out.println("      ** ");
						leastCost = cost;
						leastIDX = idx;
					}
				}
				// System.out.println("  leastIDX=" + leastIDX);

				if (VERBOSE)
				{
					std::wcout << L"  afterPrune: " << posData->count << L" arcs arriving; leastCost=" << leastCost << L" vs threshold="
							   << maxCost << L" lastLeftWordID=" << lastLeftWordID << std::endl;
				}

				if (leastIDX != -1 && leastCost <= maxCost && posData->backPos[leastIDX] != backPos)
				{
					// We should have pruned the altToken from the graph:
					BOOST_ASSERT(posData->backPos[leastIDX] != backPos);

					// Save the current compound token, to output when
					// this alternate path joins back:
					altToken =
						newLucene<Token>(backID, fragment, backPos - _lastBackTracePos, length, backType, backPos, getDict(backType));

					// Redirect our backtrace to 2nd best:
					bestIDX = leastIDX;
					nextBestIDX = posData->backIndex[bestIDX];

					backPos = posData->backPos[bestIDX];
					length = pos - backPos;
					backType = posData->backType[bestIDX];
					backID = posData->backID[bestIDX];
					backCount = 0;
					// System.out.println("  do alt token!");
				}
				else
				{
					// I think in theory it's possible there is no
					// 2nd best path, which is fine; in this case we
					// only output the compound token:
					// System.out.println("  no alt token! bestIDX=" + bestIDX);
				}
			}
		}

		const int offset = backPos - _lastBackTracePos;
		BOOST_ASSERT(offset >= 0);

		if (altToken != nullptr && altToken->getPosition() >= backPos)
		{
			// We've backtraced to the position where the
			// compound token starts; add it now:

			// The pruning we did when we created the altToken
			// ensures that the back trace will align back with
			// the start of the altToken:
			// cannot assert...
			// assert altToken.getPosition() == backPos: altToken.getPosition() + " vs " + backPos;

			if (VERBOSE)
			{
				std::wcout << L"    add altToken=" << altToken->toString() << std::endl;
			}
			if (backCount > 0)
			{
				backCount++;
				altToken->setPositionLength(backCount);
				_pending.push_back(altToken);
			}
			else
			{
				// This means alt token was all punct tokens:
				BOOST_ASSERT(_discardPunctuation);
			}
			altToken.reset();
		}

		auto const dict = getDict(backType);

		if (backType == Type::USER)
		{
			// Expand the phraseID we recorded into the actual
			// segmentation:
			const std::vector<int> wordIDAndLength = _userDictionary->lookupSegmentation(backID);
			int wordID = wordIDAndLength[0];
			int current = 0;
			for (int j = 1; j < wordIDAndLength.size(); j++)
			{
				const int len = wordIDAndLength[j];
				// System.out.println("    add user: len=" + len);
				_pending.push_back(newLucene<Token>(wordID + j - 1, fragment, current + offset, len, Type::USER, current + backPos, dict));
				if (VERBOSE)
				{
					std::wcout << L"    add USER token=" << _pending[_pending.size() - 1]->toString() << std::endl;
				}
				current += len;
			}

			// Reverse the tokens we just added, because when we
			// serve them up from incrementToken we serve in
			// reverse:
			std::reverse(_pending.begin() + (_pending.size() - (wordIDAndLength.size() - 1)), _pending.end());

			backCount += wordIDAndLength.size() - 1;
		}
		else
		{
			if (_extendedMode && backType == Type::UNKNOWN)
			{
				// In EXTENDED mode we convert unknown word into
				// unigrams:
				int unigramTokenCount = 0;
				for (int i = length - 1; i >= 0; i--)
				{
					int charLen = 1;
					if (i > 0 && UTF8Base::isTrailSurrogate(fragment[offset + i]))
					{
						i--;
						charLen = 2;
					}
					// System.out.println("    extended tok offset="
					//+ (offset + i));
					if (!_discardPunctuation || !isPunctuation(fragment[offset + i]))
					{
						_pending.push_back(newLucene<Token>(
							Dict::CharacterDefinition::NGRAM, fragment, offset + i, charLen, Type::UNKNOWN, backPos + i, _unkDictionary));
						unigramTokenCount++;
					}
				}
				backCount += unigramTokenCount;
			}
			else if (!_discardPunctuation || length == 0 || !isPunctuation(fragment[offset]))
			{
				_pending.push_back(newLucene<Token>(backID, fragment, offset, length, backType, backPos, dict));
				if (VERBOSE)
				{
					std::wcout << L"    add token=" << _pending[_pending.size() - 1]->toString() << std::endl;
				}
				backCount++;
			}
			else
			{
				if (VERBOSE)
				{
					std::wcout << L"    skip punctuation token=" << String(fragment.get(), offset, length) << std::endl;
				}
			}
		}

		lastLeftWordID = dict->getLeftId(backID);
		pos = backPos;
		bestIDX = nextBestIDX;
	}

	_lastBackTracePos = endPos;

	if (VERBOSE)
	{
		std::wcout << L"  freeBefore pos=" << endPos << std::endl;
	}
	// Notify the circular buffers that we are done with
	// these positions:
	_buffer->freeBefore(endPos);
	_positions->freeBefore(endPos);
}

Dict::DictionaryPtr JapaneseTokenizer::getDict(Type type)
{
	return _dictionaryMap.at(type);
}

bool JapaneseTokenizer::isPunctuation(wchar_t ch)
{
	return UnicodeUtil::isPunctuation(ch);
}
}
}
}
