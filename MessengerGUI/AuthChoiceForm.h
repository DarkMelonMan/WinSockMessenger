#pragma once

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;

namespace MessengerGUI {

    public ref class AuthChoiceForm : public Form {
    public:
        enum class AuthMode { Login, Register };
        property AuthMode SelectedMode;

        AuthChoiceForm() {
            Text = "Авторизация";
            Width = 250;
            Height = 130;
            FormBorderStyle = Windows::Forms::FormBorderStyle::FixedDialog;
            MaximizeBox = false;
            StartPosition = FormStartPosition::CenterScreen;

            Button^ btnLogin = gcnew Button();
            btnLogin->Text = "Войти";
            btnLogin->Location = Point(20, 20);
            btnLogin->Width = 90;
            btnLogin->Click += gcnew EventHandler(this, &AuthChoiceForm::OnLoginClick);
            Controls->Add(btnLogin);

            Button^ btnRegister = gcnew Button();
            btnRegister->Text = "Регистрация";
            btnRegister->Location = Point(130, 20);
            btnRegister->Width = 90;
            btnRegister->Click += gcnew EventHandler(this, &AuthChoiceForm::OnRegisterClick);
            Controls->Add(btnRegister);
        }

    private:
        void OnLoginClick(Object^, EventArgs^) {
            SelectedMode = AuthMode::Login;
            DialogResult = Windows::Forms::DialogResult::OK;
            Close();
        }

        void OnRegisterClick(Object^, EventArgs^) {
            SelectedMode = AuthMode::Register;
            DialogResult = Windows::Forms::DialogResult::OK;
            Close();
        }
    };
}