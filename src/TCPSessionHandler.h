#ifndef TCPSessionHandler_INCLUDED
#define TCPSessionHandler_INCLUDED

#include "TCPSession.h"

class TCPSessionHandler
{
public:
    enum Reason
    {
        ClosedByUser,
        ClosedByPeer,
        ClosedByServer
    };

    virtual ~TCPSessionHandler() { }

    virtual void handleTCPServerError(const system::error_code& error) { }

    virtual void handleTCPSessionConnect(shared_ptr<TCPSession> session) { }
    virtual void handleTCPSessionDisconnect(shared_ptr<TCPSession> session, Reason reason) { }
    virtual void handleTCPSessionReceivedData(shared_ptr<TCPSession> session, const std::string& data) { }
};

#endif // TCPSessionHandler_INCLUDED
