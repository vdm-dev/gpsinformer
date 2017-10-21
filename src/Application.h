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


#ifndef Application_INCLUDED
#define Application_INCLUDED


#include "TcpClient.h"
#include "TcpClientHandler.h"
#include "TcpServer.h"
#include "TcpSession.h"
#include "TcpSessionHandler.h"
#include "TelegramBot.h"
#include "TelegramBotHandler.h"
#include "Device.h"
#include "GpsMessage.h"
#include "UserSettings.h"


struct sqlite3;
class User;


class Application 
    : public boost::noncopyable
    , public TcpClientHandler<TcpClient>
    , public TcpSessionHandler
    , public TelegramBotHandler
{
    friend class Device;

public:
    Application();
    ~Application();

    static Application* instance();

    int main(int argc, char* argv[]);

    sqlite3* database();

private:
    bool parseCommandLine(int argc, char* argv[]);
    bool setApplicationPath(int argc, char* argv[]);
    bool loadConfiguration();
    bool saveConfiguration();

    void configureLog();

    void handleDeviceCommand(const std::vector<std::string>& arguments);

    // ApplicationReceiver
    void startReceiver();
    void handleTcpServerError(const system::error_code& error);
    void handleTcpSessionConnect(shared_ptr<TcpSession> session);
    void handleTcpSessionDisconnect(shared_ptr<TcpSession> session, TcpSessionHandler::Reason reason);
    void handleTcpSessionReceivedData(shared_ptr<TcpSession> session, const std::string& data);
    void handleReceiverTimer(const system::error_code& error);

    // ApplicationTransmitter
    void startTransmitter();
    void handleTcpClientConnect(TcpClient* client);
    void handleTcpClientDisconnect(TcpClient* client, TcpClientHandler::Reason reason);
    void handleTcpClientError(TcpClient* client, const system::error_code& error);
    void handleTcpClientReceivedData(TcpClient* client, const std::string& data);
    void handleTransmitterTimer(const system::error_code& error);

    // ApplicationTelegram
    void startTelegram();
    void sendGpsStatus(const GpsMessage& gpsMessage, int32_t userId);
    void handleInlineQuery(const TgBot::InlineQuery::Ptr& inlineQuery);
    void handleChosenInlineResult(const TgBot::ChosenInlineResult::Ptr& chosenInlineResult);
    void handleCallbackQuery(const TgBot::CallbackQuery::Ptr& callbackQuery);
    void handleMessage(const TgBot::Message::Ptr& message);
    void handleOwnMessage(const TgBot::Message::Ptr& message);
    void handleGetMe(const TgBot::User::Ptr& user);

    // ApplicationDatabase
    void openDatabase();
    void closeDatabase();
    void dbRestoreSettings();
    bool dbSaveSettings();
    bool dbAddGpsData(const GpsMessage& data);
    bool dbCreateUser(const User& user);
    bool dbGetGpsData(std::vector<GpsMessage>& data, unsigned int limit = 0, bool validOnly = false);
    bool dbSearchUser(User& user);
    bool dbUpdateUser(const User& user);

    // ApplicationCommands
    void fillCommandList();
    void handleChatCommand(const std::vector<std::string>& command, const TgBot::Message::Ptr& originalMessage);
    void handleStartCommand(const std::vector<std::string>& command, User& user, const TgBot::Message::Ptr& originalMessage);
    void handleHelpCommand(const std::vector<std::string>& command, User& user, const TgBot::Message::Ptr& originalMessage);
    void handlePasswordCommand(const std::vector<std::string>& command, User& user, const TgBot::Message::Ptr& originalMessage);
    void handleGetCommand(const std::vector<std::string>& command, User& user, const TgBot::Message::Ptr& originalMessage);
    void handleSetCommand(const std::vector<std::string>& command, User& user, const TgBot::Message::Ptr& originalMessage);
    void handleLoadCommand(const std::vector<std::string>& command, User& user, const TgBot::Message::Ptr& originalMessage);
    void handleSaveCommand(const std::vector<std::string>& command, User& user, const TgBot::Message::Ptr& originalMessage);
    void handleWhereCommand(const std::vector<std::string>& command, User& user, const TgBot::Message::Ptr& originalMessage);
    void handleStatusCommand(const std::vector<std::string>& command, User& user, const TgBot::Message::Ptr& originalMessage);
    void handleArmCommand(const std::vector<std::string>& command, User& user, const TgBot::Message::Ptr& originalMessage);
    void handleDisarmCommand(const std::vector<std::string>& command, User& user, const TgBot::Message::Ptr& originalMessage);

    // ApplicationEvents
    void handleTrackerEvent(const GpsMessage& data);

    static Application* _instance;

    filesystem::path _applicationPath;
    filesystem::path _configurationFile;

    property_tree::ptree _settings;

    asio::io_service _ioService;

    asio::deadline_timer _receiverTimer;
    asio::deadline_timer _transmitterTimer;

    TcpServer _receiver;
    TcpClient _transmitter;

    TelegramBot _telegram;

    GpsMessage _lastMessage;

    std::map<shared_ptr<TcpSession>, shared_ptr<Device>> _devices;
    std::vector<UserSettings> _userSettings;

    std::string _transmitterBuffer;

    sqlite3* _database;

    bool _transmitterAuthorized;
};


inline sqlite3* Application::database()
{
    return _database;
}


#endif // Application_INCLUDED
