#pragma once

#include "smartcn/CnTypes.h"
#include "smartcn/hhmm/AbstractDictionary.h"

namespace Lucene {
namespace Analysis {
namespace Cn {
namespace Smart {
namespace Hhmm {

/// <summary>
/// SmartChineseAnalyzer Word Dictionary
/// @lucene.experimental
/// </summary>
class LPPCONTRIBAPI WordDictionary : public AbstractDictionary
{
public:
	LUCENE_CLASS(WordDictionary);

	/// <summary>
	/// Large prime number for hash function
	/// </summary>
	static constexpr int PRIME_INDEX_LENGTH = 12071;

private:
	static WordDictionaryPtr _singleInstance;

	/// <summary>
	/// wordIndexTable guarantees to hash all Chinese characters in Unicode into
	/// PRIME_INDEX_LENGTH array. There will be conflict, but in reality this
	/// program only handles the 6768 characters found in GB2312 plus some
	/// ASCII characters. Therefore in order to guarantee better precision, it is
	/// necessary to retain the original symbol in the charIndexTable.
	/// </summary>
	std::vector<short> _wordIndexTable;

	CharArray _charIndexTable;

	/// <summary>
	/// To avoid taking too much space, the data structure needed to store the
	/// lexicon requires two multidimensional arrays to store word and frequency.
	/// Each word is placed in a char[]. Each char represents a Chinese char or
	/// other symbol.  Each frequency is put into an int. These two arrays
	/// correspond to each other one-to-one. Therefore, one can use
	/// wordItem_charArrayTable[i][j] to look up word from lexicon, and
	/// wordItem_frequencyTable[i][j] to look up the corresponding frequency.
	/// </summary>
	std::vector<std::vector<CharArray>> _wordItem_charArrayTable;

	std::vector<std::vector<int>> _wordItem_frequencyTable;

	// static Logger log = Logger.getLogger(WordDictionary.class);

public:
	/// <summary>
	/// Get the singleton dictionary instance. </summary>
	/// <returns> singleton </returns>
	static WordDictionaryPtr getInstance();

	/// <summary>
	/// Load coredict.mem file.
	/// </summary>
	virtual void load();

private:
	short getWordItemTableIndex(wchar_t c);

	/// <summary>
	/// Look up the text string corresponding with the word char array,
	/// and return the position of the word list.
	/// </summary>
	/// <param name="knownHashIndex"> already figure out position of the first word
	///   symbol charArray[0] in hash table. If not calculated yet, can be
	///   replaced with function int findInTable(char[] charArray). </param>
	/// <param name="charArray"> look up the char array corresponding with the word. </param>
	/// <returns> word location in word array.  If not found, then return -1. </returns>
	int findInTable(short knownHashIndex, const CharArray& charArray);

public:
	/// <summary>
	/// Find the first word in the dictionary that starts with the supplied prefix
	/// </summary>
	/// <seealso cref= #getPrefixMatch(char[], int) </seealso>
	/// <param name="charArray"> input prefix </param>
	/// <returns> index of word, or -1 if not found </returns>
	virtual int getPrefixMatch(const CharArray& charArray);

	/// <summary>
	/// Find the nth word in the dictionary that starts with the supplied prefix
	/// </summary>
	/// <seealso cref= #getPrefixMatch(char[]) </seealso>
	/// <param name="charArray"> input prefix </param>
	/// <param name="knownStart"> relative position in the dictionary to start </param>
	/// <returns> index of word, or -1 if not found </returns>
	virtual int getPrefixMatch(const CharArray& charArray, int knownStart);

	/// <summary>
	/// Get the frequency of a word from the dictionary
	/// </summary>
	/// <param name="charArray"> input word </param>
	/// <returns> word frequency, or zero if the word is not found </returns>
	virtual int getFrequency(const CharArray& charArray);

	/// <summary>
	/// Return true if the dictionary entry at itemIndex for table charArray[0] is charArray
	/// </summary>
	/// <param name="charArray"> input word </param>
	/// <param name="itemIndex"> item index for table charArray[0] </param>
	/// <returns> true if the entry exists </returns>
	virtual bool isEqual(const CharArray& charArray, int itemIndex);
};
}
}
}
}
}
