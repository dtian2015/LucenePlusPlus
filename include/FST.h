#pragma once

#include "CodecUtil.h"
#include "FSDirectory.h"
#include "IntsRef.h"
#include "Outputs.h"
#include "PriorityQueue.h"
#include "_SimpleFSDirectory.h"

#include <unordered_map>

namespace Lucene {
namespace Util {
namespace FST {

// TODO: The original Builder and FST are in different Java source file, but this causing issues in C++ as they both are templates and
// depending on each other. So just put them in one file for now
template <typename>
class Builder;
template <typename>
class FST;
template <typename>
class NodeHash;

#pragma Builder

/// <summary>
/// Builds a minimal FST (maps an IntsRef term to an arbitrary
/// output) from pre-sorted terms with outputs.  The FST
/// becomes an FSA if you use NoOutputs.  The FST is written
/// on-the-fly into a compact serialized format byte array, which can
/// be saved to / loaded from a Directory or used directly
/// for traversal.  The FST is always finite (no cycles).
///
/// <para>NOTE: The algorithm is described at
/// http://citeseerx.ist.psu.edu/viewdoc/summary?doi=10.1.1.24.3698</para>
///
/// The parameterized type T is the output type.  See the
/// subclasses of <seealso cref="Outputs"/>.
///
/// @lucene.experimental
/// </summary>
template <typename T>
class LPPAPI Builder : public LuceneObject
{
public:
	LUCENE_CLASS(Builder<T>);

private:
	boost::shared_ptr<NodeHash<T>> _dedupHash;
	const boost::shared_ptr<FST<T>> _fst;
	const T NO_OUTPUT;

	// private static final boolean DEBUG = true;

	// simplistic pruning: we prune node (and all following
	// nodes) if less than this number of terms go through it:
	const int _minSuffixCount1;

	// better pruning: we prune node (and all following
	// nodes) if the prior node has less than this number of
	// terms go through it:
	const int _minSuffixCount2;

	const bool _doShareNonSingletonNodes;
	const int _shareMaxTailLength;

	const IntsRefPtr _lastInput = newInstance<IntsRef>();

public:
	DECLARE_SHARED_PTR(Node)

	// NOTE: not many instances of Node or CompiledNode are in
	// memory while the FST is being built; it's only the
	// current "frontier":
	class Node : public LuceneObject
	{
	public:
		LUCENE_CLASS(Node);
		virtual bool isCompiled() = 0;
	};

public:
	/// <summary>
	/// Expert: holds a pending (seen but not yet serialized) arc.
	/// </summary>
	template <typename ArcType>
	class Arc : public LuceneObject
	{
	public:
		LUCENE_CLASS(Arc<ArcType>);

		int label = 0; // really an "unsigned" byte
		NodePtr target;
		bool isFinal = false;
		ArcType output;
		ArcType nextFinalOutput;
	};

public:
	DECLARE_SHARED_PTR(CompiledNode)

	class CompiledNode final : public Node
	{
	public:
		LUCENE_CLASS(CompiledNode);
		int node = 0;
		bool isCompiled() { return true; };
	};

public:
	/// <summary>
	/// Expert: holds a pending (seen but not yet serialized) Node.
	/// </summary>
	template <typename U>
	class UnCompiledNode final : public Node
	{
	private:
		Builder<U>* _owner;

	public:
		int numArcs = 0;
		std::vector<boost::shared_ptr<Arc<U>>> arcs;

		// TODO: instead of recording isFinal/output on the
		// node, maybe we should use -1 arc to mean "end" (like
		// we do when reading the FST).  Would simplify much
		// code here...
		U output;
		bool isFinal = false;
		long long inputCount = 0;

		/// <summary>
		/// This node's depth, starting from the automaton root. </summary>
		const int depth;

		/// <param name="depth">
		///          The node's depth starting from the automaton root. Needed for
		///          LUCENE-2934 (node expansion based on conditions other than the
		///          fanout size). </param>
		UnCompiledNode(Builder<U>* owner, int depth) : _owner(owner), depth(depth)
		{
			arcs.push_back(boost::make_shared<Arc<U>>());
			output = _owner->NO_OUTPUT;
		}

		bool isCompiled() { return false; }
		void clear()
		{
			numArcs = 0;
			isFinal = false;
			output = _owner->NO_OUTPUT;
			inputCount = 0;

			// We don't clear the depth here because it never changes
			// for nodes on the frontier (even when reused).
		}

		U getLastOutput(int labelToMatch)
		{
			BOOST_ASSERT(numArcs > 0);
			BOOST_ASSERT(arcs[numArcs - 1]->label == labelToMatch);
			return arcs[numArcs - 1]->output;
		}

		void addArc(int label, NodePtr target)
		{
			BOOST_ASSERT(label >= 0);
			BOOST_ASSERT(numArcs == 0 || label > arcs[numArcs - 1]->label);

			if (numArcs == arcs.size())
			{
				arcs.resize(numArcs + 1);
				for (int arcIdx = numArcs; arcIdx < arcs.size(); arcIdx++)
				{
					arcs[arcIdx] = boost::make_shared<Arc<U>>();
				}
			}

			auto arc = arcs[numArcs++];
			arc->label = label;
			arc->target = target;
			arc->output = arc->nextFinalOutput = _owner->NO_OUTPUT;
			arc->isFinal = false;
		}

		void replaceLast(int labelToMatch, NodePtr target, U nextFinalOutput, bool isFinal)
		{
			BOOST_ASSERT(numArcs > 0);
			auto arc = arcs[numArcs - 1];
			BOOST_ASSERT(arc->label == labelToMatch);

			arc->target = target;
			// assert target.node != -2;
			arc->nextFinalOutput = nextFinalOutput;
			arc->isFinal = isFinal;
		}

		void deleteLast(int label, NodePtr target)
		{
			BOOST_ASSERT(numArcs > 0);
			BOOST_ASSERT(label == arcs[numArcs - 1]->label);
			BOOST_ASSERT(target == arcs[numArcs - 1]->target);
			numArcs--;
		}

		void setLastOutput(int labelToMatch, U newOutput)
		{
			BOOST_ASSERT(_owner->validOutput(newOutput));
			BOOST_ASSERT(numArcs > 0);
			auto const arc = arcs[numArcs - 1];
			BOOST_ASSERT(arc->label == labelToMatch);
			arc->output = newOutput;
		}

		// pushes an output prefix forward onto all arcs
		void prependOutput(U outputPrefix)
		{
			BOOST_ASSERT(_owner->validOutput(outputPrefix));

			for (int arcIdx = 0; arcIdx < numArcs; arcIdx++)
			{
				arcs[arcIdx]->output = _owner->_fst->outputs->add(outputPrefix, arcs[arcIdx]->output);
				BOOST_ASSERT(_owner->validOutput(arcs[arcIdx]->output));
			}

			if (isFinal)
			{
				output = _owner->_fst->outputs->add(outputPrefix, output);
				BOOST_ASSERT(_owner->validOutput(output));
			}
		}
	};

public:
	/// <summary>
	/// Expert: this is invoked by Builder whenever a suffix
	///  is serialized.
	/// </summary>
	template <typename NodeT>
	class FreezeTail : public LuceneObject
	{
	public:
		virtual void freeze(
			const std::vector<boost::shared_ptr<UnCompiledNode<T>>>& frontier, int prefixLenPlus1, IntsRefPtr prevInput) = 0;
	};

private:
	// NOTE: cutting this over to ArrayList instead loses ~6%
	// in build performance on 9.8M Wikipedia terms; so we
	// left this as an array:
	// current "frontier"
	std::vector<boost::shared_ptr<UnCompiledNode<T>>> _frontier;

	const boost::shared_ptr<FreezeTail<T>> _freezeTail;

public:
	/// <summary>
	/// Instantiates an FST/FSA builder without any pruning. A shortcut
	/// to {@link #Builder(FST.INPUT_TYPE, int, int, boolean,
	/// boolean, int, Outputs, FreezeTail, boolean)} with
	/// pruning options turned off.
	/// </summary>
	Builder(typename FST<T>::INPUT_TYPE inputType, boost::shared_ptr<Outputs<T>> outputs)
		: Builder(inputType, 0, 0, true, true, std::numeric_limits<int>::max(), outputs, nullptr, false)
	{
	}

	/// <summary>
	/// Instantiates an FST/FSA builder with all the possible tuning and construction
	/// tweaks. Read parameter documentation carefully.
	/// </summary>
	/// <param name="inputType">
	///    The input type (transition labels). Can be anything from <seealso cref="INPUT_TYPE"/>
	///    enumeration. Shorter types will consume less memory. Strings (character sequences) are
	///    represented as <seealso cref="INPUT_TYPE#BYTE4"/> (full unicode codepoints).
	/// </param>
	/// <param name="minSuffixCount1">
	///    If pruning the input graph during construction, this threshold is used for telling
	///    if a node is kept or pruned. If transition_count(node) &gt;= minSuffixCount1, the node
	///    is kept.
	/// </param>
	/// <param name="minSuffixCount2">
	///    (Note: only Mike McCandless knows what this one is really doing...)
	/// </param>
	/// <param name="doShareSuffix">
	///    If <code>true</code>, the shared suffixes will be compacted into unique paths.
	///    This requires an additional hash map for lookups in memory. Setting this parameter to
	///    <code>false</code> creates a single path for all input sequences. This will result in a larger
	///    graph, but may require less memory and will speed up construction.
	/// </param>
	/// <param name="doShareNonSingletonNodes">
	///    Only used if doShareSuffix is true.  Set this to
	///    true to ensure FST is fully minimal, at cost of more
	///    CPU and more RAM during building.
	/// </param>
	/// <param name="shareMaxTailLength">
	///    Only used if doShareSuffix is true.  Set this to
	///    Integer.MAX_VALUE to ensure FST is fully minimal, at cost of more
	///    CPU and more RAM during building.
	/// </param>
	/// <param name="outputs"> The output type for each input sequence. Applies only if building an FST. For
	///    FSA, use <seealso cref="NoOutputs#getSingleton()"/> and <seealso cref="NoOutputs#getNoOutput()"/> as the
	///    singleton output object.
	/// </param>
	/// <param name="willPackFST"> Pass true if you will pack the FST before saving.  This
	///    causes the FST to create additional data structures internally to facilitate packing, but
	///    it means the resulting FST cannot be saved: it must
	///    first be packed using <seealso cref="FST#pack(int, int)"/>}. </param>
	Builder(
		typename FST<T>::INPUT_TYPE inputType,
		int minSuffixCount1,
		int minSuffixCount2,
		bool doShareSuffix,
		bool doShareNonSingletonNodes,
		int shareMaxTailLength,
		boost::shared_ptr<Outputs<T>> outputs,
		boost::shared_ptr<FreezeTail<T>> freezeTail,
		bool willPackFST)
		: _fst(newLucene<FST<T>>(inputType, outputs, willPackFST))
		, NO_OUTPUT(outputs->getNoOutput())
		, _minSuffixCount1(minSuffixCount1)
		, _minSuffixCount2(minSuffixCount2)
		, _doShareNonSingletonNodes(doShareNonSingletonNodes)
		, _shareMaxTailLength(shareMaxTailLength)
		, _freezeTail(freezeTail)
	{
		if (doShareSuffix)
		{
			_dedupHash = boost::make_shared<NodeHash<T>>(_fst);
		}
		else
		{
			_dedupHash.reset();
		}

		_frontier.resize(10);
		for (int idx = 0; idx < _frontier.size(); idx++)
		{
			_frontier[idx] = boost::make_shared<UnCompiledNode<T>>(this, idx);
		}
	}

