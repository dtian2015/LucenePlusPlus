#pragma once

#include "DataInput.h"
#include "DataOutput.h"

namespace Lucene {
namespace Util {

/// <summary>
/// Utility class for reading and writing versioned headers.
/// This is useful to ensure that a file is in the format
/// you think it is.
/// @lucene.experimental
/// </summary>
class LPPAPI CodecUtil final
{
private:
	CodecUtil();

	static constexpr int CODEC_MAGIC = 0x3fd76c17;

public:
	static boost::shared_ptr<DataOutput> writeHeader(boost::shared_ptr<DataOutput> out, const String& codec, int version);

	static int headerLength(const String& codec);

	static int checkHeader(DataInput& in, const String& codec, int minVersion, int maxVersion);
};
}
}
