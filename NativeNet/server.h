#pragma once
#include "messenger.h"
#include <unordered_map>
#include "C:\Program Files\PostgreSQL\18\include\libpq-fe.h"

class Server {
public:
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

    bool dbConnect();
    bool dbUserExists(const std::string& username);
    bool dbCreateUser(const std::string& username, const std::string& passwordHash);
    bool dbVerifyPassword(const std::string& username, const std::string& passwordHash);
    std::string hashPassword(const std::string& password);

    int m_port;
    SOCKET m_listeningSocket;
    std::unordered_map<SOCKET, std::string> m_clients;
    std::mutex m_clientsMutex;
    std::atomic<bool> m_running;

    LogCallback m_onLog;
    ClientListCallback m_onClientList;
    void* m_context;

    PGconn* m_dbConn = nullptr;
};