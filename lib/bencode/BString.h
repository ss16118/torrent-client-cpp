/**
* @file      BString.h
* @copyright (c) 2014 by Petr Zemek (s3rvac@gmail.com) and contributors
* @license   BSD, see the @c LICENSE file for more details
* @brief     Representation of a string.
*/

#ifndef BENCODING_BSTRING_H
#define BENCODING_BSTRING_H

#include <memory>
#include <string>

#include "BItem.h"

namespace bencoding {

/**
* @brief Representation of a string.
*
* Use create() to create instances of the class.
*/
class BString: public BItem {
public:
	/// Type of the underlying string value.
	using ValueType = std::string;

public:
	static std::unique_ptr<BString> create(ValueType value);

	ValueType value() const;
	void setValue(ValueType value);
	ValueType::size_type length() const;

	/// @name BItemVisitor Support
	/// @{
	virtual void accept(BItemVisitor *visitor) override;
	/// @}

private:
	explicit BString(ValueType value);

private:
	ValueType _value;
};

} // namespace bencoding

#endif
