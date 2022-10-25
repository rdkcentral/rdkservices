#pragma once

struct udev {
};

struct udev_enumerate {
};

struct udev_list_entry {
};

struct udev_device {
};

class UdevImpl {
public:
    virtual ~UdevImpl() = default;

    virtual struct udev* udev_new() = 0;
    virtual struct udev_enumerate* udev_enumerate_new(struct udev* udev) = 0;
    virtual void udev_enumerate_add_match_subsystem(struct udev_enumerate* enumerate, const char* subsystem) = 0;
    virtual void udev_enumerate_scan_devices(struct udev_enumerate* enumerate) = 0;
    virtual struct udev_list_entry* udev_enumerate_get_list_entry(struct udev_enumerate* enumerate) = 0;
    virtual void udev_enumerate_unref(struct udev_enumerate* enumerate) = 0;
    virtual void udev_unref(struct udev* udev) = 0;
    virtual const char* udev_list_entry_get_name(struct udev_list_entry* entry) = 0;
    virtual struct udev_device* udev_device_new_from_syspath(struct udev* udev, const char* path) = 0;
    virtual void udev_device_unref(struct udev_device* dev) = 0;
    virtual struct udev_device* udev_device_get_parent_with_subsystem_devtype(struct udev_device* dev, const char* subsystem, const char* devtype) = 0;
    virtual const char* udev_device_get_devtype(struct udev_device* dev) = 0;
    virtual const char* udev_device_get_devnode(struct udev_device* dev) = 0;
};

class Udev {
public:
    static Udev& getInstance()
    {
        static Udev instance;
        return instance;
    }

    UdevImpl* impl;

    static struct udev* udev_new()
    {
        return getInstance().impl->udev_new();
    }

    static udev_enumerate* udev_enumerate_new(struct udev* udev)
    {
        return getInstance().impl->udev_enumerate_new(udev);
    }

    static void udev_enumerate_add_match_subsystem(struct udev_enumerate* enumerate, const char* subsystem)
    {
        return getInstance().impl->udev_enumerate_add_match_subsystem(enumerate, subsystem);
    }

    static void udev_enumerate_scan_devices(struct udev_enumerate* enumerate)
    {
        return getInstance().impl->udev_enumerate_scan_devices(enumerate);
    }

    static struct udev_list_entry* udev_enumerate_get_list_entry(struct udev_enumerate* enumerate)
    {
        return getInstance().impl->udev_enumerate_get_list_entry(enumerate);
    }

    static void udev_enumerate_unref(struct udev_enumerate* enumerate)
    {
        return getInstance().impl->udev_enumerate_unref(enumerate);
    }

    static void udev_unref(struct udev* udev)
    {
        return getInstance().impl->udev_unref(udev);
    }

    static const char* udev_list_entry_get_name(struct udev_list_entry* entry)
    {
        return getInstance().impl->udev_list_entry_get_name(entry);
    }

    static struct udev_device* udev_device_new_from_syspath(struct udev* udev, const char* path)
    {
        return getInstance().impl->udev_device_new_from_syspath(udev, path);
    }

    static void udev_device_unref(struct udev_device* dev)
    {
        return getInstance().impl->udev_device_unref(dev);
    }

    static struct udev_device* udev_device_get_parent_with_subsystem_devtype(struct udev_device* dev, const char* subsystem, const char* devtype)
    {
        return getInstance().impl->udev_device_get_parent_with_subsystem_devtype(dev, subsystem, devtype);
    }

    static const char* udev_device_get_devtype(struct udev_device* dev)
    {
        return getInstance().impl->udev_device_get_devtype(dev);
    }

    static const char* udev_device_get_devnode(struct udev_device* dev)
    {
        return getInstance().impl->udev_device_get_devnode(dev);
    }
};

constexpr auto udev_new = &Udev::udev_new;
constexpr auto udev_enumerate_new = &Udev::udev_enumerate_new;
constexpr auto udev_enumerate_add_match_subsystem = &Udev::udev_enumerate_add_match_subsystem;
constexpr auto udev_enumerate_scan_devices = &Udev::udev_enumerate_scan_devices;
constexpr auto udev_enumerate_get_list_entry = &Udev::udev_enumerate_get_list_entry;
constexpr auto udev_enumerate_unref = &Udev::udev_enumerate_unref;
constexpr auto udev_unref = &Udev::udev_unref;
constexpr auto udev_list_entry_get_name = &Udev::udev_list_entry_get_name;
constexpr auto udev_device_new_from_syspath = &Udev::udev_device_new_from_syspath;
constexpr auto udev_device_unref = &Udev::udev_device_unref;
constexpr auto udev_device_get_parent_with_subsystem_devtype = &Udev::udev_device_get_parent_with_subsystem_devtype;
constexpr auto udev_device_get_devtype = &Udev::udev_device_get_devtype;
constexpr auto udev_device_get_devnode = &Udev::udev_device_get_devnode;

#define udev_list_entry_foreach(list_entry, first_entry) \
    for (list_entry = first_entry;                       \
         list_entry;                                     \
         list_entry = nullptr)
