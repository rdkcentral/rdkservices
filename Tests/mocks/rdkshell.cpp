#include "rdkshell.h"
#include <gmock/gmock.h>

#define RDKSHELL_FPS 40
int gCurrentFramerate = RDKSHELL_FPS;

RDKShellImpl* RDKShell::impl = nullptr;

RDKShell::RDKShell() {}

void RDKShell::setImpl(RDKShellImpl* newImpl)
{
    // Handles both resetting 'impl' to nullptr and assigning a new value to 'impl'
    EXPECT_TRUE ((nullptr == impl) || (nullptr == newImpl));
    impl = newImpl;
}


bool RDKShell::keyCodeFromWayland(uint32_t waylandKeyCode, uint32_t waylandFlags, uint32_t &mappedKeyCode, uint32_t &mappedFlags)
{
    EXPECT_NE(impl, nullptr);
    return impl->keyCodeFromWayland(waylandKeyCode, waylandFlags, mappedKeyCode, mappedFlags);
}

bool (*keyCodeFromWayland)(uint32_t waylandKeyCode, uint32_t waylandFlags, uint32_t &mappedKeyCode, uint32_t &mappedFlags)= &RDKShell::keyCodeFromWayland;

namespace RdkShell
{

 RdkShellEvent* RdkShellEventListener::impl = nullptr;
 RdkShellEventListener::RdkShellEventListener() {}

void RdkShellEventListener::setImpl(RdkShellEvent* newImpl)
{
    // Handles both resetting 'impl' to nullptr and assigning a new value to 'impl'
    EXPECT_TRUE ((nullptr == impl) || (nullptr == newImpl));
    impl = newImpl;
}

