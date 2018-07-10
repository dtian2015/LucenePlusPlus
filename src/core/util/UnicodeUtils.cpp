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

bool UnicodeUtil::validUTF16String(const CharArray& s)
{
	const int size = s.size();
	for (int i = 0; i < size; i++)
	{
		char16_t ch = s[i];

#ifdef LPP_UNICODE_CHAR_SIZE_2
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
#else
		if (ch < 0 || ch > 0xfffe)
		{
			return false;
		}
#endif
	}

	return true;
}

UTF8Result::~UTF8Result() {
}

UnicodeResult::~UnicodeResult() {
}

}
