#include "ChoiceForm.h"
#include "LoginForm.h"
#include "RegisterForm.h"
#include "ConnectForm.h"
#include "ChatListForm.h"
#include "ServerForm.h"

using namespace System;
using namespace System::Windows::Forms;
using namespace MessengerGUI;

[STAThread]
int main(array<String^>^ args) {
    Application::EnableVisualStyles();
    Application::SetCompatibleTextRenderingDefault(false);

    // 1. Выбор: сервер или клиент
    ChoiceForm^ choice = gcnew ChoiceForm();
    if (choice->ShowDialog() == Windows::Forms::DialogResult::Cancel)
        return 0;

    if (choice->SelectedMode == ChoiceForm::Mode::Server) {
        Application::Run(gcnew ServerForm());
    }
    else {
        // 2. Диалог авторизации (Вход / Регистрация)
        Form^ authChoice = gcnew Form();
        authChoice->Text = "Авторизация";
        authChoice->Width = 250;
        authChoice->Height = 130;
        authChoice->FormBorderStyle = FormBorderStyle::FixedDialog;
        authChoice->StartPosition = FormStartPosition::CenterScreen;

        Button^ btnLogin = gcnew Button();
        btnLogin->Text = "Вход";
        btnLogin->Location = Point(30, 30);
        btnLogin->Width = 80;
        btnLogin->DialogResult = Windows::Forms::DialogResult::OK;  // Закроет форму с OK
        authChoice->Controls->Add(btnLogin);

        Button^ btnRegister = gcnew Button();
        btnRegister->Text = "Регистрация";
        btnRegister->Location = Point(130, 30);
        btnRegister->Width = 80;
        btnRegister->DialogResult = Windows::Forms::DialogResult::Yes;  // Закроет форму с Yes
        authChoice->Controls->Add(btnRegister);

        // Отображение диалога и получение результата
        Windows::Forms::DialogResult authResult = authChoice->ShowDialog();
        if (authResult == Windows::Forms::DialogResult::Cancel)
            return 0;

        String^ userName;
        String^ password;
        bool isRegistration = false;

        // 3. Обработка выбора: Вход или Регистрация
        if (authResult == Windows::Forms::DialogResult::Yes) {
            RegisterForm^ regForm = gcnew RegisterForm();
            if (regForm->ShowDialog() != Windows::Forms::DialogResult::OK)
                return 0;
            userName = regForm->UserName;
            password = regForm->Password;
            isRegistration = true;
        }
        else {
            LoginForm^ loginForm = gcnew LoginForm();
            if (loginForm->ShowDialog() != Windows::Forms::DialogResult::OK)
                return 0;
            userName = loginForm->UserName;
            password = loginForm->Password;
        }

        // 4. Подключение к серверу
        ConnectForm^ connect = gcnew ConnectForm();
        connect->UserName = userName;
        connect->Password = password;
        connect->IsRegistration = isRegistration;
        if (connect->ShowDialog() != Windows::Forms::DialogResult::OK)
            return 0;

        // 5. Главное окно — список контактов
        Application::Run(gcnew ChatListForm(connect->Client));
    }
    return 0;
}