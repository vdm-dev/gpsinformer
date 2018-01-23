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


#ifndef SslClient_INCLUDED
#define SslClient_INCLUDED


#include <mbedtls/debug.h>
#include <mbedtls/ssl.h>
#include <mbedtls/entropy.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/error.h>
#include <mbedtls/certs.h>

#include "TcpClient.h"
#include "TcpClientHandler.h"


class SslClient : public TcpClientHandler<TcpClient>
{
public:
    SslClient(asio::io_service& ioService, TcpClientHandler<SslClient>* handler = 0);
    ~SslClient();

    void connect(const std::string& server, unsigned short port);
    void connect(const std::string& server, const std::string& protocol);
    void disconnect();

    void send(const std::string& data);

    bool isConnected() const;
    bool isConnecting() const;
    bool isDisconnecting() const;
    void setEventHandler(TcpClientHandler<SslClient>* handler);

private:
    enum Status
    {
        Disconnected,
        Connecting,
        Handshake,
        Connected,
        Disconnecting,
        Error
    };

    static void sslDebugCallback(void *context, int level, const char *file, int line, const char *string);
    static int  sslReadCallback(void* context, unsigned char* buffer, size_t length);
    static int  sslWriteCallback(void* context, const unsigned char* buffer, size_t length);

    bool sslInitialize();
    void sslFree();

    bool handleAnything(Status handleStatus, const system::error_code& error);
    void handleHandshake();
    bool handleRead();
    void handleWrite();

    // TCP Client Handlers
    void handleTcpClientConnect(TcpClient* client);
    void handleTcpClientDisconnect(TcpClient* client, TcpClientHandler::Reason reason);
    void handleTcpClientError(TcpClient* client, const system::error_code& error);
    void handleTcpClientReceivedData(TcpClient* client, const std::string& data);

    TcpClient _tcpClient;

    asio::deadline_timer _timer;

    mbedtls_ssl_context _sslContext;
    mbedtls_ssl_config _sslConfiguration;
    mbedtls_x509_crt _sslCaRoot;
    mbedtls_ctr_drbg_context _sslCtrDrbg;
    mbedtls_entropy_context _sslEntropy;

    std::vector<char> _sslBuffer;
    std::deque<std::string> _writeBuffer;

    TcpClientHandler<SslClient>* _handler;

    Status _status;

    int _ioCount;

    bool _sslInitialized;
    bool _wasConnected;
};


inline bool SslClient::isConnected() const
{
    return _status == Connected;
}

inline bool SslClient::isConnecting() const
{
    return _status == Connecting || _status == Handshake;
}

inline bool SslClient::isDisconnecting() const
{
    return _status == Disconnecting || _status == Error;
}

inline void SslClient::setEventHandler(TcpClientHandler<SslClient>* handler)
{
    _handler = handler;
}


#endif // SslClient_INCLUDED
