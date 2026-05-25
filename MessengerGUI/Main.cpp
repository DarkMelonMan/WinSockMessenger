#include "ChoiceForm.h"
#include "LoginForm.h"
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

    if (choice->SelectedMode == ChoiceForm::Mode::Server) {
        ServerForm^ serverForm = gcnew ServerForm();
        Application::Run(serverForm);
    }
    else {
        LoginForm^ login = gcnew LoginForm();
        if (login->ShowDialog() != Windows::Forms::DialogResult::OK) return 0;
        String^ userName = login->UserName;

        ConnectForm^ connect = gcnew ConnectForm();
        connect->UserName = userName;
        if (connect->ShowDialog() != Windows::Forms::DialogResult::OK) return 0;

        ChatListForm^ mainForm = gcnew ChatListForm(connect->Client);
        Application::Run(mainForm);
    }
    return 0;
}