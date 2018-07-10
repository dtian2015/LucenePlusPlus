#include "kuromoji/util/CSVUtil.h"
#include "TextFragment.h"

#include <boost/algorithm/string.hpp>

namespace Lucene {
namespace Analysis {
namespace Ja {
namespace Util {

const std::wregex CSVUtil::QUOTE_REPLACE_PATTERN = std::wregex(L"^\"([^\"]+)\"$");
const String CSVUtil::ESCAPED_QUOTE = L"\"\"";

CSVUtil::CSVUtil()
{
} // no instance!!!

std::vector<String> CSVUtil::parse(const String& line)
{
	bool insideQuote = false;
	std::vector<String> result;
	int quoteCount = 0;
	StringBuffer sb;

	for (int i = 0; i < line.length(); i++)
	{
		wchar_t c = line[i];

		if (c == QUOTE)
		{
			insideQuote = !insideQuote;
			quoteCount++;
		}

		if (c == COMMA && !insideQuote)
		{
			String value = sb.toString();
			value = unQuoteUnEscape(value);
			result.push_back(value);
			sb.clear();
			continue;
		}

		sb.append(c);
	}

	result.push_back(sb.toString());

	// Validate
	if (quoteCount % 2 != 0)
	{
		return std::vector<String>(0);
	}

	return result;
}

String CSVUtil::unQuoteUnEscape(const String& original)
{
	String result = original;

	// Unquote
	if (result.find(L'\"') != String::npos)
	{
		std::wsmatch match;

		if (std::regex_search(result, match, QUOTE_REPLACE_PATTERN))
		{
			result = match[1];
		}

		// Unescape
		if (result.find(ESCAPED_QUOTE) != String::npos)
		{
			boost::replace_all(result, ESCAPED_QUOTE, L"\"");
		}
	}

	return result;
}

String CSVUtil::quoteEscape(const String& original)
{
	String result = original;

	if (result.find(L'\"') != String::npos)
	{
		boost::replace_all(result, L"\"", ESCAPED_QUOTE);
	}

	if (result.find(COMMA) != String::npos)
	{
		result = L"\"" + result + L"\"";
	}

	return result;
}
}
}
}
}
