#pragma once

#include "Lucene.h"
#include "MiscUtils.h"

namespace Lucene {
namespace Analysis {
namespace Ja {

DECLARE_SHARED_PTR(GraphvizFormatter)
DECLARE_SHARED_PTR(JapaneseAnalyzer)
DECLARE_SHARED_PTR(JapaneseBaseFormFilter)
DECLARE_SHARED_PTR(JapanesePartOfSpeechStopFilter)
DECLARE_SHARED_PTR(JapaneseTokenizer)
DECLARE_SHARED_PTR(Token)

#define FSTLong Lucene::Util::FST::FST<Long>
#define FSTArcLong Lucene::Util::FST::FST<Long>::Arc<Long>

template <typename T>
T GetOutputValue(boost::shared_ptr<long> output)
{
	if (output)
	{
		return static_cast<T>(*output);
	}

	return static_cast<T>(0);
}

struct OffsetAndLength
{
	int offset = 0;
	int length = 0;
};

namespace Dict {

DECLARE_SHARED_PTR(BinaryDictionary)
DECLARE_SHARED_PTR(CharacterDefinition)
DECLARE_SHARED_PTR(ConnectionCosts)
DECLARE_SHARED_PTR(Dictionary)
DECLARE_SHARED_PTR(TokenInfoDictionary)
DECLARE_SHARED_PTR(TokenInfoFST)
DECLARE_SHARED_PTR(UnknownDictionary)
DECLARE_SHARED_PTR(UserDictionary)

static const std::string DICT_ENVIRONMENT = "LUCENE_DICT_PATH";

static String GetDictionaryPath()
{
	const std::string dictEnv = MiscUtils::GetEnvironmentVar(DICT_ENVIRONMENT);
	if (dictEnv.empty())
	{
		boost::throw_exception(RuntimeException(L"Japanese dictionary path is missing"));
	}

	return String(dictEnv.begin(), dictEnv.end());
}
}

namespace TokenAttributes {

DECLARE_SHARED_PTR(BaseFormAttribute)
DECLARE_SHARED_PTR(InflectionAttribute)
DECLARE_SHARED_PTR(PartOfSpeechAttribute)
DECLARE_SHARED_PTR(ReadingAttribute)
}
}
}
}
