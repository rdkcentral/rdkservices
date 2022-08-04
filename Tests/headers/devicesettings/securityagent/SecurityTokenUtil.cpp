#include "securityagent.h"
#include <cstring>

using namespace std;
extern "C" {
	int GetSecurityToken(unsigned short maxLength, unsigned char buffer[])
	{
    		string payload = "eyJhbGciOiIiLCJ0eXAiOiJKV1QifQ.aHR0cDovL2xvY2FsaG9zdA.U-Y6mAOly5koIzsv6CowwP1p2OhA9DnYKMohrjbfYhQ"; // example token
    		size_t len = payload.length();
    		if(!memcpy(buffer,payload.c_str(),len))
        		return -1;
    		return 0;
	}
}
