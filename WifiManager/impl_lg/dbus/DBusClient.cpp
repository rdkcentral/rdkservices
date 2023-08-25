#include "DBusClient.h"
#include "Module.h"
#include <UtilsLogging.h>
#include <UtilsFile.h>

#define NETWORK_CONFIG_DBUS_INTERFACE_NAME "com.lgi.rdk.utils.networkconfig1"
#define NETWORK_CONFIG_DBUS_INTERFACE_OBJECT_PATH "/com/lgi/rdk/utils/networkconfig1"

#define WIFI_MANAGEMENT_DBUS_INTERFACE_NAME "com.lgi.rdk.utils.wifimanagement1"
#define WIFI_MANAGEMENT_DBUS_INTERFACE_OBJECT_PATH "/com/lgi/rdk/utils/wifimanagement1"

namespace WifiManagerImpl
{
    static const std::map<std::string, InterfaceStatus> statusFromString{
        {"Disabled", Disabled},
        {"Disconnected", Disconnected},
        {"Associating", Associating},
        {"Dormant", Dormant},
        {"Binding", Binding},
        {"Assigned", Assigned},
        {"Scanning", Scanning}};

    DBusClient::DBusClient()
    {
        run();
    }

    static void handle_dbus_event(GDBusProxy *proxy,
                                  char *sender_name,
                                  char *_signal_name,
                                  GVariant *parameters,
                                  gpointer user_data)
    {
        std::string signal_name{_signal_name};

        const gsize num_params = g_variant_n_children(parameters);
        GVariantIter iter;
        g_variant_iter_init(&iter, parameters);

        DBusClient *client = static_cast<DBusClient *>(user_data);

        if (signal_name == "StatusChanged" && num_params == 2)
        {
            GVariant *vId = g_variant_iter_next_value(&iter);
            GVariant *vInterfaceStatus = g_variant_iter_next_value(&iter);
            const gchar *aId = g_variant_get_string(vId, NULL);
            const gchar *aIfaceStatus = g_variant_get_string(vInterfaceStatus, NULL);
            client->handleStatusChangedDbusEvent(aId, aIfaceStatus);
            g_variant_unref(vInterfaceStatus);
            g_variant_unref(vId);
        }
        else
        {
            LOGINFO("handle_dbus_event: unsupported event; sender_name: %s signal_name: %s, num_params: %u", sender_name, _signal_name, num_params);
        }
    }

    void DBusClient::handleStatusChangedDbusEvent(const std::string &aId, const std::string &aIfaceStatus)
    {
        std::lock_guard<std::mutex> lock(m_event_mutex);
        if (m_statusChangedHandler)
        {
            auto status = statusFromString.find(aIfaceStatus);
            if (status != statusFromString.end())
            {
                m_statusChangedHandler(aId, status->second);
            }
            else
            {
                LOGERR("networkconfig1 StatusChanged event received with unknown status string: %s", aIfaceStatus.c_str());
            }
        }
    }

    void DBusClient::run()
    {
        if (!m_dbus_data)
        {
            m_loopThread = std::thread(&DBusClient::dbusWorker, this);
            m_dbus_data = m_dbus_data_future.get_future().get();

            if (m_dbus_data)
            {
                m_handle_networkconfig_gsignal = g_signal_connect(m_dbus_data->m_networkconfig1_interface->proxy, "g-signal", G_CALLBACK(handle_dbus_event), this);
                if (!m_handle_networkconfig_gsignal)
                {
                    LOGERR("Cannot connect to networkconfig1 g-signal");
                }

                /* seems we do not need any wifimanagement1 signals for now
                m_handle_wifimanagement_gsignal = g_signal_connect(m_wifimanagement1_interface->proxy, "g-signal", G_CALLBACK(handle_dbus_event), this);
                if (!m_handle_wifimanagement_gsignal)
                {
                    LOGERR("Cannot connect to wifimanagement1 g-signal");
                }
                */
            }
            else
            {
                m_loopThread.join();
                throw std::runtime_error("failure creating dbus interfaces");
            }
        }
    }

