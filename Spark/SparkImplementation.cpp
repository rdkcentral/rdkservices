/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2020 RDK Management
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
 
#include "Module.h"
#include <interfaces/IMemory.h>
#include <interfaces/IBrowser.h>

#include <fstream>
#include <pxFont.h>
#include <pxContext.h>
#include <pxEventLoop.h>
#include <pxScene2d.h>
#include <pxWindow.h>
#include <pxTimer.h>
#include <rtPathUtils.h>
#include <rtScript.h>
#include <rtUrlUtils.h>

#include <pxUtil.h>
#include <rtSettings.h>

pxContext context;
extern rtScript script;
extern bool gDirtyRectsEnabled;
extern rtThreadQueue* gUIThreadQueue;
extern bool topSparkView;

namespace WPEFramework {
namespace Plugin {

    class SparkImplementation : public Exchange::IBrowser, public PluginHost::IStateControl {
    private:
        class Config : public Core::JSON::Container {
        private:
            Config(const Config&);
            Config& operator=(const Config&);

        public:
            Config()
                : Core::JSON::Container()
                , Url()
                , Width(1280)
                , Height(720)
                , AnimationFPS(60)
                , ClientIdentifier()
                , EGLProvider(_T("/usr/lib/libEGL.so"))
            {
                Add(_T("url"), &Url);
                Add(_T("width"), &Width);
                Add(_T("height"), &Height);
                Add(_T("animationfps"), &AnimationFPS);
                Add(_T("egl"), &EGLProvider);
                Add(_T("clientidentifier"), &ClientIdentifier);
            }
            ~Config() {}

        public:
            Core::JSON::String Url;
            Core::JSON::DecUInt32 Width;
            Core::JSON::DecUInt32 Height;
            Core::JSON::DecUInt8 AnimationFPS;
            Core::JSON::String ClientIdentifier;
            Core::JSON::String EGLProvider;
        };

       class NotificationSink : public Core::Thread {
        private:
            NotificationSink() = delete;
            NotificationSink(const NotificationSink&) = delete;
            NotificationSink& operator=(const NotificationSink&) = delete;

        public:
            NotificationSink(SparkImplementation& parent)
                : _parent(parent)
                , _waitTime(0)
                , _command(PluginHost::IStateControl::SUSPEND)
            {
            }
            virtual ~NotificationSink()
            {
                Block();
                Wait(Thread::STOPPED | Thread::BLOCKED, Core::infinite);
            }

        public:
            void RequestForStateChange(const PluginHost::IStateControl::command command)
            {

                _command = command;

                Run();
            }

        private:
            virtual uint32_t Worker()
            {

                bool success = false;

                if ((IsRunning() == true) && (success == false)) {

                    success = _parent.RequestForStateChange(_command);
                }

                Block();

                _parent.StateChangeCompleted(success, _command);

                return (Core::infinite);
            }

        private:
            SparkImplementation& _parent;
            uint32_t _waitTime;
            PluginHost::IStateControl::command _command;
        };

        class SceneWindow : public Core::Thread, public pxWindow, public pxIViewContainer {
        private:
            SceneWindow(const SceneWindow&) = delete;
            SceneWindow& operator=(const SceneWindow&) = delete;

            struct ICommand {
                ICommand() : _refCount(1) {}
                virtual ~ICommand() {}

                void AddRef() { _refCount++; }
                void Release() { if (--_refCount == 0) { delete this; } }

                virtual void Execute() = 0;

            private:
                std::atomic<uint32_t> _refCount;
            };

            class SuspendImplementation : public ICommand {
            public:
                SuspendImplementation() = delete;
                SuspendImplementation(const SuspendImplementation&) = delete;
                SuspendImplementation& operator= (const SuspendImplementation&) = delete;

                SuspendImplementation(SceneWindow& parent, const bool value)
                    : _parent(parent)
                    , _suspend(value)
                    , _status(false)
                    , _signaled(false, true) {
                }
                virtual ~SuspendImplementation() {
                }

