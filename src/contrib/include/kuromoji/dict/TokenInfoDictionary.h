#pragma once

#include "kuromoji/JaTypes.h"
#include "kuromoji/dict/BinaryDictionary.h"
#include "kuromoji/dict/TokenInfoFST.h"

namespace Lucene {
namespace Analysis {
namespace Ja {
namespace Dict {

/// <summary>
/// Binary dictionary implementation for a known-word dictionary model:
/// Words are encoded into an FST mapping to a list of wordIDs.
/// </summary>
class LPPCONTRIBAPI TokenInfoDictionary final : public BinaryDictionary
{
public:
	LUCENE_CLASS(TokenInfoDictionary);

	static const String FST_FILENAME_SUFFIX;

private:
	TokenInfoFSTPtr _fst;

	TokenInfoDictionary();

public:
	TokenInfoFSTPtr getFST();

	static const TokenInfoDictionaryPtr& getInstance();
};
}
}
}
}
