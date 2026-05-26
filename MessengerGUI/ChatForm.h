#pragma once
#include "ManagedClient.h"

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;

namespace MessengerGUI {
    public ref class ChatForm : public Form {
    public:
        ChatForm(ManagedClient^ client, String^ peer) {
            _client = client;
            _peer = peer;
            Text = "Чат с " + peer;
            Width = 500;
            Height = 400;
            StartPosition = FormStartPosition::CenterParent;

            _history = gcnew RichTextBox();
            _history->Location = Point(10, 10);
            _history->Size = Drawing::Size(460, 250);
            _history->ReadOnly = true;

            _inputBox = gcnew TextBox();
            _inputBox->Location = Point(10, 270);
            _inputBox->Width = 350;

            Button^ sendBtn = gcnew Button();
            sendBtn->Text = "Отправить";
            sendBtn->Location = Point(370, 270);
            sendBtn->Click += gcnew EventHandler(this, &ChatForm::SendBtn_Click);

            Controls->Add(_history);
            Controls->Add(_inputBox);
            Controls->Add(sendBtn);

            _messageHandler = gcnew Action<String^>(this, &ChatForm::OnMessageReceived);
            _client->MessageReceived += _messageHandler;
            this->FormClosing += gcnew FormClosingEventHandler(this, &ChatForm::OnClosing);
        }

    private:
        ManagedClient^ _client;
        String^ _peer;
        RichTextBox^ _history;
        TextBox^ _inputBox;
        Action<String^>^ _messageHandler;

        void SendBtn_Click(Object^, EventArgs^) {
            String^ text = _inputBox->Text->Trim();
            if (!String::IsNullOrEmpty(text)) {
                _client->SendMessage(_peer, text);
                AppendText("Я: " + text + "\n");
                _inputBox->Clear();
            }
        }

        void OnMessageReceived(String^ msg) {
            // Маршалинг в UI-поток
            if (_history->InvokeRequired) {
                _history->Invoke(gcnew Action<String^>(this, &ChatForm::OnMessageReceived), msg);
                return;
            }
            if (msg->StartsWith("PRIVMSG:")) {
                array<String^>^ parts = msg->Split(':');
                if (parts->Length >= 3 && parts[1] == _peer) {
                    AppendText(_peer + ": " + parts[2] + "\n");
                }
            }
            else if (msg->StartsWith("ERROR")) {
                AppendText("[Ошибка] " + msg + "\n");
            }
        }

        void AppendText(String^ text) {
            _history->AppendText(text);
            _history->ScrollToCaret();
        }

        void OnClosing(Object^, FormClosingEventArgs^ e) {
            _client->MessageReceived -= _messageHandler;
        }
    };
}