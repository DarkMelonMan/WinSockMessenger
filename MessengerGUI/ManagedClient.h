#pragma once
#include "../NativeNet/client.h"
#include <msclr/marshal_cppstd.h>
#include <string>
#include <vector>
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
            nativeClient_ = new Client(ip, port, &NativeOnMessage, &NativeOnUserList, &NativeOnDisconnect, context);
        }

        ~ManagedClient() { delete nativeClient_; _handle.Free(); }

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
            nativeClient_->disconnect();
        }

        void SendMessage(String^ to, String^ text) {
            std::string t = msclr::interop::marshal_as<std::string>(to);
            std::string msg = msclr::interop::marshal_as<std::string>(text);
            nativeClient_->sendMessage(t, msg);
        }

        void RequestUserList() {
            nativeClient_->requestUserList();
        }

    internal:
        void RaiseMessageReceived(String^ msg) {
            MessageReceived(msg);
        }
        void RaiseUserListUpdated(array<String^>^ users) {
            UserListUpdated(users);
        }
        void RaiseDisconnected() {
            Disconnected();
        }

    private:
        Client* nativeClient_;
        GCHandle _handle;
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