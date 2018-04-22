/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "IndexOutput.h"
#include "IndexInput.h"

namespace Lucene {

IndexOutput::~IndexOutput() {
}

void IndexOutput::copyBytes(const IndexInputPtr& input, int64_t numBytes)
{
	BOOST_ASSERT(numBytes >= 0);
	int64_t left = numBytes;
	if (!copyBuffer)
	{
		copyBuffer = ByteArray::newInstance(COPY_BUFFER_SIZE);
	}
	while (left > 0)
	{
		int32_t toCopy = left > COPY_BUFFER_SIZE ? COPY_BUFFER_SIZE : (int32_t)left;
		input->readBytes(copyBuffer.get(), 0, toCopy);
		writeBytes(copyBuffer.get(), 0, toCopy);
		left -= toCopy;
	}
}

void IndexOutput::setLength(int64_t length)
{
}
}
