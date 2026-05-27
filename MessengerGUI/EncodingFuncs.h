#pragma once
#include <msclr/marshal.h>
#include <string>
using namespace System;
using namespace System::Runtime::InteropServices;

static System::String^ Utf8ToString(const std::string& utf8) {
    if (utf8.empty()) return String::Empty;
    array<Byte>^ bytes = gcnew array<Byte>(static_cast<int>(utf8.size()));
    for (size_t i = 0; i < utf8.size(); ++i)
        bytes[i] = static_cast<Byte>(utf8[i]);
    return System::Text::Encoding::UTF8->GetString(bytes);
}

static std::string StringToUtf8(System::String^ str) {
    if (String::IsNullOrEmpty(str)) return std::string();
    array<Byte>^ bytes = System::Text::Encoding::UTF8->GetBytes(str);
    pin_ptr<Byte> ptr = &bytes[0];
    return std::string(reinterpret_cast<const char*>(ptr), bytes->Length);
}