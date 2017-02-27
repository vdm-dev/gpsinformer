//
// Created by Konstantin Kukin on 27/12/16.
//

#ifndef TGBOT_INLINEQUERYRESULTCACHEDVOICE_H
#define TGBOT_INLINEQUERYRESULTCACHEDVOICE_H

#include <string>
#include <memory>

#include "tgbot/types/InlineQueryResult.h"

namespace TgBot {

/**
 * Represents a link to a voice message stored on the Telegram servers.
 * @ingroup types
 */
class InlineQueryResultCachedVoice : public InlineQueryResult {
public:
	static const std::string TYPE;

	typedef std::shared_ptr<InlineQueryResultCachedVoice> Ptr;

	InlineQueryResultCachedVoice() {
		this->type = TYPE;
	}

	/**
	 * A valid file identifier of the voice message
	 */
	std::string voiceFileId;
};
}

#endif //TGBOT_INLINEQUERYRESULTCACHEDVOICE_H
