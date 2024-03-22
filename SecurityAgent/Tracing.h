#pragma once

namespace WPEFramework {
namespace Plugin {
    class Security {
    public:
        Security() = delete;
        Security(const Security& a_Copy) = delete;
        Security& operator=(const Security& a_RHS) = delete;

    public:
        Security(const TCHAR formatter[], ...)
        {
            va_list ap;
            va_start(ap, formatter);
            Core::Format(_text, formatter, ap);
            va_end(ap);
        }
        explicit Security(const string& text)
            : _text(Core::ToString(text))
        {
        }
        ~Security() = default;

    public:
        inline const char* Data() const { return (_text.c_str()); }
        inline uint16_t Length() const { return (static_cast<uint16_t>(_text.length())); }

    private:
        std::string _text;
    };
} // namespace Plugin
} // namespace WPEFramework
