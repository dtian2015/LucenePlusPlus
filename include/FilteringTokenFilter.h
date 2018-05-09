#pragma once

#include "TokenFilter.h"

namespace Lucene {

/// <summary>
/// Abstract base class for TokenFilters that may remove tokens.
/// You have to implement <seealso cref="#accept"/> and return a boolean if the current
/// token should be preserved. <seealso cref="#incrementToken"/> uses this method
/// to decide if a token should be passed to the caller.
/// </summary>
class LPPAPI FilteringTokenFilter : public TokenFilter
{
private:
	const PositionIncrementAttributePtr _posIncrAtt;
	bool _enablePositionIncrements = false; // no init needed, as ctor enforces setting value!
	bool _first = true; // only used when not preserving gaps

public:
	LUCENE_CLASS(FilteringTokenFilter);

	FilteringTokenFilter(bool enablePositionIncrements, TokenStreamPtr input);

protected:
	/// <summary>
	/// Override this method and return if the current input token should be returned by <seealso cref="#incrementToken"/>.
	/// </summary>
	virtual bool accept() = 0;

public:
	virtual bool incrementToken();

	virtual void reset();

	/// <seealso cref= #setEnablePositionIncrements(boolean) </seealso>
	virtual bool getEnablePositionIncrements();

	/// <summary>
	/// If <code>true</code>, this TokenFilter will preserve
	/// positions of the incoming tokens (ie, accumulate and
	/// set position increments of the removed tokens).
	/// Generally, <code>true</code> is best as it does not
	/// lose information (positions of the original tokens)
	/// during indexing.
	///
	/// <para> When set, when a token is stopped
	/// (omitted), the position increment of the following
	/// token is incremented.
	///
	/// </para>
	/// <para> <b>NOTE</b>: be sure to also
	/// set <seealso cref="QueryParser#setEnablePositionIncrements"/> if
	/// you use QueryParser to create queries.
	/// </para>
	/// </summary>
	virtual void setEnablePositionIncrements(bool enable);
};
}
