#pragma once

#include "Attribute.h"

namespace Lucene {

/// <summary>
/// The positionLength determines how many positions this
///  token spans.  Very few analyzer components actually
///  produce this attribute, and indexing ignores it, but
///  it's useful to express the graph structure naturally
///  produced by decompounding, word splitting/joining,
///  synonym filtering, etc.
///
/// <para>The default value is one.
/// </para>
/// </summary>
class LPPAPI PositionLengthAttribute : public Attribute
{
public:
	LUCENE_CLASS(PositionLengthAttribute);

	/// <param name="positionLength"> how many positions this token
	///  spans.  </param>
	virtual void setPositionLength(int positionLength);

	/// <summary>
	/// Returns the position length of this Token. </summary>
	/// <seealso cref= #setPositionLength </seealso>
	virtual int getPositionLength();

	virtual void clear();
	virtual bool equals(const LuceneObjectPtr& other);
	virtual int32_t hashCode();
	virtual void copyTo(const AttributePtr& target);

	virtual LuceneObjectPtr clone(const LuceneObjectPtr& other = LuceneObjectPtr());

private:
	int _positionLength = 1;
};
}
