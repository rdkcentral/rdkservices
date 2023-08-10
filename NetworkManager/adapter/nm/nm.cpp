#include <glib.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <NetworkManager.h>
#include <stdbool.h>

#define PREFIX_TO_NETMASK(prefix_len) ({ \
    static char netmask_str[16]; \
    uint32_t netmask = 0xffffffff << (32 - (prefix_len)); \
    snprintf(netmask_str, 16, "%u.%u.%u.%u", \
             (netmask >> 24) & 0xff, \
             (netmask >> 16) & 0xff, \
             (netmask >> 8) & 0xff, \
             netmask & 0xff); \
    netmask_str; \
})

NMClient *client = NULL;

int isInterfaceEnabled(char * interface); 
int getinterfaces(void);
int isConnectedToInternet(void);
int getDefaultInterface(void);
int getIpSettings(void);
char* getQuirks(void);
char* getNamedEndpoints(void);
int getStbIp(void);
int getStbIpFamily(char * f);
int setDefaultInterface(char* interface, bool enable, bool persist);
void subscribeEvents(void);
static bool ip_cache = false;
void *eventHandler(void*arg);
void subscribeToEvents();

static void e_user_function(NMDevice  *device,
        guint Nstate,
        guint Ostate,
        guint reason,
        gpointer user_data)
{
    printf("E Connection state changed: Ostate=%d, Nstate = %d, reason=%d\n", Ostate,Nstate, reason);
    if(Nstate == NM_DEVICE_STATE_ACTIVATED){
        printf(" trigger callback for onConnectionStatusChanged interface : ETH and STATUS : CONNECTED\n");
        /*also check if the ip in the cache and the current ip are same.  if not trigger onIPAddressStatusChanged */
        if(ip_cache == false){
            /*First time the IP is being saved to CACHE do not trigger onIPAddressStatusChanged */
            ip_cache = true;
            /*save ip to cache*/
        }
        else{
            /*Compare if the ip assigned is new.  if not triiger callback for onIPAddressStatusChanged*/
            /*Now save the IP address to cache*/
            ip_cache = true;

        }
    }
    if((Nstate != NM_DEVICE_STATE_ACTIVATED) && (Ostate == NM_DEVICE_STATE_ACTIVATED)){
        printf(" trigger callback for onConnectionStatusChanged interface : ETH and STATUS : DISCONNECTED\n");
        /*delete the IP address stored in the cache */
        //ip_cache = false;
    }

}
static void w_user_function(NMDevice  *device,
        guint Nstate,
        guint Ostate,
        guint reason,
        gpointer user_data)
{
    printf("W Connection state changed: Ostate=%d, Nstate = %d, reason=%d\n", Ostate,Nstate, reason);
    /***************************** START onConnectionStatusChanged**************************************/
    if(Nstate == NM_DEVICE_STATE_ACTIVATED)
        printf(" trigger callback for onConnectionStatusChanged interface : WIFI and STATUS : CONNECTED\n");
    if((Nstate != NM_DEVICE_STATE_ACTIVATED) && (Ostate == NM_DEVICE_STATE_ACTIVATED))
        printf(" trigger callback for onConnectionStatusChanged interface : WIFI and STATUS : DISCONNECTED\n");
    /***************************** END onConnectionStatusChanged**************************************/
    /***************************** START onConnectionStatusChanged**************************************/

}

void *eventHandler(void*arg)
{
    NMDevice *e_device;
    NMDevice *w_device;
    NMActiveConnection *active_connection;
    guint e_sig_id;
    guint w_sig_id;
    GMainLoop *loop = (GMainLoop *)arg;
    /* Get the device object for interface "eth0" */
    e_device = nm_client_get_device_by_iface(client, "eno1");
    if (e_device == NULL) {
        return NULL;
    }
    printf("e_device = %p\n", e_device);
    /* Register a signal handler for the "state-changed" signal */
    e_sig_id = g_signal_connect(e_device,
            "state-changed",
            G_CALLBACK(e_user_function),
            NULL);

    printf("sig_id = %d\n", e_sig_id);
    /* Get the device object for interface "eth0" */
    w_device = nm_client_get_device_by_iface(client, "wlxc4e90a09837c");
    if (w_device == NULL) {
        return NULL;
    }
    printf("w_device = %p\n", w_device);
    /* Register a signal handler for the "state-changed" signal */
    w_sig_id = g_signal_connect(w_device,
            "state-changed",
            G_CALLBACK(w_user_function),
            NULL);

    printf("w_sig_id = %d\n", w_sig_id);
    /* Start the main loop */
    g_main_loop_run(loop);


    /* Clean up */
    g_main_loop_unref(loop);
}

