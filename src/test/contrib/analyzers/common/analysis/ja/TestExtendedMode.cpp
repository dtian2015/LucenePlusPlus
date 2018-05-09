#include "TestInc.h"

#include "BaseTokenStreamFixture.h"
#include "BufferedReader.h"
#include "CharTermAttribute.h"
#include "FileReader.h"
#include "FileUtils.h"
#include "InputStreamReader.h"
#include "Random.h"
#include "ReusableAnalyzerBase.h"
#include "StringReader.h"
#include "TestUtils.h"
#include "UnicodeUtils.h"

#include "kuromoji/JapaneseTokenizer.h"

#include <boost/endian/conversion.hpp>

// Serialization
// include input and output archivers
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

// include this header to serialize vectors
#include <boost/serialization/vector.hpp>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include <fstream>
#include <locale>

using namespace Lucene;
using namespace Lucene::Analysis::Ja;

class TestExtendedMode : public BaseTokenStreamFixture
{
public:
	DECLARE_SHARED_PTR(ReusableAnalyzerExtended);
	class ReusableAnalyzerExtended : public ReusableAnalyzerBase
	{
	public:
		LUCENE_CLASS(ReusableAnalyzerExtended);

	protected:
		ReusableAnalyzerBase::TokenStreamComponentsPtr createComponents(const String& fieldName, ReaderPtr reader)
		{
			TokenizerPtr tokenizer = newLucene<JapaneseTokenizer>(reader, nullptr, true, JapaneseTokenizer::Mode::EXTENDED);
			return newLucene<TokenStreamComponents>(tokenizer, tokenizer);
		}
	};

	AnalyzerPtr extendedAnalyzer = newLucene<ReusableAnalyzerExtended>();
};

TEST_F(TestExtendedMode, testSurrogates)
{
	checkAnalyzesTo(extendedAnalyzer, L"𩬅艱鍟䇹愯瀛", newCollection<String>(L"𩬅", L"艱", L"鍟", L"䇹", L"愯", L"瀛"));
}

TEST_F(TestExtendedMode, testSurrogates2)
{
	int numIterations = atLeast(100);
	for (int i = 0; i < numIterations; i++)
	{
		String s = randomUnicodeString(newLucene<Random>(), 100);
		TokenStreamPtr ts = extendedAnalyzer->tokenStream(L"foo", newLucene<StringReader>(s));
		CharTermAttributePtr termAtt = ts->addAttribute<CharTermAttribute>();
		ts->reset();

		while (ts->incrementToken())
		{
			EXPECT_TRUE(UnicodeUtil::validUTF16String(termAtt->buffer()));
		}
	}
}

// Daniel TODO: remove this test
TEST_F(TestExtendedMode, testGB2312)
{
	String test = L"中国";

	for (const auto& ch : test)
	{
		short id = UnicodeUtil::getGB2312Id(ch);

		std::cout << "GB Id = " << id << std::endl;

		String s = UnicodeUtil::getCCByGB2312Id(id);

		std::wcout << L"converted back = " << s << std::endl;
	}
}

TEST_F(TestExtendedMode, testReadBigRamDict)
{
	const String dictFile = FileUtils::joinPath(getTestDir(), L"bigramdict.mem");
	if (!FileUtils::fileExists(dictFile))
	{
		boost::throw_exception(RuntimeException(L"Cannot find bigramdict.mem in test classpath!"));
	}

	static const int PRIME_BIGRAM_LENGTH = 402137;

	std::vector<int64_t> bigramHashTable;
	bigramHashTable.reserve(PRIME_BIGRAM_LENGTH);
	std::vector<int> frequencyTable;
	frequencyTable.reserve(PRIME_BIGRAM_LENGTH);

	std::unique_ptr<DataInput> is(new SimpleFSIndexInput(dictFile, BufferedIndexInput::BUFFER_SIZE, FSDirectory::DEFAULT_READ_CHUNK_SIZE));

	for (int i = 0; i < PRIME_BIGRAM_LENGTH; i++)
	{
		int64_t hash = is->readLong();
		int64_t converted = boost::endian::endian_reverse(hash);

		std::cout << "hash = " << hash << ", endian converted = " << converted << std::endl;

		bigramHashTable.push_back(hash);
	}
}

