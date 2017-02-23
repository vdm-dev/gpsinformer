#ifndef TcpClientHandler_INCLUDED
#define TcpClientHandler_INCLUDED


class TcpClient;


class TcpClientHandler
{
public:
    enum Reason
    {
        ClosedByUser,
        ClosedByPeer
    };

    virtual ~TcpClientHandler() { }

    virtual void handleTcpClientConnect(TcpClient* client) { }
    virtual void handleTcpClientDisconnect(TcpClient* client, Reason reason) { }
    virtual void handleTcpClientError(TcpClient* client, const system::error_code& error) { }
    virtual void handleTcpClientReceivedData(TcpClient* client, const std::string& data) { }
};


#endif // TcpClientHandler_INCLUDED
