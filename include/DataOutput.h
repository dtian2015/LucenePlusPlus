#pragma once

#include "DataInput.h"
#include "LuceneObject.h"

namespace Lucene {

/// <summary>
/// Abstract base class for performing write operations of Lucene's low-level
/// data types.
/// </summary>
class LPPAPI DataOutput : public LuceneObject
{
public:
	virtual ~DataOutput();
	LUCENE_CLASS(DataOutput);

protected:
	static const int32_t COPY_BUFFER_SIZE;
	ByteArray copyBuffer;

public:
	/// Writes a single byte.
	/// @see IndexInput#readByte()
	virtual void writeByte(uint8_t b) = 0;

	/// Writes an array of bytes.
	/// @param b the bytes to write.
	/// @param length the number of bytes to write.
	/// @see IndexInput#readBytes(uint8_t*, int32_t, int32_t)
	virtual void writeBytes(const uint8_t* b, int32_t offset, int32_t length) = 0;

public:
	/// Writes an array of bytes.
	/// @param b the bytes to write.
	/// @param length the number of bytes to write.
	/// @see IndexInput#readBytes(uint8_t*, int32_t, int32_t)
	virtual void writeBytes(const uint8_t* b, int32_t length);

	/// Writes an int as four bytes.
	/// @see IndexInput#readInt()
	virtual void writeInt(int32_t i);

	/// <summary>
	/// Writes a short as two bytes. </summary>
	/// <seealso cref= DataInput#readShort() </seealso>
	virtual void writeShort(short i);

	/// Writes an int in a variable-length format.  Writes between one and five bytes.  Smaller values take fewer bytes.
	/// Negative numbers are not supported.
	/// @see IndexInput#readVInt()
	virtual void writeVInt(int32_t i);

	/// Writes a int64 as eight bytes.
	/// @see IndexInput#readLong()
	virtual void writeLong(int64_t i);

	/// Writes an int64 in a variable-length format.  Writes between one and five bytes.  Smaller values take fewer bytes.
	/// Negative numbers are not supported.
	/// @see IndexInput#readVLong()
	virtual void writeVLong(int64_t i);

	/// Writes a string.
	/// @see IndexInput#readString()
	virtual void writeString(const String& s);

	/// Writes a sub sequence of characters from s as the old format (modified UTF-8 encoded bytes).
	/// @param s the source of the characters.
	/// @param start the first character in the sequence.
	/// @param length the number of characters in the sequence.
	/// @deprecated -- please use {@link #writeString}
	virtual void writeChars(const String& s, int32_t start, int32_t length);

	/// Write string map as a series of key/value pairs.
	/// @param map map of string-string key-values.
	virtual void writeStringStringMap(MapStringString map);

	/// Closes this stream to further operations.
	virtual void close() = 0;
};
}
