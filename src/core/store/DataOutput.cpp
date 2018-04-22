#include "LuceneInc.h"

#include "DataOutput.h"
#include "MiscUtils.h"
#include "StringUtils.h"
#include "UnicodeUtils.h"

namespace Lucene {

const int32_t DataOutput::COPY_BUFFER_SIZE = 16384;

DataOutput::~DataOutput()
{
}

void DataOutput::writeBytes(const uint8_t* b, int32_t length)
{
	writeBytes(b, 0, length);
}

void DataOutput::writeInt(int32_t i)
{
	writeByte((uint8_t)(i >> 24));
	writeByte((uint8_t)(i >> 16));
	writeByte((uint8_t)(i >> 8));
	writeByte((uint8_t)i);
}

void DataOutput::writeShort(short i)
{
	writeByte(static_cast<uint8_t>(i >> 8));
	writeByte(static_cast<uint8_t>(i));
}

void DataOutput::writeVInt(int32_t i)
{
	while ((i & ~0x7f) != 0)
	{
		writeByte((uint8_t)((i & 0x7f) | 0x80));
		i = MiscUtils::unsignedShift(i, 7);
	}
	writeByte((uint8_t)i);
}

void DataOutput::writeLong(int64_t i)
{
	writeInt((int32_t)(i >> 32));
	writeInt((int32_t)i);
}

void DataOutput::writeVLong(int64_t i)
{
	while ((i & ~0x7f) != 0)
	{
		writeByte((uint8_t)((i & 0x7f) | 0x80));
		i = MiscUtils::unsignedShift(i, (int64_t)7);
	}
	writeByte((uint8_t)i);
}

void DataOutput::writeString(const String& s)
{
	UTF8ResultPtr utf8Result(newLucene<UTF8Result>());
	StringUtils::toUTF8(s.c_str(), s.length(), utf8Result);
	writeVInt(utf8Result->length);
	writeBytes(utf8Result->result.get(), utf8Result->length);
}

void DataOutput::writeChars(const String& s, int32_t start, int32_t length)
{
	int32_t end = start + length;
	for (int32_t i = start; i < end; ++i)
	{
		int32_t code = (int32_t)s[i];
		if (code >= 0x01 && code <= 0x7f)
		{
			writeByte((uint8_t)code);
		}
		else if (((code >= 0x80) && (code <= 0x7ff)) || code == 0)
		{
			writeByte((uint8_t)(0xc0 | (code >> 6)));
			writeByte((uint8_t)(0x80 | (code & 0x3f)));
		}
		else
		{
			writeByte((uint8_t)(0xe0 | MiscUtils::unsignedShift(code, 12)));
			writeByte((uint8_t)(0x80 | ((code >> 6) & 0x3f)));
			writeByte((uint8_t)(0x80 | (code & 0x3f)));
		}
	}
}

void DataOutput::writeStringStringMap(MapStringString map)
{
	if (!map)
	{
		writeInt(0);
	}
	else
	{
		writeInt(map.size());
		for (MapStringString::iterator entry = map.begin(); entry != map.end(); ++entry)
		{
			writeString(entry->first);
			writeString(entry->second);
		}
	}
}
}
