#pragma once

struct udev {
};

struct udev_enumerate {
};

struct udev_list_entry {
};

struct udev_device {
};

#define udev_list_entry_foreach(list_entry, first_entry) \
    for (list_entry = first_entry;                       \
         list_entry;                                     \
         list_entry = nullptr)

extern struct udev* udev_new();
extern udev_enumerate* udev_enumerate_new(struct udev* udev);
extern void udev_enumerate_add_match_subsystem(struct udev_enumerate* enumerate, const char* subsystem);
extern void udev_enumerate_scan_devices(struct udev_enumerate* enumerate);
extern struct udev_list_entry* udev_enumerate_get_list_entry(struct udev_enumerate* enumerate);
extern void udev_enumerate_unref(struct udev_enumerate* enumerate);
extern void udev_unref(struct udev* udev);
extern const char* udev_list_entry_get_name(struct udev_list_entry* entry);
extern struct udev_device* udev_device_new_from_syspath(struct udev* udev, const char* path);
extern void udev_device_unref(struct udev_device* dev);
extern struct udev_device* udev_device_get_parent_with_subsystem_devtype(struct udev_device* dev, const char* subsystem, const char* devtype);
extern const char* udev_device_get_devtype(struct udev_device* dev);
extern const char* udev_device_get_devnode(struct udev_device* dev);
