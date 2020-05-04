/******************************************************************************
 *  Copyright (c) 2016 RDK Management, LLC. All rights reserved.
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to the terms and
 *  conditions of a separate, written license agreement executed between you and Broadcom
 *  (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 *  no license (express or implied), right to use, or waiver of any kind with respect to the
 *  Software, and Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 *  HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 *  NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 *  secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 *  and to use this information only in connection with your use of Broadcom integrated circuit products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *  AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *  WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 *  THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 *  OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 *  LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 *  OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 *  USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *  LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 *  EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 *  USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 *  THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 *  ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 *  LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 *  ANY LIMITED REMEDY.
 ******************************************************************************/

#ifndef GSTREAMERSOUNDPLAYER_H
#define GSTREAMERSOUNDPLAYER_H

#include <mutex>
#include <gst/gst.h>
#include <plugins/plugins.h>
#include <string>

#include "SoundPlayer.h"

namespace WPEFramework
{
    namespace Plugin
    {
        typedef unsigned int XREIndex;

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
            static void releaseCurrentDecoder();

        public:

            /**
              * Accessor for the gstreamer pipeline.
              */
            GstElement *getPipeline()
            {
#ifdef USE_PIPELINE_FACTORY
                return PipelineFactory::instance()->getPipeline(this);
#else
                return m_pipeline;
#endif
            }

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


        public:

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
            void onResourceSetVolume(unsigned volumeLevel=100);

            /**
             * sets the position to begin playing for the sound file with the
             * given id to position. Playback defaults to position 0 unless
             * this function is called or the sound is paused.
             * @param position The position to set the playhead in milliseconds.
             */
            void onResourceSetPosition(unsigned int position);

            //void onResourceSetSoundFile(QString soundFile, bool useProxy, QString proxyToUse, XREIndex resourceId);
            void onResourceSetSoundFile(std::string soundFile, bool useProxy, std::string proxyToUse, XREIndex resourceId, const std::map <std::string, std::string> &headers);

            /**
             * Slot to handle the timeout signal from progressTimer.
             */
            void checkProgress();

            void onError(const std::string &error);

        private:
            void processBusMessage(GstBus *bus, GstMessage *msg);
            void handleEoS(GstMessage *msg);
            void handleError(GstMessage *msg);
            void handleStateChanged(GstMessage *msg);

            void startProgressTimer();
            void stopProgressTimer();

            GstElement          *m_audioSink;
            GstElement          *m_pipeline;
            GstElement          *m_source;
            GstElement          *m_volumeElement;
            int                 m_volume;
            GstState            m_currentGstState;
            ISoundListener      *m_soundListener;

            std::string                m_soundUrl;
            int                 m_pipelineIndex;
            unsigned int        m_position;
            std::string m_localProxyServerSetting;
            ProgressInfo           m_progressTimer;
            bool           m_runProgressTimer;
            std::mutex      m_progressMutex;
            bool                m_useProxy;
            std::string             m_proxyToUse;
            std::map <std::string, std::string> m_headers;
            XREIndex            m_resourceId;
            int                 m_progressTimerTick;

            void pause_privLocked();
            void createPipeline(int);
            void destroyPipeline(int);

            bool aacDecodingEnabled();

        };
    }
}

#endif // GSTREAMERSOUNDPLAYER_H
