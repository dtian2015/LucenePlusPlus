#include "TestInc.h"

#include "BaseTokenStreamFixture.h"
#include "BufferedReader.h"
#include "FileReader.h"
#include "FileUtils.h"
#include "InputStreamReader.h"
#include "Random.h"
#include "ReusableAnalyzerBase.h"
#include "StringReader.h"
#include "StringUtils.h"
#include "TestUtils.h"

#include "kuromoji/GraphvizFormatter.h"
#include "kuromoji/JapaneseTokenizer.h"
#include "kuromoji/dict/ConnectionCosts.h"
#include "kuromoji/dict/UserDictionary.h"
#include "kuromoji/tokenattributes/BaseFormAttribute.h"
#include "kuromoji/tokenattributes/InflectionAttribute.h"
#include "kuromoji/tokenattributes/PartOfSpeechAttribute.h"
#include "kuromoji/tokenattributes/ReadingAttribute.h"

using namespace Lucene;
using namespace Lucene::Analysis::Ja;
using namespace Lucene::Analysis::Ja::Dict;
using namespace Lucene::Analysis::Ja::TokenAttributes;

class TestJapaneseTokenizer : public BaseTokenStreamFixture
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
			TokenizerPtr tokenizer = newLucene<JapaneseTokenizer>(reader, readDict(), false, JapaneseTokenizer::Mode::SEARCH);
			return newLucene<TokenStreamComponents>(tokenizer, tokenizer);
		}
	};

	AnalyzerPtr analyzer = newLucene<ReusableAnalyzer>();

	DECLARE_SHARED_PTR(ReusableAnalyzerNormal);
	class ReusableAnalyzerNormal : public ReusableAnalyzerBase
	{
	public:
		LUCENE_CLASS(ReusableAnalyzerNormal);

	protected:
		ReusableAnalyzerBase::TokenStreamComponentsPtr createComponents(const String& fieldName, ReaderPtr reader)
		{
			TokenizerPtr tokenizer = newLucene<JapaneseTokenizer>(reader, readDict(), false, JapaneseTokenizer::Mode::NORMAL);
			return newLucene<TokenStreamComponents>(tokenizer, tokenizer);
		}
	};

	AnalyzerPtr analyzerNormal = newLucene<ReusableAnalyzerNormal>();

	DECLARE_SHARED_PTR(ReusableAnalyzerNoPunct);
	class ReusableAnalyzerNoPunct : public ReusableAnalyzerBase
	{
	public:
		LUCENE_CLASS(ReusableAnalyzerNoPunct);

	protected:
		ReusableAnalyzerBase::TokenStreamComponentsPtr createComponents(const String& fieldName, ReaderPtr reader)
		{
			TokenizerPtr tokenizer = newLucene<JapaneseTokenizer>(reader, readDict(), true, JapaneseTokenizer::Mode::SEARCH);
			return newLucene<TokenStreamComponents>(tokenizer, tokenizer);
		}
	};

	AnalyzerPtr analyzerNoPunct = newLucene<ReusableAnalyzerNoPunct>();

	DECLARE_SHARED_PTR(ReusableAnalyzerNoPunctExtended);
	class ReusableAnalyzerNoPunctExtended : public ReusableAnalyzerBase
	{
	public:
		LUCENE_CLASS(ReusableAnalyzerNoPunctExtended);

	protected:
		ReusableAnalyzerBase::TokenStreamComponentsPtr createComponents(const String& fieldName, ReaderPtr reader)
		{
			TokenizerPtr tokenizer = newLucene<JapaneseTokenizer>(reader, readDict(), true, JapaneseTokenizer::Mode::EXTENDED);
			return newLucene<TokenStreamComponents>(tokenizer, tokenizer);
		}
	};

	AnalyzerPtr extendedAnalyzerNoPunct = newLucene<ReusableAnalyzerNoPunctExtended>();

	DECLARE_SHARED_PTR(ReusableAnalyzerFormatter);
	class ReusableAnalyzerFormatter : public ReusableAnalyzerBase
	{
	public:
		LUCENE_CLASS(ReusableAnalyzerFormatter);

		const GraphvizFormatterPtr gv2 = newLucene<GraphvizFormatter>(ConnectionCosts::getInstance());

	protected:
		ReusableAnalyzerBase::TokenStreamComponentsPtr createComponents(const String& fieldName, ReaderPtr reader)
		{
			JapaneseTokenizerPtr tokenizer = newLucene<JapaneseTokenizer>(reader, readDict(), false, JapaneseTokenizer::Mode::SEARCH);
			tokenizer->setGraphvizFormatter(gv2);
			return newLucene<TokenStreamComponents>(tokenizer, tokenizer);
		}
	};

	ReusableAnalyzerFormatterPtr analyzerFormatter = newLucene<ReusableAnalyzerFormatter>();

