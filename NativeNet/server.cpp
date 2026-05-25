#include "server.h"
#include <iostream>
#include <sstream>
#include <iomanip>

static const char* DB_CONNINFO = "host=localhost dbname=Messenger user=postgres password=qwe!123";

Server::Server(int port, LogCallback onLog, ClientListCallback onClientList, void* context)
    : m_port(port), m_listeningSocket(INVALID_SOCKET), m_running(false),
    m_onLog(onLog), m_onClientList(onClientList), m_context(context) {}

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

    if (!dbConnect()) {
        log("Database connection failed");
        WSACleanup();
        return;
    }

    m_listeningSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (m_listeningSocket == INVALID_SOCKET) {
        log("Socket creation failed");
        PQfinish(m_dbConn);
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
        PQfinish(m_dbConn);
        WSACleanup();
        return;
    }

    if (listen(m_listeningSocket, SOMAXCONN) == SOCKET_ERROR) {
        log("Listen failed");
        closesocket(m_listeningSocket);
        PQfinish(m_dbConn);
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
    PQfinish(m_dbConn);
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

    // Ожидание команды REGISTER или LOGIN
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

        if (cmd.substr(0, 9) == "REGISTER:") {
            // Формат: REGISTER:username:password
            size_t pos1 = 9;
            size_t pos2 = cmd.find(':', pos1);
            if (pos2 == std::string::npos) {
                sendToClient(clientSocket, "REGISTER_ERROR:invalid format\n");
                continue;
            }
            std::string user = cmd.substr(pos1, pos2 - pos1);
            std::string pass = cmd.substr(pos2 + 1);
            if (user.empty() || pass.empty()) {
                sendToClient(clientSocket, "REGISTER_ERROR:empty username or password\n");
                continue;
            }
            if (dbUserExists(user)) {
                sendToClient(clientSocket, "REGISTER_ERROR:username already taken\n");
                continue;
            }
            std::string hash = hashPassword(pass);
            if (!dbCreateUser(user, hash)) {
                sendToClient(clientSocket, "REGISTER_ERROR:database error\n");
                continue;
            }
            sendToClient(clientSocket, "REGISTER_OK\n");
            log("User registered: " + user);
            // После успешной регистрации автоматически входим (можно не требовать повторного LOGIN)
            name = user;
            {
                std::lock_guard<std::mutex> lock(m_clientsMutex);
                m_clients[clientSocket] = name;
            }
            sendToClient(clientSocket, "LOGIN_OK\n");
            log(name + " connected (registered)");
            notifyClientList();
            break;
        }
        else if (cmd.substr(0, 6) == "LOGIN:") {
            size_t pos1 = 6;
            size_t pos2 = cmd.find(':', pos1);
            if (pos2 == std::string::npos) {
                sendToClient(clientSocket, "LOGIN_ERROR:invalid format\n");
                continue;
            }
            std::string user = cmd.substr(pos1, pos2 - pos1);
            std::string pass = cmd.substr(pos2 + 1);
            if (user.empty() || pass.empty()) {
                sendToClient(clientSocket, "LOGIN_ERROR:empty username or password\n");
                continue;
            }
            if (!dbUserExists(user)) {
                sendToClient(clientSocket, "LOGIN_ERROR:user not found\n");
                continue;
            }
            std::string hash = hashPassword(pass);
            if (!dbVerifyPassword(user, hash)) {
                sendToClient(clientSocket, "LOGIN_ERROR:wrong password\n");
                continue;
            }
            // Проверка, не залогинен ли уже
            {
                std::lock_guard<std::mutex> lock(m_clientsMutex);
                bool already = false;
                for (auto& p : m_clients) {
                    if (p.second == user) { already = true; break; }
                }
                if (already) {
                    sendToClient(clientSocket, "LOGIN_ERROR:already logged in\n");
                    continue;
                }
                m_clients[clientSocket] = user;
                name = user;
            }
            sendToClient(clientSocket, "LOGIN_OK\n");
            log(name + " logged in");
            notifyClientList();
            break;
        }
        else {
            sendToClient(clientSocket, "ERROR:REGISTER or LOGIN required\n");
        }
    }

    // Основной цикл обмена сообщениями (как раньше)
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

