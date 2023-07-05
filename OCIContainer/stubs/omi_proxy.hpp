/*
 * Copyright (c) 2021, LIBERTY GLOBAL all rights reserved.
 */

#ifndef OMI_PROXY_HPP_
#define OMI_PROXY_HPP_

#include <i_omi_proxy.hpp>

namespace omi
{

class OmiProxy : public IOmiProxy
{
public:
    OmiProxy() = default;
    virtual ~OmiProxy() = default;

    virtual bool mountCryptedBundle(const std::string& id,
                                       const std::string& rootfs_file_path,
                                       const std::string& config_json_path,
                                       std::string& bundlePath) { return true; }

    virtual bool umountCryptedBundle(const std::string& id) { return true; }

    virtual long unsigned registerListener(const OmiErrorListener &listener, const void* cbParams) { return 0; }

    virtual void unregisterListener(long unsigned tag) {}
};

} // namespace omi

#endif // OMI_PROXY_HPP_
