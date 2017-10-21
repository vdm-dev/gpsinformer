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


#include "StdAfx.h"

#include "Application.h"


void Application::handleTrackerEvent(const GpsMessage& data)
{
    for (size_t i = 0; i < _userSettings.size(); ++i)
    {
        switch (_userSettings[i].status)
        {
        case UserSettings::Insane:
            sendGpsStatus(data, _userSettings[i].id);
            break;
        case UserSettings::Paranoid:
            if (data.keyword != _lastMessage.keyword)
                sendGpsStatus(data, _userSettings[i].id);
            break;
        case UserSettings::Alert:
            if (data.keyword != _lastMessage.keyword &&
                !iequals(data.keyword, "connect") &&
                !iequals(data.keyword, "disconnect"))
            {
                sendGpsStatus(data, _userSettings[i].id);
            }
            break;
        }
    }

    dbAddGpsData(data);

    _lastMessage = data;
}