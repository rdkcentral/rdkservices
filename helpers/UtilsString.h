#pragma once
#include <fstream>
#include <sstream>
#include "UtilsLogging.h"
#define SYSTEM_MODE_FILE "/tmp/SystemMode.txt"

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
    
    // Split string s into a vector of strings using the supplied delimiter
    inline void split(std::vector<std::string> &stringList, std::string &s, std::string delimiters)
    {
        size_t current;
        size_t next = -1;
        do
        {
            current = next + 1;
            next = s.find_first_of( delimiters, current );

            stringList.push_back(s.substr( current, next - current ));
        }
        while (next != string::npos);
     }

    static const TCHAR base64_chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                        "abcdefghijklmnopqrstuvwxyz"
                                        "0123456789+/";


    void imageEncoder(const uint8_t object[], const uint32_t length, const bool padding, string& result)
    {
        uint8_t state = 0;
        uint32_t index = 0;
        uint8_t lastStuff = 0;

        while (index < length) {
            if (state == 0) {
                result += base64_chars[((object[index] & 0xFC) >> 2)];
                lastStuff = ((object[index] & 0x03) << 4);
                state = 1;
            } else if (state == 1) {
                result += base64_chars[(((object[index] & 0xF0) >> 4) | lastStuff)];
                lastStuff = ((object[index] & 0x0F) << 2);
                state = 2;
            } else if (state == 2) {
                result += base64_chars[(((object[index] & 0xC0) >> 6) | lastStuff)];
                result += base64_chars[(object[index] & 0x3F)];
                state = 0;
            }
            index++;
        }
        if (state != 0) {
            result += base64_chars[lastStuff];

            if (padding == true) {
                if (state == 1) {
                    result += _T("==");
                } else {
                    result += _T("=");
                }
            }
        }

    }

    // Function to replace all occurrences of a substring with another substring
    std::string replaceString(std::string sentence, const std::string& oldString, const std::string& newString) {

	    if (oldString.empty()) {
		    return sentence;
	    }

	    size_t pos = 0;
	    while ((pos = sentence.find(oldString, pos)) != std::string::npos) {
		    sentence.replace(pos, oldString.length(), newString);
		    pos += newString.length();
	    }
	    return sentence;
    }

    void updateSystemModeFile(const std::string& systemMode, const std::string& property, const std::string& value, const std::string& action) {

	    if (systemMode.empty() || property.empty()) {
		    LOGINFO("Error: systemMode or property is empty. systemMode: %s property: %s", systemMode.c_str(), property.c_str());
		    return;
	    }

	    if (action != "add" && action != "delete" && action != "deleteall") {
		    LOGINFO("Error: Invalid action. Action must be 'add', 'delete', or 'deleteall'.");
		    return;
	    }

	    std::ifstream infile(SYSTEM_MODE_FILE);
	    if (!infile.good()) {
		    // File doesn't exist, so create it
		    std::ofstream outfile(SYSTEM_MODE_FILE);
		    if (outfile) {
			    LOGINFO("File created successfully: %s\n", SYSTEM_MODE_FILE);

			    //set default value for each  SystemMode
			    Utils::String::updateSystemModeFile("DEVICE_OPTIMIZE" ,"currentstate" , "VIDEO" , "add");
		    } else {
			    LOGERR("Error creating file: %s\n", SYSTEM_MODE_FILE);
			    return;
		    }
	    } 
	    std::string line;
	    std::stringstream buffer;
	    bool propertyFound = false;
	    std::string searchKey = systemMode + "_" + property;

	    // Read the file content and process it line by line
	    if (infile.is_open()) {
		    while (std::getline(infile, line)) {
			    // If the line starts with the searchKey
			    if (line.find(searchKey) == 0) {
				    propertyFound = true;
				    if (action == "deleteall" && value.empty()) {
					    // Skip adding this line to the buffer, effectively removing it
					    continue;
				    } else if (property == "currentstate") {
					    if (action == "add") {
						    // Replace the value for currentstate
						    line = searchKey + "=" + value;
					    } else if (action == "delete") {
						    // To delete a currentstate, we might want to clear or remove the line
						    line.clear(); // This effectively removes the line
					    }
				    } else if (property == "callsign") {
					    if (action == "add") {
						    // Append the value to the callsign, ensuring no duplicate entries
						    if (line.find(value) == std::string::npos) {
							    line += value + "|";
						    }
					    } else if (action == "delete") {
						    // Remove the value from the callsign
						    size_t pos = line.find(value);
						    if (pos != std::string::npos) {
							    line.erase(pos, value.length() + 1); // +1 to remove the trailing '|'
						    }
					    }
				    }
			    }
			    if (!line.empty()) {
				    buffer << line << std::endl;
			    }
		    }
		    infile.close();
	    }

	    // If the property wasn't found and the action is "add", add it to the file
	    if (!propertyFound && action == "add") {
		    if (property == "currentstate") {
			    buffer << searchKey + "=" + value << std::endl;
		    } else if (property == "callsign") {
			    buffer << searchKey + "=" + value + "|" << std::endl;
		    }
	    }

	    // Write the modified content back to the file
	    std::ofstream outfile(SYSTEM_MODE_FILE);
	    if (outfile.is_open()) {
		    outfile << buffer.str();
		    outfile.close();
		    LOGINFO("Updated file %s successfully.", SYSTEM_MODE_FILE);
	    } else {
		    LOGINFO("Failed to open file %s for writing.", SYSTEM_MODE_FILE);
	    }
    }
    bool getSystemModePropertyValue(const std::string& systemMode, const std::string& property, std::string& value)  
    {
	    if (systemMode.empty() || property.empty() ) {
		    LOGINFO("Error: systemMode or property is empty. systemMode: %s property: %s ",systemMode.c_str(),property.c_str());
		    return false;
	    }

	    std::ifstream infile(SYSTEM_MODE_FILE);
	    std::string line;
	    std::string searchKey = systemMode + "_" + property;

	    if (!infile.is_open()) {
		    std::cerr << "Failed to open file: " << SYSTEM_MODE_FILE << std::endl;
		    return false;
	    }

	    while (std::getline(infile, line)) {
		    // Check if the line starts with the search key
		    if (line.find(searchKey) == 0) {
			    // Extract the value after the '=' character
			    size_t pos = line.find('=');
			    if (pos != std::string::npos) {
				    value = line.substr(pos + 1);
				    infile.close();
				    return true;
			    }
		    }
	    }

	    infile.close();
	    return false;
    }
}
}
