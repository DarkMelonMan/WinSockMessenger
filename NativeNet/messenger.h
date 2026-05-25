#pragma once

#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>

#pragma comment(lib, "ws2_32.lib")

constexpr int DEFAULT_PORT = 54000;
constexpr int MAX_BUFFER_SIZE = 4096;