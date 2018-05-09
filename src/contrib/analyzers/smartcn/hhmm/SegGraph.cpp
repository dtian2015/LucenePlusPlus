#include "ContribInc.h"

#include "smartcn/hhmm/SegGraph.h"
#include "smartcn/hhmm/SegToken.h"

namespace Lucene {
namespace Analysis {
namespace Cn {
namespace Smart {
namespace Hhmm {

bool SegGraph::isStartExist(int s)
{
	if (_tokenListTable.contains(s))
	{
		return !_tokenListTable[s].empty();
	}

	return false;
}

Collection<SegTokenPtr> SegGraph::getStartList(int s)
{
	return _tokenListTable[s];
}

int SegGraph::getMaxStart()
{
	return _maxStart;
}

Collection<SegTokenPtr> SegGraph::makeIndex()
{
	Collection<SegTokenPtr> result = Collection<SegTokenPtr>::newInstance();
	int s = -1;
	int count = 0;
	int size = _tokenListTable.size();

	Collection<SegTokenPtr> tokenList;
	int index = 0;
	while (count < size)
	{
		if (isStartExist(s))
		{
			tokenList = _tokenListTable[s];
			for (auto st : tokenList)
			{
				st->index = index;
				result.add(st);
				index++;
			}
			count++;
		}
		s++;
	}
	return result;
}

void SegGraph::addToken(SegTokenPtr token)
{
	int s = token->startOffset;
	if (!isStartExist(s))
	{
		Collection<SegTokenPtr> newlist = Collection<SegTokenPtr>::newInstance();
		newlist.add(token);
		_tokenListTable.put(s, newlist);
	}
	else
	{
		Collection<SegTokenPtr> tokenList = _tokenListTable[s];
		tokenList.add(token);
	}

	if (s > _maxStart)
	{
		_maxStart = s;
	}
}

Collection<SegTokenPtr> SegGraph::toTokenList()
{
	Collection<SegTokenPtr> result = Collection<SegTokenPtr>::newInstance();

	int s = -1;
	int count = 0;
	int size = _tokenListTable.size();
	Collection<SegTokenPtr> tokenList;

	while (count < size)
	{
		if (isStartExist(s))
		{
			tokenList = _tokenListTable[s];
			for (auto st : tokenList)
			{
				result.add(st);
			}

			count++;
		}

		s++;
	}

	return result;
}

String SegGraph::toString()
{
	Collection<SegTokenPtr> tokenList = this->toTokenList();
	StringStream sb;

	for (auto t : tokenList)
	{
		sb << t->toString() << L"\n";
	}

	return sb.str();
}

// TODO: Daniel to remove
String SegGraph::toDebugString()
{
	StringStream buffer;

	for (auto iter = _tokenListTable.begin(); iter != _tokenListTable.end(); ++iter)
	{
		buffer << L"start = " << iter->first << ", tokens {" << L"\n";

		for (const auto& t : iter->second)
		{
			const String word(t->charArray.get(), t->charArray.size());
			buffer << L"[" << word << L"], start = " << t->startOffset << L", end = " << t->endOffset << L", index = " << t->index
				   << ", weight = " << t->weight << L"\n";
		}

		buffer << "}"
			   << "\n";
	}

	return buffer.str();
}
}
}
}
}
}
