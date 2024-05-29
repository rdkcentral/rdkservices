#ifndef STUN_CLIENT_H
#define STUN_CLIENT_H

#include <array>
#include <chrono>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <netinet/in.h>

namespace stun
{

class encoder;
class decoder;
class message;
class message_factory;

using buffer = std::vector<uint8_t>;

enum class network_access_type {
  udp_blocked,
  open_internet,
  symmetric_firewall,
  full_cone,
  symmetric_nat,
  restricted,
  port_resricted,
  unknown
};

namespace message_type {
  static uint16_t constexpr binding_request = 0x001;
  static uint16_t constexpr binding_response = 0x0101;
  static uint16_t constexpr binding_error_response = 0x0111;
  static uint16_t constexpr shared_secret_request = 0x0002;
  static uint16_t constexpr shared_secret_response = 0x0102;
  static uint16_t constexpr shared_secret_error_response = 0x0112;
}

namespace attribute_type {
  static uint16_t constexpr mapped_address = 0x001;
  static uint16_t constexpr response_address = 0x0002;
  static uint16_t constexpr change_request = 0x0003;
  static uint16_t constexpr source_address = 0x004;
  static uint16_t constexpr changed_address = 0x005;
  static uint16_t constexpr username = 0x0006;
  static uint16_t constexpr password = 0x0007;
  static uint16_t constexpr message_integrity = 0x0008;
  static uint16_t constexpr error_code = 0x0009;
  static uint16_t constexpr unknown_attributes = 0x000a;
  static uint16_t constexpr reflected_from = 0x000b;
}

struct attribute {
  uint16_t type;
  uint16_t length;
  std::vector<uint8_t> value;
};

namespace attributes {
  class address {
  public:
    address(attribute const & attr);
    inline sockaddr_storage addr() const {
      return m_addr;
    }
  private:
    sockaddr_storage m_addr;
  };
  struct mapped_address : public address {
    mapped_address(attribute const & attr) : address(attr) { }
  };
  struct source_address : public address {
    source_address(attribute const & attr) : address(attr) { }
  };
  struct changed_address : public address {
    changed_address(attribute const & attr) : address(attr) { }
  };
}

struct message_header {
  uint16_t message_type;
  uint16_t message_length;
  std::array<uint8_t, 16> transaction_id;
};

class message {
  friend class encoder;
  friend class decoder;
  friend class message_factory;
public:
  buffer encode() const;
  inline std::vector<attribute> const & attributes() const {
    return m_attrs;
  }

  attribute const * find_attribute(uint16_t attr_type) const;

private:
  message_header  m_header;
  std::vector<attribute> m_attrs;
};

class message_factory final {
public:
  static message * create_binding_request();
};

class decoder final {
public:
  static uint16_t  decode_u16(buffer const & buff, size_t * offset);
  static uint32_t  decode_u32(buffer const & buff, size_t * offset);
  static attribute decode_attr(buffer const & buff, size_t * offset);
  static message * decode_message(buffer const & buff, size_t * offset);
};

class encoder final {
public:
  static void encode_u16(buffer & buff, uint16_t n);
  static void encode_u32(buffer & buff, uint32_t n);
};

struct server {
  server(std::string const& h, uint16_t p)
    : hostname(h), port(p)
  {}
  std::string hostname;
  uint16_t port;
};

enum class protocol {
  af_inet,
  af_inet6
};

struct bind_result {
  bind_result() : public_ip() {}
  bool is_valid() { return !public_ip.empty(); }
  void invalidate() { public_ip.clear(); }
  std::string public_ip;
};

class client {
public:
  client();
  ~client();

  bool bind(std::string const & hostname, 
    uint16_t port,
    std::string const & interface, 
    protocol proto,
    uint16_t bind_timeout,
    uint16_t cache_timeout,
    bind_result& result);

  network_access_type discover_network_access_type(server const & srv);

  inline void set_verbose(bool b) {
    m_verbose = b;
  }
private:
  void verbose(char const * format, ...) __attribute__((format(printf, 2, 3)));
  void create_udp_socket(int inet_family);

  std::unique_ptr<message> send_binding_request(std::chrono::milliseconds wait_time);

  std::unique_ptr<message> send_binding_request(sockaddr_storage const & addr,
    std::chrono::milliseconds wait_time);

  message * send_message(sockaddr_storage const & remote_adr, message const & req,
    std::chrono::milliseconds wait_time, int * local_iface_index = nullptr);

private:
  server m_server;
  protocol m_protocol;
  std::string m_interface;
  uint16_t m_bind_timeout;
  uint16_t m_cache_timeout;
  std::chrono::time_point<std::chrono::steady_clock> m_last_cache_time;
  bind_result m_last_result;
  bool m_verbose;
  int m_fd;
};

std::string sockaddr_to_string(sockaddr_storage const & addr);

}

#endif
