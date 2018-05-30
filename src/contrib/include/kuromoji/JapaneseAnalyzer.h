#pragma once

#include "StopwordAnalyzerBase.h"
#include "kuromoji/JapaneseTokenizer.h"

#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/unordered_set.hpp>

namespace Lucene {
namespace Analysis {
namespace Ja {

typedef std::unordered_map<String, std::unordered_set<String>> BaseformToWordsMap;

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
	static const String BASE_FORM_MAP_FIELD_NAME;
	static const String NODE_ID_FIELD_NAME;

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

	virtual void cleanup();

	virtual boost::optional<std::pair<String, ByteArray>> getExtraStoredFieldInfo();

	// When the indexed doc is a Node, we do not need to store the baseform to words map
	// as that would already be stored in corresponding indexed source. This is to avoid unnecessary
	// index file increase
	virtual String getIgnoredDocField() const { return NODE_ID_FIELD_NAME; }
	void AddBaseformWord(const String& baseform, const String& termText);

	const BaseformToWordsMap& getBaseformMap() const { return _baseformToWordsMap; }
private:
	/// <summary>
	/// Atomically loads DEFAULT_STOP_SET, DEFAULT_STOP_TAGS in a lazy fashion once the
	/// outer class accesses the static final set the first time.
	/// </summary>
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

	BaseformToWordsMap _baseformToWordsMap;

protected:
	TokenStreamComponentsPtr createComponents(const String& fieldName, ReaderPtr reader);
};
}
}
}