#pragma mark - misc helper

	void doTestBocchan(int numIterations)
	{
		BufferedReaderPtr reader = newLucene<BufferedReader>(
			newLucene<InputStreamReader>(newLucene<FileReader>(FileUtils::joinPath(getTestDir(), L"ja/bocchan.utf-8"))));
		String line;
		reader->readLine(line);
		reader->close();
		reader->reset();

		/*
		 if (numIterations > 1) {
		 // warmup
		 for (int i = 0; i < numIterations; i++) {
		 final TokenStream ts = analyzer.tokenStream("ignored", new StringReader(line));
		 ts.reset();
		 while(ts.incrementToken());
		 }
		 }
		 */

		for (int i = 0; i < numIterations; i++)
		{
			auto const ts = analyzer->tokenStream(L"ignored", newLucene<StringReader>(line));
			ts->reset();
			while (ts->incrementToken())
				;
		}

		Collection<String> sentences = StringUtils::split(line, L"、|。");
		for (int i = 0; i < numIterations; i++)
		{
			for (const auto& sentence : sentences)
			{
				auto const ts = analyzer->tokenStream(L"ignored", newLucene<StringReader>(sentence));
				ts->reset();
				while (ts->incrementToken())
					;
			}
		}
	}

	void assertReadings(const String& input, const std::vector<String>& readings)
	{
		TokenStreamPtr ts = analyzer->tokenStream(L"ignored", newLucene<StringReader>(input));
		ReadingAttributePtr readingAtt = ts->addAttribute<ReadingAttribute>();
		ts->reset();

		for (const auto& reading : readings)
		{
			EXPECT_TRUE(ts->incrementToken());
			EXPECT_EQ(reading, readingAtt->getReading());
		}

		EXPECT_FALSE(ts->incrementToken());
		ts->end();
	}

	void assertPronunciations(const String& input, const std::vector<String>& pronunciations)
	{
		TokenStreamPtr ts = analyzer->tokenStream(L"ignored", newLucene<StringReader>(input));
		ReadingAttributePtr readingAtt = ts->addAttribute<ReadingAttribute>();
		ts->reset();

		for (const auto& pronunciation : pronunciations)
		{
			EXPECT_TRUE(ts->incrementToken());
			EXPECT_EQ(pronunciation, readingAtt->getPronunciation());
		}

		EXPECT_FALSE(ts->incrementToken());
		ts->end();
	}

	void assertBaseForms(const String& input, const std::vector<String>& baseForms)
	{
		TokenStreamPtr ts = analyzer->tokenStream(L"ignored", newLucene<StringReader>(input));
		BaseFormAttributePtr baseFormAtt = ts->addAttribute<BaseFormAttribute>();
		ts->reset();

		for (const auto& baseForm : baseForms)
		{
			EXPECT_TRUE(ts->incrementToken());
			EXPECT_EQ(baseForm, baseFormAtt->getBaseForm());
		}

		EXPECT_FALSE(ts->incrementToken());
		ts->end();
	}

	void assertInflectionTypes(const std::wstring& input, const std::vector<std::wstring>& inflectionTypes)
	{
		TokenStreamPtr ts = analyzer->tokenStream(L"ignored", newLucene<StringReader>(input));
		InflectionAttributePtr inflectionAtt = ts->addAttribute<InflectionAttribute>();
		ts->reset();

		for (const auto& inflectionType : inflectionTypes)
		{
			EXPECT_TRUE(ts->incrementToken());
			EXPECT_EQ(inflectionType, inflectionAtt->getInflectionType());
		}

		EXPECT_FALSE(ts->incrementToken());
		ts->end();
	}

	void assertInflectionForms(const String& input, const std::vector<String>& inflectionForms)
	{
		TokenStreamPtr ts = analyzer->tokenStream(L"ignored", newLucene<StringReader>(input));
		InflectionAttributePtr inflectionAtt = ts->addAttribute<InflectionAttribute>();
		ts->reset();

		for (const auto& inflectionForm : inflectionForms)
		{
			EXPECT_TRUE(ts->incrementToken());
			EXPECT_EQ(inflectionForm, inflectionAtt->getInflectionForm());
		}

		EXPECT_FALSE(ts->incrementToken());
		ts->end();
	}

	void assertPartsOfSpeech(const String& input, const std::vector<String>& partsOfSpeech)
	{
		TokenStreamPtr ts = analyzer->tokenStream(L"ignored", newLucene<StringReader>(input));
		PartOfSpeechAttributePtr partOfSpeechAtt = ts->addAttribute<PartOfSpeechAttribute>();
		ts->reset();

		for (const auto& partOfSpeech : partsOfSpeech)
		{
			EXPECT_TRUE(ts->incrementToken());
			EXPECT_EQ(partOfSpeech, partOfSpeechAtt->getPartOfSpeech());
		}

		EXPECT_FALSE(ts->incrementToken());
		ts->end();
	}
};

