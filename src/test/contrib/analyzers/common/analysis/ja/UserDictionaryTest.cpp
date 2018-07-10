#include "TestInc.h"

#include "BaseTokenStreamFixture.h"
#include "kuromoji/dict/UserDictionary.h"

using namespace Lucene;
using namespace Lucene::Analysis::Ja::Dict;

class UserDictionaryTest : public BaseTokenStreamFixture
{
public:
	CharArray toCharArray(const String& text) const
	{
		CharArray s(CharArray::newInstance(text.length()));
		std::copy(text.begin(), text.end(), s.get());

		return s;
	}
};

TEST_F(UserDictionaryTest, testLookup)
{
	UserDictionaryPtr dictionary = readDict();
	String s = L"関西国際空港に行った";

	std::vector<std::vector<int>> dictionaryEntryResult = dictionary->lookup(toCharArray(s), 0, s.size());
	// Length should be three 関西, 国際, 空港
	ASSERT_EQ(3, dictionaryEntryResult.size());

	// Test positions
	EXPECT_EQ(0, dictionaryEntryResult[0][1]); // index of 関西
	EXPECT_EQ(2, dictionaryEntryResult[1][1]); // index of 国際
	EXPECT_EQ(4, dictionaryEntryResult[2][1]); // index of 空港

	// Test lengths
	EXPECT_EQ(2, dictionaryEntryResult[0][2]); // length of 関西
	EXPECT_EQ(2, dictionaryEntryResult[1][2]); // length of 国際
	EXPECT_EQ(2, dictionaryEntryResult[2][2]); // length of 空港

	s = L"関西国際空港と関西国際空港に行った";
	std::vector<std::vector<int>> dictionaryEntryResult2 = dictionary->lookup(toCharArray(s), 0, s.size());
	// Length should be six
	EXPECT_EQ(6, dictionaryEntryResult2.size());
}

TEST_F(UserDictionaryTest, testReadings)
{
	UserDictionaryPtr dictionary = readDict();
	std::vector<std::vector<int>> result = dictionary->lookup(toCharArray(String(L"日本経済新聞")), 0, 6);
	ASSERT_EQ(3, result.size());

	int wordIdNihon = result[0][0]; // wordId of 日本 in 日本経済新聞
	EXPECT_EQ(L"ニホン", dictionary->getReading(wordIdNihon, toCharArray(String(L"日本")), 0, 2));

	result = dictionary->lookup(toCharArray(String(L"朝青龍")), 0, 3);
	ASSERT_EQ(1, result.size());

	int wordIdAsashoryu = result[0][0]; // wordId for 朝青龍
	EXPECT_EQ(L"アサショウリュウ", dictionary->getReading(wordIdAsashoryu, toCharArray(String(L"朝青龍")), 0, 3));
}

TEST_F(UserDictionaryTest, testPartOfSpeech)
{
	UserDictionaryPtr dictionary = readDict();
	std::vector<std::vector<int>> result = dictionary->lookup(toCharArray(String(L"日本経済新聞")), 0, 6);
	ASSERT_EQ(3, result.size());

	int wordIdKeizai = result[1][0]; // wordId of 経済 in 日本経済新聞
	EXPECT_EQ(L"カスタム名詞", dictionary->getPartOfSpeech(wordIdKeizai));
}

TEST_F(UserDictionaryTest, testRead)
{
	UserDictionaryPtr dictionary = readDict();
	EXPECT_NE(nullptr, dictionary);
}
