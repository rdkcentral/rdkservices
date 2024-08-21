#pragma once

extern "C" {

int EXTERNAL GetSecurityToken(unsigned short maxLength, unsigned char buffer[])
{
    if (maxLength < 1)
        return -1;

    buffer[0] = 0;
    return 0;
}

}