TEST_F(TestJapaneseTokenizer, testNormalMode)
{
	checkAnalyzesTo(analyzerNormal, L"シニアソフトウェアエンジニア", newCollection<String>(L"シニアソフトウェアエンジニア"));
}

TEST_F(TestJapaneseTokenizer, testDecomposition1)
{
	const std::vector<String> output = {L"本来", L"は",   L"貧困", L"層",		L"の",	 L"女性", L"や",   L"子供", L"に",   L"医療",
										L"保護", L"を",   L"提供", L"する",		L"ため",   L"に",   L"創設", L"さ",   L"れ",   L"た",
										L"制度", L"で",   L"ある", L"アメリカ", L"低",	 L"所得", L"者",   L"医療", L"援助", L"制度",
										L"が",   L"今日", L"で",   L"は",		L"その",   L"予算", L"の",   L"約",   L"３",   L"分の",
										L"１",   L"を",   L"老人", L"に",		L"費やし", L"て",   L"いる"};

	const std::vector<int32_t> startOffsets = {0,  2,  4,  6,  7,  8,  10, 11, 13, 14, 16, 18, 19, 21, 23, 25,
											   26, 28, 29, 30, 31, 33, 34, 37, 41, 42, 44, 45, 47, 49, 51, 53,
											   55, 56, 58, 60, 62, 63, 64, 65, 67, 68, 69, 71, 72, 75, 76};

	const std::vector<int32_t> endOffsets = {2,  3,  6,  7,  8,  10, 11, 13, 14, 16, 18, 19, 21, 23, 25, 26, 28, 29, 30, 31, 33, 34, 36, 41,
											 42, 44, 45, 47, 49, 51, 52, 55, 56, 57, 60, 62, 63, 64, 65, 67, 68, 69, 71, 72, 75, 76, 78};

	checkAnalyzesTo(
		analyzerNoPunct, String(L"本来は、貧困層の女性や子供に医療保護を提供するために創設された制度である、") +
			L"アメリカ低所得者医療援助制度が、今日では、その予算の約３分の１を老人に費やしている。",
		Collection<String>::newInstance(output.begin(), output.end()),
		Collection<int32_t>::newInstance(startOffsets.begin(), startOffsets.end()),
		Collection<int32_t>::newInstance(endOffsets.begin(), endOffsets.end()));
}

TEST_F(TestJapaneseTokenizer, testDecomposition2)
{
	checkAnalyzesTo(
		analyzerNoPunct, L"麻薬の密売は根こそぎ絶やさなければならない",
		newCollection<String>(L"麻薬", L"の", L"密売", L"は", L"根こそぎ", L"絶やさ", L"なけれ", L"ば", L"なら", L"ない"),
		newCollection<int32_t>(0, 2, 3, 5, 6, 10, 13, 16, 17, 19), newCollection<int32_t>(2, 3, 5, 6, 10, 13, 16, 17, 19, 21));
}

TEST_F(TestJapaneseTokenizer, testDecomposition3)
{
	checkAnalyzesTo(
		analyzerNoPunct, L"魔女狩大将マシュー・ホプキンス。", newCollection<String>(L"魔女", L"狩", L"大将", L"マシュー", L"ホプキンス"),
		newCollection<int32_t>(0, 2, 3, 5, 10), newCollection<int32_t>(2, 3, 5, 9, 15));
}

TEST_F(TestJapaneseTokenizer, testDecomposition4)
{
	checkAnalyzesTo(
		analyzer, L"これは本ではない", newCollection<String>(L"これ", L"は", L"本", L"で", L"は", L"ない"),
		newCollection<int32_t>(0, 2, 3, 4, 5, 6), newCollection<int32_t>(2, 3, 4, 5, 6, 8));
}

TEST_F(TestJapaneseTokenizer, testDecomposition5)
{
	auto ts = analyzer->tokenStream(
		L"bogus", newLucene<StringReader>(L"くよくよくよくよくよくよくよくよくよくよくよくよくよくよくよくよくよくよくよくよ"));
	ts->reset();
	while (ts->incrementToken())
	{
	}
	ts->end();
	ts->close();
}

