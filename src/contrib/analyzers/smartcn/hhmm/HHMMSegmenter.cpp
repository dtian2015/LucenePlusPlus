#include "ContribInc.h"

#include "smartcn/hhmm/HHMMSegmenter.h"

#include "TextFragment.h"
#include "smartcn/CharType.h"
#include "smartcn/Utility.h"
#include "smartcn/WordType.h"
#include "smartcn/hhmm/BiSegGraph.h"
#include "smartcn/hhmm/SegGraph.h"
#include "smartcn/hhmm/SegToken.h"
#include "smartcn/hhmm/WordDictionary.h"

namespace Lucene {
namespace Analysis {
namespace Cn {
namespace Smart {
namespace Hhmm {

WordDictionaryPtr HHMMSegmenter::_wordDict;

HHMMSegmenter::HHMMSegmenter()
{
	_wordDict = WordDictionary::getInstance();
}

SegGraphPtr HHMMSegmenter::createSegGraph(const String& sentence)
{
	int i = 0, j;
	int length = sentence.length();
	int foundIndex;
	std::vector<int> charTypeArray = getCharTypes(sentence);
	StringBuffer wordBuf;
	SegTokenPtr token;
	int frequency = 0; // the number of times word appears.
	bool hasFullWidth;
	WordType wordType;
	CharArray charArray;

	SegGraphPtr segGraph = newLucene<SegGraph>();
	while (i < length)
	{
		hasFullWidth = false;
		switch (charTypeArray[i])
		{
			case CharType::SPACE_LIKE:
			{
				i++;
				break;
			}
			case CharType::HANZI:
			{
				j = i + 1;
				wordBuf.clear();

				// It doesn't matter if a single Chinese character (Hanzi) can form a phrase or not,
				// it will store that single Chinese character (Hanzi) in the SegGraph.  Otherwise, it will
				// cause word division.
				wordBuf.append(sentence[i]);
				charArray = newArray(sentence[i]);
				frequency = _wordDict->getFrequency(charArray);
				token = newLucene<SegToken>(charArray, i, j, WordType::CHINESE_WORD, frequency);
				segGraph->addToken(token);

				foundIndex = _wordDict->getPrefixMatch(charArray);
				while (j <= length && foundIndex != -1)
				{
					if (_wordDict->isEqual(charArray, foundIndex) && charArray.size() > 1)
					{
						// It is the phrase we are looking for; In other words, we have found a phrase SegToken
						// from i to j.  It is not a monosyllabic word (single word).
						frequency = _wordDict->getFrequency(charArray);
						token = newLucene<SegToken>(charArray, i, j, WordType::CHINESE_WORD, frequency);
						segGraph->addToken(token);
					}

					while (j < length && charTypeArray[j] == CharType::SPACE_LIKE)
					{
						j++;
					}

					if (j < length && charTypeArray[j] == CharType::HANZI)
					{
						wordBuf.append(sentence[j]);
						const String word = wordBuf.toString();
						charArray = CharArray::newInstance(word.length());
						std::copy(word.begin(), word.end(), charArray.get());

						// idArray has been found (foundWordIndex!=-1) as a prefix before.
						// Therefore, idArray after it has been lengthened can only appear after foundWordIndex.
						// So start searching after foundWordIndex.
						foundIndex = _wordDict->getPrefixMatch(charArray, foundIndex);
						j++;
					}
					else
					{
						break;
					}
				}

				i++;
				break;
			}
			case CharType::FULLWIDTH_LETTER:
				hasFullWidth = true; // intentional fallthrough
			case CharType::LETTER:
			{
				j = i + 1;
				while (j < length && (charTypeArray[j] == CharType::LETTER || charTypeArray[j] == CharType::FULLWIDTH_LETTER))
				{
					if (charTypeArray[j] == CharType::FULLWIDTH_LETTER)
					{
						hasFullWidth = true;
					}
					j++;
				}

				// Found a Token from i to j. Type is LETTER char string.
				charArray = Utility::STRING_CHAR_ARRAY;
				frequency = _wordDict->getFrequency(charArray);
				wordType = hasFullWidth ? WordType::FULLWIDTH_STRING : WordType::STRING;
				token = newLucene<SegToken>(charArray, i, j, wordType, frequency);
				segGraph->addToken(token);
				i = j;

				break;
			}
			case CharType::FULLWIDTH_DIGIT:
				hasFullWidth = true; // intentional fallthrough
			case CharType::DIGIT:
			{
				j = i + 1;
				while (j < length && (charTypeArray[j] == CharType::DIGIT || charTypeArray[j] == CharType::FULLWIDTH_DIGIT))
				{
					if (charTypeArray[j] == CharType::FULLWIDTH_DIGIT)
					{
						hasFullWidth = true;
					}
					j++;
				}
				// Found a Token from i to j. Type is NUMBER char string.
				charArray = Utility::NUMBER_CHAR_ARRAY;
				frequency = _wordDict->getFrequency(charArray);
				wordType = hasFullWidth ? WordType::FULLWIDTH_NUMBER : WordType::NUMBER;
				token = newLucene<SegToken>(charArray, i, j, wordType, frequency);
				segGraph->addToken(token);
				i = j;

				break;
			}
			case CharType::DELIMITER:
			{
				j = i + 1;
				// No need to search the weight for the punctuation.  Picking the highest frequency will work.
				frequency = Utility::MAX_FREQUENCE;
				charArray = newArray(sentence[i]);
				token = newLucene<SegToken>(charArray, i, j, WordType::DELIMITER, frequency);
				segGraph->addToken(token);
				i = j;

				break;
			}
			default:
			{
				j = i + 1;
				// Treat the unrecognized char symbol as unknown string.
				// For example, any symbol not in GB2312 is treated as one of these.
				charArray = Utility::STRING_CHAR_ARRAY;
				frequency = _wordDict->getFrequency(charArray);
				token = newLucene<SegToken>(charArray, i, j, WordType::STRING, frequency);
				segGraph->addToken(token);
				i = j;

				break;
			}
		}
	}

	// Add two more Tokens: "beginning xx beginning"
	charArray = Utility::START_CHAR_ARRAY;
	frequency = _wordDict->getFrequency(charArray);
	token = newLucene<SegToken>(charArray, -1, 0, WordType::SENTENCE_BEGIN, frequency);
	segGraph->addToken(token);

	// "end xx end"
	charArray = Utility::END_CHAR_ARRAY;
	frequency = _wordDict->getFrequency(charArray);
	token = newLucene<SegToken>(charArray, length, length + 1, WordType::SENTENCE_END, frequency);
	segGraph->addToken(token);

	// TODO: Daniel to remove
	std::cout << "***********Processed SegGraph Start*****************" << std::endl;

	std::wcout << segGraph->toDebugString() << std::endl;

	std::cout << "***********Processed SegGraph End*****************" << std::endl;

	return segGraph;
}

std::vector<int> HHMMSegmenter::getCharTypes(const std::wstring& sentence)
{
	int length = sentence.length();
	std::vector<int> charTypeArray(length);

	// the type of each character by position
	for (int i = 0; i < length; i++)
	{
		charTypeArray[i] = Utility::getCharType(sentence[i]);
	}

	return charTypeArray;
}

Collection<SegTokenPtr> HHMMSegmenter::process(const String& sentence)
{
	SegGraphPtr segGraph = createSegGraph(sentence);
	BiSegGraphPtr biSegGraph = newLucene<BiSegGraph>(segGraph);
	Collection<SegTokenPtr> shortPath = biSegGraph->getShortPath();

	return shortPath;
}
}
}
}
}
}
