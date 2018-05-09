#include "ContribInc.h"

#include "CodecUtil.h"
#include "FSDirectory.h"
#include "FileIndexInputOutput.h"
#include "FileUtils.h"
#include "kuromoji/dict/ConnectionCosts.h"

namespace Lucene {
namespace Analysis {
namespace Ja {
namespace Dict {

const String ConnectionCosts::FILENAME = L"ConnectionCosts.dat";
const String ConnectionCosts::HEADER = L"kuromoji_cc";
String ConnectionCosts::DICT_PATH;

ConnectionCosts::ConnectionCosts()
{
	DICT_PATH = MiscUtils::GetAsianDictionaryPath();
	LuceneException priorE;
	std::vector<std::vector<short>> costs;

	const String costFile = FileUtils::joinPath(DICT_PATH, FILENAME);
	DataInputPtr is = newLucene<SimpleFSIndexInput>(costFile, BufferedIndexInput::BUFFER_SIZE, FSDirectory::DEFAULT_READ_CHUNK_SIZE);

	try
	{
		Util::CodecUtil::checkHeader(*is, HEADER, VERSION, VERSION);

		int forwardSize = is->readVInt();
		int backwardSize = is->readVInt();

		costs.resize(backwardSize);
		for (auto& element : costs)
		{
			element.resize(forwardSize);
		}

		int accum = 0;
		for (int j = 0; j < costs.size(); j++)
		{
			std::vector<short>& a = costs[j];
			for (int i = 0; i < a.size(); i++)
			{
				int raw = is->readVInt();
				accum += (static_cast<int>(static_cast<unsigned int>(raw) >> 1)) ^ -(raw & 1);
				a[i] = static_cast<short>(accum);
			}
		}
	}
	catch (const IOException& ioe)
	{
		priorE = ioe;
	}

	if (is)
	{
		is->close();
	}

	this->_costs = costs;
	priorE.throwException();
}

int ConnectionCosts::get(int forwardId, int backwardId)
{
	return _costs[backwardId][forwardId];
}

const ConnectionCostsPtr& ConnectionCosts::getInstance()
{
	static ConnectionCostsPtr INSTANCE(new ConnectionCosts());
	return INSTANCE;
}
}
}
}
}