TEST_F(TestJapaneseTokenizer, testTwoSentences)
{
	/*
	//TokenStream ts = a.tokenStream("foo", new StringReader("妹の咲子です。俺と年子で、今受験生です。"));
	TokenStream ts = analyzer.tokenStream("foo", new StringReader("&#x250cdf66<!--\"<!--#<!--;?><!--#<!--#><!---->?>-->;"));
	ts.reset();
	CharTermAttribute termAtt = ts.addAttribute(CharTermAttribute.class);
	while(ts.incrementToken()) {
	  System.out.println("  " + termAtt.toString());
	}
	System.out.println("DONE PARSE\n\n");
	*/

	checkAnalyzesTo(
		analyzerNoPunct, L"魔女狩大将マシュー・ホプキンス。 魔女狩大将マシュー・ホプキンス。",
		newCollection<String>(L"魔女", L"狩", L"大将", L"マシュー", L"ホプキンス", L"魔女", L"狩", L"大将", L"マシュー", L"ホプキンス"),
		newCollection<int32_t>(0, 2, 3, 5, 10, 17, 19, 20, 22, 27), newCollection<int32_t>(2, 3, 5, 9, 15, 19, 20, 22, 26, 32));
}

TEST_F(TestJapaneseTokenizer, testLargeDocReliability)
{
	for (int i = 0; i < 100; i++)
	{
		String s = randomUnicodeString(newLucene<Random>(), 10000);
		TokenStreamPtr ts = analyzer->tokenStream(L"foo", newLucene<StringReader>(s));
		ts->reset();
		while (ts->incrementToken())
		{
		}
	}
}

TEST_F(TestJapaneseTokenizer, testSurrogates)
{
	checkAnalyzesTo(analyzer, L"𩬅艱鍟䇹愯瀛", newCollection<String>(L"𩬅", L"艱", L"鍟", L"䇹", L"愯", L"瀛"));
}

TEST_F(TestJapaneseTokenizer, testOnlyPunctuation)
{
	auto ts = analyzerNoPunct->tokenStream(L"foo", newLucene<StringReader>(L"。、。。"));
	ts->reset();
	EXPECT_FALSE(ts->incrementToken());
	ts->end();
}

TEST_F(TestJapaneseTokenizer, testOnlyPunctuationExtended)
{
	auto ts = extendedAnalyzerNoPunct->tokenStream(L"foo", newLucene<StringReader>(L"......"));
	ts->reset();
	EXPECT_FALSE(ts->incrementToken());
	ts->end();
}

TEST_F(TestJapaneseTokenizer, testEnd)
{
	checkTokenStreamContents(
		analyzerNoPunct->tokenStream(L"foo", newLucene<StringReader>(L"これは本ではない")),
		newCollection<String>(L"これ", L"は", L"本", L"で", L"は", L"ない"), newCollection<int32_t>(0, 2, 3, 4, 5, 6),
		newCollection<int32_t>(2, 3, 4, 5, 6, 8), 8);

	checkTokenStreamContents(
		analyzerNoPunct->tokenStream(L"foo", newLucene<StringReader>(L"これは本ではない    ")),
		newCollection<String>(L"これ", L"は", L"本", L"で", L"は", L"ない"), newCollection<int32_t>(0, 2, 3, 4, 5, 6, 8),
		newCollection<int32_t>(2, 3, 4, 5, 6, 8, 9), 12);
}

TEST_F(TestJapaneseTokenizer, testUserDict)
{
	// Not a great test because w/o userdict.txt the
	// segmentation is the same:
	checkTokenStreamContents(
		analyzer->tokenStream(L"foo", newLucene<StringReader>(L"関西国際空港に行った")),
		newCollection<String>(L"関西", L"国際", L"空港", L"に", L"行っ", L"た"), newCollection<int32_t>(0, 2, 4, 6, 7, 9),
		newCollection<int32_t>(2, 4, 6, 7, 9, 10), 10);
}

TEST_F(TestJapaneseTokenizer, testUserDict2)
{
	// Better test: w/o userdict the segmentation is different:
	checkTokenStreamContents(
		analyzer->tokenStream(L"foo", newLucene<StringReader>(L"朝青龍")), newCollection<String>(L"朝青龍"), newCollection<int32_t>(0),
		newCollection<int32_t>(3), 3);
}

