//
// Created by Konstantin Kukin on 27/12/16
//

#ifndef TGBOT_INLINEQUERYRESULTDOCUMENT_H
#define TGBOT_INLINEQUERYRESULTDOCUMENT_H

#include <string>
#include <memory>

#include "tgbot/types/InlineQueryResult.h"

namespace TgBot {

/**
 * Represents a link to a file.
 * @ingroup types
 */
class InlineQueryResultDocument : public InlineQueryResult {
public:
	static const std::string TYPE;

	typedef std::shared_ptr<InlineQueryResultDocument> Ptr;

	InlineQueryResultDocument() {
		this->type = TYPE;
		this->thumbHeight = 0;
		this->thumbWidth = 0;
	}

	/**
	 * A valid URL for the file
	 */
	std::string documentUrl;

	/**
	 * Mime type of the content of the file, either �application/pdf� or �application/zip�
	 */
	std::string mimeType;

	/**
	 * Optional. Short description of the result
	 */
	std::string description;

	/**
	* Optional. Url of the thumbnail for the result
	*/
	std::string thumbUrl;

	/**
	* Optional. Thumbnail width.
	*/
	int32_t thumbWidth;

	/**
	* Optinal. Thumbnail height
	*/
	int32_t thumbHeight;
};
}

#endif //TGBOT_INLINEQUERYRESULTDOCUMENT_H
