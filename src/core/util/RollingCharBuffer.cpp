#include "RollingCharBuffer.h"
#include "MiscUtils.h"
#include "Reader.h"

namespace Lucene {

void RollingCharBuffer::reset(ReaderPtr reader)
{
	this->_reader = reader;
	_nextPos = 0;
	_nextWrite = 0;
	_count = 0;
	_end = false;
}

int RollingCharBuffer::get(int pos)
{
	if (pos == _nextPos)
	{
		if (_end)
		{
			return -1;
		}

		int ch = _reader->read();
		if (ch == -1)
		{
			_end = true;
			return -1;
		}

		if (_count == _buffer.size())
		{
			// Grow
			CharArray newBuffer = CharArray::newInstance(MiscUtils::getNextSize(1 + _count));
			// System.out.println(Thread.currentThread().getName() + ": cb grow " + newBuffer.length);
			MiscUtils::arrayCopy(_buffer.get(), _nextWrite, newBuffer.get(), 0, _buffer.size() - _nextWrite);
			MiscUtils::arrayCopy(_buffer.get(), 0, newBuffer.get(), _buffer.size() - _nextWrite, _nextWrite);
			_nextWrite = _buffer.size();
			_buffer = newBuffer;
		}

		if (_nextWrite == _buffer.size())
		{
			_nextWrite = 0;
		}

		_buffer[_nextWrite++] = static_cast<wchar_t>(ch);
		_count++;
		_nextPos++;
		return ch;
	}
	else
	{
		// Cannot read from future (except by 1):
		BOOST_ASSERT(pos < _nextPos);

		// Cannot read from already freed past:
		BOOST_ASSERT(_nextPos - pos <= _count);

		int index = getIndex(pos);
		return _buffer[index];
	}
}

bool RollingCharBuffer::inBounds(int pos)
{
	return pos >= 0 && pos < _nextPos && pos >= _nextPos - _count;
}

int RollingCharBuffer::getIndex(int pos)
{
	int index = _nextWrite - (_nextPos - pos);
	if (index < 0)
	{
		// Wrap:
		index += _buffer.size();
		BOOST_ASSERT(index >= 0);
	}

	return index;
}

CharArray RollingCharBuffer::get(int posStart, int length)
{
	BOOST_ASSERT(length > 0);
	BOOST_ASSERT(inBounds(posStart));
	// System.out.println("    buffer.get posStart=" + posStart + " len=" + length);

	const int startIndex = getIndex(posStart);
	const int endIndex = getIndex(posStart + length);
	// System.out.println("      startIndex=" + startIndex + " endIndex=" + endIndex);

	CharArray result = CharArray::newInstance(length);
	if (endIndex >= startIndex && length < _buffer.size())
	{
		MiscUtils::arrayCopy(_buffer.get(), startIndex, result.get(), 0, endIndex - startIndex);
	}
	else
	{
		// Wrapped:
		int part1 = _buffer.size() - startIndex;
		MiscUtils::arrayCopy(_buffer.get(), startIndex, result.get(), 0, part1);
		MiscUtils::arrayCopy(_buffer.get(), 0, result.get(), _buffer.size() - startIndex, length - part1);
	}

	return result;
}

void RollingCharBuffer::freeBefore(int pos)
{
	BOOST_ASSERT(pos >= 0);
	BOOST_ASSERT(pos <= _nextPos);

	const int newCount = _nextPos - pos;

	BOOST_ASSERT(newCount <= _count);
	BOOST_ASSERT(newCount <= _buffer.size());
	_count = newCount;
}
}
