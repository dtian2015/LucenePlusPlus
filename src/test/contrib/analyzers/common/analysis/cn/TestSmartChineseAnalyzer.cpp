#include "TestInc.h"

#include "BaseTokenStreamFixture.h"
#include "KeywordTokenizer.h"
#include "StringReader.h"
#include "smartcn/SmartChineseAnalyzer.h"

using namespace Lucene;
using namespace Lucene::Analysis::Cn::Smart;

class TestSmartChineseAnalyzer : public BaseTokenStreamFixture
{
public:
	static const LuceneVersion::Version TEST_VERSION_CURRENT;
};

const LuceneVersion::Version TestSmartChineseAnalyzer::TEST_VERSION_CURRENT = LuceneVersion::Version::LUCENE_30;

TEST_F(TestSmartChineseAnalyzer, testChineseStopWordsDefault)
{
	AnalyzerPtr ca = newLucene<SmartChineseAnalyzer>(TEST_VERSION_CURRENT); // will load stopwords
	String sentence = L"我购买了道具和服装。";
	Collection<String> result = newCollection<String>(L"我", L"购买", L"了", L"道具", L"和", L"服装");

	checkAnalyzesTo(ca, sentence, result);

	// set stop-words from the outer world - must yield same behavior
	ca = newLucene<SmartChineseAnalyzer>(TEST_VERSION_CURRENT, SmartChineseAnalyzer::getDefaultStopSet());
	checkAnalyzesTo(ca, sentence, result);
}

TEST_F(TestSmartChineseAnalyzer, testChineseCustomStopWords)
{
	HashSet<String> customStopWords = HashSet<String>::newInstance();
	customStopWords.add(L"我");

	AnalyzerPtr ca = newLucene<SmartChineseAnalyzer>(TEST_VERSION_CURRENT, customStopWords, true);
	String sentence = L"我购买了道具和服装。";
	Collection<String> result = newCollection<String>(L"购买", L"了", L"道具", L"和", L"服装");

	checkAnalyzesTo(ca, sentence, result);
}

/*
 * This test is the same as the above, except with two phrases.
 * This tests to ensure the SentenceTokenizer->WordTokenFilter chain works correctly.
 */
TEST_F(TestSmartChineseAnalyzer, testChineseStopWordsDefaultTwoPhrases)
{
	AnalyzerPtr ca = newLucene<SmartChineseAnalyzer>(TEST_VERSION_CURRENT); // will load stopwords
	String sentence = L"我购买了道具和服装。 我购买了道具和服装。";
	std::vector<String> output = {L"我", L"购买", L"了", L"道具", L"和", L"服装", L"我", L"购买", L"了", L"道具", L"和", L"服装"};
	Collection<String> result = Collection<String>::newInstance(output.begin(), output.end());

	checkAnalyzesTo(ca, sentence, result);
}

/*
 * This test is the same as the above, except using an ideographic space as a separator.
 * This tests to ensure the stopwords are working correctly.
 */
TEST_F(TestSmartChineseAnalyzer, testChineseStopWordsDefaultTwoPhrasesIdeoSpace)
{
	AnalyzerPtr ca = newLucene<SmartChineseAnalyzer>(TEST_VERSION_CURRENT); // will load stopwords
	std::wstring sentence = L"我购买了道具和服装　我购买了道具和服装。";
	std::vector<String> output = {L"我", L"购买", L"了", L"道具", L"和", L"服装", L"我", L"购买", L"了", L"道具", L"和", L"服装"};
	Collection<String> result = Collection<String>::newInstance(output.begin(), output.end());

	checkAnalyzesTo(ca, sentence, result);
}

/*
 * Punctuation is handled in a strange way if you disable stopwords
 * In this example the IDEOGRAPHIC FULL STOP is converted into a comma.
 * if you don't supply (true) to the constructor, or use a different stopwords list,
 * then punctuation is indexed.
 */
TEST_F(TestSmartChineseAnalyzer, testChineseStopWordsOff)
{
	std::vector<AnalyzerPtr> analyzers = {newLucene<SmartChineseAnalyzer>(TEST_VERSION_CURRENT, false),
										  newLucene<SmartChineseAnalyzer>(TEST_VERSION_CURRENT, HashSet<String>::newInstance())};
	String sentence = L"我购买了道具和服装。";
	Collection<String> result = newCollection<String>(L"我", L"购买", L"了", L"道具", L"和", L"服装", L",");

	for (auto analyzer : analyzers)
	{
		checkAnalyzesTo(analyzer, sentence, result);
		checkAnalyzesTo(analyzer, sentence, result);
	}
}

/*
 * Check that position increments after stopwords are correct,
 * when stopfilter is configured with enablePositionIncrements
 */
