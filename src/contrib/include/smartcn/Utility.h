#pragma once

#include "Lucene.h"

namespace Lucene {
namespace Analysis {
namespace Cn {
namespace Smart {

/// <summary>
/// SmartChineseAnalyzer utility constants and methods
/// @lucene.experimental
/// </summary>
class LPPCONTRIBAPI Utility
{
public:
	static CharArray const STRING_CHAR_ARRAY;

	static CharArray const NUMBER_CHAR_ARRAY;

	static CharArray const START_CHAR_ARRAY;

	static CharArray const END_CHAR_ARRAY;

	/// <summary>
	/// Delimiters will be filtered to this character by <seealso cref="SegTokenFilter"/>
	/// </summary>
	static CharArray const COMMON_DELIMITER;

	/// <summary>
	/// Space-like characters that need to be skipped: such as space, tab, newline, carriage return.
	/// </summary>
	static const String SPACES;

	/// <summary>
	/// Maximum bigram frequency (used in the smoothing function).
	/// </summary>
	static constexpr int MAX_FREQUENCE = 2079997 + 80000;

	/// <summary>
	/// compare two arrays starting at the specified offsets.
	/// </summary>
	/// <param name="larray"> left array </param>
	/// <param name="lstartIndex"> start offset into larray </param>
	/// <param name="rarray"> right array </param>
	/// <param name="rstartIndex"> start offset into rarray </param>
	/// <returns> 0 if the arrays are equal，1 if larray > rarray, -1 if larray < rarray </returns>
	static int compareArray(const CharArray& larray, int lstartIndex, const CharArray& rarray, int rstartIndex);

	/// <summary>
	/// Compare two arrays, starting at the specified offsets, but treating shortArray as a prefix to longArray.
	/// As long as shortArray is a prefix of longArray, return 0.
	/// Otherwise, behave as <seealso cref="Utility#compareArray(char[], int, char[], int)"/>
	/// </summary>
	/// <param name="shortArray"> prefix array </param>
	/// <param name="shortIndex"> offset into shortArray </param>
	/// <param name="longArray"> long array (word) </param>
	/// <param name="longIndex"> offset into longArray </param>
	/// <returns> 0 if shortArray is a prefix of longArray, otherwise act as <seealso cref="Utility#compareArray(char[], int, char[],
	/// int)"/> </returns>
	static int compareArrayByPrefix(const CharArray& shortArray, int shortIndex, const CharArray& longArray, int longIndex);

	/// <summary>
	/// Return the internal <seealso cref="CharType"/> constant of a given character. </summary>
	/// <param name="ch"> input character </param>
	/// <returns> constant from <seealso cref="CharType"/> describing the character type.
	/// </returns>
	/// <seealso cref= CharType </seealso>
	static int getCharType(wchar_t ch);
};
}
}
}
}
