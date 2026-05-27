#pragma once
#include "../NativeNet/client.h"
#include "EncodingFuncs.h"
#include <msclr/marshal_cppstd.h>
#include <gcroot.h>

using namespace System;
using namespace System::Runtime::InteropServices;

static void NativeOnMessage(const std::string& msg, void* context);

static void NativeOnUserList(const std::vector<std::string>& users, void* context);

static void NativeOnDisconnect(void* context);

static void NativeOnHistory(const std::string& peer, const std::string& history, void* context);

namespace MessengerGUI {

    public delegate void HistoryReceivedHandler(String^ peer, String^ history);

    public ref class ManagedClient {
    public:
        property String^ UserName;

        event Action<String^>^ MessageReceived;
        event Action<array<String^>^>^ UserListUpdated;
        event Action^ Disconnected;
        event HistoryReceivedHandler^ HistoryReceived;

        ManagedClient(String^ serverIp, int port) {
            std::string ip = StringToUtf8(serverIp);
            _handle = GCHandle::Alloc(this);
            void* context = GCHandle::ToIntPtr(_handle).ToPointer();
            nativeClient_ = new Client(ip, port,
                &NativeOnMessage, &NativeOnUserList, &NativeOnDisconnect,
                &NativeOnHistory, context);
        }

        ~ManagedClient() {
            if (!_disposed) {
                DisconnectInternal();
            }
            delete nativeClient_;
            nativeClient_ = nullptr;
            if (_handle.IsAllocated) {
                _handle.Free();
            }
        }

        bool Login(String^ userName, String^ password) {
            std::string u = StringToUtf8(userName);
            std::string p = StringToUtf8(password);
            bool ok = nativeClient_->connect(u, p);
            if (ok) UserName = userName;
            return ok;
        }

        bool Register(String^ userName, String^ password) {
            std::string u = StringToUtf8(userName);
            std::string p = StringToUtf8(password);
            bool ok = nativeClient_->registerUser(u, p);
            if (ok) UserName = userName;
            return ok;
        }

        void Disconnect() {
            if (!_disposed) {
                DisconnectInternal();
                if (_handle.IsAllocated) {
                    _handle.Free();
                }
            }
        }

        void SendMessage(String^ to, String^ text) {
            if (_disposed) return;
            std::string t = StringToUtf8(to);
            std::string msg = StringToUtf8(text);
            nativeClient_->sendMessage(t, msg);
        }

        void RequestUserList() {
            if (_disposed) return;
            nativeClient_->requestUserList();
        }

        void RequestHistory(String^ peer) {
            if (_disposed) return;
            std::string p = StringToUtf8(peer);
            nativeClient_->requestHistory(p);
        }

    internal:
        void RaiseMessageReceived(String^ msg) {
            if (!_disposed) MessageReceived(msg); 
        }
        void RaiseUserListUpdated(array<String^>^ users) {
            if (!_disposed) UserListUpdated(users);
        }
        void RaiseDisconnected() {
            if (!_disposed) Disconnected();
        }
        void RaiseHistoryReceived(String^ peer, String^ history) {
            if (!_disposed) HistoryReceived(peer, history);
        }

    private:
        Client* nativeClient_;
        GCHandle _handle;
        bool _disposed = false;

        void DisconnectInternal() {
            if (nativeClient_) {
                nativeClient_->disconnect();
            }
            _disposed = true;
        }
    };
}
static void NativeOnMessage(const std::string& msg, void* context) {
    GCHandle handle = GCHandle::FromIntPtr(IntPtr(context));
    MessengerGUI::ManagedClient^ self = safe_cast<MessengerGUI::ManagedClient^>(handle.Target);
    self->RaiseMessageReceived(Utf8ToString(msg));
}

static void NativeOnUserList(const std::vector<std::string>& users, void* context) {
    GCHandle handle = GCHandle::FromIntPtr(IntPtr(context));
    MessengerGUI::ManagedClient^ self = safe_cast<MessengerGUI::ManagedClient^>(handle.Target);
    array<String^>^ arr = gcnew array<String^>(static_cast<int>(users.size()));
    for (size_t i = 0; i < users.size(); ++i)
        arr[i] = Utf8ToString(users[i]);
    self->RaiseUserListUpdated(arr);
}

static void NativeOnDisconnect(void* context) {
    GCHandle handle = GCHandle::FromIntPtr(IntPtr(context));
    MessengerGUI::ManagedClient^ self = safe_cast<MessengerGUI::ManagedClient^>(handle.Target);
    self->RaiseDisconnected();
}

static void NativeOnHistory(const std::string& peer, const std::string& history, void* context) {
    GCHandle handle = GCHandle::FromIntPtr(IntPtr(context));
    MessengerGUI::ManagedClient^ self = safe_cast<MessengerGUI::ManagedClient^>(handle.Target);
    self->RaiseHistoryReceived(Utf8ToString(peer), Utf8ToString(history));
}