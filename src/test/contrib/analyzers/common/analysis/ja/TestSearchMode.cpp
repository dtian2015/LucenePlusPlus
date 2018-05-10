#include "TestInc.h"

#include "BaseTokenStreamFixture.h"
#include "BufferedReader.h"
#include "FileReader.h"
#include "FileUtils.h"
#include "InputStreamReader.h"
#include "ReusableAnalyzerBase.h"
#include "StringUtils.h"
#include "TestUtils.h"

#include "kuromoji/JapaneseTokenizer.h"

#include <regex>

#include <boost/algorithm/string.hpp>

using namespace Lucene;
using namespace Lucene::Analysis::Ja;

class TestSearchMode : public BaseTokenStreamFixture
{
public:
	DECLARE_SHARED_PTR(ReusableAnalyzerSearch);
	class ReusableAnalyzerSearch : public ReusableAnalyzerBase
	{
	public:
		LUCENE_CLASS(ReusableAnalyzerSearch);

	protected:
		ReusableAnalyzerBase::TokenStreamComponentsPtr createComponents(const String& fieldName, ReaderPtr reader)
		{
			TokenizerPtr tokenizer = newLucene<JapaneseTokenizer>(reader, nullptr, true, JapaneseTokenizer::Mode::SEARCH);
			return newLucene<TokenStreamComponents>(tokenizer, tokenizer);
		}
	};

	AnalyzerPtr analyzer = newLucene<ReusableAnalyzerSearch>();
};

TEST_F(TestSearchMode, testSearchSegmentation)
{
	const String SEGMENTATION_FILENAME = FileUtils::joinPath(getTestDir(), L"ja/search-segmentation-tests.txt");

	if (!FileUtils::fileExists(SEGMENTATION_FILENAME))
	{
		boost::throw_exception(RuntimeException(L"Cannot find " + SEGMENTATION_FILENAME + L" in test classpath"));
	}

	BufferedReaderPtr reader = newLucene<BufferedReader>(newLucene<InputStreamReader>(newLucene<FileReader>(SEGMENTATION_FILENAME)));

	String line;
	std::wregex replacePattern(L"#.*$");
	while (reader->readLine(line))
	{
		// Remove comments
		line = std::regex_replace(line, replacePattern, L"");

		// Skip empty lines or comment lines
		boost::trim(line);
		if (line.empty())
		{
			continue;
		}

		Collection<String> fields = StringUtils::split(line, L"\t");
		String sourceText = fields[0];
		Collection<String> expectedTokens = StringUtils::regexSplit(fields[1], L"\\s+");
		Collection<int32_t> expectedPosIncrs = Collection<int32_t>::newInstance(expectedTokens.size());
		Collection<int32_t> expectedPosLengths = Collection<int32_t>::newInstance(expectedTokens.size());

		for (int tokIDX = 0; tokIDX < expectedTokens.size(); tokIDX++)
		{
			if (boost::ends_with(expectedTokens[tokIDX], L"/0"))
			{
				expectedTokens[tokIDX] = boost::replace_all_copy(expectedTokens[tokIDX], L"/0", L"");
				expectedPosLengths[tokIDX] = expectedTokens.size() - 1;
			}
			else
			{
				expectedPosIncrs[tokIDX] = 1;
				expectedPosLengths[tokIDX] = 1;
			}
		}

		checkAnalyzesTo(analyzer, sourceText, expectedTokens, expectedPosIncrs);
	}
}
