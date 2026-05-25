#include "server.h"
#include <iostream>
#include <sstream>

Server::Server(int port, LogCallback onLog, ClientListCallback onClientList, void* context)
    : m_port(port), m_listeningSocket(INVALID_SOCKET), m_running(false),
    m_onLog(onLog), m_onClientList(onClientList), m_context(context) {
}

Server::~Server() {
    stop();
}

void Server::log(const std::string& msg) {
    if (m_onLog) m_onLog(msg, m_context);
}

void Server::notifyClientList() {
    if (!m_onClientList) return;
    std::vector<std::string> users;
    {
        std::lock_guard<std::mutex> lock(m_clientsMutex);
        for (auto& p : m_clients)
            users.push_back(p.second);
    }
    m_onClientList(users, m_context);
}

void Server::start() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        log("WSAStartup failed");
        return;
    }

    m_listeningSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_listeningSocket == INVALID_SOCKET) {
        log("Socket creation failed");
        WSACleanup();
        return;
    }

    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(m_port);
    hint.sin_addr.S_un.S_addr = INADDR_ANY;

    if (bind(m_listeningSocket, (sockaddr*)&hint, sizeof(hint)) == SOCKET_ERROR) {
        log("Bind failed");
        closesocket(m_listeningSocket);
        WSACleanup();
        return;
    }

    if (listen(m_listeningSocket, SOMAXCONN) == SOCKET_ERROR) {
        log("Listen failed");
        closesocket(m_listeningSocket);
        WSACleanup();
        return;
    }

    std::ostringstream oss;
    oss << "Server listening on port " << m_port;
    log(oss.str());

    m_running = true;
    acceptClients();
}

void Server::stop() {
    if (!m_running) return;
    m_running = false;
    closesocket(m_listeningSocket);
    {
        std::lock_guard<std::mutex> lock(m_clientsMutex);
        for (auto& pair : m_clients)
            closesocket(pair.first);
        m_clients.clear();
    }
    WSACleanup();
    log("Server stopped");
}

void Server::acceptClients() {
    while (m_running) {
        sockaddr_in clientAddr;
        int size = sizeof(clientAddr);
        SOCKET clientSocket = accept(m_listeningSocket, (sockaddr*)&clientAddr, &size);
        if (clientSocket == INVALID_SOCKET) {
            if (m_running) log("Accept failed");
            continue;
        }
        std::thread(&Server::handleClient, this, clientSocket).detach();
    }
}

void Server::handleClient(SOCKET clientSocket) {
    std::string buffer;
    std::string name;

    // Ожидание команды LOGIN
    while (m_running) {
        std::string cmd = readCommand(clientSocket, buffer);
        if (cmd.empty()) {
            std::lock_guard<std::mutex> lock(m_clientsMutex);
            auto it = m_clients.find(clientSocket);
            if (it != m_clients.end()) {
                log(it->second + " disconnected");
                m_clients.erase(it);
            }
            break;
        }
        if (cmd.substr(0, 6) == "LOGIN:") {
            name = cmd.substr(6);
            if (name.empty()) {
                sendToClient(clientSocket, "ERROR:empty name\n");
                continue;
            }
            {
                std::lock_guard<std::mutex> lock(m_clientsMutex);
                bool unique = true;
                for (auto& p : m_clients) {
                    if (p.second == name) { unique = false; break; }
                }
                if (!unique) {
                    sendToClient(clientSocket, "ERROR:name taken\n");
                    continue;
                }
                m_clients[clientSocket] = name;
            }
            sendToClient(clientSocket, "LOGIN_OK\n");
            log(name + " connected");
            notifyClientList();
            break;
        }
        else {
            sendToClient(clientSocket, "ERROR:LOGIN required\n");
        }
    }

    // Основной цикл
    std::string buf;
    while (m_running) {
        std::string cmd = readCommand(clientSocket, buf);
        if (cmd.empty()) break;

        if (cmd.substr(0, 8) == "MESSAGE:") {
            size_t pos1 = 8;
            size_t pos2 = cmd.find(':', pos1);
            if (pos2 == std::string::npos) continue;
            std::string target = cmd.substr(pos1, pos2 - pos1);
            std::string text = cmd.substr(pos2 + 1);

            std::lock_guard<std::mutex> lock(m_clientsMutex);
            auto it = m_clients.find(clientSocket);
            if (it == m_clients.end()) break;
            std::string from = it->second;

            for (auto& p : m_clients) {
                if (p.second == target) {
                    std::string fullMsg = "PRIVMSG:" + from + ":" + text + "\n";
                    sendToClient(p.first, fullMsg);
                    break;
                }
            }
        }
        else if (cmd == "USERLIST") {
            broadcastUserList();
        }
        else if (cmd == "LOGOUT") {
            break;
        }
    }
    
    closesocket(clientSocket);
    notifyClientList();
}

void Server::sendToClient(SOCKET s, const std::string& msg) {
    send(s, msg.c_str(), static_cast<int>(msg.size()), 0);
}

void Server::broadcastUserList() {
    std::string list = "USERLIST_UPDATE:";
    {
        std::lock_guard<std::mutex> lock(m_clientsMutex);
        for (auto& p : m_clients)
            list += p.second + ";";
    }
    list += "\n";
    std::lock_guard<std::mutex> lock(m_clientsMutex);
    for (auto& p : m_clients)
        sendToClient(p.first, list);
}

std::string Server::readCommand(SOCKET s, std::string& buffer) {
    char tmp[MAX_BUFFER_SIZE];
    while (true) {
        size_t pos = buffer.find('\n');
        if (pos != std::string::npos) {
            std::string cmd = buffer.substr(0, pos);
            buffer.erase(0, pos + 1);
            return cmd;
        }
        int bytes = recv(s, tmp, MAX_BUFFER_SIZE, 0);
        if (bytes <= 0) return "";
        buffer.append(tmp, bytes);
    }
}