	virtual int getTotStateCount() { return _fst->nodeCount; }
	virtual long long getTermCount() { return _frontier[0]->inputCount; }
	virtual int getMappedStateCount() { return _dedupHash == nullptr ? 0 : _fst->nodeCount; }
	/// <summary>
	/// Pass false to disable the array arc optimization
	///  while building the FST; this will make the resulting
	///  FST smaller but slower to traverse.
	/// </summary>
	virtual void setAllowArrayArcs(bool b) { _fst->setAllowArrayArcs(b); }
private:
	CompiledNodePtr compileNode(boost::shared_ptr<UnCompiledNode<T>> nodeIn, int tailLength)
	{
		int node = 0;
		if (_dedupHash != nullptr && (_doShareNonSingletonNodes || nodeIn->numArcs <= 1) && tailLength <= _shareMaxTailLength)
		{
			if (nodeIn->numArcs == 0)
			{
				node = _fst->addNode(nodeIn);
			}
			else
			{
				node = _dedupHash->add(nodeIn);
			}
		}
		else
		{
			node = _fst->addNode(nodeIn);
		}
		BOOST_ASSERT(node != -2);

		nodeIn->clear();

		auto fn = newInstance<CompiledNode>();
		fn->node = node;
		return fn;
	}

	void freezeTail(int prefixLenPlus1)
	{
		if (_freezeTail != nullptr)
		{
			// Custom plugin:
			_freezeTail->freeze(_frontier, prefixLenPlus1, _lastInput);
		}
		else
		{
			// System.out.println("  compileTail " + prefixLenPlus1);
			const int downTo = std::max(1, prefixLenPlus1);
			for (int idx = _lastInput->length; idx >= downTo; idx--)
			{
				bool doPrune = false;
				bool doCompile = false;

				auto node = _frontier[idx];
				auto parent = _frontier[idx - 1];

				if (node->inputCount < _minSuffixCount1)
				{
					doPrune = true;
					doCompile = true;
				}
				else if (idx > prefixLenPlus1)
				{
					// prune if parent's inputCount is less than suffixMinCount2
					if (parent->inputCount < _minSuffixCount2 || (_minSuffixCount2 == 1 && parent->inputCount == 1 && idx > 1))
					{
						// my parent, about to be compiled, doesn't make the cut, so
						// I'm definitely pruned

						// if minSuffixCount2 is 1, we keep only up
						// until the 'distinguished edge', ie we keep only the
						// 'divergent' part of the FST. if my parent, about to be
						// compiled, has inputCount 1 then we are already past the
						// distinguished edge.  NOTE: this only works if
						// the FST outputs are not "compressible" (simple
						// ords ARE compressible).
						doPrune = true;
					}
					else
					{
						// my parent, about to be compiled, does make the cut, so
						// I'm definitely not pruned
						doPrune = false;
					}
					doCompile = true;
				}
				else
				{
					// if pruning is disabled (count is 0) we can always
					// compile current node
					doCompile = _minSuffixCount2 == 0;
				}

				// System.out.println("    label=" + ((char) lastInput.ints[lastInput.offset+idx-1]) + " idx=" + idx + " inputCount=" +
				// frontier[idx].inputCount + " doCompile=" + doCompile + " doPrune=" + doPrune);

				if (node->inputCount < _minSuffixCount2 || (_minSuffixCount2 == 1 && node->inputCount == 1 && idx > 1))
				{
					// drop all arcs
					for (int arcIdx = 0; arcIdx < node->numArcs; arcIdx++)
					{
						auto target = boost::static_pointer_cast<UnCompiledNode<T>>(node->arcs[arcIdx]->target);
						target->clear();
					}
					node->numArcs = 0;
				}

				if (doPrune)
				{
					// this node doesn't make it -- deref it
					node->clear();
					parent->deleteLast(_lastInput->ints[_lastInput->offset + idx - 1], node);
				}
				else
				{
					if (_minSuffixCount2 != 0)
					{
						compileAllTargets(node, _lastInput->length - idx);
					}
					const T nextFinalOutput = node->output;

					// We "fake" the node as being final if it has no
					// outgoing arcs; in theory we could leave it
					// as non-final (the FST can represent this), but
					// FSTEnum, Util, etc., have trouble w/ non-final
					// dead-end states:
					const bool isFinal = node->isFinal || node->numArcs == 0;

					if (doCompile)
					{
						// this node makes it and we now compile it.  first,
						// compile any targets that were previously
						// undecided:
						parent->replaceLast(
							_lastInput->ints[_lastInput->offset + idx - 1], compileNode(node, 1 + _lastInput->length - idx),
							nextFinalOutput, isFinal);
					}
					else
					{
						// replaceLast just to install
						// nextFinalOutput/isFinal onto the arc
						parent->replaceLast(_lastInput->ints[_lastInput->offset + idx - 1], node, nextFinalOutput, isFinal);
						// this node will stay in play for now, since we are
						// undecided on whether to prune it.  later, it
						// will be either compiled or pruned, so we must
						// allocate a new node:
						_frontier[idx] = boost::make_shared<UnCompiledNode<T>>(this, idx);
					}
				}
			}
		}
	}

	// for debugging
	/*
	 private String toString(BytesRef b) {
	 try {
	 return b.utf8ToString() + " " + b;
	 } catch (Throwable t) {
	 return b.toString();
	 }
	 }
	 */

public:
	/// <summary>
	/// It's OK to add the same input twice in a row with
	///  different outputs, as long as outputs impls the merge
	///  method.
	/// </summary>
	virtual void add(IntsRefPtr input, T output)
	{
		/*
		 if (DEBUG) {
		 BytesRef b = new BytesRef(input.length);
		 for(int x=0;x<input.length;x++) {
		 b.bytes[x] = (byte) input.ints[x];
		 }
		 b.length = input.length;
		 if (output == NO_OUTPUT) {
		 System.out.println("\nFST ADD: input=" + toString(b) + " " + b);
		 } else {
		 System.out.println("\nFST ADD: input=" + toString(b) + " " + b + " output=" + fst.outputs.outputToString(output));
		 }
		 }
		 */

		// De-dup NO_OUTPUT since it must be a singleton:
		if (output == NO_OUTPUT)
		{
			output = NO_OUTPUT;
		}

		BOOST_ASSERT(_lastInput->length == 0 || input->compareTo(_lastInput) >= 0);
		BOOST_ASSERT(validOutput(output));

		// System.out.println("\nadd: " + input);
		if (input->length == 0)
		{
			// empty input: only allowed as first input.  we have
			// to special case this because the packed FST
			// format cannot represent the empty input since
			// 'finalness' is stored on the incoming arc, not on
			// the node
			_frontier[0]->inputCount++;
			_frontier[0]->isFinal = true;
			_fst->setEmptyOutput(output);
			return;
		}

		// compare shared prefix length
		int pos1 = 0;
		int pos2 = input->offset;
		const int pos1Stop = std::min(_lastInput->length, input->length);
		while (true)
		{
			_frontier[pos1]->inputCount++;
			// System.out.println("  incr " + pos1 + " ct=" + frontier[pos1].inputCount + " n=" + frontier[pos1]);
			if (pos1 >= pos1Stop || _lastInput->ints[pos1] != input->ints[pos2])
			{
				break;
			}
			pos1++;
			pos2++;
		}
		const int prefixLenPlus1 = pos1 + 1;

		int originalSize = _frontier.size();
		if (originalSize < input->length + 1)
		{
			_frontier.resize(input->length + 1);
			for (int idx = originalSize; idx < _frontier.size(); idx++)
			{
				_frontier[idx] = boost::make_shared<UnCompiledNode<T>>(this, idx);
			}
		}

		// minimize/compile states from previous input's
		// orphan'd suffix
		freezeTail(prefixLenPlus1);

		// init tail states for current input
		for (int idx = prefixLenPlus1; idx <= input->length; idx++)
		{
			_frontier[idx - 1]->addArc(input->ints[input->offset + idx - 1], _frontier[idx]);
			_frontier[idx]->inputCount++;
		}

		auto lastNode = _frontier[input->length];
		lastNode->isFinal = true;
		lastNode->output = NO_OUTPUT;

		// push conflicting outputs forward, only as far as
		// needed
		for (int idx = 1; idx < prefixLenPlus1; idx++)
		{
			auto node = _frontier[idx];
			auto parentNode = _frontier[idx - 1];

			const T lastOutput = parentNode->getLastOutput(input->ints[input->offset + idx - 1]);
			BOOST_ASSERT(validOutput(lastOutput));

			T commonOutputPrefix;
			T wordSuffix;

			if (lastOutput != NO_OUTPUT)
			{
				commonOutputPrefix = _fst->outputs->common(output, lastOutput);
				BOOST_ASSERT(validOutput(commonOutputPrefix));
				wordSuffix = _fst->outputs->subtract(lastOutput, commonOutputPrefix);
				BOOST_ASSERT(validOutput(wordSuffix));
				parentNode->setLastOutput(input->ints[input->offset + idx - 1], commonOutputPrefix);
				node->prependOutput(wordSuffix);
			}
			else
			{
				commonOutputPrefix = wordSuffix = NO_OUTPUT;
			}

			output = _fst->outputs->subtract(output, commonOutputPrefix);
			BOOST_ASSERT(validOutput(output));
		}

		if (_lastInput->length == input->length && prefixLenPlus1 == 1 + input->length)
		{
			// same input more than 1 time in a row, mapping to
			// multiple outputs
			lastNode->output = _fst->outputs->merge(lastNode->output, output);
		}
		else
		{
			// this new arc is private to this new input; set its
			// arc output to the leftover output:
			_frontier[prefixLenPlus1 - 1]->setLastOutput(input->ints[input->offset + prefixLenPlus1 - 1], output);
		}

		// save last input
		_lastInput->copyInts(input);

		// System.out.println("  count[0]=" + frontier[0].inputCount);
	}

private:
	bool validOutput(T output) const { return output == NO_OUTPUT || !(output == NO_OUTPUT); }
	/// <summary>
	/// Returns final FST.  NOTE: this will return null if
	///  nothing is accepted by the FST.
	/// </summary>
public:
	virtual boost::shared_ptr<FST<T>> finish()
	{
		auto root = _frontier[0];

		// minimize nodes in the last word's suffix
		freezeTail(0);
		if (root->inputCount < _minSuffixCount1 || root->inputCount < _minSuffixCount2 || root->numArcs == 0)
		{
			if (_fst->emptyOutput == nullptr)
			{
				return nullptr;
			}
			else if (_minSuffixCount1 > 0 || _minSuffixCount2 > 0)
			{
				// empty string got pruned
				return nullptr;
			}
		}
		else
		{
			if (_minSuffixCount2 != 0)
			{
				compileAllTargets(root, _lastInput->length);
			}
		}
		// if (DEBUG) System.out.println("  builder.finish root.isFinal=" + root.isFinal + " root.output=" + root.output);
		_fst->finish(compileNode(root, _lastInput->length)->node);

		return _fst;
	}

private:
	void compileAllTargets(boost::shared_ptr<UnCompiledNode<T>> node, int tailLength)
	{
		for (int arcIdx = 0; arcIdx < node->numArcs; arcIdx++)
		{
			auto arc = node->arcs[arcIdx];
			if (!arc->target->isCompiled())
			{
				// not yet compiled
				boost::shared_ptr<UnCompiledNode<T>> const n = boost::static_pointer_cast<UnCompiledNode<T>>(arc->target);

				if (n->numArcs == 0)
				{
					// System.out.println("seg=" + segment + "        FORCE final arc=" + (char) arc.label);
					arc->isFinal = n->isFinal = true;
				}

				arc->target = compileNode(n, tailLength - 1);
			}
		}
	}
};

#pragma FST

// TODO: break this into WritableFST and ReadOnlyFST.. then
// we can have subclasses of ReadOnlyFST to handle the
// different byte[] level encodings (packed or
// not)... and things like nodeCount, arcCount are read only

// TODO: if FST is pure prefix trie we can do a more compact
// job, ie, once we are at a 'suffix only', just store the
// completion labels as a string not as a series of arcs.

// TODO: maybe make an explicit thread state that holds
// reusable stuff eg BytesReader, a scratch arc

// NOTE: while the FST is able to represent a non-final
// dead-end state (NON_FINAL_END_NODE=0), the layers above
// (FSTEnum, Util) have problems with this!!

/// <summary>
/// Represents an finite state machine (FST), using a
///  compact byte[] format.
///  <para> The format is similar to what's used by Morfologik
///  (http://sourceforge.net/projects/morfologik).
///
/// </para>
///  <para><b>NOTE</b>: the FST cannot be larger than ~2.1 GB
///  because it uses int to address the byte[].
///
/// @lucene.experimental
/// </para>
/// </summary>
template <typename T>
class LPPAPI FST final : public LuceneObject
{
public:
	LUCENE_CLASS(FST<T>);

