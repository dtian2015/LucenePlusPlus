#include "ContribInc.h"

#include "kuromoji/dict/TokenInfoFST.h"

namespace Lucene {
namespace Analysis {
namespace Ja {
namespace Dict {

TokenInfoFST::TokenInfoFST(boost::shared_ptr<FSTLong> fst, bool fasterButMoreRam)
	: _fst(fst), _cacheCeiling(fasterButMoreRam ? 0x9FFF : 0x30FF), _rootCache(cacheRootArcs()), NO_OUTPUT(fst->outputs->getNoOutput())
{
}

std::vector<boost::shared_ptr<FSTArcLong>> TokenInfoFST::cacheRootArcs()
{
	std::vector<boost::shared_ptr<FSTArcLong>> rootCache = std::vector<boost::shared_ptr<FSTArcLong>>(1 + (_cacheCeiling - 0x3040));
	boost::shared_ptr<FSTArcLong> firstArc = boost::make_shared<FSTArcLong>();
	_fst->getFirstArc(firstArc);

	boost::shared_ptr<FSTArcLong> arc = boost::make_shared<FSTArcLong>();
	auto const fstReader = _fst->getBytesReader(0);

	// TODO: jump to 3040, readNextRealArc to ceiling? (just be careful we don't add bugs)
	for (int i = 0; i < rootCache.size(); i++)
	{
		if (_fst->findTargetArc(0x3040 + i, firstArc, arc, fstReader) != nullptr)
		{
			rootCache[i] = (boost::make_shared<FSTArcLong>())->copyFrom(arc);
		}
	}
	return rootCache;
}

boost::shared_ptr<FSTArcLong> TokenInfoFST::findTargetArc(
	int ch,
	boost::shared_ptr<FSTArcLong> follow,
	boost::shared_ptr<FSTArcLong> arc,
	bool useCache,
	boost::shared_ptr<FSTLong::BytesReader> fstReader)
{
	if (useCache && ch >= 0x3040 && ch <= _cacheCeiling)
	{
		BOOST_ASSERT(ch != FSTLong::END_LABEL);
		const auto& result = _rootCache[ch - 0x3040];

		if (result == nullptr)
		{
			return nullptr;
		}
		else
		{
			arc->copyFrom(result);
			return arc;
		}
	}
	else
	{
		return _fst->findTargetArc(ch, follow, arc, fstReader);
	}
}

boost::shared_ptr<FSTArcLong> TokenInfoFST::getFirstArc(boost::shared_ptr<FSTArcLong> arc)
{
	return _fst->getFirstArc(arc);
}

FSTLong::BytesReaderPtr TokenInfoFST::getBytesReader(int pos)
{
	return _fst->getBytesReader(pos);
}

boost::shared_ptr<FSTLong> TokenInfoFST::getInternalFST()
{
	return _fst;
}
}
}
}
}
