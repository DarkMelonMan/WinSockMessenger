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

    ChoiceForm^ choice = gcnew ChoiceForm();
    if (choice->ShowDialog() == Windows::Forms::DialogResult::Cancel)
        return 0;

    // выбран сервер
    if (choice->SelectedMode == ChoiceForm::Mode::Server) {
        Application::Run(gcnew ServerForm());
        return 0;
    }
    // выбран клиент
    ConnectForm^ connect = gcnew ConnectForm();
    if (connect->ShowDialog() != Windows::Forms::DialogResult::OK)
        return 0;

    ManagedClient^ client = connect->Client;
    while (true) {
        AuthChoiceForm^ authChoice = gcnew AuthChoiceForm();
        if (authChoice->ShowDialog() != Windows::Forms::DialogResult::OK) {
            delete client;
            return 0;
        }
        bool isRegistration = (authChoice->SelectedMode == AuthChoiceForm::AuthMode::Register);
        String^ userName;
        String^ password;

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
        bool ok;
        if (isRegistration)
            ok = client->Register(userName, password);
        else
            ok = client->Login(userName, password);
        if (ok) {
            Application::Run(gcnew ChatListForm(client));
            return 0;
        }
        else {
            MessageBox::Show(isRegistration ? "Registration error: name probably is being taken"
                : "Login error: invalid login or password");
        }
    }
    return 0;
}