            public:
                virtual void Execute() override {

                    if (_parent._view != nullptr) {
                        pxScriptView* realClass = static_cast<pxScriptView*>(_parent._view.getPtr());

                        if (realClass != nullptr) {
                            rtValue r;
                            bool status = false;

                            if (_suspend == true) {
                                realClass->suspend(r, status);
                                TRACE(Trace::Information, (_T("Resume requested. Success: %s"), status ? _T("true") : _T("false")));
                            }
                            else {
                                realClass->resume(r, status);
                                TRACE(Trace::Information, (_T("Resume requested. Success: %s"), status ? _T("true") : _T("false")));
                            }
                            _status = status;
                        }
                    }
                    _signaled.SetEvent();
                }
                bool Wait(const uint32_t waitTime) {
                    bool status = false;

                    if (_signaled.Lock(waitTime) == Core::ERROR_NONE) {
                        status = _status;
                    }

                    return status;
                }

            private:
                SceneWindow& _parent;
                bool _suspend;
                bool _status;

                Core::Event _signaled;
            };

            class HideImplementation : public ICommand {
            public:
                HideImplementation() = delete;
                HideImplementation(const HideImplementation&) = delete;
                HideImplementation& operator= (const HideImplementation&) = delete;

                HideImplementation(SceneWindow& parent, const bool hide)
                    : _parent(parent)
                    , _hide(hide) {
                }
                virtual ~HideImplementation() {
                }

            public:
                virtual void Execute() override {
                    // JRJR TODO Why aren't these necessary for glut... pxCore bug
                    _parent.setVisibility(!_hide);
                }

            private:
                SceneWindow& _parent;
                bool _hide;
            };

        public:
            SceneWindow()
                : Core::Thread(Core::Thread::DefaultStackSize(), _T("Spark"))
                , _eventLoop()
                , _view(nullptr)
                , _width(~0)
                , _height(~0)
                , _animationFPS(~0)
                , _url()
                , _fullPath()
            {
            }
            virtual ~SceneWindow()
            {
                Stop();
                Quit();

                Wait(Thread::STOPPED | Thread::BLOCKED, Core::infinite);

                if (gUIThreadQueue)
                {
                    gUIThreadQueue->removeAllTasksForObject(this);
                }

                ASSERT (_view == nullptr);
            }

            uint32_t Configure(PluginHost::IShell* service) {
                uint32_t result = Core::ERROR_NONE;

                static constexpr TCHAR basePermissions[] = _T("sparkpermissions.conf");
                static constexpr TCHAR baseConfig[] = _T(".sparkSettings.json");
                static constexpr TCHAR baseFPS[] = _T(".sparkFps");

                Config config;
                config.FromString(service->ConfigLine());

                _url = config.Url.Value();
                _width = config.Width.Value();
                _height = config.Height.Value();
                _animationFPS = config.AnimationFPS.Value();

                // Check if there is a persistent config file, that will overrule the one that is in the ROM.
                string permissionsFile (service->PersistentPath() + basePermissions);
                string configFile (service->PersistentPath() + baseConfig);
                string fpsFile (service->PersistentPath() + baseFPS);

                if (Core::File(permissionsFile).Exists() != true) {
                    permissionsFile = service->DataPath() + basePermissions;
                }
                if (Core::File(configFile).Exists() != true) {
                    configFile = service->DataPath() + baseConfig;
                }
                if (Core::File(fpsFile).Exists() != true) {
                    fpsFile = service->DataPath() + baseFPS;
                }

                printf("Setting permissions file: %s\n", permissionsFile.c_str());
                if (Core::File(permissionsFile).Exists() == true) {
                    Core::SystemInfo::SetEnvironment(_T("SPARK_PERMISSIONS_ENABLED"), _T("true"));
                    Core::SystemInfo::SetEnvironment(_T("SPARK_PERMISSIONS_CONFIG"), permissionsFile);
                }
                if (Core::File(configFile).Exists() == true) {
                    rtSettings::instance()->loadFromFile(rtString(configFile.c_str()));

                    rtValue screenWidth, screenHeight;
                    if (RT_OK == rtSettings::instance()->value("screenWidth", screenWidth)) {
                        _width = screenWidth.toInt32();
                    }

                    if (RT_OK == rtSettings::instance()->value("screenHeight", screenHeight)) {
                        _height = screenHeight.toInt32();
                    }

                    rtValue dirtyRectsSetting;
                    if (RT_OK == rtSettings::instance()->value("enableDirtyRects", dirtyRectsSetting)) {
                        gDirtyRectsEnabled = dirtyRectsSetting.toString().compare("true") == 0;
                    }

                    rtValue optimizedUpdateSetting;
                    if (RT_OK == rtSettings::instance()->value("enableOptimizedUpdate", optimizedUpdateSetting)) {
                        bool enable = optimizedUpdateSetting.toString().compare("true") == 0;
                        pxScene2d::enableOptimizedUpdate(enable);
                    }
                }

                if (Core::File(fpsFile).Exists() == true) {
                    std::fstream fs(fpsFile, std::fstream::in);
                    uint32_t val = 0;
                    fs >> val;
                    if (val > 0) {
                        _animationFPS = val;
                    }
                }
                Core::SystemInfo::SetEnvironment(_T("NODE_PATH"), service->DataPath());
                Core::SystemInfo::SetEnvironment(_T("SPARK_PATH"), service->DataPath());
                Core::SystemInfo::SetEnvironment(_T("RT_EGL_PROVIDER"), config.EGLProvider.Value());
                Core::SystemInfo::SetEnvironment(_T("SCREEN_WIDTH"), std::to_string(_width));
                Core::SystemInfo::SetEnvironment(_T("SCREEN_HEIGHT"), std::to_string(_height));

                if (config.ClientIdentifier.IsSet() == true) {
                    string value(service->Callsign() + ',' + config.ClientIdentifier.Value());
                    Core::SystemInfo::SetEnvironment(_T("CLIENT_IDENTIFIER"), value);
                } else {
                    Core::SystemInfo::SetEnvironment(_T("CLIENT_IDENTIFIER"), service->Callsign());
                }

                Init();

                SetURL(_url);

                return result;
            }

