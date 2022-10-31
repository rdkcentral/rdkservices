#ifndef RDKSHELL_ESSOS_INSTANCE_H
#define RDKSHELL_ESSOS_INSTANCE_H

//#include <essos.h>

namespace RdkShell
{
    class EssosInstance
    {
        public:
            ~EssosInstance();
            static EssosInstance* instance()
	    {
		    if (mInstance == NULL)
                    {
                       mInstance = new EssosInstance();
                     }
                    return mInstance;
	    }
            void initialize(bool useWayland){return;}
            void initialize(bool useWayland, uint32_t width, uint32_t height){return;}
            void configureKeyInput(uint32_t initialDelay, uint32_t repeatInterval){return;}
            void onKeyPress(uint32_t keyCode, unsigned long flags, uint64_t metadata){return;}
            void onKeyRelease(uint32_t keyCode, unsigned long flags, uint64_t metadata){return;}
            void onPointerMotion(uint32_t x, uint32_t y){return;}
            void onPointerButtonPress(uint32_t keyCode, uint32_t x, uint32_t y){return;}
            void onPointerButtonRelease(uint32_t keyCode, uint32_t x, uint32_t y){return;}
            void onDisplaySizeChanged(uint32_t width, uint32_t height){return;}
            void update(){return;}
            void resolution(uint32_t &width, uint32_t &height){return;}
            void setResolution(uint32_t width, uint32_t height){return;}
            void setKeyRepeats(bool enable){return;}
            void keyRepeats(bool& enable){return;}
            void ignoreKeyInputs(bool ignore){return;}

        private:
            EssosInstance()
	    {
               #ifdef RDKSHELL_ENABLE_KEYREPEATS
               mKeyRepeatsEnabled = true;
               #else
               mKeyRepeatsEnabled = false;
              // essosKeyListener.keyRepeat = essosKeyRepeat;
               #endif
	    }
            static EssosInstance* mInstance;
          //  EssCtx * mEssosContext;
            bool mUseWayland;
            uint32_t mWidth;
            uint32_t mHeight;
            bool mOverrideResolution;
            uint32_t mKeyInitialDelay;
            uint32_t mKeyRepeatInterval;
            bool mKeyRepeatsEnabled;
            bool mKeyInputsIgnored;
    };
    EssosInstance* EssosInstance::mInstance = NULL;
}

#endif //RDKSHELL_ESSOS_INSTANCE_H
