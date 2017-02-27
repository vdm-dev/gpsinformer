//
// Created by Konstantin Kukin on 26/12/16.
//

#ifndef TGBOT_INPUTLOCATIONMESSAGECONTENT_H
#define TGBOT_INPUTLOCATIONMESSAGECONTENT_H

#include <memory>

namespace TgBot {

/**
* Represents the content of a location message to be sent as the result of an inline query.
* @ingroup types
*/
class InputLocationMessageContent : public InputMessageContent {
public:
	typedef std::shared_ptr<InputLocationMessageContent> Ptr;

	InputLocationMessageContent() :
		InputMessageContent("InputLocationMessageContent")
	{}

	/**
	* Latitude of the location in degrees
	*/
	float latitude;

	/**
	* Longitude of the location in degrees
	*/
	float longitude;

	virtual ~InputLocationMessageContent() { }
};
}

#endif //TGBOT_INPUTLOCATIONMESSAGECONTENT_H
