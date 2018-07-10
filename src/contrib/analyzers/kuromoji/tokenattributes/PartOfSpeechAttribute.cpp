#include "ContribInc.h"

#include "kuromoji/tokenattributes/PartOfSpeechAttribute.h"

namespace Lucene {
namespace Analysis {
namespace Ja {
namespace TokenAttributes {

String PartOfSpeechAttribute::getPartOfSpeech()
{
	return _token == nullptr ? L"" : _token->getPartOfSpeech();
}

void PartOfSpeechAttribute::setToken(Ja::TokenPtr token)
{
	this->_token = token;
}

void PartOfSpeechAttribute::clear()
{
	_token.reset();
}

void PartOfSpeechAttribute::copyTo(const AttributePtr& target)
{
	PartOfSpeechAttributePtr t = boost::static_pointer_cast<PartOfSpeechAttribute>(target);
	t->setToken(_token);
}

bool PartOfSpeechAttribute::equals(const LuceneObjectPtr& other)
{
	if (Attribute::equals(other))
	{
		return true;
	}

	PartOfSpeechAttributePtr otherPartOfSpeechAttribute(boost::dynamic_pointer_cast<PartOfSpeechAttribute>(other));
	if (otherPartOfSpeechAttribute)
	{
		return toString() == otherPartOfSpeechAttribute->toString();
	}

	return false;
}

int32_t PartOfSpeechAttribute::hashCode()
{
	const auto& attributeValue = toString();
	if (attributeValue.empty())
	{
		return 0;
	}

	return MiscUtils::hashCode(attributeValue.data(), 0, attributeValue.size());
}

LuceneObjectPtr PartOfSpeechAttribute::clone(const LuceneObjectPtr& other)
{
	LuceneObjectPtr clone = Attribute::clone(other ? other : newLucene<PartOfSpeechAttribute>());

	PartOfSpeechAttributePtr cloneAttribute(boost::dynamic_pointer_cast<PartOfSpeechAttribute>(clone));
	cloneAttribute->setToken(_token);

	return clone;
}

String PartOfSpeechAttribute::toString()
{
	if (_token)
	{
		return _token->toString() + L", PartOfSpeech= " + _token->getPartOfSpeech();
	}

	return L"";
}
}
}
}
}
