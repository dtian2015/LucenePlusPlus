#pragma once

#include "kuromoji/JaTypes.h"
//#include "kuromoji/JapaneseTokenizer.h"    // TODO: Uncomment this once JapaneseTokenizer converted
#include "kuromoji/dict/Dictionary.h"

namespace Lucene {
namespace Analysis {
namespace Ja {

// TODO: Use JapaneseTokenizer::Type once JapaneseTokenizer converted
enum class Type
{
	KNOWN,
	UNKNOWN,
	USER
};

/// <summary>
/// Analyzed token with morphological data from its dictionary.
/// </summary>
class LPPCONTRIBAPI Token : public LuceneObject
{
private:
	const Dict::DictionaryPtr _dictionary;

	const int _wordId;

	CharArray _surfaceForm;
	const int _offset;
	const int _length;

	const int _position;
	int _positionLength = 0;

	// TODO: Uncomment following line once JapaneseTokenizer converted
	//	const JapaneseTokenizer::Type _type;

	const Type _type;

public:
	LUCENE_CLASS(Token);

	Token(int wordId, CharArray surfaceForm, int offset, int length, Type type, int position, Dict::DictionaryPtr dictionary);

	virtual String toString();

	/// <returns> surfaceForm </returns>
	virtual CharArray getSurfaceForm();

	/// <returns> offset into surfaceForm </returns>
	virtual int getOffset();

	/// <returns> length of surfaceForm </returns>
	virtual int getLength();

	/// <returns> surfaceForm as a String </returns>
	virtual String getSurfaceFormString();

	/// <returns> reading. null if token doesn't have reading. </returns>
	virtual String getReading();

	/// <returns> pronunciation. null if token doesn't have pronunciation. </returns>
	virtual String getPronunciation();

	/// <returns> part of speech. </returns>
	virtual String getPartOfSpeech();

	/// <returns> inflection type or null </returns>
	virtual String getInflectionType();

	/// <returns> inflection form or null </returns>
	virtual String getInflectionForm();

	/// <returns> base form or null if token is not inflected </returns>
	virtual String getBaseForm();

	/// <summary>
	/// Returns true if this token is known word </summary>
	/// <returns> true if this token is in standard dictionary. false if not. </returns>
	virtual bool isKnown();

	/// <summary>
	/// Returns true if this token is unknown word </summary>
	/// <returns> true if this token is unknown word. false if not. </returns>
	virtual bool isUnknown();

	/// <summary>
	/// Returns true if this token is defined in user dictionary </summary>
	/// <returns> true if this token is in user dictionary. false if not. </returns>
	virtual bool isUser();

	/// <summary>
	/// Get index of this token in input text </summary>
	/// <returns> position of token </returns>
	virtual int getPosition();

	/// <summary>
	/// Set the position length (in tokens) of this token.  For normal
	/// tokens this is 1; for compound tokens it's > 1.
	/// </summary>
	virtual void setPositionLength(int positionLength);

	/// <summary>
	/// Get the length (in tokens) of this token.  For normal
	/// tokens this is 1; for compound tokens it's > 1. </summary>
	/// <returns> position length of token </returns>
	virtual int getPositionLength();
};
}
}
}
