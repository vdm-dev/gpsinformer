//
// Created by Oleg Morozenkov on 25.01.17.
//

#include "tgbot/types/InputFile.h"

#include <fstream>
#include <sstream>

#include <boost/filesystem.hpp>

using namespace std;

namespace TgBot {

InputFile::Ptr InputFile::fromFile(const string& filePath, const string& mimeType) {
	InputFile::Ptr result(new InputFile);

    ifstream in(filePath, ios::in | ios::binary);

    if (in) {
        ostringstream contents;
        contents << in.rdbuf();
        in.close();
        result->data = contents.str();
    }

    boost::filesystem::path path = filePath;

	result->mimeType = mimeType;
    result->fileName = path.filename().string();
	return result;
}

};
