#pragma once

#include "LuceneObject.h"
#include "smartcn/CnTypes.h"

namespace Lucene {
namespace Analysis {
namespace Cn {
namespace Smart {
namespace Hhmm {

/// <summary>
/// Graph representing possible token pairs (bigrams) at each start offset in the sentence.
/// <para>
/// For each start offset, a list of possible token pairs is stored.
/// </para>
/// @lucene.experimental
/// </summary>
class LPPCONTRIBAPI BiSegGraph : public LuceneObject
{
private:
	HashMap<int, Collection<SegTokenPairPtr>> _tokenPairListTable = HashMap<int, Collection<SegTokenPairPtr>>::newInstance();

	Collection<SegTokenPtr> _segTokenList;

	static BigramDictionaryPtr _bigramDict;

public:
	LUCENE_CLASS(BiSegGraph);

	BiSegGraph(SegGraphPtr segGraph);

private:
	/*
	 * Generate a BiSegGraph based upon a SegGraph
	 */
	void generateBiSegGraph(SegGraphPtr segGraph);

public:
	/// <summary>
	/// Returns true if their is a list of token pairs at this offset (index of the second token)
	/// </summary>
	/// <param name="to"> index of the second token in the token pair </param>
	/// <returns> true if a token pair exists </returns>
	virtual bool isToExist(int to);

	/// <summary>
	/// Return a <seealso cref="List"/> of all token pairs at this offset (index of the second token)
	/// </summary>
	/// <param name="to"> index of the second token in the token pair </param>
	/// <returns> <seealso cref="List"/> of token pairs. </returns>
	virtual Collection<SegTokenPairPtr> getToList(int to);

	/// <summary>
	/// Add a <seealso cref="SegTokenPair"/>
	/// </summary>
	/// <param name="tokenPair"> <seealso cref="SegTokenPair"/> </param>
	virtual void addSegTokenPair(SegTokenPairPtr tokenPair);

	/// <summary>
	/// Get the number of <seealso cref="SegTokenPair"/> entries in the table. </summary>
	/// <returns> number of <seealso cref="SegTokenPair"/> entries </returns>
	virtual int getToCount();

	/// <summary>
	/// Find the shortest path with the Viterbi algorithm. </summary>
	/// <returns> <seealso cref="List"/> </returns>
	virtual Collection<SegTokenPtr> getShortPath();

	String toString();
};
}
}
}
}
}
