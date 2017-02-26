//
//  Copyright (c) 2017 Dmitry Lavygin (vdm.inbox@gmail.com)
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


#ifndef HttpRequest_INCLUDED
#define HttpRequest_INCLUDED


#include "Url.h"


class HttpRequest
{
public:
    HttpRequest();
    HttpRequest(const Url& url, const std::string& data, bool longPoll = false, unsigned int tag = 0);

    virtual ~HttpRequest();

    Url getUrl() const;
    std::string getData() const;
    unsigned int getTag() const;

    bool isLongPoll() const;

    void setUrl(const Url& url);
    void setData(const std::string& data);
    void setTag(unsigned int tag);
    void setLongPoll(bool longPoll = true);

private:
    Url _url;

    std::string _data;

    unsigned int _tag;
    bool _longPoll;
};


inline Url HttpRequest::getUrl() const
{
    return _url;
}

inline std::string HttpRequest::getData() const
{
    return _data;
}

inline unsigned int HttpRequest::getTag() const
{
    return _tag;
}

inline bool HttpRequest::isLongPoll() const
{
    return _longPoll;
}

inline void HttpRequest::setUrl(const Url& url)
{
    _url = url;
}

inline void HttpRequest::setData(const std::string& data)
{
    _data = data;
}

inline void HttpRequest::setTag(unsigned int tag)
{
    _tag = tag;
}

inline void HttpRequest::setLongPoll(bool longPoll)
{
    _longPoll = longPoll;
}


#endif // HttpRequest_INCLUDED