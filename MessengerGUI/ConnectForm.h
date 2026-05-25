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

        ConnectForm() {
            Text = "Подключение к серверу";
            Width = 300;
            Height = 200;
            FormBorderStyle = Windows::Forms::FormBorderStyle::FixedDialog;
            StartPosition = FormStartPosition::CenterScreen;

            Label^ lblIp = gcnew Label();
            lblIp->Text = "IP-адрес:";
            lblIp->Location = Point(20, 20);
            lblIp->Width = 80;

            _ipBox = gcnew TextBox();
            _ipBox->Text = "127.0.0.1";
            _ipBox->Location = Point(110, 18);
            _ipBox->Width = 140;

            Label^ lblPort = gcnew Label();
            lblPort->Text = "Порт:";
            lblPort->Location = Point(20, 55);
            lblPort->Width = 80;

            _portBox = gcnew TextBox();
            _portBox->Text = "54000";
            _portBox->Location = Point(110, 53);
            _portBox->Width = 60;

            Button^ btnConnect = gcnew Button();
            btnConnect->Text = "Подключиться";
            btnConnect->Location = Point(90, 100);
            btnConnect->Click += gcnew EventHandler(this, &ConnectForm::BtnConnect_Click);

            Controls->Add(lblIp);
            Controls->Add(_ipBox);
            Controls->Add(lblPort);
            Controls->Add(_portBox);
            Controls->Add(btnConnect);
        }

    private:
        TextBox^ _ipBox;
        TextBox^ _portBox;

        void BtnConnect_Click(Object^ sender, EventArgs^ e) {
            String^ ip = _ipBox->Text->Trim();
            int port;
            if (!Int32::TryParse(_portBox->Text->Trim(), port)) {
                MessageBox::Show("Некорректный порт");
                return;
            }

            this->Cursor = Cursors::WaitCursor;
            ManagedClient^ client = nullptr;
            try {
                client = gcnew ManagedClient(ip, port);
            }
            catch (Exception^ ex) {
                MessageBox::Show("Ошибка создания клиента: " + ex->Message);
                this->Cursor = Cursors::Default;
                return;
            }

            if (!client->Connect(UserName)) {
                MessageBox::Show("Не удалось подключиться или имя занято.");
                delete client;
                this->Cursor = Cursors::Default;
                return;
            }

            Client = client;
            this->Cursor = Cursors::Default;
            DialogResult = Windows::Forms::DialogResult::OK;
            Close();
        }
    };
}