#include "ContribInc.h"

#include "smartcn/SentenceTokenizer.h"

#include "CharReader.h"
#include "CharTermAttribute.h"
#include "OffsetAttribute.h"
#include "TextFragment.h"
#include "TypeAttribute.h"
#include "smartcn/Utility.h"

namespace Lucene {
namespace Analysis {
namespace Cn {
namespace Smart {

const String SentenceTokenizer::PUNCTION = L"。，！？；,!?;";

SentenceTokenizer::SentenceTokenizer(const ReaderPtr& reader)
	: Tokenizer(reader)
	, _buffer(newLucene<StringBuffer>())
	, _termAtt(addAttribute<CharTermAttribute>())
	, _offsetAtt(addAttribute<OffsetAttribute>())
	, _typeAtt(addAttribute<TypeAttribute>())
{
}

SentenceTokenizer::SentenceTokenizer(const AttributeSourcePtr& source, const ReaderPtr& reader)
	: Tokenizer(source, reader)
	, _buffer(newLucene<StringBuffer>())
	, _termAtt(addAttribute<CharTermAttribute>())
	, _offsetAtt(addAttribute<OffsetAttribute>())
	, _typeAtt(addAttribute<TypeAttribute>())
{
}

SentenceTokenizer::SentenceTokenizer(const AttributeFactoryPtr& factory, const ReaderPtr& reader)
	: Tokenizer(factory, reader)
	, _buffer(newLucene<StringBuffer>())
	, _termAtt(addAttribute<CharTermAttribute>())
	, _offsetAtt(addAttribute<OffsetAttribute>())
	, _typeAtt(addAttribute<TypeAttribute>())
{
}

bool SentenceTokenizer::incrementToken()
{
	clearAttributes();
	_buffer->clear();

	int ci;
	wchar_t ch;
	wchar_t pch;
	bool atBegin = true;
	_tokenStart = _tokenEnd;
	ci = input->read();
	ch = static_cast<wchar_t>(ci);

	while (true)
	{
		if (ci == -1)
		{
			break;
		}
		else if (PUNCTION.find(ch) != String::npos)
		{
			// End of a sentence
			_buffer->append(ch);
			_tokenEnd++;
			break;
		}
		else if (atBegin && Utility::SPACES.find(ch) != String::npos)
		{
			_tokenStart++;
			_tokenEnd++;
			ci = input->read();
			ch = static_cast<wchar_t>(ci);
		}
		else
		{
			_buffer->append(ch);
			atBegin = false;
			_tokenEnd++;
			pch = ch;
			ci = input->read();
			ch = static_cast<wchar_t>(ci);

			// Two spaces, such as CR, LF
			if (Utility::SPACES.find(ch) != String::npos && Utility::SPACES.find(pch) != String::npos)
			{
				// buffer.append(ch);
				_tokenEnd++;
				break;
			}
		}
	}
	if (_buffer->length() == 0)
	{
		return false;
	}
	else
	{
		_termAtt->setEmpty()->append(_buffer->toString());
		_offsetAtt->setOffset(correctOffset(_tokenStart), correctOffset(_tokenEnd));
		_typeAtt->setType(L"sentence");
		return true;
	}
}

void SentenceTokenizer::reset()
{
	Tokenizer::reset();
	_tokenStart = _tokenEnd = 0;
}

void SentenceTokenizer::reset(const ReaderPtr& input)
{
	Tokenizer::reset(input);
	reset();
}

void SentenceTokenizer::end()
{
	// set final offset
	const int finalOffset = correctOffset(_tokenEnd);
	_offsetAtt->setOffset(finalOffset, finalOffset);
}
}
}
}
}
