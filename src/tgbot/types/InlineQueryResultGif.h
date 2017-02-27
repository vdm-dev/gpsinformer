//
// Created by Andrea Giove on 27/03/16.
//

#ifndef TGBOT_INLINEQUERYRESULTGIF_H
#define TGBOT_INLINEQUERYRESULTGIF_H

#include <string>
#include <memory>

#include "tgbot/types/InlineQueryResult.h"

namespace TgBot {

/**
 * Represents a link to an animated GIF file.
 * @ingroup types
 */
class InlineQueryResultGif : public InlineQueryResult {
public:
	static const std::string TYPE;

	typedef std::shared_ptr<InlineQueryResultGif> Ptr;

	InlineQueryResultGif() {
		this->type = TYPE;
		this->gifWidth = 0;
		this->gifHeight = 0;
	}

	/**
	 * A valid URL for the GIF file.
	 */
	std::string gifUrl;

	/**
	 * Optional. Width of the GIF.
	 */
	int32_t gifWidth;

	/**
	 * Optional. Height of the GIF.
	 */
	int32_t gifHeight;

	/**
	 * URL of the static thumbnail for the result (jpeg or gif)
	 */
	std::string thumbUrl;

};
}

#endif //TGBOT_INLINEQUERYRESULTGIF_H
