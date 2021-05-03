/**
* @file      Decoder.cpp
* @copyright (c) 2014 by Petr Zemek (s3rvac@gmail.com) and contributors
* @license   BSD, see the @c LICENSE file for more details
* @brief     Implementation of the Decoder class.
*/

#include "Decoder.h"

#include <cassert>
#include <regex>
#include <sstream>

#include "BDictionary.h"
#include "BInteger.h"
#include "BList.h"
#include "BString.h"
#include "Utils.h"

namespace bencoding {

/**
* @brief Constructs a new exception with the given message.
*/
DecodingError::DecodingError(const std::string &what):
	std::runtime_error(what) {}

/**
* @brief Constructs a decoder.
*/
Decoder::Decoder() {}

/**
* @brief Creates a new decoder.
*/
std::unique_ptr<Decoder> Decoder::create() {
	return std::unique_ptr<Decoder>(new Decoder());
}

/**
* @brief Decodes the given bencoded @a data and returns them.
*
* If there are some characters left after the decoded data, this function
* throws DecodingError.
*/
std::unique_ptr<BItem> Decoder::decode(const std::string &data) {
	std::istringstream input(data);
	auto decodedData = decode(input);
	validateInputDoesNotContainUndecodedCharacters(input);
	return decodedData;
}

/**
* @brief Reads the data from the given @a input, decodes them and returns them.
*
* If there are some characters left after the decoding, they are left in @a
* input, i.e. they are not read. This behavior differs for the overload of
* decode() that takes @c std::string as the input.
*/
std::unique_ptr<BItem> Decoder::decode(std::istream &input) {
	switch (input.peek()) {
		case 'd':
			return decodeDictionary(input);
		case 'i':
			return decodeInteger(input);
		case 'l':
			return decodeList(input);
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			return decodeString(input);
		default:
			throw DecodingError(std::string("unexpected character: '") +
				static_cast<char>(input.peek()) + "'");
	}

	assert(false && "should never happen");
	return std::unique_ptr<BItem>();
}

/**
* @brief Reads @a expected_char from @a input and discards it.
*/
void Decoder::readExpectedChar(std::istream &input, char expected_char) const {
	int c = input.get();
	if (c != expected_char) {
		throw DecodingError(std::string("expected '") + expected_char +
			"', got '" + static_cast<char>(c) + "'");
	}
}

/**
* @brief Decodes a dictionary from @a input.
*
* @par Format
* @code
* d<bencoded string><bencoded element>e
* @endcode
*
* @par Example
* @code
* d3:cow3:moo4:spam4:eggse represents the dictionary {"cow": "moo", "spam": "eggs"}
* d4:spaml1:a1:bee represents the dictionary {"spam": ["a", "b"]}
* @endcode
*
* The keys must be bencoded strings. The values may be any bencoded type,
* including integers, strings, lists, and other dictionaries. This function
* supports decoding of dictionaries whose keys are not lexicographically sorted
* (according to the <a
* href="https://wiki.theory.org/BitTorrentSpecification#Bencoding">specification</a>,
* they must be sorted).
*/
std::unique_ptr<BDictionary> Decoder::decodeDictionary(std::istream &input) {
	readExpectedChar(input, 'd');
	auto bDictionary = decodeDictionaryItemsIntoDictionary(input);
	readExpectedChar(input, 'e');
	return bDictionary;
}

/**
* @brief Decodes items from @a input, adds them to a dictionary, and returns
*        that dictionary.
*/
std::unique_ptr<BDictionary> Decoder::decodeDictionaryItemsIntoDictionary(
		std::istream &input) {
	auto bDictionary = BDictionary::create();
	while (input && input.peek() != 'e') {
		std::shared_ptr<BString> key(decodeDictionaryKey(input));
		std::shared_ptr<BItem> value(decodeDictionaryValue(input));
		(*bDictionary)[key] = value;
	}
	return bDictionary;
}

/**
* @brief Decodes a dictionary key from @a input.
*/
std::shared_ptr<BString> Decoder::decodeDictionaryKey(std::istream &input) {
	std::shared_ptr<BItem> key(decode(input));
	// A dictionary key has to be a string.
	std::shared_ptr<BString> keyAsBString(key->as<BString>());
	if (!keyAsBString) {
		throw DecodingError(
			"found a dictionary key that is not a bencoded string"
		);
	}
	return keyAsBString;
}

/**
* @brief Decodes a dictionary value from @a input.
*/
std::unique_ptr<BItem> Decoder::decodeDictionaryValue(std::istream &input) {
	return decode(input);
}

/**
* @brief Decodes an integer from @a input.
*
* @par Format
* @code
* i<integer encoded in base ten ASCII>e
* @endcode
*
* @par Example
* @code
* i3e represents the integer 3
* @endcode
*
* Moreover, only the significant digits should be used, one cannot pad the
* integer with zeroes, such as @c i04e (see the <a
* href="https://wiki.theory.org/BitTorrentSpecification#Bencoding">
* specification</a>).
*/
std::unique_ptr<BInteger> Decoder::decodeInteger(std::istream &input) const {
	return decodeEncodedInteger(readEncodedInteger(input));
}

/**
* @brief Reads an encoded integer from @a input.
*/
std::string Decoder::readEncodedInteger(std::istream &input) const {
	// See the description of decodeInteger() for the format and example.
	std::string encodedInteger;
	bool encodedIntegerReadCorrectly = readUntil(input, encodedInteger, 'e');
	if (!encodedIntegerReadCorrectly) {
		throw DecodingError("error during the decoding of an integer near '" +
			encodedInteger + "'");
	}

	return encodedInteger;
}

/**
* @brief Decodes the given encoded integer.
*/
std::unique_ptr<BInteger> Decoder::decodeEncodedInteger(
		const std::string &encodedInteger) const {
	// See the description of decodeInteger() for the format and example.
	std::regex integerRegex("i([-+]?(0|[1-9][0-9]*))e");
	std::smatch match;
	bool valid = std::regex_match(encodedInteger, match, integerRegex);
	if (!valid) {
		throw DecodingError("encountered an encoded integer of invalid format: '" +
			encodedInteger + "'");
	}

	BInteger::ValueType integerValue;
	strToNum(match[1].str(), integerValue);
	return BInteger::create(integerValue);
}

/**
* @brief Decodes a list from @a input.
*
* @par Format
* @code
* l<bencoded values>e
* @endcode
*
* @par Example
* @code
* l4:spam4:eggse represents a list containing two strings "spam" and "eggs"
* @endcode
*/
std::unique_ptr<BList> Decoder::decodeList(std::istream &input) {
	readExpectedChar(input, 'l');
	auto bList = decodeListItemsIntoList(input);
	readExpectedChar(input, 'e');
	return bList;
}

/**
* @brief Decodes items from @a input, appends them to a list, and returns that
*        list.
*/
std::unique_ptr<BList> Decoder::decodeListItemsIntoList(std::istream &input) {
	auto bList = BList::create();
	while (input && input.peek() != 'e') {
		bList->push_back(decode(input));
	}
	return bList;
}

/**
* @brief Decodes a string from @a input.
*
* @par Format
* @code
* <string length encoded in base ten ASCII>:<string data>
* @endcode
*
* @par Example
* @code
* 4:test represents the string "test"
* @endcode
*/
std::unique_ptr<BString> Decoder::decodeString(std::istream &input) const {
	std::string::size_type stringLength(readStringLength(input));
	readExpectedChar(input, ':');
	std::string str(readStringOfGivenLength(input, stringLength));
	return BString::create(str);
}

/**
* @brief Reads the string length from @a input, validates it, and returns it.
*/
std::string::size_type Decoder::readStringLength(std::istream &input) const {
	std::string stringLengthInASCII;
	bool stringLengthInASCIIReadCorrectly = readUpTo(input, stringLengthInASCII, ':');
	if (!stringLengthInASCIIReadCorrectly) {
		throw DecodingError("error during the decoding of a string near '" +
			stringLengthInASCII + "'");
	}

	std::string::size_type stringLength;
	bool stringLengthIsValid = strToNum(stringLengthInASCII, stringLength);
	if (!stringLengthIsValid) {
		throw DecodingError("invalid string length: '" + stringLengthInASCII + "'");
	}

	return stringLength;
}

/**
* @brief Reads a string of the given @a length from @a input and returns it.
*/
std::string Decoder::readStringOfGivenLength(std::istream &input,
		std::string::size_type length) const {
	std::string str(length, char());
	input.read(&str[0], length);
	std::string::size_type numOfReadChars(input.gcount());
	if (numOfReadChars != length) {
		throw DecodingError("expected a string containing " + std::to_string(length) +
			" characters, but read only " + std::to_string(numOfReadChars) +
			" characters");
	}
	return str;
}

/**
* @brief Throws DecodingError if @a input has not been completely read.
*/
void Decoder::validateInputDoesNotContainUndecodedCharacters(std::istream &input) {
	if (input.peek() != std::char_traits<char>::eof()) {
		throw DecodingError("input contains undecoded characters");
	}
}

/**
* @brief Decodes the given bencoded @a data and returns them.
*
* This function can be handy if you just want to decode bencoded data without
* explicitly creating a decoder and calling @c decode() on it.
*
* See Decoder::decode() for more details.
*/
std::unique_ptr<BItem> decode(const std::string &data) {
	auto decoder = Decoder::create();
	return decoder->decode(data);
}

/**
* @brief Reads all the data from the given @a input, decodes them and returns
*        them.
*
* This function can be handy if you just want to decode bencoded data without
* explicitly creating a decoder and calling @c decode() on it.
*
* See Decoder::decode() for more details.
*/
std::unique_ptr<BItem> decode(std::istream &input) {
	auto decoder = Decoder::create();
	return decoder->decode(input);
}

} // namespace bencoding
