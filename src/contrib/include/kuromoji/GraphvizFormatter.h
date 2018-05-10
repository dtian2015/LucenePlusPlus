#pragma once

#include "LuceneObject.h"
#include "kuromoji/JapaneseTokenizer.h"

#include <unordered_map>

namespace Lucene {
namespace Analysis {
namespace Ja {

// TODO: would be nice to show 2nd best path in a diff't
// color...

/// <summary>
/// Outputs the dot (graphviz) string for the viterbi lattice.
/// </summary>
class GraphvizFormatter : public LuceneObject
{
private:
	static const String BOS_LABEL;

	static const String EOS_LABEL;

	static const String FONT_NAME;

	const Dict::ConnectionCostsPtr _costs;

	std::unordered_map<String, String> _bestPathMap;

	StringStream _sb;

public:
	LUCENE_CLASS(GraphvizFormatter);

	GraphvizFormatter(Dict::ConnectionCostsPtr costs);

	virtual String finish();

	// Backtraces another incremental fragment:
	virtual void onBacktrace(
		JapaneseTokenizerPtr tok,
		JapaneseTokenizer::WrappedPositionArrayPtr positions,
		int lastBackTracePos,
		JapaneseTokenizer::PositionPtr endPosData,
		int fromIDX,
		const CharArray& fragment,
		bool isEnd);

private:
	// Records which arcs make up the best bath:
	void setBestPathMap(
		JapaneseTokenizer::WrappedPositionArrayPtr positions, int startPos, JapaneseTokenizer::PositionPtr endPosData, int fromIDX);

	String formatNodes(
		JapaneseTokenizerPtr tok,
		JapaneseTokenizer::WrappedPositionArrayPtr positions,
		int startPos,
		JapaneseTokenizer::PositionPtr endPosData,
		const CharArray& fragment);

	String formatHeader();

	String formatTrailer();

	String getNodeID(int pos, int idx);
};
}
}
}
