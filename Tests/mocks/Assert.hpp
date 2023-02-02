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

/*
#ifndef HDMI_CCEC_ASSERT_HPP_
#define HDMI_CCEC_ASSERT_HPP_
#include "CCEC.hpp"

#include <assert.h>
#include <cstdio>
#include <stdexcept>

CCEC_BEGIN_NAMESPACE

#if 0
class Assert {
public:
	Assert(bool isTrue) : cond(isTrue) {
		if (!isTrue) {
            assert(isTrue);
		}
	}
private:
	const bool cond;
};
#else

#define Assert(expr) do{\
	if (!(expr)) printf("[%s] failed at [%s][%d]\r\n", "Assert ", __FILE__, __LINE__);\
	assert(expr);\
} while (0)
#endif

CCEC_END_NAMESPACE
#endif

*/
/** @} */
/** @} */

