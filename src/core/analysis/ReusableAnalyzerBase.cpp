#include "LuceneInc.h"

#include "ReusableAnalyzerBase.h"
#include "Tokenizer.h"

namespace Lucene {

ReusableAnalyzerBase::~ReusableAnalyzerBase()
{
}

TokenStreamPtr ReusableAnalyzerBase::reusableTokenStream(const String& fieldName, const ReaderPtr& reader)
{
	TokenStreamComponentsPtr streamChain = boost::static_pointer_cast<TokenStreamComponents>(getPreviousTokenStream());

	auto const r = initReader(reader);
	if (streamChain == nullptr || !streamChain->reset(r))
	{
		streamChain = createComponents(fieldName, r);
		setPreviousTokenStream(streamChain);
	}
	return streamChain->getTokenStream();
}

TokenStreamPtr ReusableAnalyzerBase::tokenStream(const String& fieldName, const ReaderPtr& reader)
{
	return createComponents(fieldName, initReader(reader))->getTokenStream();
}

ReaderPtr ReusableAnalyzerBase::initReader(ReaderPtr reader)
{
	return reader;
}

#pragma mark - TokenStreamComponents

ReusableAnalyzerBase::TokenStreamComponents::TokenStreamComponents(TokenizerPtr source, TokenStreamPtr result)
	: source(source), sink(result)
{
}

ReusableAnalyzerBase::TokenStreamComponents::TokenStreamComponents(TokenizerPtr source)
	: source(source), sink(boost::static_pointer_cast<TokenStream>(source))
{
}

bool ReusableAnalyzerBase::TokenStreamComponents::reset(ReaderPtr reader)
{
	source->reset(reader);
	return true;
}

TokenStreamPtr ReusableAnalyzerBase::TokenStreamComponents::getTokenStream()
{
	return sink;
}
}
