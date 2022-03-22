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
 * @file exception.hpp
 * @brief This file defines Exception class for handling exceptions in device
 * settings module.
 */



/**
* @defgroup devicesettings
* @{
* @defgroup ds
* @{
**/

#ifndef _DS_EXCEPTION_H_
#define _DS_EXCEPTION_H_

#include <string>
#include <iostream>
#include <exception>

namespace device{

/**
 * @class Exception
 * @brief This class handles exceptions occurring in DS module.
 * @ingroup devicesettingsclass
 */
class Exception : public std::exception {
        int _err;            //!< Indicates error code for the exception.
        std::string _msg;    //!< Indicates the error message.

public:

/**
 * @fn Exception::Exception(const char *msg = "No Message for this exception")
 * @brief This function is a parameterised constructor of the class Exception.
 * It initializes the instance with the msg string passed as input parameter.
 *
 * @param[in] msg Message string for the exception.
 *
 * @return None
 */
        Exception(const char *msg = "No Message for this exception") throw() : _msg(msg) {
        }


/**
 * @fn Exception::Exception(int err, const char *msg = "No Message for this Exception")
 * @brief This function is a parameterised constructor of the class Exception. It
 * initializes the instance with both message string and the error code passed as
 * input parameter.
 *
 * @param[in] err Indicates the error code.
 * @param[in] msg Message string for the exception.
 *
 * @return None
 */
        Exception(int err, const char *msg = "No Message for this Exception") throw()
        : _err(err), _msg(msg){
        };


/**
 * @fn Exception::getMessage()
 * @brief This function is used to get the message string of the exception.
 *
 * @return _msg Message string of the exception is returned.
 */
        virtual const std::string & getMessage() const {
                return _msg;
        }


/**
 * @fn Exception::getCode()
 * @brief This function is used to get the error code of the exception.
 *
 * @return _err Error code of the exception is returned.
 */
        virtual int getCode() const {
                return _err;
        }


/**
 * @fn Exception::what()
 * @brief This function is overwritten to get the null terminated character sequence
 * of the exception message.
 *
 * @return Returns a null terminated character sequence of the exception message.
 */
        virtual const char * what() const throw() {
                return _msg.c_str();
        }


/**
 * @fn Exception::~Exception()
 * @brief This function is the default destructor of Exception class.
 *
 * @return None
 */
        virtual ~Exception() throw() {};
};

}
#endif /* EXCEPTION_H_ */


/** @} */

