#include "LuceneInc.h"

#include "IntsRef.h"

namespace Lucene {
namespace Util {

IntsRef::IntsRef()
{
}

IntsRef::IntsRef(int capacity)
{
	ints = std::vector<int>(capacity);
}

IntsRef::IntsRef(const std::vector<int>& ints, int offset, int length)
{
	BOOST_ASSERT(ints.size() > 0);
	this->ints = ints;
	this->offset = offset;
	this->length = length;
}

int IntsRef::hashCode()
{
	constexpr int prime = 31;
	int result = 0;
	const int end = offset + length;

	for (int i = offset; i < end; i++)
	{
		result = prime * result + ints[i];
	}

	return result;
}

bool IntsRef::equals(const LuceneObjectPtr& other)
{
	if (LuceneObject::equals(other))
	{
		return true;
	}

	IntsRefPtr otherIntsRef(boost::dynamic_pointer_cast<IntsRef>(other));

	if (otherIntsRef)
	{
		return this->intsEquals(otherIntsRef);
	}

	return false;
}

bool IntsRef::intsEquals(const IntsRefPtr& other)
{
	if (length == other->length)
	{
		int otherUpto = other->offset;
		const std::vector<int> otherInts = other->ints;
		const int end = offset + length;

		for (int upto = offset; upto < end; upto++, otherUpto++)
		{
			if (ints[upto] != otherInts[otherUpto])
			{
				return false;
			}
		}

		return true;
	}
	else
	{
		return false;
	}
}

void IntsRef::copyInts(const IntsRefPtr& other)
{
	ints = other->ints;
	length = other->length;
	offset = 0;
}

IntsRefPtr IntsRef::deepCopyOf(const IntsRefPtr& other)
{
	IntsRefPtr clone = Lucene::newInstance<IntsRef>();
	clone->copyInts(other);
	return clone;
}

void IntsRef::grow(int newLength)
{
	ints.resize(newLength);
}

String IntsRef::toString()
{
	StringStream sb;
	sb << L'[';
	const int end = offset + length;

	for (int i = offset; i < end; i++)
	{
		if (i > offset)
		{
			sb << L' ';
		}

		sb << std::hex << ints[i];
	}

	sb << L']';

	return sb.str();
}

int32_t IntsRef::compareTo(const LuceneObjectPtr& other)
{
	if (this->equals(other))
	{
		return 0;
	}

	IntsRefPtr otherIntsRef(boost::dynamic_pointer_cast<IntsRef>(other));

	if (otherIntsRef)
	{
		return this->compareTo(otherIntsRef);
	}

	return 1;
}

int IntsRef::compareTo(IntsRefPtr other)
{
	int aUpto = offset;
	int bUpto = other->offset;

	const int aStop = aUpto + std::min(length, other->length);

	while (aUpto < aStop)
	{
		int aInt = ints[aUpto++];
		int bInt = other->ints[bUpto++];
		if (aInt > bInt)
		{
			return 1;
		}
		else if (aInt < bInt)
		{
			return -1;
		}
	}

	// One is a prefix of the other, or, they are equal:
	return length - other->length;
}
}
}
