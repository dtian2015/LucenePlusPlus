#pragma once

#include "LuceneObject.h"
#include "smartcn/CnTypes.h"

namespace Lucene {
namespace Analysis {
namespace Cn {
namespace Smart {
namespace Hhmm {

/// <summary>
/// SmartChineseAnalyzer internal node representation
/// <para>
/// Used by <seealso cref="BiSegGraph"/> to maximize the segmentation with the Viterbi algorithm.
/// </para>
/// @lucene.experimental
/// </summary>
class LPPCONTRIBAPI PathNode : public LuceneObject
{
public:
	LUCENE_CLASS(PathNode);

	double weight = 0;

	int preNode = 0;

	virtual int32_t compareTo(const LuceneObjectPtr& other);

	virtual int32_t hashCode();

	virtual bool equals(const LuceneObjectPtr& other);
};
}
}
}
}
}
