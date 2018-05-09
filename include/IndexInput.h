/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef INDEXINPUT_H
#define INDEXINPUT_H

#include "DataInput.h"
#include "LuceneObject.h"

namespace Lucene {

/// Abstract base class for input from a file in a {@link Directory}.
/// A random-access input stream.  Used for all Lucene index input operations.
/// @see Directory
class LPPAPI IndexInput : public DataInput
{
public:
	IndexInput();
	virtual ~IndexInput();

	LUCENE_CLASS(IndexInput);

public:
	/// Similar to {@link #readChars(wchar_t*, int32_t, int32_t)} but does not
	/// do any conversion operations on the bytes it is reading in.  It still
	/// has to invoke {@link #readByte()} just as {@link #readChars(wchar_t*, int32_t, int32_t)}
	/// does, but it does not need a buffer to store anything and it does not have
	/// to do any of the bitwise operations, since we don't actually care what is
	/// in the byte except to determine how many more bytes to read.
	/// @param length The number of chars to read.
	/// @deprecated this method operates on old "modified utf8" encoded strings.
	virtual void skipChars(int32_t length);

	/// Returns the current position in this file, where the next read will occur.
	/// @see #seek(int64_t)
	virtual int64_t getFilePointer() = 0;

	/// Sets current position in this file, where the next read will occur.
	/// @see #getFilePointer()
	virtual void seek(int64_t pos) = 0;

	/// The number of bytes in the file.
	virtual int64_t length() = 0;
};

}

#endif