void subscribeEvents()
{
    pthread_t tid;
    char a = '0';
    GMainLoop *loop;
    /* Crg_signal_handler_disconnecteate a new main loop */
    loop = g_main_loop_new(NULL, FALSE);
    int retVal = pthread_create(&tid,NULL, &eventHandler,(void*)loop);

    while(a!= 'x'){
        printf("value = %c, press x to exit\n",a);
        //sleep(3);
        scanf("%c",&a);
    }
    g_main_loop_quit(loop);
    pthread_join(tid,NULL);
}

int setDefaultInterface(char* interface, bool enable, bool persist)
{
return 0;

}
int getStbIp(void)
{
    GError *error = NULL;
    NMActiveConnection *active_conn = NULL;
    NMIPConfig *ipconfig = NULL;

    active_conn = nm_client_get_primary_connection(client);
    if (active_conn == NULL) {
        fprintf(stderr, "Error getting primary connection: %s\n", error->message);
        g_error_free(error);
        return 1;
    }

    ipconfig = nm_active_connection_get_ip4_config(active_conn);
    if (ipconfig != NULL) {
        const GPtrArray *p;
        int              i;
        p = nm_ip_config_get_addresses(ipconfig);
        for (i = 0; i < p->len; i++) {
            NMIPAddress *a = p->pdata[i];
            g_print("\tinet4 %s/%d\n", nm_ip_address_get_address(a), nm_ip_address_get_prefix(a));
        }
    }

    return 0;
}

int getStbIpFamily(char*f)
{
    GError *error = NULL;
    NMActiveConnection *active_conn = NULL;
    NMIPConfig *ipconfig = NULL;

    active_conn = nm_client_get_primary_connection(client);
    if (active_conn == NULL) {
        fprintf(stderr, "Error getting primary connection: %s\n", error->message);
        g_error_free(error);
        return 1;
    }

    if(strcmp(f,"AF_INET")==0){
        printf("Requested for IPV4 family \n");
    }else if(strcmp(f,"AF_INET6")==0){
        printf("Requested for IPV6 family \n");
    }
    ipconfig = nm_active_connection_get_ip4_config(active_conn);
    if (ipconfig != NULL) {
        const GPtrArray *arr;
        int count;
        arr = nm_ip_config_get_addresses(ipconfig);
        for (count = 0; count < arr->len; count++) {
            NMIPAddress *e = arr->pdata[count];
            printf("\tinet4 %s/%d\n", nm_ip_address_get_address(e), nm_ip_address_get_prefix(e));
        }
    }
    ipconfig = nm_active_connection_get_ip6_config(active_conn);
    if (ipconfig != NULL) {
        const GPtrArray *arr;
        int count;
        arr = nm_ip_config_get_addresses(ipconfig);
        for (count = 0; count < arr->len; count++) {
            NMIPAddress *e = arr->pdata[count];
            printf("\tinet4 %s/%d\n", nm_ip_address_get_address(e), nm_ip_address_get_prefix(e));
        }
    }

    return 0;
}
int isInterfaceEnabled(char * interface) 
{
    NMDevice *device ;
    NMDeviceState state;

    const char *iface_name = NULL; // replace with your interface name

    if (!(strcmp (interface, "ETHERNET") == 0 || strcmp (interface, "WIFI") == 0))
    {
        printf("failed due to invalid interface [%s]", interface);
        return -1;
    }
    if(strcmp (interface, "ETHERNET") == 0){
        iface_name = "eno1";  /*TODO get value using getenvOrDefault */
        device = nm_client_get_device_by_iface(client, iface_name);
        if (device == NULL) {
            printf("ethernet interface not available\n");
        } else {
            printf("ethernet interface available\n");
        }
        // check the state of the device
        state = nm_device_get_state(device);
        if (state == NM_DEVICE_STATE_UNMANAGED || 
            state == NM_DEVICE_STATE_UNKNOWN   ||
            state == NM_DEVICE_STATE_UNAVAILABLE ) {
            printf("Interface %s is disabled\n", iface_name);
        }
        else{
            printf("Interface %s is enabled\n", iface_name);
        }
    }
    else if(strcmp (interface, "WIFI") == 0){
        iface_name = "wlxc4e90a09837c";  /*TODO get value using getenvOrDefault */
        device  = nm_client_get_device_by_iface(client,iface_name );
        if (device == NULL) {
            printf("WiFi interface not available\n");
        } else {
            printf("WiFi interface available\n");
        }
        // check the state of the device
        state = nm_device_get_state(device);
        if (state == NM_DEVICE_STATE_UNMANAGED || 
            state == NM_DEVICE_STATE_UNKNOWN   ||
            state == NM_DEVICE_STATE_UNAVAILABLE ) {
            printf("Interface %s is disabled\n", iface_name);
        }
        else{
            printf("Interface %s is enabled\n", iface_name);
        }

    }

    printf(" connectivity = internet %s state = %d \n", (nm_client_get_connectivity(client) >2)?"Connected":"Disconnected", state);


    return 0; 
}

