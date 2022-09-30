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
const uint8_t bytes[] = { 0x00, 0x01, 0x00, 0x00, 0x00, 0x06, 0xFE, 0x03, 0x20, 0x04, 0x00, 0x01 };
}

using namespace WPEFramework;

TEST(UtilsFileTest, createFolder_createFile_moveFile_verifyFile)
{
    Core::Directory dir(_T("/tmp/UtilsFileTest"));

    EXPECT_TRUE(dir.Destroy(false));
    ASSERT_TRUE(dir.CreatePath());

    Core::File file(string(_T("/tmp/UtilsFileTest/file")));

    EXPECT_FALSE(file.Exists());
    EXPECT_TRUE(file.Create());
    EXPECT_EQ(sizeof(bytes), file.Write(bytes, sizeof(bytes)));

    Core::File file2(string(_T("/tmp/UtilsFileTest/destination/for/new/file")));

    EXPECT_FALSE(file2.Exists());
    EXPECT_TRUE(Utils::MoveFile(file.Name(), file2.Name()));
    file.LoadFileInfo();
    file2.LoadFileInfo();
    EXPECT_FALSE(file.Exists());
    EXPECT_TRUE(file2.Exists());
    EXPECT_TRUE(file2.Open(true));

    uint8_t buffer[2 * sizeof(bytes)];

    EXPECT_EQ(sizeof(bytes), file2.Read(buffer, 2 * sizeof(bytes)));
    EXPECT_EQ(0, memcmp(buffer, bytes, sizeof(bytes)));
}
