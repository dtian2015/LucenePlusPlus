#include "KeywordAttribute.h"

namespace Lucene {

bool KeywordAttribute::isKeyword()
{
	return _keyword;
}

void KeywordAttribute::setKeyword(bool isKeyword)
{
	_keyword = isKeyword;
}

void KeywordAttribute::clear()
{
	_keyword = false;
}

void KeywordAttribute::copyTo(const AttributePtr& target)
{
	KeywordAttributePtr attr(boost::dynamic_pointer_cast<KeywordAttribute>(target));
	attr->setKeyword(_keyword);
}

int KeywordAttribute::hashCode()
{
	return _keyword ? 31 : 37;
}

bool KeywordAttribute::equals(const LuceneObjectPtr& other)
{
	if (Attribute::equals(other))
	{
		return true;
	}

	KeywordAttributePtr otherKeywordAttribute(boost::dynamic_pointer_cast<KeywordAttribute>(other));

	if (otherKeywordAttribute)
	{
		return _keyword == otherKeywordAttribute->_keyword;
	}

	return false;
}

LuceneObjectPtr KeywordAttribute::clone(const LuceneObjectPtr& other)
{
	LuceneObjectPtr clone = other ? other : newLucene<KeywordAttribute>();
	KeywordAttributePtr cloneAttribute(boost::dynamic_pointer_cast<KeywordAttribute>(Attribute::clone(clone)));

	cloneAttribute->_keyword = _keyword;
	return cloneAttribute;
}
}
