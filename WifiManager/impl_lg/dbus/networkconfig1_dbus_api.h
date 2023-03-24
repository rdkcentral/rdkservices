#ifndef __NETWORKCONFIG1_DBUS_API_H__
#define __NETWORKCONFIG1_DBUS_API_H__

#include <gio/gio.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    GDBusConnection* connection;
    GDBusProxy* proxy;
} Networkconfig1;

Networkconfig1 *networkconfig1_proxy_new_for_bus_sync (
    GBusType             bus_type,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GError             **error);

gboolean
com_lgi_rdk_utils_networkconfig1_call_get_status_sync (
    Networkconfig1 *proxy,
    const gchar *arg_id,
    gint *out_status,
    gchar **out_ifaceStatus,
    GCancellable *cancellable,
    GError **error);

gboolean
com_lgi_rdk_utils_networkconfig1_call_get_param_sync (
    Networkconfig1 *proxy,
    const gchar *arg_id,
    const gchar *arg_paramName,
    gint *out_status,
    gchar **out_paramValue,
    GCancellable *cancellable,
    GError **error);

gboolean
com_lgi_rdk_utils_networkconfig1_call_get_interfaces_sync (
    Networkconfig1 *proxy,
    guint *out_count,
    gchar ***out_ids,
    GCancellable *cancellable,
    GError **error);

#ifdef __cplusplus // extern "C"
}
#endif

#endif
