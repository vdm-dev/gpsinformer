//
// Created by Andrea Giove on 27/03/16.
//

#ifndef TGBOT_INLINEQUERYRESULTVIDEO_H
#define TGBOT_INLINEQUERYRESULTVIDEO_H

namespace TgBot {

/**
 * Represents link to a page containing an embedded video player or a video file.
 * @ingroup types
 */
class InlineQueryResultVideo : public InlineQueryResult {
public:
	static const std::string TYPE;

	typedef std::shared_ptr<InlineQueryResultVideo> Ptr;

	InlineQueryResultVideo() {
		this->type = TYPE;
		this->videoWidth = 0;
		this->videoHeight = 0;
		this->videoDuration = 0;
	};

	/**
	 * A valid URL for the embedded video player or video file.
	 */
	std::string videoUrl;

	/**
	 * Mime type of the content of video url, "text/html" or "video/mp4".
	 */
	std::string mimeType;

	/**
	* URL of the thumbnail (jpeg only) for the video
	*/
	std::string thumbUrl;

	/**
	 * Optional. Video width.
	 */
	int32_t videoWidth;

	/**
	 * Optional. Video height.
	 */
	int32_t videoHeight;

	/**
	 * Optional. Video duration.
	 */
	int32_t videoDuration;

	/**
	 * Optional. Short description of the result.
	 */
	std::string description;

};
}

#endif //TGBOT_INLINEQUERYRESULTVIDEO_H
