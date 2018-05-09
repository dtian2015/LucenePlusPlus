#pragma once

#include "LuceneObject.h"

#include "TokenFilter.h"
#include "smartcn/CnTypes.h"

namespace Lucene {
namespace Analysis {
namespace Cn {
namespace Smart {

/// <summary>
/// A <seealso cref="TokenFilter"/> that breaks sentences into words.
/// @lucene.experimental
/// </summary>
class WordTokenFilter final : public TokenFilter
{
private:
	WordSegmenterPtr _wordSegmenter;

	Collection<Hhmm::SegTokenPtr> _tokenBuffer;

	Collection<Hhmm::SegTokenPtr>::iterator _tokenIter;

	const CharTermAttributePtr _termAtt;
	const OffsetAttributePtr _offsetAtt;
	const TypeAttributePtr _typeAtt;

	int _tokStart = 0; // only used if the length changed before this filter
	int _tokEnd = 0; // only used if the length changed before this filter
	bool _hasIllegalOffsets = false; // only if the length changed before this filter

public:
	LUCENE_CLASS(WordTokenFilter);

	/// <summary>
	/// Construct a new WordTokenizer.
	/// </summary>
	/// <param name="in"> <seealso cref="TokenStream"/> of sentences  </param>
	WordTokenFilter(const TokenStreamPtr& input);

	virtual bool incrementToken();

	virtual void reset();
};
}
}
}
}