TEST_F(TestSmartChineseAnalyzer, testChineseStopWords2)
{
	AnalyzerPtr ca = newLucene<SmartChineseAnalyzer>(TEST_VERSION_CURRENT); // will load stopwords
	String sentence = L"Title:San"; // : is a stopword
	Collection<String> result = newCollection<String>(L"title", L"san");
	Collection<int32_t> startOffsets = newCollection<int32_t>(0, 6);
	Collection<int32_t> endOffsets = newCollection<int32_t>(5, 9);
	Collection<int32_t> posIncr = newCollection<int32_t>(1, 2);

	checkAnalyzesTo(ca, sentence, result, startOffsets, endOffsets, posIncr);
}

TEST_F(TestSmartChineseAnalyzer, testChineseAnalyzer)
{
	AnalyzerPtr ca = newLucene<SmartChineseAnalyzer>(TEST_VERSION_CURRENT, true);
	String sentence = L"我购买了道具和服装。";
	Collection<String> result = newCollection<String>(L"我", L"购买", L"了", L"道具", L"和", L"服装");

	checkAnalyzesTo(ca, sentence, result);
}

/*
 * English words are lowercased.
 */
TEST_F(TestSmartChineseAnalyzer, testMixedLatinChinese)
{
	checkAnalyzesTo(
		newLucene<SmartChineseAnalyzer>(TEST_VERSION_CURRENT, true), L"我购买 Tests 了道具和服装",
		newCollection<String>(L"我", L"购买", L"tests", L"了", L"道具", L"和", L"服装"));
}

/*
 * Numerics are parsed as their own tokens
 */
TEST_F(TestSmartChineseAnalyzer, testNumerics)
{
	checkAnalyzesTo(
		newLucene<SmartChineseAnalyzer>(TEST_VERSION_CURRENT, true), L"我购买 Tests 了道具和服装1234",
		newCollection<String>(L"我", L"购买", L"tests", L"了", L"道具", L"和", L"服装", L"1234"));
}

/*
 * Full width alphas and numerics are folded to half-width
 */
TEST_F(TestSmartChineseAnalyzer, testFullWidth)
{
	checkAnalyzesTo(
		newLucene<SmartChineseAnalyzer>(TEST_VERSION_CURRENT, true), L"我购买 Ｔｅｓｔｓ 了道具和服装１２３４",
		newCollection<String>(L"我", L"购买", L"tests", L"了", L"道具", L"和", L"服装", L"1234"));
}

/*
 * Presentation form delimiters are removed
 */
TEST_F(TestSmartChineseAnalyzer, testDelimiters)
{
	checkAnalyzesTo(
		newLucene<SmartChineseAnalyzer>(TEST_VERSION_CURRENT, true), L"我购买︱ Tests 了道具和服装",
		newCollection<String>(L"我", L"购买", L"tests", L"了", L"道具", L"和", L"服装"));
}

/*
 * Text from writing systems other than Chinese and Latin are parsed as individual characters.
 * (regardless of Unicode category)
 */
TEST_F(TestSmartChineseAnalyzer, testNonChinese)
{
	std::vector<String> expectedOutput = {L"我", L"购买", L"ر", L"و", L"ب", L"ر", L"ت", L"tests", L"了", L"道具", L"和", L"服装"};

	checkAnalyzesTo(
		newLucene<SmartChineseAnalyzer>(TEST_VERSION_CURRENT, true), L"我购买 روبرتTests 了道具和服装",
		Collection<String>::newInstance(expectedOutput.begin(), expectedOutput.end()));
}

/*
 * Test what the analyzer does with out-of-vocabulary words.
 * In this case the name is Yousaf Raza Gillani.
 * Currently it is being analyzed into single characters...
 */
TEST_F(TestSmartChineseAnalyzer, testOOV)
{
	checkAnalyzesTo(
		newLucene<SmartChineseAnalyzer>(TEST_VERSION_CURRENT, true), L"优素福·拉扎·吉拉尼",
		newCollection<String>(L"优", L"素", L"福", L"拉", L"扎", L"吉", L"拉", L"尼"));

	checkAnalyzesTo(
		newLucene<SmartChineseAnalyzer>(TEST_VERSION_CURRENT, true), L"优素福拉扎吉拉尼",
		newCollection<String>(L"优", L"素", L"福", L"拉", L"扎", L"吉", L"拉", L"尼"));
}

TEST_F(TestSmartChineseAnalyzer, testOffsets)
{
	checkAnalyzesTo(
		newLucene<SmartChineseAnalyzer>(TEST_VERSION_CURRENT, true), L"我购买了道具和服装",
		newCollection<String>(L"我", L"购买", L"了", L"道具", L"和", L"服装"), newCollection<int32_t>(0, 1, 3, 4, 6, 7),
		newCollection<int32_t>(1, 3, 4, 6, 7, 9));
}

