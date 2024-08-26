#include "Module.h"
#include "AnalyticsBackend.h"

#ifdef ANALYTICS_SIFT_BACKEND
#include "SiftBackend.h"
#endif

namespace WPEFramework {
namespace Plugin {

const std::string IAnalyticsBackend::SIFT = "Sift";

IAnalyticsBackends& IAnalyticsBackendAdministrator::Instances()
{
    static SiftBackend siftBackend;
    static IAnalyticsBackends backendInstances = {
#ifdef ANALYTICS_SIFT_BACKEND
    {IAnalyticsBackend::SIFT, siftBackend},
#endif
    };
    return (backendInstances);
}

}
}