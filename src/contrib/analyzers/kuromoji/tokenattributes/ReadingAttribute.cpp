#include "ContribInc.h"

#include "kuromoji/tokenattributes/ReadingAttribute.h"

namespace Lucene {
namespace Analysis {
namespace Ja {
namespace TokenAttributes {

String ReadingAttribute::getReading()
{
	return _token == nullptr ? L"" : _token->getReading();
}

String ReadingAttribute::getPronunciation()
{
	return _token == nullptr ? L"" : _token->getPronunciation();
}

void ReadingAttribute::setToken(Ja::TokenPtr token)
{
	this->_token = token;
}

void ReadingAttribute::clear()
{
	_token.reset();
}

void ReadingAttribute::copyTo(const AttributePtr& target)
{
	ReadingAttributePtr t = boost::static_pointer_cast<ReadingAttribute>(target);
	t->setToken(_token);
}

bool ReadingAttribute::equals(const LuceneObjectPtr& other)
{
	if (Attribute::equals(other))
	{
		return true;
	}

	ReadingAttributePtr otherReadingAttribute(boost::dynamic_pointer_cast<ReadingAttribute>(other));
	if (otherReadingAttribute)
	{
		return toString() == otherReadingAttribute->toString();
	}

	return false;
}

int32_t ReadingAttribute::hashCode()
{
	const auto& attributeValue = toString();
	if (attributeValue.empty())
	{
		return 0;
	}

	return MiscUtils::hashCode(attributeValue.data(), 0, attributeValue.size());
}

LuceneObjectPtr ReadingAttribute::clone(const LuceneObjectPtr& other)
{
	LuceneObjectPtr clone = Attribute::clone(other ? other : newLucene<ReadingAttribute>());

	ReadingAttributePtr cloneAttribute(boost::dynamic_pointer_cast<ReadingAttribute>(clone));
	cloneAttribute->setToken(_token);

	return clone;
}

String ReadingAttribute::toString()
{
	if (_token)
	{
		return _token->toString() + L", Reading= " + _token->getReading() + L", Pronunciation=" + _token->getPronunciation();
	}

	return L"";
}
}
}
}
}
