#include "ContribInc.h"

#include "smartcn/hhmm/SegTokenFilter.h"

#include "smartcn/Utility.h"
#include "smartcn/WordType.h"
#include "smartcn/hhmm/SegToken.h"

namespace Lucene {
namespace Analysis {
namespace Cn {
namespace Smart {
namespace Hhmm {

SegTokenPtr SegTokenFilter::filter(SegTokenPtr token)
{
	switch (token->wordType)
	{
		case WordType::FULLWIDTH_NUMBER:
		case WordType::FULLWIDTH_STRING: // first convert full-width -> half-width
		{
			for (int i = 0; i < token->charArray.size(); i++)
			{
				if (token->charArray[i] >= 0xFF10)
				{
					token->charArray[i] -= 0xFEE0;
				}

				if (token->charArray[i] >= 0x0041 && token->charArray[i] <= 0x005A) // lowercase latin
				{
					token->charArray[i] += 0x0020;
				}
			}
			break;
		}
		case WordType::STRING:
		{
			for (int i = 0; i < token->charArray.size(); i++)
			{
				if (token->charArray[i] >= 0x0041 && token->charArray[i] <= 0x005A) // lowercase latin
				{
					token->charArray[i] += 0x0020;
				}
			}
			break;
		}
		case WordType::DELIMITER: // convert all punctuation to Utility.COMMON_DELIMITER
		{
			token->charArray = Utility::COMMON_DELIMITER;
			break;
		}
		default:
			break;
	}

	return token;
}
}
}
}
}
}