	/// <summary>
	/// Specifies allowed range of each int input label for
	///  this FST.
	/// </summary>
	enum class INPUT_TYPE
	{
		BYTE1,
		BYTE2,
		BYTE4
	};

	friend class Builder<T>;

public:
	INPUT_TYPE inputType;

	static const int BIT_FINAL_ARC = 1 << 0;
	static const int BIT_LAST_ARC = 1 << 1;
	static const int BIT_TARGET_NEXT = 1 << 2;

	// TODO: we can free up a bit if we can nuke this:
	static const int BIT_STOP_NODE = 1 << 3;
	static const int BIT_ARC_HAS_OUTPUT = 1 << 4;
	static const int BIT_ARC_HAS_FINAL_OUTPUT = 1 << 5;

private:
	// Arcs are stored as fixed-size (per entry) array, so
	// that we can find an arc using binary search.  We do
	// this when number of arcs is > NUM_ARCS_ARRAY:

	// If set, the target node is delta coded vs current
	// position:
	static const int BIT_TARGET_DELTA = 1 << 6;

	static const char ARCS_AS_FIXED_ARRAY = BIT_ARC_HAS_FINAL_OUTPUT;

public:
	/// <seealso cref= #shouldExpand(UnCompiledNode) </seealso>
	static const int FIXED_ARRAY_SHALLOW_DISTANCE = 3; // 0 => only root node.

	/// <seealso cref= #shouldExpand(UnCompiledNode) </seealso>
	static const int FIXED_ARRAY_NUM_ARCS_SHALLOW = 5;

	/// <seealso cref= #shouldExpand(UnCompiledNode) </seealso>
	static const int FIXED_ARRAY_NUM_ARCS_DEEP = 10;

private:
	std::vector<int> bytesPerArc;

	const String FILE_FORMAT_NAME = L"FST";
	static const int VERSION_START = 0;

	/// <summary>
	/// Changed numBytesPerArc for array'd case from byte to int. </summary>
	static const int VERSION_INT_NUM_BYTES_PER_ARC = 1;

	/// <summary>
	/// Write BYTE2 labels as 2-byte short, not vInt. </summary>
	static const int VERSION_SHORT_BYTE2_LABELS = 2;

	/// <summary>
	/// Added optional packed format. </summary>
	static const int VERSION_PACKED = 3;

	static const int VERSION_CURRENT = VERSION_PACKED;

	// Never serialized; just used to represent the virtual
	// final node w/ no arcs:
	static const int FINAL_END_NODE = -1;

	// Never serialized; just used to represent the virtual
	// non-final node w/ no arcs:
	static const int NON_FINAL_END_NODE = 0;

	// if non-null, this FST accepts the empty string and
	// produces this output
public:
	T emptyOutput;

private:
	std::vector<uint8_t> emptyOutputBytes;

	// Not private to avoid synthetic access$NNN methods:
public:
	std::vector<uint8_t> bytes;
	int byteUpto = 0;

private:
	int startNode = -1;

public:
	const boost::shared_ptr<Outputs<T>> outputs;

private:
	int lastFrozenNode = 0;

	const T NO_OUTPUT;

public:
	int nodeCount = 0;
	int arcCount = 0;
	int arcWithOutputCount = 0;

private:
	bool packed = false;
	std::vector<int> nodeRefToAddress;

public:
	// If arc has this label then that arc is final/accepted
	static const int END_LABEL = -1;

public:
	/// <summary>
	/// Represents a single arc. </summary>
	template <typename ArcType>
	class Arc final : public LuceneObject
	{
	public:
		LUCENE_CLASS(Arc<ArcType>);
		int label = 0;
		ArcType output;

		// From node (ord or address); currently only used when
		// building an FST w/ willPackFST=true:
		int node = 0;

		// To node (ord or address):
		int target = 0;

		char flags = 0;
		ArcType nextFinalOutput;

		// address (into the byte[]), or ord/address if label == END_LABEL
		int nextArc = 0;

		// This is non-zero if current arcs are fixed array:
		int posArcsStart = 0;
		int bytesPerArc = 0;
		int arcIdx = 0;
		int numArcs = 0;

		/// <summary>
		/// Returns this </summary>
		boost::shared_ptr<Arc<ArcType>> copyFrom(boost::shared_ptr<Arc<ArcType>> other)
		{
			node = other->node;
			label = other->label;
			target = other->target;
			flags = other->flags;
			output = other->output;
			nextFinalOutput = other->nextFinalOutput;
			nextArc = other->nextArc;
			bytesPerArc = other->bytesPerArc;
			if (bytesPerArc != 0)
			{
				posArcsStart = other->posArcsStart;
				arcIdx = other->arcIdx;
				numArcs = other->numArcs;
			}

			return shared_from_this();
		}

		bool flag(int flag) { return FST::flag(flags, flag); }
		bool isLast() { return flag(BIT_LAST_ARC); }
		bool isFinal() { return flag(BIT_FINAL_ARC); }
		String toString()
		{
			StringStream b;

			b << L"node=" << std::to_wstring(node);
			b << L" target=" << std::to_wstring(target);
			b << L" label=" << std::to_wstring(label);
			if (flag(BIT_LAST_ARC))
			{
				b << L" last";
			}
			if (flag(BIT_FINAL_ARC))
			{
				b << L" final";
			}
			if (flag(BIT_TARGET_NEXT))
			{
				b << L" targetNext";
			}
			if (flag(BIT_ARC_HAS_OUTPUT))
			{
				b << L" output=" << output;
			}
			if (flag(BIT_ARC_HAS_FINAL_OUTPUT))
			{
				b << L" nextFinalOutput=" << nextFinalOutput;
			}
			if (bytesPerArc != 0)
			{
				b << L" arcArray(idx=" << std::to_wstring(arcIdx) << L" of " << std::to_wstring(numArcs) << L")";
			}
			return b.str();
		}
	};

private:
	bool allowArrayArcs = true;

	std::vector<boost::shared_ptr<Arc<T>>> cachedRootArcs;

public:
	DECLARE_SHARED_PTR(BytesWriter)

	// Non-static: writes to FST's byte[]
	class BytesWriter : public DataOutput
	{
	private:
		FST<T>* _outerInstance;

	public:
		LUCENE_CLASS(BytesWriter);
		int posWrite = 0;

		explicit BytesWriter(FST<T>* outerInstance) : _outerInstance(outerInstance)
		{
			// pad: ensure no node gets address 0 which is reserved to mean
			// the stop state w/ no arcs
			posWrite = 1;
		}

		void writeByte(uint8_t b)
		{
			size_t bytesLength = _outerInstance->bytes.size();
			BOOST_ASSERT(posWrite <= bytesLength);

			if (bytesLength == posWrite)
			{
				_outerInstance->bytes.resize(static_cast<int>(static_cast<double>(bytesLength) * 1.5));
			}

			BOOST_ASSERT(posWrite < _outerInstance->bytes.size());
			_outerInstance->bytes[posWrite++] = b;
		}

		virtual void setPosWrite(int posWrite)
		{
			this->posWrite = posWrite;
			size_t bytesLength = _outerInstance->bytes.size();

			if (bytesLength < posWrite)
			{
				_outerInstance->bytes.resize(posWrite);
			}
		}

		void writeBytes(const uint8_t* b, int32_t offset, int32_t length)
		{
			const int size = posWrite + length;
			_outerInstance->bytes.resize(size);
			MiscUtils::arrayCopy(b, offset, _outerInstance->bytes.begin(), posWrite, length);
			posWrite += length;
		}

		virtual void close(){/* do nothing */};
	};

public:
	DECLARE_SHARED_PTR(BytesReader)

	/// <summary>
	/// Reads the bytes from this FST.  Use {@link
	///  #getBytesReader(int)} to obtain an instance for this
	///  FST; re-use across calls (but only within a single
	///  thread) for better performance.
	/// </summary>
	class BytesReader : public DataInput
	{
		friend class FST<T>;

	protected:
		int pos = 0;
		std::vector<uint8_t> const bytes;

		BytesReader(const std::vector<uint8_t>& bytes, int pos) : pos(pos), bytes(bytes) {}
	public:
		LUCENE_CLASS(BytesReader);

		virtual void close(){/* do nothing */};
		virtual void skip(int byteCount) = 0;
		virtual void skip(int base, int byteCount) = 0;
	};

public:
	DECLARE_SHARED_PTR(ReverseBytesReader)

	class ReverseBytesReader final : public BytesReader
	{
	public:
		LUCENE_CLASS(ReverseBytesReader);
		ReverseBytesReader(const std::vector<uint8_t>& bytes, int pos) : BytesReader(bytes, pos) {}
		uint8_t readByte() { return BytesReader::bytes[this->pos--]; }
		void readBytes(uint8_t* b, int offset, int len)
		{
			for (int i = 0; i < len; i++)
			{
				b[offset + i] = BytesReader::bytes[this->pos--];
			}
		}

		void skip(int count) { this->pos -= count; }
		void skip(int base, int count) { this->pos = base - count; }
	};

public:
	DECLARE_SHARED_PTR(ForwardBytesReader)

	// TODO: can we use just ByteArrayDataInput...?  need to
	// add a .skipBytes to DataInput.. hmm and .setPosition
	class ForwardBytesReader final : public BytesReader
	{
	public:
		LUCENE_CLASS(ForwardBytesReader);
		ForwardBytesReader(const std::vector<uint8_t>& bytes, int pos) : BytesReader(bytes, pos) {}
		uint8_t readByte() { return BytesReader::bytes[this->pos++]; }
		void readBytes(uint8_t* b, int offset, int len)
		{
			MiscUtils::arrayCopy(BytesReader::bytes.data(), this->pos, b, offset, len);
			this->pos += len;
		}

		void skip(int count) { this->pos += count; }
		void skip(int base, int count) { this->pos = base + count; }
	};

public:
	BytesReaderPtr getBytesReader(int pos)
	{
		// TODO: maybe re-use via ThreadLocal?
		if (packed)
		{
			return newLucene<ForwardBytesReader>(bytes, pos);
		}
		else
		{
			return newLucene<ReverseBytesReader>(bytes, pos);
		}
	}

private:
	static bool flag(int flags, int bit) { return (flags & bit) != 0; }
	const BytesWriterPtr writer;

	// TODO: we can save RAM here by using growable packed
	// ints...:
	std::vector<int> nodeAddress;

