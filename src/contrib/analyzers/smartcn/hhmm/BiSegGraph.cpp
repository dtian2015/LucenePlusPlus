#include "ContribInc.h"

#include "smartcn/hhmm/BiSegGraph.h"

#include "MiscUtils.h"
#include "smartcn/Utility.h"
#include "smartcn/hhmm/BigramDictionary.h"
#include "smartcn/hhmm/PathNode.h"
#include "smartcn/hhmm/SegGraph.h"
#include "smartcn/hhmm/SegToken.h"
#include "smartcn/hhmm/SegTokenPair.h"

namespace Lucene {
namespace Analysis {
namespace Cn {
namespace Smart {
namespace Hhmm {

BigramDictionaryPtr BiSegGraph::_bigramDict;

BiSegGraph::BiSegGraph(SegGraphPtr segGraph)
{
	if (!_bigramDict)
	{
		_bigramDict = BigramDictionary::getInstance();
	}

	_segTokenList = segGraph->makeIndex();
	generateBiSegGraph(segGraph);
}

void BiSegGraph::generateBiSegGraph(SegGraphPtr segGraph)
{
	double smooth = 0.1;
	int wordPairFreq = 0;
	int maxStart = segGraph->getMaxStart();
	double oneWordFreq;
	double weight;
	double tinyDouble = 1.0 / Utility::MAX_FREQUENCE;

	int next;
	CharArray idBuffer;

	// get the list of tokens ordered and indexed
	_segTokenList = segGraph->makeIndex();

	// Because the beginning position of startToken is -1, therefore startToken can be obtained when key = -1
	int key = -1;
	Collection<SegTokenPtr> nextTokens;

	while (key < maxStart)
	{
		if (segGraph->isStartExist(key))
		{
			Collection<SegTokenPtr> tokenList = segGraph->getStartList(key);

			// Calculate all tokens for a given key.
			for (auto t1 : tokenList)
			{
				oneWordFreq = t1->weight;
				next = t1->endOffset;
				nextTokens = Collection<SegTokenPtr>();

				// Find the next corresponding Token.
				// For example: "Sunny seashore", the present Token is "sunny", next one should be "sea" or "seashore".
				// If we cannot find the next Token, then go to the end and repeat the same cycle.
				while (next <= maxStart)
				{
					// Because the beginning position of endToken is sentenceLen, so equal to sentenceLen can find endToken.
					if (segGraph->isStartExist(next))
					{
						nextTokens = segGraph->getStartList(next);
						break;
					}
					next++;
				}
				if (!nextTokens)
				{
					break;
				}
				for (auto t2 : nextTokens)
				{
					idBuffer = CharArray::newInstance(t1->charArray.size() + t2->charArray.size() + 1);
					MiscUtils::arrayCopy(t1->charArray.get(), 0, idBuffer.get(), 0, t1->charArray.size());
					idBuffer[t1->charArray.size()] = BigramDictionary::WORD_SEGMENT_CHAR;

					MiscUtils::arrayCopy(t2->charArray.get(), 0, idBuffer.get(), t1->charArray.size() + 1, t2->charArray.size());

					// Two linked Words frequency
					wordPairFreq = _bigramDict->getFrequency(idBuffer);

					// Smoothing

					// -log{a*P(Ci-1)+(1-a)P(Ci|Ci-1)} Note 0<a<1
					weight = -std::log(
						smooth * (1.0 + oneWordFreq) / (Utility::MAX_FREQUENCE + 0.0) +
						(1.0 - smooth) * ((1.0 - tinyDouble) * wordPairFreq / (1.0 + oneWordFreq) + tinyDouble));

					SegTokenPairPtr tokenPair = newLucene<SegTokenPair>(idBuffer, t1->index, t2->index, weight);

					// TODO: Daniel to remove
					std::wcout << *tokenPair << std::endl;

					this->addSegTokenPair(tokenPair);
				}
			}
		}
		key++;
	}
}

bool BiSegGraph::isToExist(int to)
{
	if (_tokenPairListTable.contains(to))
	{
		return _tokenPairListTable[to].size() > 0;
	}

	return false;
}

Collection<SegTokenPairPtr> BiSegGraph::getToList(int to)
{
	return _tokenPairListTable[to];
}

void BiSegGraph::addSegTokenPair(SegTokenPairPtr tokenPair)
{
	int to = tokenPair->to;
	if (!isToExist(to))
	{
		_tokenPairListTable.put(to, newCollection(tokenPair));
	}
	else
	{
		auto& tokenPairList = _tokenPairListTable[to];
		tokenPairList.add(tokenPair);
	}
}

int BiSegGraph::getToCount()
{
	return _tokenPairListTable.size();
}

Collection<SegTokenPtr> BiSegGraph::getShortPath()
{
	int current;
	int nodeCount = getToCount();
	Collection<PathNodePtr> path = Collection<PathNodePtr>::newInstance();

	PathNodePtr zeroPath = newLucene<PathNode>();
	zeroPath->weight = 0;
	zeroPath->preNode = 0;
	path.add(zeroPath);

	for (current = 1; current <= nodeCount; current++)
	{
		double weight;
		Collection<SegTokenPairPtr> edges = getToList(current);

		double minWeight = std::numeric_limits<double>::max();
		SegTokenPairPtr minEdge = nullptr;
		for (auto edge : edges)
		{
			weight = edge->weight;
			PathNodePtr preNode = path[edge->from];
			if (preNode->weight + weight < minWeight)
			{
				minWeight = preNode->weight + weight;
				minEdge = edge;
			}
		}

		PathNodePtr newNode = newLucene<PathNode>();
		newNode->weight = minWeight;
		newNode->preNode = minEdge->from;
		path.add(newNode);
	}

	// Calculate PathNodes
	int preNode, lastNode;
	lastNode = path.size() - 1;
	current = lastNode;
	std::vector<int> rpath;
	Collection<SegTokenPtr> resultPath = Collection<SegTokenPtr>::newInstance();

	rpath.push_back(current);
	while (current != 0)
	{
		PathNodePtr currentPathNode = path[current];
		preNode = currentPathNode->preNode;
		rpath.push_back(preNode);
		current = preNode;
	}

	for (int j = rpath.size() - 1; j >= 0; j--)
	{
		int id = rpath[j];
		SegTokenPtr t = _segTokenList[id];
		resultPath.add(t);
	}

	return resultPath;
}

std::wstring BiSegGraph::toString()
{
	StringStream sb;

	for (const auto& pair : _tokenPairListTable)
	{
		for (const auto& tokenPair : pair.second)
		{
			sb << *tokenPair << L"\n";
		}
	}

	return sb.str();
}
}
}
}
}
}
