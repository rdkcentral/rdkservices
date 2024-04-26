/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2023 RDK Management
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

#include <gst/audio/audio.h>
#include <gst/app/gstappsink.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <sys/types.h>
#include <sys/syscall.h>
#include "MiracastLogger.h"
#include "MiracastController.h"
#include "SoC_abstraction.h"

MiracastGstPlayer *MiracastGstPlayer::mMiracastGstPlayer{nullptr};

MiracastGstPlayer *MiracastGstPlayer::getInstance()
{
	if (mMiracastGstPlayer == nullptr)
	{
		mMiracastGstPlayer = new MiracastGstPlayer();
	}
	return mMiracastGstPlayer;
}

void MiracastGstPlayer::destroyInstance()
{
	MIRACASTLOG_TRACE("Entering...");
	if (mMiracastGstPlayer != nullptr)
	{
		MIRACASTLOG_INFO("Stop miracast player");
		delete mMiracastGstPlayer;
		mMiracastGstPlayer = nullptr;
	}
	MIRACASTLOG_TRACE("Exiting...");
}

MiracastGstPlayer::MiracastGstPlayer()
{
}

MiracastGstPlayer::~MiracastGstPlayer()
{
}

bool MiracastGstPlayer::setVideoRectangle( VIDEO_RECT_STRUCT video_rect , bool apply )
{
	MIRACASTLOG_TRACE("Entering...");
	MIRACASTLOG_TRACE("Exiting Coords[%d,%d,%d,%d]Apply[%x]...",
			video_rect.startX,video_rect.startY,video_rect.width,video_rect.height,
			apply);
	return true;
}

bool MiracastGstPlayer::launch(std::string& localip , std::string& streaming_port, MiracastRTSPMsg *rtsp_instance)
{
	if ( nullptr != rtsp_instance )
	{
		m_rtsp_reference_instance = rtsp_instance;
		this->onFirstVideoFrameCallback(nullptr,0,0,this);
	}
	return true;
}

bool MiracastGstPlayer::pause()
{
	return true;
}

bool MiracastGstPlayer::resume()
{
	return true;
}

bool MiracastGstPlayer::stop()
{
	destroyInstance();
	return true;
}

void MiracastGstPlayer::onFirstVideoFrameCallback(GstElement* object, guint arg0, gpointer arg1,gpointer userdata)
{
	MIRACASTLOG_TRACE("Entering..!!!");
	MiracastGstPlayer *self = static_cast<MiracastGstPlayer*>(userdata);
	self->m_firstVideoFrameReceived = true;
	MIRACASTLOG_INFO("!!! First Video Frame has received !!!");
	self->notifyPlaybackState(MIRACAST_GSTPLAYER_STATE_FIRST_VIDEO_FRAME_RECEIVED);
	MIRACASTLOG_TRACE("Exiting..!!!");
}

void MiracastGstPlayer::notifyPlaybackState(eMIRA_GSTPLAYER_STATES gst_player_state)
{
	MIRACASTLOG_TRACE("Entering..!!!");
	if ( nullptr != m_rtsp_reference_instance )
	{
		RTSP_HLDR_MSGQ_STRUCT rtsp_hldr_msgq_data = {0};

		rtsp_hldr_msgq_data.state = RTSP_NOTIFY_GSTPLAYER_STATE;
		rtsp_hldr_msgq_data.gst_player_state = gst_player_state;
		MIRACASTLOG_INFO("!!! GstPlayer to RTSP [%#08X] !!!",gst_player_state);
		m_rtsp_reference_instance->send_msgto_rtsp_msg_hdler_thread(rtsp_hldr_msgq_data);
	}
	MIRACASTLOG_TRACE("Exiting..!!!");
}
