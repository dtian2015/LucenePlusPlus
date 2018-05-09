#pragma once

#include "LuceneObject.h"
#include "smartcn/CnTypes.h"

namespace Lucene {
namespace Analysis {
namespace Cn {
namespace Smart {
namespace Hhmm {

/// <summary>
/// Finds the optimal segmentation of a sentence into Chinese words
/// @lucene.experimental
/// </summary>
class LPPCONTRIBAPI HHMMSegmenter : public LuceneObject
{
private:
	static WordDictionaryPtr _wordDict;

	/// <summary>
	/// Create the <seealso cref="SegGraph"/> for a sentence.
	/// </summary>
	/// <param name="sentence"> input sentence, without start and end markers </param>
	/// <returns> <seealso cref="SegGraph"/> corresponding to the input sentence. </returns>
	SegGraphPtr createSegGraph(const String& sentence);

	/// <summary>
	/// Get the character types for every character in a sentence.
	/// </summary>
	/// <seealso cref= Utility#getCharType(char) </seealso>
	/// <param name="sentence"> input sentence </param>
	/// <returns> array of character types corresponding to character positions in the sentence </returns>
	static std::vector<int> getCharTypes(const String& sentence);

public:
	LUCENE_CLASS(HHMMSegmenter);

	HHMMSegmenter();

	/// <summary>
	/// Return a list of <seealso cref="SegToken"/> representing the best segmentation of a sentence </summary>
	/// <param name="sentence"> input sentence </param>
	/// <returns> best segmentation as a <seealso cref="List"/> </returns>
	virtual Collection<SegTokenPtr> process(const String& sentence);
};
}
}
}
}
}
