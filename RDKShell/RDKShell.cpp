#include "RDKShell.h"
#include <string>
#include <iostream>
#include <mutex>
#include <rdkshell/compositorcontroller.h>
#include <rdkshell/rdkshell.h>
#include <rdkshell/linuxkeys.h>
#include <thread>

const short WPEFramework::Plugin::RDKShell::API_VERSION_NUMBER_MAJOR = 1;
const short WPEFramework::Plugin::RDKShell::API_VERSION_NUMBER_MINOR = 0;
const string WPEFramework::Plugin::RDKShell::SERVICE_NAME = "org.rdk.RDKShell";
//methods
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_MOVE_TO_FRONT = "moveToFront";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_MOVE_TO_BACK = "moveToBack";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_MOVE_BEHIND = "moveBehind";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_SET_FOCUS = "setFocus";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_KILL = "kill";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_ADD_KEY_INTERCEPT = "addKeyIntercept";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_REMOVE_KEY_INTERCEPT = "removeKeyIntercept";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_GET_SCREEN_RESOLUTION = "getScreenResolution";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_SET_SCREEN_RESOLUTION = "setScreenResolution";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_CREATE_DISPLAY = "createDisplay";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_GET_CLIENTS = "getClients";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_GET_Z_ORDER = "getZOrder";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_GET_BOUNDS = "getBounds";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_SET_BOUNDS = "setBounds";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_GET_VISIBILITY = "getVisibility";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_SET_VISIBILITY = "setVisibility";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_GET_OPACITY = "getOpacity";
const string WPEFramework::Plugin::RDKShell::RDKSHELL_METHOD_SET_OPACITY = "setOpacity";

using namespace std;
using namespace RdkShell;
extern int gCurrentFramerate;

namespace WPEFramework {
    namespace Plugin {

        uint32_t getKeyFlag(std::string modifier)
        {
          uint32_t flag = 0;
          if (0 == modifier.compare("ctrl"))
          {
            flag = RDKSHELL_FLAGS_CONTROL;
          }
          else if (0 == modifier.compare("shift"))
          {
            flag = RDKSHELL_FLAGS_SHIFT;
          }
          else if (0 == modifier.compare("alt"))
          {
            flag = RDKSHELL_FLAGS_ALT;
          }
          return flag;
        }

        SERVICE_REGISTRATION(RDKShell, 1, 0);

        RDKShell* RDKShell::_instance = nullptr;
        std::mutex gRdkShellMutex;

        static std::thread shellThread;

        void RDKShell::MonitorClients::StateChange(PluginHost::IShell* service)
        {
            if (service)
            {
                PluginHost::IShell::state currentState(service->State());
                if (currentState == PluginHost::IShell::ACTIVATION)
                {
                   std::string configLine = service->ConfigLine();
                   if (configLine.empty())
                   {
                       return;
                   }
                   JsonObject serviceConfig = JsonObject(configLine.c_str());
                   if (serviceConfig.HasLabel("clientidentifier"))
                   {
                       std::string clientidentifier = serviceConfig["clientidentifier"].String();
                       gRdkShellMutex.lock();
                       RdkShell::CompositorController::createDisplay(service->Callsign(), clientidentifier);
                       gRdkShellMutex.unlock();
                   }
                }
                else if (currentState == PluginHost::IShell::ACTIVATED && service->Callsign() == WPEFramework::Plugin::RDKShell::SERVICE_NAME)
                {
                    PluginHost::ISubSystem* subSystems(service->SubSystems());
                    if (subSystems != nullptr)
                    {
                        subSystems->Set(PluginHost::ISubSystem::PLATFORM, nullptr);
                        subSystems->Set(PluginHost::ISubSystem::GRAPHICS, nullptr);
                        subSystems->Release();
                    }
                }
                else if (currentState == PluginHost::IShell::DEACTIVATED)
                {
                    std::string configLine = service->ConfigLine();
                    if (configLine.empty())
                    {
                        return;
                    }
                    JsonObject serviceConfig = JsonObject(configLine.c_str());
                    if (serviceConfig.HasLabel("clientidentifier"))
                    {
                        std::string clientidentifier = serviceConfig["clientidentifier"].String();
                        gRdkShellMutex.lock();
                        RdkShell::CompositorController::kill(clientidentifier);
                        gRdkShellMutex.unlock();
                    }
                }
            }
        }

