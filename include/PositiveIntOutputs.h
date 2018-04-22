#pragma once

#include "Outputs.h"

namespace Lucene {
namespace Util {
namespace FST {

bool operator>(const Long& lhs, int value);
bool operator>=(const Long& lhs, const Long& rhs);
Long operator-(const Long& lhs, const Long& rhs);
Long operator+(const Long& lhs, const Long& rhs);

/// <summary>
/// An FST <seealso cref="Outputs"/> implementation where each output
/// is a non-negative long value.
///
/// @lucene.experimental
/// </summary>
class LPPAPI PositiveIntOutputs final : public Outputs<Long>
{
private:
	const bool _doShare;

	PositiveIntOutputs(bool doShare);

public:
	LUCENE_CLASS(PositiveIntOutputs);

	static const Long NO_OUTPUT;
	static const PositiveIntOutputsPtr& getSingleton(bool doShare);

	Long common(const Long& output1, const Long& output2);

	Long subtract(const Long& output, const Long& inc);

	Long add(const Long& prefix, const Long& output);

	void write(Long output, boost::shared_ptr<DataOutput> out);

	Long read(DataInputPtr input);

private:
	bool valid(const Long& o);

public:
	Long getNoOutput();

	String outputToString(Long output);

	String toString();
};
}
}
}
