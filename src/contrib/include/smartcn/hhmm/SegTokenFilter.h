#pragma once

#include "LuceneObject.h"
#include "smartcn/CnTypes.h"

namespace Lucene {
namespace Analysis {
namespace Cn {
namespace Smart {
namespace Hhmm {

/// <summary>
/// <para>
/// Filters a <seealso cref="SegToken"/> by converting full-width latin to half-width, then lowercasing latin.
/// Additionally, all punctuation is converted into <seealso cref="Utility#COMMON_DELIMITER"/>
/// </para>
/// @lucene.experimental
/// </summary>
class LPPCONTRIBAPI SegTokenFilter : public LuceneObject
{
public:
	LUCENE_CLASS(SegTokenFilter);

	/// <summary>
	/// Filter an input <seealso cref="SegToken"/>
	/// <para>
	/// Full-width latin will be converted to half-width, then all latin will be lowercased.
	/// All punctuation is converted into <seealso cref="Utility#COMMON_DELIMITER"/>
	/// </para>
	/// </summary>
	/// <param name="token"> input <seealso cref="SegToken"/> </param>
	/// <returns> normalized <seealso cref="SegToken"/> </returns>
	virtual SegTokenPtr filter(SegTokenPtr token);
};
}
}
}
}
}
