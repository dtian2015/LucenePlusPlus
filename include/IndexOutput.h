/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#ifndef INDEXOUTPUT_H
#define INDEXOUTPUT_H

#include "DataOutput.h"
#include "LuceneObject.h"

namespace Lucene {

/// Abstract base class for output to a file in a Directory.  A random-access output stream.  Used for all
/// Lucene index output operations.
/// @see Directory
/// @see IndexInput
class LPPAPI IndexOutput : public DataOutput
{
public:
	virtual ~IndexOutput();

	LUCENE_CLASS(IndexOutput);

public:
	/// Copy numBytes bytes from input to ourself.
	void copyBytes(const IndexInputPtr& input, int64_t numBytes);

	/// Forces any buffered output to be written.
	virtual void flush() = 0;

	/// Returns the current position in this file, where the next write will occur.
	virtual int64_t getFilePointer() = 0;

	/// Sets current position in this file, where the next write will occur.
	/// @see #getFilePointer()
	virtual void seek(int64_t pos) = 0;

	/// The number of bytes in the file.
	virtual int64_t length() = 0;

public:
	/// Set the file length. By default, this method does nothing (it's optional for a Directory to implement it).
	/// But, certain Directory implementations (for example @see FSDirectory) can use this to inform the underlying IO
	/// system to pre-allocate the file to the specified size.  If the length is longer than the current file length,
	/// the bytes added to the file are undefined.  Otherwise the file is truncated.
	/// @param length file length.
	void setLength(int64_t length);
};

}

#endif
