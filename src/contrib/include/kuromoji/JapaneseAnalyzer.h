#pragma once

#include "StopwordAnalyzerBase.h"
#include "kuromoji/JapaneseTokenizer.h"

namespace Lucene {
namespace Analysis {
namespace Ja {

/// <summary>
/// Analyzer for Japanese that uses morphological analysis. </summary>
/// <seealso cref= JapaneseTokenizer </seealso>
class LPPCONTRIBAPI JapaneseAnalyzer : public StopwordAnalyzerBase
{
private:
	const JapaneseTokenizer::Mode _mode;
	const HashSet<String> _stoptags;
	const Dict::UserDictionaryPtr _userDict;

public:
	LUCENE_CLASS(JapaneseAnalyzer);

	JapaneseAnalyzer(LuceneVersion::Version matchVersion);

	JapaneseAnalyzer(
		LuceneVersion::Version matchVersion,
		Dict::UserDictionaryPtr userDict,
		JapaneseTokenizer::Mode mode,
		const HashSet<String>& stopwords,
		const HashSet<String>& stoptags);

	static HashSet<String> getDefaultStopSet();

	static HashSet<String> getDefaultStopTags();

	/// <summary>
	/// Atomically loads DEFAULT_STOP_SET, DEFAULT_STOP_TAGS in a lazy fashion once the
	/// outer class accesses the static final set the first time.
	/// </summary>
private:
	static const uint8_t DEFAULT_STOPWORD_FILE[];
	static const uint8_t DEFAULT_STOPTAGS_FILE[];

	class DefaultSetHolder
	{
	public:
		static HashSet<String> DEFAULT_STOP_SET;
		static HashSet<String> DEFAULT_STOP_TAGS;

	private:
		class StaticConstructor
		{
		public:
			StaticConstructor();
		};

	private:
		static DefaultSetHolder::StaticConstructor staticConstructor;
	};

protected:
	TokenStreamComponentsPtr createComponents(const String& fieldName, ReaderPtr reader);
};
}
}
}
