/**
* @file      PrettyPrinter.cpp
* @copyright (c) 2014 by Petr Zemek (s3rvac@gmail.com) and contributors
* @license   BSD, see the @c LICENSE file for more details
* @brief     Implementation of the PrettyPrinter class.
*/

#include "PrettyPrinter.h"

#include "BDictionary.h"
#include "BInteger.h"
#include "BList.h"
#include "BString.h"
#include "Utils.h"

namespace bencoding {

/**
* @brief Constructs a printer.
*/
PrettyPrinter::PrettyPrinter() = default;

/**
* @brief Creates a new printer.
*/
std::unique_ptr<PrettyPrinter> PrettyPrinter::create() {
	return std::unique_ptr<PrettyPrinter>(new PrettyPrinter());
}

/**
* @brief Returns a pretty representation of @a data.
*
* @param[in] data Data to return a pretty representation for.
* @param[in] indent A single level of indentation.
*/
std::string PrettyPrinter::getPrettyRepr(std::shared_ptr<BItem> data,
		const std::string &indent) {
	prettyRepr.clear();
	indentLevel = indent;
	currentIndent.clear();
	data->accept(this);
	return prettyRepr;
}

/**
* @brief Stores the current indentation into @c prettyRepr.
*/
void PrettyPrinter::storeCurrentIndent() {
	prettyRepr += currentIndent;
}

/**
* @brief Increases the current indentation by a single level.
*/
void PrettyPrinter::increaseIndentLevel() {
	currentIndent += indentLevel;
}

/**
* @brief Decreases the current indentation by a single level.
*/
void PrettyPrinter::decreaseIndentLevel() {
	currentIndent = currentIndent.substr(0,
		currentIndent.size() - indentLevel.size());
}

void PrettyPrinter::visit(BDictionary *bDictionary) {
	//
	// Format:
	//
	//    {
	//        "key1": value1,
	//        "key2": value2,
	//        ...
	//    }
	//
	prettyRepr += "{\n";
	increaseIndentLevel();
	bool putComma = false;
	for (auto &item : *bDictionary) {
		if (putComma) {
			prettyRepr += ",\n";
		}
		storeCurrentIndent();
		item.first->accept(this);
		prettyRepr += ": ";
		item.second->accept(this);
		putComma = true;
	}
	if (!bDictionary->empty()) {
		prettyRepr += "\n";
	}
	decreaseIndentLevel();
	storeCurrentIndent();
	prettyRepr += "}";
}

void PrettyPrinter::visit(BInteger *bInteger) {
	//
	// Format (the same with and without indentation):
	//
	//     int
	//
	prettyRepr += std::to_string(bInteger->value());
}

void PrettyPrinter::visit(BList *bList) {
	//
	// Format:
	//
	//     [
	//         item1,
	//         item2,
	//         ...
	//     ]
	//
	prettyRepr += "[\n";
	increaseIndentLevel();
	bool putComma = false;
	for (auto bItem : *bList) {
		if (putComma) {
			prettyRepr += ",\n";
		}
		storeCurrentIndent();
		bItem->accept(this);
		putComma = true;
	}
	if (!bList->empty()) {
		prettyRepr += "\n";
	}
	decreaseIndentLevel();
	storeCurrentIndent();
	prettyRepr += "]";
}

void PrettyPrinter::visit(BString *bString) {
	//
	// Format (the same with and without indentation):
	//
	//     "string"
	//
	// We have to put a backslash before quotes, i.e. replace " with \".
	prettyRepr += '"' + replace(bString->value(), '"', std::string(R"(\")")) + '"';
}

/**
* @brief Returns a pretty representation of @a data.
*
* This function can be handy if you just want to pretty-print data without
* explicitly creating a pretty printer and calling @c encode() on it.
*
* See PrettyPrinter::getPrettyRepr() for more details.
*/
std::string getPrettyRepr(std::shared_ptr<BItem> data,
		const std::string &indent) {
	auto prettyPrinter = PrettyPrinter::create();
	return prettyPrinter->getPrettyRepr(data, indent);
}

} // namespace bencoding
