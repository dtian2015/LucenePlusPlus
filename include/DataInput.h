#pragma once

#include "LuceneObject.h"

namespace Lucene {

/// <summary>
/// Abstract base class for performing read operations of Lucene's low-level
/// data types.
/// </summary>
class LPPAPI DataInput : public LuceneObject
{
protected:
	bool preUTF8Strings = false; // true if we are reading old (modified UTF8) string format

public:
	virtual ~DataInput();
	LUCENE_CLASS(DataInput);

	/// Reads and returns a single byte.
	/// @see IndexOutput#writeByte(uint8_t)
	virtual uint8_t readByte() = 0;

	/// Reads a specified number of bytes into an array at the specified offset.
	/// @param b the array to read bytes into.
	/// @param offset the offset in the array to start storing bytes.
	/// @param length the number of bytes to read.
	/// @see IndexOutput#writeBytes(const uint8_t*, int)
	virtual void readBytes(uint8_t* b, int32_t offset, int32_t length) = 0;

	/// Reads a specified number of bytes into an array at the specified offset
	/// with control over whether the read should be buffered (callers who have
	/// their own buffer should pass in "false" for useBuffer).  Currently only
	/// {@link BufferedIndexInput} respects this parameter.
	/// @param b the array to read bytes into.
	/// @param offset the offset in the array to start storing bytes.
	/// @param length the number of bytes to read.
	/// @param useBuffer set to false if the caller will handle buffering.
	/// @see IndexOutput#writeBytes(const uint8_t*,int)
	virtual void readBytes(uint8_t* b, int32_t offset, int32_t length, bool useBuffer);

	/// Reads four bytes and returns an int.
	/// @see IndexOutput#writeInt(int32_t)
	virtual int32_t readInt();

	/// Reads two bytes and returns a short.
	/// @see DataOutput#writeByte(byte)
	virtual short readShort();

	/// Reads an int stored in variable-length format.  Reads between one and five
	/// bytes.  Smaller values take fewer bytes.  Negative numbers are not supported.
	/// @see IndexOutput#writeVInt(int32_t)
	virtual int32_t readVInt();

	/// Reads eight bytes and returns a int64.
	/// @see IndexOutput#writeLong(int64_t)
	virtual int64_t readLong();

	/// Reads a int64 stored in variable-length format.  Reads between one and nine
	/// bytes.  Smaller values take fewer bytes.  Negative numbers are not supported.
	virtual int64_t readVLong();

	/// <summary>
	/// Call this if readString should read characters stored
	///  in the old modified UTF8 format (length in java chars
	///  and java's modified UTF8 encoding).  This is used for
	///  indices written pre-2.4 See LUCENE-510 for details.
	/// </summary>
	virtual void setModifiedUTF8StringsMode();

	/// Reads a string.
	/// @see IndexOutput#writeString(const String&)
	virtual String readString();

	/// Reads a modified UTF8 format string.
	virtual String readModifiedUTF8String();

	/// Reads Lucene's old "modified UTF-8" encoded characters into an array.
	/// @param buffer the array to read characters into.
	/// @param start the offset in the array to start storing characters.
	/// @param length the number of characters to read.
	/// @see IndexOutput#writeChars(const String& s, int32_t, int32_t)
	virtual int32_t readChars(wchar_t* buffer, int32_t start, int32_t length);

	/// Returns a clone of this stream.
	///
	/// Clones of a stream access the same data, and are positioned at the same
	/// point as the stream they were cloned from.
	///
	/// Subclasses must ensure that clones may be positioned at different points
	/// in the input from each other and from the stream they were cloned from.
	virtual LuceneObjectPtr clone(const LuceneObjectPtr& other = LuceneObjectPtr());

	/// Read string map as a series of key/value pairs.
	virtual MapStringString readStringStringMap();

	/// Closes the stream to further operations.
	virtual void close() = 0;
};
}
