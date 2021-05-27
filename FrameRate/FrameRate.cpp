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

#include "FrameRate.h"

#include "utils.h"

// Methods
#define METHOD_SET_COLLECTION_FREQUENCY "setCollectionFrequency"
#define METHOD_START_FPS_COLLECTION "startFpsCollection"
#define METHOD_STOP_FPS_COLLECTION "stopFpsCollection"
#define METHOD_UPDATE_FPS_COLLECTION "updateFps"

// Events
#define EVENT_FPS_UPDATE "onFpsEvent"

//Defines
#define DEFAULT_FPS_COLLECTION_TIME_IN_MILLISECONDS 10000
#define MINIMUM_FPS_COLLECTION_TIME_IN_MILLISECONDS 100
#define DEFAULT_MIN_FPS_VALUE 60
#define DEFAULT_MAX_FPS_VALUE -1

namespace WPEFramework
{
    namespace Plugin
    {
        SERVICE_REGISTRATION(FrameRate, 1, 0);

        FrameRate* FrameRate::_instance = nullptr;

        FrameRate::FrameRate()
        : AbstractPlugin()
          , m_fpsCollectionFrequencyInMs(DEFAULT_FPS_COLLECTION_TIME_IN_MILLISECONDS)
          , m_minFpsValue(DEFAULT_MIN_FPS_VALUE), m_maxFpsValue(DEFAULT_MAX_FPS_VALUE)
          , m_totalFpsValues(0), m_numberOfFpsUpdates(0), m_fpsCollectionInProgress(false), m_lastFpsValue(-1)
        {
            FrameRate::_instance = this;

            Register(METHOD_SET_COLLECTION_FREQUENCY, &FrameRate::setCollectionFrequencyWrapper, this);
            Register(METHOD_START_FPS_COLLECTION, &FrameRate::startFpsCollectionWrapper, this);
            Register(METHOD_STOP_FPS_COLLECTION, &FrameRate::stopFpsCollectionWrapper, this);
            Register(METHOD_UPDATE_FPS_COLLECTION, &FrameRate::updateFpsWrapper, this);
            
            m_reportFpsTimer.connect( std::bind( &FrameRate::onReportFpsTimer, this ) );
        }

        FrameRate::~FrameRate()
        {
        }

        void FrameRate::Deinitialize(PluginHost::IShell* /* service */)
        {
            FrameRate::_instance = nullptr;
        }

        uint32_t FrameRate::setCollectionFrequencyWrapper(const JsonObject& parameters, JsonObject& response)
        {
            std::lock_guard<std::mutex> guard(m_callMutex);

            LOGINFOMETHOD();
            try{
            	if (parameters.HasLabel("frequency"))
            	{
            		int fpsFrequencyInMilliseconds = DEFAULT_FPS_COLLECTION_TIME_IN_MILLISECONDS;
	                fpsFrequencyInMilliseconds = std::stod(parameters["frequency"].String());
			if( fpsFrequencyInMilliseconds > = 100) // make sure min freq is 100 and not less than that.
			{
        	    		setCollectionFrequency(fpsFrequencyInMilliseconds);
            			returnResponse(true);
			}
            	}
		}
	     catch(...)
	     {
		returnResponse(false);
 	     }	
            
             returnResponse(false);
        }
        
        uint32_t FrameRate::startFpsCollectionWrapper(const JsonObject& parameters, JsonObject& response)
        {
            std::lock_guard<std::mutex> guard(m_callMutex);

            LOGINFOMETHOD();

            returnResponse(startFpsCollection());
        }
        
        uint32_t FrameRate::stopFpsCollectionWrapper(const JsonObject& parameters, JsonObject& response)
        {
            std::lock_guard<std::mutex> guard(m_callMutex);

            LOGINFOMETHOD();

            returnResponse(stopFpsCollection());
        }
        
        uint32_t FrameRate::updateFpsWrapper(const JsonObject& parameters, JsonObject& response)
        {
            std::lock_guard<std::mutex> guard(m_callMutex);

            LOGINFOMETHOD();
            
            if (!parameters.HasLabel("newFpsValue"))
            {
                returnResponse(false);
            }
            
            updateFps(parameters["newFpsValue"].Number());

            returnResponse(true);
        }
        
        /**
        * @brief This function is used to get the amount of collection interval per milliseconds.
        *
        * @return Integer value of Amount of milliseconds per collection interval .
        */
        int FrameRate::getCollectionFrequency()
        {
            return m_fpsCollectionFrequencyInMs;
        }

        /**
        * @brief This function is used to set the amount of collection interval per milliseconds.
        *
        * @param[in] frequencyInMs Amount of milliseconds per collection interval.
        * @ingroup SERVMGR_ABSFRAMERATE_API
        */
        void FrameRate::setCollectionFrequency(int frequencyInMs)
        {
            m_fpsCollectionFrequencyInMs = frequencyInMs;
        }

