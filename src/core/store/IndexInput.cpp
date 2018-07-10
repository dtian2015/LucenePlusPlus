/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LuceneInc.h"
#include "IndexInput.h"
#include "UTF8Stream.h"
#include "Reader.h"
#include "StringUtils.h"

namespace Lucene {

IndexInput::IndexInput() {
    preUTF8Strings = false;
}

IndexInput::~IndexInput() {
}

void IndexInput::skipChars(int32_t length) {
    for (int32_t i = 0; i < length; ++i) {
        uint8_t b = readByte();
        if ((b & 0x80) == 0) {
            // do nothing, we only need one byte
        } else if ((b & 0xe0) != 0xe0) {
            readByte();    // read an additional byte
        } else {
            // read two additional bytes
            readByte();
            readByte();
        }
    }
}
}
