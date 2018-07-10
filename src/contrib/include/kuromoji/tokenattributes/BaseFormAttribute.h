#pragma once

#include "Attribute.h"
#include "kuromoji/JaTypes.h"
#include "kuromoji/Token.h"

namespace Lucene {
namespace Analysis {
namespace Ja {
namespace TokenAttributes {

/// <summary>
/// Attribute for <seealso cref="Token#getBaseForm()"/>.
/// <para>
/// Note: depending on part of speech, this value may not be applicable,
/// and will be null.
/// </para>
/// </summary>
class LPPCONTRIBAPI BaseFormAttribute : public Attribute
{
public:
	LUCENE_CLASS(BaseFormAttribute);

	virtual String getBaseForm();
	virtual void setToken(Ja::TokenPtr token);

	virtual void clear();
	virtual void copyTo(const AttributePtr& target);

	virtual bool equals(const LuceneObjectPtr& other);
	virtual int32_t hashCode();
	virtual LuceneObjectPtr clone(const LuceneObjectPtr& other = LuceneObjectPtr());

private:
	Ja::TokenPtr _token;
};
}
}
}
}
