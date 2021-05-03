/**
* @file      Encoder.h
* @copyright (c) 2014 by Petr Zemek (s3rvac@gmail.com) and contributors
* @license   BSD, see the @c LICENSE file for more details
* @brief     Data encoder.
*/

#ifndef BENCODING_ENCODER_H
#define BENCODING_ENCODER_H

#include <memory>
#include <string>

#include "BItemVisitor.h"

namespace bencoding {

class BItem;

/**
* @brief Data encoder.
*
* The format is based on the <a
* href="https://wiki.theory.org/BitTorrentSpecification#Bencoding">BitTorrent
* specification</a>.
*
* Use create() to create instances.
*/
class Encoder: private BItemVisitor {
public:
	static std::unique_ptr<Encoder> create();

	std::string encode(std::shared_ptr<BItem> data);

private:
	Encoder();

	/// @name BItemVisitor Interface
	/// @{
	virtual void visit(BDictionary *bDictionary) override;
	virtual void visit(BInteger *bInteger) override;
	virtual void visit(BList *bList) override;
	virtual void visit(BString *bString) override;
	/// @}

private:
	/// Encoded items.
	std::string encodedData;
};

/// @name Encoding Without Explicit Encoder Creation
/// @{
std::string encode(std::shared_ptr<BItem> data);
/// @}

} // namespace bencoding

#endif
