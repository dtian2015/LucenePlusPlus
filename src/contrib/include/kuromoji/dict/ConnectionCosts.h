#pragma once

#include "LuceneObject.h"
#include "kuromoji/JaTypes.h"

namespace Lucene {
namespace Analysis {
namespace Ja {
namespace Dict {

/// <summary>
/// n-gram connection cost data
/// </summary>
class LPPCONTRIBAPI ConnectionCosts final : public LuceneObject
{
public:
	LUCENE_CLASS(ConnectionCosts);

	static const String FILENAME;
	static const String HEADER;
	static constexpr int VERSION = 1;

private:
	// array is backward IDs first since get is called using the same backward ID consecutively. maybe doesn't matter.
	std::vector<std::vector<short>> _costs;

	ConnectionCosts();

	static String DICT_PATH;

public:
	int get(int forwardId, int backwardId);

	static const ConnectionCostsPtr& getInstance();
};
}
}
}
}
