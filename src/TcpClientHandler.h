#ifndef TcpClientHandler_INCLUDED
#define TcpClientHandler_INCLUDED


template <class T>
class TcpClientHandler
{
public:
    enum Reason
    {
        ClosedByUser,
        ClosedByPeer
    };

    virtual ~TcpClientHandler() { }

    virtual void handleTcpClientConnect(T* client) { }
    virtual void handleTcpClientDisconnect(T* client, Reason reason) { }
    virtual void handleTcpClientError(T* client, const system::error_code& error) { }
    virtual void handleTcpClientReceivedData(T* client, const std::string& data) { }
};


#endif // TcpClientHandler_INCLUDED
