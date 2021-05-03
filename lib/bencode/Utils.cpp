/**
* @file      Utils.cpp
* @copyright (c) 2014 by Petr Zemek (s3rvac@gmail.com) and contributors
* @license   BSD, see the @c LICENSE file for more details
* @brief     Implementation of the utilities.
*/

#include "Utils.h"

namespace bencoding {

/**
* @brief Reads data from the given @a stream up to @a sentinel, which is left
*        in @a stream.
*
* @param[in] stream Stream from which the data are read.
* @param[out] readData String into which the read data are stored.
* @param[in] sentinel The data are up to this character.
*
* @return @c true if all the data were read correctly up to @a sentinel, @c
*         false otherwise.
*
* @a sentinel is not read and is kept in the stream. If @a sentinel is not
* found during the reading, this function returns @c false. Read data are
* appended into @a readData.
*/
bool readUpTo(std::istream &stream, std::string &readData, char sentinel) {
	// Do not use std::getline() because it eats the sentinel from the stream.
	while (stream.peek() != std::char_traits<char>::eof() &&
			stream.peek() != sentinel) {
		readData += stream.get();
	}
	return stream && stream.peek() == sentinel;
}

/**
* @brief Reads data from the given @a stream until @a last, which is also read.
*
* @param[in] stream Stream from which the data are read.
* @param[out] readData String into which the read data are stored.
* @param[in] last The last character to be read.
*
* @return @c true if all the data were read correctly up and including @a last,
*         @c false otherwise.
*
* In contrast to readUpTo(), @a last is also read into @a readData. If @a last
* is not found during the reading, this function returns @c false. Read data
* are appended into @a readData.
*/
bool readUntil(std::istream &stream, std::string &readData, char last) {
	char c;
	while (stream.get(c)) {
		readData += c;
		if (c == last) {
			return true;
		}
	}
	return false;
}

/**
* @brief Replaces all occurrences of @a what with @a withWhat in @a str and
*        returns the resulting string.
*/
std::string replace(const std::string &str, char what,
		const std::string &withWhat) {
	std::string result;
	for (auto c : str) {
		if (c == what) {
			result += withWhat;
		} else {
			result += c;
		}
	}
	return result;
}

} // namespace bencoding
