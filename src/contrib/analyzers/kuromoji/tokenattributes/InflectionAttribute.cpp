#include "ContribInc.h"

#include "kuromoji/tokenattributes/InflectionAttribute.h"

namespace Lucene {
namespace Analysis {
namespace Ja {
namespace TokenAttributes {

String InflectionAttribute::getInflectionType()
{
	return _token == nullptr ? L"" : _token->getInflectionType();
}

String InflectionAttribute::getInflectionForm()
{
	return _token == nullptr ? L"" : _token->getInflectionForm();
}

void InflectionAttribute::setToken(Ja::TokenPtr token)
{
	this->_token = token;
}

void InflectionAttribute::clear()
{
	_token.reset();
}

void InflectionAttribute::copyTo(const AttributePtr& target)
{
	InflectionAttributePtr t = boost::static_pointer_cast<InflectionAttribute>(target);
	t->setToken(_token);
}

bool InflectionAttribute::equals(const LuceneObjectPtr& other)
{
	if (Attribute::equals(other))
	{
		return true;
	}

	InflectionAttributePtr otherInflectionAttribute(boost::dynamic_pointer_cast<InflectionAttribute>(other));
	if (otherInflectionAttribute)
	{
		return toString() == otherInflectionAttribute->toString();
	}

	return false;
}

int32_t InflectionAttribute::hashCode()
{
	const auto& attributeValue = toString();
	if (attributeValue.empty())
	{
		return 0;
	}

	return MiscUtils::hashCode(attributeValue.data(), 0, attributeValue.size());
}

LuceneObjectPtr InflectionAttribute::clone(const LuceneObjectPtr& other)
{
	LuceneObjectPtr clone = Attribute::clone(other ? other : newLucene<InflectionAttribute>());

	InflectionAttributePtr cloneAttribute(boost::dynamic_pointer_cast<InflectionAttribute>(clone));
	cloneAttribute->setToken(_token);

	return clone;
}

String InflectionAttribute::toString()
{
	if (_token)
	{
		return _token->toString() + L", InflectionType= " + _token->getInflectionType() + L", InflectionForm=" +
			_token->getInflectionForm();
	}

	return L"";
}
}
}
}
}
