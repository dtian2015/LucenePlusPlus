#include "CodecUtil.h"
#include "StringUtils.h"

namespace Lucene {
namespace Util {

CodecUtil::CodecUtil()
{
} // no instance

boost::shared_ptr<DataOutput> CodecUtil::writeHeader(boost::shared_ptr<DataOutput> out, const String& codec, int version)
{
	if (codec.length() >= 128)
	{
		boost::throw_exception(
			IllegalArgumentException(L"codec must be simple ASCII, less than 128 characters in length [got " + codec + L"]"));
	}
	out->writeInt(CODEC_MAGIC);
	out->writeString(codec);
	out->writeInt(version);

	return out;
}

int CodecUtil::headerLength(const String& codec)
{
	return 9 + codec.length();
}

int CodecUtil::checkHeader(DataInput& in, const String& codec, int minVersion, int maxVersion)
{
	// Safety to guard against reading a bogus string:
	const int actualHeader = in.readInt();
	if (actualHeader != CODEC_MAGIC)
	{
		boost::throw_exception(CorruptIndexException(
			L"codec header mismatch: actual header=" + std::to_wstring(actualHeader) + L" vs expected header=" +
			std::to_wstring(CODEC_MAGIC)));
	}

	const String actualCodec = in.readString();
	if (actualCodec != codec)
	{
		boost::throw_exception(CorruptIndexException(L"codec mismatch: actual codec=" + actualCodec + L" vs expected codec=" + codec));
	}

	const int actualVersion = in.readInt();
	if (actualVersion < minVersion)
	{
		boost::throw_exception(CorruptIndexException(
			L"index format too old. actual version=" + StringUtils::toString(actualVersion) + L" vs minVersion=" +
			StringUtils::toString(minVersion)));
	}
	if (actualVersion > maxVersion)
	{
		boost::throw_exception(CorruptIndexException(
			L"index format too new. actual version=" + StringUtils::toString(actualVersion) + L" vs maxVersion=" +
			StringUtils::toString(maxVersion)));
	}

	return actualVersion;
}
}
}
