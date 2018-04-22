#pragma once

#include "FilteringTokenFilter.h"
#include "kuromoji/JaTypes.h"

namespace Lucene {
namespace Analysis {
namespace Ja {

/// <summary>
/// Removes tokens that match a set of part-of-speech tags.
/// </summary>
class LPPCONTRIBAPI JapanesePartOfSpeechStopFilter final : public FilteringTokenFilter
{
private:
	const HashSet<String> _stopTags;
	const TokenAttributes::PartOfSpeechAttributePtr _posAtt;

public:
	LUCENE_CLASS(JapanesePartOfSpeechStopFilter);

	JapanesePartOfSpeechStopFilter(bool enablePositionIncrements, TokenStreamPtr input, const HashSet<String>& stopTags);

protected:
	virtual bool accept();
};
}
}
}