        RDKShell::RDKShell()
                : AbstractPlugin(), mClientsMonitor(Core::Service<MonitorClients>::Create<MonitorClients>(this))
        {
            LOGINFO("ctor");
            RDKShell::_instance = this;
            mRemoteShell = false;
            registerMethod(RDKSHELL_METHOD_MOVE_TO_FRONT, &RDKShell::moveToFrontWrapper, this);
            registerMethod(RDKSHELL_METHOD_MOVE_TO_BACK, &RDKShell::moveToBackWrapper, this);
            registerMethod(RDKSHELL_METHOD_MOVE_BEHIND, &RDKShell::moveBehindWrapper, this);
            registerMethod(RDKSHELL_METHOD_SET_FOCUS, &RDKShell::setFocusWrapper, this);
            registerMethod(RDKSHELL_METHOD_KILL, &RDKShell::killWrapper, this);
            registerMethod(RDKSHELL_METHOD_ADD_KEY_INTERCEPT, &RDKShell::addKeyInterceptWrapper, this);
            registerMethod(RDKSHELL_METHOD_REMOVE_KEY_INTERCEPT, &RDKShell::removeKeyInterceptWrapper, this);
            registerMethod(RDKSHELL_METHOD_GET_SCREEN_RESOLUTION, &RDKShell::getScreenResolutionWrapper, this);
            registerMethod(RDKSHELL_METHOD_SET_SCREEN_RESOLUTION, &RDKShell::setScreenResolutionWrapper, this);
            registerMethod(RDKSHELL_METHOD_CREATE_DISPLAY, &RDKShell::createDisplayWrapper, this);
            registerMethod(RDKSHELL_METHOD_GET_CLIENTS, &RDKShell::getClientsWrapper, this);
            registerMethod(RDKSHELL_METHOD_GET_Z_ORDER, &RDKShell::getZOrderWrapper, this);
            registerMethod(RDKSHELL_METHOD_GET_BOUNDS, &RDKShell::getBoundsWrapper, this);
            registerMethod(RDKSHELL_METHOD_SET_BOUNDS, &RDKShell::setBoundsWrapper, this);
            registerMethod(RDKSHELL_METHOD_GET_VISIBILITY, &RDKShell::getVisibilityWrapper, this);
            registerMethod(RDKSHELL_METHOD_SET_VISIBILITY, &RDKShell::setVisibilityWrapper, this);
            registerMethod(RDKSHELL_METHOD_GET_OPACITY, &RDKShell::getOpacityWrapper, this);
            registerMethod(RDKSHELL_METHOD_SET_OPACITY, &RDKShell::setOpacityWrapper, this);
        }

        RDKShell::~RDKShell()
        {
            LOGINFO("dtor");
            mClientsMonitor->Release();
            RDKShell::_instance = nullptr;
            mRemoteShell = false;
        }

        const string RDKShell::Initialize(PluginHost::IShell* service )
        {
            LOGINFO();
            
            shellThread = std::thread([]() {
                gRdkShellMutex.lock();
                RdkShell::initialize();
                gRdkShellMutex.unlock();
                while(true) {
                  const double maxSleepTime = (1000 / gCurrentFramerate) * 1000;
                  double startFrameTime = RdkShell::microseconds();
                  gRdkShellMutex.lock();
                  RdkShell::draw();
                  gRdkShellMutex.unlock();
                  double frameTime = (int)RdkShell::microseconds() - (int)startFrameTime;
                  int32_t sleepTimeInMs = gCurrentFramerate - frameTime;
                  if (frameTime < maxSleepTime)
                  {
                      int sleepTime = (int)maxSleepTime-(int)frameTime;
                      usleep(sleepTime);
                  }
                }
            });

            service->Register(mClientsMonitor);
            return "";
        }

