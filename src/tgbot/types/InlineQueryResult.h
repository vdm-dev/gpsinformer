//
// Created by Andrea Giove on 26/03/16.
//

#ifndef TGBOT_INLINEQUERYRESULT_H
#define TGBOT_INLINEQUERYRESULT_H

#include <memory>
#include <string>

#include "tgbot/types/InlineKeyboardMarkup.h"
#include "tgbot/types/InputMessageContent.h"

namespace TgBot {

/**
 * This abstract class is base of all inline query results.
 * @ingroup types
 */
class InlineQueryResult {
public:
	typedef std::shared_ptr<InlineQueryResult> Ptr;

	InlineQueryResult() {

	}

	virtual ~InlineQueryResult() { }

	/**
	 * Type of the result.
	 */
	std::string type;

	/**
	 * Unique identifier for this result. (1-64 bytes)
	 */
	std::string id;

	/**
	 * Requred, optional or missing. See description of derived classes. Title of the result.
	 */
	std::string title;

	/**
	 * Optional or missing. See description of derived classes. Caption of the file to be sent, 0-200 characters
	 */
	std::string caption;

	/**
	 * Optional. Inline keyboard attached to the message
	 */
	InlineKeyboardMarkup::Ptr replyMarkup;

	/**
	 * Requred, optional or missing. See description of derived classes. Content of the message to be sent
	 */
	InputMessageContent::Ptr inputMessageContent;
};
}

#endif //TGBOT_INLINEQUERYRESULT_H
