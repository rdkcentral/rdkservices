#ifndef SOC_ABSTRACTION
#define SOC_ABSTRACTION
#include <stdint.h>
#include "logger.h"

 
    enum MixGain {
        MIXGAIN_PRIM,
        MIXGAIN_SYS, //direct-mode=false, aml calls it sys mode
        MIXGAIN_TTS //tts=mode=true, AML calls it app mode
    };
	
    void Soc_Initialize();
    void Soc_Deinitialize();
    void SoC_ChangePrimaryVol(MixGain gain,int volume);
#endif

