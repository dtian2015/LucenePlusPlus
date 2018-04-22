#include "TestInc.h"

#include "BaseTokenStreamFixture.h"
#include "CharTermAttribute.h"
#include "Random.h"
#include "ReusableAnalyzerBase.h"
#include "StringReader.h"
#include "TestUtils.h"
#include "UnicodeUtils.h"

#include "kuromoji/JapaneseTokenizer.h"

using namespace Lucene;
using namespace Lucene::Analysis::Ja;

class TestExtendedMode : public BaseTokenStreamFixture
{
public:
	DECLARE_SHARED_PTR(ReusableAnalyzerExtended);
	class ReusableAnalyzerExtended : public ReusableAnalyzerBase
	{
	public:
		LUCENE_CLASS(ReusableAnalyzerExtended);

	protected:
		ReusableAnalyzerBase::TokenStreamComponentsPtr createComponents(const String& fieldName, ReaderPtr reader)
		{
			TokenizerPtr tokenizer = newLucene<JapaneseTokenizer>(reader, nullptr, true, JapaneseTokenizer::Mode::EXTENDED);
			return newLucene<TokenStreamComponents>(tokenizer, tokenizer);
		}
	};

	AnalyzerPtr extendedAnalyzer = newLucene<ReusableAnalyzerExtended>();
};

TEST_F(TestExtendedMode, testSurrogates)
{
	checkAnalyzesTo(extendedAnalyzer, L"𩬅艱鍟䇹愯瀛", newCollection<String>(L"𩬅", L"艱", L"鍟", L"䇹", L"愯", L"瀛"));
}

TEST_F(TestExtendedMode, testSurrogates2)
{
	int numIterations = atLeast(100);
	for (int i = 0; i < numIterations; i++)
	{
		String s = randomUnicodeString(newLucene<Random>(), 100);
		TokenStreamPtr ts = extendedAnalyzer->tokenStream(L"foo", newLucene<StringReader>(s));
		CharTermAttributePtr termAtt = ts->addAttribute<CharTermAttribute>();
		ts->reset();

		while (ts->incrementToken())
		{
			EXPECT_TRUE(UnicodeUtil::validUTF16String(termAtt->buffer()));
		}
	}
}