int getinterfaces()
{
    const GPtrArray *devices = nm_client_get_devices(client);
    for (guint i = 0; i < devices->len; i++) {
        NMDevice *device = g_ptr_array_index(devices, i);
        const char *interface_name = nm_device_get_iface(device);
        const char *hw_address = nm_device_get_hw_address(device);
        NMDeviceState state = nm_device_get_state(device);
        const char *state_str = (state > NM_DEVICE_STATE_UNAVAILABLE) ? "interface:enabled" : "interface:disabled";
        NMConnectivityState connectivity  = nm_device_get_connectivity (device, AF_INET);
        const char *conn_str = (connectivity > NM_CONNECTIVITY_LIMITED) ? "connected:true" : "connected:false";

        printf("Interface %u: %s (%s, %s, %s )\n", i, interface_name, hw_address, state_str, conn_str);
    }
    return 0;
}

int isConnectedToInternet()
{

    printf(" connectivity = internet %s\n", (nm_client_get_connectivity(client) >2)?"Connected":"Disconnected");
    return 0;

}

int getDefaultInterface()
{
    GError *error = NULL;
    NMActiveConnection *active_conn = NULL;
    NMRemoteConnection *newconn = NULL;
    NMSettingConnection *s_con;
    const char *interface_name = NULL;

    active_conn = nm_client_get_primary_connection(client);
    if (active_conn == NULL) {
        fprintf(stderr, "Error getting primary connection: %s\n", error->message);
        g_error_free(error);
        return 1;
    }
    newconn = nm_active_connection_get_connection(active_conn);
    s_con = nm_connection_get_setting_connection(NM_CONNECTION(newconn));

    interface_name = nm_connection_get_interface_name(NM_CONNECTION(newconn));
    printf("Interface name: %s\n", interface_name);
    if (g_strcmp0(nm_setting_connection_get_connection_type(s_con), NM_SETTING_WIRELESS_SETTING_NAME) == 0) {
        printf("default interface = %s(%s)\n", "WIFI", NM_SETTING_WIRELESS_SETTING_NAME);
    }
    else if (g_strcmp0(nm_setting_connection_get_connection_type(s_con), NM_SETTING_WIRED_SETTING_NAME) == 0) {
        printf("default interface = %s(%s)\n", "ETHERNET", NM_SETTING_WIRED_SETTING_NAME);
    }

    return 0;
}

int getIpSettings()
{

    char* f = "AF_INET";
    GError *error = NULL;
    NMActiveConnection *active_conn = NULL;
    NMIPConfig *ip4_config = NULL;
    NMIPConfig *ip6_config = NULL;
    const char *gateway = NULL;
	char **dns_arr = NULL;
    NMDhcpConfig *dhcp4_config = NULL;
    NMDhcpConfig *dhcp6_config = NULL;
    GHashTable * ght ;

    active_conn = nm_client_get_primary_connection(client);
    if (active_conn == NULL) {
        fprintf(stderr, "Error getting primary connection: %s\n", error->message);
        g_error_free(error);
        return 1;
    }

    if(strcmp(f,"AF_INET")==0){
        printf("Requested for IPV4 family \n");
    }else if(strcmp(f,"AF_INET6")==0){
        printf("Requested for IPV6 family \n");
    }
    printf("********IPv4 *********\n");
    ip4_config = nm_active_connection_get_ip4_config(active_conn);
    if (ip4_config != NULL) {
        const GPtrArray *p;
        int              i;
        p = nm_ip_config_get_addresses(ip4_config);
        for (i = 0; i < p->len; i++) {
            NMIPAddress *a = p->pdata[i];
            g_print("\tinet4 %s/%d, %s \n", nm_ip_address_get_address(a), nm_ip_address_get_prefix(a), PREFIX_TO_NETMASK( nm_ip_address_get_prefix(a)));
        }
        gateway = nm_ip_config_get_gateway(ip4_config);
        printf("Gateway = %s \n", gateway);
    }
    dns_arr =   (char **)nm_ip_config_get_nameservers(ip4_config);
    if((*(&dns_arr[0]))!=NULL )
    printf("Primary DNS: %s\n", *(&dns_arr[0]));
    if((*(&dns_arr[1]))!=NULL )
        printf("Secondary DNS: %s\n", *(&dns_arr[1]));
    
    dhcp4_config = nm_active_connection_get_dhcp4_config(active_conn);
    ght = nm_dhcp_config_get_options(dhcp4_config);
    printf("dhcp 4= %s , \n",(char *)g_hash_table_lookup(ght,"dhcp_server_identifier") );

    printf("********IPv6 *********\n");
    ip6_config = nm_active_connection_get_ip6_config(active_conn);
    if (ip6_config != NULL) {
        const GPtrArray *p;
        int              i;
        p = nm_ip_config_get_addresses(ip6_config);
        for (i = 0; i < p->len; i++) {
            NMIPAddress *a = p->pdata[i];
            g_print("\tinet6 %s/%d\n", nm_ip_address_get_address(a), nm_ip_address_get_prefix(a));
        }
        gateway = nm_ip_config_get_gateway(ip6_config);
        printf("Gateway = %s \n", gateway);

    dns_arr =   (char **)nm_ip_config_get_nameservers(ip6_config);
    if((*(&dns_arr[0]))!=NULL )
    printf("6 primary DNS: %s\n", *(&dns_arr[0]));
    if((*(&dns_arr[1]))!=NULL )
    printf("6 Secondary DNS: %s\n", *(&dns_arr[1]));

    dhcp6_config = nm_active_connection_get_dhcp6_config(active_conn);
    ght = nm_dhcp_config_get_options(dhcp6_config);
    printf("dhcp 6= %s , \n",(char *)g_hash_table_lookup(ght,"dhcp_server_identifier"));

    }

    return 0;

}