	// TODO: we could be smarter here, and prune periodically
	// as we go; high in-count nodes will "usually" become
	// clear early on:
	std::vector<int> inCounts;

public:
	// make a new empty FST, for building; Builder invokes this ctor
	FST(INPUT_TYPE inputType, const boost::shared_ptr<Outputs<T>>& outputs, bool willPackFST)
		: inputType(inputType), outputs(outputs), NO_OUTPUT(outputs->getNoOutput()), packed(false), writer(newLucene<BytesWriter>(this))
	{
		bytes = std::vector<uint8_t>(128);
		if (willPackFST)
		{
			nodeAddress = std::vector<int>(8);
			inCounts = std::vector<int>(8);
		}
		else
		{
			nodeAddress.clear();
			inCounts.clear();
		}

		emptyOutput = nullptr;
	}

	/// <summary>
	/// Load a previously saved FST. </summary>
	FST(const DataInputPtr& input, boost::shared_ptr<Outputs<T>> outputs)
		: inputType(INPUT_TYPE::BYTE1), outputs(outputs), NO_OUTPUT(outputs->getNoOutput())
	{
		// NOTE: only reads most recent format; we don't have
		// back-compat promise for FSTs (they are experimental):
		CodecUtil::checkHeader(*input, FILE_FORMAT_NAME, VERSION_PACKED, VERSION_PACKED);

		packed = input->readByte() == 1;
		if (input->readByte() == 1)
		{
			// accepts empty string
			int numBytes = input->readVInt();
			// messy
			bytes = std::vector<uint8_t>(numBytes);
			input->readBytes(bytes.data(), 0, numBytes);
			if (packed)
			{
				emptyOutput = outputs->read(getBytesReader(0));
			}
			else
			{
				emptyOutput = outputs->read(getBytesReader(numBytes - 1));
			}
		}
		else
		{
			emptyOutput = nullptr;
		}
		const char t = input->readByte();
		switch (t)
		{
			case 0:
				break;
			case 1:
				inputType = INPUT_TYPE::BYTE2;
				break;
			case 2:
				inputType = INPUT_TYPE::BYTE4;
				break;
			default:
				boost::throw_exception(IllegalStateException(L"invalid input type " + std::to_wstring(t)));
		}
		if (packed)
		{
			const int nodeRefCount = input->readVInt();
			nodeRefToAddress = std::vector<int>(nodeRefCount);
			for (int idx = 0; idx < nodeRefCount; idx++)
			{
				nodeRefToAddress[idx] = input->readVInt();
			}
		}
		else
		{
			nodeRefToAddress.clear();
		}
		startNode = input->readVInt();
		nodeCount = input->readVInt();
		arcCount = input->readVInt();
		arcWithOutputCount = input->readVInt();

		bytes = std::vector<uint8_t>(input->readVInt());
		input->readBytes(bytes.data(), 0, bytes.size());

		cacheRootArcs();
	}

	INPUT_TYPE getInputType() { return inputType; }
	/// <summary>
	/// Returns bytes used to represent the FST
	/// </summary>
	int sizeInBytes()
	{
		int size = bytes.size();
		if (packed)
		{
			//	  size += nodeRefToAddress.size() * RamUsageEstimator::NUM_BYTES_INT;
			size += nodeRefToAddress.size() * 4;
		}
		else if (nodeAddress.size() > 0)
		{
			//	  size += nodeAddress.size() * RamUsageEstimator::NUM_BYTES_INT;
			//	  size += inCounts.size() * RamUsageEstimator::NUM_BYTES_INT;
			size += nodeAddress.size() * 4;
			size += inCounts.size() * 4;
		}
		return size;
	}

	void finish(int startNode)
	{
		if (startNode == FINAL_END_NODE && emptyOutput != nullptr)
		{
			startNode = 0;
		}
		if (this->startNode != -1)
		{
			boost::throw_exception(IllegalStateException(L"already finished"));
		}
		std::vector<uint8_t> finalBytes(writer->posWrite);
		MiscUtils::arrayCopy(bytes.data(), 0, finalBytes.data(), 0, writer->posWrite);
		bytes = finalBytes;
		this->startNode = startNode;

		cacheRootArcs();
	}

private:
	int getNodeAddress(int node)
	{
		if (nodeAddress.size() > 0)
		{
			// Deref
			return nodeAddress[node];
		}
		else
		{
			// Straight
			return node;
		}
	}

	// Caches first 128 labels
	void cacheRootArcs()
	{
		cachedRootArcs.resize(128);
		boost::shared_ptr<Arc<T>> const arc = newLucene<Arc<T>>();
		getFirstArc(arc);
		auto const reader = getBytesReader(0);
		if (targetHasArcs(arc))
		{
			readFirstRealTargetArc(arc->target, arc, reader);
			while (true)
			{
				BOOST_ASSERT(arc->label != END_LABEL);
				if (arc->label < cachedRootArcs.size())
				{
					cachedRootArcs[arc->label] = (boost::make_shared<Arc<T>>())->copyFrom(arc);
				}
				else
				{
					break;
				}
				if (arc->isLast())
				{
					break;
				}
				readNextRealArc(arc, reader);
			}
		}
	}

public:
	T getEmptyOutput() { return emptyOutput; }
	void setEmptyOutput(T v)
	{
		if (emptyOutput != nullptr)
		{
			emptyOutput = outputs->merge(emptyOutput, v);
		}
		else
		{
			emptyOutput = v;
		}

		// TODO: this is messy -- replace with sillyBytesWriter; maybe make
		// bytes private
		const int posSave = writer->posWrite;
		outputs->write(emptyOutput, writer);
		emptyOutputBytes = std::vector<uint8_t>(writer->posWrite - posSave);

		if (!packed)
		{
			// reverse
			const int stopAt = (writer->posWrite - posSave) / 2;
			int upto = 0;
			while (upto < stopAt)
			{
				const auto b = bytes[posSave + upto];
				bytes[posSave + upto] = bytes[writer->posWrite - upto - 1];
				bytes[writer->posWrite - upto - 1] = b;
				upto++;
			}
		}
		MiscUtils::arrayCopy(bytes.data(), posSave, emptyOutputBytes.data(), 0, writer->posWrite - posSave);
		writer->posWrite = posSave;
	}

	void save(boost::shared_ptr<DataOutput> out)
	{
		if (startNode == -1)
		{
			boost::throw_exception(IllegalStateException(L"call finish first"));
		}
		if (nodeAddress.size() > 0)
		{
			boost::throw_exception(IllegalStateException(L"cannot save an FST pre-packed FST; it must first be packed"));
		}
		CodecUtil::writeHeader(out, FILE_FORMAT_NAME, VERSION_CURRENT);
		if (packed)
		{
			out->writeByte(static_cast<uint8_t>(1));
		}
		else
		{
			out->writeByte(static_cast<uint8_t>(0));
		}
		// TODO: really we should encode this as an arc, arriving
		// to the root node, instead of special casing here:
		if (emptyOutput != nullptr)
		{
			out->writeByte(static_cast<uint8_t>(1));
			out->writeVInt(emptyOutputBytes.size());
			out->writeBytes(emptyOutputBytes, 0, emptyOutputBytes.size());
		}
		else
		{
			out->writeByte(static_cast<uint8_t>(0));
		}
		char t;
		if (inputType == INPUT_TYPE::BYTE1)
		{
			t = 0;
		}
		else if (inputType == INPUT_TYPE::BYTE2)
		{
			t = 1;
		}
		else
		{
			t = 2;
		}
		out->writeByte(t);
		if (packed)
		{
			BOOST_ASSERT(nodeRefToAddress.size() > 0);
			out->writeVInt(nodeRefToAddress.size());
			for (int idx = 0; idx < nodeRefToAddress.size(); idx++)
			{
				out->writeVInt(nodeRefToAddress[idx]);
			}
		}
		out->writeVInt(startNode);
		out->writeVInt(nodeCount);
		out->writeVInt(arcCount);
		out->writeVInt(arcWithOutputCount);
		out->writeVInt(bytes.size());
		out->writeBytes(bytes, 0, bytes.size());
	}

	/// <summary>
	/// Writes an automaton to a file.
	/// </summary>
	void save(const String& file)
	{
		bool success = false;
		boost::shared_ptr<DataOutput> os = boost::make_shared<SimpleFSIndexOutput>(file);
		LuceneException finally;

		try
		{
			save(os);
			success = true;
		}
		catch (LuceneException& e)
		{
			finally = e;
		}

		if (os)
		{
			os->close();
		}

		finally.throwException();
	}

	/// <summary>
	/// Reads an automaton from a file.
	/// </summary>
	//  template<typename T>
	static boost::shared_ptr<FST<T>> read(const String& file, boost::shared_ptr<Outputs<T>> outputs)
	{
		DataInputPtr is =
			boost::make_shared<SimpleFSIndexInput>(file, BufferedIndexInput::BUFFER_SIZE, FSDirectory::DEFAULT_READ_CHUNK_SIZE);
		bool success = false;
		LuceneException finally;

		try
		{
			boost::shared_ptr<FST<T>> fst = boost::make_shared<FST<T>>(is, outputs);
			success = true;
			return fst;
		}
		catch (LuceneException& e)
		{
			finally = e;
		}

		if (is)
		{
			is->close();
		}

		finally.throwException();
	}

private:
	void writeLabel(int v)
	{
		BOOST_ASSERT(v >= 0);
		if (inputType == INPUT_TYPE::BYTE1)
		{
			BOOST_ASSERT(v <= 255);
			writer->writeByte(static_cast<uint8_t>(v));
		}
		else if (inputType == INPUT_TYPE::BYTE2)
		{
			BOOST_ASSERT(v <= 65535);
			writer->writeShort(static_cast<short>(v));
		}
		else
		{
			// writeInt(v);
			writer->writeVInt(v);
		}
	}

public:
	int readLabel(DataInputPtr input)
	{
		int v;
		if (inputType == INPUT_TYPE::BYTE1)
		{
			// Unsigned byte:
			v = input->readByte() & 0xFF;
		}
		else if (inputType == INPUT_TYPE::BYTE2)
		{
			// Unsigned short:
			v = input->readShort() & 0xFFFF;
		}
		else
		{
			v = input->readVInt();
		}
		return v;
	}

