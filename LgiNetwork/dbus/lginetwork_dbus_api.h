#ifndef __LGINETWORK_DBUS_API_H__
#define __LGINETWORK_DBUS_API_H__

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

gboolean networkconfig1_call_get_interfaces_sync (
    Networkconfig1 *proxy,
    guint *out_count,
    gchar ***out_ids,
    GCancellable *cancellable,
    GError **error);

gboolean networkconfig1_call_get_params_sync (
    Networkconfig1 *proxy,
    const gchar *arg_id,
    gint *out_status,
    guint *out_count,
    GVariant **out_params,
    GCancellable *cancellable,
    GError **error);


gboolean networkconfig1_call_get_active_interface_sync (
    Networkconfig1 *proxy,
    gchar **out_id,
    gint *out_status,
    GCancellable *cancellable,
    GError **error);

gboolean networkconfig1_call_is_enabled_sync (
    Networkconfig1 *proxy,
    const gchar *arg_id,
    gint *out_status,
    gboolean *out_enabled,
    GCancellable *cancellable,
    GError **error);

gboolean networkconfig1_call_enable_sync (
    Networkconfig1 *proxy,
    const gchar *arg_id,
    gboolean arg_enable,
    gint *out_status,
    GCancellable *cancellable,
    GError **error);

#ifdef __cplusplus // extern "C"
}
#endif

#endif
