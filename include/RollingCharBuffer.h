#pragma once

#include "LuceneObject.h"

namespace Lucene {

/// <summary>
/// Acts like a forever growing char[] as you read
///  characters into it from the provided reader, but
///  internally it uses a circular buffer to only hold the
///  characters that haven't been freed yet.  This is like a
///  PushbackReader, except you don't have to specify
///  up-front the max size of the buffer, but you do have to
///  periodically call <seealso cref="#freeBefore"/>.
/// </summary>
class LPPAPI RollingCharBuffer final : public LuceneObject
{
public:
	LUCENE_CLASS(RollingCharBuffer);

private:
	ReaderPtr _reader;

	CharArray _buffer = CharArray::newInstance(32);

	// Next array index to write to in buffer:
	int _nextWrite = 0;

	// Next absolute position to read from reader:
	int _nextPos = 0;

	// How many valid chars (wrapped) are in the buffer:
	int _count = 0;

	// True if we hit EOF
	bool _end = false;

public:
	/// <summary>
	/// Clear array and switch to new reader. </summary>
	void reset(ReaderPtr reader);

	/* Absolute position read.  NOTE: pos must not jump
	* ahead by more than 1!  Ie, it's OK to read arbitarily
	* far back (just not prior to the last {@link
	* #freeBefore}), but NOT ok to read arbitrarily far
	* ahead.  Returns -1 if you hit EOF. */
	int get(int pos);

	// For assert:
private:
	bool inBounds(int pos);

	int getIndex(int pos);

public:
	CharArray get(int posStart, int length);

	/// <summary>
	/// Call this to notify us that no chars before this
	///  absolute position are needed anymore.
	/// </summary>
	void freeBefore(int pos);
};
}
