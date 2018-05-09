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
#include "unicode/utf8.h"

#include <codecvt>
#include <iomanip>
#include <iostream>

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

// utility function for output
void hex_print(const std::string& s)
{
	std::cout << std::hex << std::setfill('0');
	for (unsigned char c : s)
		std::cout << std::setw(2) << static_cast<int>(c) << ' ';
	std::cout << std::dec << '\n';
}

ByteArray UnicodeUtil::getGB2312Bytes(wchar_t ch)
{
	constexpr int OUTLEN = 3;
	ByteArray result;

	try
	{
		String s(1, ch);
		std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
		std::string u8str = conv.to_bytes(s);

		std::cout << "UTF-8 conversion produced " << u8str.size() << " bytes" << std::endl;
		hex_print(u8str);

		char* src = strdup(u8str.c_str());
		char* buffer = new char[OUTLEN];
		utf8_to_gb(src, buffer, OUTLEN);

		int gbLength = std::strlen(buffer);

		std::cout << "GB conversion produced " << gbLength << " bytes" << std::endl;
		hex_print(buffer);

		result = ByteArray::newInstance(gbLength);
		MiscUtils::arrayCopy(buffer, 0, result.get(), 0, gbLength);

		return result;
	}
	catch (const LuceneException& e)
	{
		result = ByteArray::newInstance(0);
	}

	return result;
}

String UnicodeUtil::getStringFromGB2312Bytes(char* bytes, int size)
{
	const int OUTLEN = 4 * size;
	char* buffer = new char[OUTLEN];

	try
	{
		gb_to_utf8(bytes, buffer, OUTLEN);

		size_t utf8Lengh = std::strlen(buffer);
		std::cout << "UTF8 conversion produced " << utf8Lengh << " bytes" << std::endl;
		hex_print(buffer);

		std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> conv;
		String cchar = conv.from_bytes(buffer);
		return cchar;
	}
	catch (const LuceneException& e)
	{
		return L"";
	}
}

short UnicodeUtil::getGB2312Id(wchar_t ch)
{
	constexpr int OUTLEN = 3;
	try
	{
		//		String s(1, ch);
		//		std::wstring_convert<std::codecvt_utf8<wchar_t>> conv;
		//		std::string u8str = conv.to_bytes(s);
		//
		//		std::cout << "UTF-8 conversion produced " << u8str.size() << " bytes" << std::endl;
		//		hex_print(u8str);
		//
		//		char* src = strdup(u8str.c_str());
		//		char* buffer = new char[OUTLEN];
		//		utf8_to_gb(src, buffer, OUTLEN);
		//
		//		size_t gbLength = std::strlen(buffer);
		//		std::cout << "GB conversion produced " << gbLength << " bytes" << std::endl;
		//		hex_print(buffer);

		ByteArray buffer = getGB2312Bytes(ch);
		int gbLength = buffer.size();

		if (gbLength != 2)
		{
			// Should be a two-byte character
			return -1;
		}

		int b0 = (buffer[0] & 0x0FF) - 161; // Code starts from A1, therefore subtract 0xA1=161
		int b1 = (buffer[1] & 0x0FF) - 161; // There is no Chinese char for the first and last symbol.

		// Therefore, each code page only has 16*6-2=94 characters.
		return static_cast<short>(b0 * 94 + b1);
	}
	catch (const LuceneException& e)
	{
	}

	return -1;
}

String UnicodeUtil::getCCByGB2312Id(int ccid)
{
	//	constexpr int OUTLEN = 4;
	static constexpr int GB2312_CHAR_NUM = 87 * 94;

	if (ccid < 0 || ccid > GB2312_CHAR_NUM)
	{
		return L"";
	}

	int cc1 = ccid / 94 + 161;
	int cc2 = ccid % 94 + 161;
	std::vector<char> src(2);
	src[0] = static_cast<char>(cc1);
	src[1] = static_cast<char>(cc2);

	return getStringFromGB2312Bytes(&src[0], src.size());

	//	char* buffer = new char[OUTLEN];
	//
	//	try
	//	{
	//		gb_to_utf8(&src[0], buffer, OUTLEN);
	//
	//		size_t utf8Lengh = std::strlen(buffer);
	//		std::cout << "UTF8 conversion produced " << utf8Lengh << " bytes" << std::endl;
	//		hex_print(buffer);
	//
	//		std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> conv;
	//		String cchar = conv.from_bytes(buffer);
	//		return cchar;
	//	}
	//	catch (const LuceneException& e)
	//	{
	//		return L"";
	//	}
}

UTF8Result::~UTF8Result() {
}

UnicodeResult::~UnicodeResult() {
}

}
