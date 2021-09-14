/*
 * ============================================================================
 * RDK MANAGEMENT, LLC CONFIDENTIAL AND PROPRIETARY
 * ============================================================================
 * This file (and its contents) are the intellectual property of RDK Management, LLC.
 * It may not be used, copied, distributed or otherwise  disclosed in whole or in
 * part without the express written permission of RDK Management, LLC.
 * ============================================================================
 * Copyright (c) 2020 RDK Management, LLC. All rights reserved.
 * ============================================================================
 * Copyright (C) 2020 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 * ============================================================================
 # This program is the proprietary software of Broadcom and/or its licensors,
 # and may only be used, duplicated, modified or distributed pursuant to the terms and
 # conditions of a separate, written license agreement executed between you and Broadcom
 # (an "Authorized License").  Except as set forth in an Authorized License, Broadcom grants
 # no license (express or implied), right to use, or waiver of any kind with respect to the
 # Software, and Broadcom expressly reserves all rights in and to the Software and all
 # intellectual property rights therein.  IF YOU HAVE NO AUTHORIZED LICENSE, THEN YOU
 # HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD IMMEDIATELY
 # NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 # Except as expressly set forth in the Authorized License,
 # 1.     This program, including its structure, sequence and organization, constitutes the valuable trade
 #    1. secrets of Broadcom, and you shall use all reasonable efforts to protect the confidentiality thereof,
 # and to use this information only in connection with your use of Broadcom integrated circuit products.
 # 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 # AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 # WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH RESPECT TO
 # THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL IMPLIED WARRANTIES
 # OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR A PARTICULAR PURPOSE,
 # LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET ENJOYMENT, QUIET POSSESSION
 # OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE ENTIRE RISK ARISING OUT OF
 # USE OR PERFORMANCE OF THE SOFTWARE.
 #
 # 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 # LICENSORS BE LIABLE FOR  CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT, OR
 # EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO YOUR
 # USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN ADVISED OF
 # THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS OF THE AMOUNT
 # ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER IS GREATER. THESE
 # LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF ESSENTIAL PURPOSE OF
 # ANY LIMITED REMEDY.
 ###############################################################################
 */
 
#include "../../Module.h"
#include <interfaces/IDeviceIdentification.h>

#include <fstream>
#include <sstream>

namespace WPEFramework {
namespace Plugin {

class DeviceImplementation : public Exchange::IDeviceProperties, public PluginHost::ISubSystem::IIdentifier {
    static constexpr const TCHAR* VersionFile= _T("/version.txt");
    static constexpr const TCHAR* ProductFile= _T("/proc/device-tree/bolt/product-id");

public:
    DeviceImplementation()
    {
        UpdateChipset(_chipset);
        UpdateFirmwareVersion(_firmwareVersion);
        UpdateIdentifier(_identifier);
    }

    DeviceImplementation(const DeviceImplementation&) = delete;
    DeviceImplementation& operator= (const DeviceImplementation&) = delete;
    virtual ~DeviceImplementation()
    {
    }

public:
    // Device Propertirs interface
    const string Chipset() const override
    {
        return _chipset;
    }
    const string FirmwareVersion() const override
    {
        return _firmwareVersion;
    }

    // Identifier interface
    uint8_t Identifier(const uint8_t length, uint8_t buffer[]) const override
    {
        uint8_t ret = 0;
        if (_identifier.length()) {
            ret = (_identifier.length() > length ? length : _identifier.length());
            ::memcpy(buffer, _identifier.c_str(), ret);
        } 
        return ret;
    }

    BEGIN_INTERFACE_MAP(DeviceImplementation)
        INTERFACE_ENTRY(Exchange::IDeviceProperties)
        INTERFACE_ENTRY(PluginHost::ISubSystem::IIdentifier)
    END_INTERFACE_MAP

private:
    inline void UpdateFirmwareVersion(string& firmwareVersion) const
    {
        string line;
        std::ifstream file(VersionFile);
        if (file.is_open()) {
            while (getline(file, line)) {
                if (line.find("SDK_VERSION") != std::string::npos) {
                    std::size_t position = line.find('=');
                    if (position != std::string::npos) {
                        firmwareVersion.assign(line.substr(position + 1, string::npos));
                    }
                }
            }
            file.close();
        }
    }
    inline void UpdateChipset(string& chipset) const
    {
        string s;
        std::ifstream file(ProductFile);
	unsigned char product_id[4];
        int product_id_int;

        if (file.is_open()) {
            file >> product_id[0];
            file >> product_id[1];
            file >> product_id[2];
            file >> product_id[3];
            product_id_int = (unsigned int) product_id[0] << 24 | (unsigned int) product_id[1] << 16 | (unsigned int) product_id[2] << 8 | (unsigned int) product_id[3];
            s << "BCM" << std::hex << product_id_int;
            chipset = s.str();
            file.close();
        }
    }

   inline void UpdateIdentifier(string &identifier) const
   {
       if (system("/lib/rdk/serialNumber.sh > /tmp/.deviceid.serialno") == 0) {
          std::ifstream file("/tmp/.deviceid.serialno");
          if (file.is_open()) {
             getline(file, identifier);
             system("rm -rf /tmp/.deviceid.serialno");
          }
      }
   }

private:
    string _chipset;
    string _firmwareVersion;
    string _identifier;
};

    SERVICE_REGISTRATION(DeviceImplementation, 1, 0);
}
}
