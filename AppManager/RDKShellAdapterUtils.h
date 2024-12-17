#ifndef RDKSHELL_ADAPTER_UTILS_H
#define RDKSHELL_ADAPTER_UTILS_H
#include "Module.h"
#include <string>

namespace WPEFramework
{
    namespace Plugin
    {
        namespace AppManagerV1
        {
#ifndef USE_THUNDER_R4
            class Job : public Core::IDispatchType<void> {
#else
            class Job : public Core::IDispatch {
#endif /* USE_THUNDER_R4 */
            public:
                Job(std::function<void()> work);
                void Dispatch() override;

            private:
                std::function<void()> _work;
            };

            uint32_t cloneService(PluginHost::IShell* shell, const string& basecallsign, const string& newcallsign);
            uint32_t getConfig(PluginHost::IShell* shell, const string& callsign, string& config);
            uint32_t setConfig(PluginHost::IShell* shell, const string& callsign, const string& config);
            uint32_t getServiceState(PluginHost::IShell* shell, const string& callsign, PluginHost::IShell::state& state);
            uint32_t activate(PluginHost::IShell* shell, const string& callsign);
            uint32_t deactivate(PluginHost::IShell* shell, const string& callsign);
        }
    }
}
#endif
