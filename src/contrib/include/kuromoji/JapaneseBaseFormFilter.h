#pragma once

#include "TokenFilter.h"
#include "kuromoji/JaTypes.h"

namespace Lucene {
namespace Analysis {
namespace Ja {

/// <summary>
/// Replaces term text with the <seealso cref="BaseFormAttribute"/>.
/// <para>
/// This acts as a lemmatizer for verbs and adjectives.
/// </para>
/// <para>
/// To prevent terms from being stemmed use an instance of
/// <seealso cref="KeywordMarkerFilter"/> or a custom <seealso cref="TokenFilter"/> that sets
/// the <seealso cref="KeywordAttribute"/> before this <seealso cref="TokenStream"/>.
/// </para>
/// </summary>
class LPPCONTRIBAPI JapaneseBaseFormFilter final : public TokenFilter
{
private:
	const CharTermAttributePtr _termAtt;
	const TokenAttributes::BaseFormAttributePtr _basicFormAtt;
	const KeywordAttributePtr _keywordAtt;
	const UpdateCallbackFunc _callbackFunc;

public:
	LUCENE_CLASS(JapaneseBaseFormFilter);

	JapaneseBaseFormFilter(TokenStreamPtr input, UpdateCallbackFunc callbackFunc = nullptr);

	virtual bool incrementToken();
};
}
}
}