 void RdkShellEventListener::onApplicationLaunched(const std::string& client)
 {
    EXPECT_NE(impl, nullptr);
    impl->onApplicationLaunched(client);
 }
 void RdkShellEventListener::onApplicationConnected(const std::string& client)
 {
    EXPECT_NE(impl, nullptr);
    impl->onApplicationConnected(client);
 }
 void RdkShellEventListener::onApplicationDisconnected(const std::string& client)
 {
    EXPECT_NE(impl, nullptr);
    impl->onApplicationDisconnected(client);
 }
 void RdkShellEventListener::onApplicationTerminated(const std::string& client)
 {
    EXPECT_NE(impl, nullptr);
    impl->onApplicationTerminated(client);
 }
 void RdkShellEventListener::onApplicationFirstFrame(const std::string& client)
 {
    EXPECT_NE(impl, nullptr);
    impl->onApplicationFirstFrame(client);
 }
 void RdkShellEventListener::onApplicationSuspended(const std::string& client)
 {
    EXPECT_NE(impl, nullptr);
    impl->onApplicationSuspended(client);
 }
 void RdkShellEventListener::onApplicationResumed(const std::string& client)
 {
    EXPECT_NE(impl, nullptr);
    impl->onApplicationResumed(client);
 }
 void RdkShellEventListener::onApplicationActivated(const std::string& client)
 {
    EXPECT_NE(impl, nullptr);
    impl->onApplicationActivated(client);
 }
 void RdkShellEventListener::onUserInactive(const double minutes)
 {
    EXPECT_NE(impl, nullptr);
    impl->onUserInactive(minutes);
 }
 void RdkShellEventListener::onDeviceLowRamWarning(const int32_t freeKb, const int32_t availableKb, const int32_t usedSwapKb)
 {
    EXPECT_NE(impl, nullptr);
    impl->onDeviceLowRamWarning(freeKb, availableKb, usedSwapKb);
 }
 void RdkShellEventListener::onDeviceCriticallyLowRamWarning(const int32_t freeKb, const int32_t availableKb, const int32_t usedSwapKb)
 {
    EXPECT_NE(impl, nullptr);
    impl->onDeviceCriticallyLowRamWarning(freeKb, availableKb, usedSwapKb);
 }
 void RdkShellEventListener::onDeviceLowRamWarningCleared(const int32_t freeKb, const int32_t availableKb, const int32_t usedSwapKb)
 {
    EXPECT_NE(impl, nullptr);
    impl->onDeviceLowRamWarningCleared(freeKb, availableKb, usedSwapKb);
 }
 void RdkShellEventListener::onDeviceCriticallyLowRamWarningCleared(const int32_t freeKb, const int32_t availableKb, const int32_t usedSwapKb)
 {
    EXPECT_NE(impl, nullptr);
    impl->onDeviceCriticallyLowRamWarningCleared(freeKb, availableKb, usedSwapKb);
 }
 void RdkShellEventListener::onEasterEgg(const std::string& name, const std::string& actionJson)
 {
    EXPECT_NE(impl, nullptr);
    impl->onEasterEgg(name, actionJson);
 }
 void RdkShellEventListener::onPowerKey()
 {
    EXPECT_NE(impl, nullptr);
    impl->onPowerKey();
 }
 void RdkShellEventListener::onSizeChangeComplete(const std::string& client)
 {
    EXPECT_NE(impl, nullptr);
    impl->onSizeChangeComplete(client);
 }

LoggerImpl* Logger::impl = nullptr;
Logger::Logger() {}

void Logger::setImpl(LoggerImpl* newImpl)
{
    // Handles both resetting 'impl' to nullptr and assigning a new value to 'impl'
    EXPECT_TRUE ((nullptr == impl) || (nullptr == newImpl));
    impl = newImpl;
}

void Logger::enableFlushing(bool enable)
{
    EXPECT_NE(impl, nullptr);
    impl->enableFlushing(enable);
}
bool Logger::isFlushingEnabled()
{
    EXPECT_NE(impl, nullptr);
    return impl->isFlushingEnabled();
}

RdkShellApiImpl* RdkShellApi::impl = nullptr;

RdkShellApi::RdkShellApi() {}

void RdkShellApi::setImpl(RdkShellApiImpl* newImpl)
{
    // Handles both resetting 'impl' to nullptr and assigning a new value to 'impl'
    EXPECT_TRUE ((nullptr == impl) || (nullptr == newImpl));
    impl = newImpl;
}


void RdkShellApi::addEasterEgg(std::vector<RdkShellEasterEggKeyDetails>& details, std::string name, uint32_t timeout, std::string actionJson)
{
    EXPECT_NE(impl, nullptr);
    impl->addEasterEgg(details, name, timeout, actionJson);
}
void RdkShellApi::removeEasterEgg(std::string name)
{
    EXPECT_NE(impl, nullptr);
    impl->removeEasterEgg(name);
}
void RdkShellApi::getEasterEggs(std::vector<RdkShellEasterEggDetails>& easterEggs)
{
    EXPECT_NE(impl, nullptr);
    impl->getEasterEggs(easterEggs);
}
void RdkShellApi::initialize()
{
    EXPECT_NE(impl, nullptr);
    impl->initialize();
}
void RdkShellApi::update()
{
    EXPECT_NE(impl, nullptr);
    impl->update();
}
void RdkShellApi::draw()
{
    EXPECT_NE(impl, nullptr);
    impl->draw();
}
void RdkShellApi::deinitialize()
{
    EXPECT_NE(impl, nullptr);
    impl->deinitialize();
}
double RdkShellApi::seconds()
{
    EXPECT_NE(impl, nullptr);
    return impl->seconds();
}
double RdkShellApi::milliseconds()
{
    EXPECT_NE(impl, nullptr);
    return impl->milliseconds();
}
double RdkShellApi::microseconds()
{
    EXPECT_NE(impl, nullptr);
    return impl->microseconds();
}
bool RdkShellApi::systemRam(uint32_t& freeKb, uint32_t& totalKb, uint32_t& availableKb, uint32_t& usedSwapKb)
{
    EXPECT_NE(impl, nullptr);
    return impl->systemRam(freeKb, totalKb, availableKb, usedSwapKb);
}
void RdkShellApi::setMemoryMonitor(std::map<std::string, RdkShellData> &configuration)
{
    EXPECT_NE(impl, nullptr);
    impl->setMemoryMonitor(configuration);
}

void (*addEasterEgg)(std::vector<RdkShellEasterEggKeyDetails>&, std::string, uint32_t, std::string) = &RdkShellApi::addEasterEgg;
void (*removeEasterEgg)(std::string) = &RdkShellApi::removeEasterEgg;
void (*getEasterEggs)(std::vector<RdkShellEasterEggDetails>&) = &RdkShellApi::getEasterEggs;
void (*initialize)() = &RdkShellApi::initialize;
void (*update)() = &RdkShellApi::update;
void (*draw)() = &RdkShellApi::draw;
void (*deinitialize)() = &RdkShellApi::deinitialize;
double (*seconds)() = &RdkShellApi::seconds;
double (*milliseconds)() = &RdkShellApi::milliseconds;
double (*microseconds)() = &RdkShellApi::microseconds;
bool (*systemRam)(uint32_t&, uint32_t&, uint32_t&, uint32_t&) = &RdkShellApi::systemRam;
void (*setMemoryMonitor)(std::map<std::string, RdkShellData>&) = &RdkShellApi::setMemoryMonitor;


CompositorControllerImpl* CompositorController::impl = nullptr;

CompositorController::CompositorController() {}

void CompositorController::setImpl(CompositorControllerImpl* newImpl)
{
    // Handles both resetting 'impl' to nullptr and assigning a new value to 'impl'
    EXPECT_TRUE ((nullptr == impl) || (nullptr == newImpl));
    impl = newImpl;
}

bool CompositorController::setVisibility(const std::string& client, const bool visible)
{
    return impl->setVisibility(client, visible);
}
bool CompositorController::moveToFront(const std::string& client)
{
    return impl->moveToFront(client);
}
bool CompositorController::moveToBack(const std::string& client)
{
    return impl->moveToBack(client);
}
bool CompositorController::moveBehind(const std::string& client, const std::string& target)
{
    return impl->moveBehind(client, target);
}
bool CompositorController::setFocus(const std::string& client)
{
    return impl->setFocus(client);
}
bool CompositorController::getFocused(std::string& client)
{
    return impl->getFocused(client);
}
 bool CompositorController::kill(const std::string& client)
{
    return impl->kill(client);
}
bool CompositorController::addKeyIntercept(const std::string& client, const uint32_t& keyCode, const uint32_t& flags)
{
    return impl->addKeyIntercept(client, keyCode, flags);
}
bool CompositorController::removeKeyIntercept(const std::string& client, const uint32_t& keyCode, const uint32_t& flags)
{
     return impl->removeKeyIntercept(client, keyCode, flags);
}
bool CompositorController::addKeyListener(const std::string& client, const uint32_t& keyCode, const uint32_t& flags, std::map<std::string, RdkShellData> &listenerProperties)
{
    return impl->addKeyListener(client, keyCode, flags, listenerProperties);
}
bool CompositorController::addNativeKeyListener(const std::string& client, const uint32_t& keyCode, const uint32_t& flags, std::map<std::string, RdkShellData> &listenerProperties)
{
    return impl->addNativeKeyListener(client, keyCode, flags, listenerProperties);
}
bool CompositorController::removeKeyListener(const std::string& client, const uint32_t& keyCode, const uint32_t& flags)
{
    return impl->removeKeyListener(client, keyCode, flags);
}
 bool CompositorController::removeNativeKeyListener(const std::string& client, const uint32_t& keyCode, const uint32_t& flags)
{
    return impl->removeNativeKeyListener(client, keyCode, flags);
}
bool CompositorController::addKeyMetadataListener(const std::string& client)
{
    return impl->addKeyMetadataListener(client);
}
bool CompositorController::removeKeyMetadataListener(const std::string& client)
{
    return impl->removeKeyMetadataListener(client);
}
bool CompositorController::injectKey(const uint32_t& keyCode, const uint32_t& flags)
{
    return impl->injectKey(keyCode, flags);
}
bool CompositorController::generateKey(const std::string& client, const uint32_t& keyCode, const uint32_t& flags, std::string virtualKey)
{
    return impl->generateKey(client, keyCode, flags, virtualKey);
}

bool CompositorController::generateKey(const std::string& client, const uint32_t& keyCode, const uint32_t& flags, std::string virtualKey, double duration)
{
    return impl->generateKey(client, keyCode, flags, virtualKey, duration);
}

bool CompositorController::getScreenResolution(uint32_t &width, uint32_t &height)
{
    return impl->getScreenResolution(width,height);
}
bool CompositorController::setScreenResolution(const uint32_t width, const uint32_t height)
{
    return impl->setScreenResolution(width,height);
}
bool CompositorController::getClients(std::vector<std::string>& clients)
{
    return impl->getClients(clients);
}
bool CompositorController::getZOrder(std::vector<std::string>&clients)
{
    return impl->getZOrder(clients);
}
bool CompositorController::getBounds(const std::string& client, uint32_t &x, uint32_t &y, uint32_t &width, uint32_t &height)
{
    return impl->getBounds(client, x, y, width, height);
}
bool CompositorController::setBounds(const std::string& client, const uint32_t x, const uint32_t y, const uint32_t width, const uint32_t height )
{
    return impl->setBounds(client, x, y, width, height);
}
bool CompositorController::getVisibility(const std::string& client, bool& visible)
{
    return impl->getVisibility(client, visible);
}
bool CompositorController::getOpacity(const std::string& client, unsigned int& opacity)
{
    return impl->getOpacity(client, opacity);
}
bool CompositorController::setOpacity(const std::string& client, const unsigned int opacity)
{
    return impl->setOpacity(client, opacity);
}
bool CompositorController::getScale(const std::string& client, double &scaleX, double &scaleY)
{
    return impl->getScale(client, scaleX, scaleY);
}
bool CompositorController::setScale(const std::string& client, double scaleX, double scaleY)
{
    return impl->setScale(client, scaleX, scaleY);
}
bool CompositorController::getHolePunch(const std::string& client, bool& holePunch)
{
    return impl->getHolePunch(client, holePunch);
}
bool CompositorController::setHolePunch(const std::string& client, const bool holePunch)
{
    return impl->setHolePunch(client, holePunch);
}
bool CompositorController::scaleToFit(const std::string& client, const int32_t x, const int32_t y, const uint32_t width, const uint32_t height)
{
    return impl->scaleToFit(client,x,y,width,height);
}
bool CompositorController::createDisplay(const std::string& client, const std::string& displayName, uint32_t displayWidth, uint32_t displayHeight,
                bool virtualDisplayEnabled, uint32_t virtualWidth, uint32_t virtualHeight, bool topmost, bool focus , bool autodestroy)
{
    return impl->createDisplay(client, displayName, displayWidth, displayHeight, virtualDisplayEnabled, virtualWidth, virtualHeight, topmost, focus, autodestroy);
}
bool CompositorController::addAnimation(const std::string& client, double duration, std::map<std::string, RdkShellData> &animationProperties)
{
    return impl->addAnimation(client, duration, animationProperties);
}
bool CompositorController::removeAnimation(const std::string& client)
{
    return impl->removeAnimation(client);
}
bool CompositorController::addListener(const std::string& client, std::shared_ptr<RdkShellEventListener> listener)
{
    return impl->addListener(client, listener);
}
bool CompositorController::removeListener(const std::string& client, std::shared_ptr<RdkShellEventListener> listener)
{
    return impl->removeListener(client, listener);
}
void CompositorController::enableInactivityReporting(const bool enable)
{
    EXPECT_NE(impl, nullptr);
    impl->enableInactivityReporting(enable);
}
void CompositorController::setInactivityInterval(const double minutes)
{
    EXPECT_NE(impl, nullptr);
    impl->setInactivityInterval(minutes);
}
void CompositorController::resetInactivityTime()
{
    EXPECT_NE(impl, nullptr);
    impl->resetInactivityTime();
}
void CompositorController::setEventListener(std::shared_ptr<RdkShellEventListener> listener)
{
    EXPECT_NE(impl, nullptr);
    impl->setEventListener(listener);
}
bool CompositorController::launchApplication(const std::string& client, const std::string& uri, const std::string& mimeType, bool topmost, bool focus)
{
    EXPECT_NE(impl, nullptr);
    return impl->launchApplication(client, uri, mimeType, topmost, focus);
}
bool CompositorController::suspendApplication(const std::string& client)
{
    EXPECT_NE(impl, nullptr);
    return impl->suspendApplication(client);
}
bool CompositorController::resumeApplication(const std::string& client)
{
    EXPECT_NE(impl, nullptr);
    return impl->resumeApplication(client);
}
bool CompositorController::getMimeType(const std::string& client, std::string& mimeType)
{
    EXPECT_NE(impl, nullptr);
    return impl->getMimeType(client, mimeType);
}
bool CompositorController::setMimeType(const std::string& client, const std::string& mimeType)
{
    EXPECT_NE(impl, nullptr);
    return impl->setMimeType(client, mimeType);
}
bool CompositorController::hideSplashScreen()
{
    EXPECT_NE(impl, nullptr);
    return impl->hideSplashScreen();
}
bool CompositorController::showSplashScreen(uint32_t displayTimeInSeconds)
{
    EXPECT_NE(impl, nullptr);
    return impl->showSplashScreen(displayTimeInSeconds);
}
bool CompositorController::hideWatermark()
{
    EXPECT_NE(impl, nullptr);
    return impl->hideWatermark();
}
bool CompositorController::showWatermark()
{
    EXPECT_NE(impl, nullptr);
    return impl->showWatermark();
}
bool CompositorController::hideFullScreenImage()
{
    EXPECT_NE(impl, nullptr);
    return impl->hideFullScreenImage();
}
bool CompositorController::showFullScreenImage(std::string file)
{
    EXPECT_NE(impl, nullptr);
    return impl->showFullScreenImage(file);
}
bool CompositorController::setLogLevel(const std::string level)
{
    EXPECT_NE(impl, nullptr);
    return impl->setLogLevel(level);
}
bool CompositorController::getLogLevel(std::string& level)
{
    EXPECT_NE(impl, nullptr);
    return impl->getLogLevel(level);
}
bool CompositorController::setTopmost(const std::string& client, bool topmost, bool focus)
{
    EXPECT_NE(impl, nullptr);
    return impl->setTopmost(client, topmost, focus);
}
bool CompositorController::getTopmost(std::string& client)
{
    EXPECT_NE(impl, nullptr);
    return impl->getTopmost(client);
}
bool CompositorController::isSurfaceModeEnabled()
{
    EXPECT_NE(impl, nullptr);
    return impl->isSurfaceModeEnabled();
}
bool CompositorController::enableKeyRepeats(bool enable)
{
    EXPECT_NE(impl, nullptr);
    return impl->enableKeyRepeats(enable);
}
bool CompositorController::getKeyRepeatsEnabled(bool& enable)
{
    EXPECT_NE(impl, nullptr);
    return impl->getKeyRepeatsEnabled(enable);
}
bool CompositorController::getVirtualResolution(const std::string& client, uint32_t &virtualWidth, uint32_t &virtualHeight)
{
    EXPECT_NE(impl, nullptr);
    return impl->getVirtualResolution(client, virtualWidth, virtualHeight);
}
bool CompositorController::setVirtualResolution(const std::string& client, const uint32_t virtualWidth, const uint32_t virtualHeight)
{
    EXPECT_NE(impl, nullptr);
    return impl->setVirtualResolution(client, virtualWidth, virtualHeight);
}
bool CompositorController::enableVirtualDisplay(const std::string& client, const bool enable)
{
    EXPECT_NE(impl, nullptr);
    return impl->enableVirtualDisplay(client, enable);
}
bool CompositorController::getVirtualDisplayEnabled(const std::string& client, bool &enabled)
{
    EXPECT_NE(impl, nullptr);
    return impl->getVirtualDisplayEnabled(client, enabled);
}
bool CompositorController::getLastKeyPress(uint32_t &keyCode, uint32_t &modifiers, uint64_t &timestampInSeconds)
{
    EXPECT_NE(impl, nullptr);
    return impl->getLastKeyPress(keyCode, modifiers, timestampInSeconds);
}
bool CompositorController::ignoreKeyInputs(bool ignore)
{
    EXPECT_NE(impl, nullptr);
    return impl->ignoreKeyInputs(ignore);
}
bool CompositorController::screenShot(uint8_t* &data, uint32_t &size)
{
    EXPECT_NE(impl, nullptr);
    return impl->screenShot(data, size);
}
bool CompositorController::enableInputEvents(const std::string& client, bool enable)
{
    EXPECT_NE(impl, nullptr);
    return impl->enableInputEvents(client, enable);
}
bool CompositorController::showCursor()
{
    EXPECT_NE(impl, nullptr);
    return impl->showCursor();
}
bool CompositorController::hideCursor()
{
    EXPECT_NE(impl, nullptr);
    return impl->hideCursor();
}
bool CompositorController::setCursorSize(uint32_t width, uint32_t height)
{
    EXPECT_NE(impl, nullptr);
    return impl->setCursorSize(width, height);
}
bool CompositorController::getCursorSize(uint32_t& width, uint32_t& height)
{
    EXPECT_NE(impl, nullptr);
    return impl->getCursorSize(width, height);
}
void CompositorController::setKeyRepeatConfig(bool enabled, int32_t initialDelay, int32_t repeatInterval)
{
    EXPECT_NE(impl, nullptr);
    impl->setKeyRepeatConfig(enabled, initialDelay, repeatInterval);
}
bool CompositorController::setAVBlocked(std::string callsign, bool blockAV)
{
    EXPECT_NE(impl, nullptr);
    return impl->setAVBlocked(callsign, blockAV);
}
bool CompositorController::getBlockedAVApplications(std::vector<std::string>& apps)
{
    EXPECT_NE(impl, nullptr);
    return impl->getBlockedAVApplications(apps);
}
bool CompositorController::isErmEnabled()
{
    EXPECT_NE(impl, nullptr);
    return impl->isErmEnabled();
}
}


