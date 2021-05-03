/**
* @file      BString.cpp
* @copyright (c) 2014 by Petr Zemek (s3rvac@gmail.com) and contributors
* @license   BSD, see the @c LICENSE file for more details
* @brief     Implementation of the BString class.
*/

#include "BString.h"

#include "BItemVisitor.h"

namespace bencoding {

/**
* @brief Constructs the string with the given @a value.
*/
BString::BString(ValueType value): _value(value) {}

/**
* @brief Creates and returns a new string.
*/
std::unique_ptr<BString> BString::create(ValueType value) {
	return std::unique_ptr<BString>(new BString(value));
}

/**
* @brief Returns the string's value.
*/
auto BString::value() const -> ValueType {
	return _value;
}

/**
* @brief Sets a new value.
*/
void BString::setValue(ValueType value) {
	_value = value;
}

/**
* @brief Returns the number of characters in the string.
*/
auto BString::length() const -> ValueType::size_type {
	return _value.length();
}

void BString::accept(BItemVisitor *visitor) {
	visitor->visit(this);
}

} // namespace bencoding
