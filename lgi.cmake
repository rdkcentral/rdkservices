# Broadcom specifics
message("Building for Onemw...")

add_definitions (-DUSE_IARM)
option(USE_IARM "USE_IARM" ON)
add_definitions (-DUSE_DEVICE_SETTINGS_SERVICE)
add_definitions (-DRDK_USE_NXCLIENT)
add_definitions (-DDISABLE_PRE_RES_CHANGE_EVENTS)
