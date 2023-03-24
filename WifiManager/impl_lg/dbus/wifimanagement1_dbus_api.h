#ifndef __WIFIMANAGEMENT1_DBUS_API_H__
#define __WIFIMANAGEMENT1_DBUS_API_H__

#include <gio/gio.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    GDBusConnection* connection;
    GDBusProxy* proxy;
} Wifimanagement1;

Wifimanagement1 *
com_lgi_rdk_utils_wifimanagement1_proxy_new_for_bus_sync (
    GBusType bus_type,
    GDBusProxyFlags flags,
    const gchar *name,
    const gchar *object_path,
    GCancellable *cancellable,
    GError **error);

gboolean
com_lgi_rdk_utils_wifimanagement1_call_get_ssidparams_sync (
    Wifimanagement1 *proxy,
    const gchar *arg_id,
    const gchar *arg_netid,
    gint *out_status,
    guint *out_count,
    GVariant **out_params,
    GCancellable *cancellable,
    GError **error);

#ifdef __cplusplus // extern "C"
}
#endif

#endif
