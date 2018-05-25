#include "ContribInc.h"

#include "kuromoji/JapaneseBaseFormFilter.h"

#include "CharTermAttribute.h"
#include "KeywordAttribute.h"
#include "kuromoji/tokenattributes/BaseFormAttribute.h"

namespace Lucene {
namespace Analysis {
namespace Ja {

JapaneseBaseFormFilter::JapaneseBaseFormFilter(TokenStreamPtr input, UpdateCallbackFunc callbackFunc)
	: TokenFilter(input)
	, _termAtt(addAttribute<CharTermAttribute>())
	, _basicFormAtt(addAttribute<TokenAttributes::BaseFormAttribute>())
	, _keywordAtt(addAttribute<KeywordAttribute>())
	, _callbackFunc(callbackFunc)
{
}

bool JapaneseBaseFormFilter::incrementToken()
{
	if (input->incrementToken())
	{
		if (!_keywordAtt->isKeyword())
		{
			String baseForm = _basicFormAtt->getBaseForm();
			if (baseForm != L"")
			{
				// store base form to original term mapping as base form is for word frequency query while the original term
				// is used by text search query
				if (_callbackFunc)
				{
					_callbackFunc(baseForm, _termAtt->toString());
				}

				// replace original term with base form
				_termAtt->setEmpty()->append(baseForm);
			}
		}

		return true;
	}
	else
	{
		return false;
	}
}
}
}
}
