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
#include "Protobuf.h"

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

        class KeyId : public Exchange::KeyId {
        public:
            inline KeyId()
                : Exchange::KeyId()
                , _systems(0)
            {
            }
            inline KeyId(const systemType type, const uint8_t kid[], const uint8_t length)
                : Exchange::KeyId(kid, length)
                , _systems(type)
            {
            }
            inline KeyId(const systemType type, const uint32_t a, const uint16_t b, const uint16_t c, const uint8_t d[])
                : Exchange::KeyId(a, b, c, d)
                , _systems(type)
            {
            }
            inline KeyId(const KeyId& copy)
                : Exchange::KeyId(copy)
                , _systems(copy._systems)
            {
            }
            inline ~KeyId()
            {
            }

            inline KeyId& operator=(const KeyId& rhs)
            {
                Exchange::KeyId::operator=(rhs);
                _systems = rhs._systems;
                return (*this);
            }

        public:
            inline bool operator==(const Exchange::KeyId& rhs) const
            {
                return (Exchange::KeyId::operator== (rhs));
            }   
            inline bool operator!=(const Exchange::KeyId& rhs) const
            {
                return (!operator==(rhs));
            }
            inline bool operator==(const KeyId& rhs) const
            {
                return (Exchange::KeyId::operator== (rhs));
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
        inline Exchange::ISession::KeyStatus Status() const
        {
            return (_keyIds.size() > 0 ? _keyIds.begin()->Status() : Exchange::ISession::StatusPending);
        }
        inline Exchange::ISession::KeyStatus Status(const KeyId& key) const
        {
            Exchange::ISession::KeyStatus result(Exchange::ISession::StatusPending);
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
        inline bool HasKeyId(const Exchange::KeyId& keyId) const
        {
            return (std::find(_keyIds.begin(), _keyIds.end(), keyId) != _keyIds.end());
        }
        inline void AddKeyId(const KeyId& key)
        {
            std::list<KeyId>::iterator index(std::find(_keyIds.begin(), _keyIds.end(), key));

            if (index == _keyIds.end()) {
                TRACE(Trace::Information, (_T("Added key: %s for system: %02X\n"), key.ToString().c_str(), key.Systems()));
                _keyIds.emplace_back(key);
            } else {
                TRACE(Trace::Information, (_T("Updated key: %s for system: %02X\n"), key.ToString().c_str(), key.Systems()));
                index->Flag(key.Systems());
            }
        }
        inline const KeyId* UpdateKeyStatus(Exchange::ISession::KeyStatus status, const KeyId& key)
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
                uint32_t sizeBE = ((data[offset] << 24) | (data[offset + 1] << 16) | (data[offset + 2] << 8) | data[offset + 3]);
                if ((sizeBE <= static_cast<uint32_t>(length - offset)) && ((length - offset) >= 4)
                        && (::memcmp(&(data[offset + 4]), PSSHeader, 4) == 0)) {
                    TRACE(Trace::Information, (_T("Initdata contains a PSSH box")));
                    ParsePSSHBox(&(data[offset + 4 + 4]), (sizeBE - 4 - 4));
                    offset += sizeBE;
                } else if (offset == 0) {
                    uint32_t sizeLE = (data[offset] | (data[offset + 1] << 8) | (data[offset + 2] << 16) | (data[offset + 3] << 24));
                    if ((data[0] == '<') && (data[2] == 'W') && (data[4] == 'R') && (data[6] == 'M')) {
                        // Playready XML data without PSSH header and withouth Playready Rights Managment Header
                        TRACE(Trace::Information, (_T("Initdata contains Playready XML data")));
                        ParseXMLBox(data, length);
                        offset = length;
                    } else if (std::string(reinterpret_cast<const char*>(data), length).find(JSONKeyIds) != std::string::npos) {
                        // keyids initdata type
                        TRACE(Trace::Information, (_T("Initdata contains ClearKey key IDs")));
                        ParseJSONInitData(reinterpret_cast<const char*>(data), length);
                        offset = length;
                    } else if (sizeLE == length) {
                        // Seems like it is an XMLBlob, without PSSH header, we have seen that on PlayReady only..
                        TRACE(Trace::Information, (_T("Initdata contains Playready PSSH payload")));
                        if (ParsePlayReadyPSSHData(&data[offset], length) == true) {
                            offset = length;
                        }
                    }
                    break;
                } else {
                    break;
                }
            } while (offset < length);

            if (offset != length) {
                TRACE(Trace::Information, (_T("Have no clue what this is!!!")));
            }
        }

        void ParsePSSHBox(const uint8_t data[], const uint16_t length)
        {
            const uint16_t PSSH_HEADER_SIZE_V0 = 24;
            const uint16_t PSSH_HEADER_SIZE_V1 = 28;

            if (length >= PSSH_HEADER_SIZE_V0) {
                systemType system(COMMON);
                const uint8_t version(data[0]);
                const uint8_t* keyIdData(nullptr);
                uint32_t keyIdCount(0);
                const uint8_t* psshData(nullptr);
                uint32_t psshDataSize(0);

                auto Read32BE = [](const uint8_t ptr[]) -> uint32_t {
                    return ((ptr[0] << 24) | (ptr[1] << 16) | (ptr[2] << 8) | ptr[3]);
                };

                if (version == 0) {
                    psshData = &data[PSSH_HEADER_SIZE_V0];
                    psshDataSize = Read32BE(psshData - 4);
                    if ((psshDataSize + PSSH_HEADER_SIZE_V0) > length) {
                        psshData = nullptr;
                    }
                } else if (version == 1) {
                    /* Version 1 inserts raw key IDs before DRM system specific payload. */
                    keyIdData = &data[PSSH_HEADER_SIZE_V0];
                    keyIdCount = Read32BE(keyIdData - 4);
                    if (((keyIdCount * KeyId::Length()) + PSSH_HEADER_SIZE_V1) > length) {
                        keyIdData = nullptr;
                    } else {
                        psshData = &data[PSSH_HEADER_SIZE_V1 + (keyIdCount * KeyId::Length())];
                        psshDataSize = Read32BE(psshData - 4);
                        if (psshDataSize + PSSH_HEADER_SIZE_V1 + (keyIdCount * KeyId::Length()) > length) {
                            psshData = nullptr;
                        }
                    }
                } else {
                    TRACE(Trace::Error, (_T("Unsupported PSSH version (%hhu)"), version));
                }

                if ((keyIdData != nullptr) || (psshData != nullptr)) {
                    if (::memcmp(&(data[4]), CommonEncryption, 16) == 0) {
                        TRACE(Trace::Information, (_T("Common encryption detected")));
                    } else if (::memcmp(&(data[4]), PlayReady, 16) == 0) {
                        TRACE(Trace::Information, (_T("PlayReady detected")));
                        system = PLAYREADY;
                        if (psshData != nullptr) {
                            ParsePlayReadyPSSHData(psshData, psshDataSize);
                        }
                    } else if (::memcmp(&(data[4]), WideVine, 16) == 0) {
                        TRACE(Trace::Information, (_T("Widevine detected")));
                        system = WIDEVINE;
                        if (psshData != nullptr) {
                            ParseWidevinePSSHData(psshData, psshDataSize);
                        }
                    } else if (::memcmp(&(data[4]), ClearKey, 16) == 0) {
                        TRACE(Trace::Information, (_T("ClearKey detected")));
                        system = CLEARKEY;
                    } else {
                        TRACE(Trace::Information, (_T("Unknown DRM system: %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X"),
                                data[4], data[5], data[6], data[7], data[8], data[9], data[10], data[11]));
                    }

                    if (keyIdData != nullptr) {
                        TRACE(Trace::Information, (_T("Adding %d keys from PSSHv1 box"), keyIdCount));
                        while (keyIdCount-- != 0) {
                            AddKeyId(KeyId(system, keyIdData, KeyId::Length()));
                            keyIdData += KeyId::Length();
                        }
                    }
                } else {
                    TRACE(Trace::Error, (_T("Invalid PSSH data")));
                }
            } else {
                TRACE(Trace::Error, (_T("Invalid PSSH box")));
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
                TRACE(Trace::Information, (_T("clearkey: keyID %s, length %d"), keyID64.c_str(), static_cast<int>(keyID64.length())));
                Core::FromString(keyID64, keyID, decodeLength);
                AddKeyId(KeyId(system, keyID, static_cast<uint8_t>(sizeof(keyID))));
            }
        }

        bool ParsePlayReadyPSSHData(const uint8_t data[], const uint16_t length)
        {
            auto Read16LE = [](const uint8_t ptr[]) -> uint16_t {
                return (ptr[0] | (ptr[1] << 8));
            };
            auto Read32LE = [](const uint8_t ptr[]) -> uint32_t {
                return (ptr[0] | (ptr[1] << 8) | (ptr[2] << 16) | (ptr[3] << 24));
            };

            bool result = false;

            uint32_t size = Read32LE(data);
            data += 4;
            if (size == length) {
                uint16_t count = Read16LE(data);
                if (count > 0) {
                    const uint8_t* dataEnd = (data + length);
                    data += 2;
                    while ((data + 4) < dataEnd) {
                        const uint16_t recordType = Read16LE(data);
                        const uint16_t recordLength = Read16LE(data + 2);
                        data += 4;
                        if ((data + recordLength) >= dataEnd) {
                            break;
                        }
                        if (recordType == 1 /* rights management */) {
                            ParseXMLBox(data, recordLength);
                        }
                        data += recordLength;
                        if (--count == 0) {
                            break;
                        }
                    }
                }
                result = (count == 0);
            }

            return (result);
        }

        bool ParseWidevinePSSHData(const uint8_t data[], const uint16_t length)
        {
            class WidevinePsshPB2 : public Protobuf::Message {
            public:
                enum class algorithm : Protobuf::UInt32::type {
                    UNENCRYPTED,
                    AES_CTR = 1
                };

            public:
                WidevinePsshPB2()
                {
                    Add(1, &Algorithm);
                    Add(2, &KeyIDs);
                    Add(3, &Provider);
                    Add(4, &ContentID);
                    Add(5, &TrackType);
                    Add(6, &Policy);
                    Add(7, &CryptoPeriodIndex);
                    Add(8, &GroupedLicense);
                    Add(9, &ProtectionScheme);
                    Add(10,&CryptoPeriodDuration);
                }

            public:
                Protobuf::EnumType<algorithm> Algorithm;
                Protobuf::RepeatedType<Protobuf::Bytes> KeyIDs;
                Protobuf::Utf8String Provider;
                Protobuf::Utf8String ContentID;
                Protobuf::Utf8String TrackType;
                Protobuf::Utf8String Policy;
                Protobuf::UInt32 CryptoPeriodIndex;
                Protobuf::Bytes GroupedLicense;
                Protobuf::UInt32 ProtectionScheme;
                Protobuf::UInt32 CryptoPeriodDuration;
            };

            bool result = false;

            WidevinePsshPB2 wvpb2;
            if ((wvpb2.FromBuffer(data, length) == true) && (wvpb2.IsValid() == true)) {
                if (wvpb2.KeyIDs.IsSet() == true) {
                    for (auto const& keyID : wvpb2.KeyIDs.Elements()) {
                        AddKeyId(KeyId(WIDEVINE, keyID.Value().data(), keyID.Value().size()));
                    }
                } else {
                    TRACE(Trace::Information, (_T("No key IDs specified in Widevine PSSH data")));
                }

                result = true;
            }

            return (result);
        }

    private:
        std::list<KeyId> _keyIds;
    };
}
} // namespace WPEFramework::Plugin

#endif // __CENCPARSER_H
