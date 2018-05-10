#pragma once

#include "FST.h"
#include "Tokenizer.h"
#include "kuromoji/dict/Dictionary.h"

namespace Lucene {
namespace Analysis {
namespace Ja {

// TODO: somehow factor out a reusable viterbi search here,
// so other decompounders/tokenizers can reuse...

/// <summary>
/// Tokenizer for Japanese that uses morphological analysis.
/// <para>
/// This tokenizer sets a number of additional attributes:
/// <ul>
///   <li><seealso cref="BaseFormAttribute"/> containing base form for inflected
///       adjectives and verbs.
///   <li><seealso cref="PartOfSpeechAttribute"/> containing part-of-speech.
///   <li><seealso cref="ReadingAttribute"/> containing reading and pronunciation.
///   <li><seealso cref="InflectionAttribute"/> containing additional part-of-speech
///       information for inflected forms.
/// </ul>
/// </para>
/// <para>
/// This tokenizer uses a rolling Viterbi search to find the
/// least cost segmentation (path) of the incoming characters.
/// For tokens that appear to be compound (> length 2 for all
/// Kanji, or > length 7 for non-Kanji), we see if there is a
/// 2nd best segmentation of that token after applying
/// penalties to the long tokens.  If so, and the Mode is
/// <seealso cref="Mode#SEARCH"/>, we output the alternate segmentation
/// as well.
/// </para>
/// </summary>
class LPPCONTRIBAPI JapaneseTokenizer final : public Tokenizer
{
public:
	LUCENE_CLASS(JapaneseTokenizer);

	/// <summary>
	/// Tokenization mode: this determines how the tokenizer handles
	/// compound and unknown words.
	/// </summary>
	enum class Mode
	{
		/// <summary>
		/// Ordinary segmentation: no decomposition for compounds,
		/// </summary>
		NORMAL,

		/// <summary>
		/// Segmentation geared towards search: this includes a
		/// decompounding process for long nouns, also including
		/// the full compound token as a synonym.
		/// </summary>
		SEARCH,

		/// <summary>
		/// Extended mode outputs unigrams for unknown words.
		/// @lucene.experimental
		/// </summary>
		EXTENDED
	};

public:
	/// <summary>
	/// Default tokenization mode. Currently this is <seealso cref="Mode#SEARCH"/>.
	/// </summary>
	static const Mode DEFAULT_MODE;

public:
	enum class Type
	{
		KNOWN,
		UNKNOWN,
		USER
	};

	static const std::map<Type, String> TYPE_STRING_MAP;

private:
	const bool VERBOSE = false;

	const int SEARCH_MODE_KANJI_LENGTH = 2;

	const int SEARCH_MODE_OTHER_LENGTH = 7; // Must be >= SEARCH_MODE_KANJI_LENGTH

	const int SEARCH_MODE_KANJI_PENALTY = 3000;

	const int SEARCH_MODE_OTHER_PENALTY = 1700;

	// For safety:
	const int MAX_UNKNOWN_WORD_LENGTH = 1024;
	const int MAX_BACKTRACE_GAP = 1024;

	std::map<Type, Dict::DictionaryPtr> _dictionaryMap;

	Dict::TokenInfoFSTPtr _fst;
	Dict::TokenInfoDictionaryPtr _dictionary;
	Dict::UnknownDictionaryPtr _unkDictionary;
	Dict::ConnectionCostsPtr _costs;
	Dict::UserDictionaryPtr _userDictionary;
	Dict::CharacterDefinitionPtr _characterDefinition;

	const boost::shared_ptr<FSTArcLong> _arc = boost::make_shared<FSTArcLong>();
	FSTLong::BytesReaderPtr _fstReader;

	OffsetAndLength _wordIdRef;

	FSTLong::BytesReaderPtr _userFSTReader;
	Dict::TokenInfoFSTPtr _userFST;

	RollingCharBufferPtr _buffer;

	bool _discardPunctuation = false;
	bool _searchMode = false;
	bool _extendedMode = false;
	bool _outputCompounds = false;

	// Index of the last character of unknown word:
	int _unknownWordEndIndex = -1;

	// True once we've hit the EOF from the input reader:
	bool _end = false;

	// Last absolute position we backtraced from:
	int _lastBackTracePos = 0;

	// Position of last token we returned; we use this to
	// figure out whether to set posIncr to 0 or 1:
	int _lastTokenPos = 0;

	// Next absolute position to process:
	int _pos = 0;

	// Already parsed, but not yet passed to caller, tokens:
	std::vector<Ja::TokenPtr> _pending;