        void RDKShell::Deinitialize(PluginHost::IShell* service)
        {
            LOGINFO();
            service->Unregister(mClientsMonitor);
        }

        string RDKShell::Information() const
        {
            return(string("{\"service\": \"") + SERVICE_NAME + string("\"}"));
        }

        // Registered methods (wrappers) begin
        uint32_t RDKShell::moveToFrontWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = true;
            if (!parameters.HasLabel("client"))
            {
                result = false;
                response["message"] = "please specify client";
            }
            if (result)
            {
                const string client  = parameters["client"].String();
                result = moveToFront(client);
                if (false == result) {
                  response["message"] = "failed to move front";
                }
            }
            returnResponse(result);
        }

        uint32_t RDKShell::moveToBackWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = true;
            if (!parameters.HasLabel("client"))
            {
                result = false;
                response["message"] = "please specify client";
            }
            if (result)
            {
                const string client  = parameters["client"].String();
                result = moveToBack(client);
                if (false == result) {
                  response["message"] = "failed to move back";
                }
            }
            returnResponse(result);
        }

        uint32_t RDKShell::moveBehindWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = true;
            if (!parameters.HasLabel("client"))
            {
                result = false;
                response["message"] = "please specify client";
            }
            if (!parameters.HasLabel("target"))
            {
                result = false;
                response["message"] = "please specify target";
            }
            if (result)
            {
                const string client  = parameters["client"].String();
                const string target  = parameters["target"].String();
                result = moveBehind(client, target);
                if (false == result) {
                  response["message"] = "failed to move behind";
                }
            }
            returnResponse(result);
        }

        uint32_t RDKShell::setFocusWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = true;
            if (!parameters.HasLabel("client"))
            {
                result = false;
                response["message"] = "please specify client";
            }
            if (result)
            {
                const string client  = parameters["client"].String();
                result = setFocus(client);
                if (false == result) {
                  response["message"] = "failed to set focus";
                }
            }
            returnResponse(result);
        }

        uint32_t RDKShell::killWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = true;
            if (!parameters.HasLabel("client"))
            {
                result = false;
                response["message"] = "please specify client";
            }
            if (result)
            {
                const string client  = parameters["client"].String();
                result = kill(client);
                if (false == result) {
                  response["message"] = "failed to kill client";
                }
            }
            returnResponse(result);
        }

        uint32_t RDKShell::addKeyInterceptWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = true;

            if (!parameters.HasLabel("keyCode"))
            {
                result = false;
                response["message"] = "please specify keyCode";
            }
            if (!parameters.HasLabel("client"))
            {
                result = false;
                response["message"] = "please specify client";
            }
            if (result)
            {
                //optional param?
                const JsonArray modifiers = parameters.HasLabel("modifiers") ? parameters["modifiers"].Array() : JsonArray();

                const uint32_t keyCode = parameters["keyCode"].Number();
                const string client  = parameters["client"].String();
                result = addKeyIntercept(keyCode, modifiers, client);
                if (false == result) {
                  response["message"] = "failed to add key intercept";
                }
            }
            returnResponse(result);
        }

        uint32_t RDKShell::removeKeyInterceptWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = true;

            if (!parameters.HasLabel("keyCode"))
            {
                result = false;
                response["message"] = "please specify keyCode";
            }
            if (!parameters.HasLabel("client"))
            {
                result = false;
                response["message"] = "please specify client";
            }
            if (result)
            {
                //optional param?
                const JsonArray modifiers = parameters.HasLabel("modifiers") ? parameters["modifiers"].Array() : JsonArray();

                const uint32_t keyCode = parameters["keyCode"].Number();
                const string client  = parameters["client"].String();
                result = removeKeyIntercept(keyCode, modifiers, client);
                if (false == result) {
                  response["message"] = "failed to remove key intercept";
                }
            }
            returnResponse(result);
        }

        uint32_t RDKShell::getScreenResolutionWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            UNUSED(parameters);

            bool result = true;
            if (!getScreenResolution(response))
            {
                response["message"] = "failed to get screen resolution";
                result = false;
            }
            returnResponse(result);
        }

        uint32_t RDKShell::setScreenResolutionWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = true;
            if (!parameters.HasLabel("w"))
            {
                result = false;
                response["message"] = "please specify w";
            }
            if (!parameters.HasLabel("h"))
            {
                result = false;
                response["message"] = "please specify h";
            }
            if (result)
            {
                const unsigned int w  = parameters["w"].Number();
                const unsigned int h  = parameters["h"].Number();

                result = setScreenResolution(w, h);
                // Just realized: we need one more string& param for the the error message in case setScreenResolution() fails internally
                // Also, we might not need a "non-wrapper" method at all, nothing prevents us from implementing it right here
                if (false == result) {
                  response["message"] = "failed to set screen resolution";
                }
            }
            returnResponse(result);
        }

        uint32_t RDKShell::createDisplayWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = true;
            if (!parameters.HasLabel("client"))
            {
                result = false;
                response["message"] = "please specify client";
            }
            if (result)
            {
                const string client  = parameters["client"].String();
                string displayName("");
                if (parameters.HasLabel("displayName"))
                {
                    displayName = parameters["displayName"].String();
                }
                result = createDisplay(client, displayName);
                if (false == result) {
                  response["message"] = "failed to create display";
                }
            }
            returnResponse(result);
        }

        uint32_t RDKShell::getClientsWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            UNUSED(parameters);
            bool result = true;
            JsonArray clients;
            if (!getClients(clients))
            {
                response["message"] = "failed to get clients";
                result = false;
            } else {
                response["clients"] = clients;
                result = true;
            }
            returnResponse(result);
        }

        uint32_t RDKShell::getZOrderWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            UNUSED(parameters);
            bool result = true;
            JsonArray clients;
            if (!getZOrder(clients))
            {
                response["message"] = "failed to get clients";
                result = false;
            } else {
                response["clients"] = clients;
                result = true;
            }
            returnResponse(result);
        }

        uint32_t RDKShell::getBoundsWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            UNUSED(parameters);
            bool result = true;
            if (!parameters.HasLabel("client"))
            {
                result = false;
                response["message"] = "please specify client";
            }
            if (result) {
              JsonObject bounds;
              const string client  = parameters["client"].String();
              if (!getBounds(client, bounds))
              {
                  response["message"] = "failed to get bounds";
                  result = false;
              } else {
                  response["bounds"] = bounds;
                  result = true;
              }
            }
            returnResponse(result);
        }

        uint32_t RDKShell::setBoundsWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = true;
            if (!parameters.HasLabel("client"))
            {
                result = false;
                response["message"] = "please specify client";
            }
            if (!parameters.HasLabel("x"))
            {
                result = false;
                response["message"] = "please specify x";
            }
            if (!parameters.HasLabel("y"))
            {
                result = false;
                response["message"] = "please specify y";
            }
            if (!parameters.HasLabel("w"))
            {
                result = false;
                response["message"] = "please specify w";
            }
            if (!parameters.HasLabel("h"))
            {
                result = false;
                response["message"] = "please specify h";
            }
            if (result)
            {
                const string client  = parameters["client"].String();
                const unsigned int x  = parameters["x"].Number();
                const unsigned int y  = parameters["y"].Number();
                const unsigned int w  = parameters["w"].Number();
                const unsigned int h  = parameters["h"].Number();

                result = setBounds(client, x, y, w, h);
                if (false == result) {
                  response["message"] = "failed to set bounds";
                }
            }
            returnResponse(result);
        }

        uint32_t RDKShell::getVisibilityWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = true;
            if (!parameters.HasLabel("client"))
            {
                result = false;
                response["message"] = "please specify client";
            }
            if (result)
            {
                const string client  = parameters["client"].String();
                bool visible;
                result = getVisibility(client, visible);
                if (false == result) {
                  response["message"] = "failed to get visibility";
                }
                else {
                  response["visible"] = visible;
                }
            }
            returnResponse(result);
        }

        uint32_t RDKShell::setVisibilityWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = true;
            if (!parameters.HasLabel("client"))
            {
                result = false;
                response["message"] = "please specify client";
            }
            if (!parameters.HasLabel("visible"))
            {
                result = false;
                response["message"] = "please specify visibility (visible = true/false)";
            }
            if (result)
            {
                const string client  = parameters["client"].String();
                const bool visible  = parameters["visible"].Boolean();

                result = setVisibility(client, visible);
                // Just realized: we need one more string& param for the the error message in case setScreenResolution() fails internally
                // Also, we might not need a "non-wrapper" method at all, nothing prevents us from implementing it right here

                if (false == result) {
                  response["message"] = "failed to set visibility";
                }
            }
            returnResponse(result);
        }

        uint32_t RDKShell::getOpacityWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            UNUSED(parameters);
            bool result = true;
            if (!parameters.HasLabel("client"))
            {
                result = false;
                response["message"] = "please specify client";
            }

            if (result)
            {
                const string client  = parameters["client"].String();
                unsigned int opacity;
                if (!getOpacity(client, opacity))
                {
                    response["message"] = "failed to get opacity";
                    result = false;
                } else {
                    response["opacity"] = opacity;
                    result = true;
                }
            }

            returnResponse(result);
        }

        uint32_t RDKShell::setOpacityWrapper(const JsonObject& parameters, JsonObject& response)
        {
            LOGINFOMETHOD();
            bool result = true;
            if (!parameters.HasLabel("client"))
            {
                result = false;
                response["message"] = "please specify client";
            }
            if (!parameters.HasLabel("opacity"))
            {
                result = false;
                response["message"] = "please specify opacity";
            }
            if (result)
            {
                const string client  = parameters["client"].String();
                const unsigned int  opacity  = parameters["opacity"].Number();

                result = setOpacity(client, opacity);
                if (false == result) {
                  response["message"] = "failed to set opacity";
                }
                // handle the result
            }
            returnResponse(result);
        }

        // Registered methods begin

        // Events begin
