/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright 2019 RDK Management
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

/**
 * @defgroup servicemanager
 * @{
 * @defgroup include
 * @{
 **/


#ifndef THERMALMONITOR_HPP
#define THERMALMONITOR_HPP

#ifndef ENABLE_THERMAL_PROTECTION
#define ENABLE_THERMAL_PROTECTION
#endif

#include "SystemServices.h"

/**
 * This class defines the functionalities for thermal monitoring.
 **/
namespace WPEFramework {
    namespace Plugin {

        class CThermalMonitor {
            public:
                static CThermalMonitor* instance();

                void addEventObserver(WPEFramework::Plugin::SystemServices* service);
                void removeEventObserver(WPEFramework::Plugin::SystemServices* service);

                bool getCoreTemperature(float& temperature) const;
                void emitTemperatureThresholdChange(std::string thresholdType, bool isAboveThreshold, float temperature);
                bool getCoreTempThresholds(float& high, float& critical) const;
                bool setCoreTempThresholds(float high, float critical) const;
		bool getOvertempGraceInterval(int& graceInterval) const;
		bool setOvertempGraceInterval(int graceInterval) const;
                void reportTemperatureThresholdChange(std::string thresholdType, bool isAboveThreshold, float temperature);
        };
    }
}
#endif

/** @} */
/** @} */
