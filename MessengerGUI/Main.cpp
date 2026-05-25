#include "ChoiceForm.h"
#include "AuthChoiceForm.h"
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

    // 1. Выбор клиент / сервер
    ChoiceForm^ choice = gcnew ChoiceForm();
    if (choice->ShowDialog() == Windows::Forms::DialogResult::Cancel)
        return 0;

    if (choice->SelectedMode == ChoiceForm::Mode::Server) {
        Application::Run(gcnew ServerForm());
    }
    else {
        // 2. Выбор авторизации / регистрации
        AuthChoiceForm^ authChoice = gcnew AuthChoiceForm();
        if (authChoice->ShowDialog() == Windows::Forms::DialogResult::Cancel)
            return 0;

        String^ userName;
        String^ password;
        bool isRegistration = (authChoice->SelectedMode == AuthChoiceForm::AuthMode::Register);
        if (isRegistration) {
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

        // 5. Список контактов
        Application::Run(gcnew ChatListForm(connect->Client));
    }
    return 0;
}