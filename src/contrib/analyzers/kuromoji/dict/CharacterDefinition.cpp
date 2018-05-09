#include "ContribInc.h"

#include "kuromoji/dict/CharacterDefinition.h"

#include "CodecUtil.h"
#include "FSDirectory.h"
#include "FileUtils.h"
#include "_SimpleFSDirectory.h"

namespace Lucene {
namespace Analysis {
namespace Ja {
namespace Dict {

const String CharacterDefinition::FILENAME_CHARACTER_DEFINITION = L"CharacterDefinition.dat";
const String CharacterDefinition::HEADER = L"kuromoji_cd";
String CharacterDefinition::DICT_PATH;

const char CharacterDefinition::NGRAM = static_cast<char>(static_cast<int>(CharacterClass::NGRAM));
const char CharacterDefinition::DEFAULT = static_cast<char>(static_cast<int>(CharacterClass::DEFAULT));
const char CharacterDefinition::SPACE = static_cast<char>(static_cast<int>(CharacterClass::SPACE));
const char CharacterDefinition::SYMBOL = static_cast<char>(static_cast<int>(CharacterClass::SYMBOL));
const char CharacterDefinition::NUMERIC = static_cast<char>(static_cast<int>(CharacterClass::NUMERIC));
const char CharacterDefinition::ALPHA = static_cast<char>(static_cast<int>(CharacterClass::ALPHA));
const char CharacterDefinition::CYRILLIC = static_cast<char>(static_cast<int>(CharacterClass::CYRILLIC));
const char CharacterDefinition::GREEK = static_cast<char>(static_cast<int>(CharacterClass::GREEK));
const char CharacterDefinition::HIRAGANA = static_cast<char>(static_cast<int>(CharacterClass::HIRAGANA));
const char CharacterDefinition::KATAKANA = static_cast<char>(static_cast<int>(CharacterClass::KATAKANA));
const char CharacterDefinition::KANJI = static_cast<char>(static_cast<int>(CharacterClass::KANJI));
const char CharacterDefinition::KANJINUMERIC = static_cast<char>(static_cast<int>(CharacterClass::KANJINUMERIC));

const std::map<String, CharacterDefinition::CharacterClass> CharacterDefinition::CHARACTERCLASS_STRING_MAP = {
	{L"ngram", CharacterClass::NGRAM},		 {L"default", CharacterClass::DEFAULT}, {L"space", CharacterClass::SPACE},
	{L"symbol", CharacterClass::SYMBOL},	 {L"numeric", CharacterClass::NUMERIC}, {L"alpha", CharacterClass::ALPHA},
	{L"cyrillic", CharacterClass::CYRILLIC}, {L"greek", CharacterClass::GREEK},		{L"hiragana", CharacterClass::HIRAGANA},
	{L"katakana", CharacterClass::KATAKANA}, {L"kanji", CharacterClass::KANJI},		{L"kanjinumeric", CharacterClass::KANJINUMERIC}};

CharacterDefinition::CharacterDefinition()
{
	DICT_PATH = GetDictionaryPath();
	LuceneException priorE;

	const String definitionFile = FileUtils::joinPath(DICT_PATH, FILENAME_CHARACTER_DEFINITION);
	DataInputPtr is = newLucene<SimpleFSIndexInput>(definitionFile, BufferedIndexInput::BUFFER_SIZE, FSDirectory::DEFAULT_READ_CHUNK_SIZE);

	try
	{
		Util::CodecUtil::checkHeader(*is, HEADER, VERSION, VERSION);

		is->readBytes(_characterCategoryMap.get(), 0, _characterCategoryMap.size());

		for (int i = 0; i < CLASS_COUNT; i++)
		{
			const char b = is->readByte();
			_invokeMap[i] = (b & 0x01) != 0;
			_groupMap[i] = (b & 0x02) != 0;
		}
	}
	catch (const IOException& ioe)
	{
		priorE = ioe;
	}

	if (is)
	{
		is->close();
	}

	priorE.throwException();
}

char CharacterDefinition::getCharacterClass(char16_t c) const
{
	return _characterCategoryMap[c];
}

bool CharacterDefinition::isInvoke(char16_t c) const
{
	return _invokeMap[_characterCategoryMap[c]];
}

bool CharacterDefinition::isGroup(char16_t c) const
{
	return _groupMap[_characterCategoryMap[c]];
}

bool CharacterDefinition::isKanji(char16_t c) const
{
	const char characterClass = _characterCategoryMap[c];
	return characterClass == KANJI || characterClass == KANJINUMERIC;
}

char CharacterDefinition::lookupCharacterClass(const String& characterClassName)
{
	String characterClass = characterClassName;
	std::for_each(characterClass.begin(), characterClass.end(), std::towlower);
	return static_cast<char>(CHARACTERCLASS_STRING_MAP.at(characterClass));
}

const CharacterDefinitionPtr& CharacterDefinition::getInstance()
{
	static CharacterDefinitionPtr INSTANCE(new CharacterDefinition());
	return INSTANCE;
}
}
}
}
}
