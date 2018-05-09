#include "DataInput.h"
#include "LuceneInc.h"
#include "Reader.h"
#include "StringUtils.h"
#include "UTF8Stream.h"

namespace Lucene {

DataInput::~DataInput()
{
}

void DataInput::readBytes(uint8_t* b, int32_t offset, int32_t length, bool useBuffer)
{
	// default to ignoring useBuffer entirely
	readBytes(b, offset, length);
}

int32_t DataInput::readInt()
{
	int32_t i = (readByte() & 0xff) << 24;
	i |= (readByte() & 0xff) << 16;
	i |= (readByte() & 0xff) << 8;
	i |= (readByte() & 0xff);
	return i;
}

short DataInput::readShort()
{
	return static_cast<short>(((readByte() & 0xFF) << 8) | (readByte() & 0xFF));
}

int32_t DataInput::readVInt()
{
	uint8_t b = readByte();
	int32_t i = (b & 0x7f);

	for (int32_t shift = 7; (b & 0x80) != 0; shift += 7)
	{
		b = readByte();
		i |= (b & 0x7f) << shift;
	}
	return i;
}

int64_t DataInput::readLong()
{
	int64_t i = (int64_t)readInt() << 32;
	i |= (readInt() & 0xffffffffLL);
	return i;
}

int64_t DataInput::readVLong()
{
	uint8_t b = readByte();
	int64_t i = (b & 0x7f);

	for (int32_t shift = 7; (b & 0x80) != 0; shift += 7)
	{
		b = readByte();
		i |= (int64_t)(b & 0x7f) << shift;
	}
	return i;
}

void DataInput::setModifiedUTF8StringsMode()
{
	preUTF8Strings = true;
}

String DataInput::readString()
{
	if (preUTF8Strings)
	{
		return readModifiedUTF8String();
	}
	int32_t length = readVInt();
	ByteArray bytes(ByteArray::newInstance(length));
	readBytes(bytes.get(), 0, length);
	return StringUtils::toUnicode(bytes.get(), length);
}

String DataInput::readModifiedUTF8String()
{
	int32_t length = readVInt();
	CharArray chars(CharArray::newInstance(length));
	return String(chars.get(), readChars(chars.get(), 0, length));
}

int32_t DataInput::readChars(wchar_t* buffer, int32_t start, int32_t length)
{
	Array<uint16_t> chars(Array<uint16_t>::newInstance(length));
	for (int32_t i = 0; i < length; ++i)
	{
		uint8_t b = readByte();
		if ((b & 0x80) == 0)
		{
			chars[i] = (uint16_t)(b & 0x7f);
		}
		else if ((b & 0xe0) != 0xe0)
		{
			chars[i] = (uint16_t)(((b & 0x1f) << 6) | (readByte() & 0x3f));
		}
		else
		{
			uint32_t ch = ((b & 0x0f) << 12);
			ch |= (readByte() & 0x3f) << 6;
			ch |= (readByte() & 0x3f);
			chars[i] = (uint16_t)ch;
		}
	}
	UTF16DecoderPtr utf16Decoder(newLucene<UTF16Decoder>(chars.get(), chars.get() + length));
	int32_t decodeLength = utf16Decoder->decode(buffer + start, length);
	return decodeLength == Reader::READER_EOF ? 0 : decodeLength;
}

MapStringString DataInput::readStringStringMap()
{
	MapStringString map(MapStringString::newInstance());
	int32_t count = readInt();
	for (int32_t i = 0; i < count; ++i)
	{
		String key(readString());
		String val(readString());
		map.put(key, val);
	}
	return map;
}

LuceneObjectPtr DataInput::clone(const LuceneObjectPtr& other)
{
	DataInputPtr cloneDataInput(boost::dynamic_pointer_cast<DataInput>(LuceneObject::clone(other)));
	cloneDataInput->preUTF8Strings = preUTF8Strings;
	return cloneDataInput;
}
}
