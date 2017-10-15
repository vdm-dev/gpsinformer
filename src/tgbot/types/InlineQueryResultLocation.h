//
// Created by Konstantin Kukin on 27/12/16
//

#ifndef TGBOT_INLINEQUERYRESULTLOCATION_H
#define TGBOT_INLINEQUERYRESULTLOCATION_H

#include <string>
#include <memory>

#include "tgbot/types/InlineQueryResult.h"

namespace TgBot {

/**
 * Represents a location on a map. 
 * @ingroup types
 */
class InlineQueryResultLocation : public InlineQueryResult {
public:
	static const std::string TYPE;

	typedef std::shared_ptr<InlineQueryResultLocation> Ptr;

	InlineQueryResultLocation()
        : latitude(0.0)
        , longitude(0.0)
        , thumbHeight(0)
        , thumbWidth(0)
    {
		this->type = TYPE;
	}

	/**
	* Location latitude in degrees
	*/
	float latitude;

	/**
	* Location longitude in degrees
	*/
	float longitude;

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

#endif //TGBOT_INLINEQUERYRESULTLOCATION_H
