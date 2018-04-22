#include "LuceneInc.h"

#include "FilteringTokenFilter.h"
#include "PositionIncrementAttribute.h"

namespace Lucene {

FilteringTokenFilter::FilteringTokenFilter(bool enablePositionIncrements, TokenStreamPtr input)
	: TokenFilter(input), _posIncrAtt(addAttribute<PositionIncrementAttribute>()), _enablePositionIncrements(enablePositionIncrements)
{
}

bool FilteringTokenFilter::incrementToken()
{
	if (_enablePositionIncrements)
	{
		int skippedPositions = 0;
		while (input->incrementToken())
		{
			if (accept())
			{
				if (skippedPositions != 0)
				{
					_posIncrAtt->setPositionIncrement(_posIncrAtt->getPositionIncrement() + skippedPositions);
				}

				return true;
			}

			skippedPositions += _posIncrAtt->getPositionIncrement();
		}
	}
	else
	{
		while (input->incrementToken())
		{
			if (accept())
			{
				if (_first)
				{
					// first token having posinc=0 is illegal.
					if (_posIncrAtt->getPositionIncrement() == 0)
					{
						_posIncrAtt->setPositionIncrement(1);
					}

					_first = false;
				}

				return true;
			}
		}
	}

	// reached EOS -- return false
	return false;
}

void FilteringTokenFilter::reset()
{
	TokenFilter::reset();
	_first = true;
}

bool FilteringTokenFilter::getEnablePositionIncrements()
{
	return _enablePositionIncrements;
}

void FilteringTokenFilter::setEnablePositionIncrements(bool enable)
{
	_enablePositionIncrements = enable;
}
}
