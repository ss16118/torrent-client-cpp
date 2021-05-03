/**
* @file      Encoder.cpp
* @copyright (c) 2014 by Petr Zemek (s3rvac@gmail.com) and contributors
* @license   BSD, see the @c LICENSE file for more details
* @brief     Implementation of the Encoder class.
*/

#include "Encoder.h"

#include "BDictionary.h"
#include "BInteger.h"
#include "BList.h"
#include "BString.h"
#include "Utils.h"

namespace bencoding {

/**
* @brief Constructs an encoder.
*/
Encoder::Encoder() {}

/**
* @brief Creates a new encoder.
*/
std::unique_ptr<Encoder> Encoder::create() {
	return std::unique_ptr<Encoder>(new Encoder());
}

/**
* @brief Encodes the given @a data and returns them.
*/
std::string Encoder::encode(std::shared_ptr<BItem> data) {
	data->accept(this);
	return encodedData;
}

void Encoder::visit(BDictionary *bDictionary) {
	// See the description of Decoder::decodeDictionary() for the format and
	// example.
	encodedData += "d";
	for (auto item : *bDictionary) {
		item.first->accept(this);
		item.second->accept(this);
	}
	encodedData += "e";
}

void Encoder::visit(BInteger *bInteger) {
	// See the description of Decoder::decodeInteger() for the format and
	// example.
	std::string encodedInteger("i" + std::to_string(bInteger->value()) + "e");
	encodedData += encodedInteger;
}

void Encoder::visit(BList *bList) {
	// See the description of Decoder::decodeList() for the format and example.
	encodedData += "l";
	for (auto bItem : *bList) {
		bItem->accept(this);
	}
	encodedData += "e";
}

void Encoder::visit(BString *bString) {
	// See the description of Decoder::decodeString() for the format and
	// example.
	std::string encodedString(
		std::to_string(bString->length()) + ":" + bString->value()
	);
	encodedData += encodedString;
}

/**
* @brief Encodes the given @a data and returns them.
*
* This function can be handy if you just want to encode data without explicitly
* creating an encoder and calling @c encode() on it.
*
* See Encoder::encode() for more details.
*/
std::string encode(std::shared_ptr<BItem> data) {
	auto encoder = Encoder::create();
	return encoder->encode(data);
}

} // namespace bencoding
