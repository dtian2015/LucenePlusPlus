#include "ContribInc.h"

#include "kuromoji/GraphvizFormatter.h"

#include "kuromoji/dict/ConnectionCosts.h"

namespace Lucene {
namespace Analysis {
namespace Ja {

const String GraphvizFormatter::BOS_LABEL = L"BOS";
const String GraphvizFormatter::EOS_LABEL = L"EOS";
const String GraphvizFormatter::FONT_NAME = L"Helvetica";

GraphvizFormatter::GraphvizFormatter(Dict::ConnectionCostsPtr costs) : _costs(costs)
{
	_sb << formatHeader();
	_sb << L"  init [style=invis]\n";
	_sb << L"  init -> 0.0 [label=\"" + BOS_LABEL + L"\"]\n";
}

String GraphvizFormatter::finish()
{
	_sb << formatTrailer();
	return _sb.str();
}

void GraphvizFormatter::onBacktrace(
	JapaneseTokenizerPtr tok,
	JapaneseTokenizer::WrappedPositionArrayPtr positions,
	int lastBackTracePos,
	JapaneseTokenizer::PositionPtr endPosData,
	int fromIDX,
	const CharArray& fragment,
	bool isEnd)
{
	setBestPathMap(positions, lastBackTracePos, endPosData, fromIDX);
	_sb << formatNodes(tok, positions, lastBackTracePos, endPosData, fragment);

	if (isEnd)
	{
		_sb << L"  fini [style=invis]\n";
		_sb << L"  ";
		_sb << getNodeID(endPosData->pos, fromIDX);
		_sb << L" -> fini [label=\"" + EOS_LABEL + L"\"]";
	}
}

void GraphvizFormatter::setBestPathMap(
	JapaneseTokenizer::WrappedPositionArrayPtr positions, int startPos, JapaneseTokenizer::PositionPtr endPosData, int fromIDX)
{
	_bestPathMap.clear();

	int pos = endPosData->pos;
	int bestIDX = fromIDX;
	while (pos > startPos)
	{
		auto const posData = positions->get(pos);

		const int backPos = posData->backPos[bestIDX];
		const int backIDX = posData->backIndex[bestIDX];

		const String toNodeID = getNodeID(pos, bestIDX);
		const String fromNodeID = getNodeID(backPos, backIDX);

		BOOST_ASSERT(_bestPathMap.find(fromNodeID) == _bestPathMap.end());
		BOOST_ASSERT(std::find_if(_bestPathMap.begin(), _bestPathMap.end(), [&toNodeID](const std::pair<String, String> pair) {
						 return pair.second == toNodeID;
					 }) == _bestPathMap.end());

		_bestPathMap.emplace(fromNodeID, toNodeID);
		pos = backPos;
		bestIDX = backIDX;
	}
}

String GraphvizFormatter::formatNodes(
	JapaneseTokenizerPtr tok,
	JapaneseTokenizer::WrappedPositionArrayPtr positions,
	int startPos,
	JapaneseTokenizer::PositionPtr endPosData,
	const CharArray& fragment)
{
	StringStream sb;

	// Output nodes
	for (int pos = startPos + 1; pos <= endPosData->pos; pos++)
	{
		auto const posData = positions->get(pos);
		for (int idx = 0; idx < posData->count; idx++)
		{
			sb << L"  ";
			sb << getNodeID(pos, idx);
			sb << L" [label=\"";
			sb << pos;
			sb << L": ";
			sb << posData->lastRightID[idx];
			sb << L"\"]\n";
		}
	}

	// Output arcs
	for (int pos = endPosData->pos; pos > startPos; pos--)
	{
		auto const posData = positions->get(pos);
		for (int idx = 0; idx < posData->count; idx++)
		{
			auto const backPosData = positions->get(posData->backPos[idx]);
			const String toNodeID = getNodeID(pos, idx);
			const String fromNodeID = getNodeID(posData->backPos[idx], posData->backIndex[idx]);

			sb << L"  ";
			sb << fromNodeID;
			sb << L" -> ";
			sb << toNodeID;

			String attrs;
			if (toNodeID == _bestPathMap[fromNodeID])
			{
				// This arc is on best path
				attrs = L" color=\"#40e050\" fontcolor=\"#40a050\" penwidth=3 fontsize=20";
			}
			else
			{
				attrs = L"";
			}

			auto const dict = tok->getDict(posData->backType[idx]);
			const int wordCost = dict->getWordCost(posData->backID[idx]);
			const int bgCost = _costs->get(backPosData->lastRightID[posData->backIndex[idx]], dict->getLeftId(posData->backID[idx]));

			const String surfaceForm = String(fragment.get(), posData->backPos[idx] - startPos, pos - posData->backPos[idx]);

			sb << L" [label=\"";
			sb << surfaceForm;
			sb << L' ';
			sb << wordCost;

			if (bgCost >= 0)
			{
				sb << L'+';
			}

			sb << bgCost;
			sb << L"\"";
			sb << attrs;
			sb << L"]\n";
		}
	}

	return sb.str();
}

String GraphvizFormatter::formatHeader()
{
	StringStream sb;

	sb << L"digraph viterbi {\n";
	sb << L"  graph [ fontsize=30 labelloc=\"t\" label=\"\" splines=true overlap=false rankdir = \"LR\"];\n";
	// sb.append("  // A2 paper size\n");
	// sb.append("  size = \"34.4,16.5\";\n");
	// sb.append("  // try to fill paper\n");
	// sb.append("  ratio = fill;\n");
	sb << L"  edge [ fontname=\"" + FONT_NAME + L"\" fontcolor=\"red\" color=\"#606060\" ]\n";
	sb << L"  node [ style=\"filled\" fillcolor=\"#e8e8f0\" shape=\"Mrecord\" fontname=\"" + FONT_NAME + L"\" ]\n";

	return sb.str();
}

String GraphvizFormatter::formatTrailer()
{
	return L"}";
}

String GraphvizFormatter::getNodeID(int pos, int idx)
{
	return std::to_wstring(pos) + L"." + std::to_wstring(idx);
}
}
}
}
