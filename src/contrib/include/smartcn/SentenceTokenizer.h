#pragma once

#include "LuceneContrib.h"

#include "Tokenizer.h"

namespace Lucene {
namespace Analysis {
namespace Cn {
namespace Smart {

/// <summary>
/// Tokenizes input text into sentences.
/// <para>
/// The output tokens can then be broken into words with <seealso cref="WordTokenFilter"/>
/// </para>
/// @lucene.experimental
/// </summary>
class LPPCONTRIBAPI SentenceTokenizer final : public Tokenizer
{
private:
	/// <summary>
	/// End of sentence punctuation: 。，！？；,!?;
	/// </summary>
	static const String PUNCTION;

	StringBufferPtr _buffer;

	int _tokenStart = 0;
	int _tokenEnd = 0;

	const CharTermAttributePtr _termAtt;
	const OffsetAttributePtr _offsetAtt;
	const TypeAttributePtr _typeAtt;

public:
	LUCENE_CLASS(SentenceTokenizer);

	SentenceTokenizer(const ReaderPtr& reader);

	SentenceTokenizer(const AttributeSourcePtr& source, const ReaderPtr& reader);

	SentenceTokenizer(const AttributeFactoryPtr& factory, const ReaderPtr& reader);

	virtual bool incrementToken();

	virtual void reset();

	virtual void reset(const ReaderPtr& input);

	virtual void end();
};
}
}
}
}