//        void RDKShell::onSomeEvent(JsonObject& url)
//        {
//            LOGINFO();
//            sendNotify(C_STR(RDKSHELL_EVENT_SOME_EVENT), url);
//        }
        // Events end

        // Internal methods begin
        bool RDKShell::moveToFront(const string& client)
        {
            bool ret = false;
            gRdkShellMutex.lock();
            ret = CompositorController::moveToFront(client);
            gRdkShellMutex.unlock();
            return ret;
        }

        bool RDKShell::moveToBack(const string& client)
        {
            bool ret = false;
            gRdkShellMutex.lock();
            ret = CompositorController::moveToBack(client);
            gRdkShellMutex.unlock();
            return ret;
        }

        bool RDKShell::moveBehind(const string& client, const string& target)
        {
            bool ret = false;
            gRdkShellMutex.lock();
            ret = CompositorController::moveBehind(client, target);
            gRdkShellMutex.unlock();
            return ret;
        }

        bool RDKShell::setFocus(const string& client)
        {
            bool ret = false;
            gRdkShellMutex.lock();
            ret = CompositorController::setFocus(client);
            gRdkShellMutex.unlock();
            return ret;
        }

        bool RDKShell::kill(const string& client)
        {
            bool ret = false;
            gRdkShellMutex.lock();
            ret = CompositorController::kill(client);
            gRdkShellMutex.unlock();
            return ret;
        }

        bool RDKShell::addKeyIntercept(const uint32_t& keyCode, const JsonArray& modifiers, const string& client)
        {
            uint32_t flags = 0;
            for (int i=0; i<modifiers.Length(); i++) {
              flags |= getKeyFlag(modifiers[i].String());
            }
            bool ret = false;
            gRdkShellMutex.lock();
            ret = CompositorController::addKeyIntercept(client, keyCode, flags);
            gRdkShellMutex.unlock();
            return ret;
        }

        bool RDKShell::removeKeyIntercept(const uint32_t& keyCode, const JsonArray& modifiers, const string& client)
        {
            uint32_t flags = 0;
            for (int i=0; i<modifiers.Length(); i++) {
              flags |= getKeyFlag(modifiers[i].String());
            }
            bool ret = false;
            gRdkShellMutex.lock();
            ret = CompositorController::removeKeyIntercept(client, keyCode, flags);
            gRdkShellMutex.unlock();
            return ret;
        }

        bool RDKShell::getScreenResolution(JsonObject& out)
        {
            unsigned int width=0,height=0;
            bool ret = false;
            gRdkShellMutex.lock();
            ret = CompositorController::getScreenResolution(width, height);
            gRdkShellMutex.unlock();
            if (true == ret) {
              out["w"] = width;
              out["h"] = height;
              return true;
            }
            return false;
        }

        bool RDKShell::setScreenResolution(const unsigned int w, const unsigned int h)
        {
            bool ret = false;
            gRdkShellMutex.lock();
            ret = CompositorController::setScreenResolution(w, h);
            gRdkShellMutex.unlock();
            return ret;
        }

        bool RDKShell::createDisplay(const string& client, const string& displayName)
        {
            bool ret = false;
            gRdkShellMutex.lock();
            ret = CompositorController::createDisplay(client, displayName);
            gRdkShellMutex.unlock();
            return ret;
        }

        bool RDKShell::getClients(JsonArray& clients)
        {
            std::vector<std::string> clientList;
            gRdkShellMutex.lock();
            CompositorController::getClients(clientList);
            gRdkShellMutex.unlock();
            for (size_t i=0; i<clientList.size(); i++) {
              clients.Add(clientList[i]);
            }
            return true;
        }

        bool RDKShell::getZOrder(JsonArray& clients)
        {
            std::vector<std::string> zOrderList;
            gRdkShellMutex.lock();
            CompositorController::getZOrder(zOrderList);
            gRdkShellMutex.unlock();
            for (size_t i=0; i<zOrderList.size(); i++) {
              clients.Add(zOrderList[i]);
            }
            return true;
        }

        bool RDKShell::getBounds(const string& client, JsonObject& bounds)
        {
            unsigned int x=0,y=0,width=0,height=0;
            bool ret = false;
            gRdkShellMutex.lock();
            ret = CompositorController::getBounds(client, x, y, width, height);
            gRdkShellMutex.unlock();
            if (true == ret) {
              bounds["x"] = x;
              bounds["y"] = y;
              bounds["w"] = width;
              bounds["h"] = height;
              return true;
            }
            return false;
        }

        bool RDKShell::setBounds(const std::string& client, const unsigned int x, const unsigned int y, const unsigned int w, const unsigned int h)
        {
            bool ret = false;
            gRdkShellMutex.lock();
            ret = CompositorController::setBounds(client, x, y, w, h);
            gRdkShellMutex.unlock();
            return ret;
        }

        bool RDKShell::getVisibility(const string& client, bool& visible)
        {
            bool ret = false;
            gRdkShellMutex.lock();
            ret = CompositorController::getVisibility(client, visible);
            gRdkShellMutex.unlock();
            return ret;
        }

        bool RDKShell::setVisibility(const string& client, const bool visible)
        {
            bool ret = false;
            gRdkShellMutex.lock();
            ret = CompositorController::setVisibility(client, visible);
            gRdkShellMutex.unlock();
            return ret;
        }

        bool RDKShell::getOpacity(const string& client, unsigned int& opacity)
        {
            bool ret = false;
            gRdkShellMutex.lock();
            ret = CompositorController::getOpacity(client, opacity);
            gRdkShellMutex.unlock();
            return ret;
        }

        bool RDKShell::setOpacity(const string& client, const unsigned int opacity)
        {
            bool ret = false;
            gRdkShellMutex.lock();
            ret = CompositorController::setOpacity(client, opacity);
            gRdkShellMutex.unlock();
            return ret;
        }
        // Internal methods end
    } // namespace Plugin
} // namespace WPEFramework
