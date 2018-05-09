#include "ContribInc.h"

#include "kuromoji/tokenattributes/BaseFormAttribute.h"

namespace Lucene {
namespace Analysis {
namespace Ja {
namespace TokenAttributes {

String BaseFormAttribute::getBaseForm()
{
	return _token == nullptr ? L"" : _token->getBaseForm();
}

void BaseFormAttribute::setToken(Ja::TokenPtr token)
{
	this->_token = token;
}

void BaseFormAttribute::clear()
{
	_token.reset();
}

void BaseFormAttribute::copyTo(const AttributePtr& target)
{
	BaseFormAttributePtr t = boost::static_pointer_cast<BaseFormAttribute>(target);
	t->setToken(_token);
}

bool BaseFormAttribute::equals(const LuceneObjectPtr& other)
{
	if (Attribute::equals(other))
	{
		return true;
	}

	BaseFormAttributePtr otherBaseFormAttribute(boost::dynamic_pointer_cast<BaseFormAttribute>(other));
	if (otherBaseFormAttribute)
	{
		String tokeValue = _token ? _token->toString() : L"";
		String otherTokenValue = otherBaseFormAttribute->_token ? otherBaseFormAttribute->_token->toString() : L"";

		return tokeValue == otherTokenValue;
	}

	return false;
}

int32_t BaseFormAttribute::hashCode()
{
	if (!_token)
	{
		return 0;
	}

	return MiscUtils::hashCode(_token->getSurfaceForm().get(), _token->getOffset(), _token->getLength());
}

LuceneObjectPtr BaseFormAttribute::clone(const LuceneObjectPtr& other)
{
	LuceneObjectPtr clone = Attribute::clone(other ? other : newLucene<BaseFormAttribute>());

	BaseFormAttributePtr cloneAttribute(boost::dynamic_pointer_cast<BaseFormAttribute>(clone));
	cloneAttribute->setToken(_token);

	return clone;
}
}
}
}
}
