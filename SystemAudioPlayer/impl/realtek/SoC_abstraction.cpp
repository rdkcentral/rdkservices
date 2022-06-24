#include "RtkHALMisc.h"
#include "../SoC_abstraction.h"

void Soc_Initialize()
{
	
}
void Soc_Deinitialize()
{

}

void SoC_ChangePrimaryVol(MixGain gain, int volume)
{
	 if(gain == MIXGAIN_PRIM)
	 {		
		hal_set_audio_volume(volume);		 
	 }		
}
