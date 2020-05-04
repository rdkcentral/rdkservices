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

#ifndef GSTREAMERSOUNDPLAYER_H
#define GSTREAMERSOUNDPLAYER_H

#include <mutex>
#include <gst/gst.h>

#include <string>

#include "SoundPlayer.h"

#define USE_MPEG_AUDIO_PARSE

typedef unsigned int XREIndex;

namespace WPEFramework
{
    namespace Plugin
    {
        class CGStreamerSoundPlayer;

        class ProgressInfo
        {
        private:
            ProgressInfo() = delete;
            ProgressInfo& operator=(const ProgressInfo& RHS) = delete;

        public:
            ProgressInfo(CGStreamerSoundPlayer* sp)
            : m_soundPlayer(sp)
            {
            }
            ProgressInfo(const ProgressInfo& copy)
            : m_soundPlayer(copy.m_soundPlayer)
            {
            }
            ~ProgressInfo() {}

            inline bool operator==(const ProgressInfo& RHS) const
            {
                return(m_soundPlayer == RHS.m_soundPlayer);
            }

        public:
            uint64_t Timed(const uint64_t scheduledTime);

        private:
            CGStreamerSoundPlayer* m_soundPlayer;
        };

        class CGStreamerSoundPlayer
        {
        public:
            CGStreamerSoundPlayer(ISoundListener *soundListener);
            virtual ~CGStreamerSoundPlayer();

        public:

            GstElement *getPipeline() { return m_pipeline; }

            GstElement *getID3Demux() { return m_id3demux; }

        #ifdef USE_MPEG_AUDIO_PARSE
            GstElement *getParser() { return m_parse; }
        #endif

            GstElement *getAudioSink() { return m_audioSink; }

            /**
             * unloads the sound file with the given XREIndex. No further
             * calls except setSoundFile() will result in a playable sound.
             */
            void deleteSoundFile();

            /**
             * Slot to handle change of gstreamer states.
             */
            //void onBusStateChanged(GstState newstate);

            /**
             * loads the file at soundFileUrl.
             * @param soundFileUrl The URL to the audio file. If not found, this
             * CGStreamerSoundItem attempts to load the sound from the file system
             * from the path specified by soundFileUrl.
             */
            void setSoundFile(std::string soundFile);

            /**
             * If not paused, start playback from the beginning; if paused, resume
             * playback.
             */
            void onResourcePlay();

            /**
             * If playing, pause playback.
             */
            void onResourcePause();

            /**
             * sets the volume for the sound file with the given id to
             * volumeLevel.
             * @param volumeLevel The volume level, 0 to 100. Defaults to 100.
             */
            void onResourceSetVolume(unsigned int volumeLevel=100);

            /**
             * sets the position to begin playing for the sound file with the
             * given id to position. Playback defaults to position 0 unless
             * this function is called or the sound is paused.
             * @param position The position to set the playhead in milliseconds.
             */
            void onResourceSetPosition(unsigned int position);

            void onResourceDeleteSoundItem();

            void onResourceSetSoundFile(std::string soundFile, bool useProxy, std::string proxyToUse, XREIndex resourceId, const std::map <std::string, std::string> &headers);

            /**
              * Slot to handle the timeout signal from progressTimer.
              */
            void checkProgress();

        private:

            bool aacDecodingEnabled();

            void processBusMessage(GstBus *bus, GstMessage *msg);
            void handleEoS(GstMessage *msg);
            void handleError(GstMessage *msg);
            void handleStateChanged(GstMessage *msg);

            void startProgressTimer();
            void stopProgressTimer();

            GstElement          *m_id3demux;
        #ifdef USE_MPEG_AUDIO_PARSE
            GstElement          *m_parse;
        #endif
            GstElement          *m_audioSink;
            GstElement          *m_pipeline;
            ProgressInfo           m_progressTimer;
            bool                m_runProgressTimer;
            std::mutex          m_progressMutex;

            GstState            m_currentGstState;
            ISoundListener      *m_soundListener;

            std::string m_soundFile;
            bool m_useProxy;
            bool m_startedPlayback;
            std::string m_proxyToUse;
            XREIndex m_resourceId;
            int m_progressTimerTick;

            std::map <std::string, std::string> m_headers;

            GstState validateStateWithMsTimeout(GstState stateToValidate, guint msTimeOut);
        };

    }
}


#endif // GSTREAMERSOUNDPLAYER_H
