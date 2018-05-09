#include "ContribInc.h"

#include "smartcn/hhmm/PathNode.h"

#include "MiscUtils.h"

namespace Lucene {
namespace Analysis {
namespace Cn {
namespace Smart {
namespace Hhmm {

int32_t PathNode::compareTo(const LuceneObjectPtr& other)
{
	if (this->equals(other))
	{
		return 0;
	}

	PathNodePtr otherPathNode(boost::dynamic_pointer_cast<PathNode>(other));

	if (otherPathNode == nullptr)
	{
		return 1;
	}

	if (weight < otherPathNode->weight)
	{
		return -1;
	}
	else if (weight == otherPathNode->weight)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}

int PathNode::hashCode()
{
	constexpr int prime = 31;
	int result = 1;
	result = prime * result + preNode;

	int64_t temp;
	temp = MiscUtils::doubleToLongBits(weight);
	result = prime * result + static_cast<int>(temp ^ (static_cast<int64_t>(static_cast<uint64_t>(temp) >> 32)));

	return result;
}

bool PathNode::equals(const LuceneObjectPtr& other)
{
	if (LuceneObject::equals(other))
	{
		return true;
	}

	PathNodePtr otherPathNode(boost::dynamic_pointer_cast<PathNode>(other));

	if (otherPathNode == nullptr)
	{
		return false;
	}

	if (preNode != otherPathNode->preNode)
	{
		return false;
	}

	if (MiscUtils::doubleToLongBits(weight) != MiscUtils::doubleToLongBits(otherPathNode->weight))
	{
		return false;
	}

	return true;
}
}
}
}
}
}
