/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////

#include "LowerCaseFilter.h"
#include "CharFolder.h"
#include "CharTermAttribute.h"
#include "LuceneInc.h"
#include "TermAttribute.h"

namespace Lucene {

LowerCaseFilter::LowerCaseFilter(const TokenStreamPtr& input) : TokenFilter(input) {
	termAtt = input->hasAttribute<CharTermAttribute>() ? addAttribute<CharTermAttribute>() : addAttribute<TermAttribute>();
}

LowerCaseFilter::~LowerCaseFilter() {
}

bool LowerCaseFilter::incrementToken() {
    if (input->incrementToken()) {
        wchar_t* buffer = termAtt->termBufferArray();
        CharFolder::toLower(buffer, buffer + termAtt->termLength());
        return true;
    }
    return false;
}

}
