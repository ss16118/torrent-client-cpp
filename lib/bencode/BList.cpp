/**
* @file      BList.cpp
* @copyright (c) 2014 by Petr Zemek (s3rvac@gmail.com) and contributors
* @license   BSD, see the @c LICENSE file for more details
* @brief     Implementation of the BList class.
*/

#include "BList.h"
#include "BDictionary.h"
#include <cassert>

#include "BItemVisitor.h"

namespace bencoding {

/**
* @brief Constructs an empty list.
*/
BList::BList() = default;

/**
* @brief Constructs a list containing the given @a items.
*/
BList::BList(std::initializer_list<value_type> items):
	itemList(items) {}

/**
* @brief Creates and returns a new list.
*/
std::unique_ptr<BList> BList::create() {
	return std::unique_ptr<BList>(new BList());
}

/**
* @brief Creates a returns a new list containing the given @a items.
*/
std::unique_ptr<BList> BList::create(std::initializer_list<value_type> items) {
	return std::unique_ptr<BList>(new BList(items));
}

/**
* @brief Returns the number of items in the list.
*/
BList::size_type BList::size() const {
	return itemList.size();
}

/**
* @brief Checks if the list is empty.
*
* @return @c true if the list is empty, @c false otherwise.
*/
bool BList::empty() const {
	return itemList.empty();
}

/**
* @brief Appends the given item to the end of the list.
*
* @preconditions
*  - @a bItem is non-null
*/
void BList::push_back(const value_type &bItem) {
	assert(bItem && "cannot add a null item to the list");

	itemList.push_back(bItem);
}

/**
* @brief Removes the last element of the list.
*
* References and iterators to the erased element are invalidated.
*
* @preconditions
*  - list is non-empty
*/
void BList::pop_back() {
	assert(!empty() && "cannot call pop_back() on an empty list");

	itemList.pop_back();
}

/**
* @brief Returns a reference to the first item in the list.
*
* @preconditions
*  - list is non-empty
*/
BList::reference BList::front() {
	assert(!empty() && "cannot call front() on an empty list");

	return itemList.front();
}

/**
* @brief Returns a constant reference to the first item in the list.
*
* @preconditions
*  - list is non-empty
*/
BList::const_reference BList::front() const {
	assert(!empty() && "cannot call front() on an empty list");

	return itemList.front();
}

/**
* @brief Returns a reference to the last item in the list.
*
* @preconditions
*  - list is non-empty
*/
BList::reference BList::back() {
	assert(!empty() && "cannot call back() on an empty list");

	return itemList.back();
}

/**
* @brief Returns a constant reference to the last item in the list.
*
* @preconditions
*  - list is non-empty
*/
BList::const_reference BList::back() const {
	assert(!empty() && "cannot call back() on an empty list");

	return itemList.back();
}

/**
* @brief Returns an iterator to the beginning of the list.
*/
BList::iterator BList::begin() {
	return itemList.begin();
}

/**
* @brief Returns an iterator to the end of the list.
*/
BList::iterator BList::end() {
	return itemList.end();
}

/**
* @brief Returns a constant iterator to the beginning of the list.
*/
BList::const_iterator BList::begin() const {
	return itemList.begin();
}

/**
* @brief Returns a constant iterator to the end of the list.
*/
BList::const_iterator BList::end() const {
	return itemList.end();
}

/**
* @brief Returns a constant iterator to the beginning of the list.
*/
BList::const_iterator BList::cbegin() const {
	return itemList.cbegin();
}

/**
* @brief Returns a constant iterator to the end of the list.
*/
BList::const_iterator BList::cend() const {
	return itemList.cend();
}

void BList::accept(BItemVisitor *visitor) {
	visitor->visit(this);
}

/**
 * If the list contains dictionaries, searches all the dictionaries for the
 * given key, and return the corresponding value if found.
 */
std::shared_ptr<BItem> BList::getValue(const std::string &key) {
    for (const auto& item : *this)
    {
        if (typeid(*item) == typeid(BDictionary))
        {
            std::shared_ptr<BDictionary> subDictionary = std::dynamic_pointer_cast<BDictionary>(item);
            auto potentialValue = subDictionary->getValue(key);
            if (potentialValue)
                return potentialValue;
        }
    }
    return std::shared_ptr<BItem>();
}

} // namespace bencoding
