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


void Application::startTransmitter()
{
    bool           enabled        = _settings.get("transmitter.enabled", false);
    std::string    host           = _settings.get("transmitter.host", std::string());
    unsigned short port           = _settings.get("transmitter.port", 0);

    if (!enabled)
    {
        BOOST_LOG_TRIVIAL(info) << "Transmitter is disabled";
        return;
    }

    BOOST_LOG_TRIVIAL(info) << "Transmitter is connecting to <" << host << ":" << port << ">";

    _transmitter.connect(host, port);
}

void Application::handleTcpClientConnect(TcpClient* client)
{
    _transmitterAuthorized = false;
    _transmitterBuffer.clear();

    system::error_code error;

    std::string address = "unknown";
    unsigned short port = 0;

    asio::ip::tcp::endpoint endpoint = client->socket().remote_endpoint(error);
    if (!error)
    {
        address = endpoint.address().to_string(error);
        port = endpoint.port();
    }

    BOOST_LOG_TRIVIAL(info) << "Transmitter connected to <" << address << ":" << port << ">";

    std::string imei = _settings.get("imei", "");

    if (imei.size())
    {
        BOOST_LOG_TRIVIAL(debug) << "Transmitter is sending authentication message";
        _transmitter.send("##,imei:" + imei + ",A;\r\n");
    }
}

void Application::handleTcpClientDisconnect(TcpClient* client, TcpClientHandler::Reason reason)
{
    _transmitterAuthorized = false;
    _transmitterBuffer.clear();

    if (reason == TcpClientHandler::ClosedByUser)
    {
        BOOST_LOG_TRIVIAL(warning) << "Transmitter disconnected by user";
        return;
    }

    system::error_code error;

    handleTcpClientError(client, error);
}

void Application::handleTcpClientError(TcpClient* client, const system::error_code& error)
{
    _transmitterAuthorized = false;
    _transmitterBuffer.clear();

    std::string message = "Transmitter disconnected by peer";

    if (error)
        message = "Transmitter can't connect to a remote host";

    int64_t reconnectDelay = _settings.get("transmitter.reconnect_delay", 0);

    system::error_code errorCode;

    _transmitterTimer.expires_from_now(posix_time::milliseconds(reconnectDelay), errorCode);
    if (errorCode || (reconnectDelay < 1))
    {
        BOOST_LOG_TRIVIAL(warning) << message << " (no auto-reconnect)";
        return;
    }

    BOOST_LOG_TRIVIAL(warning) << message << ". Trying to reconnect in " << (reconnectDelay / 1000.0) << " second(s)";

    _transmitterTimer.async_wait(bind(&Application::handleTransmitterTimer, this, asio::placeholders::error));
}

void Application::handleTcpClientReceivedData(TcpClient* client, const std::string& data)
{
    _transmitterBuffer += data;

    erase_all(_transmitterBuffer, "\r");
    erase_all(_transmitterBuffer, "\n");

    int64_t heartbeat = _settings.get("transmitter.heartbeat", 30000);

    if (heartbeat < 10000)
        heartbeat = 10000;

    system::error_code errorCode;

    _transmitterTimer.expires_from_now(posix_time::milliseconds(heartbeat), errorCode);

    while (true)
    {
        std::string::size_type offset = _transmitterBuffer.find_first_of(';');

        std::string command = _transmitterBuffer.substr(0, offset);

        if (offset == std::string::npos)
        {
            if (_transmitterBuffer.size() < 2)
                break;

            if (!_transmitterAuthorized)
            {

                if (_transmitterBuffer.size() < 4)
                    break;

                std::string::size_type subOffset = _transmitterBuffer.find("LOAD");

                if (subOffset != 0)
                {
                    // Access denied or protocol error
                    BOOST_LOG_TRIVIAL(error) << "Transmitter access denied";
                    _transmitter.disconnect();
                    return;
                }
                else
                {
                    BOOST_LOG_TRIVIAL(info) << "Transmitter access granted";
                    _transmitterAuthorized = true;
                    _transmitterBuffer.erase(0, 4);
                    _transmitterTimer.async_wait(bind(&Application::handleTransmitterTimer, this, asio::placeholders::error));
                    continue;
                }
            }

            std::string::size_type subOffset = _transmitterBuffer.find("ON");

            if (subOffset == 0)
            {
                BOOST_LOG_TRIVIAL(debug) << "Transmitter received heartbeat answer";
                _transmitterBuffer.erase(0, 2);
                _transmitterTimer.async_wait(bind(&Application::handleTransmitterTimer, this, asio::placeholders::error));
                continue;
            }

            break;
        }

        if (!_transmitterAuthorized)
        {
            // Access denied or protocol error
            BOOST_LOG_TRIVIAL(error) << "Transmitter access denied (protocol error)";
            _transmitter.disconnect();
            return;
        }

        _transmitterBuffer.erase(0, offset + 1);

        if (command.size() < 2)
            continue;

        std::string::size_type subOffset = command.find("ON");

        if (subOffset == 0)
        {
            BOOST_LOG_TRIVIAL(debug) << "Transmitter received heartbeat answer (with command)";
            command.erase(0, 2);
            _transmitterTimer.async_wait(bind(&Application::handleTransmitterTimer, this, asio::placeholders::error));
        }

        if (!command.size())
            continue;

        shared_ptr<TcpSession> session = Device::authorizedSession();

        if (session && _devices.count(session))
            _devices[session]->processClientData(command + ";\r\n");
    }

    if (_transmitterBuffer.size() > 32768)
        _transmitter.disconnect();
}

void Application::handleTransmitterTimer(const system::error_code& error)
{
    if (_transmitterAuthorized)
    {
        std::string imei = _settings.get("imei", "");

        if (imei.size())
        {
            BOOST_LOG_TRIVIAL(debug) << "Transmitter is sending heartbeat message";
            _transmitter.send(imei +";\r\n");
        }
    }
    else
    {
        startTransmitter();
    }
}
