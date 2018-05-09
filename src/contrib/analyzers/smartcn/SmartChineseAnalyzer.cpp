#include "ContribInc.h"

#include "smartcn/SmartChineseAnalyzer.h"

#include "smartcn/SentenceTokenizer.h"
#include "smartcn/WordTokenFilter.h"

#include "StopFilter.h"
#include "StringUtils.h"

namespace Lucene {
namespace Analysis {
namespace Cn {
namespace Smart {

DECLARE_SHARED_PTR(SmartChineseAnalyzerSavedStreams)

class SmartChineseAnalyzerSavedStreams final : public LuceneObject
{
public:
	LUCENE_CLASS(SmartChineseAnalyzerSavedStreams);

	TokenizerPtr tokenStream;
	TokenStreamPtr filteredTokenStream;
};

const uint8_t SmartChineseAnalyzer::DEFAULT_STOPWORD_FILE[] = {
	0x2c, 0x0a, 0x2e, 0x0a, 0x60, 0x0a, 0x2d, 0x0a, 0x5f, 0x0a, 0x3d, 0x0a, 0x3f, 0x0a, 0x27, 0x0a, 0x7c, 0x0a, 0x22, 0x0a, 0x28, 0x0a,
	0x29, 0x0a, 0x7b, 0x0a, 0x7d, 0x0a, 0x5b, 0x0a, 0x5d, 0x0a, 0x3c, 0x0a, 0x3e, 0x0a, 0x2a, 0x0a, 0x23, 0x0a, 0x26, 0x0a, 0x5e, 0x0a,
	0x24, 0x0a, 0x40, 0x0a, 0x21, 0x0a, 0x7e, 0x0a, 0x3a, 0x0a, 0x3b, 0x0a, 0x2b, 0x0a, 0x2f, 0x0a, 0x5c, 0x0a, 0xe3, 0x80, 0x8a, 0x0a,
	0xe3, 0x80, 0x8b, 0x0a, 0xe2, 0x80, 0x94, 0x0a, 0xef, 0xbc, 0x8d, 0x0a, 0xef, 0xbc, 0x8c, 0x0a, 0xe3, 0x80, 0x82, 0x0a, 0xe3, 0x80,
	0x81, 0x0a, 0xef, 0xbc, 0x9a, 0x0a, 0xef, 0xbc, 0x9b, 0x0a, 0xef, 0xbc, 0x81, 0x0a, 0xc2, 0xb7, 0x0a, 0xef, 0xbc, 0x9f, 0x0a, 0xe2,
	0x80, 0x9c, 0x0a, 0xe2, 0x80, 0x9d, 0x0a, 0xef, 0xbc, 0x89, 0x0a, 0xef, 0xbc, 0x88, 0x0a, 0xe3, 0x80, 0x90, 0x0a, 0xe3, 0x80, 0x91,
	0x0a, 0xef, 0xbc, 0xbb, 0x0a, 0xef, 0xbc, 0xbd, 0x0a, 0xe2, 0x97, 0x8f, 0x0a, 0xe3, 0x80, 0x80};

HashSet<String> SmartChineseAnalyzer::DefaultSetHolder::DEFAULT_STOP_SET;

SmartChineseAnalyzer::SmartChineseAnalyzer(LuceneVersion::Version matchVersion) : SmartChineseAnalyzer(matchVersion, true)
{
}

SmartChineseAnalyzer::SmartChineseAnalyzer(LuceneVersion::Version matchVersion, bool useDefaultStopWords)
	: _stopWords(useDefaultStopWords ? DefaultSetHolder::DEFAULT_STOP_SET : HashSet<String>::newInstance()), matchVersion(matchVersion)
{
}

SmartChineseAnalyzer::SmartChineseAnalyzer(
	LuceneVersion::Version matchVersion, const HashSet<String>& stopWords, bool includeDefaultStopWords)
	: _stopWords(stopWords), matchVersion(matchVersion)
{
	if (includeDefaultStopWords)
	{
		const auto& defaultStopWords = DefaultSetHolder::DEFAULT_STOP_SET;
		_stopWords.addAll(defaultStopWords.begin(), defaultStopWords.end());
	}
}

HashSet<String> SmartChineseAnalyzer::getDefaultStopSet()
{
	return DefaultSetHolder::DEFAULT_STOP_SET;
}

TokenStreamPtr SmartChineseAnalyzer::tokenStream(const String& fieldName, const ReaderPtr& reader)
{
	TokenStreamPtr result = newLucene<SentenceTokenizer>(reader);
	result = newLucene<WordTokenFilter>(result);

	// result = new LowerCaseFilter(result);
	// LowerCaseFilter is not needed, as SegTokenFilter lowercases Basic Latin text.
	// The porter stemming is too strict, this is not a bug, this is a feature:)

	// Converstion note: We don't use porter stemmer, to be consistent with Lucene.Net
	// result = newLucene<PorterStemFilter>(result);

	if (!_stopWords.empty())
	{
		result = newLucene<StopFilter>(StopFilter::getEnablePositionIncrementsVersionDefault(matchVersion), result, _stopWords);
	}
	return result;
}

TokenStreamPtr SmartChineseAnalyzer::reusableTokenStream(const String& fieldName, const ReaderPtr& reader)
{
	SmartChineseAnalyzerSavedStreamsPtr streams(boost::dynamic_pointer_cast<SmartChineseAnalyzerSavedStreams>(getPreviousTokenStream()));

	if (!streams)
	{
		streams = newLucene<SmartChineseAnalyzerSavedStreams>();
		setPreviousTokenStream(streams);

		streams->tokenStream = newLucene<SentenceTokenizer>(reader);
		streams->filteredTokenStream = newLucene<WordTokenFilter>(streams->tokenStream);

		// Conversion note: We don't use porter stemmer, to be consistent with Lucene.Net
		// streams->filteredTokenStream = newLucene<PorterStemFilter>(streams->filteredTokenStream);

		if (!_stopWords.empty())
		{
			streams->filteredTokenStream = newLucene<StopFilter>(
				StopFilter::getEnablePositionIncrementsVersionDefault(matchVersion), streams->filteredTokenStream, _stopWords);
		}
	}
	else
	{
		streams->tokenStream->reset(reader);
		streams->filteredTokenStream->reset(); // reset WordTokenFilter's state
	}

	return streams->filteredTokenStream;
}

SmartChineseAnalyzer::DefaultSetHolder::StaticConstructor SmartChineseAnalyzer::DefaultSetHolder::staticConstructor;

SmartChineseAnalyzer::DefaultSetHolder::StaticConstructor::StaticConstructor()
{
	try
	{
		if (!DEFAULT_STOP_SET)
		{
			String stopWords(UTF8_TO_STRING(DEFAULT_STOPWORD_FILE));
			Collection<String> words(StringUtils::split(stopWords, L"\n"));
			DEFAULT_STOP_SET = HashSet<String>::newInstance(words.begin(), words.end());
		}
	}
	catch (const IOException& ex)
	{
		boost::throw_exception(RuntimeException(L"Unable to load default stopwords"));
	}
}
}
}
}
}
