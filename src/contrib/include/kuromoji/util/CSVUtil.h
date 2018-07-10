#pragma once

#include "Lucene.h"

#include <regex>

namespace Lucene {
namespace Analysis {
namespace Ja {
namespace Util {

/// <summary>
/// Utility class for parsing CSV text
/// </summary>
class LPPCONTRIBAPI CSVUtil final
{
private:
	static constexpr wchar_t QUOTE = L'"';

	static constexpr wchar_t COMMA = L',';

	static const std::wregex QUOTE_REPLACE_PATTERN;

	static const String ESCAPED_QUOTE;

	CSVUtil();

public:
	/// <summary>
	/// Parse CSV line </summary>
	/// <param name="line"> </param>
	/// <returns> Array of values </returns>
	static std::vector<String> parse(const String& line);

private:
	static String unQuoteUnEscape(const String& original);

public:
	/// <summary>
	/// Quote and escape input value for CSV </summary>
	/// <param name="original"> </param>
	static String quoteEscape(const String& original);
};
}
}
}
}
