/**
* @file      BList.h
* @copyright (c) 2014 by Petr Zemek (s3rvac@gmail.com) and contributors
* @license   BSD, see the @c LICENSE file for more details
* @brief     Representation of a list.
*/

#ifndef BENCODING_BLIST_H
#define BENCODING_BLIST_H

#include <initializer_list>
#include <list>
#include <memory>

#include "BItem.h"

namespace bencoding {

/**
* @brief Representation of a list.
*
* The interface models the interface of @c std::list.
*
* Use create() to create instances of the class.
*/
class BList: public BItem {
private:
	/// List of items.
	using BItemList = std::list<std::shared_ptr<BItem>>;

public:
	/// Value type.
	using value_type = BItemList::value_type;

	/// Size type.
	using size_type = BItemList::size_type;

	/// Reference.
	using reference = BItemList::reference;

	/// Constant reference.
	using const_reference = BItemList::const_reference;

	/// Iterator (@c BidirectionalIterator).
	using iterator = BItemList::iterator;

	/// Constant iterator (constant @c BidirectionalIterator).
	using const_iterator = BItemList::const_iterator;

public:
	static std::unique_ptr<BList> create();
	static std::unique_ptr<BList> create(std::initializer_list<value_type> items);

	/// @name Capacity
	/// @{
	size_type size() const;
	bool empty() const;
	/// @}

	/// @name Modifiers
	/// @{
	void push_back(const value_type &bItem);
	void pop_back();
	/// @}

	/// @name Element Access
	/// @{
	reference front();
	const_reference front() const;
	reference back();
	const_reference back() const;
	/// @}

	/// @name Iterators
	/// @{
	iterator begin();
	iterator end();
	const_iterator begin() const;
	const_iterator end() const;
	const_iterator cbegin() const;
	const_iterator cend() const;
	/// @}

	/// @name BItemVisitor Support
	/// @{
	virtual void accept(BItemVisitor *visitor) override;
	/// @}
    std::shared_ptr<BItem> getValue(const std::string& key);
private:
	BList();
	explicit BList(std::initializer_list<value_type> items);

private:
	/// Underlying list of items.
	BItemList itemList;
};

} // namespace bencoding

#endif
