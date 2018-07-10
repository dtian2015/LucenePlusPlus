#pragma once

#include "LuceneContrib.h"
#include "LuceneObject.h"
#include "kuromoji/JaTypes.h"

#include <string>
#include <vector>

namespace Lucene {
namespace Analysis {
namespace Ja {
namespace Dict {

/// <summary>
/// Dictionary interface for retrieving morphological data
/// by id.
/// </summary>
class LPPCONTRIBAPI Dictionary : public LuceneObject
{
public:
	virtual ~Dictionary() {}
	LUCENE_CLASS(Dictionary);

	const String INTERNAL_SEPARATOR = L" ";

	/// <summary>
	/// Get left id of specified word </summary>
	/// <param name="wordId">
	/// @return	left id </param>
	virtual int getLeftId(int wordId) = 0;

	/// <summary>
	/// Get right id of specified word </summary>
	/// <param name="wordId">
	/// @return	left id </param>
	virtual int getRightId(int wordId) = 0;

	/// <summary>
	/// Get word cost of specified word </summary>
	/// <param name="wordId">
	/// @return	left id </param>
	virtual int getWordCost(int wordId) = 0;

	/// <summary>
	/// Get Part-Of-Speech of tokens </summary>
	/// <param name="wordId"> word ID of token </param>
	/// <returns> Part-Of-Speech of the token </returns>
	virtual String getPartOfSpeech(int wordId) = 0;

	/// <summary>
	/// Get reading of tokens </summary>
	/// <param name="wordId"> word ID of token </param>
	/// <returns> Reading of the token </returns>
	virtual String getReading(int wordId, CharArray surface, int off, int len) = 0;

	/// <summary>
	/// Get base form of word </summary>
	/// <param name="wordId"> word ID of token </param>
	/// <returns> Base form (only different for inflected words, otherwise null) </returns>
	virtual String getBaseForm(int wordId, CharArray surface, int off, int len) = 0;

	/// <summary>
	/// Get pronunciation of tokens </summary>
	/// <param name="wordId"> word ID of token </param>
	/// <returns> Pronunciation of the token </returns>
	virtual String getPronunciation(int wordId, CharArray surface, int off, int len) = 0;

	/// <summary>
	/// Get inflection type of tokens </summary>
	/// <param name="wordId"> word ID of token </param>
	/// <returns> inflection type, or null </returns>
	virtual String getInflectionType(int wordId) = 0;

	/// <summary>
	/// Get inflection form of tokens </summary>
	/// <param name="wordId"> word ID of token </param>
	/// <returns> inflection form, or null </returns>
	virtual String getInflectionForm(int wordId) = 0;
	// TODO: maybe we should have a optimal method, a non-typesafe
	// 'getAdditionalData' if other dictionaries like unidic have additional data
};
}
}
}
}
