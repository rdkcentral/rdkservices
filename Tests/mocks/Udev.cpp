/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright 2023 Synamedia
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
**/

#include "Udev.h"
#include <gmock/gmock.h>

UdevImpl* Udev::impl = nullptr;

Udev::Udev() {}

 void Udev::setImpl(UdevImpl* newImpl)
{
    // Handles both resetting 'impl' to nullptr and assigning a new value to 'impl'
    EXPECT_TRUE ((nullptr == impl) || (nullptr == newImpl));
    impl = newImpl;
}

struct udev* Udev::udev_new()
{
    EXPECT_NE(impl, nullptr);
    return impl->udev_new();
}

udev_enumerate* Udev::udev_enumerate_new(struct udev* udev)
{
    EXPECT_NE(impl, nullptr);
    return impl->udev_enumerate_new(udev);
}

void Udev::udev_enumerate_add_match_subsystem(struct udev_enumerate* enumerate, const char* subsystem)
{
    EXPECT_NE(impl, nullptr);
    return impl->udev_enumerate_add_match_subsystem(enumerate, subsystem);
}

void Udev::udev_enumerate_scan_devices(struct udev_enumerate* enumerate)
{
    EXPECT_NE(impl, nullptr);
    return impl->udev_enumerate_scan_devices(enumerate);
}

struct udev_list_entry* Udev::udev_enumerate_get_list_entry(struct udev_enumerate* enumerate)
{
    EXPECT_NE(impl, nullptr);
    return impl->udev_enumerate_get_list_entry(enumerate);
}

void Udev::udev_enumerate_unref(struct udev_enumerate* enumerate)
    {
    EXPECT_NE(impl, nullptr);
    return impl->udev_enumerate_unref(enumerate);
}

void Udev::udev_unref(struct udev* udev)
{
    EXPECT_NE(impl, nullptr);
    return impl->udev_unref(udev);
}

const char* Udev::udev_list_entry_get_name(struct udev_list_entry* entry)
{
    EXPECT_NE(impl, nullptr);
    return impl->udev_list_entry_get_name(entry);
}

struct udev_device* Udev::udev_device_new_from_syspath(struct udev* udev, const char* path)
{
    EXPECT_NE(impl, nullptr);
    return impl->udev_device_new_from_syspath(udev, path);
}

void Udev::udev_device_unref(struct udev_device* dev)
{
    EXPECT_NE(impl, nullptr);
    return impl->udev_device_unref(dev);
}

struct udev_device* Udev::udev_device_get_parent_with_subsystem_devtype(struct udev_device* dev, const char* subsystem, const char* devtype)
{
    EXPECT_NE(impl, nullptr);
    return impl->udev_device_get_parent_with_subsystem_devtype(dev, subsystem, devtype);
}

const char* Udev::udev_device_get_devtype(struct udev_device* dev)
{
    EXPECT_NE(impl, nullptr);
    return impl->udev_device_get_devtype(dev);
}

const char* Udev::udev_device_get_devnode(struct udev_device* dev)
{
    EXPECT_NE(impl, nullptr);
    return impl->udev_device_get_devnode(dev);
}

struct udev* (*udev_new)() = &Udev::udev_new;
udev_enumerate* (*udev_enumerate_new)(struct udev*) = &Udev::udev_enumerate_new;
void (*udev_enumerate_add_match_subsystem)(struct udev_enumerate*, const char*) = &Udev::udev_enumerate_add_match_subsystem;
void (*udev_enumerate_scan_devices)(struct udev_enumerate*) = &Udev::udev_enumerate_scan_devices;
struct udev_list_entry* (*udev_enumerate_get_list_entry)(struct udev_enumerate*) = &Udev::udev_enumerate_get_list_entry;
void (*udev_enumerate_unref)(struct udev_enumerate*) = &Udev::udev_enumerate_unref;
void (*udev_unref)(struct udev*) = &Udev::udev_unref;
const char* (*udev_list_entry_get_name)(struct udev_list_entry*) = &Udev::udev_list_entry_get_name;
struct udev_device* (*udev_device_new_from_syspath)(struct udev*, const char*) = &Udev::udev_device_new_from_syspath;
void (*udev_device_unref)(struct udev_device*) = &Udev::udev_device_unref;
struct udev_device* (*udev_device_get_parent_with_subsystem_devtype)(struct udev_device*, const char*, const char*) = &Udev::udev_device_get_parent_with_subsystem_devtype;
const char* (*udev_device_get_devtype)(struct udev_device*) = &Udev::udev_device_get_devtype;
const char* (*udev_device_get_devnode)(struct udev_device*) = &Udev::udev_device_get_devnode;