TEST_F(TestJapaneseTokenizer, testUserDict3)
{
	// Test entry that breaks into multiple tokens:
	checkTokenStreamContents(
		analyzer->tokenStream(L"foo", newLucene<StringReader>(L"abcd")), newCollection<String>(L"a", L"b", L"cd"),
		newCollection<int32_t>(0, 1, 2), newCollection<int32_t>(1, 2, 4), 4);
}

TEST_F(TestJapaneseTokenizer, testSegmentation)
{
	// Skip tests for Michelle Kwan -- UniDic segments Kwan as ク ワン
	//		String input = "ミシェル・クワンが優勝しました。スペースステーションに行きます。うたがわしい。";
	//		String[] surfaceForms = {
	//				"ミシェル", "・", "クワン", "が", "優勝", "し", "まし", "た", "。",
	//				"スペース", "ステーション", "に", "行き", "ます", "。",
	//				"うたがわしい", "。"
	//		};
	String input = L"スペースステーションに行きます。うたがわしい。";
	Collection<String> surfaceForms =
		newCollection<String>(L"スペース", L"ステーション", L"に", L"行き", L"ます", L"。", L"うたがわしい", L"。");
	checkAnalyzesTo(analyzer, input, surfaceForms);
}

TEST_F(TestJapaneseTokenizer, testLatticeToDot)
{
	String input = L"スペースステーションに行きます。うたがわしい。";
	Collection<String> surfaceForms =
		newCollection<String>(L"スペース", L"ステーション", L"に", L"行き", L"ます", L"。", L"うたがわしい", L"。");
	checkAnalyzesTo(analyzerFormatter, input, surfaceForms);

	EXPECT_TRUE(analyzerFormatter->gv2->finish().find(L"22.0") != std::wstring::npos);
}

TEST_F(TestJapaneseTokenizer, testReadings)
{
	assertReadings(L"寿司が食べたいです。", {L"スシ", L"ガ", L"タベ", L"タイ", L"デス", L"。"});
}

TEST_F(TestJapaneseTokenizer, testReadings2)
{
	assertReadings(L"多くの学生が試験に落ちた。", {L"オオク", L"ノ", L"ガクセイ", L"ガ", L"シケン", L"ニ", L"オチ", L"タ", L"。"});
}

TEST_F(TestJapaneseTokenizer, testPronunciations)
{
	assertPronunciations(L"寿司が食べたいです。", {L"スシ", L"ガ", L"タベ", L"タイ", L"デス", L"。"});
}

TEST_F(TestJapaneseTokenizer, testPronunciations2)
{
	// pronunciation differs from reading here
	assertPronunciations(L"多くの学生が試験に落ちた。", {L"オーク", L"ノ", L"ガクセイ", L"ガ", L"シケン", L"ニ", L"オチ", L"タ", L"。"});
}

TEST_F(TestJapaneseTokenizer, testBasicForms)
{
	assertBaseForms(L"それはまだ実験段階にあります。", {L"", L"", L"", L"", L"", L"", L"ある", L"", L""});
}

TEST_F(TestJapaneseTokenizer, testInflectionTypes)
{
	assertInflectionTypes(L"それはまだ実験段階にあります。", {L"", L"", L"", L"", L"", L"", L"五段・ラ行", L"特殊・マス", L""});
}

TEST_F(TestJapaneseTokenizer, testInflectionForms)
{
	assertInflectionForms(L"それはまだ実験段階にあります。", {L"", L"", L"", L"", L"", L"", L"連用形", L"基本形", L""});
}

TEST_F(TestJapaneseTokenizer, testPartOfSpeech)
{
	assertPartsOfSpeech(
		L"それはまだ実験段階にあります。", {L"名詞-代名詞-一般", L"助詞-係助詞", L"副詞-助詞類接続", L"名詞-サ変接続", L"名詞-一般",
											L"助詞-格助詞-一般", L"動詞-自立", L"助動詞", L"記号-句点"});
}

TEST_F(TestJapaneseTokenizer, testYabottai)
{
	checkAnalyzesTo(analyzer, L"やぼったい", newCollection<String>(L"やぼったい"));
}

TEST_F(TestJapaneseTokenizer, testTsukitosha)
{
	checkAnalyzesTo(analyzer, L"突き通しゃ", newCollection<String>(L"突き通しゃ"));
}

// This test takes around 3 seconds on Release mode. But around 30 seconds on Debug mode
// The test file has 104408 Japanese characters
TEST_F(TestJapaneseTokenizer, testBocchan)
{
	doTestBocchan(1);
}

// This test is a bit time consuming so disabled by default
TEST_F(TestJapaneseTokenizer, DISABLED_testBocchanBig)
{
	doTestBocchan(3);
}