TEST_F(TestSmartChineseAnalyzer, testReusableTokenStream)
{
	AnalyzerPtr a = newLucene<SmartChineseAnalyzer>(TEST_VERSION_CURRENT);

	checkAnalyzesToReuse(
		a, L"我购买 Tests 了道具和服装", newCollection<String>(L"我", L"购买", L"tests", L"了", L"道具", L"和", L"服装"),
		newCollection<int32_t>(0, 1, 4, 10, 11, 13, 14), newCollection<int32_t>(1, 3, 9, 11, 13, 14, 16));

	checkAnalyzesToReuse(
		a, L"我购买了道具和服装。", newCollection<String>(L"我", L"购买", L"了", L"道具", L"和", L"服装"),
		newCollection<int32_t>(0, 1, 3, 4, 6, 7), newCollection<int32_t>(1, 3, 4, 6, 7, 9));
}

TEST_F(TestSmartChineseAnalyzer, testLargeDocument)
{
	StringStream sb;
	for (int i = 0; i < 5000; i++)
	{
		sb << L"我购买了道具和服装。";
	}

	AnalyzerPtr analyzer = newLucene<SmartChineseAnalyzer>(TEST_VERSION_CURRENT);
	TokenStreamPtr stream = analyzer->reusableTokenStream(L"", newLucene<StringReader>(sb.str()));
	stream->reset();

	while (stream->incrementToken())
	{
	}
}

TEST_F(TestSmartChineseAnalyzer, testLargeSentence)
{
	StringStream sb;
	for (int i = 0; i < 5000; i++)
	{
		sb << L"我购买了道具和服装";
	}

	AnalyzerPtr analyzer = newLucene<SmartChineseAnalyzer>(TEST_VERSION_CURRENT);
	TokenStreamPtr stream = analyzer->reusableTokenStream(L"", newLucene<StringReader>(sb.str()));
	stream->reset();

	while (stream->incrementToken())
	{
	}
}

// TODO: MockTokenizer
// TEST_F(TestSmartChineseAnalyzer, testInvalidOffset)
//{
//	std::shared_ptr<Analyzer> analyzer = std::make_shared<ReusableAnalyzerBaseAnonymousInnerClass>(shared_from_this());
//
//	assertAnalyzesTo(analyzer, L"mosfellsbær", std::vector<std::wstring>{L"mosfellsbaer"}, std::vector<int>{0}, std::vector<int>{11});
//}

// TestSmartChineseAnalyzer::ReusableAnalyzerBaseAnonymousInnerClass::ReusableAnalyzerBaseAnonymousInnerClass(
//	std::shared_ptr<TestSmartChineseAnalyzer> outerInstance)
//{
//	this->outerInstance = outerInstance;
//}
//
// std::shared_ptr<ReusableAnalyzerBase::TokenStreamComponents> TestSmartChineseAnalyzer::ReusableAnalyzerBaseAnonymousInnerClass::
//	createComponents(const std::wstring& fieldName, std::shared_ptr<Reader> reader)
//{
//	std::shared_ptr<Tokenizer> tokenizer = std::make_shared<MockTokenizer>(reader, MockTokenizer::WHITESPACE, false);
//	std::shared_ptr<TokenFilter> filters = std::make_shared<ASCIIFoldingFilter>(tokenizer);
//	filters = std::make_shared<WordTokenFilter>(filters);
//	return std::make_shared<ReusableAnalyzerBase::TokenStreamComponents>(tokenizer, filters);
//}

// TEST_F(TestSmartChineseAnalyzer, testEmptyTerm)
//{
//	std::shared_ptr<Analyzer> a = std::make_shared<ReusableAnalyzerBaseAnonymousInnerClass2>(shared_from_this());
//	checkAnalysisConsistency(random, a, random->nextBoolean(), L"");
//}

// TestSmartChineseAnalyzer::ReusableAnalyzerBaseAnonymousInnerClass2::ReusableAnalyzerBaseAnonymousInnerClass2(
//	std::shared_ptr<TestSmartChineseAnalyzer> outerInstance)
//{
//	this->outerInstance = outerInstance;
//}
//
// std::shared_ptr<ReusableAnalyzerBase::TokenStreamComponents> TestSmartChineseAnalyzer::ReusableAnalyzerBaseAnonymousInnerClass2::
//	createComponents(const std::wstring& fieldName, std::shared_ptr<Reader> reader)
//{
//	std::shared_ptr<Tokenizer> tokenizer = std::make_shared<KeywordTokenizer>(reader);
//	return std::make_shared<ReusableAnalyzerBase::TokenStreamComponents>(tokenizer, std::make_shared<WordTokenFilter>(tokenizer));
//}
