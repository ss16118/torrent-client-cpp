/**
* @file      BItem.h
* @copyright (c) 2014 by Petr Zemek (s3rvac@gmail.com) and contributors
* @license   BSD, see the @c LICENSE file for more details
* @brief     Base class for all items (integers, strings, etc.).
*/

#ifndef BENCODING_BITEM_H
#define BENCODING_BITEM_H

#include <memory>

namespace bencoding {

class BItemVisitor;

/**
* @brief Base class for all items (integers, strings, etc.).
*/
class BItem: public std::enable_shared_from_this<BItem> {
public:
	virtual ~BItem() = 0;

	/// @name BItemVisitor Support
	/// @{

	/**
	* @brief Accepts the item by the given @a visitor.
	*
	* Subclasses should implement this function in this way:
	* @code
	* void Subclass::accept(BItemVisitor *visitor) {
	*     visitor->visit(this);
	* }
	* @endcode
	*/
	virtual void accept(BItemVisitor *visitor) = 0;

	/// @}

	/**
	* @brief Casts the item to the given subclass of BItem.
	*
	* @tparam T Subclass of BItem.
	*/
	template <typename T>
	std::shared_ptr<T> as() {
		static_assert(std::is_base_of<BItem, T>::value,
			"T has to be a subclass of BItem");

		return std::dynamic_pointer_cast<T>(shared_from_this());
	}

protected:
	BItem();

private:
	// Disable copy construction and assignment for this class and subclasses.
	BItem(const BItem &) = delete;
	BItem &operator=(const BItem &) = delete;
};

} // namespace bencoding

#endif
