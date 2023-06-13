#include "../SoC_abstraction.h"
#include "audio_if.h"
#include <cmath>

static audio_hw_device_t *m_audio_dev=NULL;

void Soc_Initialize()
{
	if(!m_audio_dev)
    {
        int ret = audio_hw_load_interface(&m_audio_dev);
        if (ret) 
        {
            SAPLOG_ERROR("SAP: Amlogic audio_hw_load_interface failed:%d, can not control mix gain", ret);
            return;
        }
        int inited = m_audio_dev->init_check(m_audio_dev);
        if (inited) 
        {
            SAPLOG_ERROR("SAP: Amlogic audio device not inited, can not control mix gain\n");
            audio_hw_unload_interface(m_audio_dev);
            m_audio_dev = NULL;
            return;
        }  
        SAPLOG_INFO("SAP: Amlogic audio device loaded, can control mix gain");
        return ;
    }
	
}

void Soc_Deinitialize()
{
    if(m_audio_dev)
    {
        audio_hw_unload_interface(m_audio_dev);
        m_audio_dev = NULL;
    }	
}

void SoC_ChangePrimaryVol(MixGain gain, int volume)
{
     int ret;
     double volGain,dbOut;
     if(volume != 0)
     {
         volGain = (double)volume/100;
         //convert voltage gain/loss to db
         dbOut = round(1000000*20*(std::log(volGain)/std::log(10)))/1000000;
     }
     else
     {
         //minimum dBout value -96dB
         dbOut = -96.00000;
     }
     char mixgain_cmd[32];
     if(gain == MIXGAIN_PRIM)
         snprintf(mixgain_cmd, sizeof(mixgain_cmd), "prim_mixgain=%d",(int)round(dbOut));
        else if( gain == MIXGAIN_SYS )
                snprintf(mixgain_cmd, sizeof(mixgain_cmd), "syss_mixgain=%d",(int)round(dbOut));
        else if(gain == MIXGAIN_TTS)
                snprintf(mixgain_cmd, sizeof(mixgain_cmd), "apps_mixgain=%d",(int)round(dbOut));
         else {
                SAPLOG_ERROR("SAP: Unsuported Gain type=%d",gain);
                return;
        }

      if(m_audio_dev) {
         ret = m_audio_dev->set_parameters(m_audio_dev, mixgain_cmd );
         if(!ret) {
             SAPLOG_INFO("SAP: Amlogic audio dev  set param=%s success\n",mixgain_cmd);
         }
          else {
                SAPLOG_ERROR("SAP: Amlogic audio dev  set_param=%s failed  error=%d\n",mixgain_cmd,ret);
          }
     }
 	
}
