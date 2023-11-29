#include "omi_dbus_api.h"

Omi1 *omi1_proxy_new_for_bus_sync(
    GBusType bus_type,
    GDBusProxyFlags flags,
    const gchar *name,
    const gchar *object_path,
    GCancellable *cancellable,
    GError **error)
{
    static const gchar *INTERFACE_NAME = "com.lgi.onemw.omi1";

    GDBusConnection *connection = g_bus_get_sync(G_BUS_TYPE_SYSTEM, cancellable, error);
    if (!connection)
    {
        printf("omi1_proxy_new_for_bus_sync: failed g_bus_get_sync; error: '%s'\n", (*error) ? (*error)->message : "?");
        return NULL;
    }
    else
    {
        GDBusProxy *proxy = g_dbus_proxy_new_sync(
            connection,
            flags,
            NULL, // A GDBusInterfaceInfo specifying the minimal interface that proxy conforms to or NULL
            name,
            object_path,
            INTERFACE_NAME,
            NULL, // cancellable
            error);
        if (!proxy)
        {
            printf("omi1_proxy_new_for_bus_sync: failed g_dbus_proxy_new_sync; error: '%s'\n", (*error) ? (*error)->message : "?");
            g_object_unref(connection);
            return NULL;
        }
        else
        {
            Omi1 *ret = g_malloc(sizeof(Omi1));
            ret->connection = connection;
            ret->proxy = proxy;
            return ret;
        }
    }
}

/* omi1_call_mount_sync method is almost directly copied from gdbus-codegen generated version
   the only change is in first arg to g_dbus_proxy_call_sync */
gboolean
omi1_call_mount_sync (
    Omi1 *proxy,
    const gchar *arg_id,
    const gchar *arg_rootfs_path,
    const gchar *arg_config_json_path,
    gchar **out_container_path,
    GCancellable *cancellable,
    GError **error)
{
  GVariant *_ret;
  _ret = g_dbus_proxy_call_sync (proxy->proxy,
    "Mount",
    g_variant_new ("(sss)",
                   arg_id,
                   arg_rootfs_path,
                   arg_config_json_path),
    G_DBUS_CALL_FLAGS_NONE,
    -1,
    cancellable,
    error);
  if (_ret == NULL)
    goto _out;
  g_variant_get (_ret,
                 "(s)",
                 out_container_path);
  g_variant_unref (_ret);
_out:
  return _ret != NULL;
}

/* omi1_call_umount_sync method is almost directly copied from gdbus-codegen generated version
   the only change is in first arg to g_dbus_proxy_call_sync */
gboolean
omi1_call_umount_sync (
    Omi1 *proxy,
    const gchar *arg_id,
    GCancellable *cancellable,
    GError **error)
{
  GVariant *_ret;
  _ret = g_dbus_proxy_call_sync (proxy->proxy,
    "Umount",
    g_variant_new ("(s)",
                   arg_id),
    G_DBUS_CALL_FLAGS_NONE,
    -1,
    cancellable,
    error);
  if (_ret == NULL)
    goto _out;
  g_variant_get (_ret,
                 "()");
  g_variant_unref (_ret);
_out:
  return _ret != NULL;
}
