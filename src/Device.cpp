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

#include "Device.h"
#include "Application.h"


shared_ptr<TcpSession> Device::_authorizedSession;


Device::Device(shared_ptr<TcpSession> session)
    : _session(session)
{
}

Device::~Device()
{
    if (_authorizedSession == _session)
        _authorizedSession.reset();
}

shared_ptr<TcpSession> Device::authorizedSession()
{
    return _authorizedSession;
}

void Device::processClientData(const std::string& data)
{
    _session->send(data);
}

void Device::processData(const std::string& data)
{
    _buffer += data;

    erase_all(_buffer, "\r");
    erase_all(_buffer, "\n");

    std::string::size_type offset = _buffer.find_first_of(';');

    while (offset != std::string::npos)
    {
        std::string command = _buffer.substr(0, offset);

        _buffer.erase(0, offset + 1);

        offset = _buffer.find_first_of(';');

        if (!command.size())
            continue;

        std::vector<std::string> arguments;

        split(arguments, command, is_any_of(","), token_compress_off);

        if (!arguments.size() || !arguments[0].size())
            continue;

        bool result = true;
        bool transmit = true;

        if (arguments[0] == "##") // Logon Packet
        {
            transmit = false;
            result = commandLogon(arguments);
        }
        else if (_imei.size() && (_authorizedSession == _session))
        {
            if (arguments.size() == 1) // Heartbeat Packet
            {
                transmit = false;
                result  = commandHeartbeat(arguments);
            }
        }

        if (!result || !_imei.size() || (_authorizedSession != _session))
        {
            _session->close();
            return;
        }

        if (transmit)
            Application::instance()->handleDeviceCommand(arguments);
    }

    if (_buffer.size() > 32768)
        _session->close();
}

bool Device::commandLogon(const std::vector<std::string>& arguments)
{
    if (arguments.size() != 3)
        return false;

    if (arguments[2] != "A")
        return false;

    _imei = arguments[1];

    if ((_imei.size() != 20) || (_imei.find("imei:") != 0))
    {
        _imei = "";

        return false;
    }

    _imei = _imei.substr(5);

    std::string imei = Application::instance()->_settings.get("imei", "");

    if (_imei == imei)
    {
        BOOST_LOG_TRIVIAL(debug) << "Receiver get authorized imei (" << imei << ")";

        _authorizedSession = _session;

        _session->send("LOAD");
    }

    return true;
}

bool Device::commandHeartbeat(const std::vector<std::string>& arguments)
{
    if (arguments[0] != _imei)
        return false;

    BOOST_LOG_TRIVIAL(debug) << "Receiver get heartbeat message";

    _session->send("ON");

    return true;
}
