#pragma once

namespace Utils {
namespace String {
    // locale-wise comparison
    template <typename charT>
    struct loc_equal {
        explicit loc_equal(const std::locale& loc)
            : loc_(loc)
        {
        }
        bool operator()(charT ch1, charT ch2)
        {
            return std::toupper(ch1, loc_) == std::toupper(ch2, loc_);
        }

    private:
        const std::locale& loc_;
    };

    // Case-insensitive substring lookup.
    // Returns the substring position or -1
    // Example: int pos = find_substr_ci(string, substring, std::locale());
    template <typename T>
    int find_substr_ci(const T& string, const T& substring, const std::locale& loc = std::locale())
    {
        typename T::const_iterator it = std::search(string.begin(), string.end(),
            substring.begin(), substring.end(), loc_equal<typename T::value_type>(loc));
        if (it != string.end())
            return it - string.begin();
        else
            return -1; // not found
    }

    // Case-insensitive substring inclusion lookup.
    // Example: if (Utils::String::contains(result, processName)) {..}
    template <typename T>
    bool contains(const T& string, const T& substring, const std::locale& loc = std::locale())
    {
        int pos = find_substr_ci(string, substring, loc);
        return pos != -1;
    }

    // Case-insensitive substring inclusion lookup.
    // Example: if(Utils::String::contains(tmp, "grep -i")) {..}
    template <typename T>
    bool contains(const T& string, const char* c_substring, const std::locale& loc = std::locale())
    {
        std::string substring(c_substring);
        int pos = find_substr_ci(string, substring, loc);
        return pos != -1;
    }

    // Case-insensitive string comparison
    // returns true if the strings are equal, otherwise returns false
    // Example: if (Utils::String::equal(line, provisionType)) {..}
    template <typename T>
    bool equal(const T& string, const T& string2, const std::locale& loc = std::locale())
    {
        int pos = find_substr_ci(string, string2, loc);
        bool res = (pos == 0) && (string.length() == string2.length());
        return res;
    }

    // Case-insensitive string comparison
    // returns true if the strings are equal, otherwise returns false
    // Example: if(Utils::String::equal(line,"CRYPTANIUM")) {..}
    template <typename T>
    bool equal(const T& string, const char* c_string2, const std::locale& loc = std::locale())
    {
        std::string string2(c_string2);
        int pos = find_substr_ci(string, string2, loc);
        bool res = (pos == 0) && (string.length() == string2.length());
        return res;
    }

    // Trim space characters (' ', '\n', '\v', '\f', \r') on the left side of string
    inline void ltrim(std::string& s)
    {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
            return !std::isspace(ch);
        }));
    }

    // Trim space characters (' ', '\n', '\v', '\f', \r') on the right side of string
    inline void rtrim(std::string& s)
    {
        s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
            return !std::isspace(ch);
        }).base(),
            s.end());
    }

    // Trim space characters (' ', '\n', '\v', '\f', \r') on both sides of string
    inline void trim(std::string& s)
    {
        ltrim(s);
        rtrim(s);
    }

    inline void toUpper(std::string& s)
    {
        std::transform(s.begin(), s.end(), s.begin(), ::toupper);
    }

    inline void toLower(std::string& s)
    {
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    }

    // case insensitive comparison of strings
    inline bool stringContains(const std::string& s1, const std::string& s2)
    {
        return search(s1.begin(), s1.end(), s2.begin(), s2.end(), [](char c1, char c2) { return toupper(c1) == toupper(c2); }) != s1.end();
    }

    // case insensitive comparison of strings
    inline bool stringContains(const std::string& s1, const char* s2)
    {
        return stringContains(s1, std::string(s2));
    }
}
}