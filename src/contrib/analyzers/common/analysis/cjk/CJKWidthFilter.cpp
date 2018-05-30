#include "ContribInc.h"

#include "CJKWidthFilter.h"

#include "CharTermAttribute.h"
#include "MiscUtils.h"

namespace Lucene {
namespace Analysis {
namespace Cjk {

namespace {

/// <summary>
/// Delete a character in-place
/// </summary>
/// <param name="s"> Input Buffer </param>
/// <param name="pos"> Position of character to delete </param>
/// <param name="len"> length of input buffer </param>
/// <returns> length of input buffer after deletion </returns>
static int deleteCharacter(CharArray& s, int pos, int len)
{
	if (pos < len)
	{
		MiscUtils::arrayCopy(s.get(), pos + 1, s.get(), pos, len - pos - 1);
	}

	return len - 1;
}
}
const std::vector<wchar_t> CJKWidthFilter::KANA_NORM = std::vector<wchar_t>{
	0x30fb, 0x30f2, 0x30a1, 0x30a3, 0x30a5, 0x30a7, 0x30a9, 0x30e3, 0x30e5, 0x30e7, 0x30c3, 0x30fc, 0x30a2, 0x30a4, 0x30a6,
	0x30a8, 0x30aa, 0x30ab, 0x30ad, 0x30af, 0x30b1, 0x30b3, 0x30b5, 0x30b7, 0x30b9, 0x30bb, 0x30bd, 0x30bf, 0x30c1, 0x30c4,
	0x30c6, 0x30c8, 0x30ca, 0x30cb, 0x30cc, 0x30cd, 0x30ce, 0x30cf, 0x30d2, 0x30d5, 0x30d8, 0x30db, 0x30de, 0x30df, 0x30e0,
	0x30e1, 0x30e2, 0x30e4, 0x30e6, 0x30e8, 0x30e9, 0x30ea, 0x30eb, 0x30ec, 0x30ed, 0x30ef, 0x30f3, 0x3099, 0x309A};

const std::vector<char> CJKWidthFilter::KANA_COMBINE_VOICED = std::vector<char>{
	78, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0,
	1,  0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 8, 8, 8, 8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1};

const std::vector<char> CJKWidthFilter::KANA_COMBINE_HALF_VOICED = std::vector<char>{
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0,
	2, 0, 0, 2, 0, 0, 2, 0, 0, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

CJKWidthFilter::CJKWidthFilter(TokenStreamPtr input, Ja::UpdateCallbackFunc callbackFunc)
	: TokenFilter(input), _termAtt(addAttribute<CharTermAttribute>()), _callbackFunc(callbackFunc)
{
}

bool CJKWidthFilter::incrementToken()
{
	if (input->incrementToken())
	{
		const String& original = _termAtt->toString();
		bool termChanged = false;

		CharArray text = _termAtt->buffer();
		int length = _termAtt->length();
		for (int i = 0; i < length; i++)
		{
			const wchar_t ch = text[i];
			if (ch >= 0xFF01 && ch <= 0xFF5E)
			{
				// Fullwidth ASCII variants
				text[i] -= 0xFEE0;
				termChanged = true;
			}
			else if (ch >= 0xFF65 && ch <= 0xFF9F)
			{
				// Halfwidth Katakana variants
				if ((ch == 0xFF9E || ch == 0xFF9F) && i > 0 && combine(text, i, ch))
				{
					length = deleteCharacter(text, i--, length);
					termChanged = true;
				}
				else
				{
					text[i] = KANA_NORM[ch - 0xFF65];
					termChanged = true;
				}
			}
		}

		_termAtt->setLength(length);

		if (termChanged)
		{
			_callbackFunc(_termAtt->toString(), original);
		}

		return true;
	}
	else
	{
		return false;
	}
}

bool CJKWidthFilter::combine(CharArray& text, int pos, wchar_t ch)
{
	const wchar_t prev = text[pos - 1];
	if (prev >= 0x30A6 && prev <= 0x30FD)
	{
		text[pos - 1] += (ch == 0xFF9F) ? KANA_COMBINE_HALF_VOICED[prev - 0x30A6] : KANA_COMBINE_VOICED[prev - 0x30A6];
		return text[pos - 1] != prev;
	}

	return false;
}
}
}
}
