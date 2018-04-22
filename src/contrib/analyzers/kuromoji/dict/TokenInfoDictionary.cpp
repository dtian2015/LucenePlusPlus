#include "ContribInc.h"

#include "kuromoji/dict/TokenInfoDictionary.h"

#include "CodecUtil.h"
#include "FSDirectory.h"
#include "FileUtils.h"
#include "PositiveIntOutputs.h"
#include "_SimpleFSDirectory.h"

namespace Lucene {
namespace Analysis {
namespace Ja {
namespace Dict {

const String TokenInfoDictionary::FST_FILENAME_SUFFIX = L"_fst.dat";

TokenInfoDictionary::TokenInfoDictionary()
{
	initializeDictionaries();
	LuceneException priorE;

	const String fstFile = FileUtils::joinPath(DICT_PATH, getClassName() + FST_FILENAME_SUFFIX);
	DataInputPtr is = newLucene<SimpleFSIndexInput>(fstFile, BufferedIndexInput::BUFFER_SIZE, FSDirectory::DEFAULT_READ_CHUNK_SIZE);
	boost::shared_ptr<FSTLong> fst = nullptr;

	try
	{
		fst = newLucene<FSTLong>(is, Util::FST::PositiveIntOutputs::getSingleton(true));
	}
	catch (const IOException& ioe)
	{
		priorE = ioe;
	}

	if (is)
	{
		is->close();
	}

	// TODO: some way to configure?
	this->_fst = newLucene<TokenInfoFST>(fst, true);

	priorE.throwException();
}

TokenInfoFSTPtr TokenInfoDictionary::getFST()
{
	return _fst;
}

const TokenInfoDictionaryPtr& TokenInfoDictionary::getInstance()
{
	static TokenInfoDictionaryPtr INSTANCE(new TokenInfoDictionary());
	return INSTANCE;
}
}
}
}
}
