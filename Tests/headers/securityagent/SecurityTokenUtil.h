#pragma once
extern "C" {
    int GetSecurityToken(unsigned short maxLength, unsigned char buffer[]){
    	buffer = (unsigned char*)"";
        return 1;
    }
}