	CharTermAttributePtr _termAtt;
	OffsetAttributePtr _offsetAtt;
	PositionIncrementAttributePtr _posIncAtt;
	PositionLengthAttributePtr _posLengthAtt;
	TokenAttributes::BaseFormAttributePtr _basicFormAtt;
	TokenAttributes::PartOfSpeechAttributePtr _posAtt;
	TokenAttributes::ReadingAttributePtr _readingAtt;
	TokenAttributes::InflectionAttributePtr _inflectionAtt;

public:
	/// <summary>
	/// Create a new JapaneseTokenizer.
	/// </summary>
	/// <param name="input"> Reader containing text </param>
	/// <param name="userDictionary"> Optional: if non-null, user dictionary. </param>
	/// <param name="discardPunctuation"> true if punctuation tokens should be dropped from the output. </param>
	/// <param name="mode"> tokenization mode. </param>
	JapaneseTokenizer(ReaderPtr input, Dict::UserDictionaryPtr userDictionary, bool discardPunctuation, Mode mode);

private:
	GraphvizFormatterPtr _dotOut;

public:
	/// <summary>
	/// Expert: set this to produce graphviz (dot) output of
	///  the Viterbi lattice
	/// </summary>
	void setGraphvizFormatter(GraphvizFormatterPtr dotOut);

	void reset(const ReaderPtr& input);

	void reset();

private:
	void resetState();

public:
	void end();

private:
	// Returns the added cost that a 2nd best segmentation is
	// allowed to have.  Ie, if we see path with cost X,
	// ending in a compound word, and this method returns
	// threshold > 0, then we will also find the 2nd best
	// segmentation and if its path score is within this
	// threshold of X, we'll include it in the output:
	int computeSecondBestThreshold(int pos, int length);

	int computePenalty(int pos, int length);

public:
	DECLARE_SHARED_PTR(Position)

	// Holds all back pointers arriving to this position:
	class Position final : public LuceneObject
	{
	public:
		LUCENE_CLASS(Position);
		int pos = 0;

		int count = 0;

		// maybe single int array * 5?
		std::vector<int> costs = std::vector<int>(8);
		std::vector<int> lastRightID = std::vector<int>(8);
		std::vector<int> backPos = std::vector<int>(8);
		std::vector<int> backIndex = std::vector<int>(8);
		std::vector<int> backID = std::vector<int>(8);
		std::vector<Type> backType = std::vector<Type>(8);

		// Only used when finding 2nd best segmentation under a
		// too-long token:
		int forwardCount = 0;
		std::vector<int> forwardPos = std::vector<int>(8);
		std::vector<int> forwardID = std::vector<int>(8);
		std::vector<int> forwardIndex = std::vector<int>(8);
		std::vector<Type> forwardType = std::vector<Type>(8);

		void grow();

		void growForward();

		void add(int cost, int lastRightID, int backPos, int backIndex, int backID, Type backType);

		void addForward(int forwardPos, int forwardIndex, int forwardID, Type forwardType);

		void reset();
	};

private:
	void add(Dict::DictionaryPtr dict, PositionPtr fromPosData, int endPos, int wordID, Type type, bool addPenalty);

public:
	bool incrementToken();

public:
	DECLARE_SHARED_PTR(WrappedPositionArray)

	// TODO: make generic'd version of this "circular array"?
	// It's a bit tricky because we do things to the Position
	// (eg, set .pos = N on reuse)...
	class WrappedPositionArray final : public LuceneObject
	{
	private:
		std::vector<PositionPtr> _positions = std::vector<PositionPtr>(8);

	public:
		LUCENE_CLASS(WrappedPositionArray);

		WrappedPositionArray();

	private:
		// Next array index to write to in positions:
		int _nextWrite = 0;

		// Next position to write:
		int _nextPos = 0;

		// How many valid Position instances are held in the
		// positions array:
		int _count = 0;

	public:
		void reset();

		/// <summary>
		/// Get Position instance for this absolute position;
		///  this is allowed to be arbitrarily far "in the
		///  future" but cannot be before the last freeBefore.
		/// </summary>
		PositionPtr get(int pos);

		int getNextPos();

		// For assert:
	private:
		bool inBounds(int pos);

		int getIndex(int pos);

	public:
		void freeBefore(int pos);
	};

private:
	const WrappedPositionArrayPtr _positions = newLucene<WrappedPositionArray>();

	/* Incrementally parse some more characters.  This runs
	 * the viterbi search forwards "enough" so that we
	 * generate some more tokens.  How much forward depends on
	 * the chars coming in, since some chars could cause
	 * longer-lasting ambiguity in the parsing.  Once the
	 * ambiguity is resolved, then we back trace, produce
	 * the pending tokens, and return. */
	void parse();

	// Eliminates arcs from the lattice that are compound
	// tokens (have a penalty) or are not congruent with the
	// compound token we've matched (ie, span across the
	// startPos).  This should be fairly efficient, because we
	// just keep the already intersected structure of the
	// graph, eg we don't have to consult the FSTs again:
	void pruneAndRescore(int startPos, int endPos, int bestStartIDX);

	// Backtrace from the provided position, back to the last
	// time we back-traced, accumulating the resulting tokens to
	// the pending list.  The pending list is then in-reverse
	// (last token should be returned first).
	void backtrace(PositionPtr endPosData, int const fromIDX);

public:
	Dict::DictionaryPtr getDict(Type type);

private:
	static bool isPunctuation(wchar_t ch);
};
}
}
}
