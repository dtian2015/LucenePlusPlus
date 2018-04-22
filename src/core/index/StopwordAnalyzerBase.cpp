#include "LuceneInc.h"

#include "StopwordAnalyzerBase.h"
#include "WordlistLoader.h"

namespace Lucene {

HashSet<String> StopwordAnalyzerBase::getStopwordSet()
{
	return stopwords;
}

StopwordAnalyzerBase::StopwordAnalyzerBase(LuceneVersion::Version version, const HashSet<String>& stopwords)
	: stopwords(stopwords), matchVersion(version)
{
	// analyzers should use char array set for stopwords!
}

StopwordAnalyzerBase::StopwordAnalyzerBase(LuceneVersion::Version version) : StopwordAnalyzerBase(version, HashSet<String>())
{
}

HashSet<String> StopwordAnalyzerBase::loadStopwordSet(const String& stopwordsFile, const String& comment)
{
	return WordlistLoader::getWordSet(stopwordsFile, comment);
}

HashSet<String> StopwordAnalyzerBase::loadStopwordSet(ReaderPtr stopwords, const String& comment)
{
	return WordlistLoader::getWordSet(stopwords, comment);
}
}
