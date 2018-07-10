#pragma once

#include "Lucene.h"

#include <unordered_map>

namespace Lucene {
namespace Analysis {
namespace Ja {
namespace Util {

/// <summary>
/// Utility class for english translations of morphological data,
/// used only for debugging.
/// </summary>
class LPPCONTRIBAPI ToStringUtil
{
private:
	// a translation map for parts of speech, only used for reflectWith
	static const std::unordered_map<String, String> _posTranslations;

public:
	/// <summary>
	/// Get the english form of a POS tag
	/// </summary>
	static String getPOSTranslation(const String& s);

private:
	// a translation map for inflection types, only used for reflectWith
	static const std::unordered_map<String, String> _inflTypeTranslations;

public:
	/// <summary>
	/// Get the english form of inflection type
	/// </summary>
	static String getInflectionTypeTranslation(const String& s);

private:
	// a translation map for inflection forms, only used for reflectWith
	static const std::unordered_map<String, String> _inflFormTranslations;

public:
	/// <summary>
	/// Get the english form of inflected form
	/// </summary>
	static String getInflectedFormTranslation(const String& s);

	/// <summary>
	/// Romanize katakana with modified hepburn
	/// </summary>
	static String getRomanization(const String& s);

	/// <summary>
	/// Romanize katakana with modified hepburn
	/// </summary>
	static void getRomanization(StringStream& builder, const CharArray& s);
};
}
}
}
}