            string GetURL() const {
                return (_url);
            }

            void SetURL(const string& url)
            {
                _url = url;

                Quit();

                Wait(Thread::STOPPED | Thread::BLOCKED, Core::infinite);

                if (url.empty() == true)
                {
                    _fullPath.clear();

                } else {
                    string prefix = "shell.js?url=";
                    TCHAR buffer[MAX_URL_SIZE + prefix.size()];
                    memset(buffer, 0, MAX_URL_SIZE + prefix.size());

                    Core::URL analyser(url.c_str());
                    uint16_t length = 0;

                    if (!prefix.empty()) {
                        strncpy(buffer, prefix.c_str(), prefix.size());
                    }
                    length = std::min(url.length(), sizeof(buffer) - prefix.size());

                    if (length >= (sizeof(buffer) - sizeof(prefix))) {

                        SYSLOG(Trace::Warning, (_T("URL size greater than 8000 bytes, so resetting url to browser.js")));
                        ::strcat(buffer, _T("browser.js"));
                    } else {
                        strncat(buffer, url.c_str(), length);
                    }

                    ENTERSCENELOCK()

                    _fullPath = buffer;

                    EXITSCENELOCK()

                    TRACE(Trace::Information, (_T("Request URL: %s"), url.c_str()));

                    Run();
                }
            }

            void Quit()
            { 
                Block();
                _eventLoop.exit();
            }

            void Hidden (const bool hidden) 
            {
                if (gUIThreadQueue)
                {
                    gUIThreadQueue->addTask(
                        windowThread,
                        nullptr,
                        static_cast<ICommand*>(new HideImplementation(*this, hidden)));
                }
            }

            bool Suspend(const bool suspend)
            {
                bool result = false;

                if (gUIThreadQueue)
                {
                    SuspendImplementation* command = new SuspendImplementation(*this, suspend);

                    command->AddRef();

                    gUIThreadQueue->addTask(
                        windowThread,
                        nullptr,
                        command);

                    result = command->Wait(Core::infinite);
                    command->Release();
                }

                return (result);
            }

            uint8_t AnimationFPS() const
            {
                return (_animationFPS);
            }

            virtual void onClose() override
            {
            }

        private:
            static void windowThread(void* context, void* data)
            {
                ICommand* command = reinterpret_cast<ICommand*>(data);
                command->Execute();
                command->Release();
            }
            virtual void* getInterface(const char* /* name */) override
            {
                return nullptr;
            }
            virtual void onCreate() override 
            {
            }
            virtual void invalidateRect(pxRect* r) override
            {
                pxWindow::invalidateRect(r);
            }
            virtual void onCloseRequest() override
            {
                ENTERSCENELOCK();
                if (_view != nullptr) {
                    _view->setViewContainer(nullptr);
                    _view->onCloseRequest();
                    _view = nullptr;

                    pxFontManager::clearAllFonts();
                    script.pump();
                    script.collectGarbage();
                }
                EXITSCENELOCK();
            }

