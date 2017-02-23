#ifndef TcpSessionHandler_INCLUDED
#define TcpSessionHandler_INCLUDED


#include "TcpSession.h"


class TcpSessionHandler
{
public:
    enum Reason
    {
        ClosedByUser,
        ClosedByPeer,
        ClosedByServer
    };

    virtual ~TcpSessionHandler() { }

    virtual void handleTcpServerError(const system::error_code& error) { }

    virtual void handleTcpSessionConnect(shared_ptr<TcpSession> session) { }
    virtual void handleTcpSessionDisconnect(shared_ptr<TcpSession> session, Reason reason) { }
    virtual void handleTcpSessionReceivedData(shared_ptr<TcpSession> session, const std::string& data) { }
};


#endif // TcpSessionHandler_INCLUDED
