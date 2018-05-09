#pragma once

#include "Attribute.h"
#include "kuromoji/JaTypes.h"
#include "kuromoji/Token.h"

namespace Lucene {
namespace Analysis {
namespace Ja {
namespace TokenAttributes {

/// <summary>
/// Attribute for Kuromoji reading data
/// </summary>
class LPPCONTRIBAPI ReadingAttribute : public Attribute
{
public:
	LUCENE_CLASS(ReadingAttribute);

	virtual String getReading();
	virtual String getPronunciation();
	virtual void setToken(Ja::TokenPtr token);

	void clear();
	void copyTo(const AttributePtr& target);

	virtual bool equals(const LuceneObjectPtr& other);
	virtual int32_t hashCode();
	virtual LuceneObjectPtr clone(const LuceneObjectPtr& other = LuceneObjectPtr());

	virtual String toString();

private:
	Ja::TokenPtr _token;
};
}
}
}
}
