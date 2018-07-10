#pragma once

#include "LuceneObject.h"

namespace Lucene {
namespace Util {

/// <summary>
/// Represents int[], as a slice (offset + length) into an
///  existing int[].
///
///  @lucene.internal
/// </summary>
class LPPAPI IntsRef final : public LuceneObject
{
public:
	LUCENE_CLASS(IntsRef);

	std::vector<int> ints;
	int offset = 0;
	int length = 0;

	IntsRef();

	IntsRef(int capacity);

	IntsRef(const std::vector<int>& ints, int offset, int length);

	virtual int32_t hashCode();

	virtual bool equals(const LuceneObjectPtr& other);

	bool intsEquals(const IntsRefPtr& other);

	void copyInts(const IntsRefPtr& other);

	void grow(int newLength);

	String toString();

	/// <summary>
	/// Creates a new IntsRef that points to a copy of the ints from
	/// <code>other</code>
	/// <para>
	/// The returned IntsRef will have a length of other.length
	/// and an offset of zero.
	/// </para>
	/// </summary>
	static IntsRefPtr deepCopyOf(const IntsRefPtr& other);

	virtual int32_t compareTo(const LuceneObjectPtr& other);

	virtual int compareTo(IntsRefPtr other);
};
}
}
