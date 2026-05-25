#pragma once
#include "ManagedClient.h"

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;

namespace MessengerGUI {
    public ref class ConnectForm : public Form {
    public:
        property ManagedClient^ Client;
        property String^ UserName;
        property String^ Password;
        property bool IsRegistration; // флаг, регистрация или вход

        ConnectForm() {
            Text = "Подключение к серверу";
            Width = 300; Height = 200;
            StartPosition = FormStartPosition::CenterScreen;

            Label^ lblIp = gcnew Label(); lblIp->Text = "IP:"; lblIp->Location = Point(20, 20);
            _ipBox = gcnew TextBox(); _ipBox->Text = "127.0.0.1"; _ipBox->Location = Point(80, 18); _ipBox->Width = 150;
            Label^ lblPort = gcnew Label(); lblPort->Text = "Порт:"; lblPort->Location = Point(20, 55);
            _portBox = gcnew TextBox(); _portBox->Text = "54000"; _portBox->Location = Point(80, 53); _portBox->Width = 60;
            Button^ btnConnect = gcnew Button(); btnConnect->Text = "Подключиться"; btnConnect->Location = Point(90, 100);
            btnConnect->Click += gcnew EventHandler(this, &ConnectForm::OnConnect);

            Controls->Add(lblIp); Controls->Add(_ipBox);
            Controls->Add(lblPort); Controls->Add(_portBox);
            Controls->Add(btnConnect);
        }

    private:
        TextBox^ _ipBox;
        TextBox^ _portBox;

        void OnConnect(Object^, EventArgs^) {
            String^ ip = _ipBox->Text->Trim();
            int port;
            if (!Int32::TryParse(_portBox->Text->Trim(), port)) {
                MessageBox::Show("Некорректный порт"); return;
            }
            Cursor = Cursors::WaitCursor;
            ManagedClient^ client = gcnew ManagedClient(ip, port);
            bool ok;
            if (IsRegistration)
                ok = client->Register(UserName, Password);
            else
                ok = client->Login(UserName, Password);

            Cursor = Cursors::Default;
            if (!ok) {
                String^ err = IsRegistration ? "Ошибка регистрации. Возможно, имя занято." : "Ошибка входа. Проверьте логин/пароль.";
                MessageBox::Show(err);
                delete client;
                return;
            }
            Client = client;
            DialogResult = Windows::Forms::DialogResult::OK;
            Close();
        }
    };
}