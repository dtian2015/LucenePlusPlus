#pragma once

#include "LuceneObject.h"
#include "smartcn/CnTypes.h"

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace Lucene {
namespace Analysis {
namespace Cn {
namespace Smart {
namespace Hhmm {

/// <summary>
/// Graph representing possible tokens at each start offset in the sentence.
/// <para>
/// For each start offset, a list of possible tokens is stored.
/// </para>
/// @lucene.experimental
/// </summary>
class LPPCONTRIBAPI SegGraph : public LuceneObject
{
private:
	/// <summary>
	/// Map of start offsets to ArrayList of tokens at that position
	/// </summary>
	HashMap<int, Collection<SegTokenPtr>> _tokenListTable = HashMap<int, Collection<SegTokenPtr>>::newInstance();

	int _maxStart = -1;

public:
	LUCENE_CLASS(SegGraph);

	/// <summary>
	/// Returns true if a mapping for the specified start offset exists
	/// </summary>
	/// <param name="s"> startOffset </param>
	/// <returns> true if there are tokens for the startOffset </returns>
	virtual bool isStartExist(int s);

	/// <summary>
	/// Get the list of tokens at the specified start offset
	/// </summary>
	/// <param name="s"> startOffset </param>
	/// <returns> List of tokens at the specified start offset. </returns>
	virtual Collection<SegTokenPtr> getStartList(int s);

	/// <summary>
	/// Get the highest start offset in the map
	/// </summary>
	/// <returns> maximum start offset, or -1 if the map is empty. </returns>
	virtual int getMaxStart();

	/// <summary>
	/// Set the <seealso cref="SegToken#index"/> for each token, based upon its order by startOffset. </summary>
	/// <returns> a <seealso cref="List"/> of these ordered tokens. </returns>
	virtual Collection<SegTokenPtr> makeIndex();

	/// <summary>
	/// Add a <seealso cref="SegToken"/> to the mapping, creating a new mapping at the token's startOffset if one does not exist. </summary>
	/// <param name="token"> <seealso cref="SegToken"/> </param>
	virtual void addToken(SegTokenPtr token);

	/// <summary>
	/// Return a <seealso cref="List"/> of all tokens in the map, ordered by startOffset.
	/// </summary>
	/// <returns> <seealso cref="List"/> of all tokens in the map. </returns>
	virtual Collection<SegTokenPtr> toTokenList();

	String toString();

	// TODO: Daniel to remove
	String toDebugString();
};
}
}
}
}
}
