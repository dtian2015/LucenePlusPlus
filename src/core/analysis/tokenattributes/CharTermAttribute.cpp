
#include "LuceneInc.h"

#include "CharTermAttribute.h"
#include "MiscUtils.h"

namespace Lucene {

CharTermAttribute::CharTermAttribute()
{
	initTermBuffer();
}

CharTermAttribute::~CharTermAttribute()
{
}

void CharTermAttribute::copyBuffer(const wchar_t* buffer, int offset, int length)
{
	growTermBuffer(length);
	MiscUtils::arrayCopy(buffer, offset, _termBuffer.get(), 0, length);
	_termLength = length;
}

CharArray CharTermAttribute::buffer()
{
	return _termBuffer;
}

CharArray CharTermAttribute::resizeBuffer(int newSize)
{
	return resizeTermBuffer(newSize);
}

CharTermAttributePtr CharTermAttribute::setLength(int length)
{
	setTermLength(length);
	return shared_from_this();
}

CharTermAttributePtr CharTermAttribute::setEmpty()
{
	clear();
	return shared_from_this();
}

int CharTermAttribute::length()
{
	return termLength();
}

wchar_t CharTermAttribute::charAt(int index)
{
	if (index >= _termLength)
	{
		boost::throw_exception(IndexOutOfBoundsException());
	}

	return _termBuffer[index];
}

CharArray CharTermAttribute::subSequence(int const start, int const end)
{
	if (start > _termLength || end > _termLength)
	{
		boost::throw_exception(IndexOutOfBoundsException());
	}

	int length = end - start;
	CharArray result = CharArray::newInstance(MiscUtils::getNextSize(std::max(length, MIN_BUFFER_SIZE)));
	MiscUtils::arrayCopy(_termBuffer.get(), start, result.get(), 0, length);

	return result;
}

CharTermAttributePtr CharTermAttribute::append(CharArray csq)
{
	if (csq.get() == nullptr) // needed for Appendable compliance
	{
		return appendNull();
	}

	return append(csq, 0, csq.size());
}

CharTermAttributePtr CharTermAttribute::append(CharArray csq, int start, int end)
{
	if (csq.get() == nullptr) // needed for Appendable compliance
	{
		csq = CharArray::newInstance(MiscUtils::getNextSize(std::max(4, MIN_BUFFER_SIZE)));
		csq[0] = L'n';
		csq[1] = L'u';
		csq[2] = L'l';
		csq[3] = L'l';
	}

	int len = end - start;
	int csqlen = csq.size();
	if (len < 0 || start > csqlen || end > csqlen)
	{
		boost::throw_exception(IndexOutOfBoundsException());
	}

	if (len == 0)
	{
		return shared_from_this();
	}

	resizeBuffer(_termLength + len);
	while (start < end)
	{
		_termBuffer[_termLength] = csq[start++];
	}

	return shared_from_this();
}

CharTermAttributePtr CharTermAttribute::append(wchar_t c)
{
	resizeBuffer(_termLength + 1)[_termLength] = c;
	++_termLength;
	return shared_from_this();
}

CharTermAttributePtr CharTermAttribute::append(const String& s)
{
	if (s == L"") // needed for Appendable compliance
	{
		return appendNull();
	}

	int len = s.length();
	MiscUtils::arrayCopy(s.begin(), 0, resizeBuffer(_termLength + len).get(), _termLength, len);
	_termLength += len;
	return shared_from_this();
}

CharTermAttributePtr CharTermAttribute::append(CharTermAttributePtr ta)
{
	if (ta == nullptr) // needed for Appendable compliance
	{
		return appendNull();
	}

	int len = ta->length();
	MiscUtils::arrayCopy(ta->buffer().get(), 0, resizeBuffer(_termLength + len).get(), _termLength, len);
	_termLength += len;
	return shared_from_this();
}

CharTermAttributePtr CharTermAttribute::appendNull()
{
	resizeBuffer(_termLength + 4);

	_termBuffer[_termLength++] = L'n';
	_termBuffer[_termLength++] = L'u';
	_termBuffer[_termLength++] = L'l';
	_termBuffer[_termLength++] = L'l';

	return shared_from_this();
}

String CharTermAttribute::toString()
{
	return String(_termBuffer.get(), _termBuffer.size());
}
}