	// returns true if the node at this address has any
	// outgoing arcs
	//  template<typename T>
	static bool targetHasArcs(boost::shared_ptr<Arc<T>> arc) { return arc->target > 0; }
	// serializes new node by appending its bytes to the end
	// of the current byte[]
	int addNode(boost::shared_ptr<typename Builder<T>::template UnCompiledNode<T>> nodeIn)
	{
		// System.out.println("FST.addNode pos=" + writer.posWrite + " numArcs=" + nodeIn.numArcs);
		if (nodeIn->numArcs == 0)
		{
			if (nodeIn->isFinal)
			{
				return FINAL_END_NODE;
			}
			else
			{
				return NON_FINAL_END_NODE;
			}
		}

		int startAddress = writer->posWrite;
		// System.out.println("  startAddr=" + startAddress);

		const bool doFixedArray = shouldExpand(nodeIn);
		int fixedArrayStart;
		if (doFixedArray)
		{
			if (bytesPerArc.size() < nodeIn->numArcs)
			{
				bytesPerArc.resize(nodeIn->numArcs);
			}

			// write a "false" first arc:
			writer->writeByte(ARCS_AS_FIXED_ARRAY);
			writer->writeVInt(nodeIn->numArcs);

			// placeholder -- we'll come back and write the number
			// of bytes per arc (int) here:
			// TODO: we could make this a vInt instead
			writer->writeInt(0);
			fixedArrayStart = writer->posWrite;
			// System.out.println("  do fixed arcs array arcsStart=" + fixedArrayStart);
		}
		else
		{
			fixedArrayStart = 0;
		}

		arcCount += nodeIn->numArcs;

		const int lastArc = nodeIn->numArcs - 1;

		int lastArcStart = writer->posWrite;
		int maxBytesPerArc = 0;
		for (int arcIdx = 0; arcIdx < nodeIn->numArcs; arcIdx++)
		{
			auto arc = nodeIn->arcs[arcIdx];
			auto const target = boost::static_pointer_cast<typename Builder<T>::CompiledNode>(arc->target);
			int flags = 0;

			if (arcIdx == lastArc)
			{
				flags += BIT_LAST_ARC;
			}

			if (lastFrozenNode == target->node && !doFixedArray)
			{
				// TODO: for better perf (but more RAM used) we
				// could avoid this except when arc is "near" the
				// last arc:
				flags += BIT_TARGET_NEXT;
			}

			if (arc->isFinal)
			{
				flags += BIT_FINAL_ARC;
				if (arc->nextFinalOutput != NO_OUTPUT)
				{
					flags += BIT_ARC_HAS_FINAL_OUTPUT;
				}
			}
			else
			{
				BOOST_ASSERT(arc->nextFinalOutput == NO_OUTPUT);
			}

			bool targetHasArcs = target->node > 0;

			if (!targetHasArcs)
			{
				flags += BIT_STOP_NODE;
			}
			else if (inCounts.size() > 0)
			{
				inCounts[target->node]++;
			}

			if (arc->output != NO_OUTPUT)
			{
				flags += BIT_ARC_HAS_OUTPUT;
			}

			writer->writeByte(static_cast<uint8_t>(flags));
			writeLabel(arc->label);

			// System.out.println("  write arc: label=" + (char) arc.label + " flags=" + flags + " target=" + target.node + " pos=" +
			// writer->posWrite + " output=" + outputs.outputToString(arc.output);

			if (arc->output != NO_OUTPUT)
			{
				outputs->write(arc->output, writer);
				// System.out.println("    write output");
				arcWithOutputCount++;
			}

			if (arc->nextFinalOutput != NO_OUTPUT)
			{
				// System.out.println("    write final output");
				outputs->write(arc->nextFinalOutput, writer);
			}

			if (targetHasArcs && (flags & BIT_TARGET_NEXT) == 0)
			{
				BOOST_ASSERT(target->node > 0);
				// System.out.println("    write target");
				writer->writeInt(target->node);
			}

			// just write the arcs "like normal" on first pass,
			// but record how many bytes each one took, and max
			// byte size:
			if (doFixedArray)
			{
				bytesPerArc[arcIdx] = writer->posWrite - lastArcStart;
				lastArcStart = writer->posWrite;
				maxBytesPerArc = std::max(maxBytesPerArc, bytesPerArc[arcIdx]);
				// System.out.println("    bytes=" + bytesPerArc[arcIdx]);
			}
		}

		// TODO: if arc'd arrays will be "too wasteful" by some
		// measure, eg if arcs have vastly different sized
		// outputs, then we should selectively disable array for
		// such cases

		if (doFixedArray)
		{
			// System.out.println("  doFixedArray");
			BOOST_ASSERT(maxBytesPerArc > 0);
			// 2nd pass just "expands" all arcs to take up a fixed
			// byte size
			const int sizeNeeded = fixedArrayStart + nodeIn->numArcs * maxBytesPerArc;
			if (bytes.size() < sizeNeeded)
			{
				bytes.resize(MiscUtils::getNextSize(sizeNeeded));
			}

			// TODO: we could make this a vInt instead
			bytes[fixedArrayStart - 4] = static_cast<uint8_t>(maxBytesPerArc >> 24);
			bytes[fixedArrayStart - 3] = static_cast<uint8_t>(maxBytesPerArc >> 16);
			bytes[fixedArrayStart - 2] = static_cast<uint8_t>(maxBytesPerArc >> 8);
			bytes[fixedArrayStart - 1] = static_cast<uint8_t>(maxBytesPerArc);

			// expand the arcs in place, backwards
			int srcPos = writer->posWrite;
			int destPos = fixedArrayStart + nodeIn->numArcs * maxBytesPerArc;
			writer->posWrite = destPos;
			for (int arcIdx = nodeIn->numArcs - 1; arcIdx >= 0; arcIdx--)
			{
				// System.out.println("  repack arcIdx=" + arcIdx + " srcPos=" + srcPos + " destPos=" + destPos);
				destPos -= maxBytesPerArc;
				srcPos -= bytesPerArc[arcIdx];
				if (srcPos != destPos)
				{
					BOOST_ASSERT(destPos > srcPos);
					MiscUtils::arrayCopy(bytes.data(), srcPos, bytes.data(), destPos, bytesPerArc[arcIdx]);
				}
			}
		}

		// reverse bytes in-place; we do this so that the
		// "BIT_TARGET_NEXT" opto can work, ie, it reads the
		// node just before the current one
		const int endAddress = writer->posWrite - 1;

		int left = startAddress;
		int right = endAddress;
		while (left < right)
		{
			const uint8_t b = bytes[left];
			bytes[left++] = bytes[right];
			bytes[right--] = b;
		}
		// System.out.println("  endAddress=" + endAddress);

		nodeCount++;
		int node;
		if (nodeAddress.size() > 0)
		{
			// Nodes are addressed by 1+ord:
			if (nodeCount == nodeAddress.size())
			{
				nodeAddress.resize(MiscUtils::getNextSize(nodeAddress.size() + 1));
				inCounts.resize(MiscUtils::getNextSize(inCounts.size() + 1));
			}

			nodeAddress[nodeCount] = endAddress;
			// System.out.println("  write nodeAddress[" + nodeCount + "] = " + endAddress);
			node = nodeCount;
		}
		else
		{
			node = endAddress;
		}

		lastFrozenNode = node;

		return node;
	}

	/// <summary>
	/// Fills virtual 'start' arc, ie, an empty incoming arc to
	///  the FST's start node
	/// </summary>
	boost::shared_ptr<Arc<T>> getFirstArc(boost::shared_ptr<Arc<T>> arc)
	{
		if (emptyOutput != nullptr)
		{
			arc->flags = BIT_FINAL_ARC | BIT_LAST_ARC;
			arc->nextFinalOutput = emptyOutput;
		}
		else
		{
			arc->flags = BIT_LAST_ARC;
			arc->nextFinalOutput = NO_OUTPUT;
		}
		arc->output = NO_OUTPUT;

		// If there are no nodes, ie, the FST only accepts the
		// empty string, then startNode is 0
		arc->target = startNode;
		return arc;
	}

	/// <summary>
	/// Follows the <code>follow</code> arc and reads the last
	///  arc of its target; this changes the provided
	///  <code>arc</code> (2nd arg) in-place and returns it.
	/// </summary>
	/// <returns> Returns the second argument
	/// (<code>arc</code>).  </returns>
	boost::shared_ptr<Arc<T>> readLastTargetArc(boost::shared_ptr<Arc<T>> follow, boost::shared_ptr<Arc<T>> arc)
	{
		// System.out.println("readLast");
		if (!targetHasArcs(follow))
		{
			// System.out.println("  end node");
			BOOST_ASSERT(follow->isFinal());
			arc->label = END_LABEL;
			arc->target = FINAL_END_NODE;
			arc->output = follow->nextFinalOutput;
			arc->flags = BIT_LAST_ARC;
			return arc;
		}
		else
		{
			auto const reader = getBytesReader(getNodeAddress(follow->target));
			arc->node = follow->target;
			constexpr char b = reader->readByte();
			if (b == ARCS_AS_FIXED_ARRAY)
			{
				// array: jump straight to end
				arc->numArcs = reader->readVInt();
				if (packed)
				{
					arc->bytesPerArc = reader->readVInt();
				}
				else
				{
					arc->bytesPerArc = reader->readInt();
				}
				// System.out.println("  array numArcs=" + arc.numArcs + " bpa=" + arc.bytesPerArc);
				arc->posArcsStart = reader->pos;
				arc->arcIdx = arc->numArcs - 2;
			}
			else
			{
				arc->flags = b;
				// non-array: linear scan
				arc->bytesPerArc = 0;
				// System.out.println("  scan");
				while (!arc->isLast())
				{
					// skip this arc:
					readLabel(reader);
					if (arc->flag(BIT_ARC_HAS_OUTPUT))
					{
						outputs->read(reader);
					}
					if (arc->flag(BIT_ARC_HAS_FINAL_OUTPUT))
					{
						outputs->read(reader);
					}
					if (arc->flag(BIT_STOP_NODE))
					{
					}
					else if (arc->flag(BIT_TARGET_NEXT))
					{
					}
					else
					{
						if (packed)
						{
							reader->readVInt();
						}
						else
						{
							reader->skip(4);
						}
					}
					arc->flags = reader->readByte();
				}
				// Undo the byte flags we read:
				reader->skip(-1);
				arc->nextArc = reader->pos;
			}
			readNextRealArc(arc, reader);
			BOOST_ASSERT(arc->isLast());
			return arc;
		}
	}

	/// <summary>
	/// Follow the <code>follow</code> arc and read the first arc of its target;
	/// this changes the provided <code>arc</code> (2nd arg) in-place and returns
	/// it.
	/// </summary>
	/// <returns> Returns the second argument (<code>arc</code>). </returns>
	boost::shared_ptr<Arc<T>> readFirstTargetArc(boost::shared_ptr<Arc<T>> follow, boost::shared_ptr<Arc<T>> arc)
	{
		// int pos = address;
		// System.out.println("    readFirstTarget follow.target=" + follow.target + " isFinal=" + follow.isFinal());
		if (follow->isFinal())
		{
			// Insert "fake" final first arc:
			arc->label = END_LABEL;
			arc->output = follow->nextFinalOutput;
			arc->flags = BIT_FINAL_ARC;
			if (follow->target <= 0)
			{
				arc->flags |= BIT_LAST_ARC;
			}
			else
			{
				arc->node = follow->target;
				// NOTE: nextArc is a node (not an address!) in this case:
				arc->nextArc = follow->target;
			}
			arc->target = FINAL_END_NODE;
			// System.out.println("    insert isFinal; nextArc=" + follow.target + " isLast=" + arc.isLast() + " output=" +
			// outputs.outputToString(arc.output));
			return arc;
		}
		else
		{
			return readFirstRealTargetArc(follow->target, arc, getBytesReader(0));
		}
	}

	boost::shared_ptr<Arc<T>> readFirstRealTargetArc(int node, boost::shared_ptr<Arc<T>> arc, BytesReaderPtr reader)
	{
		BOOST_ASSERT(reader->bytes.size() == bytes.size());
		const int address = getNodeAddress(node);
		reader->pos = address;
		// System.out.println("  readFirstRealTargtArc address="
		//+ address);
		// System.out.println("   flags=" + arc.flags);
		arc->node = node;

		if (reader->readByte() == ARCS_AS_FIXED_ARRAY)
		{
			// System.out.println("  fixedArray");
			// this is first arc in a fixed-array
			arc->numArcs = reader->readVInt();
			if (packed)
			{
				arc->bytesPerArc = reader->readVInt();
			}
			else
			{
				arc->bytesPerArc = reader->readInt();
			}
			arc->arcIdx = -1;
			arc->nextArc = arc->posArcsStart = reader->pos;
			// System.out.println("  bytesPer=" + arc.bytesPerArc + " numArcs=" + arc.numArcs + " arcsStart=" + pos);
		}
		else
		{
			// arc.flags = b;
			arc->nextArc = address;
			arc->bytesPerArc = 0;
		}

		return readNextRealArc(arc, reader);
	}

	/// <summary>
	/// Checks if <code>arc</code>'s target state is in expanded (or vector) format.
	/// </summary>
	/// <returns> Returns <code>true</code> if <code>arc</code> points to a state in an
	/// expanded array format. </returns>
	bool isExpandedTarget(boost::shared_ptr<Arc<T>> follow)
	{
		if (!targetHasArcs(follow))
		{
			return false;
		}
		else
		{
			auto const reader = getBytesReader(getNodeAddress(follow->target));
			return reader->readByte() == ARCS_AS_FIXED_ARRAY;
		}
	}

