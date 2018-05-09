#pragma once

#include "DataInput.h"
#include "DataOutput.h"

namespace Lucene {
namespace Util {
namespace FST {

bool operator==(const Long& lhs, const Long& rhs);

/// <summary>
/// Represents the outputs for an FST, providing the basic
/// algebra required for building and traversing the FST.
///
/// <para>Note that any operation that returns NO_OUTPUT must
/// return the same singleton object from {@link
/// #getNoOutput}.</para>
///
/// @lucene.experimental
/// </summary>

template <typename T>
class LPPAPI Outputs : public LuceneObject
{
public:
	LUCENE_CLASS(Outputs<T>);

	// TODO: maybe change this API to allow for re-use of the
	// output instances -- this is an insane amount of garbage
	// (new object per byte/char/int) if eg used during
	// analysis

	/// <summary>
	/// Eg common("foo", "foobar") -> "foo" </summary>
	virtual T common(const T& output1, const T& output2) = 0;

	/// <summary>
	/// Eg subtract("foobar", "foo") -> "bar" </summary>
	virtual T subtract(const T& output, const T& inc) = 0;

	/// <summary>
	/// Eg add("foo", "bar") -> "foobar" </summary>
	virtual T add(const T& prefix, const T& output) = 0;

	virtual void write(T output, boost::shared_ptr<DataOutput> out) = 0;

	virtual T read(DataInputPtr input) = 0;

	/// <summary>
	/// NOTE: this output is compared with == so you must
	///  ensure that all methods return the single object if
	///  it's really no output
	/// </summary>
	virtual T getNoOutput() = 0;

	virtual String outputToString(T output) = 0;

	// TODO: maybe make valid(T output) public...?  for asserts

	virtual T merge(T first, T second) { throw boost::make_shared<UnsupportedOperationException>(); }
};

inline bool operator==(const Long& lhs, const Long& rhs)
{
	if (lhs == nullptr && rhs == nullptr)
	{
		return true;
	}

	if (lhs && rhs)
	{
		return *lhs == *rhs;
	}

	return false;
}
}
}
}
