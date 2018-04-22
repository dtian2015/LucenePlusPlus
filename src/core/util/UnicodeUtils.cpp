/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "UnicodeUtils.h"
#include "BufferedReader.h"
#include "LuceneInc.h"
#include "StringReader.h"
#include "UTF8Stream.h"
#include "unicode/guniprop.h"

namespace Lucene {

std::u16string make_u16string(const String& ws)
/* Creates a UTF-16 string from a wide-character string.  Any wide characters
 * outside the allowed range of UTF-16 are mapped to the sentinel value U+FFFD,
 * per the Unicode documentation. (http://www.unicode.org/faq/private_use.html
 * retrieved 12 March 2017.) Unpaired surrogates in ws are also converted to
 * sentinel values.  Noncharacters, however, are left intact.  As a fallback,
 * if wide characters are the same size as char16_t, this does a more trivial
 * construction using that implicit conversion.
 */
{
	/* We assume that, if this test passes, a wide-character string is already
	 * UTF-16, or at least converts to it implicitly without needing surrogate
	 * pairs.
	 */
	if (sizeof(wchar_t) == sizeof(char16_t))
	{
		return std::u16string(ws.begin(), ws.end());
	}
	else
	{
		/* The conversion from UTF-32 to UTF-16 might possibly require surrogates.
		 * A surrogate pair suffices to represent all wide characters, because all
		 * characters outside the range will be mapped to the sentinel value
		 * U+FFFD.  Add one character for the terminating NUL.
		 */
		const size_t max_len = 2 * ws.length() + 1;
		// Our temporary UTF-16 string.
		std::u16string result;

		result.reserve(max_len);

		for (const wchar_t& wc : ws)
		{
			const std::wint_t chr = wc;

			if (chr < 0 || chr > 0x10FFFF || (chr >= 0xD800 && chr <= 0xDFFF))
			{
				// Invalid code point.  Replace with sentinel, per Unicode standard:
				constexpr char16_t sentinel = u'\uFFFD';
				result.push_back(sentinel);
			}
			else if (chr < 0x10000UL)
			{ // In the BMP.
				result.push_back(static_cast<char16_t>(wc));
			}
			else
			{
				const char16_t leading = static_cast<char16_t>(((chr - 0x10000UL) / 0x400U) + 0xD800U);
				const char16_t trailing = static_cast<char16_t>(((chr - 0x10000UL) % 0x400U) + 0xDC00U);

				result.append({leading, trailing});
			} // end if
		} // end for

		/* The returned string is shrunken to fit, which might not be the Right
		 * Thing if there is more to be added to the string.
		 */
		result.shrink_to_fit();

		// We depend here on the compiler to optimize the move constructor.
		return result;
	} // end if
	// Not reached.
}

UnicodeUtil::~UnicodeUtil() {
}

bool UnicodeUtil::isAlnum(wchar_t c) {
    return g_unichar_isalnum(c);
}

bool UnicodeUtil::isAlpha(wchar_t c) {
    return g_unichar_isalpha(c);
}

bool UnicodeUtil::isDigit(wchar_t c) {
    return g_unichar_isdigit(c);
}

bool UnicodeUtil::isSpace(wchar_t c) {
    return g_unichar_isspace(c);
}

bool UnicodeUtil::isUpper(wchar_t c) {
    return g_unichar_isupper(c);
}

bool UnicodeUtil::isLower(wchar_t c) {
    return g_unichar_islower(c);
}

bool UnicodeUtil::isOther(wchar_t c) {
    return (g_unichar_type(c) == G_UNICODE_OTHER_LETTER);
}

bool UnicodeUtil::isNonSpacing(wchar_t c) {
    return (g_unichar_type(c) == G_UNICODE_NON_SPACING_MARK);
}

bool UnicodeUtil::isPunctuation(wchar_t c)
{
	return g_unichar_isspace(c) || g_unichar_ispunct(c) || (g_unichar_type(c) == G_UNICODE_CONTROL) ||
		(g_unichar_type(c) == G_UNICODE_FORMAT);
}

wchar_t UnicodeUtil::toUpper(wchar_t c) {
    return (wchar_t)g_unichar_toupper(c);
}

wchar_t UnicodeUtil::toLower(wchar_t c) {
    return (wchar_t)g_unichar_tolower(c);
}

bool UnicodeUtil::validUTF16String(const CharArray& text)
{
	std::u16string s = make_u16string(String(text.get(), text.size()));
	const int size = s.size();
	for (int i = 0; i < size; i++)
	{
		char16_t ch = s[i];
		if (ch >= UNI_SUR_HIGH_START && ch <= UNI_SUR_HIGH_END)
		{
			if (i < size - 1)
			{
				i++;
				char16_t nextCH = s[i];
				if (nextCH >= UNI_SUR_LOW_START && nextCH <= UNI_SUR_LOW_END)
				{
					// Valid surrogate pair
				}
				else
				{
					// Unmatched high surrogate
					return false;
				}
			}
			else
			{
				// Unmatched high surrogate
				return false;
			}
		}
		else if (ch >= UNI_SUR_LOW_START && ch <= UNI_SUR_LOW_END)
		{
			// Unmatched low surrogate
			return false;
		}
	}

	return true;
}

UTF8Result::~UTF8Result() {
}

UnicodeResult::~UnicodeResult() {
}

}
