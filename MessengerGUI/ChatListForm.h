#pragma once
#include "ManagedClient.h"
#include "ChatForm.h"

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;

namespace MessengerGUI {
    public ref class ChatListForm : public Form {
    public:
        ChatListForm(ManagedClient^ client) {
            _client = client;
            Text = "Контакты — " + _client->UserName;
            Width = 320;
            Height = 400;
            StartPosition = FormStartPosition::CenterScreen;

            _listBox = gcnew ListBox();
            _listBox->Dock = DockStyle::Fill;
            Controls->Add(_listBox);

            _listBox->DoubleClick += gcnew EventHandler(this, &ChatListForm::ListBox_DoubleClick);
            _userListHandler = gcnew Action<array<String^>^>(this, &ChatListForm::OnUserListUpdated);
            _client->UserListUpdated += _userListHandler;

            this->Load += gcnew EventHandler(this, &ChatListForm::OnLoad);
            this->FormClosing += gcnew FormClosingEventHandler(this, &ChatListForm::OnClosing);
        }

    private:
        ManagedClient^ _client;
        ListBox^ _listBox;
        Action<array<String^>^>^ _userListHandler;
        array<String^>^ _pendingUsers;

        void OnLoad(Object^ sender, EventArgs^ e) {
            _client->RequestUserList();
        }

        void OnUserListUpdated(array<String^>^ users) {
            if (_listBox->InvokeRequired) {
                _pendingUsers = users;
                _listBox->Invoke(gcnew MethodInvoker(this, &ChatListForm::UpdateUserListUI));
                return;
            }
            _listBox->Items->Clear();
            for each (String ^ user in users) {
                if (user != _client->UserName)
                    _listBox->Items->Add(user);
            }
        }

        void UpdateUserListUI() {
            _listBox->Items->Clear();
            for each (String ^ user in _pendingUsers) {
                if (user != _client->UserName)
                    _listBox->Items->Add(user);
            }
        }

        void ListBox_DoubleClick(Object^, EventArgs^) {
            if (_listBox->SelectedItem != nullptr) {
                String^ peer = safe_cast<String^>(_listBox->SelectedItem);
                ChatForm^ chat = gcnew ChatForm(_client, peer);
                chat->ShowDialog();
            }
        }

        void OnClosing(Object^, FormClosingEventArgs^ e) {
            _client->UserListUpdated -= _userListHandler;
            _client->Disconnect();
        }
    };
}