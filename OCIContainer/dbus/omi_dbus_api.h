#ifndef __OMI_DBUS_API_H__
#define __OMI_DBUS_API_H__

#include <gio/gio.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    GDBusConnection* connection;
    GDBusProxy* proxy;
} Omi1;

Omi1 *omi1_proxy_new_for_bus_sync (
    GBusType             bus_type,
    GDBusProxyFlags      flags,
    const gchar         *name,
    const gchar         *object_path,
    GCancellable        *cancellable,
    GError             **error);

gboolean omi1_call_mount_sync (
    Omi1 *proxy,
    const gchar *arg_id,
    const gchar *arg_rootfs_path,
    const gchar *arg_config_json_path,
    gchar **out_container_path,
    GCancellable *cancellable,
    GError **error);

gboolean omi1_call_umount_sync (
    Omi1 *proxy,
    const gchar *arg_id,
    GCancellable *cancellable,
    GError **error);

#ifdef __cplusplus // extern "C"
}
#endif

#endif
