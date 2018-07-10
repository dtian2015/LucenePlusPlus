#pragma once

#include "ReusableAnalyzerBase.h"

namespace Lucene {

/// <summary>
/// Base class for Analyzers that need to make use of stopword sets.
/// </summary>
class LPPAPI StopwordAnalyzerBase : public ReusableAnalyzerBase
{
protected:
	/// <summary>
	/// An immutable stopword set
	/// </summary>
	const HashSet<String> stopwords;

	const LuceneVersion::Version matchVersion;

public:
	LUCENE_CLASS(StopwordAnalyzerBase);

	/// <summary>
	/// Returns the analyzer's stopword set or an empty set if the analyzer has no
	/// stopwords
	/// </summary>
	/// <returns> the analyzer's stopword set or an empty set if the analyzer has no
	///         stopwords </returns>
	virtual HashSet<String> getStopwordSet();

protected:
	/// <summary>
	/// Creates a new instance initialized with the given stopword set
	/// </summary>
	/// <param name="version">
	///          the Lucene version for cross version compatibility </param>
	/// <param name="stopwords">
	///          the analyzer's stopword set </param>
	StopwordAnalyzerBase(LuceneVersion::Version version, const HashSet<String>& stopwords);

	/// <summary>
	/// Creates a new Analyzer with an empty stopword set
	/// </summary>
	/// <param name="version">
	///          the Lucene version for cross version compatibility </param>
	StopwordAnalyzerBase(LuceneVersion::Version version);

	/// <summary>
	/// Creates a CharArraySet from a file resource associated with a class. (See
	/// <seealso cref="Class#getResourceAsStream(String)"/>).
	/// </summary>
	/// <param name="stopwordsFile">the file containing stop words/param>
	/// <param name="comment">
	///          comment string to ignore in the stopword file </param>
	/// <returns> a HashSet containing the distinct stopwords from the given
	///         file </returns>
	static HashSet<String> loadStopwordSet(const String& stopwordsFile, const String& comment);

	/// <summary>
	/// Creates a CharArraySet from a file.
	/// </summary>
	/// <param name="stopwords">
	///          the stopwords reader to load
	/// </param>
	/// <param name="comment">
	///          comment string to ignore in the stopword file </param>
	/// <returns> a HashSet containing the distinct stopwords from the given
	///         reader </returns>
	static HashSet<String> loadStopwordSet(ReaderPtr stopwords, const String& comment);
};
}