            virtual void onAnimationTimer() override
            {
                ENTERSCENELOCK();
                if (_view != nullptr) {
                    _view->onUpdate(pxSeconds());
                }
                EXITSCENELOCK();
                script.pump();
            }

            virtual void onSize(int32_t w, int32_t h) override
            {
                ENTERSCENELOCK();
                if (_view != nullptr) {
                    _view->onSize(w, h);
                }
                EXITSCENELOCK();
            }

            virtual void onMouseDown(int32_t x, int32_t y, uint32_t flags) override
            {
                ENTERSCENELOCK();
                if (_view != nullptr) {
                    _view->onMouseDown(x, y, flags);
                }
                EXITSCENELOCK();
            }
            virtual void onMouseUp(int32_t x, int32_t y, uint32_t flags) override
            {
                ENTERSCENELOCK();
                if (_view != nullptr) {
                    _view->onMouseUp(x, y, flags);
                }
                EXITSCENELOCK();
            }

            virtual void onMouseLeave() override
            {
                ENTERSCENELOCK();
                if (_view != nullptr) {
                    _view->onMouseLeave();
                }
                EXITSCENELOCK();
            }

            virtual void onMouseMove(int32_t x, int32_t y) override
            {
                ENTERSCENELOCK();
                if (_view != nullptr) {
                    _view->onMouseMove(x, y);
                }
                EXITSCENELOCK();
            }

            virtual void onKeyDown(uint32_t keycode, uint32_t flags) override
            {
                ENTERSCENELOCK();
                if (_view != nullptr) {
                    _view->onKeyDown(keycode, flags);
                }
                EXITSCENELOCK();
            }

            virtual void onKeyUp(uint32_t keycode, uint32_t flags) override
            {
                ENTERSCENELOCK();
                if (_view != nullptr) {
                    _view->onKeyUp(keycode, flags);
                }
                EXITSCENELOCK();
            }

            virtual void onChar(uint32_t c) override
            {
                ENTERSCENELOCK();
                if (_view != nullptr) {
                    _view->onChar(c);
                }
                EXITSCENELOCK();
            }

            virtual void onDraw(pxSurfaceNative) override
            {
                context.updateRenderTick();
                ENTERSCENELOCK();
                if (_view != nullptr) {
                    _view->onDraw();
                }
                EXITSCENELOCK();
            }

            virtual void onMouseEnter() override
            {
                ENTERSCENELOCK();
                if (_view != nullptr) {
                    _view->onMouseEnter();
                }
                EXITSCENELOCK();
            }

            virtual void onFocus() override
            {
                ENTERSCENELOCK();
                if (_view != nullptr) {
                    _view->onFocus();
                }
                EXITSCENELOCK();
            }

            virtual void onBlur() override
            {
                ENTERSCENELOCK();
                if (_view != nullptr) {
                    _view->onBlur();
                }
                EXITSCENELOCK();
            }

            virtual void onScrollWheel(float dx, float dy) override
            {
                ENTERSCENELOCK();
                if (_view != nullptr) {
                    _view->onScrollWheel(dx, dy);
                }
                EXITSCENELOCK();
            }

            virtual void onDragDrop(int32_t x, int32_t y, int32_t type, const char* dropped) override
            {
                ENTERSCENELOCK();
                if (_view != nullptr) {
                    _view->onDragDrop(x, y, type, dropped);
                }
                EXITSCENELOCK();
            }

        private:
            virtual bool Initialize()
            {
                script.init();
                pxWindow::init(0, 0, _width, _height);

                context.init();
                TCHAR buffer[64];

                setAnimationFPS(_animationFPS);

                sprintf(buffer, "Spark: %s", PX_SCENE_VERSION);
                setTitle(buffer);

                return true;
            }
            virtual uint32_t Worker()
            {
                ENTERSCENELOCK()

                if (_fullPath.empty() == true) {
                    Block();
                }
                else {
                    topSparkView = true; // Set new Scene as the topest

                    pxScriptView *scriptView = new pxScriptView(_fullPath.c_str(),"javascript/node/v8");
                    _view = static_cast<pxViewRef> (scriptView);
                    _view->setViewContainer(this);
                    _view->onSize(_width, _height);
 
                    EXITSCENELOCK()

                    TRACE(Trace::Information, (_T("Showing URL: %s"), _fullPath.c_str()));

                    if (IsRunning() == true) {
                        _eventLoop.run();
                    }

                    ENTERSCENELOCK();

                    onCloseRequest();
                }

                EXITSCENELOCK()

                return (Core::infinite);
            }

