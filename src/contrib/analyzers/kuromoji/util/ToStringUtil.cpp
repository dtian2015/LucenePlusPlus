#include "kuromoji/util/ToStringUtil.h"
#include "MiscUtils.h"

namespace Lucene {
namespace Analysis {
namespace Ja {
namespace Util {

const std::unordered_map<String, String> ToStringUtil::_posTranslations = {
	{L"名詞", L"noun"},
	{L"名詞-一般", L"noun-common"},
	{L"名詞-固有名詞", L"noun-proper"},
	{L"名詞-固有名詞-一般", L"noun-proper-misc"},
	{L"名詞-固有名詞-人名", L"noun-proper-person"},
	{L"名詞-固有名詞-人名-一般", L"noun-proper-person-misc"},
	{L"名詞-固有名詞-人名-姓", L"noun-proper-person-surname"},
	{L"名詞-固有名詞-人名-名", L"noun-proper-person-given_name"},
	{L"名詞-固有名詞-組織", L"noun-proper-organization"},
	{L"名詞-固有名詞-地域", L"noun-proper-place"},
	{L"名詞-固有名詞-地域-一般", L"noun-proper-place-misc"},
	{L"名詞-固有名詞-地域-国", L"noun-proper-place-country"},
	{L"名詞-代名詞", L"noun-pronoun"},
	{L"名詞-代名詞-一般", L"noun-pronoun-misc"},
	{L"名詞-代名詞-縮約", L"noun-pronoun-contraction"},
	{L"名詞-副詞可能", L"noun-adverbial"},
	{L"名詞-サ変接続", L"noun-verbal"},
	{L"名詞-形容動詞語幹", L"noun-adjective-base"},
	{L"名詞-数", L"noun-numeric"},
	{L"名詞-非自立", L"noun-affix"},
	{L"名詞-非自立-一般", L"noun-affix-misc"},
	{L"名詞-非自立-副詞可能", L"noun-affix-adverbial"},
	{L"名詞-非自立-助動詞語幹", L"noun-affix-aux"},
	{L"名詞-非自立-形容動詞語幹", L"noun-affix-adjective-base"},
	{L"名詞-特殊", L"noun-special"},
	{L"名詞-特殊-助動詞語幹", L"noun-special-aux"},
	{L"名詞-接尾", L"noun-suffix"},
	{L"名詞-接尾-一般", L"noun-suffix-misc"},
	{L"名詞-接尾-人名", L"noun-suffix-person"},
	{L"名詞-接尾-地域", L"noun-suffix-place"},
	{L"名詞-接尾-サ変接続", L"noun-suffix-verbal"},
	{L"名詞-接尾-助動詞語幹", L"noun-suffix-aux"},
	{L"名詞-接尾-形容動詞語幹", L"noun-suffix-adjective-base"},
	{L"名詞-接尾-副詞可能", L"noun-suffix-adverbial"},
	{L"名詞-接尾-助数詞", L"noun-suffix-classifier"},
	{L"名詞-接尾-特殊", L"noun-suffix-special"},
	{L"名詞-接続詞的", L"noun-suffix-conjunctive"},
	{L"名詞-動詞非自立的", L"noun-verbal_aux"},
	{L"名詞-引用文字列", L"noun-quotation"},
	{L"名詞-ナイ形容詞語幹", L"noun-nai_adjective"},
	{L"接頭詞", L"prefix"},
	{L"接頭詞-名詞接続", L"prefix-nominal"},
	{L"接頭詞-動詞接続", L"prefix-verbal"},
	{L"接頭詞-形容詞接続", L"prefix-adjectival"},
	{L"接頭詞-数接続", L"prefix-numerical"},
	{L"動詞", L"verb"},
	{L"動詞-自立", L"verb-main"},
	{L"動詞-非自立", L"verb-auxiliary"},
	{L"動詞-接尾", L"verb-suffix"},
	{L"形容詞", L"adjective"},
	{L"形容詞-自立", L"adjective-main"},
	{L"形容詞-非自立", L"adjective-auxiliary"},
	{L"形容詞-接尾", L"adjective-suffix"},
	{L"副詞", L"adverb"},
	{L"副詞-一般", L"adverb-misc"},
	{L"副詞-助詞類接続", L"adverb-particle_conjunction"},
	{L"連体詞", L"adnominal"},
	{L"接続詞", L"conjunction"},
	{L"助詞", L"particle"},
	{L"助詞-格助詞", L"particle-case"},
	{L"助詞-格助詞-一般", L"particle-case-misc"},
	{L"助詞-格助詞-引用", L"particle-case-quote"},
	{L"助詞-格助詞-連語", L"particle-case-compound"},
	{L"助詞-接続助詞", L"particle-conjunctive"},
	{L"助詞-係助詞", L"particle-dependency"},
	{L"助詞-副助詞", L"particle-adverbial"},
	{L"助詞-間投助詞", L"particle-interjective"},
	{L"助詞-並立助詞", L"particle-coordinate"},
	{L"助詞-終助詞", L"particle-final"},
	{L"助詞-副助詞／並立助詞／終助詞", L"particle-adverbial/conjunctive/final"},
	{L"助詞-連体化", L"particle-adnominalizer"},
	{L"助詞-副詞化", L"particle-adnominalizer"},
	{L"助詞-特殊", L"particle-special"},
	{L"助動詞", L"auxiliary-verb"},
	{L"感動詞", L"interjection"},
	{L"記号", L"symbol"},
	{L"記号-一般", L"symbol-misc"},
	{L"記号-句点", L"symbol-period"},
	{L"記号-読点", L"symbol-comma"},
	{L"記号-空白", L"symbol-space"},
	{L"記号-括弧開", L"symbol-open_bracket"},
	{L"記号-括弧閉", L"symbol-close_bracket"},
	{L"記号-アルファベット", L"symbol-alphabetic"},
	{L"その他", L"other"},
	{L"その他-間投", L"other-interjection"},
	{L"フィラー", L"filler"},
	{L"非言語音", L"non-verbal"},
	{L"語断片", L"fragment"},
	{L"未知語", L"unknown"}};

const std::unordered_map<String, String> ToStringUtil::_inflTypeTranslations = {{L"*", L"*"},
																				{L"形容詞・アウオ段", L"adj-group-a-o-u"},
																				{L"形容詞・イ段", L"adj-group-i"},
																				{L"形容詞・イイ", L"adj-group-ii"},
																				{L"不変化型", L"non-inflectional"},
																				{L"特殊・タ", L"special-da"},
																				{L"特殊・ダ", L"special-ta"},
																				{L"文語・ゴトシ", L"classical-gotoshi"},
																				{L"特殊・ジャ", L"special-ja"},
																				{L"特殊・ナイ", L"special-nai"},
																				{L"五段・ラ行特殊", L"5-row-cons-r-special"},
																				{L"特殊・ヌ", L"special-nu"},
																				{L"文語・キ", L"classical-ki"},
																				{L"特殊・タイ", L"special-tai"},
																				{L"文語・ベシ", L"classical-beshi"},
																				{L"特殊・ヤ", L"special-ya"},
																				{L"文語・マジ", L"classical-maji"},
																				{L"下二・タ行", L"2-row-lower-cons-t"},
																				{L"特殊・デス", L"special-desu"},
																				{L"特殊・マス", L"special-masu"},
																				{L"五段・ラ行アル", L"5-row-aru"},
																				{L"文語・ナリ", L"classical-nari"},
																				{L"文語・リ", L"classical-ri"},
																				{L"文語・ケリ", L"classical-keri"},
																				{L"文語・ル", L"classical-ru"},
																				{L"五段・カ行イ音便", L"5-row-cons-k-i-onbin"},
																				{L"五段・サ行", L"5-row-cons-s"},
																				{L"一段", L"1-row"},
																				{L"五段・ワ行促音便", L"5-row-cons-w-cons-onbin"},
																				{L"五段・マ行", L"5-row-cons-m"},
																				{L"五段・タ行", L"5-row-cons-t"},
																				{L"五段・ラ行", L"5-row-cons-r"},
																				{L"サ変・−スル", L"irregular-suffix-suru"},
																				{L"五段・ガ行", L"5-row-cons-g"},
																				{L"サ変・−ズル", L"irregular-suffix-zuru"},
																				{L"五段・バ行", L"5-row-cons-b"},
																				{L"五段・ワ行ウ音便", L"5-row-cons-w-u-onbin"},
																				{L"下二・ダ行", L"2-row-lower-cons-d"},
																				{L"五段・カ行促音便ユク", L"5-row-cons-k-cons-onbin-yuku"},
																				{L"上二・ダ行", L"2-row-upper-cons-d"},
																				{L"五段・カ行促音便", L"5-row-cons-k-cons-onbin"},
																				{L"一段・得ル", L"1-row-eru"},
																				{L"四段・タ行", L"4-row-cons-t"},
																				{L"五段・ナ行", L"5-row-cons-n"},
																				{L"下二・ハ行", L"2-row-lower-cons-h"},
																				{L"四段・ハ行", L"4-row-cons-h"},
																				{L"四段・バ行", L"4-row-cons-b"},
																				{L"サ変・スル", L"irregular-suru"},
																				{L"上二・ハ行", L"2-row-upper-cons-h"},
																				{L"下二・マ行", L"2-row-lower-cons-m"},
																				{L"四段・サ行", L"4-row-cons-s"},
																				{L"下二・ガ行", L"2-row-lower-cons-g"},
																				{L"カ変・来ル", L"kuru-kanji"},
																				{L"一段・クレル", L"1-row-kureru"},
																				{L"下二・得", L"2-row-lower-u"},
																				{L"カ変・クル", L"kuru-kana"},
																				{L"ラ変", L"irregular-cons-r"},
																				{L"下二・カ行", L"2-row-lower-cons-k"}};

const std::unordered_map<String, String> ToStringUtil::_inflFormTranslations = {
	{L"*", L"*"},
	{L"基本形", L"base"},
	{L"文語基本形", L"classical-base"},
	{L"未然ヌ接続", L"imperfective-nu-connection"},
	{L"未然ウ接続", L"imperfective-u-connection"},
	{L"連用タ接続", L"conjunctive-ta-connection"},
	{L"連用テ接続", L"conjunctive-te-connection"},
	{L"連用ゴザイ接続", L"conjunctive-gozai-connection"},
	{L"体言接続", L"uninflected-connection"},
	{L"仮定形", L"subjunctive"},
	{L"命令ｅ", L"imperative-e"},
	{L"仮定縮約１", L"conditional-contracted-1"},
	{L"仮定縮約２", L"conditional-contracted-2"},
	{L"ガル接続", L"garu-connection"},
	{L"未然形", L"imperfective"},
	{L"連用形", L"conjunctive"},
	{L"音便基本形", L"onbin-base"},
	{L"連用デ接続", L"conjunctive-de-connection"},
	{L"未然特殊", L"imperfective-special"},
	{L"命令ｉ", L"imperative-i"},
	{L"連用ニ接続", L"conjunctive-ni-connection"},
	{L"命令ｙｏ", L"imperative-yo"},
	{L"体言接続特殊", L"adnominal-special"},
	{L"命令ｒｏ", L"imperative-ro"},
	{L"体言接続特殊２", L"uninflected-special-connection-2"},
	{L"未然レル接続", L"imperfective-reru-connection"},
	{L"現代基本形", L"modern-base"},
	{L"基本形-促音便", L"base-onbin"} // not sure about this
};

String ToStringUtil::getPOSTranslation(const String& s)
{
	const auto& matchIter = _posTranslations.find(s);
	return matchIter == _posTranslations.end() ? L"" : matchIter->second;
}

String ToStringUtil::getInflectionTypeTranslation(const String& s)
{
	const auto& matchIter = _inflTypeTranslations.find(s);
	return matchIter == _inflTypeTranslations.end() ? L"" : matchIter->second;
}

String ToStringUtil::getInflectedFormTranslation(const String& s)
{
	const auto& matchIter = _inflFormTranslations.find(s);
	return matchIter == _inflFormTranslations.end() ? L"" : matchIter->second;
}

String ToStringUtil::getRomanization(const String& s)
{
	StringStream out;
	try
	{
		CharArray chars = CharArray::newInstance(s.size());
		MiscUtils::arrayCopy(s.data(), 0, chars.get(), 0, s.size());
		getRomanization(out, chars);
	}
	catch (const IOException& bogus)
	{
		boost::throw_exception(RuntimeException(bogus.getError()));
	}

	return out.str();
}

void ToStringUtil::getRomanization(StringStream& builder, const CharArray& s)
{
	const int len = s.size();
	for (int i = 0; i < len; i++)
	{
		// maximum lookahead: 3
		wchar_t ch = s[i];
		wchar_t ch2 = (i < len - 1) ? s[i + 1] : 0;
		wchar_t ch3 = (i < len - 2) ? s[i + 2] : 0;

		switch (ch)
		{
			case L'ッ':
				switch (ch2)
				{
					case L'カ':
					case L'キ':
					case L'ク':
					case L'ケ':
					case L'コ':
						builder << L'k';
						break;
					case L'サ':
					case L'シ':
					case L'ス':
					case L'セ':
					case L'ソ':
						builder << L's';
						break;
					case L'タ':
					case L'チ':
					case L'ツ':
					case L'テ':
					case L'ト':
						builder << L't';
						break;
					case L'パ':
					case L'ピ':
					case L'プ':
					case L'ペ':
					case L'ポ':
						builder << L'p';
						break;
				}
				break;
			case L'ア':
				builder << L'a';
				break;
			case L'イ':
				if (ch2 == L'ィ')
				{
					builder << L"yi";
					i++;
				}
				else if (ch2 == L'ェ')
				{
					builder << L"ye";
					i++;
				}
				else
				{
					builder << L'i';
				}
				break;
			case L'ウ':
				switch (ch2)
				{
					case L'ァ':
						builder << L"wa";
						i++;
						break;
					case L'ィ':
						builder << L"wi";
						i++;
						break;
					case L'ゥ':
						builder << L"wu";
						i++;
						break;
					case L'ェ':
						builder << L"we";
						i++;
						break;
					case L'ォ':
						builder << L"wo";
						i++;
						break;
					case L'ュ':
						builder << L"wyu";
						i++;
						break;
					default:
						builder << L'u';
						break;
				}
				break;
			case L'エ':
				builder << L'e';
				break;
			case L'オ':
				if (ch2 == L'ウ')
				{
					builder << L'ō';
					i++;
				}
				else
				{
					builder << L'o';
				}
				break;
			case L'カ':
				builder << L"ka";
				break;
			case L'キ':
				if (ch2 == L'ョ' && ch3 == L'ウ')
				{
					builder << L"kyō";
					i += 2;
				}
				else if (ch2 == L'ュ' && ch3 == L'ウ')
				{
					builder << L"kyū";
					i += 2;
				}
				else if (ch2 == L'ャ')
				{
					builder << L"kya";
					i++;
				}
				else if (ch2 == L'ョ')
				{
					builder << L"kyo";
					i++;
				}
				else if (ch2 == L'ュ')
				{
					builder << L"kyu";
					i++;
				}
				else if (ch2 == L'ェ')
				{
					builder << L"kye";
					i++;
				}
				else
				{
					builder << L"ki";
				}
				break;
			case L'ク':
				switch (ch2)
				{
					case L'ァ':
						builder << L"kwa";
						i++;
						break;
					case L'ィ':
						builder << L"kwi";
						i++;
						break;
					case L'ェ':
						builder << L"kwe";
						i++;
						break;
					case L'ォ':
						builder << L"kwo";
						i++;
						break;
					case L'ヮ':
						builder << L"kwa";
						i++;
						break;
					default:
						builder << L"ku";
						break;
				}
				break;
			case L'ケ':
				builder << L"ke";
				break;
			case L'コ':
				if (ch2 == L'ウ')
				{
					builder << L"kō";
					i++;
				}
				else
				{
					builder << L"ko";
				}
				break;
			case L'サ':
				builder << L"sa";
				break;
			case L'シ':
				if (ch2 == L'ョ' && ch3 == L'ウ')
				{
					builder << L"shō";
					i += 2;
				}
				else if (ch2 == L'ュ' && ch3 == L'ウ')
				{
					builder << L"shū";
					i += 2;
				}
				else if (ch2 == L'ャ')
				{
					builder << L"sha";
					i++;
				}
				else if (ch2 == L'ョ')
				{
					builder << L"sho";
					i++;
				}
				else if (ch2 == L'ュ')
				{
					builder << L"shu";
					i++;
				}
				else if (ch2 == L'ェ')
				{
					builder << L"she";
					i++;
				}
				else
				{
					builder << L"shi";
				}
				break;
			case L'ス':
				if (ch2 == L'ィ')
				{
					builder << L"si";
					i++;
				}
				else
				{
					builder << L"su";
				}
				break;
			case L'セ':
				builder << L"se";
				break;
			case L'ソ':
				if (ch2 == L'ウ')
				{
					builder << L"sō";
					i++;
				}
				else
				{
					builder << L"so";
				}
				break;
			case L'タ':
				builder << L"ta";
				break;
			case L'チ':
				if (ch2 == L'ョ' && ch3 == L'ウ')
				{
					builder << L"chō";
					i += 2;
				}
				else if (ch2 == L'ュ' && ch3 == L'ウ')
				{
					builder << L"chū";
					i += 2;
				}
				else if (ch2 == L'ャ')
				{
					builder << L"cha";
					i++;
				}
				else if (ch2 == L'ョ')
				{
					builder << L"cho";
					i++;
				}
				else if (ch2 == L'ュ')
				{
					builder << L"chu";
					i++;
				}
				else if (ch2 == L'ェ')
				{
					builder << L"che";
					i++;
				}
				else
				{
					builder << L"chi";
				}
				break;
			case L'ツ':
				if (ch2 == L'ァ')
				{
					builder << L"tsa";
					i++;
				}
				else if (ch2 == L'ィ')
				{
					builder << L"tsi";
					i++;
				}
				else if (ch2 == L'ェ')
				{
					builder << L"tse";
					i++;
				}
				else if (ch2 == L'ォ')
				{
					builder << L"tso";
					i++;
				}
				else if (ch2 == L'ュ')
				{
					builder << L"tsyu";
					i++;
				}
				else
				{
					builder << L"tsu";
				}
				break;
			case L'テ':
				if (ch2 == L'ィ')
				{
					builder << L"ti";
					i++;
				}
				else if (ch2 == L'ゥ')
				{
					builder << L"tu";
					i++;
				}
				else if (ch2 == L'ュ')
				{
					builder << L"tyu";
					i++;
				}
				else
				{
					builder << L"te";
				}
				break;
			case L'ト':
				if (ch2 == L'ウ')
				{
					builder << L"tō";
					i++;
				}
				else
				{
					builder << L"to";
				}
				break;
			case L'ナ':
				builder << L"na";
				break;
			case L'ニ':
				if (ch2 == L'ョ' && ch3 == L'ウ')
				{
					builder << L"nyō";
					i += 2;
				}
				else if (ch2 == L'ュ' && ch3 == L'ウ')
				{
					builder << L"nyū";
					i += 2;
				}
				else if (ch2 == L'ャ')
				{
					builder << L"nya";
					i++;
				}
				else if (ch2 == L'ョ')
				{
					builder << L"nyo";
					i++;
				}
				else if (ch2 == L'ュ')
				{
					builder << L"nyu";
					i++;
				}
				else if (ch2 == L'ェ')
				{
					builder << L"nye";
					i++;
				}
				else
				{
					builder << L"ni";
				}
				break;
			case L'ヌ':
				builder << L"nu";
				break;
			case L'ネ':
				builder << L"ne";
				break;
			case L'ノ':
				if (ch2 == L'ウ')
				{
					builder << L"nō";
					i++;
				}
				else
				{
					builder << L"no";
				}
				break;
			case L'ハ':
				builder << L"ha";
				break;
			case L'ヒ':
				if (ch2 == L'ョ' && ch3 == L'ウ')
				{
					builder << L"hyō";
					i += 2;
				}
				else if (ch2 == L'ュ' && ch3 == L'ウ')
				{
					builder << L"hyū";
					i += 2;
				}
				else if (ch2 == L'ャ')
				{
					builder << L"hya";
					i++;
				}
				else if (ch2 == L'ョ')
				{
					builder << L"hyo";
					i++;
				}
				else if (ch2 == L'ュ')
				{
					builder << L"hyu";
					i++;
				}
				else if (ch2 == L'ェ')
				{
					builder << L"hye";
					i++;
				}
				else
				{
					builder << L"hi";
				}
				break;
			case L'フ':
				if (ch2 == L'ャ')
				{
					builder << L"fya";
					i++;
				}
				else if (ch2 == L'ュ')
				{
					builder << L"fyu";
					i++;
				}
				else if (ch2 == L'ィ' && ch3 == L'ェ')
				{
					builder << L"fye";
					i += 2;
				}
				else if (ch2 == L'ョ')
				{
					builder << L"fyo";
					i++;
				}
				else if (ch2 == L'ァ')
				{
					builder << L"fa";
					i++;
				}
				else if (ch2 == L'ィ')
				{
					builder << L"fi";
					i++;
				}
				else if (ch2 == L'ェ')
				{
					builder << L"fe";
					i++;
				}
				else if (ch2 == L'ォ')
				{
					builder << L"fo";
					i++;
				}
				else
				{
					builder << L"fu";
				}
				break;
			case L'ヘ':
				builder << L"he";
				break;
			case L'ホ':
				if (ch2 == L'ウ')
				{
					builder << L"hō";
					i++;
				}
				else if (ch2 == L'ゥ')
				{
					builder << L"hu";
					i++;
				}
				else
				{
					builder << L"ho";
				}
				break;
			case L'マ':
				builder << L"ma";
				break;
			case L'ミ':
				if (ch2 == L'ョ' && ch3 == L'ウ')
				{
					builder << L"myō";
					i += 2;
				}
				else if (ch2 == L'ュ' && ch3 == L'ウ')
				{
					builder << L"myū";
					i += 2;
				}
				else if (ch2 == L'ャ')
				{
					builder << L"mya";
					i++;
				}
				else if (ch2 == L'ョ')
				{
					builder << L"myo";
					i++;
				}
				else if (ch2 == L'ュ')
				{
					builder << L"myu";
					i++;
				}
				else if (ch2 == L'ェ')
				{
					builder << L"mye";
					i++;
				}
				else
				{
					builder << L"mi";
				}
				break;
			case L'ム':
				builder << L"mu";
				break;
			case L'メ':
				builder << L"mi";
				break;
			case L'モ':
				if (ch2 == L'ウ')
				{
					builder << L"mō";
					i++;
				}
				else
				{
					builder << L"mo";
				}
				break;
			case L'ヤ':
				builder << L"ya";
				break;
			case L'ユ':
				builder << L"yu";
				break;
			case L'ヨ':
				if (ch2 == L'ウ')
				{
					builder << L"yō";
					i++;
				}
				else
				{
					builder << L"yo";
				}
				break;
			case L'ラ':
				builder << L"ra";
				break;
			case L'リ':
				if (ch2 == L'ョ' && ch3 == L'ウ')
				{
					builder << L"ryō";
					i += 2;
				}
				else if (ch2 == L'ュ' && ch3 == L'ウ')
				{
					builder << L"ryū";
					i += 2;
				}
				else if (ch2 == L'ャ')
				{
					builder << L"rya";
					i++;
				}
				else if (ch2 == L'ョ')
				{
					builder << L"ryo";
					i++;
				}
				else if (ch2 == L'ュ')
				{
					builder << L"ryu";
					i++;
				}
				else if (ch2 == L'ェ')
				{
					builder << L"rye";
					i++;
				}
				else
				{
					builder << L"ri";
				}
				break;
			case L'ル':
				builder << L"ru";
				break;
			case L'レ':
				builder << L"re";
				break;
			case L'ロ':
				if (ch2 == L'ウ')
				{
					builder << L"rō";
					i++;
				}
				else
				{
					builder << L"ro";
				}
				break;
			case L'ワ':
				builder << L"wa";
				break;
			case L'ヰ':
				builder << L"i";
				break;
			case L'ヱ':
				builder << L"e";
				break;
			case L'ヲ':
				builder << L"o";
				break;
			case L'ン':
				switch (ch2)
				{
					case L'バ':
					case L'ビ':
					case L'ブ':
					case L'ベ':
					case L'ボ':
					case L'パ':
					case L'ピ':
					case L'プ':
					case L'ペ':
					case L'ポ':
					case L'マ':
					case L'ミ':
					case L'ム':
					case L'メ':
					case L'モ':
						builder << L'm';
						break;
					case L'ヤ':
					case L'ユ':
					case L'ヨ':
					case L'ア':
					case L'イ':
					case L'ウ':
					case L'エ':
					case L'オ':
						builder << L"n'";
						break;
					default:
						builder << L"n";
						break;
				}
				break;
			case L'ガ':
				builder << L"ga";
				break;
			case L'ギ':
				if (ch2 == L'ョ' && ch3 == L'ウ')
				{
					builder << L"gyō";
					i += 2;
				}
				else if (ch2 == L'ュ' && ch3 == L'ウ')
				{
					builder << L"gyū";
					i += 2;
				}
				else if (ch2 == L'ャ')
				{
					builder << L"gya";
					i++;
				}
				else if (ch2 == L'ョ')
				{
					builder << L"gyo";
					i++;
				}
				else if (ch2 == L'ュ')
				{
					builder << L"gyu";
					i++;
				}
				else if (ch2 == L'ェ')
				{
					builder << L"gye";
					i++;
				}
				else
				{
					builder << L"gi";
				}
				break;
			case L'グ':
				switch (ch2)
				{
					case L'ァ':
						builder << L"gwa";
						i++;
						break;
					case L'ィ':
						builder << L"gwi";
						i++;
						break;
					case L'ェ':
						builder << L"gwe";
						i++;
						break;
					case L'ォ':
						builder << L"gwo";
						i++;
						break;
					case L'ヮ':
						builder << L"gwa";
						i++;
						break;
					default:
						builder << L"gu";
						break;
				}
				break;
			case L'ゲ':
				builder << L"ge";
				break;
			case L'ゴ':
				if (ch2 == L'ウ')
				{
					builder << L"gō";
					i++;
				}
				else
				{
					builder << L"go";
				}
				break;
			case L'ザ':
				builder << L"za";
				break;
			case L'ジ':
				if (ch2 == L'ョ' && ch3 == L'ウ')
				{
					builder << L"jō";
					i += 2;
				}
				else if (ch2 == L'ュ' && ch3 == L'ウ')
				{
					builder << L"jū";
					i += 2;
				}
				else if (ch2 == L'ャ')
				{
					builder << L"ja";
					i++;
				}
				else if (ch2 == L'ョ')
				{
					builder << L"jo";
					i++;
				}
				else if (ch2 == L'ュ')
				{
					builder << L"ju";
					i++;
				}
				else if (ch2 == L'ェ')
				{
					builder << L"je";
					i++;
				}
				else
				{
					builder << L"ji";
				}
				break;
			case L'ズ':
				if (ch2 == L'ィ')
				{
					builder << L"zi";
					i++;
				}
				else
				{
					builder << L"zu";
				}
				break;
			case L'ゼ':
				builder << L"ze";
				break;
			case L'ゾ':
				if (ch2 == L'ウ')
				{
					builder << L"zō";
					i++;
				}
				else
				{
					builder << L"zo";
				}
				break;
			case L'ダ':
				builder << L"da";
				break;
			case L'ヂ':
				builder << L"ji";
				break;
			case L'ヅ':
				builder << L"zu";
				break;
			case L'デ':
				if (ch2 == L'ィ')
				{
					builder << L"di";
					i++;
				}
				else if (ch2 == L'ュ')
				{
					builder << L"dyu";
					i++;
				}
				else
				{
					builder << L"de";
				}
				break;
			case L'ド':
				if (ch2 == L'ウ')
				{
					builder << L"dō";
					i++;
				}
				else if (ch2 == L'ゥ')
				{
					builder << L"du";
					i++;
				}
				else
				{
					builder << L"do";
				}
				break;
			case L'バ':
				builder << L"ba";
				break;
			case L'ビ':
				if (ch2 == L'ョ' && ch3 == L'ウ')
				{
					builder << L"byō";
					i += 2;
				}
				else if (ch2 == L'ュ' && ch3 == L'ウ')
				{
					builder << L"byū";
					i += 2;
				}
				else if (ch2 == L'ャ')
				{
					builder << L"bya";
					i++;
				}
				else if (ch2 == L'ョ')
				{
					builder << L"byo";
					i++;
				}
				else if (ch2 == L'ュ')
				{
					builder << L"byu";
					i++;
				}
				else if (ch2 == L'ェ')
				{
					builder << L"bye";
					i++;
				}
				else
				{
					builder << L"bi";
				}
				break;
			case L'ブ':
				builder << L"bu";
				break;
			case L'ベ':
				builder << L"be";
				break;
			case L'ボ':
				if (ch2 == L'ウ')
				{
					builder << L"bō";
					i++;
				}
				else
				{
					builder << L"bo";
				}
				break;
			case L'パ':
				builder << L"pa";
				break;
			case L'ピ':
				if (ch2 == L'ョ' && ch3 == L'ウ')
				{
					builder << L"pyō";
					i += 2;
				}
				else if (ch2 == L'ュ' && ch3 == L'ウ')
				{
					builder << L"pyū";
					i += 2;
				}
				else if (ch2 == L'ャ')
				{
					builder << L"pya";
					i++;
				}
				else if (ch2 == L'ョ')
				{
					builder << L"pyo";
					i++;
				}
				else if (ch2 == L'ュ')
				{
					builder << L"pyu";
					i++;
				}
				else if (ch2 == L'ェ')
				{
					builder << L"pye";
					i++;
				}
				else
				{
					builder << L"pi";
				}
				break;
			case L'プ':
				builder << L"pu";
				break;
			case L'ペ':
				builder << L"pe";
				break;
			case L'ポ':
				if (ch2 == L'ウ')
				{
					builder << L"pō";
					i++;
				}
				else
				{
					builder << L"po";
				}
				break;
			case L'ヴ':
				if (ch2 == L'ィ' && ch3 == L'ェ')
				{
					builder << L"vye";
					i += 2;
				}
				else
				{
					builder << L'v';
				}
				break;
			case L'ァ':
				builder << L'a';
				break;
			case L'ィ':
				builder << L'i';
				break;
			case L'ゥ':
				builder << L'u';
				break;
			case L'ェ':
				builder << L'e';
				break;
			case L'ォ':
				builder << L'o';
				break;
			case L'ヮ':
				builder << L"wa";
				break;
			case L'ャ':
				builder << L"ya";
				break;
			case L'ュ':
				builder << L"yu";
				break;
			case L'ョ':
				builder << L"yo";
				break;
			case L'ー':
				break;
			default:
				builder << ch;
				break;
		}
	}
}
}
}
}
}
