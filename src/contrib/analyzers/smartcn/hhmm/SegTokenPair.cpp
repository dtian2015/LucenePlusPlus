#include "ContribInc.h"

#include "smartcn/hhmm/SegTokenPair.h"

#include "MiscUtils.h"
#include "smartcn/CnTypes.h"

namespace Lucene {
namespace Analysis {
namespace Cn {
namespace Smart {
namespace Hhmm {

SegTokenPair::SegTokenPair(const CharArray& idArray, int from, int to, double weight)
	: charArray(idArray), from(from), to(to), weight(weight)
{
}

int32_t SegTokenPair::hashCode()
{
	constexpr int prime = 31;
	int result = 1;

	for (int i = 0; i < charArray.size(); i++)
	{
		result = prime * result + charArray[i];
	}

	result = prime * result + from;
	result = prime * result + to;

	int64_t temp = MiscUtils::doubleToLongBits(weight);
	result = prime * result + static_cast<int>(temp ^ (static_cast<int64_t>(static_cast<uint64_t>(temp) >> 32)));
	return result;
}

bool SegTokenPair::equals(const LuceneObjectPtr& other)
{
	if (LuceneObject::equals(other))
	{
		return true;
	}

	SegTokenPairPtr otherTokenPair(boost::dynamic_pointer_cast<SegTokenPair>(other));

	if (otherTokenPair == nullptr)
	{
		return false;
	}

	if (!(charArray == otherTokenPair->charArray))
	{
		return false;
	}

	if (from != otherTokenPair->from)
	{
		return false;
	}

	if (to != otherTokenPair->to)
	{
		return false;
	}

	if (MiscUtils::doubleToLongBits(weight) != MiscUtils::doubleToLongBits(otherTokenPair->weight))
	{
		return false;
	}

	return true;
}

std::wostream& operator<<(std::wostream& os, const SegTokenPair& item)
{
	return os << L"{charArray =" << String(item.charArray.get(), item.charArray.size()) << L", from = " << std::to_wstring(item.from)
			  << L", to = " << std::to_wstring(item.to) << L", weight = " << std::to_wstring(item.weight) << L"}";
}
}
}
}
}
}
