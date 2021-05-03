/**
* @file      BDictionary.h
* @copyright (c) 2014 by Petr Zemek (s3rvac@gmail.com) and contributors
* @license   BSD, see the @c LICENSE file for more details
* @brief     Representation of a dictionary.
*/

#ifndef BENCODING_BDICTIONARY_H
#define BENCODING_BDICTIONARY_H

#include <initializer_list>
#include <map>
#include <memory>

#include "BItem.h"

namespace bencoding {

class BString;

/**
* @brief Representation of a dictionary.
*
* The interface models the interface of @c std::map.
*
* According to the <a
* href="https://wiki.theory.org/BitTorrentSpecification#Bencoding">specification</a>,
* the keys should appear in a lexicographical order by the string values. This
* has the following consequences for the users of the BDictionary class:
*  - When accessing or modifying elements by @c std::shared_ptr<BString>, its
*    value is taken into account, not its address. That is, two different smart
*    pointers pointing to strings with equal values are considered to be equal
*    when accessing or modifying elements.
*  - The iterators return elements in a sorted order by the values of string
*    keys, despite using smart pointers to index the dictionary.
*
* Use create() to create instances of the class.
*/
class BDictionary: public BItem {
private:
	/**
	* @brief Comparator of keys for the dictionary.
	*
	* It compares the instances of BString inside smart pointers by their value
	* rather than by their address.
	*/
	class BStringByValueComparator {
	public:
		bool operator()(const std::shared_ptr<BString> &lhs,
			const std::shared_ptr<BString> &rhs) const;
	};

private:
	/// Mapping of strings into items.
	// See the class description for the reason why a custom comparator is
	// used instead of @c std::less<>.
	using BItemMap = std::map<std::shared_ptr<BString>,
		std::shared_ptr<BItem>, BStringByValueComparator>;

public:
	/// Key type.
	using key_type = BItemMap::key_type;

	/// Mapped type.
	using mapped_type = BItemMap::mapped_type;

	/// Value type.
	using value_type = BItemMap::value_type;

	/// Size type.
	using size_type = BItemMap::size_type;

	/// Reference.
	using reference = BItemMap::reference;

	/// Constant reference.
	using const_reference = BItemMap::const_reference;

	/// Iterator.
	using iterator = BItemMap::iterator;

	/// Constant iterator.
	using const_iterator = BItemMap::const_iterator;

public:
	static std::unique_ptr<BDictionary> create();
	static std::unique_ptr<BDictionary> create(
		std::initializer_list<value_type> items);

	/// @name Capacity
	/// @{
	size_type size() const;
	bool empty() const;
	/// @}

	/// @name Element Access and Modifiers
	/// @{
	mapped_type &operator[](const key_type &key);
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
	BDictionary();
	explicit BDictionary(std::initializer_list<value_type> items);

private:
	/// Underlying list of items.
	BItemMap itemMap;
};

} // namespace bencoding

#endif