    template <class T>
    static void release_dbus_interface(T *interface)
    {
        if (interface->proxy)
        {
            g_object_unref(interface->proxy);
        }
        g_dbus_connection_flush_sync(interface->connection, NULL, NULL);
        g_object_unref(interface->connection);
        g_free(interface);
    }

    void DBusClient::stop()
    {
        if (m_dbus_data)
        {
            if (m_handle_networkconfig_gsignal != 0)
            {
                g_signal_handler_disconnect(m_dbus_data->m_networkconfig1_interface->proxy, m_handle_networkconfig_gsignal);
            }

            if (m_dbus_data->m_networkconfig1_interface)
                release_dbus_interface(m_dbus_data->m_networkconfig1_interface);
            if (m_dbus_data->m_wifimanagement1_interface)
                release_dbus_interface(m_dbus_data->m_wifimanagement1_interface);

            g_main_context_invoke(
                m_dbus_data->m_mainContext, +[](gpointer ptr) -> gboolean
                {
                LOGINFO("LgiNetworkClient::Stop() quit main loop TID: %u", gettid());
                g_main_loop_quit((GMainLoop*)ptr);
                return FALSE; },
                (gpointer)m_dbus_data->m_mainLoop);

            if (m_loopThread.joinable())
            {
                m_loopThread.join();
            }
            else
            {
                LOGERR("Worker thread should be joinable");
            }
            LOGINFO("signals disconnected");

            delete m_dbus_data;
            m_dbus_data = nullptr;
        }
    }

    bool DBusClient::networkconfig1_GetInterfaces(std::vector<std::string> &out)
    {
        GError *error{nullptr};
        bool ret = false;
        guint count = 0;
        gchar **ids{nullptr};

        if (com_lgi_rdk_utils_networkconfig1_call_get_interfaces_sync(
                m_dbus_data->m_networkconfig1_interface,
                &count,
                &ids,
                nullptr,
                &error))
        {
            for (guint i = 0; i < count; ++i)
            {
                out.push_back(ids[i]);
            }
            ret = true;
        }
        else
        {
            LOGERR("Failed to call networkconfig1_call_get_interfaces_sync - %s", error ? error->message : "(unknown)");
        }
        if (error)
            g_error_free(error);
        if (ids)
            g_free(ids);
        return ret;
    }

    bool DBusClient::networkconfig1_GetParam(const std::string &interface, const std::string &paramName, std::string &res)
    {
        gint status = 0;
        gchar *paramValue{nullptr};
        GError *error{nullptr};
        bool ret = false;
        if (
            com_lgi_rdk_utils_networkconfig1_call_get_param_sync(
                m_dbus_data->m_networkconfig1_interface,
                interface.c_str(),
                paramName.c_str(),
                &status,
                &paramValue,
                nullptr,
                &error))
        {
            if (status == 0)
            {
                res = paramValue;
                ret = true;
            }
            else
            {
                LOGERR("Failed to call networkconfig1_call_get_param_sync - status: %d", status);
            }
        }
        else
        {
            LOGERR("Failed to call networkconfig1_call_get_param_sync - %s", error ? error->message : "(unknown)");
        }
        if (error)
            g_error_free(error);
        if (paramValue)
            g_free(paramValue);
        return ret;
    }

    bool DBusClient::networkconfig1_GetStatus(const std::string &interface, InterfaceStatus &out)
    {
        gint status = 0;
        gchar *ifaceStatus{nullptr};
        GError *error{nullptr};
        bool ret = false;
        if (
            com_lgi_rdk_utils_networkconfig1_call_get_status_sync(
                m_dbus_data->m_networkconfig1_interface,
                interface.c_str(),
                &status,
                &ifaceStatus,
                nullptr,
                &error))
        {
            if (status == 0)
            {
                auto it = statusFromString.find(ifaceStatus);
                if (it != statusFromString.end())
                {
                    out = it->second;
                    ret = true;
                }
                else
                {
                    LOGERR("networkconfig1_call_get_status_sync returned unknown status string: %s", ifaceStatus);
                }
            }
            else
            {
                LOGERR("Failed to call networkconfig1_call_get_status_sync - status: %d", status);
            }
        }
        else
        {
            LOGERR("Failed to call networkconfig1_call_get_status_sync - %s", error ? error->message : "(unknown)");
        }
        if (error)
            g_error_free(error);
        if (ifaceStatus)
            g_free(ifaceStatus);
        return ret;
    }

