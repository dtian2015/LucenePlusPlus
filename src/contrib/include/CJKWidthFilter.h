#pragma once

#include "TokenFilter.h"
#include "kuromoji/JaTypes.h"

#include <vector>

namespace Lucene {
namespace Analysis {
namespace Cjk {

/// <summary>
/// A <seealso cref="TokenFilter"/> that normalizes CJK width differences:
/// <ul>
///   <li>Folds fullwidth ASCII variants into the equivalent basic latin
///   <li>Folds halfwidth Katakana variants into the equivalent kana
/// </ul>
/// <para>
/// NOTE: this filter can be viewed as a (practical) subset of NFKC/NFKD
/// Unicode normalization. See the normalization support in the ICU package
/// for full normalization.
/// </para>
/// </summary>
class LPPCONTRIBAPI CJKWidthFilter final : public TokenFilter
{
private:
	CharTermAttributePtr _termAtt;

	/* halfwidth kana mappings: 0xFF65-0xFF9D
	 *
	 * note: 0xFF9C and 0xFF9D are only mapped to 0x3099 and 0x309A
	 * as a fallback when they cannot properly combine with a preceding
	 * character into a composed form.
	 */
	static std::vector<wchar_t> const KANA_NORM;

	const Ja::UpdateCallbackFunc _callbackFunc;

public:
	LUCENE_CLASS(CJKWidthFilter);

	CJKWidthFilter(TokenStreamPtr input, Ja::UpdateCallbackFunc callbackFunc = nullptr);

	virtual bool incrementToken();

private:
	/* kana combining diffs: 0x30A6-0x30FD */
	static std::vector<char> const KANA_COMBINE_VOICED;

	static std::vector<char> const KANA_COMBINE_HALF_VOICED;

	/// <summary>
	/// returns true if we successfully combined the voice mark
	/// </summary>
	static bool combine(CharArray& text, int pos, wchar_t ch);
};
}
}
}
