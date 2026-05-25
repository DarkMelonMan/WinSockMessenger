#pragma once

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;

namespace MessengerGUI {
    public ref class RegisterForm : public Form {
    public:
        property String^ UserName;
        property String^ Password;

        RegisterForm() {
            Text = "Регистрация";
            Width = 300; Height = 220;
            FormBorderStyle = Windows::Forms::FormBorderStyle::FixedDialog;
            StartPosition = FormStartPosition::CenterScreen;

            // Логин
            Label^ lblLogin = gcnew Label();
            lblLogin->Text = "Логин:";
            lblLogin->Location = Point(20, 20);
            Controls->Add(lblLogin);

            _loginBox = gcnew TextBox();
            _loginBox->Location = Point(120, 18);
            _loginBox->Width = 140;
            Controls->Add(_loginBox);

            // Пароль
            Label^ lblPass = gcnew Label();
            lblPass->Text = "Пароль:";
            lblPass->Location = Point(20, 55);
            Controls->Add(lblPass);

            _passBox = gcnew TextBox();
            _passBox->Location = Point(120, 53);
            _passBox->Width = 140;
            _passBox->PasswordChar = '*';
            Controls->Add(_passBox);

            // Подтверждение пароля
            Label^ lblConfirm = gcnew Label();
            lblConfirm->Text = "Подтвердите:";
            lblConfirm->Location = Point(20, 90);
            Controls->Add(lblConfirm);

            _confirmBox = gcnew TextBox();
            _confirmBox->Location = Point(120, 88);
            _confirmBox->Width = 140;
            _confirmBox->PasswordChar = '*';
            Controls->Add(_confirmBox);

            // Кнопка "Зарегистрироваться"
            Button^ btnRegister = gcnew Button();
            btnRegister->Text = "Зарегистрироваться";
            btnRegister->Location = Point(70, 130);
            btnRegister->Width = 150;
            btnRegister->Click += gcnew EventHandler(this, &RegisterForm::OnRegister);
            Controls->Add(btnRegister);
        }

    private:
        TextBox^ _loginBox;
        TextBox^ _passBox;
        TextBox^ _confirmBox;

        void OnRegister(Object^, EventArgs^) {
            String^ login = _loginBox->Text->Trim();
            String^ pass = _passBox->Text;
            String^ confirm = _confirmBox->Text;

            if (String::IsNullOrEmpty(login) || String::IsNullOrEmpty(pass) || String::IsNullOrEmpty(confirm)) {
                MessageBox::Show("Заполните все поля");
                return;
            }
            if (pass != confirm) {
                MessageBox::Show("Пароли не совпадают");
                return;
            }
            UserName = login;
            Password = pass;
            DialogResult = Windows::Forms::DialogResult::OK;
            Close();
        }
    };
}