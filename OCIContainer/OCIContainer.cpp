#include "OCIContainer.h"

#include <Dobby/DobbyProxy.h>
#include <Dobby/IpcService/IpcFactory.h>

#include "UtilsJsonRpc.h"

#define API_VERSION_NUMBER_MAJOR 1
#define API_VERSION_NUMBER_MINOR 0
#define API_VERSION_NUMBER_PATCH 0

namespace WPEFramework
{

namespace {

    static Plugin::Metadata<Plugin::OCIContainer> metadata(
        // Version (Major, Minor, Patch)
        API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH,
        // Preconditions
        {},
        // Terminations
        {},
        // Controls
        {}
    );
}

namespace Plugin
{

namespace WPEC = WPEFramework::Core;
namespace WPEJ = WPEFramework::Core::JSON;

SERVICE_REGISTRATION(OCIContainer, API_VERSION_NUMBER_MAJOR, API_VERSION_NUMBER_MINOR, API_VERSION_NUMBER_PATCH);

OCIContainer::OCIContainer()
    : PluginHost::JSONRPC()
{
    Register("listContainers", &OCIContainer::listContainers, this);
    Register("getContainerState", &OCIContainer::getContainerState, this);
    Register("getContainerInfo", &OCIContainer::getContainerInfo, this);
    Register("startContainer", &OCIContainer::startContainer, this);
    Register("startContainerFromDobbySpec", &OCIContainer::startContainerFromDobbySpec, this);
    Register("stopContainer", &OCIContainer::stopContainer, this);
    Register("pauseContainer", &OCIContainer::pauseContainer, this);
    Register("resumeContainer", &OCIContainer::resumeContainer, this);
    Register("executeCommand", &OCIContainer::executeCommand, this);
}

OCIContainer::~OCIContainer()
{
}

const string OCIContainer::Initialize(PluginHost::IShell *service)
{
    mIpcService = AI_IPC::createIpcService("unix:path=/var/run/dbus/system_bus_socket", "com.sky.dobby.thunder");

    if (!mIpcService)
    {
        LOGERR("Failed to create IPC service");
        return("Failed to create IPC service");
    }
    else
    {
        // Start the IPCService which kicks off the event dispatcher thread
        mIpcService->start();
    }

    // Create a DobbyProxy remote service that wraps up the dbus API
    // calls to the Dobby daemon
    mDobbyProxy = std::make_shared<DobbyProxy>(mIpcService, DOBBY_SERVICE, DOBBY_OBJECT);

    // Register a state change event listener
    mEventListenerId = mDobbyProxy->registerListener(stateListener, static_cast<const void*>(this));

    return string();
}

void OCIContainer::Deinitialize(PluginHost::IShell *service)
{
    mDobbyProxy->unregisterListener(mEventListenerId);
    Unregister("listContainers");
    Unregister("getContainerState");
    Unregister("getContainerInfo");
    Unregister("startContainer");
    Unregister("startContainerFromDobbySpec");
    Unregister("stopContainer");
    Unregister("pauseContainer");
    Unregister("resumeContainer");
    Unregister("executeCommand");
}

string OCIContainer::Information() const
{
    // No additional info to report.
    return string();
}

// Begin Thunder Methods

/**
 * @brief Get all the currently running containers from Dobby
 *
 * @param[in]  parameters   No params processed.
 * @param[out] response     A list of running containers.
 *
 * @return                  A code indicating success.
 */
uint32_t OCIContainer::listContainers(const JsonObject &parameters, JsonObject &response)
{
    LOGINFO("List containers");

    // Get the running containers from Dobby
    std::list<std::pair<int32_t, std::string>> runningContainers;
    JsonObject containerJson;
    JsonArray containerArray;

    runningContainers = mDobbyProxy->listContainers();

    // Build the response if containers were found
    if (!runningContainers.empty())
    {
        for (const std::pair<int32_t, std::string>& c : runningContainers)
        {
            containerJson["Descriptor"] = c.first;
            containerJson["Id"] = c.second;

            containerArray.Add(containerJson);
        }
    }

    response["containers"] = containerArray;
    returnResponse(true);
}

/**
 * @brief Get the state of a known container
 *
 * @param[in]  parameters   Must include 'containerId' of the container whose state is requested.
 * @param[out] response     Container ID and container state.
 *
 * @return                  A code indicating success.
 */
uint32_t OCIContainer::getContainerState(const JsonObject &parameters, JsonObject &response)
{
    LOGINFO("Get container state");

    // Need to have an ID to find the state of a container
    returnIfStringParamNotFound(parameters, "containerId");
    std::string id = parameters["containerId"].String();

    int cd = GetContainerDescriptorFromId(id);
    if (cd < 0)
    {
        returnResponse(false);
    }

    std::string containerState;

    // We got a state back successfully, work out what that means in English
    switch (static_cast<IDobbyProxyEvents::ContainerState>(mDobbyProxy->getContainerState(cd)))
    {
    case IDobbyProxyEvents::ContainerState::Invalid:
        containerState = "Invalid";
        break;
    case IDobbyProxyEvents::ContainerState::Starting:
        containerState = "Starting";
        break;
    case IDobbyProxyEvents::ContainerState::Running:
        containerState = "Running";
        break;
    case IDobbyProxyEvents::ContainerState::Stopping:
        containerState = "Stopping";
        break;
    case IDobbyProxyEvents::ContainerState::Paused:
        containerState = "Paused";
        break;
    case IDobbyProxyEvents::ContainerState::Stopped:
        containerState = "Stopped";
        break;
    default:
        containerState = "Unknown";
        break;
    }

    response["containerId"] = id;
    response["state"] = containerState;
    returnResponse(true);
}

/**
 * @brief Get statistics about a currently running container such as CPU and
 * GPU usage
 *
 * @param[in]  parameters   Must include 'containerId' of the container whose info is requested.
 * @param[out] response     Container information.
 *
 * @return                  A code indicating success.
 */
uint32_t OCIContainer::getContainerInfo(const JsonObject &parameters, JsonObject &response)
{
    LOGINFO("Get container info");

    // Need to have an ID to find the info of a container
    returnIfStringParamNotFound(parameters, "containerId");
    std::string id = parameters["containerId"].String();

    int cd = GetContainerDescriptorFromId(id);
    if (cd < 0)
    {
        returnResponse(false);
    }

    std::string containerInfoString = mDobbyProxy->getContainerInfo(cd);

    if (containerInfoString.empty())
    {
        LOGERR("Failed to get info for container %s", id.c_str());
        returnResponse(false);
    }

    // Dobby returns the container info as JSON, so parse it
    JsonObject containerInfoJson;
    WPEC::OptionalType<WPEJ::Error> error;
    if (!WPEJ::IElement::FromString(containerInfoString, containerInfoJson, error)) {
        LOGERR("Failed to parse Dobby Spec JSON due to: %s", WPEJ::ErrorDisplayMessage(error).c_str());
        returnResponse(false);
    }

    response["info"] = containerInfoJson;
    returnResponse(true);
}

/**
 * @brief Starts a container from an OCI bundle
 *
 * @param[in]  parameters   - Must include 'containerId' and 'bundlePath'.
 * @param[out] response     - Dobby descriptor of the started container.
 *
 * @return                  A code indicating success.
 */
uint32_t OCIContainer::startContainer(const JsonObject &parameters, JsonObject &response)
{
    LOGINFO("Start container from OCI bundle");

    // To start a container, we need a path to an OCI bundle and an ID for the container
    returnIfStringParamNotFound(parameters, "containerId");
    returnIfStringParamNotFound(parameters, "bundlePath");

    std::string id = parameters["containerId"].String();
    std::string bundlePath = parameters["bundlePath"].String();
    std::string command = parameters["command"].String();
    std::string westerosSocket = parameters["westerosSocket"].String();

    // Can be used to pass file descriptors to container construction.
    // Currently unsupported, see DobbyProxy::startContainerFromBundle().
    std::list<int> emptyList;

    int descriptor;
    // If no additional arguments, start the container
    if ((command == "null" || command.empty()) && (westerosSocket == "null" || westerosSocket.empty()))
    {
        descriptor = mDobbyProxy->startContainerFromBundle(id, bundlePath, emptyList);
    }
    else
    {
        // Dobby expects empty strings if values not set
        if (command == "null" || command.empty())
        {
            command = "";
        }
        if (westerosSocket == "null" || westerosSocket.empty())
        {
            westerosSocket = "";
        }
        descriptor = mDobbyProxy->startContainerFromBundle(id, bundlePath, emptyList, command, westerosSocket);
    }

    // startContainer returns -1 on failure
    if (descriptor <= 0)
    {
        LOGERR("Failed to start container - internal Dobby error.");
        returnResponse(false);
    }

    response["descriptor"] = descriptor;
    returnResponse(true);
}

/**
 * @brief Starts a container using a Dobby spec file
 *
 * Provides legacy support for starting containers from a Dobby spec string.
 *
 * @param[in]  parameters   Must include 'containerId' and 'dobbySpec' of container to start.
 * @param[out] response     Dobby descriptor of the started container.
 *
 * @return                  A code indicating success.
 */
uint32_t OCIContainer::startContainerFromDobbySpec(const JsonObject &parameters,
                                                   JsonObject &response)
{
    LOGINFO("Start container from Dobby spec");

    // To start a container, we need a Dobby spec and an ID for the container
    returnIfStringParamNotFound(parameters, "containerId");
    std::string id = parameters["containerId"].String();
    JsonObject dobbySpec = parameters["dobbySpec"].Object();
    std::string command = parameters["command"].String();
    std::string westerosSocket = parameters["westerosSocket"].String();

    std::string specString;
    if (!WPEFramework::Core::JSON::IElement::ToString(dobbySpec, specString))
    {
        LOGERR("Failed to convert Dobby spec to string");
        returnResponse(false);
    }

    // Can be used to pass file descriptors to container construction.
    // Currently unsupported, see DobbyProxy::startContainerFromSpec().
    std::list<int> emptyList;

    int descriptor;
    // If no additional arguments, start the container
    if ((command == "null" || command.empty()) && (westerosSocket == "null" || westerosSocket.empty()))
    {
        descriptor = mDobbyProxy->startContainerFromSpec(id, specString, emptyList);
    }
    else
    {
        // Dobby expects empty strings if values not set
        if (command == "null" || command.empty())
        {
            command = "";
        }
        if (westerosSocket == "null" || westerosSocket.empty())
        {
            westerosSocket = "";
        }
        descriptor = mDobbyProxy->startContainerFromSpec(id, specString, emptyList, command, westerosSocket);
    }

    // startContainer returns -1 on failure
    if (descriptor <= 0)
    {
        LOGERR("Failed to start container - internal Dobby error.");
        returnResponse(false);
    }

    response["descriptor"] = descriptor;
    returnResponse(true);
}

/**
 * @brief Stop a currently running container
 *
 * To force stop a container, for example in case of a paused container, use 'force' = true.
 *
 * @param[in]  parameters   Must include 'containerId' of container to stop and optionally
 *                          'force'.
 * @param[out] response     Success.
 *
 * @return                  A code indicating success.
 */
uint32_t OCIContainer::stopContainer(const JsonObject &parameters, JsonObject &response)
{
    LOGINFO("Stop container");

    // Need to have an ID to stop
    returnIfStringParamNotFound(parameters, "containerId");
    std::string id = parameters["containerId"].String();

    int cd = GetContainerDescriptorFromId(id);
    if (cd < 0)
    {
        returnResponse(false);
    }

    bool forceStop = parameters["force"].Boolean();
    LOGINFO("Force stop is %d", forceStop);

    bool stoppedSuccessfully = mDobbyProxy->stopContainer(cd, forceStop);

    if (!stoppedSuccessfully)
    {
        LOGERR("Failed to stop container - internal Dobby error.");
        returnResponse(false);
    }

    returnResponse(true);
}

/**
 * @brief Pause a currently running container
 *
 * @param[in]  parameters   Must include 'containerId' of container to pause.
 * @param[out] response     Success.
 *
 * @return                  A code indicating success.
 */
uint32_t OCIContainer::pauseContainer(const JsonObject &parameters, JsonObject &response)
{
    LOGINFO("Pause container");

    // Need to have an ID to pause
    returnIfStringParamNotFound(parameters, "containerId");
    std::string id = parameters["containerId"].String();

    int cd = GetContainerDescriptorFromId(id);
    if (cd < 0)
    {
        returnResponse(false);
    }

    bool pausedSuccessfully = mDobbyProxy->pauseContainer(cd);

    if (!pausedSuccessfully)
    {
        LOGERR("Failed to pause container - internal Dobby error.");
        returnResponse(false);
    }

    returnResponse(true);
}

/**
 * @brief Resume a paused container
 *
 * @param[in]  parameters   Must include 'containerId' of container to resume.
 * @param[out] response     Success.
 *
 * @return                  A code indicating success.
 */
uint32_t OCIContainer::resumeContainer(const JsonObject &parameters, JsonObject &response)
{
    LOGINFO("Resume container");

    // Need to have an ID to resume
    returnIfStringParamNotFound(parameters, "containerId");
    std::string id = parameters["containerId"].String();

    int cd = GetContainerDescriptorFromId(id);
    if (cd < 0)
    {
        returnResponse(false);
    }

    bool resumedSuccessfully = mDobbyProxy->resumeContainer(cd);

    if (!resumedSuccessfully)
    {
        LOGERR("Failed to resume container - internal Dobby error.");
        returnResponse(false);
    }

    returnResponse(true);
}

/**
 * @brief Execute a command in a container.
 *
 * @param[in]  parameters   Must include 'containerId' and 'command' to execute. Optionally
 *                          'options' can be added for global options for "crun exec".
 * @param[out] response     Success.
 *
 * @return                  A code indicating success.
 */
uint32_t OCIContainer::executeCommand(const JsonObject &parameters, JsonObject &response)
{
    LOGINFO("Execute command inside container");

    // Need to have an ID and a command to execute a command
    returnIfStringParamNotFound(parameters, "containerId");
    returnIfStringParamNotFound(parameters, "command");

    std::string id = parameters["containerId"].String();

    int cd = GetContainerDescriptorFromId(id);
    if (cd < 0)
    {
        returnResponse(false);
    }

    std::string options = parameters["options"].String();
    std::string command = parameters["command"].String();

    bool executedSuccessfully = mDobbyProxy->execInContainer(cd, options, command);

    if (!executedSuccessfully)
    {
        LOGERR("Failed to execute command in container - internal Dobby error.");
        returnResponse(false);
    }

    returnResponse(true);
}


/**
 * @brief Send an event notifying that a container has started.
 *
 * @param descriptor    Container descriptor.
 * @param name          Container name.
 */
void OCIContainer::onContainerStarted(int32_t descriptor, const std::string& name)
{
    JsonObject params;
    params["descriptor"] = std::to_string(descriptor);
    params["name"] = name;
    sendNotify("onContainerStarted", params);
}

/**
 * @brief Send an event notifying that a container has stopped.
 *
 * @param descriptor    Container descriptor.
 * @param name          Container name.
 */
void OCIContainer::onContainerStopped(int32_t descriptor, const std::string& name)
{
    JsonObject params;
    params["descriptor"] = std::to_string(descriptor);
    params["name"] = name;
    sendNotify("onContainerStopped", params);
}

// End Thunder methods


// Begin Internal Methods

/**
 * @brief Converts the OCI container ID into the internal Dobby descriptor int
 *
 * Will only return a value if Dobby knows about the running container
 * (e.g. the container was started by Dobby, not manually using the OCI runtime).
 *
 * @param containerId The container ID used by the OCI runtime - not the Dobby descriptor
 *
 * @return Descriptor value
 */
const int OCIContainer::GetContainerDescriptorFromId(const std::string& containerId)
{
    const std::list<std::pair<int32_t, std::string>> containers = mDobbyProxy->listContainers();

    for (const std::pair<int32_t, std::string>& container : containers)
    {
        char strDescriptor[32];
        sprintf(strDescriptor, "%d", container.first);

        if ((containerId == strDescriptor) || (containerId == container.second))
        {
            return container.first;
        }
    }

    LOGERR("Failed to find container %s", containerId.c_str());
    return -1;
}

/**
 * @brief Callback listener for state change events.
 *
 * @param descriptor Container descriptor
 * @param name       Container name
 * @param state      Container state
 * @param _this      Callback parameters, or in this case, the pointer to 'this'
 */
const void OCIContainer::stateListener(int32_t descriptor, const std::string& name, IDobbyProxyEvents::ContainerState state, const void* _this)
{
    JsonObject params;
    params["descriptor"] = std::to_string(descriptor);
    params["name"] = name;

    // Cast const void* back to OCIContainer* type to get 'this'
    OCIContainer* __this = const_cast<OCIContainer*>(reinterpret_cast<const OCIContainer*>(_this));

    if (state == IDobbyProxyEvents::ContainerState::Running)
    {
        __this->onContainerStarted(descriptor, name);
    }
    else if (state == IDobbyProxyEvents::ContainerState::Stopped)
    {
        __this->onContainerStopped(descriptor, name);
    }
    else
    {
        LOGINFO("Received an unknown state event for container '%s'.", name.c_str());
    }
}
// End Internal methods

} // namespace Plugin
} // namespace WPEFramework
