
#include "opkg/opkg.h"
#include "opkg/opkg_conf.h"

static opkg_conf_t _conf;
opkg_conf_t *opkg_config = &_conf;

int opkg_update_package_lists(opkg_progress_callback_t progress_callback, void *user_data)
{
	return 0;
}

int opkg_install_package(const char *package_url, opkg_progress_callback_t progress_callback,void *user_data)
{
   return 0;
}

void opkg_conf_deinit(void)
{
	return;
}

int opkg_compare_versions(const char *ver1, const char *ver2)
{
	return 0;
}

int opkg_upgrade_package(const char *package_name, opkg_progress_callback_t callback, void *user_data)
{
	return 0;
}

int opkg_new(void)
{
	return 0;
}

void opkg_download_cleanup(void)
{
	return;
}

int opkg_list_upgradable_packages(opkg_package_callback_t callback, void *user_data)
{
        return 0;
}