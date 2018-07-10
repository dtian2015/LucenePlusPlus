#include "TestInc.h"

#include "BaseTokenStreamFixture.h"
#include "StringReader.h"
#include "kuromoji/JapaneseAnalyzer.h"

using namespace Lucene;
using namespace Lucene::Analysis::Ja;

class TestJapaneseAnalyzer : public BaseTokenStreamFixture
{
public:
	static const LuceneVersion::Version TEST_VERSION_CURRENT;
};

const LuceneVersion::Version TestJapaneseAnalyzer::TEST_VERSION_CURRENT = LuceneVersion::Version::LUCENE_30;

TEST_F(TestJapaneseAnalyzer, testResourcesAvailable)
{
	JapaneseAnalyzerPtr analyzer = newLucene<JapaneseAnalyzer>(TEST_VERSION_CURRENT);
	EXPECT_TRUE(analyzer);
}

TEST_F(TestJapaneseAnalyzer, testBasics)
{
	checkAnalyzesTo(
		newLucene<JapaneseAnalyzer>(TEST_VERSION_CURRENT), L"多くの学生が試験に落ちた。",
		newCollection<String>(L"多く", L"学生", L"試験", L"落ちる"), newCollection<int32_t>(0, 3, 6, 9),
		newCollection<int32_t>(2, 5, 8, 11), newCollection<int32_t>(1, 2, 2, 2));
}

TEST_F(TestJapaneseAnalyzer, testBasicsWithoutStopWordsKeepingPunctuation)
{
	checkAnalyzesTo(
		newLucene<JapaneseAnalyzer>(TEST_VERSION_CURRENT, HashSet<String>::newInstance(), false), L"多くの学生が試験に落ちた。",
		newCollection<String>(L"多く", L"の", L"学生", L"が", L"試験", L"に", L"落ちる", L"た", L"。"));
}

TEST_F(TestJapaneseAnalyzer, testWeirdJapaneseNumberBaseFormMap)
{
	JapaneseAnalyzerPtr analyzer = newLucene<JapaneseAnalyzer>(TEST_VERSION_CURRENT);

	checkAnalyzesTo(analyzer, L"４４Ｋ", newCollection<String>(L"4", L"4", L"k"));

	const auto& map = analyzer->getBaseformMap();
	ASSERT_EQ(2, map.size());

	const auto& entry4 = map.find(L"4");
	ASSERT_TRUE(entry4 != map.end());
	EXPECT_TRUE(entry4->second.count(L"４") == 1);

	const auto& entryk = map.find(L"K");
	ASSERT_TRUE(entryk != map.end());
	EXPECT_TRUE(entryk->second.count(L"Ｋ") == 1);
}

TEST_F(TestJapaneseAnalyzer, testDecomposition)
{
	AnalyzerPtr const a = newLucene<JapaneseAnalyzer>(
		TEST_VERSION_CURRENT, nullptr, JapaneseTokenizer::Mode::SEARCH, JapaneseAnalyzer::getDefaultStopSet(),
		JapaneseAnalyzer::getDefaultStopTags());

	// Senior software engineer:
	checkAnalyzesToPositions(
		a, L"シニアソフトウェアエンジニア",
		newCollection<String>(L"シニア", L"シニアソフトウェアエンジニア", L"ソフトウェア", L"エンジニア"),
		newCollection<int32_t>(1, 0, 1, 1), newCollection<int32_t>(1, 3, 1, 1)); // zero pos inc

	// Senior project manager: also tests katakana spelling variation stemming
	checkAnalyzesToPositions(
		a, L"シニアプロジェクトマネージャー",
		newCollection<String>(L"シニア", L"シニアプロジェクトマネージャー", L"プロジェクト", L"マネージャー"),
		newCollection<int32_t>(1, 0, 1, 1), newCollection<int32_t>(1, 3, 1, 1));

	// Kansai International Airport:
	checkAnalyzesToPositions(
		a, L"関西国際空港", newCollection<String>(L"関西", L"関西国際空港", L"国際", L"空港"), newCollection<int32_t>(1, 0, 1, 1),
		newCollection<int32_t>(1, 3, 1, 1)); // zero pos inc

	// Konika Minolta Holdings; not quite the right
	// segmentation (see LUCENE-3726):
	checkAnalyzesToPositions(
		a, L"コニカミノルタホールディングス",
		newCollection<String>(L"コニカ", L"コニカミノルタホールディングス", L"ミノルタ", L"ホールディングス"),
		newCollection<int32_t>(1, 0, 1, 1), newCollection<int32_t>(1, 3, 1, 1)); // zero pos inc

	// Narita Airport
	checkAnalyzesToPositions(
		a, L"成田空港", newCollection<String>(L"成田", L"成田空港", L"空港"), newCollection<int32_t>(1, 0, 1),
		newCollection<int32_t>(1, 2, 1));

	// Kyoto University Baseball Club
	checkAnalyzesToPositions(
		newLucene<JapaneseAnalyzer>(TEST_VERSION_CURRENT), L"京都大学硬式野球部",
		newCollection<String>(L"京都大", L"学", L"硬式", L"野球", L"部"), newCollection<int32_t>(1, 1, 1, 1, 1),
		newCollection<int32_t>(1, 1, 1, 1, 1));
	// toDotFile(a, "成田空港", "/mnt/scratch/out.dot");
}

TEST_F(TestJapaneseAnalyzer, testUserDict3)
{
	// Test entry that breaks into multiple tokens:
	AnalyzerPtr const a = newLucene<JapaneseAnalyzer>(
		TEST_VERSION_CURRENT, readDict(), JapaneseTokenizer::Mode::SEARCH, JapaneseAnalyzer::getDefaultStopSet(),
		JapaneseAnalyzer::getDefaultStopTags());
	checkTokenStreamContents(
		a->tokenStream(L"foo", newLucene<StringReader>(L"abcd")), newCollection<String>(L"a", L"b", L"cd"), newCollection<int32_t>(0, 1, 2),
		newCollection<int32_t>(1, 2, 4), 4);
}
