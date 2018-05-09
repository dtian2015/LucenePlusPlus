#include "ContribInc.h"

#include "kuromoji/dict/UserDictionary.h"

#include "BufferedReader.h"
#include "CodecUtil.h"
#include "IntsRef.h"
#include "PositiveIntOutputs.h"
#include "StringUtils.h"
#include "kuromoji/util/CSVUtil.h"

#include <regex>

#include <boost/algorithm/string.hpp>

namespace Lucene {
namespace Analysis {
namespace Ja {
namespace Dict {

const std::vector<std::vector<int>> UserDictionary::EMPTY_RESULT;

using namespace Lucene::Util::FST;

UserDictionary::UserDictionary(ReaderPtr reader)
{
	int wordId = CUSTOM_DICTIONARY_WORD_ID_OFFSET;
	std::vector<std::vector<String>> featureEntries;

	// text, segmentation, readings, POS
	std::wregex replacePattern(L"#.*$");
	BufferedReaderPtr br = newLucene<BufferedReader>(reader);
	String line;
	while (br->readLine(line))
	{
		// Remove comments
		line = std::regex_replace(line, replacePattern, L"");

		// Skip empty lines or comment lines
		boost::trim(line);
		if (line.empty())
		{
			continue;
		}

		std::vector<String> values = Util::CSVUtil::parse(line);
		featureEntries.push_back(values);
	}

	// TODO: should we allow multiple segmentations per input 'phrase'?
	// the old treemap didn't support this either, and i'm not sure if its needed/useful?

	std::sort(featureEntries.begin(), featureEntries.end(), [](const std::vector<String>& left, const std::vector<String>& right) -> bool {
		String leftContent = left.empty() ? L"" : left[0];
		String rightContent = right.empty() ? L"" : right[0];

		return leftContent < rightContent;
	});

	Collection<String> data = Collection<String>::newInstance();
	std::vector<std::vector<int>> segmentations;
	segmentations.reserve(featureEntries.size());

	auto fstOutput = Lucene::Util::FST::PositiveIntOutputs::getSingleton(true);
	boost::shared_ptr<Builder<Long>> fstBuilder = newLucene<Builder<Long>>(FST<Long>::INPUT_TYPE::BYTE2, fstOutput);
	Lucene::Util::IntsRefPtr scratch = newLucene<Lucene::Util::IntsRef>();
	long ord = 0;

	replacePattern = std::wregex(L"  *");
	for (auto values : featureEntries)
	{
		auto segmentation = StringUtils::split(std::regex_replace(values[1], replacePattern, L" "), L" ");
		auto readings = StringUtils::split(std::regex_replace(values[2], replacePattern, L" "), L" ");
		String pos = values[3];

		if (segmentation.size() != readings.size())
		{
			boost::throw_exception(RuntimeException(
				L"Illegal user dictionary entry " + values[0] + L" - the number of segmentations (" + std::to_wstring(segmentation.size()) +
				L")" + L" does not the match number of readings (" + std::to_wstring(readings.size()) + L")"));
		}

		std::vector<int> wordIdAndLength(segmentation.size() + 1); // wordId offset, length, length....
		wordIdAndLength[0] = wordId;
		for (int i = 0; i < segmentation.size(); i++)
		{
			wordIdAndLength[i + 1] = segmentation[i].length();
			data.add(readings[i] + INTERNAL_SEPARATOR + pos);
			wordId++;
		}

		// add mapping to FST
		String token = values[0];
		scratch->grow(token.length());
		scratch->length = token.length();

		for (int i = 0; i < token.length(); i++)
		{
			scratch->ints[i] = static_cast<int>(token[i]);
		}

		fstBuilder->add(scratch, boost::make_shared<long>(ord));
		segmentations.push_back(wordIdAndLength);
		ord++;
	}

	_fst = newLucene<TokenInfoFST>(fstBuilder->finish(), false);
	_data = data;
	_segmentations = segmentations;
}

std::vector<std::vector<int>> UserDictionary::lookup(CharArray chars, int off, int len)
{
	// TODO: can we avoid this treemap/toIndexArray?
	std::map<int, std::vector<int>> result; // index, [length, length...]
	bool found = false; // true if we found any results

	auto fstReader = _fst->getBytesReader(0);

	auto arc = newLucene<FSTArcLong>();
	int end = off + len;
	for (int startOffset = off; startOffset < end; startOffset++)
	{
		arc = _fst->getFirstArc(arc);
		int output = 0;
		int remaining = end - startOffset;
		for (int i = 0; i < remaining; i++)
		{
			int ch = chars[startOffset + i];
			if (_fst->findTargetArc(ch, arc, arc, i == 0, fstReader) == nullptr)
			{
				break; // continue to next position
			}
			output += *(arc->output);
			if (arc->isFinal())
			{
				const int finalOutput = output + *(arc->nextFinalOutput);
				result.emplace(startOffset - off, _segmentations[finalOutput]);
				found = true;
			}
		}
	}

	return found ? toIndexArray(result) : EMPTY_RESULT;
}

TokenInfoFSTPtr UserDictionary::getFST()
{
	return _fst;
}

std::vector<std::vector<int>> UserDictionary::toIndexArray(const std::map<int, std::vector<int>>& input)
{
	std::vector<std::vector<int>> result;
	for (auto i : input)
	{
		const std::vector<int>& wordIdAndLength = i.second;
		int wordId = wordIdAndLength[0];
		// convert length to index
		int current = i.first;
		for (int j = 1; j < wordIdAndLength.size(); j++)
		{
			// first entry is wordId offset
			std::vector<int> token = {wordId + j - 1, current, wordIdAndLength[j]};
			result.push_back(token);
			current += wordIdAndLength[j];
		}
	}

	return result;
}

std::vector<int> UserDictionary::lookupSegmentation(int phraseID)
{
	return _segmentations[phraseID];
}

int UserDictionary::getLeftId(int wordId)
{
	return LEFT_ID;
}

int UserDictionary::getRightId(int wordId)
{
	return RIGHT_ID;
}

int UserDictionary::getWordCost(int wordId)
{
	return WORD_COST;
}

String UserDictionary::getReading(int wordId, CharArray surface, int off, int len)
{
	return getFeature(wordId, {0});
}

String UserDictionary::getPartOfSpeech(int wordId)
{
	return getFeature(wordId, {1});
}

String UserDictionary::getBaseForm(int wordId, CharArray surface, int off, int len)
{
	return L""; // TODO: add support?
}

String UserDictionary::getPronunciation(int wordId, CharArray surface, int off, int len)
{
	return L""; // TODO: add support?
}

String UserDictionary::getInflectionType(int wordId)
{
	return L""; // TODO: add support?
}

String UserDictionary::getInflectionForm(int wordId)
{
	return L""; // TODO: add support?
}

Collection<String> UserDictionary::getAllFeaturesArray(int wordId)
{
	String allFeatures = _data[wordId - CUSTOM_DICTIONARY_WORD_ID_OFFSET];
	if (allFeatures == L"")
	{
		return Collection<String>();
	}

	return StringUtils::split(allFeatures, INTERNAL_SEPARATOR);
}

String UserDictionary::getFeature(int wordId, const std::vector<int>& fields)
{
	auto allFeatures = getAllFeaturesArray(wordId);
	if (allFeatures.empty())
	{
		return L"";
	}

	StringStream sb;
	if (fields.empty())
	{
		// All features
		for (auto i = 0; i < allFeatures.size(); ++i)
		{
			sb << Util::CSVUtil::quoteEscape(allFeatures[i]);
			if (i < allFeatures.size() - 1)
			{
				sb << L",";
			}
		}
	}
	else if (fields.size() == 1)
	{
		// One feature doesn't need to escape value
		sb << allFeatures[fields[0]];
	}
	else
	{
		for (auto field = 0; field < fields.size(); ++field)
		{
			sb << Util::CSVUtil::quoteEscape(allFeatures[field]);
			if (field < fields.size() - 1)
			{
				sb << L",";
			}
		}
	}

	return sb.str();
}
}
}
}
}
