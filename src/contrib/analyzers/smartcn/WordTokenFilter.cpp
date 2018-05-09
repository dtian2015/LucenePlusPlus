#include "ContribInc.h"

#include "smartcn/WordTokenFilter.h"

#include "CharTermAttribute.h"
#include "OffsetAttribute.h"
#include "TypeAttribute.h"
#include "smartcn/WordSegmenter.h"
#include "smartcn/hhmm/SegToken.h"

namespace Lucene {
namespace Analysis {
namespace Cn {
namespace Smart {

using namespace Hhmm;

WordTokenFilter::WordTokenFilter(const TokenStreamPtr& input)
	: TokenFilter(input)
	, _wordSegmenter(newLucene<WordSegmenter>())
	, _tokenBuffer(Collection<Hhmm::SegTokenPtr>::newInstance())
	, _tokenIter(_tokenBuffer.end())
	, _termAtt(addAttribute<CharTermAttribute>())
	, _offsetAtt(addAttribute<OffsetAttribute>())
	, _typeAtt(addAttribute<TypeAttribute>())
{
}

bool WordTokenFilter::incrementToken()
{
	if (_tokenIter == _tokenBuffer.end())
	{
		// there are no remaining tokens from the current sentence... are there more sentences?
		if (input->incrementToken())
		{
			_tokStart = _offsetAtt->startOffset();
			_tokEnd = _offsetAtt->endOffset();

			// if length by start + end offsets doesn't match the term text then assume
			// this is a synonym and don't adjust the offsets.
			_hasIllegalOffsets = (_tokStart + _termAtt->length()) != _tokEnd;

			// a new sentence is available: process it
			_tokenBuffer = _wordSegmenter->segmentSentence(_termAtt->toString(), _offsetAtt->startOffset());
			_tokenIter = _tokenBuffer.begin();

			/*
			 * it should not be possible to have a sentence with 0 words, check just in case.
			 * returning EOS isn't the best either, but its the behavior of the original code.
			 */
			if (_tokenIter == _tokenBuffer.end())
			{
				return false;
			}
		}
		else
		{
			return false; // no more sentences, end of stream!
		}
	}

	// WordTokenFilter must clear attributes, as it is creating new tokens.
	clearAttributes();

	// There are remaining tokens from the current sentence, return the next one.
	SegTokenPtr nextWord = *_tokenIter;
	_termAtt->copyBuffer(nextWord->charArray.get(), 0, nextWord->charArray.size());

	if (_hasIllegalOffsets)
	{
		_offsetAtt->setOffset(_tokStart, _tokEnd);
	}
	else
	{
		_offsetAtt->setOffset(nextWord->startOffset, nextWord->endOffset);
	}

	_typeAtt->setType(L"word");
	++_tokenIter;

	return true;
}

void WordTokenFilter::reset()
{
	TokenFilter::reset();
	_tokenIter = _tokenBuffer.end();
}
}
}
}
}