        /**
        * @brief This function is used to start the fps collection. Stop the fps timer before
        * start the fps collection frequency. Fps collection frequency is updated to minimum fps
        * collection time if the fps collection frequency is less than the minimum fps collection time
        * and start the fps timer.
        *
        * @return true on success else false if there was an error.
        * @ingroup SERVMGR_ABSFRAMERATE_API
        */
        bool FrameRate::startFpsCollection()
        {
            if (m_fpsCollectionInProgress)
            {
                return false;
            }
            if (m_reportFpsTimer.isActive())
            {
                m_reportFpsTimer.stop();
            }
            m_minFpsValue = DEFAULT_MIN_FPS_VALUE;
            m_maxFpsValue = DEFAULT_MAX_FPS_VALUE;
            m_totalFpsValues = 0;
            m_numberOfFpsUpdates = 0;
            m_fpsCollectionInProgress = true;
            int fpsCollectionFrequency = m_fpsCollectionFrequencyInMs;
            if (fpsCollectionFrequency < MINIMUM_FPS_COLLECTION_TIME_IN_MILLISECONDS)
            {
                fpsCollectionFrequency = MINIMUM_FPS_COLLECTION_TIME_IN_MILLISECONDS;
            }
            m_reportFpsTimer.start(fpsCollectionFrequency);
            enableFpsCollection();
            return true;
        }

        /**
        * @brief This function is used to stops the fps collection. Stop the fps timer before disable the
        * fps collection. If the number of fps updates is greater than 0, update the fps collection by
        * passing the minimum fps, maximum fps and average fps values  and disable the fps collection.
        *
        * @return true on success or false if there was an error.
        * @ingroup SERVMGR_ABSFRAMERATE_API
        */
        bool FrameRate::stopFpsCollection()
        {
            if (m_reportFpsTimer.isActive())
            {
                m_reportFpsTimer.stop();
            }
            if (m_fpsCollectionInProgress)
            {
                m_fpsCollectionInProgress = false;
                int averageFps = -1;
                int minFps = -1;
                int maxFps = -1;
                if (m_numberOfFpsUpdates > 0)
                {
                averageFps = (m_totalFpsValues / m_numberOfFpsUpdates);
                minFps = m_minFpsValue;
                maxFps = m_maxFpsValue;
                fpsCollectionUpdate(averageFps, minFps, maxFps);
                }
                disableFpsCollection();
            }
            return true;
        }

        /**
        * @brief This function is used to update the FPS value.
        *
        * @param[in] newFpsValue Latest amount of milliseconds per collection interval.
        * @ingroup SERVMGR_ABSFRAMERATE_API
        */
        void FrameRate::updateFps(int newFpsValue)
        {
            if (newFpsValue > m_maxFpsValue)
            {
                m_maxFpsValue = newFpsValue;
            }
            if (newFpsValue < m_minFpsValue)
            {
                m_minFpsValue = newFpsValue;
            }
            m_totalFpsValues += newFpsValue;
            m_numberOfFpsUpdates++;
            m_lastFpsValue = newFpsValue;
        }
        
        void FrameRate::fpsCollectionUpdate( int averageFps, int minFps, int maxFps )
        {
            JsonObject params;
            params["average"] = averageFps;
            params["min"] = minFps;
            params["max"] = maxFps;
            
            sendNotify(EVENT_FPS_UPDATE, params);
        }
        
        void FrameRate::onReportFpsTimer()
        {
            std::lock_guard<std::mutex> guard(m_callMutex);
            
            int averageFps = -1;
            int minFps = -1;
            int maxFps = -1;
            if (m_numberOfFpsUpdates > 0)
            {
                averageFps = (m_totalFpsValues / m_numberOfFpsUpdates);
                minFps = m_minFpsValue;
                maxFps = m_maxFpsValue;
            }
            fpsCollectionUpdate(averageFps, minFps, maxFps);
            if (m_lastFpsValue >= 0)
            {
                // store the last fps value just in case there are no updates
                m_minFpsValue = m_lastFpsValue;
                m_maxFpsValue = m_lastFpsValue;
                m_totalFpsValues = m_lastFpsValue;
                m_numberOfFpsUpdates = 1;
            }
            else
            {
                m_minFpsValue = DEFAULT_MIN_FPS_VALUE;
                m_maxFpsValue = DEFAULT_MAX_FPS_VALUE;
                m_totalFpsValues = 0;
                m_numberOfFpsUpdates = 0;
            }
        }
        
    } // namespace Plugin
} // namespace WPEFramework
