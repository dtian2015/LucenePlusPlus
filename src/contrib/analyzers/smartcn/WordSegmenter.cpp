#include "ContribInc.h"

#include "smartcn/WordSegmenter.h"

#include "smartcn/WordType.h"
#include "smartcn/hhmm/HHMMSegmenter.h"
#include "smartcn/hhmm/SegToken.h"
#include "smartcn/hhmm/SegTokenFilter.h"

namespace Lucene {
namespace Analysis {
namespace Cn {
namespace Smart {

using namespace Hhmm;

WordSegmenter::WordSegmenter()
{
	_hhmmSegmenter = newLucene<HHMMSegmenter>();
	_tokenFilter = newLucene<SegTokenFilter>();
}

Collection<SegTokenPtr> WordSegmenter::segmentSentence(const String& sentence, int startOffset)
{
	Collection<SegTokenPtr> segTokenList = _hhmmSegmenter->process(sentence);

	// tokens from sentence, excluding WordType.SENTENCE_BEGIN and WordType.SENTENCE_END
	Collection<SegTokenPtr> result = Collection<SegTokenPtr>::newInstance();

	if (segTokenList.size() > 2) // if its not an empty sentence
	{
		result.addAll(segTokenList.begin() + 1, segTokenList.end() - 1);
	}

	for (auto st : result)
	{
		convertSegToken(st, sentence, startOffset);
	}

	// TODO: Daniel to remove debug info
	std::cout << "start ---------segmentSentence() result----------------------" << std::endl;

	for (const auto& token : result)
	{
		std::cout << "start = " << token->startOffset << ", end = " << token->endOffset << ", weight = " << token->weight
				  << ", index = " << token->index << std::endl;
	}

	return result;
}

SegTokenPtr WordSegmenter::convertSegToken(SegTokenPtr st, const String& sentence, int sentenceStartOffset)
{
	switch (st->wordType)
	{
		case WordType::STRING:
		case WordType::NUMBER:
		case WordType::FULLWIDTH_NUMBER:
		case WordType::FULLWIDTH_STRING:
		{
			String token = sentence.substr(st->startOffset, st->endOffset - st->startOffset);
			CharArray text = CharArray::newInstance(token.size());
			std::copy(token.begin(), token.end(), text.get());

			st->charArray = text;
			break;
		}
		default:
			break;
	}

	st = _tokenFilter->filter(st);
	st->startOffset += sentenceStartOffset;
	st->endOffset += sentenceStartOffset;

	return st;
}
}
}
}
}
