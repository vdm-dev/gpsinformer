#ifndef Device_INCLUDED
#define Device_INCLUDED

#include "TCPSession.h"

class Device
{
public:
    Device(shared_ptr<TCPSession> session);
    ~Device();

    static shared_ptr<TCPSession> authorizedSession();

    void processClientData(const std::string& data);
    void processData(const std::string& data);

private:
    bool commandLogon(const std::vector<std::string>& arguments);
    bool commandHeartbeat(const std::vector<std::string>& arguments);

    static shared_ptr<TCPSession> _authorizedSession;

    shared_ptr<TCPSession> _session;

    std::string _buffer;
    std::string _imei;
};

#endif // Device_INCLUDED
