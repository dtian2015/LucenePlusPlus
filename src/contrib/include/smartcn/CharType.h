#pragma once

namespace Lucene {
namespace Analysis {
namespace Cn {
namespace Smart {

/// <summary>
/// Internal SmartChineseAnalyzer character type constants.
/// @lucene.experimental
/// </summary>
class CharType
{
public:
	/// <summary>
	/// Punctuation Characters
	/// </summary>
	static constexpr int DELIMITER = 0;

	/// <summary>
	/// Letters
	/// </summary>
	static constexpr int LETTER = 1;

	/// <summary>
	/// Numeric Digits
	/// </summary>
	static constexpr int DIGIT = 2;

	/// <summary>
	/// Han Ideographs
	/// </summary>
	static constexpr int HANZI = 3;

	/// <summary>
	/// Characters that act as a space
	/// </summary>
	static constexpr int SPACE_LIKE = 4;

	/// <summary>
	/// Full-Width letters
	/// </summary>
	static constexpr int FULLWIDTH_LETTER = 5;

	/// <summary>
	/// Full-Width alphanumeric characters
	/// </summary>
	static constexpr int FULLWIDTH_DIGIT = 6;

	/// <summary>
	/// Other (not fitting any of the other categories)
	/// </summary>
	static constexpr int OTHER = 7;
};
}
}
}
}
