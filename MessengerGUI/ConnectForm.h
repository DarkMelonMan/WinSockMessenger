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
            Controls->Add(lblIp);

            _ipBox = gcnew TextBox();
            _ipBox->Text = "127.0.0.1";
            _ipBox->Location = Point(110, 18);
            _ipBox->Width = 140;
            Controls->Add(_ipBox);

            Label^ lblPort = gcnew Label();
            lblPort->Text = "Порт:";
            lblPort->Location = Point(20, 55);
            lblPort->Width = 80;
            Controls->Add(lblPort);

            _portBox = gcnew TextBox();
            _portBox->Text = "54000";
            _portBox->Location = Point(110, 53);
            _portBox->Width = 60;
            Controls->Add(_portBox);

            Button^ btnOk = gcnew Button();
            btnOk->Text = "Далее";
            btnOk->Location = Point(100, 100);
            btnOk->Click += gcnew EventHandler(this, &ConnectForm::OnOk);
            Controls->Add(btnOk);
        }

    private:
        TextBox^ _ipBox;
        TextBox^ _portBox;

        void OnOk(Object^, EventArgs^) {
            String^ ip = _ipBox->Text->Trim();
            int port;
            if (!Int32::TryParse(_portBox->Text->Trim(), port) || port < 1 || port > 65535) {
                MessageBox::Show("Некорректный порт");
                return;
            }
            try {
                Client = gcnew ManagedClient(ip, port);
            }
            catch (Exception^ ex) {
                MessageBox::Show("Ошибка создания клиента: " + ex->Message);
                return;
            }
            DialogResult = Windows::Forms::DialogResult::OK;
            Close();
        }
    };
}