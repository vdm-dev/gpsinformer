#ifndef TCPClientHandler_INCLUDED
#define TCPClientHandler_INCLUDED

class TCPClient;

class TCPClientHandler
{
public:
    enum Reason
    {
        ClosedByUser,
        ClosedByPeer
    };

    virtual ~TCPClientHandler() { }

    virtual void handleTCPClientConnect(TCPClient* client) { }
    virtual void handleTCPClientDisconnect(TCPClient* client, Reason reason) { }
    virtual void handleTCPClientError(TCPClient* client, const system::error_code& error) { }
    virtual void handleTCPClientReceivedData(TCPClient* client, const std::string& data) { }
};

#endif // TCPClientHandler_INCLUDED
