#pragma once
#include "ManagedServer.h"

using namespace System;
using namespace System::Windows::Forms;

namespace MessengerGUI {

    public ref class ServerForm : public Form {
    public:
        ServerForm() {
            Text = "Сервер мессенджера";
            Width = 500;
            Height = 400;
            StartPosition = FormStartPosition::CenterScreen;

            // Порт
            Label^ lblPort = gcnew Label();
            lblPort->Text = "Порт:";
            lblPort->Location = Point(10, 15);
            lblPort->Width = 40;
            Controls->Add(lblPort);

            _portBox = gcnew TextBox();
            _portBox->Text = "54000";
            _portBox->Location = Point(55, 12);
            _portBox->Width = 60;
            Controls->Add(_portBox);

            // Кнопки
            _startBtn = gcnew Button();
            _startBtn->Text = "Запустить";
            _startBtn->Location = Point(130, 10);
            _startBtn->Click += gcnew EventHandler(this, &ServerForm::StartServer);
            Controls->Add(_startBtn);

            _stopBtn = gcnew Button();
            _stopBtn->Text = "Остановить";
            _stopBtn->Location = Point(220, 10);
            _stopBtn->Enabled = false;
            _stopBtn->Click += gcnew EventHandler(this, &ServerForm::StopServer);
            Controls->Add(_stopBtn);

            // Список клиентов
            Label^ lblClients = gcnew Label();
            lblClients->Text = "Подключённые клиенты:";
            lblClients->Location = Point(10, 50);
            lblClients->Width = 150;
            Controls->Add(lblClients);

            _clientListBox = gcnew ListBox();
            _clientListBox->Location = Point(10, 70);
            _clientListBox->Size = Drawing::Size(200, 150);
            Controls->Add(_clientListBox);

            // Лог
            Label^ lblLog = gcnew Label();
            lblLog->Text = "Журнал событий:";
            lblLog->Location = Point(10, 230);
            lblLog->Width = 150;
            Controls->Add(lblLog);

            _logBox = gcnew RichTextBox();
            _logBox->Location = Point(10, 250);
            _logBox->Size = Drawing::Size(460, 100);
            _logBox->ReadOnly = true;
            Controls->Add(_logBox);
        }

    private:
        TextBox^ _portBox;
        Button^ _startBtn;
        Button^ _stopBtn;
        ListBox^ _clientListBox;
        RichTextBox^ _logBox;
        ManagedServer^ _server;

        void StartServer(Object^, EventArgs^) {
            int port;
            if (!Int32::TryParse(_portBox->Text->Trim(), port) || port < 1 || port > 65535) {
                MessageBox::Show("Некорректный порт");
                return;
            }

            _server = gcnew ManagedServer(port);
            _server->LogMessage += gcnew Action<String^>(this, &ServerForm::OnLogMessage);
            _server->ClientListUpdated += gcnew Action<array<String^>^>(this, &ServerForm::OnClientListUpdated);
            _server->Start();

            _startBtn->Enabled = false;
            _stopBtn->Enabled = true;
            _portBox->Enabled = false;
            Log("Сервер запущен на порту " + port);
        }

        void StopServer(Object^, EventArgs^) {
            if (_server != nullptr) {
                _server->Stop();
                delete _server;
                _server = nullptr;
            }

            _startBtn->Enabled = true;
            _stopBtn->Enabled = false;
            _portBox->Enabled = true;
            _clientListBox->Items->Clear();
            Log("Сервер остановлен");
        }

        void OnLogMessage(String^ msg) {
            Log(msg);
        }

        void OnClientListUpdated(array<String^>^ clients) {
            if (_clientListBox->InvokeRequired) {
                _clientListBox->Invoke(
                    gcnew Action<array<String^>^>(this, &ServerForm::OnClientListUpdated),
                    gcnew array<Object^> { clients });
                return;
            }
            _clientListBox->Items->Clear();
            for each (String ^ client in clients)
                _clientListBox->Items->Add(client);
        }

        void Log(String^ msg) {
            if (_logBox->InvokeRequired) {
                _logBox->Invoke(
                    gcnew Action<String^>(this, &ServerForm::Log),
                    gcnew array<Object^> { msg });
                return;
            }
            _logBox->AppendText(DateTime::Now.ToString("HH:mm:ss") + " " + msg + "\n");
            _logBox->ScrollToCaret();
        }
    };
}