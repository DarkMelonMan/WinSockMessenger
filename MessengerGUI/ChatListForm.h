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

            _client->UserListUpdated += gcnew Action<array<String^>^>(this, &ChatListForm::OnUserListUpdated);
            _client->RequestUserList();

            this->FormClosing += gcnew FormClosingEventHandler(this, &ChatListForm::OnClosing);
        }

    private:
        ManagedClient^ _client;
        ListBox^ _listBox;

        void OnUserListUpdated(array<String^>^ users) {
            _listBox->Items->Clear();
            for each (String ^ user in users) {
                if (user != _client->UserName) {
                    _listBox->Items->Add(user);
                }
            }
        }

        void ListBox_DoubleClick(Object^, EventArgs^) {
            if (_listBox->SelectedItem != nullptr) {
                String^ peer = safe_cast<String^>(_listBox->SelectedItem);
                ChatForm^ chat = gcnew ChatForm(_client, peer);
                chat->ShowDialog(); // модально, после закрытия возвращаемся к списку
            }
        }

        void OnClosing(Object^, FormClosingEventArgs^ e) {
            _client->Disconnect();
        }
    };
}