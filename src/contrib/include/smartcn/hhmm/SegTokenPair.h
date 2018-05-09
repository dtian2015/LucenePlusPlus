#pragma once

#include "LuceneObject.h"

#include <memory>
#include <vector>

namespace Lucene {
namespace Analysis {
namespace Cn {
namespace Smart {
namespace Hhmm {

/// <summary>
/// A pair of tokens in <seealso cref="SegGraph"/>
/// @lucene.experimental
/// </summary>
class LPPCONTRIBAPI SegTokenPair : public LuceneObject
{
public:
	LUCENE_CLASS(SegTokenPair);

	CharArray charArray;

	/// <summary>
	/// index of the first token in <seealso cref="SegGraph"/>
	/// </summary>
	int from = 0;

	/// <summary>
	/// index of the second token in <seealso cref="SegGraph"/>
	/// </summary>
	int to = 0;

	double weight = 0;

	SegTokenPair(const CharArray& idArray, int from, int to, double weight);

	virtual int32_t hashCode();

	virtual bool equals(const LuceneObjectPtr& other);
};

std::wostream& operator<<(std::wostream& os, const SegTokenPair& item);
}
}
}
}
}
