/**
* @file      Decoder.h
* @copyright (c) 2014 by Petr Zemek (s3rvac@gmail.com) and contributors
* @license   BSD, see the @c LICENSE file for more details
* @brief     Decoder of bencoded data.
*/

#ifndef BENCODING_DECODER_H
#define BENCODING_DECODER_H

#include <exception>
#include <memory>
#include <string>

#include "BItem.h"

namespace bencoding {

class BDictionary;
class BInteger;
class BList;
class BString;

/**
* @brief Exception thrown when there is an error during the decoding.
*/
class DecodingError: public std::runtime_error {
public:
	explicit DecodingError(const std::string &what);
};

/**
* @brief Decoder of bencoded data.
*
* The format is based on the <a
* href="https://wiki.theory.org/BitTorrentSpecification#Bencoding">BitTorrent
* specification</a>.
*
* Use create() to create instances.
*/
class Decoder {
public:
	static std::unique_ptr<Decoder> create();

	std::unique_ptr<BItem> decode(const std::string &data);
	std::unique_ptr<BItem> decode(std::istream &input);

private:
	Decoder();

	void readExpectedChar(std::istream &input, char expected_char) const;

	/// @name Integer Decoding
	/// @{
	std::unique_ptr<BDictionary> decodeDictionary(std::istream &input);
	std::unique_ptr<BDictionary> decodeDictionaryItemsIntoDictionary(
		std::istream &input);
	std::shared_ptr<BString> decodeDictionaryKey(std::istream &input);
	std::unique_ptr<BItem> decodeDictionaryValue(std::istream &input);
	/// @}

	/// @name Integer Decoding
	/// @{
	std::unique_ptr<BInteger> decodeInteger(std::istream &input) const;
	std::string readEncodedInteger(std::istream &input) const;
	std::unique_ptr<BInteger> decodeEncodedInteger(
		const std::string &encodedInteger) const;
	/// @}

	/// @name List Decoding
	/// @{
	std::unique_ptr<BList> decodeList(std::istream &input);
	std::unique_ptr<BList> decodeListItemsIntoList(std::istream &input);
	/// @}

	/// @name String Decoding
	/// @{
	std::unique_ptr<BString> decodeString(std::istream &input) const;
	std::string::size_type readStringLength(std::istream &input) const;
	std::string readStringOfGivenLength(std::istream &input,
		std::string::size_type length) const;
	/// @}

	void validateInputDoesNotContainUndecodedCharacters(std::istream &input);
};

/// @name Decoding Without Explicit Decoder Creation
/// @{
std::unique_ptr<BItem> decode(const std::string &data);
std::unique_ptr<BItem> decode(std::istream &input);
/// @}

} // namespace bencoding

#endif