TEST_F(TestExtendedMode, testSerialization)
{
	std::vector<double> v = {1, 2, 3.4, 5.6};
	std::vector<int64_t> lnumber = {1000, -22232323, 3467676767, -898989856};

	// serialize vector
	{
		std::ofstream ofs("/Users/dtian/Temp/copy_binary.ser");
		//		boost::archive::text_oarchive oa(ofs);
		boost::archive::binary_oarchive oa(ofs);
		oa& v;
		oa& lnumber;
	}

	std::vector<double> v2;
	std::vector<int64_t> lnumber2;

	// load serialized vector into vector 2
	{
		std::ifstream ifs("/Users/dtian/Temp/copy_binary.ser");
		//		boost::archive::text_iarchive ia(ifs);
		boost::archive::binary_iarchive ia(ifs);
		ia& v2;
		ia& lnumber2;
	}

	// printout v2 values
	for (auto& d : v2)
	{
		std::cout << d << std::endl;
	}

	std::cout << "--------Restored Long-------" << std::endl;
	for (auto& d : lnumber2)
	{
		std::cout << d << std::endl;
	}
}

TEST_F(TestExtendedMode, testUTF8string)
{
	RandomPtr random = newLucene<Random>();

	CharArray result = CharArray::newInstance(5);

	std::cout << std::hex << std::setfill('0');

	for (int i = 0; i < result.size(); i++)
	{
		result[i] = nextInt(random, 0x00010000, 0x0010FFFF);

		std::cout << std::setw(4) << static_cast<int>(result[i]) << ' ';
	}

	String buffer(result.get(), result.size());

	std::cout << "UTF8 generated " << std::endl;
}

TEST_F(TestExtendedMode, testUnicodeRange)
{
	std::cout << "Range  0x00010000 - 0x0010FFFF {" << 0x00010000 << ", " << 0x0010FFFF << std::endl;

	//  0x80, 0x800
	std::cout << "Range  0x80 - 0x800 {" << 0x80 << ", " << 0x800 << std::endl;

	//  0x800, 0xd7ff
	std::cout << "Range  0x800 - 0xd7ff {" << 0x800 << ", " << 0xd7ff << std::endl;

	//  0xe000, 0xfffe
	std::cout << "Range  0xe000 - 0xfffe {" << 0xe000 << ", " << 0xfffe << std::endl;

	CharArray result = CharArray::newInstance(5);
	RandomPtr random = newLucene<Random>();

	for (int i = 0; i < result.size(); i++)
	{
		int n = nextInt(random, 0x00010000, 0x0010FFFF);

		result[i] = static_cast<wchar_t>(n);

		std::cout << "random number = " << n << ", back from wchar_t " << static_cast<int>(result[i]) << std::endl;
	}

	for (int i = 0; i < 50; i++)
	{
		std::cout << "no range random number = " << random->nextInt(0x80) << std::endl;
	}
}

static const int PRIME_INDEX_LENGTH = 12071;
static const std::string DICT_FOLDER = "/Users/DanielTian/qsr/Temp/ChineseDict";
// static const std::string DICT_FOLDER = "/Users/dtian/Temp/ChineseDict"; // Home

static const int GB2312_CHAR_NUM = 87 * 94;

