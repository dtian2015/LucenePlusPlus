#include "ContribInc.h"

#include "smartcn/hhmm/WordDictionary.h"

#include "MiscUtils.h"
#include "smartcn/Utility.h"

#include <fstream>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/vector.hpp>

namespace Lucene {
namespace Analysis {
namespace Cn {
namespace Smart {
namespace Hhmm {

WordDictionaryPtr WordDictionary::_singleInstance;

WordDictionaryPtr WordDictionary::getInstance()
{
	if (!_singleInstance)
	{
		_singleInstance = newLucene<WordDictionary>();
		_singleInstance->load();
	}

	return _singleInstance;
}

void WordDictionary::load()
{
	// Deserialize from binary dictionay file
	const SingleString dictFile = MiscUtils::GetAsianDictionaryPathAsSingleString() + "/coredict.dat";
	std::ifstream ifs(dictFile);
	boost::archive::binary_iarchive ia(ifs);

	// extract word index table
	ia& _wordIndexTable;

	// extract char index table
	{
		String tmpCharsIndexTable;
		ia& tmpCharsIndexTable;

		_charIndexTable = CharArray::newInstance(tmpCharsIndexTable.size());
		std::copy(tmpCharsIndexTable.begin(), tmpCharsIndexTable.end(), _charIndexTable.get());
	}

	// extract word frequency table
	ia& _wordItem_frequencyTable;

	// extract word char array table
	{
		std::vector<std::vector<String>> tmpWordTable;
		ia& tmpWordTable;

		_wordItem_charArrayTable.resize(tmpWordTable.size());

		for (size_t i = 0; i < tmpWordTable.size(); ++i)
		{
			const auto& words = tmpWordTable[i];
			_wordItem_charArrayTable[i].resize(words.size());

			for (size_t j = 0; j < words.size(); ++j)
			{
				const auto& word = tmpWordTable[i][j];

				if (!word.empty())
				{
					_wordItem_charArrayTable[i][j] = CharArray::newInstance(word.size());
					std::copy(word.begin(), word.end(), _wordItem_charArrayTable[i][j].get());
				}
			}
		}
	}

	ifs.close();
}

short WordDictionary::getWordItemTableIndex(wchar_t c)
{
	int hash1 = static_cast<int>(this->hash1(c) % PRIME_INDEX_LENGTH);
	int hash2 = this->hash2(c) % PRIME_INDEX_LENGTH;

	if (hash1 < 0)
	{
		hash1 = PRIME_INDEX_LENGTH + hash1;
	}

	if (hash2 < 0)
	{
		hash2 = PRIME_INDEX_LENGTH + hash2;
	}

	int index = hash1;
	int i = 1;
	while (_charIndexTable[index] != 0 && _charIndexTable[index] != c && i < PRIME_INDEX_LENGTH)
	{
		index = (hash1 + i * hash2) % PRIME_INDEX_LENGTH;
		i++;
	}

	if (i < PRIME_INDEX_LENGTH && _charIndexTable[index] == c)
	{
		return static_cast<short>(index);
	}
	else
	{
		return -1;
	}
}

int WordDictionary::findInTable(short knownHashIndex, const CharArray& charArray)
{
	if (!charArray || charArray.size() == 0)
	{
		return -1;
	}

	std::vector<CharArray> items = _wordItem_charArrayTable[_wordIndexTable[knownHashIndex]];
	int start = 0, end = items.size() - 1;
	int mid = (start + end) / 2, cmpResult;

	// Binary search for the index of idArray
	while (start <= end)
	{
		cmpResult = Utility::compareArray(items[mid], 0, charArray, 1);

		if (cmpResult == 0)
		{
			return mid; // find it
		}
		else if (cmpResult < 0)
		{
			start = mid + 1;
		}
		else if (cmpResult > 0)
		{
			end = mid - 1;
		}

		mid = (start + end) / 2;
	}

	return -1;
}

int WordDictionary::getPrefixMatch(const CharArray& charArray)
{
	return getPrefixMatch(charArray, 0);
}

int WordDictionary::getPrefixMatch(const CharArray& charArray, int knownStart)
{
	short index = getWordItemTableIndex(charArray[0]);
	if (index == -1)
	{
		return -1;
	}

	std::vector<CharArray> items = _wordItem_charArrayTable[_wordIndexTable[index]];
	int start = knownStart, end = items.size() - 1;

	int mid = (start + end) / 2, cmpResult;

	// Binary search for the index of idArray
	while (start <= end)
	{
		cmpResult = Utility::compareArrayByPrefix(charArray, 1, items[mid], 0);
		if (cmpResult == 0)
		{
			// Get the first item which match the current word
			while (mid >= 0 && Utility::compareArrayByPrefix(charArray, 1, items[mid], 0) == 0)
			{
				mid--;
			}

			mid++;
			return mid; // Find the first word that uses charArray as prefix.
		}
		else if (cmpResult < 0)
		{
			end = mid - 1;
		}
		else
		{
			start = mid + 1;
		}
		mid = (start + end) / 2;
	}
	return -1;
}

int WordDictionary::getFrequency(const CharArray& charArray)
{
	short hashIndex = getWordItemTableIndex(charArray[0]);
	if (hashIndex == -1)
	{
		return 0;
	}

	int itemIndex = findInTable(hashIndex, charArray);
	if (itemIndex != -1)
	{
		return _wordItem_frequencyTable[_wordIndexTable[hashIndex]][itemIndex];
	}

	return 0;
}

bool WordDictionary::isEqual(const CharArray& charArray, int itemIndex)
{
	short hashIndex = getWordItemTableIndex(charArray[0]);

	return Utility::compareArray(charArray, 1, _wordItem_charArrayTable[_wordIndexTable[hashIndex]][itemIndex], 0) == 0;
}
}
}
}
}
}
