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

#include "focusmanager.h"

#include <algorithm>
#include <deque>
#include <glib.h>
#include <simpleshell-client-protocol.h>
#include <wayland-client.h>

namespace {

class WesterosFocusManager : public Testing::FocusManager
{
public:
    WesterosFocusManager() = default;
    ~WesterosFocusManager() override;

    bool Start() override;
    bool Stop() override;

    void onRegistryGlobal(wl_registry* registry, uint32_t name, const char* interface, uint32_t version);
    void onSurfaceCreated(uint32_t surfaceId, const char *name);
    void onSurfaceDestroyed(uint32_t surfaceId, const char *name);
    void onSurfaceStatus(uint32_t surfaceId, const char *name);

private:

    static gboolean onWaylandEvent(GIOChannel* channel, GIOCondition condition, gpointer data);
    void focusSurface(uint32_t surfaceId);

    struct {
        wl_display* display = nullptr;
        wl_registry* registry = nullptr;
        wl_simple_shell* simpleshell = nullptr;
    } m_wl;
    GSource *m_waylandSource;
    std::deque<uint32_t> m_surfaces;
};

static void simpleshellSurfaceId(void *data, struct wl_simple_shell *wl_simple_shell, struct wl_surface *surface, uint32_t surfaceId) {}
static void simpleshellSurfaceCreated(void *data, struct wl_simple_shell *wl_simple_shell, uint32_t surfaceId, const char *name) {
    WesterosFocusManager* focusManager = static_cast<WesterosFocusManager*>(data);
    focusManager->onSurfaceCreated(surfaceId, name);
}
static void simpleshellSurfaceDestroyed(void *data, struct wl_simple_shell *wl_simple_shell, uint32_t surfaceId, const char *name) {
    WesterosFocusManager* focusManager = static_cast<WesterosFocusManager*>(data);
    focusManager->onSurfaceDestroyed(surfaceId, name);
}
static void simpleshellSurfaceStatus(void *data, struct wl_simple_shell *wl_simple_shell, uint32_t surfaceId, const char *name, uint32_t visible,
                                     int32_t x, int32_t y, int32_t width, int32_t height, wl_fixed_t opacity, wl_fixed_t zorder) {
    WesterosFocusManager* focusManager = static_cast<WesterosFocusManager*>(data);
    focusManager->onSurfaceStatus(surfaceId, name);
}
static void simpleshellGetSurfacesDone(void* data, wl_simple_shell* simpleshell) {}
static wl_simple_shell_listener s_simpleshell_listener = {
    .surface_id = simpleshellSurfaceId,
    .surface_created = simpleshellSurfaceCreated,
    .surface_destroyed = simpleshellSurfaceDestroyed,
    .surface_status = simpleshellSurfaceStatus,
    .get_surfaces_done = simpleshellGetSurfacesDone
};

static void registryGlobal(void* data, wl_registry* registry, uint32_t name, const char* interface, uint32_t version) {
    WesterosFocusManager* focusManager = static_cast<WesterosFocusManager*>(data);
    focusManager->onRegistryGlobal(registry, name, interface, version);
}
static void registryGlobalRemove(void* data, wl_registry* registry, uint32_t name) {}
static wl_registry_listener s_registry_listener = {
    .global = registryGlobal,
    .global_remove = registryGlobalRemove
};

WesterosFocusManager::~WesterosFocusManager() {
    Stop();
}

bool WesterosFocusManager::Start() {
    if (m_wl.display) {
        g_warning("Already started");
        return false;
    }

    m_wl.display = wl_display_connect(nullptr);
    if (!m_wl.display) {
        g_warning("Failed to connect wayland display");
        return false;
    }

    m_wl.registry = wl_display_get_registry(m_wl.display);
    wl_registry_add_listener(m_wl.registry, &s_registry_listener, this);

    wl_display_roundtrip(m_wl.display);
    if (!m_wl.simpleshell) {
        g_warning("Failed to get simpleshell interface");
        return false;
    }

    int fd = wl_display_get_fd(m_wl.display);
    if (fd < 0) {
        g_warning("Failed to get wayland display fd");
        return false;
    }

    GIOChannel *waylandChannel = g_io_channel_unix_new(fd);
    m_waylandSource = g_io_create_watch(waylandChannel, G_IO_IN);
    g_source_set_callback(m_waylandSource,
                          (GSourceFunc) &WesterosFocusManager::onWaylandEvent,
                          this, nullptr);
    g_source_attach(m_waylandSource, g_main_context_get_thread_default());
    g_io_channel_unref(waylandChannel);
    return true;
}

bool WesterosFocusManager::Stop() {
    if (m_waylandSource) {
        g_source_destroy(m_waylandSource);
        m_waylandSource = nullptr;
    }

    if (m_wl.simpleshell) {
        wl_simple_shell_destroy(m_wl.simpleshell);
        m_wl.simpleshell = nullptr;
    }
    if (m_wl.registry) {
        wl_registry_destroy(m_wl.registry);
        m_wl.registry = nullptr;
    }
    if (m_wl.display) {
        wl_display_disconnect(m_wl.display);
        m_wl.display = nullptr;
    }
    m_surfaces.clear();

    return true;
}

void WesterosFocusManager::onRegistryGlobal(wl_registry* registry, uint32_t name, const char* interface, uint32_t version) {
    if (strcmp(interface, wl_simple_shell_interface.name) == 0) {
        m_wl.simpleshell = (wl_simple_shell*)wl_registry_bind(registry, name, &wl_simple_shell_interface, version);
        wl_simple_shell_add_listener(m_wl.simpleshell, &s_simpleshell_listener, this);
        wl_simple_shell_get_surfaces(m_wl.simpleshell);
        wl_display_flush(m_wl.display);
    }
}

void WesterosFocusManager::onSurfaceCreated(uint32_t surfaceId, const char *name) {
    if (std::find(m_surfaces.begin(), m_surfaces.end(), surfaceId) != m_surfaces.end()) {
        g_warning("Surface already present %u %s", surfaceId, name);
        return;
    }
    m_surfaces.push_back(surfaceId);
    // focus new surface
    focusSurface(surfaceId);
}

void WesterosFocusManager::onSurfaceDestroyed(uint32_t surfaceId, const char *name) {
    // Expecting the last surface removal
    if (m_surfaces.empty()) {
        g_warning("Unexpected surface destroyed %u %s", surfaceId, name);
        return;
    }
    if (m_surfaces.back() != surfaceId) {
        g_warning("Unexpected surface destroyed %u %s", surfaceId, name);
        std::remove(m_surfaces.begin(), m_surfaces.end(), surfaceId);
        return;
    }
    m_surfaces.pop_back();
    if (m_surfaces.empty())
        return;

    // move focus back to previous surface
    focusSurface(m_surfaces.back());
}

void WesterosFocusManager::onSurfaceStatus(uint32_t surfaceId, const char *name) {
    if (std::find(m_surfaces.begin(), m_surfaces.end(), surfaceId) != m_surfaces.end()) {
        // we already have know this surface
        return;
    }
    // For surfaces recognized before we started listening.
    // It is expected to receive only one surface here for parent page.
    m_surfaces.push_back(surfaceId);
}

void WesterosFocusManager::focusSurface(uint32_t surfaceId) {
    g_message("WesterosFocusManager: Focus surface %u", surfaceId);
    wl_simple_shell_set_focus(m_wl.simpleshell, surfaceId);
    wl_display_flush(m_wl.display);
}

gboolean WesterosFocusManager::onWaylandEvent(
        GIOChannel* channel, GIOCondition condition, gpointer data)
{
    WesterosFocusManager& focusManager = *static_cast<WesterosFocusManager*>(data);
    if (focusManager.m_wl.display) {
        if (wl_display_dispatch(focusManager.m_wl.display) < 0) {
            g_warning("Failed to dispatch wayland events");
            return G_SOURCE_REMOVE;
        }
    }
    return G_SOURCE_CONTINUE;
}

} // namespace

namespace Testing {
FocusManager* FocusManager::Instance() {
    static WesterosFocusManager focusManager;
    return &focusManager;
}
} // namespace Testing
