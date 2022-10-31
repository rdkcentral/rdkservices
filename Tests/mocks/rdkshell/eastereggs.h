#ifndef RDKSHELL_EASTER_EGG_H
#define RDKSHELL_EASTER_EGG_H

#include <string>
#include "rdkshellevents.h"

namespace RdkShell
{
    struct RdkShellEasterEggKeyDetails
    {
        RdkShellEasterEggKeyDetails() {}
        RdkShellEasterEggKeyDetails(uint32_t code, uint32_t modifiers, uint32_t holdTime): keyCode(code), keyModifiers(modifiers), keyHoldTime(holdTime) {}
        uint32_t keyCode;
        uint32_t keyModifiers;
        uint32_t keyHoldTime;
    };

    struct RdkShellEasterEggDetails
    {
        std::string id;
        std::vector<RdkShellEasterEggKeyDetails> keySequence;
        uint32_t duration; 
        std::string api;
    };

    class EasterEgg
    {
        public:
             EasterEgg (std::vector<RdkShellEasterEggKeyDetails>& details, std::string name, uint32_t timeout, std::string actionJson);
             void checkKey(uint32_t keyCode, uint32_t flags, double keyPressTime);
             bool invokeEvent();
             void reset();
             size_t numberOfKeys();
             std::string name();
             uint32_t timeout();
             std::vector<RdkShellEasterEggKeyDetails> keyDetails();
             std::string actionJson();
        private:
            void toggleForce720();
            std::vector<RdkShellEasterEggKeyDetails> mKeyDetails;
            std::string mName;
            uint32_t mTimeout; 
            std::string mActionJson;
            uint32_t mCurrentKeyIndex;
            double mTotalUsedTime;
            bool mSatisfied;
    };

    void populateEasterEggDetails(){return;}
    void checkEasterEggs(uint32_t keyCode, uint32_t flags, double time){return;}
    void resolveWaitingEasterEggs(){return;}
    void addEasterEgg(std::vector<RdkShellEasterEggKeyDetails>& details, std::string name, uint32_t timeout, std::string actionJson){return;}
    void removeEasterEgg(std::string name){return;}
    void getEasterEggs(std::vector<RdkShellEasterEggDetails>& easterEggs){return;}
}

#endif //RDKSHELL_EASTER_EGG_H
