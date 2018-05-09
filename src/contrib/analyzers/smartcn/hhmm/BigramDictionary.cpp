#include "ContribInc.h"

#include "smartcn/hhmm/BigramDictionary.h"

#include "MiscUtils.h"

#include <fstream>

#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/vector.hpp>

namespace Lucene {
namespace Analysis {
namespace Cn {
namespace Smart {
namespace Hhmm {

BigramDictionaryPtr BigramDictionary::_singleInstance;

BigramDictionaryPtr BigramDictionary::getInstance()
{
	if (!_singleInstance)
	{
		_singleInstance = newLucene<BigramDictionary>();
		_singleInstance->load();
	}

	return _singleInstance;
}

void BigramDictionary::load()
{
	// Deserialize from binary dictionay file
	const SingleString dictFile = MiscUtils::GetAsianDictionaryPathAsSingleString() + "/bigramdict.dat";
	std::ifstream ifs(dictFile);
	boost::archive::binary_iarchive ia(ifs);

	// extract bigram hash table
	ia& _bigramHashTable;

	// extract bigram frequency table
	ia& _frequencyTable;

	ifs.close();
}

int BigramDictionary::getAvaliableIndex(int64_t hashId, const CharArray& carray)
{
	int hash1 = static_cast<int>(hashId % PRIME_BIGRAM_LENGTH);
	int hash2 = this->hash2(carray) % PRIME_BIGRAM_LENGTH;

	if (hash1 < 0)
	{
		hash1 = PRIME_BIGRAM_LENGTH + hash1;
	}

	if (hash2 < 0)
	{
		hash2 = PRIME_BIGRAM_LENGTH + hash2;
	}

	int index = hash1;
	int i = 1;
	while (_bigramHashTable[index] != 0 && _bigramHashTable[index] != hashId && i < PRIME_BIGRAM_LENGTH)
	{
		index = (hash1 + i * hash2) % PRIME_BIGRAM_LENGTH;
		i++;
	}
	// System.out.println(i - 1);

	if (i < PRIME_BIGRAM_LENGTH && (_bigramHashTable[index] == 0 || _bigramHashTable[index] == hashId))
	{
		return index;
	}
	else
	{
		return -1;
	}
}

int BigramDictionary::getBigramItemIndex(const CharArray& carray)
{
	int64_t hashId = hash1(carray);
	int hash1 = static_cast<int>(hashId % PRIME_BIGRAM_LENGTH);
	int hash2 = this->hash2(carray) % PRIME_BIGRAM_LENGTH;
	if (hash1 < 0)
	{
		hash1 = PRIME_BIGRAM_LENGTH + hash1;
	}
	if (hash2 < 0)
	{
		hash2 = PRIME_BIGRAM_LENGTH + hash2;
	}
	int index = hash1;
	int i = 1;
	repeat++;
	while (_bigramHashTable[index] != 0 && _bigramHashTable[index] != hashId && i < PRIME_BIGRAM_LENGTH)
	{
		index = (hash1 + i * hash2) % PRIME_BIGRAM_LENGTH;
		i++;
		repeat++;
		if (i > max)
		{
			max = i;
		}
	}
	// System.out.println(i - 1);

	if (i < PRIME_BIGRAM_LENGTH && _bigramHashTable[index] == hashId)
	{
		return index;
	}
	else
	{
		return -1;
	}
}

int BigramDictionary::getFrequency(const CharArray& carray)
{
	int index = getBigramItemIndex(carray);
	if (index != -1)
	{
		return _frequencyTable[index];
	}

	return 0;
}
}
}
}
}
}
