#pragma once

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

        LoginForm() {
            Text = "Вход в мессенджер";
            Width = 300;
            Height = 150;
            FormBorderStyle = Windows::Forms::FormBorderStyle::FixedDialog;
            MaximizeBox = false;
            StartPosition = FormStartPosition::CenterScreen;

            Label^ label = gcnew Label();
            label->Text = "Имя пользователя:";
            label->Location = Point(20, 20);
            label->Width = 110;

            TextBox^ textBox = gcnew TextBox();
            textBox->Location = Point(140, 18);
            textBox->Width = 120;

            Button^ btnOk = gcnew Button();
            btnOk->Text = "Войти";
            btnOk->Location = Point(100, 60);
            btnOk->Click += gcnew EventHandler(this, &LoginForm::BtnOk_Click);

            Controls->Add(label);
            Controls->Add(textBox);
            Controls->Add(btnOk);
            _textBox = textBox;
        }

    private:
        TextBox^ _textBox;

        void BtnOk_Click(Object^ sender, EventArgs^ e) {
            String^ name = _textBox->Text->Trim();
            if (String::IsNullOrEmpty(name)) {
                MessageBox::Show("Введите имя!");
                return;
            }
            UserName = name;
            DialogResult = Windows::Forms::DialogResult::OK;
            Close();
        }
    };
}