	/// <summary>
	/// In-place read; returns the arc. </summary>
	boost::shared_ptr<Arc<T>> readNextArc(boost::shared_ptr<Arc<T>> arc)
	{
		if (arc->label == END_LABEL)
		{
			// This was a fake inserted "final" arc
			if (arc->nextArc <= 0)
			{
				boost::throw_exception(IllegalStateException(L"cannot readNextArc when arc.isLast()=true"));
			}
			return readFirstRealTargetArc(arc->nextArc, arc, getBytesReader(0));
		}
		else
		{
			return readNextRealArc(arc, getBytesReader(0));
		}
	}

	/// <summary>
	/// Peeks at next arc's label; does not alter arc.  Do
	///  not call this if arc.isLast()!
	/// </summary>
	int readNextArcLabel(boost::shared_ptr<Arc<T>> arc)
	{
		BOOST_ASSERT(!arc->isLast());

		BytesReaderPtr const reader;
		if (arc->label == END_LABEL)
		{
			// System.out.println("    nextArc fake " + arc.nextArc);
			reader = getBytesReader(getNodeAddress(arc->nextArc));
			constexpr char b = bytes[reader->pos];
			if (b == ARCS_AS_FIXED_ARRAY)
			{
				// System.out.println("    nextArc fake array");
				reader->skip(1);
				reader->readVInt();
				if (packed)
				{
					reader->readVInt();
				}
				else
				{
					reader->readInt();
				}
			}
		}
		else
		{
			if (arc->bytesPerArc != 0)
			{
				// System.out.println("    nextArc real array");
				// arcs are at fixed entries
				reader = getBytesReader(arc->posArcsStart);
				reader->skip((1 + arc->arcIdx) * arc->bytesPerArc);
			}
			else
			{
				// arcs are packed
				// System.out.println("    nextArc real packed");
				reader = getBytesReader(arc->nextArc);
			}
		}
		// skip flags
		reader->readByte();
		return readLabel(reader);
	}

	/// <summary>
	/// Never returns null, but you should never call this if
	///  arc.isLast() is true.
	/// </summary>
	boost::shared_ptr<Arc<T>> readNextRealArc(boost::shared_ptr<Arc<T>> arc, BytesReaderPtr reader)
	{
		BOOST_ASSERT(reader->bytes.size() == bytes.size());

		// TODO: can't assert this because we call from readFirstArc
		// assert !flag(arc.flags, BIT_LAST_ARC);

		// this is a continuing arc in a fixed array
		if (arc->bytesPerArc != 0)
		{
			// arcs are at fixed entries
			arc->arcIdx++;
			BOOST_ASSERT(arc->arcIdx < arc->numArcs);
			reader->skip(arc->posArcsStart, arc->arcIdx * arc->bytesPerArc);
		}
		else
		{
			// arcs are packed
			reader->pos = arc->nextArc;
		}
		arc->flags = reader->readByte();
		arc->label = readLabel(reader);

		if (arc->flag(BIT_ARC_HAS_OUTPUT))
		{
			arc->output = outputs->read(reader);
		}
		else
		{
			arc->output = outputs->getNoOutput();
		}

		if (arc->flag(BIT_ARC_HAS_FINAL_OUTPUT))
		{
			arc->nextFinalOutput = outputs->read(reader);
		}
		else
		{
			arc->nextFinalOutput = outputs->getNoOutput();
		}

		if (arc->flag(BIT_STOP_NODE))
		{
			if (arc->flag(BIT_FINAL_ARC))
			{
				arc->target = FINAL_END_NODE;
			}
			else
			{
				arc->target = NON_FINAL_END_NODE;
			}
			arc->nextArc = reader->pos;
		}
		else if (arc->flag(BIT_TARGET_NEXT))
		{
			arc->nextArc = reader->pos;
			// TODO: would be nice to make this lazy -- maybe
			// caller doesn't need the target and is scanning arcs...
			if (nodeAddress.empty())
			{
				if (!arc->flag(BIT_LAST_ARC))
				{
					if (arc->bytesPerArc == 0)
					{
						// must scan
						seekToNextNode(reader);
					}
					else
					{
						reader->skip(arc->posArcsStart, arc->bytesPerArc * arc->numArcs);
					}
				}
				arc->target = reader->pos;
			}
			else
			{
				arc->target = arc->node - 1;
				BOOST_ASSERT(arc->target > 0);
			}
		}
		else
		{
			if (packed)
			{
				const int pos = reader->pos;
				const int code = reader->readVInt();
				if (arc->flag(BIT_TARGET_DELTA))
				{
					// Address is delta-coded from current address:
					arc->target = pos + code;
					// System.out.println("    delta pos=" + pos + " delta=" + code + " target=" + arc.target);
				}
				else if (code < nodeRefToAddress.size())
				{
					// Deref
					arc->target = nodeRefToAddress[code];
					// System.out.println("    deref code=" + code + " target=" + arc.target);
				}
				else
				{
					// Absolute
					arc->target = code;
					// System.out.println("    abs code=" + code + " derefLen=" + nodeRefToAddress.length);
				}
			}
			else
			{
				arc->target = reader->readInt();
			}
			arc->nextArc = reader->pos;
		}
		return arc;
	}

	/// <summary>
	/// Finds an arc leaving the incoming arc, replacing the arc in place.
	///  This returns null if the arc was not found, else the incoming arc.
	/// </summary>
	boost::shared_ptr<Arc<T>> findTargetArc(
		int labelToMatch, boost::shared_ptr<Arc<T>> follow, boost::shared_ptr<Arc<T>> arc, BytesReaderPtr reader)
	{
		BOOST_ASSERT(cachedRootArcs.size() > 0);
		BOOST_ASSERT(reader->bytes.size() == bytes.size());

		if (labelToMatch == END_LABEL)
		{
			if (follow->isFinal())
			{
				if (follow->target <= 0)
				{
					arc->flags = BIT_LAST_ARC;
				}
				else
				{
					arc->flags = 0;
					// NOTE: nextArc is a node (not an address!) in this case:
					arc->nextArc = follow->target;
					arc->node = follow->target;
				}
				arc->output = follow->nextFinalOutput;
				arc->label = END_LABEL;
				return arc;
			}
			else
			{
				return nullptr;
			}
		}

		// Short-circuit if this arc is in the root arc cache:
		if (follow->target == startNode && labelToMatch < cachedRootArcs.size())
		{
			boost::shared_ptr<Arc<T>> const result = cachedRootArcs[labelToMatch];
			if (result == nullptr)
			{
				return result;
			}
			else
			{
				arc->copyFrom(result);
				return arc;
			}
		}

		if (!targetHasArcs(follow))
		{
			return nullptr;
		}

		reader->pos = getNodeAddress(follow->target);

		arc->node = follow->target;

		// System.out.println("fta label=" + (char) labelToMatch);

		if (reader->readByte() == ARCS_AS_FIXED_ARRAY)
		{
			// Arcs are full array; do binary search:
			arc->numArcs = reader->readVInt();
			if (packed)
			{
				arc->bytesPerArc = reader->readVInt();
			}
			else
			{
				arc->bytesPerArc = reader->readInt();
			}
			arc->posArcsStart = reader->pos;
			int low = 0;
			int high = arc->numArcs - 1;
			while (low <= high)
			{
				// System.out.println("    cycle");
				int mid = static_cast<int>(static_cast<unsigned int>((low + high)) >> 1);
				reader->skip(arc->posArcsStart, arc->bytesPerArc * mid + 1);
				int midLabel = readLabel(reader);
				const int cmp = midLabel - labelToMatch;
				if (cmp < 0)
				{
					low = mid + 1;
				}
				else if (cmp > 0)
				{
					high = mid - 1;
				}
				else
				{
					arc->arcIdx = mid - 1;
					// System.out.println("    found!");
					return readNextRealArc(arc, reader);
				}
			}

			return nullptr;
		}

		// Linear scan
		readFirstRealTargetArc(follow->target, arc, reader);

		while (true)
		{
			// System.out.println("  non-bs cycle");
			// TODO: we should fix this code to not have to create
			// object for the output of every arc we scan... only
			// for the matching arc, if found
			if (arc->label == labelToMatch)
			{
				// System.out.println("    found!");
				return arc;
			}
			else if (arc->label > labelToMatch)
			{
				return nullptr;
			}
			else if (arc->isLast())
			{
				return nullptr;
			}
			else
			{
				readNextRealArc(arc, reader);
			}
		}
	}

private:
	void seekToNextNode(BytesReaderPtr reader)
	{
		while (true)
		{
			const int flags = reader->readByte();
			readLabel(reader);

			if (flag(flags, BIT_ARC_HAS_OUTPUT))
			{
				outputs->read(reader);
			}

			if (flag(flags, BIT_ARC_HAS_FINAL_OUTPUT))
			{
				outputs->read(reader);
			}

			if (!flag(flags, BIT_STOP_NODE) && !flag(flags, BIT_TARGET_NEXT))
			{
				if (packed)
				{
					reader->readVInt();
				}
				else
				{
					reader->readInt();
				}
			}

			if (flag(flags, BIT_LAST_ARC))
			{
				return;
			}
		}
	}

public:
	int getNodeCount()
	{
		// 1+ in order to count the -1 implicit final node
		return 1 + nodeCount;
	}

	int getArcCount() { return arcCount; }
	int getArcWithOutputCount() { return arcWithOutputCount; }
	void setAllowArrayArcs(bool v) { allowArrayArcs = v; }
	/// <summary>
	/// Nodes will be expanded if their depth (distance from the root node) is
	/// &lt;= this value and their number of arcs is &gt;=
	/// <seealso cref="#FIXED_ARRAY_NUM_ARCS_SHALLOW"/>.
	///
	/// <para>
	/// Fixed array consumes more RAM but enables binary search on the arcs
	/// (instead of a linear scan) on lookup by arc label.
	///
	/// </para>
	/// </summary>
	/// <returns> <code>true</code> if <code>node</code> should be stored in an
	///         expanded (array) form.
	/// </returns>
	/// <seealso cref= #FIXED_ARRAY_NUM_ARCS_DEEP </seealso>
	/// <seealso cref= Builder.UnCompiledNode#depth </seealso>
private:
	bool shouldExpand(boost::shared_ptr<typename Builder<T>::template UnCompiledNode<T>> node)
	{
		return allowArrayArcs && ((node->depth <= FIXED_ARRAY_SHALLOW_DISTANCE && node->numArcs >= FIXED_ARRAY_NUM_ARCS_SHALLOW) ||
								  node->numArcs >= FIXED_ARRAY_NUM_ARCS_DEEP);
	}

private:
	//  template<typename TS>
	//  class ArcAndState : public std::enable_shared_from_this<ArcAndState<TS>>
	//  {
	// public:
	//	const boost::shared_ptr<Arc<TS>> arc;
	//	const boost::shared_ptr<IntsRef> chain;
	//
	//	ArcAndState(boost::shared_ptr<Arc<TS>> arc, boost::shared_ptr<IntsRef> chain) : arc(arc), chain(chain)
	//	{
	//	}
	//  };

