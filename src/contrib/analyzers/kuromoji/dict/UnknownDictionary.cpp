#include "ContribInc.h"

#include "kuromoji/dict/UnknownDictionary.h"

namespace Lucene {
namespace Analysis {
namespace Ja {
namespace Dict {

UnknownDictionary::UnknownDictionary()
{
	initializeDictionaries();
}

int UnknownDictionary::lookup(CharArray text, int offset, int len) const
{
	if (!_characterDefinition->isGroup(text[offset]))
	{
		return 1;
	}

	// Extract unknown word. Characters with the same character class are considered to be part of unknown word
	char characterIdOfFirstCharacter = _characterDefinition->getCharacterClass(static_cast<char16_t>(text[offset]));
	int length = 1;

	for (int i = 1; i < len; i++)
	{
		if (characterIdOfFirstCharacter == _characterDefinition->getCharacterClass(static_cast<char16_t>(text[offset + i])))
		{
			length++;
		}
		else
		{
			break;
		}
	}

	return length;
}

const CharacterDefinitionPtr& UnknownDictionary::getCharacterDefinition()
{
	return _characterDefinition;
}

String UnknownDictionary::getReading(int wordId, CharArray surface, int off, int len)
{
	return L"";
}

String UnknownDictionary::getInflectionType(int wordId)
{
	return L"";
}

String UnknownDictionary::getInflectionForm(int wordId)
{
	return L"";
}

const UnknownDictionaryPtr& UnknownDictionary::getInstance()
{
	static UnknownDictionaryPtr INSTANCE(new UnknownDictionary());
	return INSTANCE;
}
}
}
}
}
