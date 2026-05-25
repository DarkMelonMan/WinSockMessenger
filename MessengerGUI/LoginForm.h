#pragma once
#include "ManagedClient.h"

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;

namespace MessengerGUI {
    public ref class LoginForm : public Form {
    public:
        property String^ UserName;
        property String^ Password;

        LoginForm() {
            Text = "Вход в мессенджер";
            Width = 300; Height = 180;
            FormBorderStyle = Windows::Forms::FormBorderStyle::FixedDialog;
            StartPosition = FormStartPosition::CenterScreen;

            // Логин
            Label^ lblLogin = gcnew Label();
            lblLogin->Text = "Логин:";
            lblLogin->Location = Point(20, 20);
            Controls->Add(lblLogin);

            _loginBox = gcnew TextBox();
            _loginBox->Location = Point(100, 18);
            _loginBox->Width = 150;
            Controls->Add(_loginBox);

            // Пароль
            Label^ lblPass = gcnew Label();
            lblPass->Text = "Пароль:";
            lblPass->Location = Point(20, 55);
            Controls->Add(lblPass);

            _passBox = gcnew TextBox();
            _passBox->Location = Point(100, 53);
            _passBox->Width = 150;
            _passBox->PasswordChar = '*';
            Controls->Add(_passBox);

            // Кнопка "Войти"
            Button^ btnLogin = gcnew Button();
            btnLogin->Text = "Войти";
            btnLogin->Location = Point(90, 95);
            btnLogin->Click += gcnew EventHandler(this, &LoginForm::OnLogin);
            Controls->Add(btnLogin);
        }

    private:
        TextBox^ _loginBox;
        TextBox^ _passBox;

        void OnLogin(Object^, EventArgs^) {
            String^ login = _loginBox->Text->Trim();
            String^ pass = _passBox->Text;
            if (String::IsNullOrEmpty(login) || String::IsNullOrEmpty(pass)) {
                MessageBox::Show("Заполните все поля");
                return;
            }
            UserName = login;
            Password = pass;
            DialogResult = Windows::Forms::DialogResult::OK;
            Close();
        }
    };
}