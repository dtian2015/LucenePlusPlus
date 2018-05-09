#pragma once

#include "LuceneObject.h"
#include "smartcn/CnTypes.h"

namespace Lucene {
namespace Analysis {
namespace Cn {
namespace Smart {

/// <summary>
/// Segment a sentence of Chinese text into words.
/// @lucene.experimental
/// </summary>
class LPPCONTRIBAPI WordSegmenter : public LuceneObject
{
private:
	Hhmm::HHMMSegmenterPtr _hhmmSegmenter;

	Hhmm::SegTokenFilterPtr _tokenFilter;

public:
	LUCENE_CLASS(WordSegmenter);

	WordSegmenter();

	/// <summary>
	/// Segment a sentence into words with <seealso cref="HHMMSegmenter"/>
	/// </summary>
	/// <param name="sentence"> input sentence </param>
	/// <param name="startOffset"> start offset of sentence </param>
	/// <returns> <seealso cref="List"/> of <seealso cref="SegToken"/> </returns>
	virtual Collection<Hhmm::SegTokenPtr> segmentSentence(const String& sentence, int startOffset);

	/// <summary>
	/// Process a <seealso cref="SegToken"/> so that it is ready for indexing.
	///
	/// This method calculates offsets and normalizes the token with <seealso cref="SegTokenFilter"/>.
	/// </summary>
	/// <param name="st"> input <seealso cref="SegToken"/> </param>
	/// <param name="sentence"> associated Sentence </param>
	/// <param name="sentenceStartOffset"> offset into sentence </param>
	/// <returns> Lucene <seealso cref="SegToken"/> </returns>
	virtual Hhmm::SegTokenPtr convertSegToken(Hhmm::SegTokenPtr st, const String& sentence, int sentenceStartOffset);
};
}
}
}
}
