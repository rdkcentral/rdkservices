#pragma once

#include <ctype.h>

namespace Utils {
inline bool isValidInt(char* x)
{
    bool Checked = true;
    int i = 0;

    if (x[0] == '-') {
        i = 1;
    }

    do {
        //valid digit?
        if (isdigit(x[i])) {
            //to the next character
            i++;
            Checked = true;
        } else {
            //to the next character
            i++;
            Checked = false;
            break;
        }
    } while (x[i] != '\0');
    return Checked;
}

inline bool isValidUnsignedInt(char* x)
{
    bool Checked = true;
    int i = 0;

    do {
        //valid digit?
        if (isdigit(x[i])) {
            //to the next character
            i++;
            Checked = true;
        } else {
            //to the next character
            i++;
            Checked = false;
            break;
        }
    } while (x[i] != '\0');
    return Checked;
}
}
