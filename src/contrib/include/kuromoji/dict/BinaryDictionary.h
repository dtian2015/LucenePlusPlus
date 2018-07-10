#pragma once

#include "IntsRef.h"
#include "kuromoji/JaTypes.h"
#include "kuromoji/dict/Dictionary.h"

namespace Lucene {
namespace Analysis {
namespace Ja {
namespace Dict {

/// <summary>
/// Base class for a binary-encoded in-memory dictionary.
/// </summary>
class LPPCONTRIBAPI BinaryDictionary : public Dictionary
{
public:
	virtual ~BinaryDictionary() {}
	LUCENE_CLASS(BinaryDictionary);

	static const String DICT_FILENAME_SUFFIX;
	static const String TARGETMAP_FILENAME_SUFFIX;
	static const String POSDICT_FILENAME_SUFFIX;

	static const String DICT_HEADER;
	static const String TARGETMAP_HEADER;
	static const String POSDICT_HEADER;
	static constexpr int VERSION = 1;

private:
	ByteArray _buffer;
	std::vector<int> _targetMapOffsets;
	std::vector<int> _targetMap;
	std::vector<String> _posDict;
	std::vector<String> _inflTypeDict;
	std::vector<String> _inflFormDict;

protected:
	BinaryDictionary();

	virtual void initializeDictionaries();

	static String DICT_PATH;

public:
	virtual void lookupWordIds(int sourceId, OffsetAndLength& ref);

	virtual int getWordId(int offset);

	virtual int getLeftId(int wordId);

	virtual int getRightId(int wordId);

	virtual int getWordCost(int wordId);

	virtual String getBaseForm(int wordId, CharArray surfaceForm, int off, int len);

	virtual String getReading(int wordId, CharArray surface, int off, int len);

	virtual String getPartOfSpeech(int wordId);

	virtual String getPronunciation(int wordId, CharArray surface, int off, int len);

	virtual String getInflectionType(int wordId);

	virtual String getInflectionForm(int wordId);

private:
	static int baseFormOffset(int wordId);

	int readingOffset(int wordId);

	int pronunciationOffset(int wordId);

	bool hasBaseFormData(int wordId);

	bool hasReadingData(int wordId);

	bool hasPronunciationData(int wordId);

	String readString(int offset, int length, bool kana);

public:
	/// <summary>
	/// flag that the entry has baseform data. otherwise its not inflected (same as surface form) </summary>
	static constexpr int HAS_BASEFORM = 1;

	/// <summary>
	/// flag that the entry has reading data. otherwise reading is surface form converted to katakana </summary>
	static constexpr int HAS_READING = 2;

	/// <summary>
	/// flag that the entry has pronunciation data. otherwise pronunciation is the reading </summary>
	static constexpr int HAS_PRONUNCIATION = 4;
};
}
}
}
}
