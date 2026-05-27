#pragma once
#include "../NativeNet/server.h"
#include "EncodingFuncs.h"
#include <msclr/marshal_cppstd.h>
#include <gcroot.h>
#include <vector>
#include <string>

using namespace System;
using namespace System::Runtime::InteropServices;

// Свободные нативные функции для обратного вызова
static void NativeServerLog(const std::string& msg, void* context);

static void NativeServerClientList(const std::vector<std::string>& clients, void* context);

namespace MessengerGUI {

    public ref class ManagedServer {
    public:
        event Action<String^>^ LogMessage;
        event Action<array<String^>^>^ ClientListUpdated;

        ManagedServer(int port) {
            _handle = GCHandle::Alloc(this);
            void* context = GCHandle::ToIntPtr(_handle).ToPointer();
            nativeServer_ = new Server(port, &NativeServerLog, &NativeServerClientList, context);
        }

        ~ManagedServer() {
            Stop();
            _handle.Free();
        }

        void Start() {
            if (_running) return;
            _running = true;
            System::Threading::Thread^ t = gcnew System::Threading::Thread(
                gcnew System::Threading::ThreadStart(this, &ManagedServer::Run));
            t->IsBackground = true;
            t->Start();
        }

        void Stop() {
            if (!_running) return;
            _running = false;
            nativeServer_->stop();
            delete nativeServer_;
            nativeServer_ = nullptr;
        }

    internal:
        void RaiseLog(String^ msg) {
            try { LogMessage(msg); }
            catch (...) {}
        }
        void RaiseClientListUpdated(array<String^>^ clients) {
            try { ClientListUpdated(clients); }
            catch (...) {}
        }

    private:
        Server* nativeServer_;
        GCHandle _handle;
        bool _running = false;

        void Run() {
            nativeServer_->start();
        }
    };
}
static void NativeServerLog(const std::string& msg, void* context) {
    GCHandle handle = GCHandle::FromIntPtr(IntPtr(context));
    MessengerGUI::ManagedServer^ self = safe_cast<MessengerGUI::ManagedServer^>(handle.Target);
    self->RaiseLog(Utf8ToString(msg));
}

static void NativeServerClientList(const std::vector<std::string>& clients, void* context) {
    GCHandle handle = GCHandle::FromIntPtr(IntPtr(context));
    MessengerGUI::ManagedServer^ self = safe_cast<MessengerGUI::ManagedServer^>(handle.Target);
    array<String^>^ arr = gcnew array<String^>(static_cast<int>(clients.size()));
    for (size_t i = 0; i < clients.size(); ++i)
        arr[i] = Utf8ToString(clients[i]);
    self->RaiseClientListUpdated(arr);
}