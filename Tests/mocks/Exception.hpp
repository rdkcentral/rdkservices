/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2016 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/



/**
* @defgroup hdmicec
* @{
* @defgroup ccec
* @{
**/


#ifndef HDMI_CCEC_EXCEPTION_HPP_
#define HDMI_CCEC_EXCEPTION_HPP_
#include "CCEC.hpp"


CCEC_BEGIN_NAMESPACE

typedef struct _Throw_e{
	/*Empty Struct */
}Throw_e;


class Exception : public std::exception
{
public:
	virtual const char* what() const throw()
	{
		return "Base Exception..";
	}
private:
};

class CECNoAckException : public Exception
{
public:
	virtual const char* what() const throw()
	{
		return "Ack not received..";
	}
};

class OperationNotSupportedException : public Exception
{
public:
	virtual const char* what() const throw()
	{
		return "Operation Not Supported..";
	}
};


class IOException : public Exception
{
public:
	virtual const char* what() const throw()
	{
		return "IO Exception..";
	}
};


class InvalidStateException : public Exception
{
public:
	virtual const char* what() const throw()
	{
		return "Invalid State Exception..";
	}
};

class InvalidParamException : public Exception
{
public:
	virtual const char* what() const throw()
	{
		return "Invalid Param Exception..";
	}
};

class AddressNotAvailableException : public Exception
{
public:
	virtual const char* what() const throw()
	{
		return "Address Not Available Exception..";
	}
};

CCEC_END_NAMESPACE


#endif


/** @} */
/** @} */

