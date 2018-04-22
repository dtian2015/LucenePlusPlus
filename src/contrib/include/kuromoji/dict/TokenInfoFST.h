#pragma once

#include "FST.h"
#include "kuromoji/JaTypes.h"

namespace Lucene {
namespace Analysis {
namespace Ja {
namespace Dict {

/// <summary>
/// Thin wrapper around an FST with root-arc caching for Japanese.
/// <para>
/// Depending upon fasterButMoreRam, either just kana (191 arcs),
/// or kana and han (28,607 arcs) are cached. The latter offers
/// additional performance at the cost of more RAM.
/// </para>
/// </summary>
class LPPCONTRIBAPI TokenInfoFST final : public LuceneObject
{
private:
	const boost::shared_ptr<FSTLong> _fst;

	// depending upon fasterButMoreRam, we cache root arcs for either
	// kana (0x3040-0x30FF) or kana + han (0x3040-0x9FFF)
	// false: 191 arcs
	// true:  28,607 arcs (costs ~1.5MB)
	const int _cacheCeiling;
	std::vector<boost::shared_ptr<FSTArcLong>> const _rootCache;

public:
	LUCENE_CLASS(TokenInfoFST);
	const Long NO_OUTPUT;

	TokenInfoFST(boost::shared_ptr<FSTLong> fst, bool fasterButMoreRam);

private:
	std::vector<boost::shared_ptr<FSTArcLong>> cacheRootArcs();

public:
	boost::shared_ptr<FSTArcLong> findTargetArc(
		int ch,
		boost::shared_ptr<FSTArcLong> follow,
		boost::shared_ptr<FSTArcLong> arc,
		bool useCache,
		boost::shared_ptr<FSTLong::BytesReader> fstReader);

	boost::shared_ptr<FSTArcLong> getFirstArc(boost::shared_ptr<FSTArcLong> arc);

	FSTLong::BytesReaderPtr getBytesReader(int pos);

	/// <summary>
	/// @lucene.internal for testing only </summary>
	boost::shared_ptr<FSTLong> getInternalFST();
};
}
}
}
}