std::vector<short> GetWordIndexTable()
{
	// word index table
	std::vector<short> wordIndexTable;
	wordIndexTable.reserve(PRIME_INDEX_LENGTH);

	std::ifstream infile{DICT_FOLDER + "/wordIndexTable.txt"};
	std::string rawContent{std::istreambuf_iterator<char>(infile), std::istreambuf_iterator<char>()};
	const std::string content = rawContent.substr(1, rawContent.length() - 2);
	std::vector<std::string> words;

	boost::split(words, content, boost::is_any_of(","));

	std::transform(words.begin(), words.end(), std::back_inserter(wordIndexTable), [](const std::string& word) {
		const std::string trimmedWord = boost::trim_copy(word);
		return boost::lexical_cast<short>(trimmedWord);
	});

	std::cout << "*****************Start Word Index Table*******************************" << std::endl;
	for (const auto& w : wordIndexTable)
	{
		std::cout << w << ",";
	}

	std::cout << std::endl << "*****************End Word Index Table*******************************" << std::endl;

	EXPECT_EQ(PRIME_INDEX_LENGTH, wordIndexTable.size());
	return wordIndexTable;
}

CharArray GetCharIndexTable()
{
	const String dict_folder(DICT_FOLDER.begin(), DICT_FOLDER.end());
	BufferedReaderPtr reader = newLucene<BufferedReader>(
		newLucene<InputStreamReader>(newLucene<FileReader>(FileUtils::joinPath(dict_folder, L"/charIndexTable.txt"))));

	StringStream buffer;
	String line;
	while (reader->readLine(line))
	{
		buffer << line;
	}

	String rawContent = buffer.str();
	String content = rawContent.substr(1, rawContent.length() - 2);
	Collection<String> chars = StringUtils::split(content, L",");

	CharArray result = CharArray::newInstance(PRIME_INDEX_LENGTH);

	std::wcout.sync_with_stdio(false);
	std::wcout.imbue(std::locale("en_AU.UTF-8"));

	std::cout << "*****************Char Index Table*******************************" << std::endl;
	StringStream sb;

	for (int i = 0; i < chars.size(); ++i)
	{
		const String ch = boost::trim_copy(chars[i]);

		result[i] = ch[0];

		if (result[i] == 0)
		{
			sb << L"";
		}
		else
		{
			sb << result[i];
		}

		sb << L",";
	}

	std::wcout << sb.str();
	std::cout << std::endl << "*****************End Char Index Table*******************************" << std::endl;

	return result;
}

std::vector<std::vector<int>> GetWordFrequencyTable()
{
	std::vector<std::vector<int>> result;
	result.resize(GB2312_CHAR_NUM);

	std::ifstream infile{DICT_FOLDER + "/wordItem_frequencyTable.txt"};
	std::string line;
	std::vector<std::string> rawContent;

	while (std::getline(infile, line))
	{
		rawContent.push_back(line);
	}

	std::cout << "*****************Word Frequency  Table*******************************" << std::endl;
	std::stringstream buffer;

	for (int i = 0; i < rawContent.size(); ++i)
	{
		const std::string text = boost::trim_copy(rawContent[i]);

		buffer << text << "\n";

		if (text == "null")
		{
			continue;
		}

		const std::string rawFreqText = text.substr(1, text.size() - 2);
		std::vector<std::string> freqs;
		boost::split(freqs, rawFreqText, boost::is_any_of(","));

		for (auto& freq : freqs)
		{
			boost::trim(freq);
			result[i].push_back(boost::lexical_cast<int>(freq));
		}
	}

	std::cout << buffer.str();
	std::cout << std::endl << "*****************End Word Frequency Table*******************************" << std::endl;

	return result;
}

void ProcessCharArrayTable(const std::vector<String>& rawContent, std::vector<std::vector<CharArray>>& result, int index)
{
	result[index].resize(rawContent.size());

	for (int i = 0; i < rawContent.size(); ++i)
	{
		String copy = rawContent[i];
		if (copy == L"null" || copy == L"[]")
		{
			continue;
		}

		boost::replace_all(copy, "[", "");
		boost::replace_all(copy, "]", "");

		Collection<String> charText = StringUtils::split(copy, L",");
		CharArray chars = CharArray::newInstance(charText.size());

		for (int i = 0; i < charText.size(); ++i)
		{
			String text = charText[i];
			boost::trim(text);
			EXPECT_EQ(1, text.size());

			if (text.size() != 1)
			{
				std::wcout << L"Text size = " << text.size() << L", content = [" << text << L"]" << std::endl;
			}

			chars[i] = text[0];
		}

		result[index][i] = chars;
	}
}

