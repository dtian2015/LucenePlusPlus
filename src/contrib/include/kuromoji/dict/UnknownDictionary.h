#pragma once

#include "kuromoji/JaTypes.h"
#include "kuromoji/dict/BinaryDictionary.h"
#include "kuromoji/dict/CharacterDefinition.h"

namespace Lucene {
namespace Analysis {
namespace Ja {
namespace Dict {

/// <summary>
/// Dictionary for unknown-word handling.
/// </summary>
class LPPCONTRIBAPI UnknownDictionary final : public BinaryDictionary
{
private:
	const CharacterDefinitionPtr& _characterDefinition = CharacterDefinition::getInstance();

public:
	LUCENE_CLASS(UnknownDictionary);

	int lookup(CharArray text, int offset, int len) const;

	const CharacterDefinitionPtr& getCharacterDefinition();

	String getReading(int wordId, CharArray surface, int off, int len);

	String getInflectionType(int wordId);

	String getInflectionForm(int wordId);

	static const UnknownDictionaryPtr& getInstance();

protected:
	UnknownDictionary();
};
}
}
}
}
