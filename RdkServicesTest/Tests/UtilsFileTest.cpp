/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2022 RDK Management
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
 */

#include <gtest/gtest.h>

#include "Module.h"

#include "UtilsFile.h"

namespace {
const string testFolder = _T("/tmp/UtilsFileTest");
const string fileFrom = _T("/tmp/UtilsFileTest/file");
const string fileTo = _T("/tmp/UtilsFileTest/destination/for/new/file");

const uint32_t numBytes = 12;
const uint8_t bytes[numBytes] = { 0x00, 0x01, 0x00, 0x00, 0x00, 0x06, 0xFE, 0x03, 0x20, 0x04, 0x00, 0x01 };
}

using namespace WPEFramework;

TEST(UtilsFileTest, createFolder)
{
    EXPECT_TRUE(Core::Directory(testFolder.c_str()).Destroy(false));

    EXPECT_TRUE(Core::Directory(testFolder.c_str()).CreatePath());
    EXPECT_TRUE(Core::File(testFolder).Exists());
}

TEST(UtilsFileTest, createFile)
{
    Core::File file(fileFrom);

    EXPECT_FALSE(file.Exists());
    EXPECT_TRUE(file.Create());
    EXPECT_EQ(numBytes, file.Write(bytes, numBytes));
}

TEST(UtilsFileTest, moveFile)
{
    EXPECT_TRUE(Core::File(fileFrom).Exists());
    EXPECT_FALSE(Core::File(fileTo).Exists());

    EXPECT_TRUE(Utils::MoveFile(fileFrom, fileTo));

    EXPECT_FALSE(Core::File(fileFrom).Exists());
    EXPECT_TRUE(Core::File(fileTo).Exists());
}

TEST(UtilsFileTest, verifyFile)
{
    Core::File file(fileTo);

    EXPECT_TRUE(file.Open(true));

    uint8_t buffer[2 * numBytes];

    EXPECT_EQ(numBytes, file.Read(buffer, 2 * numBytes));
    EXPECT_EQ(0, memcmp(buffer, bytes, numBytes));
}
