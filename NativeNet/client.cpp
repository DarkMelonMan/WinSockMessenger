#include "client.h"
#include <iostream>

Client::Client(const std::string& serverIp, int port,
    MessageCallback onMsg, UserListCallback onUserList, VoidCallback onDisconnect,
    void* context)
    : m_serverIp(serverIp), m_port(port), m_socket(INVALID_SOCKET), m_running(false),
    m_onMsg(onMsg), m_onUserList(onUserList), m_onDisconnect(onDisconnect), m_context(context) {
}

Client::~Client() {
    disconnect();
}

bool Client::connect(const std::string& userName, const std::string& password) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) return false;

    m_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_socket == INVALID_SOCKET) { WSACleanup(); return false; }

    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(m_port);
    inet_pton(AF_INET, m_serverIp.c_str(), &hint.sin_addr);

    if (::connect(m_socket, (sockaddr*)&hint, sizeof(hint)) == SOCKET_ERROR) {
        closesocket(m_socket); WSACleanup(); return false;
    }

    // Отправка LOGIN:user:pass
    std::string cmd = "LOGIN:" + userName + ":" + password + "\n";
    if (send(m_socket, cmd.c_str(), static_cast<int>(cmd.size()), 0) == SOCKET_ERROR) {
        closesocket(m_socket); WSACleanup(); return false;
    }

    char buffer[MAX_BUFFER_SIZE];
    int bytes = recv(m_socket, buffer, MAX_BUFFER_SIZE, 0);
    if (bytes <= 0) { closesocket(m_socket); WSACleanup(); return false; }
    std::string response(buffer, bytes);
    if (response.find("LOGIN_OK") == std::string::npos) {
        closesocket(m_socket); WSACleanup(); return false;
    }

    m_running = true;
    m_recvThread = std::thread(&Client::receiverThread, this);
    return true;
}

bool Client::registerUser(const std::string& userName, const std::string& password) {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) return false;

    m_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_socket == INVALID_SOCKET) { WSACleanup(); return false; }

    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(m_port);
    inet_pton(AF_INET, m_serverIp.c_str(), &hint.sin_addr);

    if (::connect(m_socket, (sockaddr*)&hint, sizeof(hint)) == SOCKET_ERROR) {
        closesocket(m_socket); WSACleanup(); return false;
    }

    std::string cmd = "REGISTER:" + userName + ":" + password + "\n";
    if (send(m_socket, cmd.c_str(), static_cast<int>(cmd.size()), 0) == SOCKET_ERROR) {
        closesocket(m_socket); WSACleanup(); return false;
    }

    char buffer[MAX_BUFFER_SIZE];
    int bytes = recv(m_socket, buffer, MAX_BUFFER_SIZE, 0);
    if (bytes <= 0) { closesocket(m_socket); WSACleanup(); return false; }
    std::string response(buffer, bytes);
    if (response.find("REGISTER_OK") == std::string::npos) {
        closesocket(m_socket); WSACleanup(); return false;
    }
    std::string resp = response;
    while (resp.find("LOGIN_OK") == std::string::npos) {
        bytes = recv(m_socket, buffer, MAX_BUFFER_SIZE, 0);
        if (bytes <= 0) { closesocket(m_socket); WSACleanup(); return false; }
        resp += std::string(buffer, bytes);
    }

    m_running = true;
    m_recvThread = std::thread(&Client::receiverThread, this);
    return true;
}

void Client::disconnect() {
    if (!m_running) return;
    m_running = false;
    send(m_socket, "LOGOUT\n", 7, 0);
    closesocket(m_socket);
    if (m_recvThread.joinable())
        m_recvThread.join();
    WSACleanup();
}

void Client::sendMessage(const std::string& to, const std::string& text) {
    std::string cmd = "MESSAGE:" + to + ":" + text + "\n";
    std::lock_guard<std::mutex> lock(m_sendMutex);
    send(m_socket, cmd.c_str(), static_cast<int>(cmd.size()), 0);
}

void Client::requestUserList() {
    std::string cmd = "USERLIST\n";
    std::lock_guard<std::mutex> lock(m_sendMutex);
    send(m_socket, cmd.c_str(), static_cast<int>(cmd.size()), 0);
}

void Client::receiverThread() {
    char buffer[MAX_BUFFER_SIZE];
    std::string buf;
    while (m_running) {
        int bytes = recv(m_socket, buffer, MAX_BUFFER_SIZE, 0);
        if (bytes <= 0) break;
        buf.append(buffer, bytes);
        size_t pos;
        while ((pos = buf.find('\n')) != std::string::npos) {
            std::string cmd = buf.substr(0, pos);
            buf.erase(0, pos + 1);
            processCommand(cmd);
        }
    }
    m_running = false;
    if (m_onDisconnect) m_onDisconnect(m_context);
}

void Client::processCommand(const std::string& cmd) {
    if (cmd.substr(0, 15) == "USERLIST_UPDATE") {
        std::string listStr = cmd.substr(16);
        std::vector<std::string> users;
        size_t start = 0, end;
        while ((end = listStr.find(';', start)) != std::string::npos) {
            users.push_back(listStr.substr(start, end - start));
            start = end + 1;
        }
        if (m_onUserList) m_onUserList(users, m_context);
    }
    else if (cmd.substr(0, 8) == "PRIVMSG:") {
        if (m_onMsg) m_onMsg(cmd, m_context);
    }
    else if (cmd.substr(0, 5) == "ERROR") {
        if (m_onMsg) m_onMsg(cmd, m_context);
    }
}