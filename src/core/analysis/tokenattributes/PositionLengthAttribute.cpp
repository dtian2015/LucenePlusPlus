#include "PositionLengthAttribute.h"

namespace Lucene {

void PositionLengthAttribute::setPositionLength(int positionLength)
{
	if (positionLength < 1)
	{
		boost::throw_exception(IllegalArgumentException(L"Position length must be 1 or greater: got " + std::to_wstring(positionLength)));
	}

	this->_positionLength = positionLength;
}

int PositionLengthAttribute::getPositionLength()
{
	return _positionLength;
}

void PositionLengthAttribute::clear()
{
	this->_positionLength = 1;
}

bool PositionLengthAttribute::equals(const LuceneObjectPtr& other)
{
	if (Attribute::equals(other))
	{
		return true;
	}

	PositionLengthAttributePtr otherPositionLengthAttribute(boost::dynamic_pointer_cast<PositionLengthAttribute>(other));
	if (otherPositionLengthAttribute)
	{
		return _positionLength == otherPositionLengthAttribute->_positionLength;
	}

	return false;
}

int32_t PositionLengthAttribute::hashCode()
{
	return _positionLength;
}

void PositionLengthAttribute::copyTo(const AttributePtr& target)
{
	PositionLengthAttributePtr targetPositionLengthAttribute(boost::dynamic_pointer_cast<PositionLengthAttribute>(target));
	targetPositionLengthAttribute->setPositionLength(_positionLength);
}

LuceneObjectPtr PositionLengthAttribute::clone(const LuceneObjectPtr& other)
{
	LuceneObjectPtr clone = other ? other : newLucene<PositionLengthAttribute>();
	PositionLengthAttributePtr cloneAttribute(boost::dynamic_pointer_cast<PositionLengthAttribute>(Attribute::clone(clone)));
	cloneAttribute->_positionLength = _positionLength;
	return cloneAttribute;
}
}
