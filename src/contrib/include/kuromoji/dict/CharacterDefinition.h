#pragma once

#include "LuceneObject.h"
#include "kuromoji/JaTypes.h"

namespace Lucene {
namespace Analysis {
namespace Ja {
namespace Dict {

/// <summary>
/// Character category data.
/// </summary>
class LPPCONTRIBAPI CharacterDefinition final : public LuceneObject
{
public:
	LUCENE_CLASS(CharacterDefinition);

	static const String FILENAME_CHARACTER_DEFINITION;
	static const String HEADER;
	static constexpr int VERSION = 1;

	static const int CLASS_COUNT = 12;

private:
	// only used internally for lookup:
	enum class CharacterClass
	{
		NGRAM,
		DEFAULT,
		SPACE,
		SYMBOL,
		NUMERIC,
		ALPHA,
		CYRILLIC,
		GREEK,
		HIRAGANA,
		KATAKANA,
		KANJI,
		KANJINUMERIC
	};

	static const std::map<String, CharacterClass> CHARACTERCLASS_STRING_MAP;

	static String DICT_PATH;

private:
	ByteArray _characterCategoryMap = ByteArray::newInstance(0x10000);

	std::vector<bool> _invokeMap = std::vector<bool>(CLASS_COUNT);
	std::vector<bool> _groupMap = std::vector<bool>(CLASS_COUNT);

public:
	// the classes:
	static const char NGRAM;
	static const char DEFAULT;
	static const char SPACE;
	static const char SYMBOL;
	static const char NUMERIC;
	static const char ALPHA;
	static const char CYRILLIC;
	static const char GREEK;
	static const char HIRAGANA;
	static const char KATAKANA;
	static const char KANJI;
	static const char KANJINUMERIC;

private:
	CharacterDefinition();

public:
	char getCharacterClass(char16_t c) const;

	bool isInvoke(char16_t c) const;

	bool isGroup(char16_t c) const;

	bool isKanji(char16_t c) const;

	static char lookupCharacterClass(const String& characterClassName);

	static const CharacterDefinitionPtr& getInstance();
};
}
}
}
}
