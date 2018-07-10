#include "TestInc.h"

#include "BaseTokenStreamFixture.h"
#include "KeywordTokenizer.h"
#include "ReusableAnalyzerBase.h"

#include "kuromoji/JapaneseBaseFormFilter.h"
#include "kuromoji/JapaneseTokenizer.h"

using namespace Lucene;
using namespace Lucene::Analysis::Ja;

class TestJapaneseBaseFormFilter : public BaseTokenStreamFixture
{
public:
	DECLARE_SHARED_PTR(ReusableAnalyzer);
	class ReusableAnalyzer : public ReusableAnalyzerBase
	{
	public:
		LUCENE_CLASS(ReusableAnalyzer);

	protected:
		ReusableAnalyzerBase::TokenStreamComponentsPtr createComponents(const String& fieldName, ReaderPtr reader)
		{
			TokenizerPtr tokenizer = newLucene<JapaneseTokenizer>(reader, nullptr, true, JapaneseTokenizer::DEFAULT_MODE);
			return newLucene<TokenStreamComponents>(tokenizer, newLucene<JapaneseBaseFormFilter>(tokenizer));
		}
	};

	AnalyzerPtr analyzer = newLucene<ReusableAnalyzer>();

	DECLARE_SHARED_PTR(ReusableKeywordAnalyzer);
	class ReusableKeywordAnalyzer : public ReusableAnalyzerBase
	{
	public:
		LUCENE_CLASS(ReusableKeywordAnalyzer);

	protected:
		ReusableAnalyzerBase::TokenStreamComponentsPtr createComponents(const String& fieldName, ReaderPtr reader)
		{
			TokenizerPtr tokenizer = newLucene<KeywordTokenizer>(reader);
			return newLucene<TokenStreamComponents>(tokenizer, newLucene<JapaneseBaseFormFilter>(tokenizer));
		}
	};
};

TEST_F(TestJapaneseBaseFormFilter, testBasics)
{
	checkAnalyzesTo(
		analyzer, L"それはまだ実験段階にあります",
		newCollection<String>(L"それ", L"は", L"まだ", L"実験", L"段階", L"に", L"ある", L"ます"));
}

TEST_F(TestJapaneseBaseFormFilter, testEnglish)
{
	checkAnalyzesTo(analyzer, L"this atest", newCollection<String>(L"this", L"atest"));
}

TEST_F(TestJapaneseBaseFormFilter, testEmptyTerm)
{
	AnalyzerPtr keywordAnalyzer = newLucene<ReusableKeywordAnalyzer>();
	checkOneTermReuse(keywordAnalyzer, L"", L"");
}
