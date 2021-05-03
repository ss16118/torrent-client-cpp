/**
* @file      BInteger.h
* @copyright (c) 2014 by Petr Zemek (s3rvac@gmail.com) and contributors
* @license   BSD, see the @c LICENSE file for more details
* @brief     Representation of an integer.
*/

#ifndef BENCODING_BINTEGER_H
#define BENCODING_BINTEGER_H

#include <cstdint>
#include <memory>

#include "BItem.h"

namespace bencoding {

/**
* @brief Representation of an integer.
*
* Use create() to create instances of the class.
*/
class BInteger: public BItem {
public:
	/// Type of the underlying integral value.
	using ValueType = int64_t;

public:
	static std::unique_ptr<BInteger> create(ValueType value);

	ValueType value() const;
	void setValue(ValueType value);

	/// @name BItemVisitor Support
	/// @{
	virtual void accept(BItemVisitor *visitor) override;
	/// @}

private:
	explicit BInteger(ValueType value);

private:
	ValueType _value;
};

} // namespace bencoding

#endif
