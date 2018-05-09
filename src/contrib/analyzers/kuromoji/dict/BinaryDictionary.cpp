#include "ContribInc.h"

#include "CodecUtil.h"
#include "FSDirectory.h"
#include "FileUtils.h"
#include "MiscUtils.h"
#include "_SimpleFSDirectory.h"
#include "kuromoji/dict/BinaryDictionary.h"

namespace Lucene {
namespace Analysis {
namespace Ja {
namespace Dict {

namespace {

short toInt16FromBigEndian(const ByteArray& bytes, int startPosition)
{
	return (short)(((unsigned short)(bytes[startPosition]) << 8) + (unsigned char)(bytes[startPosition + 1]));
}
}

const String BinaryDictionary::DICT_FILENAME_SUFFIX = L"_buffer.dat";
const String BinaryDictionary::TARGETMAP_FILENAME_SUFFIX = L"_targetMap.dat";
const String BinaryDictionary::POSDICT_FILENAME_SUFFIX = L"_posDict.dat";
const String BinaryDictionary::DICT_HEADER = L"kuromoji_dict";
const String BinaryDictionary::TARGETMAP_HEADER = L"kuromoji_dict_map";
const String BinaryDictionary::POSDICT_HEADER = L"kuromoji_dict_pos";

String BinaryDictionary::DICT_PATH;

BinaryDictionary::BinaryDictionary()
{
	DICT_PATH = GetDictionaryPath();
}

void BinaryDictionary::initializeDictionaries()
{
	std::unique_ptr<DataInput> mapIS;
	std::unique_ptr<DataInput> dictIS;
	std::unique_ptr<DataInput> posIS;

	LuceneException priorE;
	std::vector<int> targetMapOffsets;
	std::vector<int> targetMap;
	std::vector<String> posDict;
	std::vector<String> inflFormDict;
	std::vector<String> inflTypeDict;
	ByteArray buffer;

	try
	{
		const String targetMapFile = FileUtils::joinPath(DICT_PATH, getClassName() + TARGETMAP_FILENAME_SUFFIX);
		mapIS.reset(new SimpleFSIndexInput(targetMapFile, BufferedIndexInput::BUFFER_SIZE, FSDirectory::DEFAULT_READ_CHUNK_SIZE));
		Util::CodecUtil::checkHeader(*mapIS, TARGETMAP_HEADER, VERSION, VERSION);
		int accum = 0, sourceId = 0;
		targetMap.resize(mapIS->readVInt());
		targetMapOffsets.resize(mapIS->readVInt());

		for (int ofs = 0; ofs < targetMap.size(); ofs++)
		{
			const int val = mapIS->readVInt();
			if ((val & 0x01) != 0)
			{
				targetMapOffsets[sourceId] = ofs;
				sourceId++;
			}
			accum += static_cast<int>(static_cast<unsigned int>(val) >> 1);
			targetMap[ofs] = accum;
		}
		if (sourceId + 1 != targetMapOffsets.size())
		{
			boost::throw_exception(IOException(L"targetMap file format broken"));
		}

		targetMapOffsets[sourceId] = targetMap.size();
		mapIS->close();
		mapIS.reset();

		const String postDictFile = FileUtils::joinPath(DICT_PATH, getClassName() + POSDICT_FILENAME_SUFFIX);
		posIS.reset(new SimpleFSIndexInput(postDictFile, BufferedIndexInput::BUFFER_SIZE, FSDirectory::DEFAULT_READ_CHUNK_SIZE));
		Util::CodecUtil::checkHeader(*posIS, POSDICT_HEADER, VERSION, VERSION);
		int posSize = posIS->readVInt();
		posDict.resize(posSize);
		inflTypeDict.resize(posSize);
		inflFormDict.resize(posSize);

		for (int j = 0; j < posSize; j++)
		{
			posDict[j] = posIS->readString();
			inflTypeDict[j] = posIS->readString();
			inflFormDict[j] = posIS->readString();
			// this is how we encode null inflections
			if (inflTypeDict[j].length() == 0)
			{
				inflTypeDict[j] = L"";
			}
			if (inflFormDict[j].length() == 0)
			{
				inflFormDict[j] = L"";
			}
		}

		posIS->close();
		posIS.reset();

		const String dictFile = FileUtils::joinPath(DICT_PATH, getClassName() + DICT_FILENAME_SUFFIX);
		dictIS.reset(new SimpleFSIndexInput(dictFile, BufferedIndexInput::BUFFER_SIZE, FSDirectory::DEFAULT_READ_CHUNK_SIZE));
		// no buffering here, as we load in one large buffer
		Util::CodecUtil::checkHeader(*dictIS, DICT_HEADER, VERSION, VERSION);
		const int size = dictIS->readVInt();
		buffer = ByteArray::newInstance(size);

		dictIS->readBytes(buffer.get(), 0, buffer.size());
		if (buffer.size() != size)
		{
			boost::throw_exception(IOException(L"Cannot read whole dictionary"));
		}

		dictIS->close();
		dictIS.reset();
	}
	catch (const IOException& ioe)
	{
		priorE = ioe;
	}

	if (mapIS)
	{
		mapIS->close();
	}
	if (dictIS)
	{
		dictIS->close();
	}
	if (posIS)
	{
		posIS->close();
	}

	this->_targetMap = targetMap;
	this->_targetMapOffsets = targetMapOffsets;
	this->_posDict = posDict;
	this->_inflTypeDict = inflTypeDict;
	this->_inflFormDict = inflFormDict;
	this->_buffer = buffer;

	priorE.throwException();
}

void BinaryDictionary::lookupWordIds(int sourceId, OffsetAndLength& ref)
{
	ref.offset = _targetMapOffsets[sourceId];

	// targetMapOffsets always has one more entry pointing behind last:
	ref.length = _targetMapOffsets[sourceId + 1] - ref.offset;
}

int BinaryDictionary::getWordId(int offset)
{
	return _targetMap[offset];
}

int BinaryDictionary::getLeftId(int wordId)
{
	// false means from BigIndian Bits
	return static_cast<int>(static_cast<unsigned int>(toInt16FromBigEndian(_buffer, wordId)) >> 3);
}

int BinaryDictionary::getRightId(int wordId)
{
	// false means from BigIndian Bits
	return static_cast<int>(static_cast<unsigned int>(toInt16FromBigEndian(_buffer, wordId)) >> 3);
}

int BinaryDictionary::getWordCost(int wordId)
{
	return toInt16FromBigEndian(_buffer, wordId + 2); // Skip id
}

String BinaryDictionary::getBaseForm(int wordId, CharArray surfaceForm, int off, int len)
{
	if (hasBaseFormData(wordId))
	{
		int offset = baseFormOffset(wordId);
		int data = _buffer[offset++] & 0xff;
		int prefix = static_cast<int>(static_cast<unsigned int>(data) >> 4);
		int suffix = data & 0xF;

		CharArray text = CharArray::newInstance(prefix + suffix);
		MiscUtils::arrayCopy(surfaceForm.get(), off, text.get(), 0, prefix);

		for (int i = 0; i < suffix; i++)
		{
			text[prefix + i] = static_cast<wchar_t>(toInt16FromBigEndian(_buffer, offset + (i << 1)));
		}

		return String(text.get(), text.size());
	}
	else
	{
		return L"";
	}
}

String BinaryDictionary::getReading(int wordId, CharArray surface, int off, int len)
{
	if (hasReadingData(wordId))
	{
		int offset = readingOffset(wordId);
		int readingData = _buffer[offset++] & 0xff;
		return readString(offset, static_cast<int>(static_cast<unsigned int>(readingData) >> 1), (readingData & 1) == 1);
	}
	else
	{
		// the reading is the surface form, with hiragana shifted to katakana
		CharArray text = CharArray::newInstance(len);
		for (int i = 0; i < len; i++)
		{
			wchar_t ch = surface[off + i];
			if (ch > 0x3040 && ch < 0x3097)
			{
				text[i] = static_cast<wchar_t>(ch + 0x60);
			}
			else
			{
				text[i] = ch;
			}
		}
		return String(text.get(), text.size());
	}
}

String BinaryDictionary::getPartOfSpeech(int wordId)
{
	return _posDict[getLeftId(wordId)];
}

String BinaryDictionary::getPronunciation(int wordId, CharArray surface, int off, int len)
{
	if (hasPronunciationData(wordId))
	{
		int offset = pronunciationOffset(wordId);
		int pronunciationData = _buffer[offset++] & 0xff;
		return readString(offset, static_cast<int>(static_cast<unsigned int>(pronunciationData) >> 1), (pronunciationData & 1) == 1);
	}
	else
	{
		return getReading(wordId, surface, off, len); // same as the reading
	}
}

String BinaryDictionary::getInflectionType(int wordId)
{
	return _inflTypeDict[getLeftId(wordId)];
}

String BinaryDictionary::getInflectionForm(int wordId)
{
	return _inflFormDict[getLeftId(wordId)];
}

int BinaryDictionary::baseFormOffset(int wordId)
{
	return wordId + 4;
}

int BinaryDictionary::readingOffset(int wordId)
{
	int offset = baseFormOffset(wordId);
	if (hasBaseFormData(wordId))
	{
		int baseFormLength = _buffer[offset++] & 0xf;
		return offset + (baseFormLength << 1);
	}
	else
	{
		return offset;
	}
}

int BinaryDictionary::pronunciationOffset(int wordId)
{
	if (hasReadingData(wordId))
	{
		int offset = readingOffset(wordId);
		int readingData = _buffer[offset++] & 0xff;
		int readingLength;

		if ((readingData & 1) == 0)
		{
			readingLength = readingData & 0xfe; // UTF-16: mask off kana bit
		}
		else
		{
			readingLength = static_cast<int>(static_cast<unsigned int>(readingData) >> 1);
		}

		return offset + readingLength;
	}
	else
	{
		return readingOffset(wordId);
	}
}

bool BinaryDictionary::hasBaseFormData(int wordId)
{
	return (toInt16FromBigEndian(_buffer, wordId) & HAS_BASEFORM) != 0;
}

bool BinaryDictionary::hasReadingData(int wordId)
{
	return (toInt16FromBigEndian(_buffer, wordId) & HAS_READING) != 0;
}

bool BinaryDictionary::hasPronunciationData(int wordId)
{
	return (toInt16FromBigEndian(_buffer, wordId) & HAS_PRONUNCIATION) != 0;
}

String BinaryDictionary::readString(int offset, int length, bool kana)
{
	CharArray text = CharArray::newInstance(length);
	if (kana)
	{
		for (int i = 0; i < length; i++)
		{
			text[i] = static_cast<wchar_t>(0x30A0 + (_buffer[offset + i] & 0xff));
		}
	}
	else
	{
		for (int i = 0; i < length; i++)
		{
			text[i] = static_cast<wchar_t>(toInt16FromBigEndian(_buffer, offset + (i << 1)));
		}
	}

	return String(text.get(), text.size());
}
}
}
}
}
