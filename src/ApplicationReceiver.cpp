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


void Application::startReceiver()
{
    bool           enabled           = _settings.get("receiver.enabled", false);
    std::string    host              = _settings.get("receiver.host", std::string("0.0.0.0"));
    unsigned short port              = _settings.get("receiver.port", 9090);

    if (!enabled)
    {
        BOOST_LOG_TRIVIAL(info) << "Receiver is disabled";
        return;
    }

    BOOST_LOG_TRIVIAL(info) << "Receiver is listening on <" << host << ":" << port << ">";

    _receiver.listen(host, port);
}

void Application::handleTcpServerError(const system::error_code& error)
{
    _devices.clear();

    std::string message = "Receiver error: " + error.message();

    int64_t rebindDelay = _settings.get("receiver.rebind_delay", 0);

    system::error_code errorCode;

    _receiverTimer.expires_from_now(posix_time::milliseconds(rebindDelay), errorCode);
    if (errorCode || (rebindDelay < 1))
    {
        BOOST_LOG_TRIVIAL(warning) << message << " (no auto-rebind)";
        return;
    }

    BOOST_LOG_TRIVIAL(warning) << message << ". Trying to rebind in " << (rebindDelay / 1000.0) << " second(s)";

    _receiverTimer.async_wait(bind(&Application::handleReceiverTimer, this, asio::placeholders::error));
}

void Application::handleTcpSessionConnect(shared_ptr<TcpSession> session)
{
    _devices[session] = make_shared<Device>(session);


    system::error_code error;

    std::string address = session->endpoint().address().to_string(error);

    if (error)
        address = "unknown";

    BOOST_LOG_TRIVIAL(info) << "Client <" << address << "> connected";
}

void Application::handleTcpSessionDisconnect(shared_ptr<TcpSession> session, TcpSessionHandler::Reason reason)
{
    if (!session->isActive())
        return;

    _devices.erase(session);

    if (reason == TcpSessionHandler::ClosedByServer)
        return;

    system::error_code error;

    std::string address = session->endpoint().address().to_string(error);

    if (error)
        address = "unknown";

    BOOST_LOG_TRIVIAL(info) << "Client <" << address << "> disconnected";
}

void Application::handleTcpSessionReceivedData(shared_ptr<TcpSession> session, const std::string& data)
{
    if (_devices.count(session))
        _devices[session]->processData(data);
}

void Application::handleReceiverTimer(const system::error_code& error)
{
    startReceiver();
}