// --- Вспомогательные методы сети ---
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

// --- Работа с БД ---
bool Server::dbConnect() {
    m_dbConn = PQconnectdb(DB_CONNINFO);
    if (PQstatus(m_dbConn) != CONNECTION_OK) {
        log(std::string("DB connection error: ") + PQerrorMessage(m_dbConn));
        PQfinish(m_dbConn);
        m_dbConn = nullptr;
        return false;
    }
    log("Connected to PostgreSQL");
    return true;
}

bool Server::dbUserExists(const std::string& username) {
    const char* params[1] = { username.c_str() };
    PGresult* res = PQexecParams(m_dbConn,
        "SELECT 1 FROM users WHERE username=$1",
        1, nullptr, params, nullptr, nullptr, 0);
    bool exists = (PQntuples(res) > 0);
    PQclear(res);
    return exists;
}

bool Server::dbCreateUser(const std::string& username, const std::string& passwordHash) {
    const char* params[2] = { username.c_str(), passwordHash.c_str() };
    PGresult* res = PQexecParams(m_dbConn,
        "INSERT INTO users (username, password_hash) VALUES ($1, $2)",
        2, nullptr, params, nullptr, nullptr, 0);
    bool ok = (PQresultStatus(res) == PGRES_COMMAND_OK);
    PQclear(res);
    return ok;
}

bool Server::dbVerifyPassword(const std::string& username, const std::string& passwordHash) {
    const char* params[2] = { username.c_str(), passwordHash.c_str() };
    PGresult* res = PQexecParams(m_dbConn,
        "SELECT 1 FROM users WHERE username=$1 AND password_hash=$2",
        2, nullptr, params, nullptr, nullptr, 0);
    bool ok = (PQntuples(res) > 0);
    PQclear(res);
    return ok;
}

std::string Server::hashPassword(const std::string& password) {
    BCRYPT_ALG_HANDLE hAlg = nullptr;
    BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_SHA256_ALGORITHM, nullptr, 0);
    BCryptCloseAlgorithmProvider(hAlg, 0); // упрощённо – для примера используем готовую функцию
    // Корректная реализация:
    NTSTATUS status;
    BCRYPT_ALG_HANDLE hHashAlg;
    status = BCryptOpenAlgorithmProvider(&hHashAlg, BCRYPT_SHA256_ALGORITHM, nullptr, 0);
    if (!BCRYPT_SUCCESS(status)) return "";

    DWORD cbHashObject, cbData;
    status = BCryptGetProperty(hHashAlg, BCRYPT_OBJECT_LENGTH, (PBYTE)&cbHashObject, sizeof(DWORD), &cbData, 0);
    if (!BCRYPT_SUCCESS(status)) { BCryptCloseAlgorithmProvider(hHashAlg, 0); return ""; }

    BYTE* pbHashObject = new BYTE[cbHashObject];
    status = BCryptCreateHash(hHashAlg, &hHashAlg, pbHashObject, cbHashObject, nullptr, 0, 0);
    if (!BCRYPT_SUCCESS(status)) { delete[] pbHashObject; BCryptCloseAlgorithmProvider(hHashAlg, 0); return ""; }

    status = BCryptHashData(hHashAlg, (PBYTE)password.c_str(), (ULONG)password.size(), 0);
    BYTE hash[32];
    status = BCryptFinishHash(hHashAlg, hash, 32, 0);

    BCryptDestroyHash(hHashAlg);
    BCryptCloseAlgorithmProvider(hHashAlg, 0);
    delete[] pbHashObject;

    // Преобразуем в hex-строку
    std::stringstream ss;
    for (int i = 0; i < 32; ++i)
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    return ss.str();
}
