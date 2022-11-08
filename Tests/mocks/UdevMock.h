#pragma once

#include <gmock/gmock.h>

class UdevMock {
public:
    static UdevMock* instance{ nullptr };
    UdevMock()
    {
        instance = this;
    }
    virtual ~UdevMock()
    {
        instance = nullptr;
    }

    MOCK_METHOD(struct udev*, udev_new, ());
    MOCK_METHOD(struct udev_enumerate*, udev_enumerate_new, (struct udev * udev));
    MOCK_METHOD(void, udev_enumerate_add_match_subsystem, (struct udev_enumerate * enumerate, const char* subsystem));
    MOCK_METHOD(void, udev_enumerate_scan_devices, (struct udev_enumerate * enumerate));
    MOCK_METHOD(struct udev_list_entry*, udev_enumerate_get_list_entry, (struct udev_enumerate * enumerate));
    MOCK_METHOD(void, udev_enumerate_unref, (struct udev_enumerate * enumerate));
    MOCK_METHOD(void, udev_unref, (struct udev * udev));
    MOCK_METHOD(const char*, udev_list_entry_get_name, (struct udev_list_entry * entry));
    MOCK_METHOD(struct udev_device*, udev_device_new_from_syspath, (struct udev * udev, const char* path));
    MOCK_METHOD(void, udev_device_unref, (struct udev_device * dev));
    MOCK_METHOD(struct udev_device*, udev_device_get_parent_with_subsystem_devtype, (struct udev_device * dev, const char* subsystem, const char* devtype));
    MOCK_METHOD(const char*, udev_device_get_devtype, (struct udev_device * dev));
    MOCK_METHOD(const char*, udev_device_get_devnode, (struct udev_device * dev));

    static struct udev* _udev_new()
    {
        return instance->udev_new();
    }

    static udev_enumerate* _udev_enumerate_new(struct udev* udev)
    {
        return instance->udev_enumerate_new(udev);
    }

    static void _udev_enumerate_add_match_subsystem(struct udev_enumerate* enumerate, const char* subsystem)
    {
        return instance->udev_enumerate_add_match_subsystem(enumerate, subsystem);
    }

    static void _udev_enumerate_scan_devices(struct udev_enumerate* enumerate)
    {
        return instance->udev_enumerate_scan_devices(enumerate);
    }

    static struct _udev_list_entry* udev_enumerate_get_list_entry(struct udev_enumerate* enumerate)
    {
        return instance->udev_enumerate_get_list_entry(enumerate);
    }

    static void _udev_enumerate_unref(struct udev_enumerate* enumerate)
    {
        return instance->udev_enumerate_unref(enumerate);
    }

    static void _udev_unref(struct udev* udev)
    {
        return instance->udev_unref(udev);
    }

    static const char* _udev_list_entry_get_name(struct udev_list_entry* entry)
    {
        return instance->udev_list_entry_get_name(entry);
    }

    static struct _udev_device* udev_device_new_from_syspath(struct udev* udev, const char* path)
    {
        return instance->udev_device_new_from_syspath(udev, path);
    }

    static void _udev_device_unref(struct udev_device* dev)
    {
        return instance->udev_device_unref(dev);
    }

    static struct udev_device* _udev_device_get_parent_with_subsystem_devtype(struct udev_device* dev, const char* subsystem, const char* devtype)
    {
        return instance->udev_device_get_parent_with_subsystem_devtype(dev, subsystem, devtype);
    }

    static const char* _udev_device_get_devtype(struct udev_device* dev)
    {
        return instance->udev_device_get_devtype(dev);
    }

    static const char* _udev_device_get_devnode(struct udev_device* dev)
    {
        return instance->udev_device_get_devnode(dev);
    }
};

constexpr auto udev_new = &UdevMock::_udev_new;
constexpr auto udev_enumerate_new = &UdevMock::_udev_enumerate_new;
constexpr auto udev_enumerate_add_match_subsystem = &UdevMock::_udev_enumerate_add_match_subsystem;
constexpr auto udev_enumerate_scan_devices = &UdevMock::_udev_enumerate_scan_devices;
constexpr auto udev_enumerate_get_list_entry = &UdevMock::_udev_enumerate_get_list_entry;
constexpr auto udev_enumerate_unref = &UdevMock::_udev_enumerate_unref;
constexpr auto udev_unref = &UdevMock::_udev_unref;
constexpr auto udev_list_entry_get_name = &UdevMock::_udev_list_entry_get_name;
constexpr auto udev_device_new_from_syspath = &UdevMock::_udev_device_new_from_syspath;
constexpr auto udev_device_unref = &UdevMock::_udev_device_unref;
constexpr auto udev_device_get_parent_with_subsystem_devtype = &UdevMock::_udev_device_get_parent_with_subsystem_devtype;
constexpr auto udev_device_get_devtype = &UdevMock::_udev_device_get_devtype;
constexpr auto udev_device_get_devnode = &UdevMock::_udev_device_get_devnode;
