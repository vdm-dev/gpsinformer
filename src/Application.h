#ifndef Application_INCLUDED
#define Application_INCLUDED

#include "TCPClient.h"
#include "TCPClientHandler.h"
#include "TCPServer.h"
#include "TCPSession.h"
#include "TCPSessionHandler.h"
#include "Device.h"

class Application 
    : public boost::noncopyable
    , public TCPClientHandler
    , public TCPSessionHandler
    , public gloox::LogHandler
    , public gloox::ConnectionListener
    , public gloox::MessageHandler
    , public gloox::SubscriptionHandler
{
    friend class Device;

public:
    Application();
    ~Application();

    static Application* instance();

    int main(int argc, char* argv[]);

private:
    bool parseCommandLine(int argc, char* argv[]);
    bool setApplicationPath(int argc, char* argv[]);
    bool loadConfiguration();
    bool saveConfiguration();

    void configureLog();

    void handleDeviceCommand(const std::vector<std::string>& arguments);

    // ApplicationJabber
    void startJabber();
    void onConnect();
    void onDisconnect(gloox::ConnectionError error);
    bool onTLSConnect(const gloox::CertInfo& info);
    void handleJabberTimer(const system::error_code& error);
    void handleLog(gloox::LogLevel level, gloox::LogArea area, const std::string& message);
    void handleMessage(const gloox::Message& message, gloox::MessageSession* session = 0);
    void handleSubscription(const gloox::Subscription& subscription);

    // ApplicationReceiver
    void startReceiver();
    void handleTCPServerError(const system::error_code& error);
    void handleTCPSessionConnect(shared_ptr<TCPSession> session);
    void handleTCPSessionDisconnect(shared_ptr<TCPSession> session, TCPSessionHandler::Reason reason);
    void handleTCPSessionReceivedData(shared_ptr<TCPSession> session, const std::string& data);
    void handleReceiverTimer(const system::error_code& error);

    // ApplicationTransmitter
    void startTransmitter();
    void handleTCPClientConnect(TCPClient* client);
    void handleTCPClientDisconnect(TCPClient* client, TCPClientHandler::Reason reason);
    void handleTCPClientError(TCPClient* client, const system::error_code& error);
    void handleTCPClientReceivedData(TCPClient* client, const std::string& data);
    void handleTransmitterTimer(const system::error_code& error);

    // ApplicationCommands
    void handleChatCommand(const std::vector<std::string>& command, const gloox::Message& message, gloox::MessageSession* session = 0);
    void handleGetCommand(const std::vector<std::string>& command, const gloox::JID& sender);
    void handleSetCommand(const std::vector<std::string>& command, const gloox::JID& sender);
    void handleLoadCommand(const std::vector<std::string>& command, const gloox::JID& sender);
    void handleSaveCommand(const std::vector<std::string>& command, const gloox::JID& sender);

    static Application* _instance;

    filesystem::path _applicationPath;
    filesystem::path _configurationFile;

    property_tree::ptree _settings;

    asio::io_service _ioService;

    asio::deadline_timer _jabberTimer;
    asio::deadline_timer _receiverTimer;
    asio::deadline_timer _transmitterTimer;

    gloox::Client _jabber;

    TCPServer _receiver;
    TCPClient _transmitter;

    std::map<shared_ptr<TCPSession>, shared_ptr<Device>> _devices;

    std::string _transmitterBuffer;

    bool _transmitterAuthorized;
};

#endif // Application_INCLUDED
