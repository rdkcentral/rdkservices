#pragma once

#include <thread>
#include <future>
#include <string>
#include <vector>
#include <mutex>
#include <functional>
#include <map>

#include <gio/gio.h>
#include <glib.h>
#include "networkconfig1_dbus_api.h"
#include "wifimanagement1_dbus_api.h"

namespace WifiManagerImpl
{

    enum InterfaceStatus
    {
        Disabled,
        Disconnected,
        Associating,
        Dormant,
        Binding,
        Assigned,
        Scanning
    };

    class DBusClient
    {
    public:
        using StatusChangedHandler = std::function<void(const std::string &interface, InterfaceStatus state)>;

        static DBusClient &getInstance()
        {
            static DBusClient client;
            return client;
        }

        void run();
        void stop();

        void registerStatusChanged(StatusChangedHandler handler)
        {
            std::lock_guard<std::mutex> lock(m_event_mutex);
            m_statusChangedHandler = handler;
        }

        bool networkconfig1_GetInterfaces(std::vector<std::string> &ret);
        bool networkconfig1_GetParam(const std::string &interface, const std::string &name, std::string &res);
        bool networkconfig1_GetStatus(const std::string &interface, InterfaceStatus &out);
        bool wifimanagement1_GetSSIDParams(const std::string &ssid, const std::string &netid, std::map<std::string, std::string> &params);

        void handleStatusChangedDbusEvent(const std::string &aId, const std::string &aIfaceStatus);

    private:
        DBusClient();
        ~DBusClient() = default;

        void dbusWorker();

        std::thread m_loopThread;

        struct DbusData {
            GMainContext *m_mainContext{nullptr};
            GMainLoop *m_mainLoop{nullptr};
            Networkconfig1 *m_networkconfig1_interface{nullptr};
            Wifimanagement1 *m_wifimanagement1_interface{nullptr};
        };

        std::promise<DbusData*> m_dbus_data_future;
        DbusData* m_dbus_data{nullptr};

        std::mutex m_event_mutex;
        StatusChangedHandler m_statusChangedHandler {nullptr};

        gulong m_handle_networkconfig_gsignal{0};
        /* seems we do not need any wifimanagement1 signals for now
        gulong m_handle_wifimanagement_gsignal;
        */
    };
}