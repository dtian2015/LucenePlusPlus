#include "ContribInc.h"

#include "kuromoji/JapaneseBaseFormFilter.h"

#include "CharTermAttribute.h"
#include "KeywordAttribute.h"
#include "kuromoji/tokenattributes/BaseFormAttribute.h"

namespace Lucene {
namespace Analysis {
namespace Ja {

JapaneseBaseFormFilter::JapaneseBaseFormFilter(TokenStreamPtr input)
	: TokenFilter(input)
	, _termAtt(addAttribute<CharTermAttribute>())
	, _basicFormAtt(addAttribute<TokenAttributes::BaseFormAttribute>())
	, _keywordAtt(addAttribute<KeywordAttribute>())
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
