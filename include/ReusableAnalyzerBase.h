#pragma once

#include "Analyzer.h"

namespace Lucene {

/// <summary>
/// An convenience subclass of Analyzer that makes it easy to implement
/// <seealso cref="TokenStream"/> reuse.
/// <para>
/// ReusableAnalyzerBase is a simplification of Analyzer that supports easy reuse
/// for the most common use-cases. Analyzers such as
/// <seealso cref="PerFieldAnalyzerWrapper"/> that behave differently depending upon the
/// field name need to subclass Analyzer directly instead.
/// </para>
/// <para>
/// To prevent consistency problems, this class does not allow subclasses to
/// extend <seealso cref="#reusableTokenStream(String, Reader)"/> or
/// <seealso cref="#tokenStream(String, Reader)"/> directly. Instead, subclasses must
/// implement <seealso cref="#createComponents(String, Reader)"/>.
/// </para>
/// </summary>
class LPPAPI ReusableAnalyzerBase : public Analyzer
{
public:
	DECLARE_SHARED_PTR(TokenStreamComponents)

	virtual ~ReusableAnalyzerBase();
	LUCENE_CLASS(ReusableAnalyzerBase);

protected:
	/// <summary>
	/// Creates a new <seealso cref="TokenStreamComponents"/> instance for this analyzer.
	/// </summary>
	/// <param name="fieldName">
	///          the name of the fields content passed to the
	///          <seealso cref="TokenStreamComponents"/> sink as a reader </param>
	/// <param name="reader">
	///          the reader passed to the <seealso cref="Tokenizer"/> constructor </param>
	/// <returns> the <seealso cref="TokenStreamComponents"/> for this analyzer. </returns>
	virtual TokenStreamComponentsPtr createComponents(const String& fieldName, ReaderPtr reader) = 0;

public:
	/// <summary>
	/// This method uses <seealso cref="#createComponents(String, Reader)"/> to obtain an
	/// instance of <seealso cref="TokenStreamComponents"/>. It returns the sink of the
	/// components and stores the components internally. Subsequent calls to this
	/// method will reuse the previously stored components if and only if the
	/// <seealso cref="TokenStreamComponents#reset(Reader)"/> method returned
	/// <code>true</code>. Otherwise a new instance of
	/// <seealso cref="TokenStreamComponents"/> is created.
	/// </summary>
	/// <param name="fieldName"> the name of the field the created TokenStream is used for </param>
	/// <param name="reader"> the reader the streams source reads from </param>
	virtual TokenStreamPtr reusableTokenStream(const String& fieldName, const ReaderPtr& reader);

	/// <summary>
	/// This method uses <seealso cref="#createComponents(String, Reader)"/> to obtain an
	/// instance of <seealso cref="TokenStreamComponents"/> and returns the sink of the
	/// components. Each calls to this method will create a new instance of
	/// <seealso cref="TokenStreamComponents"/>. Created <seealso cref="TokenStream"/> instances are
	/// never reused.
	/// </summary>
	/// <param name="fieldName"> the name of the field the created TokenStream is used for </param>
	/// <param name="reader"> the reader the streams source reads from </param>
	virtual TokenStreamPtr tokenStream(const String& fieldName, const ReaderPtr& reader);

	/// <summary>
	/// Override this if you want to add a CharFilter chain.
	/// </summary>
protected:
	virtual ReaderPtr initReader(ReaderPtr reader);

	/// <summary>
	/// This class encapsulates the outer components of a token stream. It provides
	/// access to the source (<seealso cref="Tokenizer"/>) and the outer end (sink), an
	/// instance of <seealso cref="TokenFilter"/> which also serves as the
	/// <seealso cref="TokenStream"/> returned by
	/// <seealso cref="Analyzer#tokenStream(String, Reader)"/> and
	/// <seealso cref="Analyzer#reusableTokenStream(String, Reader)"/>.
	/// </summary>
public:
	class TokenStreamComponents : public LuceneObject
	{
	protected:
		const TokenizerPtr source;
		const TokenStreamPtr sink;

	public:
		/// <summary>
		/// Creates a new <seealso cref="TokenStreamComponents"/> instance.
		/// </summary>
		/// <param name="source">the analyzer's tokenizer </param>
		/// <param name="result">the analyzer's resulting token stream </param>
		TokenStreamComponents(TokenizerPtr source, TokenStreamPtr result);

		/// <summary>
		/// Creates a new <seealso cref="TokenStreamComponents"/> instance.
		/// </summary>
		/// <param name="source">the analyzer's tokenizer </param>
		TokenStreamComponents(TokenizerPtr source);

	protected:
		friend class ReusableAnalyzerBase;

		/// <summary>
		/// Resets the encapsulated components with the given reader. This method by
		/// default returns <code>true</code> indicating that the components have
		/// been reset successfully. Subclasses of <seealso cref="ReusableAnalyzerBase"/> might use
		/// their own <seealso cref="TokenStreamComponents"/> returning <code>false</code> if
		/// the components cannot be reset.
		/// </summary>
		/// <param name="reader">
		///          a reader to reset the source component </param>
		/// <returns> <code>true</code> if the components were reset, otherwise
		///         <code>false</code> </returns>
		virtual bool reset(ReaderPtr reader);

		/// <summary>
		/// Returns the sink <seealso cref="TokenStream"/>
		/// </summary>
		/// <returns> the sink <seealso cref="TokenStream"/> </returns>
		virtual TokenStreamPtr getTokenStream();
	};
};
}
