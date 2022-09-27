#pragma once

#include <gmock/gmock.h>

#include "libudev.h"

class UdevImplMock : public UdevImpl {
public:
    virtual ~UdevImplMock() = default;

    MOCK_METHOD(struct udev*, udev_new, (), (override));
    MOCK_METHOD(struct udev_enumerate*, udev_enumerate_new, (struct udev * udev), (override));
    MOCK_METHOD(void, udev_enumerate_add_match_subsystem, (struct udev_enumerate * enumerate, const char* subsystem), (override));
    MOCK_METHOD(void, udev_enumerate_scan_devices, (struct udev_enumerate * enumerate), (override));
    MOCK_METHOD(struct udev_list_entry*, udev_enumerate_get_list_entry, (struct udev_enumerate * enumerate), (override));
    MOCK_METHOD(void, udev_enumerate_unref, (struct udev_enumerate * enumerate), (override));
    MOCK_METHOD(void, udev_unref, (struct udev * udev), (override));
    MOCK_METHOD(const char*, udev_list_entry_get_name, (struct udev_list_entry * entry), (override));
    MOCK_METHOD(struct udev_device*, udev_device_new_from_syspath, (struct udev * udev, const char* path), (override));
    MOCK_METHOD(void, udev_device_unref, (struct udev_device * dev), (override));
    MOCK_METHOD(struct udev_device*, udev_device_get_parent_with_subsystem_devtype, (struct udev_device * dev, const char* subsystem, const char* devtype), (override));
    MOCK_METHOD(const char*, udev_device_get_devtype, (struct udev_device * dev), (override));
    MOCK_METHOD(const char*, udev_device_get_devnode, (struct udev_device * dev), (override));
};
