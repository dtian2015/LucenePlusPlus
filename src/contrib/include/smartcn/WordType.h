#pragma once

namespace Lucene {
namespace Analysis {
namespace Cn {
namespace Smart {

/// <summary>
/// Internal SmartChineseAnalyzer token type constants
/// @lucene.experimental
/// </summary>
enum class WordType
{
	/// <summary>
	/// Start of a Sentence
	/// </summary>
	SENTENCE_BEGIN = 0,

	/// <summary>
	/// End of a Sentence
	/// </summary>
	SENTENCE_END = 1,

	/// <summary>
	/// Chinese Word
	/// </summary>
	CHINESE_WORD = 2,

	/// <summary>
	/// ASCII String
	/// </summary>
	STRING = 3,

	/// <summary>
	/// ASCII Alphanumeric
	/// </summary>
	NUMBER = 4,

	/// <summary>
	/// Punctuation Symbol
	/// </summary>
	DELIMITER = 5,

	/// <summary>
	/// Full-Width String
	/// </summary>
	FULLWIDTH_STRING = 6,

	/// <summary>
	/// Full-Width Alphanumeric
	/// </summary>
	FULLWIDTH_NUMBER = 7
};
}
}
}
}
