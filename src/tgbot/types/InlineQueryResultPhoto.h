//
// Created by Andrea Giove on 26/03/16.
//

#ifndef TGBOT_INLINEQUERYRESULTPHOTO_H
#define TGBOT_INLINEQUERYRESULTPHOTO_H

#include <string>
#include <memory>

#include "tgbot/types/InlineQueryResult.h"

namespace TgBot {

/**
 * Represents a link to a photo.
 * @ingroup types
 */
class InlineQueryResultPhoto : public InlineQueryResult {
public:
	static const std::string TYPE;

	typedef std::shared_ptr<InlineQueryResultPhoto> Ptr;

	InlineQueryResultPhoto() {
		this->type = TYPE;
		this->photoWidth = 0;
		this->photoHeight = 0;
	}

	/**
	 * A valid URL of the photo.
	 */
	std::string photoUrl;

	/**
	 * URL of the thumbnail for the photo
	 */
	std::string thumbUrl;

	/**
	 * Optional. Width of the photo.
	 */
	int32_t photoWidth;

	/**
	 * Optional. Height of the photo.
	 */
	int32_t photoHeight;

	/**
	 * Optional. Short description of the result.
	 */
	std::string description;
};
}

#endif //TGBOT_INLINEQUERYRESULTPHOTO_H
