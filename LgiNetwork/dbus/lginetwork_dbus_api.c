
#include "lginetwork_dbus_api.h"

Networkconfig1 *networkconfig1_proxy_new_for_bus_sync(
    GBusType bus_type,
    GDBusProxyFlags flags,
    const gchar *name,
    const gchar *object_path,
    GCancellable *cancellable,
    GError **error)
{
    static const gchar *INTERFACE_NAME = "com.lgi.rdk.utils.networkconfig1";

    GDBusConnection *connection = g_bus_get_sync(G_BUS_TYPE_SYSTEM, cancellable, error);
    if (!connection)
    {
        printf("networkconfig1_proxy_new_for_bus_sync: failed g_bus_get_sync; error: '%s'\n", (*error) ? (*error)->message : "?");
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
            printf("networkconfig1_proxy_new_for_bus_sync: failed g_dbus_proxy_new_sync; error: '%s'\n", (*error) ? (*error)->message : "?");
            g_object_unref(connection);
            return NULL;
        }
        else
        {
            Networkconfig1 *ret = g_malloc(sizeof(Networkconfig1));
            ret->connection = connection;
            ret->proxy = proxy;
            return ret;
        }
    }
}

/* networkconfig1_call_get_interfaces_sync method is almost directly copied from gdbus-codegen generated version
   the only change is in first arg to g_dbus_proxy_call_sync */
gboolean networkconfig1_call_get_interfaces_sync(
    Networkconfig1 *proxy,
    guint *out_count,
    gchar ***out_ids,
    GCancellable *cancellable,
    GError **error)
{
    GVariant *_ret;
    _ret = g_dbus_proxy_call_sync(proxy->proxy,
                                  "GetInterfaces",
                                  g_variant_new("()"),
                                  G_DBUS_CALL_FLAGS_NONE,
                                  -1,
                                  cancellable,
                                  error);
    if (_ret == NULL)
        goto _out;
    g_variant_get(_ret,
                  "(u^as)",
                  out_count,
                  out_ids);
    g_variant_unref(_ret);
_out:
    return _ret != NULL;
}

/* networkconfig1_call_get_params_sync method is almost directly copied from gdbus-codegen generated version
   the only change is in first arg to g_dbus_proxy_call_sync */
gboolean networkconfig1_call_get_params_sync(
    Networkconfig1 *proxy,
    const gchar *arg_id,
    gint *out_status,
    guint *out_count,
    GVariant **out_params,
    GCancellable *cancellable,
    GError **error)
{
    GVariant *_ret;
    _ret = g_dbus_proxy_call_sync(proxy->proxy,
                                  "GetParams",
                                  g_variant_new("(s)",
                                                arg_id),
                                  G_DBUS_CALL_FLAGS_NONE,
                                  -1,
                                  cancellable,
                                  error);
    if (_ret == NULL)
        goto _out;
    g_variant_get(_ret,
                  "(iu@a{ss})",
                  out_status,
                  out_count,
                  out_params);
    g_variant_unref(_ret);
_out:
    return _ret != NULL;
}

/* networkconfig1_call_get_active_interface_sync method is almost directly copied from gdbus-codegen generated version
   the only change is in first arg to g_dbus_proxy_call_sync */
gboolean networkconfig1_call_get_active_interface_sync(
    Networkconfig1 *proxy,
    gchar **out_id,
    gint *out_status,
    GCancellable *cancellable,
    GError **error)
{
    GVariant *_ret;
    _ret = g_dbus_proxy_call_sync(proxy->proxy,
                                  "GetActiveInterface",
                                  g_variant_new("()"),
                                  G_DBUS_CALL_FLAGS_NONE,
                                  -1,
                                  cancellable,
                                  error);
    if (_ret == NULL)
        goto _out;
    g_variant_get(_ret,
                  "(si)",
                  out_id,
                  out_status);
    g_variant_unref(_ret);
_out:
    return _ret != NULL;
}

/* networkconfig1_call_is_enabled_sync method is almost directly copied from gdbus-codegen generated version
   the only change is in first arg to g_dbus_proxy_call_sync */
gboolean networkconfig1_call_is_enabled_sync(
    Networkconfig1 *proxy,
    const gchar *arg_id,
    gint *out_status,
    gboolean *out_enabled,
    GCancellable *cancellable,
    GError **error)
{
    GVariant *_ret;
    _ret = g_dbus_proxy_call_sync(proxy->proxy,
                                  "IsEnabled",
                                  g_variant_new("(s)",
                                                arg_id),
                                  G_DBUS_CALL_FLAGS_NONE,
                                  -1,
                                  cancellable,
                                  error);
    if (_ret == NULL)
        goto _out;
    g_variant_get(_ret,
                  "(ib)",
                  out_status,
                  out_enabled);
    g_variant_unref(_ret);
_out:
    return _ret != NULL;
}

/* networkconfig1_call_enable_sync method is almost directly copied from gdbus-codegen generated version
   the only change is in first arg to g_dbus_proxy_call_sync */
gboolean networkconfig1_call_enable_sync(
    Networkconfig1 *proxy,
    const gchar *arg_id,
    gboolean arg_enable,
    gint *out_status,
    GCancellable *cancellable,
    GError **error)
{
    GVariant *_ret;
    _ret = g_dbus_proxy_call_sync(proxy->proxy,
                                  "Enable",
                                  g_variant_new("(sb)",
                                                arg_id,
                                                arg_enable),
                                  G_DBUS_CALL_FLAGS_NONE,
                                  -1,
                                  cancellable,
                                  error);
    if (_ret == NULL)
        goto _out;
    g_variant_get(_ret,
                  "(i)",
                  out_status);
    g_variant_unref(_ret);
_out:
    return _ret != NULL;
}
