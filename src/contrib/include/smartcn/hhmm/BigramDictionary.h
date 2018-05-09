#pragma once

#include "smartcn/CnTypes.h"
#include "smartcn/hhmm/AbstractDictionary.h"

namespace Lucene {
namespace Analysis {
namespace Cn {
namespace Smart {
namespace Hhmm {

/// <summary>
/// SmartChineseAnalyzer Bigram dictionary.
/// @lucene.experimental
/// </summary>
class LPPCONTRIBAPI BigramDictionary : public AbstractDictionary
{
public:
	LUCENE_CLASS(BigramDictionary);

	static constexpr wchar_t WORD_SEGMENT_CHAR = L'@';
	static constexpr int PRIME_BIGRAM_LENGTH = 402137;

private:
	static BigramDictionaryPtr _singleInstance;

	/*
	 * The word associations are stored as FNV1 hashcodes, which have a small probability of collision, but save memory.
	 */
	std::vector<int64_t> _bigramHashTable;

	std::vector<int> _frequencyTable;

	int max = 0;

	int repeat = 0;

	// static Logger log = Logger.getLogger(BigramDictionary.class);

public:
	static BigramDictionaryPtr getInstance();

private:
	void load();

	int getAvaliableIndex(int64_t hashId, const CharArray& carray);

	/*
	 * lookup the index into the frequency array.
	 */
	int getBigramItemIndex(const CharArray& carray);

public:
	virtual int getFrequency(const CharArray& carray);
};
}
}
}
}
}
