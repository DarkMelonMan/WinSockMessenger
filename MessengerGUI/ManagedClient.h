#pragma once
#include "../NativeNet/client.h"
#include <msclr/marshal_cppstd.h>
#include <gcroot.h>

using namespace System;
using namespace System::Runtime::InteropServices;

static void NativeOnMessage(const std::string& msg, void* context);

static void NativeOnUserList(const std::vector<std::string>& users, void* context);

static void NativeOnDisconnect(void* context);

namespace MessengerGUI {

    public ref class ManagedClient {
    public:
        property String^ UserName;

        event Action<String^>^ MessageReceived;
        event Action<array<String^>^>^ UserListUpdated;
        event Action^ Disconnected;

        ManagedClient(String^ serverIp, int port) {
            std::string ip = msclr::interop::marshal_as<std::string>(serverIp);
            _handle = GCHandle::Alloc(this);
            void* context = GCHandle::ToIntPtr(_handle).ToPointer();
            nativeClient_ = new Client(ip, port,
                &NativeOnMessage, &NativeOnUserList, &NativeOnDisconnect, context);
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
            std::string u = msclr::interop::marshal_as<std::string>(userName);
            std::string p = msclr::interop::marshal_as<std::string>(password);
            bool ok = nativeClient_->connect(u, p);
            if (ok) UserName = userName;
            return ok;
        }

        bool Register(String^ userName, String^ password) {
            std::string u = msclr::interop::marshal_as<std::string>(userName);
            std::string p = msclr::interop::marshal_as<std::string>(password);
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
            std::string t = msclr::interop::marshal_as<std::string>(to);
            std::string msg = msclr::interop::marshal_as<std::string>(text);
            nativeClient_->sendMessage(t, msg);
        }

        void RequestUserList() {
            if (_disposed) return;
            nativeClient_->requestUserList();
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
    self->RaiseMessageReceived(msclr::interop::marshal_as<String^>(msg));
}

static void NativeOnUserList(const std::vector<std::string>& users, void* context) {
    GCHandle handle = GCHandle::FromIntPtr(IntPtr(context));
    MessengerGUI::ManagedClient^ self = safe_cast<MessengerGUI::ManagedClient^>(handle.Target);
    array<String^>^ arr = gcnew array<String^>(static_cast<int>(users.size()));
    for (size_t i = 0; i < users.size(); ++i)
        arr[i] = msclr::interop::marshal_as<String^>(users[i]);
    self->RaiseUserListUpdated(arr);
}

static void NativeOnDisconnect(void* context) {
    GCHandle handle = GCHandle::FromIntPtr(IntPtr(context));
    MessengerGUI::ManagedClient^ self = safe_cast<MessengerGUI::ManagedClient^>(handle.Target);
    self->RaiseDisconnected();
}