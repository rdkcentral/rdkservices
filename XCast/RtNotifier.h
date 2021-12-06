/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright 2020 RDK Management
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
**/
#ifndef RT_NOTIFIER_H
#define RT_NOTIFIER_H

/**
* Abstract class for Notification.
*/
using namespace std;
class RtNotifier
{
public:
    
            virtual void onRtServiceDisconnected(void)=0;
            virtual void onXcastApplicationLaunchRequest(string appName, string parameter)=0;
            virtual void onXcastApplicationLaunchRequestWithLaunchParam (string appName,
                                        string strPayLoad, string strQuery, string strAddDataUrl)=0;
            virtual void onXcastApplicationStopRequest(string appName, string appID)=0;
            virtual void onXcastApplicationHideRequest(string appName, string appID)=0;
            virtual void onXcastApplicationResumeRequest(string appName, string appID)=0;
            virtual void onXcastApplicationStateRequest(string appName, string appID)=0;

            virtual bool onXcastSystemApplicationSleepRequest(string key) = 0;
};
#endif

