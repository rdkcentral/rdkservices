#pragma once

#include <fstream>
#include <iostream>
#define READ_BUFFER_SIZE 1024
namespace Utils {

/**
 * @brief Read the property value from the given file based on the provided property name.
 * @param[in] filename - The name of the file from which to read the properties.
 * @param[in] property -  The name of the property to search for in the file.
 * @param[out] propertyValue - The value of the property will be stored in this string.
 * @return          bool  True if the property is found and successfully read, false otherwise.
 */
inline bool readPropertyFromFile(const char* filename, const std::string& property, std::string& propertyValue)
{
    std::string line = "";
    bool found = false;
    std::ifstream file(filename);
    propertyValue = "";
    if (file.is_open())
    {
        while (std::getline(file, line))
        {
            // Skip lines that start with '#' (single-line comments)
            if (!line.empty() && line[0] == '#')
            {
                continue;
            }
            if (line.find(property + " ") == 0 || line.find(property + "=") == 0)
            {
                std::string propertyContent = line.substr(line.find("=") + 1);

                // If the property value starts with '$',recursively expand it
                if (!propertyContent.empty() && propertyContent[0] == '$')
                {
                    std::string expandedProperty = propertyContent.substr(1);
                    if (readPropertyFromFile(filename, expandedProperty, propertyValue))
                    {
                        found = true;
                        break;
                    }
                    else
                    {
                        LOGERR("Failed to find expanded property: %s", expandedProperty.c_str());
                    }
                }
                else
                {
                    // If it does not start with '$', set propertyValue directly
                    propertyValue = propertyContent;
                    if (!propertyValue.empty())
                    {
                        // Remove new line character from end of the string if it exists
                        if ((propertyValue.back() == '\r') || (propertyValue.back() == '\n'))
                        {
                            propertyValue.pop_back();
                        }
                    }
                    found = true;
                    break;
                }
            }
        }
    }
    else
    {
        LOGERR("File is not open");
    }

    // If the property was not found, set the propertyValue to an empty string
    if (!found)
    {
       LOGERR("Variable value is empty");
    }

    return found;
}

/**
 * @brief Read the content of a file and store it in the provided string.
 * @param[in] filename - The name of the file to read.
 * @param[out] content - The content of the file will be stored in this string.
 * @return  bool  True if the file is successfully read and its content is stored in 'content', false otherwise.
 */

inline bool readFileContent(const char* filename, std::string& content)
{
    char buffer[READ_BUFFER_SIZE];
    FILE* file = fopen(filename, "r");
    bool found = false;

    if (file)
    {
        while (fgets(buffer, sizeof(buffer), file) != NULL)
        {
            content += buffer;
        }
        fclose(file);
        found = true;
    }
    else
    {
        LOGERR("Failed to open the file");
    }

    return found;
}


/**
 * @brief Check if a given path corresponds to a regular file.
 * @param[in] path - The path to check.
 * @return bool - True if the path corresponds to a regular file, false otherwise.
 */
inline bool isRegularFile(const std::string& path)
{
    struct stat st;
    if (stat(path.c_str(), &st) == 0)
    {
        return S_ISREG(st.st_mode);
    }
    return false;
}

/**
 * @brief Recursively search for files and directories in a given directory path, with depth limits and exclusions.
 * @param[in] inputPath - The directory path to start the search from.
 * @param[in] maxDepth - The maximum depth of subdirectories to search (0 for no limit).
 * @param[in] minDepth - The minimum depth of subdirectories to start searching from (0 to start from inputPath).
 * @param[in] exclusions - A list of paths to exclude from the search.
 * @param[out] result - The search results will be stored in this string.Results are capped at 10.
 * @return bool - True if the search operation is successful, false otherwise.
 */
inline bool searchFiles(std::string& inputPath, int maxDepth, int minDepth, const std::list<std::string>& exclusions, std::string& result)
{
    int count = 0;
    if (minDepth <= 0)
    {
        // Process files and directories in the current directory
        DIR* dir = opendir(inputPath.c_str());
        if (!dir)
        {
            LOGERR("Failed to open the directory");
            return false;
        }

        struct dirent* entry;
        while ((entry = readdir(dir)))
        {
            std::string fileName = entry->d_name;

            if (fileName == "." || fileName == "..")
            {
                continue;
            }

            std::string filePath = inputPath + "/" + fileName;

            if (std::find(exclusions.begin(), exclusions.end(), filePath) == exclusions.end())
            {
                if (isRegularFile(filePath))
                {
                    result += "f: " + filePath + "\n";
                }
                else if (entry->d_type == DT_DIR)
                {
                    result += "d: " + filePath + "\n";
                }
                count++;
                if (count >= 10)
                    break; // Stop when count reaches 10
            }
        }
        closedir(dir);
    }

    if (maxDepth > 1 || (maxDepth == 1 && minDepth == 1))
    {
        // Recursively process subdirectories
        DIR* dir = opendir(inputPath.c_str());
        if (!dir)
        {
            LOGERR("Failed to open the directory");
            return false;
        }

        struct dirent* entry;
        while ((entry = readdir(dir)))
        {
            std::string fileName = entry->d_name;
            if (fileName == "." || fileName == "..")
            {
                continue;
            }

            std::string filePath = inputPath + "/" + fileName;
            if (entry->d_type == DT_DIR)
            {
                if (minDepth <= 0)
                {
                    // Process subdirectory
                    std::string subResult;
                    if (searchFiles(filePath, maxDepth - 1, 0, exclusions, subResult))
                    {
                        result += "d: " + filePath + "\n" + subResult;
                        count++;
                        if (count >= 10)
                            break;
                     }
                }
                else
                {
                    // Recursively process subdirectory with reduced minDepth
                    std::string subResult;
                    if (searchFiles(filePath, maxDepth - 1, minDepth - 1, exclusions, subResult))
                    {
                        result += "d: " + filePath + "\n" + subResult;
                        count++;
                        if (count >= 10)
                            break; // Stop when count reaches 10
                    }
                }
            }
        }

        closedir(dir);
    }
    return true;
}

/**
 * @brief Process a string containing variables and replace them with corresponding values.
 *
 * @param[in] input - The input string containing variables; delimited by ' ' or '/'
 * @param[in] filePath - The path to the file containing property values.
 * @param[out] expandedString - The string with variables replaced by values.
 * @return bool - True if the processing and replacement are successful, false otherwise.
 */
inline bool ExpandPropertiesInString(const char* input, const char* filePath, std::string & expandedString)
{
    const char* variablePos = strchr(input, '$');
    while (variablePos)
    {
        expandedString.assign(input, variablePos - input);
        const char* endPos = strpbrk(variablePos, " /");
        if (endPos)
        {
            size_t variableLength = endPos - variablePos - 1;
            char variable[variableLength + 1];
            strncpy(variable, variablePos + 1, variableLength);
            variable[variableLength] = '\0';

            std::string tempPropertyValue;
            if (readPropertyFromFile(filePath, variable, tempPropertyValue))
            {
                const char* propertyValue = tempPropertyValue.c_str();
                expandedString += tempPropertyValue;
                variablePos += strlen(propertyValue);
            }
            else
            {
                LOGERR("Variable '%s' not found or error reading value.\n", variable);
                return false;
            }
         }

        else
        {
            endPos = variablePos + 1;
        }
        variablePos = strchr(endPos, '$');
        if (variablePos)
        {
            expandedString.append(endPos, variablePos - endPos);
        }
        else
        {
            expandedString += endPos;
        }
    }

    return true;
}

}
