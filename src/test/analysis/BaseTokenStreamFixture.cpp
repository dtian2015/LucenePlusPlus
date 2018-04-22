/////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2009-2014 Alan Wright. All rights reserved.
// Distributable under the terms of either the Apache License (Version 2.0)
// or the GNU Lesser General Public License.
/////////////////////////////////////////////////////////////////////////////
#include "TestInc.h"

#include "Analyzer.h"
#include "BaseTokenStreamFixture.h"
#include "CharTermAttribute.h"
#include "FileReader.h"
#include "FileUtils.h"
#include "InputStreamReader.h"
#include "OffsetAttribute.h"
#include "PositionIncrementAttribute.h"
#include "PositionLengthAttribute.h"
#include "StringReader.h"
#include "TermAttribute.h"
#include "TestUtils.h"
#include "TokenStream.h"
#include "TypeAttribute.h"
#include "kuromoji/dict/UserDictionary.h"

using namespace Lucene::Analysis::Ja::Dict;

namespace Lucene {

CheckClearAttributesAttribute::CheckClearAttributesAttribute()
{
	clearCalled = false;
}

CheckClearAttributesAttribute::~CheckClearAttributesAttribute()
{
}

bool CheckClearAttributesAttribute::getAndResetClearCalled()
{
	bool _clearCalled = clearCalled;
	clearCalled = false;
	return _clearCalled;
}

void CheckClearAttributesAttribute::clear()
{
	clearCalled = true;
}

bool CheckClearAttributesAttribute::equals(const LuceneObjectPtr& other)
{
	if (Attribute::equals(other))
	{
		return true;
	}

	CheckClearAttributesAttributePtr otherAttribute(boost::dynamic_pointer_cast<CheckClearAttributesAttribute>(other));
	if (otherAttribute)
	{
		return (otherAttribute->clearCalled == clearCalled);
	}

	return false;
}

int32_t CheckClearAttributesAttribute::hashCode()
{
	return 76137213 ^ (clearCalled ? 1231 : 1237);
}

void CheckClearAttributesAttribute::copyTo(const AttributePtr& target)
{
	CheckClearAttributesAttributePtr clearAttribute(boost::dynamic_pointer_cast<CheckClearAttributesAttribute>(target));
	clearAttribute->clear();
}

LuceneObjectPtr CheckClearAttributesAttribute::clone(const LuceneObjectPtr& other)
{
	LuceneObjectPtr clone = other ? other : newLucene<CheckClearAttributesAttribute>();
	CheckClearAttributesAttributePtr cloneAttribute(boost::dynamic_pointer_cast<CheckClearAttributesAttribute>(Attribute::clone(clone)));
	cloneAttribute->clearCalled = clearCalled;
	return cloneAttribute;
}

BaseTokenStreamFixture::~BaseTokenStreamFixture()
{
}

void BaseTokenStreamFixture::checkTokenStreamContents(
	const TokenStreamPtr& ts,
	Collection<String> output,
	Collection<int32_t> startOffsets,
	Collection<int32_t> endOffsets,
	Collection<String> types,
	Collection<int32_t> posIncrements,
	int32_t finalOffset,
	Collection<int32_t> posLengths)
{
	EXPECT_TRUE(output);
	CheckClearAttributesAttributePtr checkClearAtt = ts->addAttribute<CheckClearAttributesAttribute>();

	bool hasTermAttribute = ts->hasAttribute<TermAttribute>();
	bool hasCharTermAttribute = ts->hasAttribute<CharTermAttribute>();

	if (!hasCharTermAttribute)
	{
		EXPECT_TRUE(hasTermAttribute);
	}

	TermAttributePtr termAtt = hasCharTermAttribute ? ts->getAttribute<CharTermAttribute>() : ts->getAttribute<TermAttribute>();

	OffsetAttributePtr offsetAtt;
	if (startOffsets || endOffsets || finalOffset != -1)
	{
		EXPECT_TRUE(ts->hasAttribute<OffsetAttribute>());
		offsetAtt = ts->getAttribute<OffsetAttribute>();
	}

	TypeAttributePtr typeAtt;
	if (types)
	{
		EXPECT_TRUE(ts->hasAttribute<TypeAttribute>());
		typeAtt = ts->getAttribute<TypeAttribute>();
	}

	PositionIncrementAttributePtr posIncrAtt;
	if (posIncrements)
	{
		EXPECT_TRUE(ts->hasAttribute<PositionIncrementAttribute>());
		posIncrAtt = ts->getAttribute<PositionIncrementAttribute>();
	}

	PositionLengthAttributePtr posLengthAtt;
	if (posLengths)
	{
		EXPECT_TRUE(ts->hasAttribute<PositionLengthAttribute>());
		posLengthAtt = ts->getAttribute<PositionLengthAttribute>();
	}

	ts->reset();
	for (int32_t i = 0; i < output.size(); ++i)
	{
		// extra safety to enforce, that the state is not preserved and also assign bogus values
		ts->clearAttributes();
		termAtt->setTermBuffer(L"bogusTerm");
		if (offsetAtt)
		{
			offsetAtt->setOffset(14584724, 24683243);
		}
		if (typeAtt)
		{
			typeAtt->setType(L"bogusType");
		}
		if (posIncrAtt)
		{
			posIncrAtt->setPositionIncrement(45987657);
		}
		if (posLengthAtt)
		{
			posLengthAtt->setPositionLength(45987653);
		}

		checkClearAtt->getAndResetClearCalled(); // reset it, because we called clearAttribute() before
		EXPECT_TRUE(ts->incrementToken());
		EXPECT_TRUE(checkClearAtt->getAndResetClearCalled());

		EXPECT_EQ(output[i], termAtt->term());
		if (startOffsets)
		{
			EXPECT_EQ(startOffsets[i], offsetAtt->startOffset());
		}
		if (endOffsets)
		{
			EXPECT_EQ(endOffsets[i], offsetAtt->endOffset());
		}
		if (types)
		{
			EXPECT_EQ(types[i], typeAtt->type());
		}
		if (posIncrements)
		{
			EXPECT_EQ(posIncrements[i], posIncrAtt->getPositionIncrement());
		}
		if (posLengths)
		{
			EXPECT_EQ(posLengths[i], posLengthAtt->getPositionLength());
		}

		if (offsetAtt)
		{
			EXPECT_TRUE(offsetAtt->startOffset() >= 0);
			EXPECT_TRUE(offsetAtt->endOffset() >= 0);
			EXPECT_TRUE(offsetAtt->endOffset() >= offsetAtt->startOffset());
		}

		if (posIncrAtt)
		{
			if (i == 0)
			{
				EXPECT_TRUE(posIncrAtt->getPositionIncrement() >= 1);
			}
			else
			{
				EXPECT_TRUE(posIncrAtt->getPositionIncrement() >= 0);
			}
		}

		if (posLengthAtt)
		{
			EXPECT_TRUE(posLengthAtt->getPositionLength() >= 1);
		}
	}
	EXPECT_TRUE(!ts->incrementToken());
	ts->end();
	if (finalOffset != -1)
	{
		EXPECT_EQ(finalOffset, offsetAtt->endOffset());
	}
	ts->close();
}

void BaseTokenStreamFixture::checkTokenStreamContents(const TokenStreamPtr& ts, Collection<String> output)
{
	checkTokenStreamContents(ts, output, Collection<int32_t>(), Collection<int32_t>(), Collection<String>(), Collection<int32_t>(), -1);
}

void BaseTokenStreamFixture::checkTokenStreamContents(const TokenStreamPtr& ts, Collection<String> output, Collection<String> types)
{
	checkTokenStreamContents(ts, output, Collection<int32_t>(), Collection<int32_t>(), types, Collection<int32_t>(), -1);
}

void BaseTokenStreamFixture::checkTokenStreamContents(
	const TokenStreamPtr& ts, Collection<String> output, Collection<int32_t> posIncrements)
{
	checkTokenStreamContents(ts, output, Collection<int32_t>(), Collection<int32_t>(), Collection<String>(), posIncrements, -1);
}

void BaseTokenStreamFixture::checkTokenStreamContents(
	const TokenStreamPtr& ts, Collection<String> output, Collection<int32_t> startOffsets, Collection<int32_t> endOffsets)
{
	checkTokenStreamContents(ts, output, startOffsets, endOffsets, Collection<String>(), Collection<int32_t>(), -1);
}

void BaseTokenStreamFixture::checkTokenStreamContents(
	const TokenStreamPtr& ts,
	Collection<String> output,
	Collection<int32_t> startOffsets,
	Collection<int32_t> endOffsets,
	int32_t finalOffset)
{
	checkTokenStreamContents(ts, output, startOffsets, endOffsets, Collection<String>(), Collection<int32_t>(), finalOffset);
}

void BaseTokenStreamFixture::checkTokenStreamContents(
	const TokenStreamPtr& ts,
	Collection<String> output,
	Collection<int32_t> startOffsets,
	Collection<int32_t> endOffsets,
	Collection<int32_t> posIncrements)
{
	checkTokenStreamContents(ts, output, startOffsets, endOffsets, Collection<String>(), posIncrements, -1);
}

void BaseTokenStreamFixture::checkTokenStreamContents(
	const TokenStreamPtr& ts,
	Collection<String> output,
	Collection<int32_t> startOffsets,
	Collection<int32_t> endOffsets,
	Collection<int32_t> posIncrements,
	int32_t finalOffset)
{
	checkTokenStreamContents(ts, output, startOffsets, endOffsets, Collection<String>(), posIncrements, finalOffset);
}

void BaseTokenStreamFixture::checkAnalyzesTo(
	const AnalyzerPtr& analyzer,
	const String& input,
	Collection<String> output,
	Collection<int32_t> startOffsets,
	Collection<int32_t> endOffsets,
	Collection<String> types,
	Collection<int32_t> posIncrements)
{
	checkTokenStreamContents(
		analyzer->tokenStream(L"dummy", newLucene<StringReader>(input)), output, startOffsets, endOffsets, types, posIncrements,
		(int32_t)input.length());
}

void BaseTokenStreamFixture::checkAnalyzesTo(const AnalyzerPtr& analyzer, const String& input, Collection<String> output)
{
	checkAnalyzesTo(analyzer, input, output, Collection<int32_t>(), Collection<int32_t>(), Collection<String>(), Collection<int32_t>());
}

void BaseTokenStreamFixture::checkAnalyzesTo(
	const AnalyzerPtr& analyzer, const String& input, Collection<String> output, Collection<String> types)
{
	checkAnalyzesTo(analyzer, input, output, Collection<int32_t>(), Collection<int32_t>(), types, Collection<int32_t>());
}

void BaseTokenStreamFixture::checkAnalyzesTo(
	const AnalyzerPtr& analyzer, const String& input, Collection<String> output, Collection<int32_t> posIncrements)
{
	checkAnalyzesTo(analyzer, input, output, Collection<int32_t>(), Collection<int32_t>(), Collection<String>(), posIncrements);
}

void BaseTokenStreamFixture::checkAnalyzesTo(
	const AnalyzerPtr& analyzer,
	const String& input,
	Collection<String> output,
	Collection<int32_t> startOffsets,
	Collection<int32_t> endOffsets)
{
	checkAnalyzesTo(analyzer, input, output, startOffsets, endOffsets, Collection<String>(), Collection<int32_t>());
}

void BaseTokenStreamFixture::checkAnalyzesTo(
	const AnalyzerPtr& analyzer,
	const String& input,
	Collection<String> output,
	Collection<int32_t> startOffsets,
	Collection<int32_t> endOffsets,
	Collection<int32_t> posIncrements)
{
	checkAnalyzesTo(analyzer, input, output, startOffsets, endOffsets, Collection<String>(), posIncrements);
}

void BaseTokenStreamFixture::checkAnalyzesToPositions(
	const AnalyzerPtr& analyzer,
	const String& input,
	Collection<String> output,
	Collection<int32_t> posIncrements,
	Collection<int32_t> posLengths)
{
	checkTokenStreamContents(
		analyzer->tokenStream(L"dummy", newLucene<StringReader>(input)), output, Collection<int32_t>(), Collection<int32_t>(),
		Collection<String>(), posIncrements, static_cast<int32_t>(input.length()), posLengths);
}

void BaseTokenStreamFixture::checkAnalyzesToReuse(
	const AnalyzerPtr& analyzer,
	const String& input,
	Collection<String> output,
	Collection<int32_t> startOffsets,
	Collection<int32_t> endOffsets,
	Collection<String> types,
	Collection<int32_t> posIncrements)
{
	checkTokenStreamContents(
		analyzer->reusableTokenStream(L"dummy", newLucene<StringReader>(input)), output, startOffsets, endOffsets, types, posIncrements,
		(int32_t)input.length());
}

void BaseTokenStreamFixture::checkAnalyzesToReuse(const AnalyzerPtr& analyzer, const String& input, Collection<String> output)
{
	checkAnalyzesToReuse(
		analyzer, input, output, Collection<int32_t>(), Collection<int32_t>(), Collection<String>(), Collection<int32_t>());
}

void BaseTokenStreamFixture::checkAnalyzesToReuse(
	const AnalyzerPtr& analyzer, const String& input, Collection<String> output, Collection<String> types)
{
	checkAnalyzesToReuse(analyzer, input, output, Collection<int32_t>(), Collection<int32_t>(), types, Collection<int32_t>());
}

void BaseTokenStreamFixture::checkAnalyzesToReuse(
	const AnalyzerPtr& analyzer, const String& input, Collection<String> output, Collection<int32_t> posIncrements)
{
	checkAnalyzesToReuse(analyzer, input, output, Collection<int32_t>(), Collection<int32_t>(), Collection<String>(), posIncrements);
}

void BaseTokenStreamFixture::checkAnalyzesToReuse(
	const AnalyzerPtr& analyzer,
	const String& input,
	Collection<String> output,
	Collection<int32_t> startOffsets,
	Collection<int32_t> endOffsets)
{
	checkAnalyzesToReuse(analyzer, input, output, startOffsets, endOffsets, Collection<String>(), Collection<int32_t>());
}

void BaseTokenStreamFixture::checkAnalyzesToReuse(
	const AnalyzerPtr& analyzer,
	const String& input,
	Collection<String> output,
	Collection<int32_t> startOffsets,
	Collection<int32_t> endOffsets,
	Collection<int32_t> posIncrements)
{
	checkAnalyzesToReuse(analyzer, input, output, startOffsets, endOffsets, Collection<String>(), posIncrements);
}

void BaseTokenStreamFixture::checkOneTerm(const AnalyzerPtr& analyzer, const String& input, const String& expected)
{
	checkAnalyzesTo(analyzer, input, newCollection<String>(expected));
}

void BaseTokenStreamFixture::checkOneTermReuse(const AnalyzerPtr& analyzer, const String& input, const String& expected)
{
	checkAnalyzesToReuse(analyzer, input, newCollection<String>(expected));
}

UserDictionaryPtr BaseTokenStreamFixture::readDict()
{
	String dictFile = FileUtils::joinPath(getTestDir(), L"ja/userdict.txt");
	if (!FileUtils::fileExists(dictFile))
	{
		boost::throw_exception(RuntimeException(L"Cannot find userdict.txt in test classpath!"));
	}

	try
	{
		InputStreamReaderPtr is = newLucene<InputStreamReader>(newLucene<FileReader>(dictFile));
		return newLucene<UserDictionary>(is);
	}
	catch (const IOException& ioe)
	{
		boost::throw_exception(RuntimeException(ioe.getError()));
	}
}
}