std::vector<std::vector<CharArray>> GetWordCharArrayTable()
{
	std::vector<std::vector<CharArray>> result;
	result.resize(GB2312_CHAR_NUM);

	const String dict_folder(DICT_FOLDER.begin(), DICT_FOLDER.end());
	BufferedReaderPtr reader = newLucene<BufferedReader>(
		newLucene<InputStreamReader>(newLucene<FileReader>(FileUtils::joinPath(dict_folder, L"/wordItem_charArrayTable.txt"))));

	std::vector<String> secondLevel;

	const static String SEPARATOR_LINE = L"**********************************************";

	std::wcout.sync_with_stdio(false);
	std::wcout.imbue(std::locale("en_AU.UTF-8"));

	std::cout << "*****************Word Char Array  Table*******************************" << std::endl;
	StringStream buffer;

	int index = -1;
	String line;
	while (reader->readLine(line))
	{
		buffer << line << L"\n";

		if (line == SEPARATOR_LINE)
		{
			++index;
			if (secondLevel.size() > 1 || secondLevel[0] != L"null")
			{
				ProcessCharArrayTable(secondLevel, result, index);
			}

			secondLevel.clear();
			continue;
		}

		secondLevel.push_back(line);
	}

	std::wcout << buffer.str();
	std::cout << std::endl << "*****************End Word Char Array Table*******************************" << std::endl;

	return result;
}

TEST_F(TestExtendedMode, testWordDictionary)
{
	// word index table
	const std::vector<short> wordIndexTable = GetWordIndexTable();

	// char index table
	const CharArray charIndexTable = GetCharIndexTable();

	// word frequency table
	const std::vector<std::vector<int>> wordFrequencyTable = GetWordFrequencyTable();

	// word char array table
	const std::vector<std::vector<CharArray>> wordCharArrayTable = GetWordCharArrayTable();
	std::vector<std::vector<String>> converted;
	converted.resize(wordCharArrayTable.size());

	for (int i = 0; i < wordCharArrayTable.size(); ++i)
	{
		converted[i].resize(wordCharArrayTable[i].size());
		for (int j = 0; j < wordCharArrayTable[i].size(); ++j)
		{
			CharArray ca = wordCharArrayTable[i][j];

			if (ca)
			{
				converted[i][j] = String(ca.get(), ca.size());
			}
		}
	}

	//	const std::string dictFile = "/Users/dtian/Temp/ChineseDict/coredict.dat"; // Home

	const std::string dictFile = "/Users/DanielTian/qsr/Temp/ChineseDict/coredict_test.dat";
	// serialize
	{
		std::ofstream ofs(dictFile);
		boost::archive::binary_oarchive oa(ofs);

		oa& wordIndexTable;

		String chars(charIndexTable.get(), charIndexTable.size());
		oa& chars;

		oa& wordFrequencyTable;

		// char array table
		oa& converted;
	}

	std::vector<short> wordIndexTable_fromFile;
	CharArray charIndexTable_fromFile;
	std::vector<std::vector<int>> wordFrequencyTable_fromFile;
	std::vector<std::vector<String>> converted_fromFile;

	// deserialize
	{
		std::ifstream ifs(dictFile);
		boost::archive::binary_iarchive ia(ifs);

		ia& wordIndexTable_fromFile;

		String chars_formFile;
		ia& chars_formFile;

		charIndexTable_fromFile = CharArray::newInstance(chars_formFile.size());
		std::copy(chars_formFile.begin(), chars_formFile.end(), charIndexTable_fromFile.get());

		ia& wordFrequencyTable_fromFile;

		// char array table
		ia& converted_fromFile;
	}

	EXPECT_EQ(wordIndexTable, wordIndexTable_fromFile);
	EXPECT_EQ(charIndexTable.size(), charIndexTable_fromFile.size());

	for (int i = 0; i < charIndexTable.size(); ++i)
	{
		EXPECT_EQ(charIndexTable[i], charIndexTable_fromFile[i]);
	}

	EXPECT_EQ(wordFrequencyTable, wordFrequencyTable_fromFile);
	EXPECT_EQ(converted, converted_fromFile);
}

