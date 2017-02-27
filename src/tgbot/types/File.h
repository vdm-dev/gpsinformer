//
// Created by Konstantin Kukin on 26/12/16.
//

#ifndef TGBOT_CPP_FILE_H
#define TGBOT_CPP_FILE_H

#include <string>
#include <memory>

namespace TgBot {

/**
 * This object represents a file ready to be downloaded. 
 * The file can be downloaded via the link https://api.telegram.org/file/bot<token>/<file_path>.
 * It is guaranteed that the link will be valid for at least 1 hour. 
 * When the link expires, a new one can be requested by calling getFile.
 * Maximum file size to download is 20 MB
 * @ingroup types
 */
class File {

public:
	typedef std::shared_ptr<File> Ptr;

	/**
	 * Unique identifier for this file
	 */
	std::string fileId;

	/**
	 * Optional. File size, if known
	 */
	int32_t fileSize;

	/**
	 * Optional. File path.
	 * Use https://api.telegram.org/file/bot<token>/<file_path> to get the file.
	 */
	std::string filePath;
};

}

#endif //TGBOT_CPP_FILE_H
