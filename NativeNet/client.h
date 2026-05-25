#pragma once
#include "messenger.h"
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>

typedef void (*MessageCallback)(const std::string& msg, void* context);
typedef void (*UserListCallback)(const std::vector<std::string>& users, void* context);
typedef void (*VoidCallback)(void* context);

class Client {
public:
    Client(const std::string& serverIp, int port,
        MessageCallback onMsg, UserListCallback onUserList, VoidCallback onDisconnect,
        void* context);
    ~Client();

    bool connect(const std::string& userName);
    void disconnect();
    void sendMessage(const std::string& to, const std::string& text);
    void requestUserList();

private:
    void receiverThread();
    void processCommand(const std::string& cmd);

    std::string m_serverIp;
    int m_port;
    SOCKET m_socket;
    std::atomic<bool> m_running;
    std::thread m_recvThread;

    MessageCallback m_onMsg;
    UserListCallback m_onUserList;
    VoidCallback m_onDisconnect;
    void* m_context;

    std::mutex m_sendMutex;
};