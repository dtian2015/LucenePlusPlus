#include "ContribInc.h"

#include "smartcn/hhmm/AbstractDictionary.h"

#include "UnicodeUtils.h"

namespace Lucene {
namespace Analysis {
namespace Cn {
namespace Smart {
namespace Hhmm {

String AbstractDictionary::getCCByGB2312Id(int ccid)
{
	if (ccid < 0 || ccid > AbstractDictionary::GB2312_CHAR_NUM)
	{
		return L"";
	}

	int cc1 = ccid / 94 + 161;
	int cc2 = ccid % 94 + 161;

	std::vector<char> buffer(2);
	buffer[0] = static_cast<char>(cc1);
	buffer[1] = static_cast<char>(cc2);

	return UnicodeUtil::getStringFromGB2312Bytes(&buffer[0], buffer.size());
}

short AbstractDictionary::getGB2312Id(wchar_t ch)
{
	ByteArray buffer = UnicodeUtil::getGB2312Bytes(ch);
	if (buffer.size() != 2)
	{
		// Should be a two-byte character
		return -1;
	}

	int b0 = (buffer[0] & 0x0FF) - 161; // Code starts from A1, therefore subtract 0xA1=161
	int b1 = (buffer[1] & 0x0FF) - 161; // There is no Chinese char for the first and last symbol.

	// Therefore, each code page only has 16*6-2=94 characters.
	return static_cast<short>(b0 * 94 + b1);
}

int64_t AbstractDictionary::hash1(wchar_t c)
{
	constexpr int64_t p = 1099511628211;
	int64_t hash = 0xcbf29ce484222325;

	hash = (hash ^ (c & 0x00FF)) * p;
	hash = (hash ^ (c >> 8)) * p;
	hash += hash << 13;
	hash ^= hash >> 7;
	hash += hash << 3;
	hash ^= hash >> 17;
	hash += hash << 5;

	return hash;
}

int64_t AbstractDictionary::hash1(const CharArray& carray)
{
	constexpr int64_t p = 1099511628211;
	int64_t hash = 0xcbf29ce484222325;

	for (int i = 0; i < carray.size(); i++)
	{
		wchar_t d = carray[i];
		hash = (hash ^ (d & 0x00FF)) * p;
		hash = (hash ^ (d >> 8)) * p;
	}

	// hash += hash << 13;
	// hash ^= hash >> 7;
	// hash += hash << 3;
	// hash ^= hash >> 17;
	// hash += hash << 5;
	return hash;
}

int AbstractDictionary::hash2(wchar_t c)
{
	int hash = 5381;

	/* hash 33 + c */
	hash = ((hash << 5) + hash) + c & 0x00FF;
	hash = (((hash << 5) + hash) + c) >> 8;

	return hash;
}

int AbstractDictionary::hash2(const CharArray& carray)
{
	int hash = 5381;

	/* hash 33 + c */
	for (int i = 0; i < carray.size(); i++)
	{
		wchar_t d = carray[i];
		hash = ((hash << 5) + hash) + d & 0x00FF;
		hash = (((hash << 5) + hash) + d) >> 8;
	}

	return hash;
}
}
}
}
}
}