char* getNamedEndpoints()
{
    return "CMTS";
#if 0
    uint32_t Network::getNamedEndpoints(const JsonObject& parameters, JsonObject& response)
    {
        JsonArray namedEndpoints;
        namedEndpoints.Add("CMTS");

        response["endpoints"] = namedEndpoints;
        returnResponse(true)
    }
#endif
}

char* getQuirks(void)
{
    return "RDK-20093";
#if 0
uint32_t Network::getQuirks(const JsonObject& parameters, JsonObject& response)
{
JsonArray array;
array.Add("RDK-20093");
response["quirks"] = array;
returnResponse(true)
}
#endif
}


int main(int argc, char *argv[]) {
    GError *error = NULL;
    NMDevice *device = NULL;
    const char *iface_name = "eno1"; // replace with your interface name
    NMDeviceState state;


    // initialize the NMClient object
    client = nm_client_new(NULL, &error);
    if (client == NULL) {
        fprintf(stderr, "Error initializing NMClient: %s\n", error->message);
        g_error_free(error);
        return EXIT_FAILURE;
    }

    // get the device object for the specified interface name
    device = nm_client_get_device_by_iface(client, iface_name);
    if (device == NULL) {
        fprintf(stderr, "Error getting device for interface %s\n", iface_name);
        return EXIT_FAILURE;
    }

    // check the state of the device
    state = nm_device_get_state(device);
    if (state == NM_DEVICE_STATE_ACTIVATED) {
        printf("Interface %s is enabled and active.\n", iface_name);
    } else if (state == NM_DEVICE_STATE_UNMANAGED) {
        printf("Interface %s is unmanaged.\n", iface_name);
    } else if (state == NM_DEVICE_STATE_DISCONNECTED) {
        printf("Interface %s is enabled but not active.\n", iface_name);
    } else {
        printf("Interface %s is in an unknown state.\n", iface_name);
    }


    switch(1) {
	case 1:
        printf("\n\n\nisInterfaceEnabled\n");
		isInterfaceEnabled( "ETHERNET");
		isInterfaceEnabled( "WIFI");

        printf("\n\n\n  getinterfaces\n");
        getinterfaces();

        printf("\n\n\n isConnectedToInternet\n");
        isConnectedToInternet();

        printf("\n\n\n getDefaultInterface\n");
        getDefaultInterface();

        printf("\n\n\n getQuirks\n");
        printf(" getQuirks = %s\n", getQuirks());

        printf("\n\n\n getNamedEndpoints\n");
        printf(" getNamedEndpoints = %s\n", getNamedEndpoints());


        printf("\n\n\n getStbIp\n");
        getStbIp();
        getStbIpFamily("AF_INET");
        getStbIpFamily("AF_INET6");

        printf("\n\n\n getIpSettings \n");
        getIpSettings();
        printf("\n\n\n setDefaultInterface\n");
        setDefaultInterface("WIFI", TRUE, TRUE);

        printf("\n\n subscribe for events ");
        subscribeEvents();
    }


   // clean up
    //g_object_unref(device);
    g_object_unref(client);

    return EXIT_SUCCESS;
}

