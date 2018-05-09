#pragma once

#include "Analyzer.h"

namespace Lucene {
namespace Analysis {
namespace Cn {
namespace Smart {

/// <summary>
/// <para>
/// SmartChineseAnalyzer is an analyzer for Chinese or mixed Chinese-English text.
/// The analyzer uses probabilistic knowledge to find the optimal word segmentation for Simplified Chinese text.
/// The text is first broken into sentences, then each sentence is segmented into words.
/// </para>
/// <para>
/// Segmentation is based upon the <a href="http://en.wikipedia.org/wiki/Hidden_Markov_Model">Hidden Markov Model</a>.
/// A large training corpus was used to calculate Chinese word frequency probability.
/// </para>
/// <para>
/// This analyzer requires a dictionary to provide statistical data.
/// SmartChineseAnalyzer has an included dictionary out-of-box.
/// </para>
/// <para>
/// The included dictionary data is from <a href="http://www.ictclas.org">ICTCLAS1.0</a>.
/// Thanks to ICTCLAS for their hard work, and for contributing the data under the Apache 2 License!
/// </para>
/// @lucene.experimental
/// </summary>
class LPPCONTRIBAPI SmartChineseAnalyzer final : public Analyzer
{
private:
	HashSet<String> _stopWords;

	static const uint8_t DEFAULT_STOPWORD_FILE[];

public:
	LUCENE_CLASS(SmartChineseAnalyzer);

	/// <summary>
	/// Returns an unmodifiable instance of the default stop-words set. </summary>
	/// <returns> an unmodifiable instance of the default stop-words set. </returns>
	static HashSet<String> getDefaultStopSet();

	/// <summary>
	/// Atomically loads the DEFAULT_STOP_SET in a lazy fashion once the outer class
	/// accesses the static final set the first time.;
	/// </summary>
private:
	class DefaultSetHolder
	{
	public:
		static HashSet<String> DEFAULT_STOP_SET;

	private:
		class StaticConstructor
		{
		public:
			StaticConstructor();
		};

	private:
		static DefaultSetHolder::StaticConstructor staticConstructor;
	};

private:
	const LuceneVersion::Version matchVersion;

	/// <summary>
	/// Create a new SmartChineseAnalyzer, using the default stopword list.
	/// </summary>
public:
	SmartChineseAnalyzer(LuceneVersion::Version matchVersion);

	/// <summary>
	/// <para>
	/// Create a new SmartChineseAnalyzer, optionally using the default stopword list.
	/// </para>
	/// <para>
	/// The included default stopword list is simply a list of punctuation.
	/// If you do not use this list, punctuation will not be removed from the text!
	/// </para>
	/// </summary>
	/// <param name="useDefaultStopWords"> true to use the default stopword list. </param>
	SmartChineseAnalyzer(LuceneVersion::Version matchVersion, bool useDefaultStopWords);

	/// <summary>
	/// <para>
	/// Create a new SmartChineseAnalyzer, using the provided <seealso cref="Set"/> of stopwords.
	/// </para>
	/// <para>
	/// Note: the set should include punctuation, unless you want to index punctuation!
	/// </para> </summary>
	/// <param name="stopWords"> <seealso cref="Set"/> of stopwords to use. </param>
	SmartChineseAnalyzer(LuceneVersion::Version matchVersion, const HashSet<String>& stopWords, bool includeDefaultStopWords = false);

	virtual TokenStreamPtr tokenStream(const String& fieldName, const ReaderPtr& reader);

public:
	virtual TokenStreamPtr reusableTokenStream(const String& fieldName, const ReaderPtr& reader);
};
}
}
}
}
