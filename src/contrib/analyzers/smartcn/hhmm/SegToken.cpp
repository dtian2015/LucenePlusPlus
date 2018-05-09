#include "ContribInc.h"

#include "smartcn/hhmm/SegToken.h"

namespace Lucene {
namespace Analysis {
namespace Cn {
namespace Smart {
namespace Hhmm {

SegToken::SegToken(const CharArray& idArray, int start, int end, WordType wordType, int weight)
	: charArray(idArray), startOffset(start), endOffset(end), wordType(wordType), weight(weight)
{
}

int32_t SegToken::hashCode()
{
	constexpr int prime = 31;
	int result = 1;

	for (int i = 0; i < charArray.size(); i++)
	{
		result = prime * result + charArray[i];
	}

	result = prime * result + endOffset;
	result = prime * result + index;
	result = prime * result + startOffset;
	result = prime * result + weight;
	result = prime * result + static_cast<int>(wordType);
	return result;
}

bool SegToken::equals(const LuceneObjectPtr& other)
{
	if (LuceneObject::equals(other))
	{
		return true;
	}

	SegTokenPtr otherSegToken(boost::dynamic_pointer_cast<SegToken>(other));

	if (otherSegToken == nullptr)
	{
		return false;
	}

	if (!(charArray == otherSegToken->charArray))
	{
		return false;
	}

	if (endOffset != otherSegToken->endOffset)
	{
		return false;
	}

	if (index != otherSegToken->index)
	{
		return false;
	}

	if (startOffset != otherSegToken->startOffset)
	{
		return false;
	}

	if (weight != otherSegToken->weight)
	{
		return false;
	}

	if (wordType != otherSegToken->wordType)
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
