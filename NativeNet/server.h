#pragma once
#include "messenger.h"
#include <string>
#include <unordered_map>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <functional>

class Server {
public:
    // Типы коллбэков с void*-контекстом
    using LogCallback = void(*)(const std::string& message, void* context);
    using ClientListCallback = void(*)(const std::vector<std::string>& clients, void* context);

    Server(int port, LogCallback onLog, ClientListCallback onClientList, void* context);
    ~Server();

    void start();
    void stop();

private:
    void acceptClients();
    void handleClient(SOCKET clientSocket);
    void sendToClient(SOCKET s, const std::string& msg);
    void broadcastUserList();
    std::string readCommand(SOCKET s, std::string& buffer);
    void log(const std::string& msg);
    void notifyClientList();

    int m_port;
    SOCKET m_listeningSocket;
    std::unordered_map<SOCKET, std::string> m_clients;
    std::mutex m_clientsMutex;
    std::atomic<bool> m_running;

    LogCallback m_onLog;
    ClientListCallback m_onClientList;
    void* m_context;
};