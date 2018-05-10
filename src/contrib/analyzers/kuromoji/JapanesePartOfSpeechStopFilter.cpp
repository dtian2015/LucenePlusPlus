#include "ContribInc.h"

#include "kuromoji/JapanesePartOfSpeechStopFilter.h"

#include "kuromoji/tokenattributes/PartOfSpeechAttribute.h"

namespace Lucene {
namespace Analysis {
namespace Ja {

JapanesePartOfSpeechStopFilter::JapanesePartOfSpeechStopFilter(
	bool enablePositionIncrements, TokenStreamPtr input, const HashSet<String>& stopTags)
	: FilteringTokenFilter(enablePositionIncrements, input)
	, _stopTags(stopTags)
	, _posAtt(addAttribute<TokenAttributes::PartOfSpeechAttribute>())
{
}

bool JapanesePartOfSpeechStopFilter::accept()
{
	const String pos = _posAtt->getPartOfSpeech();
	return pos == L"" || !_stopTags.contains(pos);
}
}
}
}
