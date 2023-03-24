
#include "networkconfig1_dbus_api.h"

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

/* copied from gdbus-codegen generated version; the only change is in first arg to g_dbus_proxy_call_sync */
/**
 * com_lgi_rdk_utils_networkconfig1_call_get_status_sync:
 * @proxy: A #ComLgiRdkUtilsNetworkconfig1Proxy.
 * @arg_id: Argument to pass with the method invocation.
 * @out_status: (out): Return location for return parameter or %NULL to ignore.
 * @out_ifaceStatus: (out): Return location for return parameter or %NULL to ignore.
 * @cancellable: (nullable): A #GCancellable or %NULL.
 * @error: Return location for error or %NULL.
 *
 * Synchronously invokes the <link linkend="gdbus-method-com-lgi-rdk-utils-networkconfig1.GetStatus">GetStatus()</link> D-Bus method on @proxy. The calling thread is blocked until a reply is received.
 *
 * See com_lgi_rdk_utils_networkconfig1_call_get_status() for the asynchronous version of this method.
 *
 * Returns: (skip): %TRUE if the call succeded, %FALSE if @error is set.
 */
gboolean
com_lgi_rdk_utils_networkconfig1_call_get_status_sync (
    Networkconfig1 *proxy,
    const gchar *arg_id,
    gint *out_status,
    gchar **out_ifaceStatus,
    GCancellable *cancellable,
    GError **error)
{
  GVariant *_ret;
  _ret = g_dbus_proxy_call_sync (proxy->proxy,
    "GetStatus",
    g_variant_new ("(s)",
                   arg_id),
    G_DBUS_CALL_FLAGS_NONE,
    -1,
    cancellable,
    error);
  if (_ret == NULL)
    goto _out;
  g_variant_get (_ret,
                 "(is)",
                 out_status,
                 out_ifaceStatus);
  g_variant_unref (_ret);
_out:
  return _ret != NULL;
}

/* copied from gdbus-codegen generated version; the only change is in first arg to g_dbus_proxy_call_sync */
/**
 * com_lgi_rdk_utils_networkconfig1_call_get_param_sync:
 * @proxy: A #ComLgiRdkUtilsNetworkconfig1Proxy.
 * @arg_id: Argument to pass with the method invocation.
 * @arg_paramName: Argument to pass with the method invocation.
 * @out_status: (out): Return location for return parameter or %NULL to ignore.
 * @out_paramValue: (out): Return location for return parameter or %NULL to ignore.
 * @cancellable: (nullable): A #GCancellable or %NULL.
 * @error: Return location for error or %NULL.
 *
 * Synchronously invokes the <link linkend="gdbus-method-com-lgi-rdk-utils-networkconfig1.GetParam">GetParam()</link> D-Bus method on @proxy. The calling thread is blocked until a reply is received.
 *
 * See com_lgi_rdk_utils_networkconfig1_call_get_param() for the asynchronous version of this method.
 *
 * Returns: (skip): %TRUE if the call succeded, %FALSE if @error is set.
 */
gboolean
com_lgi_rdk_utils_networkconfig1_call_get_param_sync (
    Networkconfig1 *proxy,
    const gchar *arg_id,
    const gchar *arg_paramName,
    gint *out_status,
    gchar **out_paramValue,
    GCancellable *cancellable,
    GError **error)
{
  GVariant *_ret;
  _ret = g_dbus_proxy_call_sync (proxy->proxy,
    "GetParam",
    g_variant_new ("(ss)",
                   arg_id,
                   arg_paramName),
    G_DBUS_CALL_FLAGS_NONE,
    -1,
    cancellable,
    error);
  if (_ret == NULL)
    goto _out;
  g_variant_get (_ret,
                 "(is)",
                 out_status,
                 out_paramValue);
  g_variant_unref (_ret);
_out:
  return _ret != NULL;
}

/* copied from gdbus-codegen generated version; the only change is in first arg to g_dbus_proxy_call_sync */
/**
 * com_lgi_rdk_utils_networkconfig1_call_get_interfaces_sync:
 * @proxy: A #ComLgiRdkUtilsNetworkconfig1Proxy.
 * @out_count: (out): Return location for return parameter or %NULL to ignore.
 * @out_ids: (out) (array zero-terminated=1): Return location for return parameter or %NULL to ignore.
 * @cancellable: (nullable): A #GCancellable or %NULL.
 * @error: Return location for error or %NULL.
 *
 * Synchronously invokes the <link linkend="gdbus-method-com-lgi-rdk-utils-networkconfig1.GetInterfaces">GetInterfaces()</link> D-Bus method on @proxy. The calling thread is blocked until a reply is received.
 *
 * See com_lgi_rdk_utils_networkconfig1_call_get_interfaces() for the asynchronous version of this method.
 *
 * Returns: (skip): %TRUE if the call succeded, %FALSE if @error is set.
 */
gboolean
com_lgi_rdk_utils_networkconfig1_call_get_interfaces_sync (
    Networkconfig1 *proxy,
    guint *out_count,
    gchar ***out_ids,
    GCancellable *cancellable,
    GError **error)
{
  GVariant *_ret;
  _ret = g_dbus_proxy_call_sync (proxy->proxy,
    "GetInterfaces",
    g_variant_new ("()"),
    G_DBUS_CALL_FLAGS_NONE,
    -1,
    cancellable,
    error);
  if (_ret == NULL)
    goto _out;
  g_variant_get (_ret,
                 "(u^as)",
                 out_count,
                 out_ids);
  g_variant_unref (_ret);
_out:
  return _ret != NULL;
}