	/*
	public void countSingleChains() throws IOException {
	  // TODO: must assert this FST was built with
	  // "willRewrite"

	  final List<ArcAndState<T>> queue = new ArrayList<ArcAndState<T>>();

	  // TODO: use bitset to not revisit nodes already
	  // visited

	  FixedBitSet seen = new FixedBitSet(1+nodeCount);
	  int saved = 0;

	  queue.add(new ArcAndState<T>(getFirstArc(new Arc<T>()), new IntsRef()));
	  Arc<T> scratchArc = new Arc<T>();
	  while(queue.size() > 0) {
		//System.out.println("cycle size=" + queue.size());
		//for(ArcAndState<T> ent : queue) {
		//  System.out.println("  " + Util.toBytesRef(ent.chain, new BytesRef()));
		//  }
		final ArcAndState<T> arcAndState = queue.get(queue.size()-1);
		seen.set(arcAndState.arc.node);
		final BytesRef br = Util.toBytesRef(arcAndState.chain, new BytesRef());
		if (br.length > 0 && br.bytes[br.length-1] == -1) {
		  br.length--;
		}
		//System.out.println("  top node=" + arcAndState.arc.target + " chain=" + br.utf8ToString());
		if (targetHasArcs(arcAndState.arc) && !seen.get(arcAndState.arc.target)) {
		  // push
		  readFirstTargetArc(arcAndState.arc, scratchArc);
		  //System.out.println("  push label=" + (char) scratchArc.label);
		  //System.out.println("    tonode=" + scratchArc.target + " last?=" + scratchArc.isLast());

		  final IntsRef chain = IntsRef.deepCopyOf(arcAndState.chain);
		  chain.grow(1+chain.length);
		  // TODO
		  //assert scratchArc.label != END_LABEL;
		  chain.ints[chain.length] = scratchArc.label;
		  chain.length++;

		  if (scratchArc.isLast()) {
			if (scratchArc.target != -1 && inCounts[scratchArc.target] == 1) {
			  //System.out.println("    append");
			} else {
			  if (arcAndState.chain.length > 1) {
				saved += chain.length-2;
				try {
				  System.out.println("chain: " + Util.toBytesRef(chain, new BytesRef()).utf8ToString());
				} catch (AssertionError ae) {
				  System.out.println("chain: " + Util.toBytesRef(chain, new BytesRef()));
				}
			  }
			  chain.length = 0;
			}
		  } else {
			//System.out.println("    reset");
			if (arcAndState.chain.length > 1) {
			  saved += arcAndState.chain.length-2;
			  try {
				System.out.println("chain: " + Util.toBytesRef(arcAndState.chain, new BytesRef()).utf8ToString());
			  } catch (AssertionError ae) {
				System.out.println("chain: " + Util.toBytesRef(arcAndState.chain, new BytesRef()));
			  }
			}
			if (scratchArc.target != -1 && inCounts[scratchArc.target] != 1) {
			  chain.length = 0;
			} else {
			  chain.ints[0] = scratchArc.label;
			  chain.length = 1;
			}
		  }
		  // TODO: instead of new Arc() we can re-use from
		  // a by-depth array
		  queue.add(new ArcAndState<T>(new Arc<T>().copyFrom(scratchArc), chain));
		} else if (!arcAndState.arc.isLast()) {
		  // next
		  readNextArc(arcAndState.arc);
		  //System.out.println("  next label=" + (char) arcAndState.arc.label + " len=" + arcAndState.chain.length);
		  if (arcAndState.chain.length != 0) {
			arcAndState.chain.ints[arcAndState.chain.length-1] = arcAndState.arc.label;
		  }
		} else {
		  if (arcAndState.chain.length > 1) {
			saved += arcAndState.chain.length-2;
			System.out.println("chain: " + Util.toBytesRef(arcAndState.chain, new BytesRef()).utf8ToString());
		  }
		  // pop
		  //System.out.println("  pop");
		  queue.remove(queue.size()-1);
		  while(queue.size() > 0 && queue.get(queue.size()-1).arc.isLast()) {
			queue.remove(queue.size()-1);
		  }
		  if (queue.size() > 0) {
			final ArcAndState<T> arcAndState2 = queue.get(queue.size()-1);
			readNextArc(arcAndState2.arc);
			//System.out.println("  read next=" + (char) arcAndState2.arc.label + " queue=" + queue.size());
			assert arcAndState2.arc.label != END_LABEL;
			if (arcAndState2.chain.length != 0) {
			  arcAndState2.chain.ints[arcAndState2.chain.length-1] = arcAndState2.arc.label;
			}
		  }
		}
	  }

	  System.out.println("TOT saved " + saved);
	}
   */

private:
	// Creates a packed FST
	FST(INPUT_TYPE inputType, std::vector<int>& nodeRefToAddress, boost::shared_ptr<Outputs<T>> outputs)
		: inputType(inputType)
		, outputs(outputs)
		, NO_OUTPUT(outputs->getNoOutput())
		, packed(true)
		, nodeRefToAddress(nodeRefToAddress)
		, writer(newLucene<BytesWriter>(this))
	{
		bytes = std::vector<uint8_t>(128);
	}

public:
	/// <summary>
	/// Expert: creates an FST by packing this one.  This
	///  process requires substantial additional RAM (currently
	///  ~8 bytes per node), but then should produce a smaller FST.
	/// </summary>
	boost::shared_ptr<FST<T>> pack(int minInCountDeref, int maxDerefNodes)
	{
		// TODO: other things to try
		//   - renumber the nodes to get more next / better locality?
		//   - allow multiple input labels on an arc, so
		//     singular chain of inputs can take one arc (on
		//     wikipedia terms this could save another ~6%)
		//   - in the ord case, the output '1' is presumably
		//     very common (after NO_OUTPUT)... maybe use a bit
		//     for it..?
		//   - use spare bits in flags.... for top few labels /
		//     outputs / targets

		if (nodeAddress.empty())
		{
			boost::throw_exception(IllegalArgumentException(L"this FST was not built with willPackFST=true"));
		}

		boost::shared_ptr<Arc<T>> arc = boost::make_shared<Arc<T>>();

		auto const r = getBytesReader(0);

		constexpr int topN = std::min(maxDerefNodes, inCounts.size());

		// Find top nodes with highest number of incoming arcs:
		boost::shared_ptr<NodeQueue> q = boost::make_shared<NodeQueue>(topN);

		// TODO: we could use more RAM efficient selection algo here...
		boost::shared_ptr<NodeAndInCount> bottom = nullptr;
		for (int node = 0; node < inCounts.size(); node++)
		{
			if (inCounts[node] >= minInCountDeref)
			{
				if (bottom == nullptr)
				{
					q->add(boost::make_shared<NodeAndInCount>(node, inCounts[node]));
					if (q->size() == topN)
					{
						bottom = q->top();
					}
				}
				else if (inCounts[node] > bottom->count)
				{
					q->insertWithOverflow(boost::make_shared<NodeAndInCount>(node, inCounts[node]));
				}
			}
		}

		// Free up RAM:
		inCounts.clear();

		const std::unordered_map<int, int> topNodeMap = std::unordered_map<int, int>();
		for (int downTo = q->size() - 1; downTo >= 0; downTo--)
		{
			boost::shared_ptr<NodeAndInCount> n = q->pop();
			topNodeMap.emplace(n->node, downTo);
			// System.out.println("map node=" + n.node + " inCount=" + n.count + " to newID=" + downTo);
		}

		// TODO: we can use packed ints:
		// +1 because node ords start at 1 (0 is reserved as
		// stop node):
		std::vector<int> nodeRefToAddressIn = std::vector<int>(topNodeMap.size());

		boost::shared_ptr<FST<T>> const fst = boost::make_shared<FST<T>>(inputType, nodeRefToAddressIn, outputs);

		auto const writer = fst->writer;

		std::vector<int> newNodeAddress = std::vector<int>(1 + nodeCount);

		// Fill initial coarse guess:
		for (int node = 1; node <= nodeCount; node++)
		{
			newNodeAddress[node] = 1 + bytes.size() - nodeAddress[node];
		}

		int absCount;
		int deltaCount;
		int topCount;
		int nextCount;

		// Iterate until we converge:
		while (true)
		{
			// System.out.println("\nITER");
			bool changed = false;

			// for assert:
			bool negDelta = false;

			writer->posWrite = 0;
			// Skip 0 byte since 0 is reserved target:
			writer->writeByte(static_cast<uint8_t>(0));

			fst->arcWithOutputCount = 0;
			fst->nodeCount = 0;
			fst->arcCount = 0;

			absCount = deltaCount = topCount = nextCount = 0;

			int changedCount = 0;

			int addressError = 0;

			// int totWasted = 0;

			// Since we re-reverse the bytes, we now write the
			// nodes backwards, so that BIT_TARGET_NEXT is
			// unchanged:
			for (int node = nodeCount; node >= 1; node--)
			{
				fst->nodeCount++;
				constexpr int address = writer->posWrite;
				// System.out.println("  node: " + node + " address=" + address);
				if (address != newNodeAddress[node])
				{
					addressError = address - newNodeAddress[node];
					// System.out.println("    change: " + (address - newNodeAddress[node]));
					changed = true;
					newNodeAddress[node] = address;
					changedCount++;
				}

				int nodeArcCount = 0;
				int bytesPerArc = 0;

				bool retry = false;

				// for assert:
				bool anyNegDelta = false;

				// Retry loop: possibly iterate more than once, if
				// this is an array'd node and bytesPerArc changes:
				while (true)
				{ // retry writing this node

					readFirstRealTargetArc(node, arc, r);

					constexpr bool useArcArray = arc->bytesPerArc != 0;
					if (useArcArray)
					{
						// Write false first arc:
						if (bytesPerArc == 0)
						{
							bytesPerArc = arc->bytesPerArc;
						}
						writer->writeByte(ARCS_AS_FIXED_ARRAY);
						writer->writeVInt(arc->numArcs);
						writer->writeVInt(bytesPerArc);
						// System.out.println("node " + node + ": " + arc.numArcs + " arcs");
					}

					int maxBytesPerArc = 0;
					// int wasted = 0;
					while (true)
					{ // iterate over all arcs for this node

						// System.out.println("    arc label=" + arc.label + " target=" + arc.target + " pos=" + writer.posWrite);
						constexpr int arcStartPos = writer->posWrite;
						nodeArcCount++;

						char flags = 0;

						if (arc->isLast())
						{
							flags += BIT_LAST_ARC;
						}
						/*
						if (!useArcArray && nodeUpto < nodes.length-1 && arc.target == nodes[nodeUpto+1]) {
						  flags += BIT_TARGET_NEXT;
						}
						*/
						if (!useArcArray && node != 1 && arc->target == node - 1)
						{
							flags += BIT_TARGET_NEXT;
							if (!retry)
							{
								nextCount++;
							}
						}
						if (arc->isFinal())
						{
							flags += BIT_FINAL_ARC;
							if (arc->nextFinalOutput != NO_OUTPUT)
							{
								flags += BIT_ARC_HAS_FINAL_OUTPUT;
							}
						}
						else
						{
							BOOST_ASSERT(arc->nextFinalOutput == NO_OUTPUT);
						}
						if (!targetHasArcs(arc))
						{
							flags += BIT_STOP_NODE;
						}

						if (arc->output != NO_OUTPUT)
						{
							flags += BIT_ARC_HAS_OUTPUT;
						}

						std::unique_ptr<int> ptr;
						int absPtr;
						constexpr bool doWriteTarget = targetHasArcs(arc) && (flags & BIT_TARGET_NEXT) == 0;
						if (doWriteTarget)
						{
							ptr.reset(topNodeMap[arc->target]);
							if (ptr != nullptr)
							{
								absPtr = *ptr;
							}
							else
							{
								absPtr = topNodeMap.size() + newNodeAddress[arc->target] + addressError;
							}

							int delta = newNodeAddress[arc->target] + addressError - writer->posWrite - 2;
							if (delta < 0)
							{
								// System.out.println("neg: " + delta);
								anyNegDelta = true;
								delta = 0;
							}

							if (delta < absPtr)
							{
								flags |= BIT_TARGET_DELTA;
							}
						}
						else
						{
							ptr = nullptr;
							absPtr = 0;
						}

						writer->writeByte(flags);
						fst->writeLabel(arc->label);

						if (arc->output != NO_OUTPUT)
						{
							outputs->write(arc->output, writer);
							if (!retry)
							{
								fst->arcWithOutputCount++;
							}
						}
						if (arc->nextFinalOutput != NO_OUTPUT)
						{
							outputs->write(arc->nextFinalOutput, writer);
						}

						if (doWriteTarget)
						{
							int delta = newNodeAddress[arc->target] + addressError - writer->posWrite;
							if (delta < 0)
							{
								anyNegDelta = true;
								// System.out.println("neg: " + delta);
								delta = 0;
							}

							if (flag(flags, BIT_TARGET_DELTA))
							{
								// System.out.println("        delta");
								writer->writeVInt(delta);
								if (!retry)
								{
									deltaCount++;
								}
							}
							else
							{
								/*
								if (ptr != null) {
								  System.out.println("        deref");
								} else {
								  System.out.println("        abs");
								}
								*/
								writer->writeVInt(absPtr);
								if (!retry)
								{
									if (absPtr >= topNodeMap.size())
									{
										absCount++;
									}
									else
									{
										topCount++;
									}
								}
							}
						}

						if (useArcArray)
						{
							constexpr int arcBytes = writer->posWrite - arcStartPos;
							// System.out.println("  " + arcBytes + " bytes");
							maxBytesPerArc = std::max(maxBytesPerArc, arcBytes);
							// NOTE: this may in fact go "backwards", if
							// somehow (rarely, possibly never) we use
							// more bytesPerArc in this rewrite than the
							// incoming FST did... but in this case we
							// will retry (below) so it's OK to ovewrite
							// bytes:
							// wasted += bytesPerArc - arcBytes;
							writer->setPosWrite(arcStartPos + bytesPerArc);
						}

						if (arc->isLast())
						{
							break;
						}

						readNextRealArc(arc, r);
					}

					if (useArcArray)
					{
						if (maxBytesPerArc == bytesPerArc || (retry && maxBytesPerArc <= bytesPerArc))
						{
							// converged
							// System.out.println("  bba=" + bytesPerArc + " wasted=" + wasted);
							// totWasted += wasted;
							break;
						}
					}
					else
					{
						break;
					}

					// System.out.println("  retry this node maxBytesPerArc=" + maxBytesPerArc + " vs " + bytesPerArc);

					// Retry:
					bytesPerArc = maxBytesPerArc;
					writer->posWrite = address;
					nodeArcCount = 0;
					retry = true;
					anyNegDelta = false;
				}
				negDelta |= anyNegDelta;

				fst->arcCount += nodeArcCount;
			}

			if (!changed)
			{
				// We don't renumber the nodes (just reverse their
				// order) so nodes should only point forward to
				// other nodes because we only produce acyclic FSTs
				// w/ nodes only pointing "forwards":
				BOOST_ASSERT(!negDelta);
				// System.out.println("TOT wasted=" + totWasted);
				// Converged!
				break;
			}
			// System.out.println("  " + changedCount + " of " + fst.nodeCount + " changed; retry");
		}

		for (auto ent : topNodeMap)
		{
			nodeRefToAddressIn[ent.second] = newNodeAddress[ent.first];
		}

		fst->startNode = newNodeAddress[startNode];
		// System.out.println("new startNode=" + fst.startNode + " old startNode=" + startNode);

		if (emptyOutput != nullptr)
		{
			fst->setEmptyOutput(emptyOutput);
		}

		BOOST_ASSERT(fst->nodeCount == nodeCount);
		BOOST_ASSERT(fst->arcCount == arcCount);
		BOOST_ASSERT(fst->arcWithOutputCount == arcWithOutputCount);

		const std::vector<char> finalBytes = std::vector<char>(writer->posWrite);
		// System.out.println("resize " + fst.bytes.length + " down to " + writer.posWrite);
		MiscUtils::arrayCopy(fst->bytes, 0, finalBytes, 0, writer->posWrite);
		fst->bytes = finalBytes;
		fst->cacheRootArcs();

		// final int size = fst.sizeInBytes();
		// System.out.println("nextCount=" + nextCount + " topCount=" + topCount + " deltaCount=" + deltaCount + " absCount=" + absCount);

		return fst;
	}

private:
	DECLARE_SHARED_PTR(NodeAndInCount)

