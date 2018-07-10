#pragma once

#include "LuceneTypes.h"
#include "TermAttribute.h"

namespace Lucene {

/// <summary>
/// The term text of a Token.
/// </summary>
class LPPAPI CharTermAttribute : public TermAttribute
{
public:
	CharTermAttribute();
	virtual ~CharTermAttribute();

	LUCENE_CLASS(CharTermAttribute);

public:
	/// <summary>
	/// Copies the contents of buffer, starting at offset for
	///  length characters, into the termBuffer array. </summary>
	///  <param name="buffer"> the buffer to copy </param>
	///  <param name="offset"> the index in the buffer of the first character to copy </param>
	///  <param name="length"> the number of characters to copy </param>
	virtual void copyBuffer(const wchar_t* buffer, int offset, int length);

	/// <summary>
	/// Returns the internal termBuffer character array which
	///  you can then directly alter.  If the array is too
	///  small for your token, use {@link
	///  #resizeBuffer(int)} to increase it.  After
	///  altering the buffer be sure to call {@link
	///  #setLength} to record the number of valid
	///  characters that were placed into the termBuffer.
	/// </summary>
	virtual CharArray buffer();

	/// <summary>
	/// Grows the termBuffer to at least size newSize, preserving the
	///  existing content. </summary>
	///  <param name="newSize"> minimum size of the new termBuffer </param>
	///  <returns> newly created termBuffer with length >= newSize </returns>
	virtual CharArray resizeBuffer(int newSize);

	/// <summary>
	/// Set number of valid characters (length of the term) in
	///  the termBuffer array. Use this to truncate the termBuffer
	///  or to synchronize with external manipulation of the termBuffer.
	///  Note: to grow the size of the array,
	///  use <seealso cref="#resizeBuffer(int)"/> first. </summary>
	///  <param name="length"> the truncated length </param>
	virtual CharTermAttributePtr setLength(int length);

	/// <summary>
	/// Sets the length of the termBuffer to zero.
	/// Use this method before appending contents
	/// using the <seealso cref="Appendable"/> interface.
	/// </summary>
	virtual CharTermAttributePtr setEmpty();

	virtual int length();

	virtual wchar_t charAt(int index);

	virtual CharArray subSequence(int const start, int const end);

	// the following methods are redefined to get rid of IOException declaration:
	virtual CharTermAttributePtr append(CharArray csq);
	virtual CharTermAttributePtr append(CharArray, int start, int end);
	virtual CharTermAttributePtr append(wchar_t c);

	/// <summary>
	/// Appends the specified {@code String} to this character sequence.
	/// <para>The characters of the {@code String} argument are appended, in order, increasing the length of
	/// this sequence by the length of the argument. If argument is {@code null}, then the four
	/// characters {@code "null"} are appended.
	/// </para>
	/// </summary>
	virtual CharTermAttributePtr append(const String& s);

	/// <summary>
	/// Appends the contents of the other {@code CharTermAttribute} to this character sequence.
	/// <para>The characters of the {@code CharTermAttribute} argument are appended, in order, increasing the length of
	/// this sequence by the length of the argument. If argument is {@code null}, then the four
	/// characters {@code "null"} are appended.
	/// </para>
	/// </summary>
	virtual CharTermAttributePtr append(CharTermAttributePtr termAtt);

	virtual String toString();

private:
	CharTermAttributePtr appendNull();
};
}