        private:
            pxEventLoop _eventLoop;
            pxViewRef _view;
            uint32_t _width;
            uint32_t _height;
            uint8_t _animationFPS;
            string _url;
            string _fullPath;
        };

   private:
        static constexpr uint32_t MaxSuspendTime = 3000;

        SparkImplementation(const SparkImplementation&) = delete;
        SparkImplementation& operator=(const SparkImplementation&) = delete;

    public:
        SparkImplementation()
            : _adminLock()
            , _window()
            , _state(PluginHost::IStateControl::UNINITIALIZED)
            , _sparkClients()
            , _stateControlClients()
            , _sink(*this)
        {
        }

        virtual ~SparkImplementation()
        {
        }

        virtual uint32_t Configure(PluginHost::IShell* service)
        {
            uint32_t result = _window.Configure(service);

            _state = PluginHost::IStateControl::RESUMED;

            return (result);
        }
        virtual void SetURL(const string& URL) override {
            _window.SetURL(URL);
        }
        virtual string GetURL() const override {
            return (_window.GetURL());
        }
        virtual uint32_t GetFPS() const override {
            return (_window.AnimationFPS());
        }
        virtual void Hide(const bool hidden) {
            _window.Hidden(hidden);
        }
        virtual void Register(Exchange::IBrowser::INotification* sink)
        {
            _adminLock.Lock();

            // Make sure a sink is not registered multiple times.
            ASSERT(std::find(_sparkClients.begin(), _sparkClients.end(), sink) == _sparkClients.end());

            _sparkClients.push_back(sink);
            sink->AddRef();

            _adminLock.Unlock();
        }

        virtual void Unregister(Exchange::IBrowser::INotification* sink)
        {
            _adminLock.Lock();

            std::list<Exchange::IBrowser::INotification*>::iterator index(
                std::find(_sparkClients.begin(), _sparkClients.end(), sink));

            // Make sure you do not unregister something you did not register !!!
            ASSERT(index != _sparkClients.end());

            if (index != _sparkClients.end()) {
                (*index)->Release();
                _sparkClients.erase(index);
            }

            _adminLock.Unlock();
        }

        virtual void Register(PluginHost::IStateControl::INotification* sink)
        {
            _adminLock.Lock();

            // Make sure a sink is not registered multiple times.
            ASSERT(std::find(_stateControlClients.begin(), _stateControlClients.end(), sink) == _stateControlClients.end());

            _stateControlClients.push_back(sink);
            sink->AddRef();

            _adminLock.Unlock();
        }

        virtual void Unregister(PluginHost::IStateControl::INotification* sink)
        {
            _adminLock.Lock();

            std::list<PluginHost::IStateControl::INotification*>::iterator index(std::find(_stateControlClients.begin(), _stateControlClients.end(), sink));

            // Make sure you do not unregister something you did not register !!!
            ASSERT(index != _stateControlClients.end());

            if (index != _stateControlClients.end()) {
                (*index)->Release();
                _stateControlClients.erase(index);
            }

            _adminLock.Unlock();
        }

        virtual PluginHost::IStateControl::state State() const
        {
            return (_state);
        }