    bool DBusClient::wifimanagement1_GetSSIDParams(const std::string &ssid, const std::string &netid, std::map<std::string, std::string> &params)
    {
        bool ret = false;
        gint status = 0;
        guint count = 0;
        GVariant *out_params{nullptr};
        GError *error{nullptr};

        if (com_lgi_rdk_utils_wifimanagement1_call_get_ssidparams_sync(
                m_dbus_data->m_wifimanagement1_interface,
                ssid.c_str(),  // const gchar *arg_id,
                netid.c_str(), // const gchar *arg_netid,
                &status,
                &count,
                &out_params,
                NULL,
                &error))
        {
            if (status)
            {
                LOGERR("GetSSIDParams failed; status: %u error: '%s'", status, error ? error->message : "(unknown)");
            }
            else
            {
                GVariantIter iter;
                gchar *key;
                gchar *value;

                if (out_params)
                {
                    g_variant_iter_init(&iter, out_params);
                    while (g_variant_iter_loop(&iter, "{ss}", &key, &value))
                    {
                        params[key] = value;
                    }
                }
                ret = true;
            }
        }
        else
        {
            LOGERR("Failed to call wifimanagement1_call_get_ssidparams_sync - %s", error ? error->message : "(unknown)");
        }
        if (error)
            g_error_free(error);
        if (out_params)
            g_variant_unref(out_params);
        return ret;
    }

    void DBusClient::dbusWorker()
    {
        LOGINFO("LgiNetworkClient::Worker() TID: %u", gettid());

        DbusData *dbusData = new DbusData();
        dbusData->m_mainContext = g_main_context_new();
        dbusData->m_mainLoop = g_main_loop_new(dbusData->m_mainContext, false);

        bool initialization_complete = false;
        GError *error = nullptr;

        if (dbusData->m_mainLoop && dbusData->m_mainContext)
        {
            g_main_context_push_thread_default(dbusData->m_mainContext);
            dbusData->m_networkconfig1_interface = networkconfig1_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
                                                                                         G_DBUS_PROXY_FLAGS_NONE,
                                                                                         NETWORK_CONFIG_DBUS_INTERFACE_NAME,
                                                                                         NETWORK_CONFIG_DBUS_INTERFACE_OBJECT_PATH,
                                                                                         NULL, /* GCancellable */
                                                                                         &error);
            if (error)
            {
                LOGERR("Failed to create networkconfig proxy: %s", error->message);
            }
            else
            {

                dbusData->m_wifimanagement1_interface = com_lgi_rdk_utils_wifimanagement1_proxy_new_for_bus_sync(G_BUS_TYPE_SYSTEM,
                                                                                                                 G_DBUS_PROXY_FLAGS_NONE,
                                                                                                                 WIFI_MANAGEMENT_DBUS_INTERFACE_NAME,
                                                                                                                 WIFI_MANAGEMENT_DBUS_INTERFACE_OBJECT_PATH,
                                                                                                                 NULL, /* GCancellable */
                                                                                                                 &error);
                if (error)
                {
                    LOGERR("Failed to create networkconfig proxy: %s", error->message);
                }
                else
                {
                    m_dbus_data_future.set_value(dbusData);
                    initialization_complete = true;
                    LOGINFO("DBusClient::dbusWorker() start main loop TID: %u", gettid());
                    g_main_loop_run(dbusData->m_mainLoop); // blocks
                    LOGINFO("DBusClient::dbusWorker() main loop finished TID: %u", gettid());
                    g_main_context_pop_thread_default(dbusData->m_mainContext);
                }
            }
        }
        else
        {
            LOGERR("Failed to create glib main loop");
            m_dbus_data_future.set_value(nullptr);
        }

        if (error)
        {
            g_error_free(error);
        }
        if (dbusData->m_mainLoop)
        {
            g_main_loop_unref(dbusData->m_mainLoop);
        }
        if (dbusData->m_mainContext)
        {
            g_main_context_unref(dbusData->m_mainContext);
        }
        if (!initialization_complete)
        {
            m_dbus_data_future.set_value(nullptr);
        }
    }
}