// Bigram dictionary
static const int PRIME_BIGRAM_LENGTH = 402137;

std::vector<int64_t> GetBigramHashTable(bool isDebug = false)
{
	std::vector<int64_t> result;
	result.reserve(PRIME_BIGRAM_LENGTH);

	std::ifstream infile{DICT_FOLDER + "/bigram_HashTable.txt"};
	std::string rawContent{std::istreambuf_iterator<char>(infile), std::istreambuf_iterator<char>()};
	const std::string content = rawContent.substr(1, rawContent.length() - 2);
	std::vector<std::string> hashes;

	boost::split(hashes, content, boost::is_any_of(","));

	std::transform(hashes.begin(), hashes.end(), std::back_inserter(result), [](const std::string& hash) {
		const std::string trimmedHash = boost::trim_copy(hash);
		return boost::lexical_cast<int64_t>(trimmedHash);
	});

	if (isDebug)
	{
		std::cout << "*****************Start Bigram Hash Table*******************************" << std::endl;
		for (const auto& h : result)
		{
			std::cout << h << ",";
		}

		std::cout << std::endl << "*****************End Bigram Hash Table*******************************" << std::endl;
	}

	EXPECT_EQ(PRIME_BIGRAM_LENGTH, result.size());
	return result;
}

std::vector<int> GetBigramFrequencyTable(bool isDebug = false)
{
	std::vector<int> result;
	result.reserve(PRIME_BIGRAM_LENGTH);

	std::ifstream infile{DICT_FOLDER + "/bigram_FrequencyTable.txt"};
	std::string rawContent{std::istreambuf_iterator<char>(infile), std::istreambuf_iterator<char>()};
	const std::string content = rawContent.substr(1, rawContent.length() - 2);
	std::vector<std::string> frequencies;

	boost::split(frequencies, content, boost::is_any_of(","));

	std::transform(frequencies.begin(), frequencies.end(), std::back_inserter(result), [](const std::string& frequency) {
		const std::string trimmedFrequency = boost::trim_copy(frequency);
		return boost::lexical_cast<int>(trimmedFrequency);
	});

	if (isDebug)
	{
		std::cout << "*****************Start Bigram Frequency Table*******************************" << std::endl;
		for (const auto& f : result)
		{
			std::cout << f << ",";
		}

		std::cout << std::endl << "*****************End Bigram Frequency Table*******************************" << std::endl;
	}

	EXPECT_EQ(PRIME_BIGRAM_LENGTH, result.size());
	return result;
}

TEST_F(TestExtendedMode, testBigramDictionary)
{
	// bigram hashtable
	const std::vector<int64_t> bigramHashTable = GetBigramHashTable();

	// bigram frequency table
	const std::vector<int> frequencyTable = GetBigramFrequencyTable(true);

	const std::string dictFile = "/Users/DanielTian/qsr/Temp/ChineseDict/bigramdict.dat";
	// serialize
	{
		std::ofstream ofs(dictFile);
		boost::archive::binary_oarchive oa(ofs);

		oa& bigramHashTable;

		oa& frequencyTable;
	}

	std::vector<int64_t> bigramHashTable_fromFile;
	std::vector<int> frequencyTable_fromFile;

	// deserialize
	{
		std::ifstream ifs(dictFile);
		boost::archive::binary_iarchive ia(ifs);

		ia& bigramHashTable_fromFile;

		ia& frequencyTable_fromFile;
	}

	EXPECT_EQ(bigramHashTable, bigramHashTable_fromFile);

	EXPECT_EQ(frequencyTable, frequencyTable_fromFile);
}