        virtual uint32_t Request(const PluginHost::IStateControl::command command)
        {
            uint32_t result = Core::ERROR_ILLEGAL_STATE;

            _adminLock.Lock();

            if (_state == PluginHost::IStateControl::UNINITIALIZED) {
                // Seems we are passing state changes before we reached an operational Spark.
                // Just move the state to what we would like it to be :-)
                _state = (command == PluginHost::IStateControl::SUSPEND ? PluginHost::IStateControl::SUSPENDED : PluginHost::IStateControl::RESUMED);
                result = Core::ERROR_NONE;
            }
            else {
                switch (command) {
                case PluginHost::IStateControl::SUSPEND:
                    if (_state == PluginHost::IStateControl::RESUMED) {
                        _sink.RequestForStateChange(PluginHost::IStateControl::SUSPEND);
                        result = Core::ERROR_NONE;
                    }
                    break;
                case PluginHost::IStateControl::RESUME:
                    if (_state == PluginHost::IStateControl::SUSPENDED) {
                        _sink.RequestForStateChange(PluginHost::IStateControl::RESUME);
                        result = Core::ERROR_NONE;
                    }
                    break;
                default:
                    break;
                }
            }

            _adminLock.Unlock();

            return result;
        }

        void StateChangeCompleted(bool success, const PluginHost::IStateControl::command request)
        {
            if (success) {
                switch (request) {
                case PluginHost::IStateControl::RESUME:

                    _adminLock.Lock();

                    if (_state != PluginHost::IStateControl::RESUMED) {
                        StateChange(PluginHost::IStateControl::RESUMED);
                    }

                    _adminLock.Unlock();
                    break;
                case PluginHost::IStateControl::SUSPEND:

                    _adminLock.Lock();

                    if (_state != PluginHost::IStateControl::SUSPENDED) {
                        StateChange(PluginHost::IStateControl::SUSPENDED);
                    }

                    _adminLock.Unlock();
                    break;
                default:
                    ASSERT(false);
                    break;
                }
            } else {
                StateChange(PluginHost::IStateControl::EXITED);
            }
        }

        BEGIN_INTERFACE_MAP(SparkImplementation)
            INTERFACE_ENTRY(Exchange::IBrowser)
            INTERFACE_ENTRY(PluginHost::IStateControl)
        END_INTERFACE_MAP

    private:

        inline bool RequestForStateChange(const PluginHost::IStateControl::command command)
        {
            bool result = false;

            switch (command) {
            case PluginHost::IStateControl::SUSPEND: {
                if (_window.Suspend(true) == true) {
                    result = true;
               }
                break;
            }
            case PluginHost::IStateControl::RESUME: {
                if (_window.Suspend(false) == true) {
                    result = true;
                }
                break;
            }
            default:
                ASSERT(false);
                break;
            }

            return result;
        }

        void StateChange(const PluginHost::IStateControl::state newState)
        {
            _adminLock.Lock();

            _state = newState;

            std::list<PluginHost::IStateControl::INotification*>::iterator index(_stateControlClients.begin());

            while (index != _stateControlClients.end()) {
                (*index)->StateChange(newState);
                index++;
            }

            _adminLock.Unlock();
        }
        mutable Core::CriticalSection _adminLock;
        SceneWindow _window;
        PluginHost::IStateControl::state _state;
        std::list<Exchange::IBrowser::INotification*> _sparkClients;
        std::list<PluginHost::IStateControl::INotification*> _stateControlClients;
        NotificationSink _sink;
    };

    SERVICE_REGISTRATION(SparkImplementation, 1, 0);

} /* namespace Plugin */

namespace Spark {

    class MemoryObserverImpl : public Exchange::IMemory {
    private:
        MemoryObserverImpl();
        MemoryObserverImpl(const MemoryObserverImpl&);
        MemoryObserverImpl& operator=(const MemoryObserverImpl&);

    public:
        MemoryObserverImpl(const uint32_t id)
            : _main(id == 0 ? Core::ProcessInfo().Id() : id)
        {
        }
        ~MemoryObserverImpl() {}

    public:
        virtual uint64_t Resident() const
        {
            return _main.Resident();
        }
        virtual uint64_t Allocated() const
        {
            return _main.Allocated();
        }
        virtual uint64_t Shared() const
        {
            return _main.Shared();
        }
        virtual uint8_t Processes() const { return (IsOperational() ? 1 : 0); }

        virtual const bool IsOperational() const
        {
            return _main.IsActive();
        }

        BEGIN_INTERFACE_MAP(MemoryObserverImpl)
        INTERFACE_ENTRY(Exchange::IMemory)
        END_INTERFACE_MAP

    private:
        Core::ProcessInfo _main;
    };

    Exchange::IMemory* MemoryObserver(const uint32_t PID)
    {
        return (Core::Service<MemoryObserverImpl>::Create<Exchange::IMemory>(PID));
    }
}
} // namespace
