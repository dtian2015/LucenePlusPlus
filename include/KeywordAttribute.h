#pragma once

#include "Attribute.h"

namespace Lucene {

/// <summary>
/// This attribute can be used to mark a token as a keyword. Keyword aware
/// <seealso cref="TokenStream"/>s can decide to modify a token based on the return value
/// of <seealso cref="#isKeyword()"/> if the token is modified. Stemming filters for
/// instance can use this attribute to conditionally skip a term if
/// <seealso cref="#isKeyword()"/> returns <code>true</code>.
/// </summary>
class LPPAPI KeywordAttribute : public Attribute
{
public:
	/// <summary>
	/// Returns <code>true</code> iff the current token is a keyword, otherwise
	/// <code>false</code>/
	/// </summary>
	/// <returns> <code>true</code> iff the current token is a keyword, otherwise
	///         <code>false</code>/ </returns>
	virtual bool isKeyword();

	/// <summary>
	/// Marks the current token as keyword iff set to <code>true</code>.
	/// </summary>
	/// <param name="isKeyword">
	///          <code>true</code> iff the current token is a keyword, otherwise
	///          <code>false</code>. </param>
	virtual void setKeyword(bool isKeyword);

	virtual void clear();
	virtual bool equals(const LuceneObjectPtr& other);
	virtual int32_t hashCode();
	virtual void copyTo(const AttributePtr& target);

	virtual LuceneObjectPtr clone(const LuceneObjectPtr& other = LuceneObjectPtr());

private:
	bool _keyword = false;
};
}
