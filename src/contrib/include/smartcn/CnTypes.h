#pragma once

#include "Lucene.h"

namespace Lucene {
namespace Analysis {
namespace Cn {
namespace Smart {

DECLARE_SHARED_PTR(WordSegmenter)

namespace Hhmm {
DECLARE_SHARED_PTR(AbstractDictionary)
DECLARE_SHARED_PTR(BigramDictionary)
DECLARE_SHARED_PTR(BiSegGraph)
DECLARE_SHARED_PTR(HHMMSegmenter)
DECLARE_SHARED_PTR(WordDictionary)
DECLARE_SHARED_PTR(PathNode)
DECLARE_SHARED_PTR(SegGraph)
DECLARE_SHARED_PTR(SegToken)
DECLARE_SHARED_PTR(SegTokenFilter)
DECLARE_SHARED_PTR(SegTokenPair)
}
}
}
}
}
