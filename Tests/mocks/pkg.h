
#ifndef OPKG_H
#define OPKG_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct opkg_conf {

    char *conf_file;
    char *tmp_dir;
    char *cache_dir;
    unsigned int pfm;       /* package field mask */
    int check_pkg_signature;
    char *signature_type;
    int nodeps;             /* do not follow dependencies */
    int verbosity;
    int volatile_cache;
    int host_cache_dir;
	char *lists_dir;

} opkg_conf_t;

struct pkg {
    char *name;
	char *local_filename;
	char *version;
	
};
typedef struct pkg pkg_t;

extern opkg_conf_t *opkg_config;

struct _opkg_progress_data_t {
    int percentage;
    int action;
    pkg_t *pkg;
};

enum _opkg_action_t {
    OPKG_INSTALL,
    OPKG_REMOVE,
    OPKG_DOWNLOAD
};

typedef struct _opkg_progress_data_t opkg_progress_data_t;
typedef void (*opkg_package_callback_t) (pkg_t * pkg, void *user_data);
typedef void (*opkg_progress_callback_t) (const opkg_progress_data_t *
                                          progress, void *user_data);
										  
int opkg_list_upgradable_packages(opkg_package_callback_t callback,
                                  void *user_data);
int opkg_install_package(const char *package_url,
                         opkg_progress_callback_t callback,
                         void *user_data);
int opkg_update_package_lists(opkg_progress_callback_t progress_callback, void *user_data);
void opkg_conf_deinit(void);
int opkg_compare_versions(const char *ver1, const char *ver2);
int opkg_upgrade_package(const char *package_name, opkg_progress_callback_t callback, void *user_data);
int opkg_new(void);
void opkg_download_cleanup(void);
int opkg_list_upgradable_packages(opkg_package_callback_t callback, void *user_data);

#ifdef __cplusplus
}
#endif
#endif