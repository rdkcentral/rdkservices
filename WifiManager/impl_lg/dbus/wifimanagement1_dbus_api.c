#include "wifimanagement1_dbus_api.h"

Wifimanagement1 *
com_lgi_rdk_utils_wifimanagement1_proxy_new_for_bus_sync (
    GBusType bus_type,
    GDBusProxyFlags flags,
    const gchar *name,
    const gchar *object_path,
    GCancellable *cancellable,
    GError **error)
{
    static const gchar *INTERFACE_NAME = "com.lgi.rdk.utils.wifimanagement1";

    GDBusConnection *connection = g_bus_get_sync(G_BUS_TYPE_SYSTEM, cancellable, error);
    if (!connection)
    {
        printf("com_lgi_rdk_utils_wifimanagement1_proxy_new_for_bus_sync: failed g_bus_get_sync; error: '%s'\n", (*error) ? (*error)->message : "?");
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
            printf("com_lgi_rdk_utils_wifimanagement1_proxy_new_for_bus_sync: failed g_dbus_proxy_new_sync; error: '%s'\n", (*error) ? (*error)->message : "?");
            g_object_unref(connection);
            return NULL;
        }
        else
        {
            Wifimanagement1 *ret = g_malloc(sizeof(Wifimanagement1));
            ret->connection = connection;
            ret->proxy = proxy;
            return ret;
        }
    }
}

/* copied from gdbus-codegen generated version; the only change is in first arg to g_dbus_proxy_call_sync */
/**
 * com_lgi_rdk_utils_wifimanagement1_call_get_ssidparams_sync:
 * @proxy: Wifimanagement1 proxy.
 * @arg_id: Argument to pass with the method invocation.
 * @arg_netid: Argument to pass with the method invocation.
 * @out_status: (out): Return location for return parameter or %NULL to ignore.
 * @out_count: (out): Return location for return parameter or %NULL to ignore.
 * @out_params: (out): Return location for return parameter or %NULL to ignore.
 * @cancellable: (nullable): A #GCancellable or %NULL.
 * @error: Return location for error or %NULL.
 *
 * Synchronously invokes the <link linkend="gdbus-method-com-lgi-rdk-utils-wifimanagement1.GetSSIDParams">GetSSIDParams()</link> D-Bus method on @proxy. The calling thread is blocked until a reply is received.
 *
 * See com_lgi_rdk_utils_wifimanagement1_call_get_ssidparams() for the asynchronous version of this method.
 *
 * Returns: (skip): %TRUE if the call succeded, %FALSE if @error is set.
 */
gboolean
com_lgi_rdk_utils_wifimanagement1_call_get_ssidparams_sync (
    Wifimanagement1 *proxy,
    const gchar *arg_id,
    const gchar *arg_netid,
    gint *out_status,
    guint *out_count,
    GVariant **out_params,
    GCancellable *cancellable,
    GError **error)
{
  GVariant *_ret;
  _ret = g_dbus_proxy_call_sync (proxy->proxy,
    "GetSSIDParams",
    g_variant_new ("(ss)",
                   arg_id,
                   arg_netid),
    G_DBUS_CALL_FLAGS_NONE,
    -1,
    cancellable,
    error);
  if (_ret == NULL)
    goto _out;
  g_variant_get (_ret,
                 "(iu@a{ss})",
                 out_status,
                 out_count,
                 out_params);
  g_variant_unref (_ret);
_out:
  return _ret != NULL;
}