	class NodeAndInCount : public LuceneObject
	{
	public:
		LUCENE_CLASS(NodeAndInCount);

		const int node;
		const int count;

		NodeAndInCount(int node, int count) : node(node), count(count) {}
		virtual int compareTo(NodeAndInCountPtr other)
		{
			if (count > other->count)
			{
				return 1;
			}
			else if (count < other->count)
			{
				return -1;
			}
			else
			{
				// Tie-break: smaller node compares as greater than
				return other->node - node;
			}
		}

		virtual int32_t compareTo(const LuceneObjectPtr& other)
		{
			NodeAndInCountPtr otherNodeAndInCount(boost::dynamic_pointer_cast<NodeAndInCount>(other));
			return compareTo(otherNodeAndInCount);
		}
	};

private:
	DECLARE_SHARED_PTR(NodeQueue)

	class NodeQueue : public PriorityQueue<NodeAndInCountPtr>
	{
	public:
		LUCENE_CLASS(NodeQueue);
		NodeQueue(int topN) { PriorityQueue<NodeAndInCountPtr>::initialize(); }
	protected:
		virtual bool lessThan(const NodeAndInCountPtr& a, const NodeAndInCountPtr& b)
		{
			constexpr int cmp = a->compareTo(b);
			BOOST_ASSERT(cmp != 0);
			return cmp < 0;
		}
	};
};

#pragma NodeHash

// Used to dedup states (lookup already-frozen states)
template <typename T>
class NodeHash final : public LuceneObject
{
private:
	std::vector<int> _table;

	int _count = 0;
	int _mask = 0;
	const boost::shared_ptr<FST<T>> _fst;
	const boost::shared_ptr<typename FST<T>::template Arc<T>> _scratchArc = newLucene<typename FST<T>::template Arc<T>>();

public:
	LUCENE_CLASS(NodeHash<T>)
	NodeHash(boost::shared_ptr<FST<T>> fst) : _fst(fst)
	{
		_table = std::vector<int>(16);
		_mask = 15;
	}

private:
	bool nodesEqual(
		boost::shared_ptr<typename Builder<T>::template UnCompiledNode<T>> node,
		int address,
		boost::shared_ptr<typename FST<T>::BytesReader> reader)
	{
		_fst->readFirstRealTargetArc(address, _scratchArc, reader);
		if (_scratchArc->bytesPerArc != 0 && node->numArcs != _scratchArc->numArcs)
		{
			return false;
		}
		for (int arcUpto = 0; arcUpto < node->numArcs; arcUpto++)
		{
			auto const arc = node->arcs[arcUpto];
			if (arc->label != _scratchArc->label || !(arc->output == _scratchArc->output) ||
				(boost::static_pointer_cast<typename Builder<T>::CompiledNode>(arc->target))->node != _scratchArc->target ||
				!(arc->nextFinalOutput == _scratchArc->nextFinalOutput) || arc->isFinal != _scratchArc->isFinal())
			{
				return false;
			}

			if (_scratchArc->isLast())
			{
				if (arcUpto == node->numArcs - 1)
				{
					return true;
				}
				else
				{
					return false;
				}
			}
			_fst->readNextRealArc(_scratchArc, reader);
		}

		return false;
	}

	// hash code for an unfrozen node.  This must be identical
	// to the un-frozen case (below)!!
	int hash(boost::shared_ptr<typename Builder<T>::template UnCompiledNode<T>> node)
	{
		constexpr int PRIME = 31;
		// System.out.println("hash unfrozen");
		int h = 0;
		// TODO: maybe if number of arcs is high we can safely subsample?
		for (int arcIdx = 0; arcIdx < node->numArcs; arcIdx++)
		{
			auto const arc = node->arcs[arcIdx];
			// System.out.println("  label=" + arc.label + " target=" + ((Builder.CompiledNode) arc.target).node + " h=" + h + " output=" +
			// fst.outputs.outputToString(arc.output) + " isFinal?=" + arc.isFinal);
			h = PRIME * h + arc->label;
			h = PRIME * h + (boost::static_pointer_cast<typename Builder<T>::CompiledNode>(arc->target))->node;
			//			h = PRIME * h + arc->output.hashCode();
			//			h = PRIME * h + arc->nextFinalOutput.hashCode();
			h = PRIME * h + *(arc->output);
			h = PRIME * h + *(arc->nextFinalOutput);
			if (arc->isFinal)
			{
				h += 17;
			}
		}
		// System.out.println("  ret " + (h&Integer.MAX_VALUE));
		return h & std::numeric_limits<int>::max();
	}

	// hash code for a frozen node
	int hash(int node)
	{
		constexpr int PRIME = 31;
		auto const reader = _fst->getBytesReader(0);
		// System.out.println("hash frozen node=" + node);
		int h = 0;
		_fst->readFirstRealTargetArc(node, _scratchArc, reader);

		while (true)
		{
			// System.out.println("  label=" + scratchArc.label + " target=" + scratchArc.target + " h=" + h + " output=" +
			// fst.outputs.outputToString(scratchArc.output) + " next?=" + scratchArc.flag(4) + " final?=" + scratchArc.isFinal());
			h = PRIME * h + _scratchArc->label;
			h = PRIME * h + _scratchArc->target;
			//			h = PRIME * h + _scratchArc->output.hashCode();
			//			h = PRIME * h + _scratchArc->nextFinalOutput.hashCode();
			h = PRIME * h + *(_scratchArc->output);
			h = PRIME * h + *(_scratchArc->nextFinalOutput);

			if (_scratchArc->isFinal())
			{
				h += 17;
			}

			if (_scratchArc->isLast())
			{
				break;
			}

			_fst->readNextRealArc(_scratchArc, reader);
		}
		// System.out.println("  ret " + (h&Integer.MAX_VALUE));
		return h & std::numeric_limits<int>::max();
	}

public:
	int add(boost::shared_ptr<typename Builder<T>::template UnCompiledNode<T>> nodeIn)
	{
		// System.out.println("hash: add count=" + count + " vs " + table.length);
		auto const reader = _fst->getBytesReader(0);
		const int h = hash(nodeIn);
		int pos = h & _mask;
		int c = 0;
		while (true)
		{
			const int v = _table[pos];
			if (v == 0)
			{
				// freeze & add
				const int node = _fst->addNode(nodeIn);
				// System.out.println("  now freeze node=" + node);
				BOOST_ASSERT(hash(node) == h);
				_count++;
				_table[pos] = node;

				if (_table.size() < 2 * _count)
				{
					rehash();
				}

				return node;
			}
			else if (nodesEqual(nodeIn, v, reader))
			{
				// same node is already here
				return v;
			}

			// quadratic probe
			pos = (pos + (++c)) & _mask;
		}
	}

private:
	// called only by rehash
	void addNew(int address)
	{
		int pos = hash(address) & _mask;
		int c = 0;
		while (true)
		{
			if (_table[pos] == 0)
			{
				_table[pos] = address;
				break;
			}

			// quadratic probe
			pos = (pos + (++c)) & _mask;
		}
	}

	void rehash()
	{
		const std::vector<int> oldTable = _table;
		_table = std::vector<int>(2 * _table.size());
		_mask = _table.size() - 1;

		for (int idx = 0; idx < oldTable.size(); idx++)
		{
			const int address = oldTable[idx];
			if (address != 0)
			{
				addNew(address);
			}
		}
	}

public:
	int count() { return _count; }
};
}
}
}
