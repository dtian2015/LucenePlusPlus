#include "TestInc.h"

#include "LuceneTestFixture.h"
#include "kuromoji/util/ToStringUtil.h"

using namespace Lucene;
using namespace Lucene::Analysis::Ja::Util;

typedef LuceneTestFixture TestToStringUtil;

TEST_F(TestToStringUtil, testPOS)
{
	ASSERT_EQ(L"noun-suffix-verbal", ToStringUtil::getPOSTranslation(L"名詞-接尾-サ変接続"));
}

TEST_F(TestToStringUtil, testHepburn)
{
	EXPECT_EQ(L"majan", ToStringUtil::getRomanization(L"マージャン"));
	EXPECT_EQ(L"uroncha", ToStringUtil::getRomanization(L"ウーロンチャ"));
	EXPECT_EQ(L"chahan", ToStringUtil::getRomanization(L"チャーハン"));
	EXPECT_EQ(L"chashu", ToStringUtil::getRomanization(L"チャーシュー"));
	EXPECT_EQ(L"shumai", ToStringUtil::getRomanization(L"シューマイ"));
}
