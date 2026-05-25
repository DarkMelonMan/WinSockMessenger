#pragma once


using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;

namespace MessengerGUI {

    public ref class ChoiceForm : public Form {
    public:
        enum class Mode { Client, Server };
        property Mode SelectedMode;

        ChoiceForm() {
            Text = "Мессенджер";
            Width = 280;
            Height = 150;
            FormBorderStyle = Windows::Forms::FormBorderStyle::FixedDialog;
            MaximizeBox = false;
            StartPosition = FormStartPosition::CenterScreen;

            Button^ clientBtn = gcnew Button();
            clientBtn->Text = "Запустить клиент";
            clientBtn->Location = Point(70, 15);
            clientBtn->Width = 120;
            clientBtn->Click += gcnew EventHandler(this, &ChoiceForm::OnClientClick);
            Controls->Add(clientBtn);

            Button^ serverBtn = gcnew Button();
            serverBtn->Text = "Запустить сервер";
            serverBtn->Location = Point(70, 55);
            serverBtn->Width = 120;
            serverBtn->Click += gcnew EventHandler(this, &ChoiceForm::OnServerClick);
            Controls->Add(serverBtn);
        }

    private:
        void OnClientClick(Object^ sender, EventArgs^ e) {
            SelectedMode = Mode::Client;
            DialogResult = Windows::Forms::DialogResult::OK;
            Close();
        }

        void OnServerClick(Object^ sender, EventArgs^ e) {
            SelectedMode = Mode::Server;
            DialogResult = Windows::Forms::DialogResult::Yes;  // или OK, но различаем
            Close();
        }
    };
}