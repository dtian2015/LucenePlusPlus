#pragma once

#include "TokenInfoFST.h"
#include "kuromoji/JaTypes.h"
#include "kuromoji/dict/Dictionary.h"

namespace Lucene {
namespace Analysis {
namespace Ja {
namespace Dict {

/// <summary>
/// Class for building a User Dictionary.
/// This class allows for custom segmentation of phrases.
/// </summary>
class LPPCONTRIBAPI UserDictionary final : public Dictionary
{
private:
	// phrase text -> phrase ID
	TokenInfoFSTPtr _fst;

	// holds wordid, length, length... indexed by phrase ID
	std::vector<std::vector<int>> _segmentations;

	// holds readings and POS, indexed by wordid
	Collection<String> _data;

	static constexpr int CUSTOM_DICTIONARY_WORD_ID_OFFSET = 100000000;

public:
	LUCENE_CLASS(UserDictionary);
	static constexpr int WORD_COST = -100000;

	static constexpr int LEFT_ID = 5;

	static constexpr int RIGHT_ID = 5;

	UserDictionary(ReaderPtr reader);

public:
	/// <summary>
	/// Lookup words in text </summary>
	/// <param name="chars"> text </param>
	/// <param name="off"> offset into text </param>
	/// <param name="len"> length of text </param>
	/// <returns> array of {wordId, position, length} </returns>
	std::vector<std::vector<int>> lookup(CharArray chars, int off, int len);

	TokenInfoFSTPtr getFST();

private:
	static std::vector<std::vector<int>> const EMPTY_RESULT;

	/// <summary>
	/// Convert Map of index and wordIdAndLength to array of {wordId, index, length} </summary>
	/// <param name="input"> </param>
	/// <returns> array of {wordId, index, length} </returns>
	std::vector<std::vector<int>> toIndexArray(const std::map<int, std::vector<int>>& input);

public:
	std::vector<int> lookupSegmentation(int phraseID);

	int getLeftId(int wordId);

	int getRightId(int wordId);

	int getWordCost(int wordId);

	String getReading(int wordId, CharArray surface, int off, int len);

	String getPartOfSpeech(int wordId);

	String getBaseForm(int wordId, CharArray surface, int off, int len);

	String getPronunciation(int wordId, CharArray surface, int off, int len);

	String getInflectionType(int wordId);

	String getInflectionForm(int wordId);

private:
	Collection<String> getAllFeaturesArray(int wordId);

	String getFeature(int wordId, const std::vector<int>& fields);
};
}
}
}
}
