#pragma once

#include <exception>
#include <string>

namespace device {

class Exception : public std::exception {
    int _err;
    std::string _msg;

public:
    Exception(const char* msg = "No Message for this exception") throw()
        : _msg(msg)
    {
    }

    Exception(int err, const char* msg = "No Message for this Exception") throw()
        : _err(err)
        , _msg(msg){};

    virtual const std::string& getMessage() const
    {
        return _msg;
    }

    virtual int getCode() const
    {
        return _err;
    }

    virtual const char* what() const throw()
    {
        return _msg.c_str();
    }

    virtual ~Exception() throw(){};
};

}
