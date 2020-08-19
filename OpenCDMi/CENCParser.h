/*
 * If not stated otherwise in this file or this component's LICENSE file the
 * following copyright and licenses apply:
 *
 * Copyright 2020 RDK Management
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

#ifndef __CENCPARSER_H
#define __CENCPARSER_H

#include "Module.h"

namespace WPEFramework {
namespace Plugin {

    //This class is not Thread Safe. The user of this class must ensure thread saftey (single thread access) !!!!
    class CommonEncryptionData {
    private:
        CommonEncryptionData() = delete;
        CommonEncryptionData& operator=(const CommonEncryptionData&) = delete;

        static const uint8_t PSSHeader[];
        static const uint8_t CommonEncryption[];
        static const uint8_t PlayReady[];
        static const uint8_t WideVine[];
        static const uint8_t ClearKey[];
        static const    char JSONKeyIds[];

    public:
        enum systemType {
            COMMON = 0x0001,
            CLEARKEY = 0x0002,
            PLAYREADY = 0x0004,
            WIDEVINE = 0x0008
        };

        class KeyId : public OCDM::KeyId {
        public:
            inline KeyId()
                : OCDM::KeyId()
                , _systems(0)
            {
            }
            inline KeyId(const systemType type, const uint8_t kid[], const uint8_t length)
                : OCDM::KeyId(kid, length)
                , _systems(type)
            {
            }
            inline KeyId(const systemType type, const uint32_t a, const uint16_t b, const uint16_t c, const uint8_t d[])
                : OCDM::KeyId(a, b, c, d)
                , _systems(type)
            {
            }
            inline KeyId(const KeyId& copy)
                : OCDM::KeyId(copy)
                , _systems(copy._systems)
            {
            }
            inline ~KeyId()
            {
            }

            inline KeyId& operator=(const KeyId& rhs)
            {
                OCDM::KeyId::operator=(rhs);
                _systems = rhs._systems;
                return (*this);
            }

        public:
            inline bool operator==(const OCDM::KeyId& rhs) const
            {
                return (OCDM::KeyId::operator== (rhs));
            }   
            inline bool operator!=(const OCDM::KeyId& rhs) const
            {
                return (!operator==(rhs));
            }
            inline bool operator==(const KeyId& rhs) const
            {
                return (OCDM::KeyId::operator== (rhs));
            }   
            inline bool operator!=(const KeyId& rhs) const
            {
                return (!operator==(rhs));
            }
            inline void Flag(const uint32_t systems)
            {
                _systems |= systems;
            }
            inline uint32_t Systems() const
            {
                return (_systems);
            }

        private:
            uint32_t _systems;
        };

        typedef Core::IteratorType<const std::list<KeyId>, const KeyId&, std::list<KeyId>::const_iterator> Iterator;

    public:
        CommonEncryptionData(const uint8_t data[], const uint16_t length)
            : _keyIds()
        {
            Parse(data, length);
        }
        CommonEncryptionData(const CommonEncryptionData& copy)
            : _keyIds(copy._keyIds)
        {
        }
        ~CommonEncryptionData()
        {
        }

    public:
        inline ::OCDM::ISession::KeyStatus Status() const
        {
            return (_keyIds.size() > 0 ? _keyIds.begin()->Status() : ::OCDM::ISession::StatusPending);
        }
        inline ::OCDM::ISession::KeyStatus Status(const KeyId& key) const
        {
            ::OCDM::ISession::KeyStatus result(::OCDM::ISession::StatusPending);
            if (key.IsValid() == true) {
                std::list<KeyId>::const_iterator index(std::find(_keyIds.begin(), _keyIds.end(), key));
                if (index != _keyIds.end()) {
                    result = index->Status();
                }
            }
            return (result);
        }
        inline Iterator Keys() const
        {
            return (Iterator(_keyIds));
        }
        inline bool HasKeyId(const OCDM::KeyId& keyId) const
        {
            return (std::find(_keyIds.begin(), _keyIds.end(), keyId) != _keyIds.end());
        }
        inline void AddKeyId(const KeyId& key)
        {
            std::list<KeyId>::iterator index(std::find(_keyIds.begin(), _keyIds.end(), key));

            if (index == _keyIds.end()) {
                TRACE_L1("Added key: %s for system: %02X\n", key.ToString().c_str(), key.Systems());
                _keyIds.emplace_back(key);
            } else {
                TRACE_L1("Updated key: %s for system: %02X\n", key.ToString().c_str(), key.Systems());
                index->Flag(key.Systems());
            }
        }
        inline const KeyId* UpdateKeyStatus(::OCDM::ISession::KeyStatus status, const KeyId& key)
        {
            KeyId* entry = nullptr;

            ASSERT(key.IsValid() == true);

            std::list<KeyId>::iterator index(std::find(_keyIds.begin(), _keyIds.end(), key));

            if (index == _keyIds.end()) {
                _keyIds.emplace_back(key);
                entry = &(_keyIds.back());
            } else {
                entry = &(*index);
            }
            entry->Status(status);

            return (entry);
        }
        inline bool IsSupported(const CommonEncryptionData& keys) const
        {

            bool result = true;
            std::list<KeyId>::const_iterator requested(keys._keyIds.begin());

            while ((requested != keys._keyIds.end()) && (result == true)) {
                std::list<KeyId>::const_iterator index(_keyIds.begin());

                while ((index != _keyIds.end()) && (*index != *requested)) {
                    index++;
                }

                result = (index != _keyIds.end());

                requested++;
            }

            return (result);
        }
        inline bool IsEmpty() const {
            return _keyIds.empty();
        }
    private:
        uint8_t Base64(const uint8_t value[], const uint8_t sourceLength, uint8_t object[], const uint8_t length)
        {
            uint8_t state = 0;
            uint8_t index = 0;
            uint8_t filler = 0;
            uint8_t lastStuff = 0;

            while ((index < sourceLength) && (filler < length)) {
                uint8_t converted;
                uint8_t current = value[index];

                if ((current >= 'A') && (current <= 'Z')) {
                    converted = static_cast<uint8_t>(current - 'A');
                } else if ((current >= 'a') && (current <= 'z')) {
                    converted = static_cast<uint8_t>(current - 'a' + 26);
                } else if ((current >= '0') && (current <= '9')) {
                    converted = static_cast<uint8_t>(current - '0' + 52);
                } else if (current == '+') {
                    converted = 62;
                } else if (current == '/') {
                    converted = 63;
                } else {
                    break;
                }

                if (state == 0) {
                    lastStuff = converted << 2;
                    state = 1;
                } else if (state == 1) {
                    object[filler++] = (((converted & 0x30) >> 4) | lastStuff);
                    lastStuff = ((converted & 0x0F) << 4);
                    state = 2;
                } else if (state == 2) {
                    object[filler++] = (((converted & 0x3C) >> 2) | lastStuff);
                    lastStuff = ((converted & 0x03) << 6);
                    state = 3;
                } else if (state == 3) {
                    object[filler++] = ((converted & 0x3F) | lastStuff);
                    state = 0;
                }
                index++;
                index++;
            }

            return (filler);
        }

        void Parse(const uint8_t data[], const uint16_t length)
        {
            uint16_t offset = 0;

            do {
                // Check if this is a PSSH box...
                uint32_t size = (data[offset] << 24) | (data[offset + 1] << 16) | (data[offset + 2] << 8) | data[offset + 3];
                if (size == 0) {
                    TRACE_L1("While parsing CENC, found chunk of size 0, are you sure the data is valid? %d\n", __LINE__);
                    break;
                }

                if ((size <= static_cast<uint32_t>(length - offset)) && (memcmp(&(data[offset + 4]), PSSHeader, 4) == 0)) {
                    ParsePSSHBox(&(data[offset + 4 + 4]), size - 4 - 4);
                } else {
                    uint32_t XMLSize = (data[offset] | (data[offset + 1] << 8) | (data[offset + 2] << 16) | (data[offset + 3] << 24));

                    if (XMLSize <= static_cast<uint32_t>(length - offset)) {

                        uint16_t stringLength = (data[offset + 8] | (data[offset + 9] << 8));
                        if (stringLength <= (XMLSize - 10)) {

                            // Seems like it is an XMLBlob, without PSSH header, we have seen that on PlayReady only..
                            ParseXMLBox(&(data[offset + 10]), stringLength);
                        }

                        offset += XMLSize;

                    } else if ((offset == 0) && (data[0] == '<') && (data[2] == 'W') && (data[4] == 'R') && (data[6] == 'M')) {
                        ParseXMLBox(data, size);
                        offset = length;
                    } else if (std::string(reinterpret_cast<const char*>(data), length).find(JSONKeyIds) != std::string::npos) {
                        /* keyids initdata type */
                        TRACE_L1("Initdata contains clearkey's key ids");

                        ParseJSONInitData(reinterpret_cast<const char*>(data), length);
                    } else {
                        TRACE_L1("Have no clue what this is!!! %d\n", __LINE__);
                    }
                }
                offset += size;

            } while (offset < length);
        }

        void ParsePSSHBox(const uint8_t data[], const uint16_t length)
        {
            systemType system(COMMON);
            const uint8_t* psshData(&(data[KeyId::Length() + 4 /* flags */]));
            uint32_t count((psshData[0] << 24) | (psshData[1] << 16) | (psshData[2] << 8) | psshData[3]);
            uint16_t stringLength = (data[8] | (data[9] << 8));

            if (::memcmp(&(data[4]), CommonEncryption, KeyId::Length()) == 0) {
                psshData += 4;
                TRACE_L1("Common detected [%d]\n", __LINE__);
            } else if (::memcmp(&(data[4]), PlayReady, KeyId::Length()) == 0) {
                if (stringLength <= (length - 10)) {
                    ParseXMLBox(&(psshData[10]), count);
                    TRACE_L1("PlayReady XML detected [%d]\n", __LINE__);
                    count = 0;
                } else {
                    TRACE_L1("PlayReady BIN detected [%d]\n", __LINE__);
                    system = PLAYREADY;
                    psshData += 4;
                }
            } else if (::memcmp(&(data[4]), WideVine, KeyId::Length()) == 0) {
                TRACE_L1("WideVine detected [%d]\n", __LINE__);
                system = WIDEVINE;
                psshData += 4 + 4 /* God knows what this uint32 means, we just skip it. */;
            } else if (::memcmp(&(data[4]), ClearKey, KeyId::Length()) == 0) {
                TRACE_L1("ClearKey detected [%d]\n", __LINE__);
                system = CLEARKEY;
                psshData += 4;
            } else {
                TRACE_L1("Unknown system: %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X.\n", data[4], data[5], data[6], data[7], data[8], data[9], data[10], data[11]);
                count = 0;
            }

            if (data[0] != 1) {
                count /= KeyId::Length();
            }

            TRACE_L1("Adding %d keys from PSSH box\n", count);

            while (count-- != 0) {
                AddKeyId(KeyId(system, psshData, KeyId::Length()));
                psshData += KeyId::Length();
            }
        }

        uint16_t FindInXML(const uint8_t data[], const uint16_t length, const char key[], const uint8_t keyLength)
        {
            uint8_t index = 0;
            uint16_t result = 0;

            while ((result <= length) && (index < keyLength)) {
                if (static_cast<uint8_t>(key[index]) == data[result]) {
                    index++;
                    result += 2;
                } else if (index > 0) {
                    result -= ((index - 1) * 2);
                    index = 0;
                } else {
                    result += 2;
                }
            }
            return ((index == keyLength) ? (result - (keyLength * 2)) : result);
        }

        void ParseXMLBox(const uint8_t data[], const uint16_t length)
        {
                uint16_t begin;
                const uint8_t* slot = data;
                uint16_t size = length;

                // Now find the string <KID> in this text
                // this will process  PlayReady header format v.4.0.0.0
                // https://docs.microsoft.com/en-us/playready/specifications/playready-header-specification#36-v4000
                //
                // we want to find and process this utf16 string:
                // <KID>q5HgCTj40kGeNVhTH9Gexw==</KID>
                //
                while ((size > 0) && ((begin = FindInXML(slot, size, "<KID>", 5)) < size)) {
                    uint16_t end = FindInXML(&(slot[begin + 10]), size - begin - 10, "</KID>", 6);

                    if (end < (size - begin - 10)) {
                        uint8_t byteArray[32];

                        // We got a KID, translate it
                        if (Base64(&(slot[begin + 10]), static_cast<uint8_t>(end), byteArray, sizeof(byteArray)) == KeyId::Length()) {
                            // Pass it the microsoft way :-(
                            uint32_t a = byteArray[0];
                            a = (a << 8) | byteArray[1];
                            a = (a << 8) | byteArray[2];
                            a = (a << 8) | byteArray[3];
                            uint16_t b = byteArray[4];
                            b = (b << 8) | byteArray[5];
                            uint16_t c = byteArray[6];
                            c = (c << 8) | byteArray[7];
                            uint8_t* d = &byteArray[8];

                            // Add them in both endiannesses, since we have encountered both in the wild.
                            AddKeyId(KeyId(PLAYREADY, a, b, c, d));
                        }
                        size -= (begin + 10 + end + 12);
                        slot += (begin + 10 + end + 12);
                    } else {
                        size = 0;
                    }
                }

                // this will process PlayReady header format v.4.1.0.0/v.4.2.0.0/v.4.3.0.0
                // https://docs.microsoft.com/en-us/playready/specifications/playready-header-specification#35-v4100
                // https://docs.microsoft.com/en-us/playready/specifications/playready-header-specification#34-v4200
                // https://docs.microsoft.com/en-us/playready/specifications/playready-header-specification#33-v4300
                //
                // we want to find and process this utf16 string:
                // <KID ALGID="AESCTR" CHECKSUM="xNvWVxoWk04=" VALUE="0IbHou/5s0yzM80yOkKEpQ=="></KID>
                //
                // Now find the string "<KID " in this text
                while ((size > 0) && ((begin = FindInXML(slot, size, "<KID ", 5)) < size)) {
                    uint16_t end = FindInXML(&(slot[begin + 10]), size - begin - 10, "</KID>", 6);

                    uint16_t keyValue = FindInXML(&(slot[begin + 10]), end, "VALUE", 5);  
                    uint16_t keyStart = FindInXML(&(slot[begin + 10 + keyValue + 10]), end - keyValue - 10, "\"", 1) + 2;
                    uint16_t keyLength = FindInXML(&(slot[begin + 10 + keyValue + 10 + keyStart]), end - keyValue - 10 - keyStart - 2, "\"", 1) - 2;
                    
                    if (end < (size - begin - 10)) {
                        uint8_t byteArray[32];

                        // We got a KID, translate its
                        if (Base64(&(slot[begin + 10 + keyValue + 10 + keyStart]), static_cast<uint8_t>(keyLength), byteArray, sizeof(byteArray)) == KeyId::Length()) {
                            // Pass it the microsoft way :-(
                            uint32_t a = byteArray[0];
                            a = (a << 8) | byteArray[1];
                            a = (a << 8) | byteArray[2];
                            a = (a << 8) | byteArray[3];
                            uint16_t b = byteArray[4];
                            b = (b << 8) | byteArray[5];
                            uint16_t c = byteArray[6];
                            c = (c << 8) | byteArray[7];
                            uint8_t* d = &byteArray[8];

                            // Add them in both endiannesses, since we have encountered both in the wild.
                            AddKeyId(KeyId(PLAYREADY, a, b, c, d));
                        }
                        size -= (begin + 10 + end + 12);
                        slot += (begin + 10 + end + 12);
                    } else {
                        size = 0;
                    }
                }
        }

        using JSONStringArray = Core::JSON::ArrayType<Core::JSON::String>;

        void ParseJSONInitData(const char data[], uint16_t length) {
            systemType system(CLEARKEY);

            class InitData : public Core::JSON::Container {
            public:
                InitData() : Core::JSON::Container() , KeyIds() {
                    Add(_T("kids"), &KeyIds);
                }
                virtual ~InitData() {
                }

            public:
                JSONStringArray KeyIds;
            } initData;

            initData.FromString(std::string(data, length));

            JSONStringArray::ConstIterator index(static_cast<const InitData&>(initData).KeyIds.Elements());

            uint16_t decodeLength = KeyId::Length();

            while(index.Next()) {
                uint8_t keyID[KeyId::KEY_LENGTH];
                std::string keyID64 = index.Current().Value();
                TRACE_L1("clearkey: keyID %s, length %d", keyID64.c_str(), static_cast<int>(keyID64.length()));
                Core::FromString(keyID64, keyID, decodeLength);
                AddKeyId(KeyId(system, keyID, static_cast<uint8_t>(sizeof(keyID))));
            }
        }

    private:
        std::list<KeyId> _keyIds;
    };
}
} // namespace WPEFramework::Plugin

#endif // __CENCPARSER_H
