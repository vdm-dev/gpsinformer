//
// Created by Konstantin Kukin on 27/12/16.
//

#ifndef TGBOT_INLINEQUERYRESULTCACHEDVIDEO_H
#define TGBOT_INLINEQUERYRESULTCACHEDVIDEO_H

#include <string>
#include <memory>

#include "tgbot/types/InlineQueryResult.h"

namespace TgBot {

/**
 * Represents a link to a video file stored on the Telegram servers. 
 * @ingroup types
 */
class InlineQueryResultCachedVideo : public InlineQueryResult {
public:
	static const std::string TYPE;

	typedef std::shared_ptr<InlineQueryResultCachedVideo> Ptr;

	InlineQueryResultCachedVideo() {
		this->type = TYPE;
	}

	/**
	 * A valid file identifier of the video
	 */
	std::string videoFileId;

	/**
	* Optional. Short description of the result
	*/
	std::string description;
};
}

#endif //TGBOT_INLINEQUERYRESULTCACHEDVIDEO_H
