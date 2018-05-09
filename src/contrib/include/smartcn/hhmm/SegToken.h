#pragma once

#include "LuceneObject.h"
#include "smartcn/CnTypes.h"
#include "smartcn/WordType.h"

#include <memory>
#include <vector>

namespace Lucene {
namespace Analysis {
namespace Cn {
namespace Smart {
namespace Hhmm {

/// <summary>
/// SmartChineseAnalyzer internal token
/// @lucene.experimental
/// </summary>
class LPPCONTRIBAPI SegToken : public LuceneObject
{
public:
	LUCENE_CLASS(SegToken);

	/// <summary>
	/// Character array containing token text
	/// </summary>
	CharArray charArray;

	/// <summary>
	/// start offset into original sentence
	/// </summary>
	int startOffset = 0;

	/// <summary>
	/// end offset into original sentence
	/// </summary>
	int endOffset = 0;

	/// <summary>
	/// <seealso cref="WordType"/> of the text
	/// </summary>
	WordType wordType = WordType::SENTENCE_BEGIN;

	/// <summary>
	/// word frequency
	/// </summary>
	int weight = 0;

	/// <summary>
	/// during segmentation, this is used to store the index of the token in the token list table
	/// </summary>
	int index = 0;

	/// <summary>
	/// Create a new SegToken from a character array.
	/// </summary>
	/// <param name="idArray"> character array containing text </param>
	/// <param name="start"> start offset of SegToken in original sentence </param>
	/// <param name="end"> end offset of SegToken in original sentence </param>
	/// <param name="wordType"> <seealso cref="WordType"/> of the text </param>
	/// <param name="weight"> word frequency </param>
	SegToken(const CharArray& idArray, int start, int end, WordType wordType, int weight);

	virtual int32_t hashCode();

	virtual bool equals(const LuceneObjectPtr& other);
};
}
}
}
}
}
