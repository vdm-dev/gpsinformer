//
//  Copyright (c) 2017 Dmitry Lavygin (vdm.inbox@gmail.com)
//  Copyright (c) 2015 Oleg Morozenkov
// 
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
// 
//  The above copyright notice and this permission notice shall be included in all
//  copies or substantial portions of the Software.
// 
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//  SOFTWARE.
//


#ifndef HttpParser_INCLUDED
#define HttpParser_INCLUDED


#include <string>
#include <map>
#include <vector>

#include "Url.h"
#include "HttpReqArg.h"


class HttpParser
{
public:
	static HttpParser& getInstance();

	static std::string generateRequest(const Url& url, const std::vector<HttpReqArg>& args, bool isKeepAlive = false);
	static std::string generateMultipartFormData(const std::vector<HttpReqArg>& args, const std::string& bondary);
	static std::string generateMultipartBoundary(const std::vector<HttpReqArg>& args);
	static std::string generateWwwFormUrlencoded(const std::vector<HttpReqArg>& args);

    std::string generateResponse(const std::string& data, const std::string& mimeType = "text/plain", short unsigned statusCode = 200, const std::string& statusStr = "OK", bool isKeepAlive = false);

    inline static std::string parseRequest(const std::string& data, std::map<std::string, std::string>& headers) {
		return parseHttp(true, data, headers);
	}

    inline static std::string parseRequest(const std::string& data) {
		return parseHttp(true, data);
	}

    inline static std::string parseResponse(const std::string& data, std::map<std::string, std::string>& headers) {
		return parseHttp(false, data, headers);
	}

    inline static std::string parseResponse(const std::string& data) {
		return parseHttp(false, data);
	}

private:
	static std::string parseHttp(bool isRequest, const std::string& data, std::map<std::string, std::string>& headers);
	static std::string parseHttp(bool isRequest, const std::string& data);
};


#endif // HttpParser_INCLUDED
