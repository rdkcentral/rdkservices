#include "StunClient.h"
#include <assert.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <net/if.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <algorithm>
#include <exception>
#include <limits>
#include <memory>
#include <random>
#include <set>
#include <sstream>
#include <thread>
#include <iostream>

#define STUN_DEFAULT_WAIT_INTERVAL 2000 //miliseconds
#define STUN_DEFAULT_MAX_ATTEMPTS 30

//#define _STUN_DEBUG 1
//#define _STUN_USE_MSGHDR

namespace stun {
namespace details {
  static int constexpr binding_requests_max = 9;
  static std::chrono::milliseconds binding_requests_wait_time_max(1600);

  static char const * family_to_string(int family) {
    if (family == AF_INET)
      return "ipv4";
    if (family == AF_INET6)
      return "ipv6";
    return "unknown";
  }

  class file_descriptor {
  public:
    file_descriptor(int n) : m_fd(n) { }
    ~file_descriptor() {
      if (m_fd > 0)
        close(m_fd);
    }
    operator int() const { return m_fd; }
  private:
    int m_fd;
  };

  #ifdef _STUN_DEBUG
  void dump_buffer(char const * prefix, buffer const & buff)
  {
    if (prefix)
      printf("%s", prefix);
    for (uint8_t b : buff)
      printf("0x%02x ", b);
    printf("\n");
    return;
  }
  #endif

  #ifdef _STUN_DEBUG
  #define STUN_TRACE(format, ...) printf("STUN:" format __VA_OPT__(,) __VA_ARGS__)
  #else
  #define STUN_TRACE(format, ...)
  #endif


  void throw_error(char const * format, ...)
  {
    char buff[256] = {};

    va_list ap;
    va_start(ap, format);
    vsnprintf(buff, sizeof(buff) - 1, format, ap);
    va_end(ap);

    buff[255] = '\0';
#ifdef __cpp_exceptions
    throw std::runtime_error(buff);
#else
    printf("%s", buff);
#endif
    return;
  }

  template<typename iterator>
  inline void random_fill(iterator begin, iterator end) {
    std::random_device rdev;
    std::default_random_engine random_engine(rdev());
    std::uniform_int_distribution<uint8_t> uniform_dist(0, std::numeric_limits<uint8_t>::max());
    while (begin != end)
      *begin++ = uniform_dist(random_engine);
  }

  sockaddr_storage get_interface_address(std::string const & iface, int family)
  {
    bool found_iface_info = false;
    sockaddr_storage iface_info = {};

    struct ifaddrs * address_list = nullptr;
    if (getifaddrs(&address_list) == -1) {
      details::throw_error("getifaddrs failed. %s", strerror(errno));
      return iface_info;
    }

    for (auto * addr = address_list; addr != nullptr; addr = addr->ifa_next) {
      if (iface != addr->ifa_name)
        continue;
      if (family != addr->ifa_addr->sa_family)
        continue;
      iface_info = * reinterpret_cast<sockaddr_storage *>(addr->ifa_addr);
      iface_info.ss_family = addr->ifa_addr->sa_family;
      found_iface_info = true;
      break;
    }

    if (address_list)
      freeifaddrs(address_list);

    if (!found_iface_info) {
      details::throw_error("failed to find ip for interface:%s", iface.c_str());
      return iface_info;
    }
    STUN_TRACE("local_addr:%s\n", sockaddr_to_string(iface_info).c_str());

    return iface_info;
  }

  uint16_t sockaddr_get_port(sockaddr_storage const & addr)
  {
    uint16_t port = 0;
    if (addr.ss_family== AF_INET) {
      sockaddr_in const * v4 = reinterpret_cast< sockaddr_in const *>(&addr);
      port = htons(v4->sin_port);
    }
    else if (addr.ss_family == AF_INET6) {
      sockaddr_in6 const * v6 = reinterpret_cast< sockaddr_in6 const *>(&addr);
      port = htons(v6->sin6_port);
    }
    else
      throw_error("can't convert address with family:%d to a string.", addr.ss_family);

    return port;
  }

  std::string sockaddr_to_string2(sockaddr const * addr, int family)
  {
    char buff[INET6_ADDRSTRLEN] = {};

    char const * p = nullptr;

    if (family == AF_INET) {
      sockaddr_in const * v4 = reinterpret_cast< sockaddr_in const *>(addr);
      p = inet_ntop(AF_INET, &v4->sin_addr, buff, INET6_ADDRSTRLEN);
    }
    else if (family == AF_INET6) {
      sockaddr_in6 const * v6 = reinterpret_cast< sockaddr_in6 const *>(addr);
      p = inet_ntop(AF_INET6, &v6->sin6_addr, buff, INET6_ADDRSTRLEN);
    }
    else
      throw_error("can't convert address with family:%d to a string.", family);

    if (!p)
      throw_error("failed to convert address to string");

    buff[INET6_ADDRSTRLEN - 1] = '\0';
    return std::string(buff);
  }

  std::vector<sockaddr_storage> resolve_hostname(std::string const & host, uint16_t port, stun::protocol proto)
  {
    std::vector<sockaddr_storage> addrs;
    std::set<std::string> already_seen;

    struct addrinfo * stun_addrs = nullptr;
    int ret = getaddrinfo(host.c_str(), nullptr, nullptr, &stun_addrs);
    if (ret != 0) {
      std::stringstream error_message;
      error_message << "getaddrinfo failed. ";
      if (ret == EAI_SYSTEM)
        error_message << strerror(errno);
      else
        error_message << gai_strerror(ret);
      throw_error(error_message.str().c_str());
    }

    int protocol_family = AF_INET;
    if (proto == stun::protocol::af_inet)
      protocol_family = AF_INET;
    else if (proto == stun::protocol::af_inet6)
      protocol_family = AF_INET6;
    else
      throw_error("invalid protocol family");

    for (struct addrinfo * addr = stun_addrs; addr; addr = addr->ai_next) {
      if (addr->ai_family != AF_INET && addr->ai_family != AF_INET6)
        continue;
      if (addr->ai_family != protocol_family)
        continue;

      std::string const s = sockaddr_to_string2(addr->ai_addr, addr->ai_family);

      if (already_seen.find(s) == std::end(already_seen)) {
        struct sockaddr_storage temp = {};
        memcpy(&temp, addr->ai_addr, addr->ai_addrlen);

        if (addr->ai_family == AF_INET) {
          sockaddr_in * v4 = reinterpret_cast< sockaddr_in *>(&temp);
          v4->sin_port = ntohs(port);
        }
        else if (addr->ai_family == AF_INET6) {
          sockaddr_in6 * v6 = reinterpret_cast< sockaddr_in6 *>(&temp);
          v6->sin6_port = ntohs(port);
        }

        addrs.push_back(temp);
        already_seen.insert(s);
      }
    }

    if (stun_addrs)
      freeaddrinfo(stun_addrs);

    return addrs;
  }

  socklen_t socket_length(sockaddr_storage const & addr)
  {
    if (addr.ss_family == AF_INET)
      return sizeof(sockaddr_in);
    if (addr.ss_family == AF_INET6)
      return sizeof(sockaddr_in6);
    return 0;
  }

}  // end namespace details

attribute const * message::find_attribute(uint16_t attr_type) const
{
  std::vector<attribute>::const_iterator itr = std::find_if (
    std::begin(m_attrs), std::end(m_attrs), [attr_type](attribute const & attr) {
      return attr_type == attr.type;
    });
  if (itr == m_attrs.end())
    return nullptr;
  attribute const & temp = *itr;
  return &temp;
}


buffer message::encode() const
{
  buffer bytes;
  encoder::encode_u16(bytes, m_header.message_type);
  encoder::encode_u16(bytes, m_header.message_length);
  for (uint8_t b : m_header.transaction_id)
    bytes.push_back(static_cast<uint8_t>(b));
  for (attribute const & v : m_attrs) {
    encoder::encode_u16(bytes, v.type);
    encoder::encode_u16(bytes, v.length);
    bytes.insert(std::end(bytes), std::begin(v.value), std::end(v.value));
  }
  return bytes;
}

message * message_factory::create_binding_request()
{
  message * change_request = new message();
  change_request->m_header.message_type = 1;
  change_request->m_header.message_length = 8;
  details::random_fill(std::begin(change_request->m_header.transaction_id),
    std::end(change_request->m_header.transaction_id));

  // CHANGE-REQUEST
  change_request->m_attrs.push_back({3, 4, {0, 0, 0, 0}});

  return change_request;
}

client::client()
    : m_server("", 0)
    , m_protocol(protocol::af_inet)
    , m_interface("")
    , m_bind_timeout(30)
    , m_cache_timeout(30)
    , m_last_cache_time()
    , m_last_result()
    , m_verbose(true)
    , m_fd(-1)
{
}

client::~client()
{
  if (m_fd != -1)
    close(m_fd);
}

bool client::bind(
    std::string const & hostname, 
    uint16_t port,
    std::string const & interface, 
    protocol proto,
    uint16_t bind_timeout,
    uint16_t cache_timeout,
    bind_result& result)
{
    /*if params change then we force a bind request now instead of using any cached value*/
    bool dirty = false;
    bool ret_ok = false;

    if(m_server.hostname != hostname)
    {
        m_server.hostname = hostname;
        dirty = true;
    }

    if(m_server.port != port)
    {
        m_server.port = port;
        dirty = true;
    }

    if(m_interface != interface)
    {
      m_interface = interface;
      dirty = true;
    }

    if(m_protocol != proto)
    {
      m_protocol = proto;
      dirty = true;
    }

    if(m_bind_timeout != bind_timeout)
    {
      m_bind_timeout = bind_timeout;
      dirty = true;
    }

    if(m_cache_timeout != cache_timeout)
    {
      m_cache_timeout = cache_timeout;
      dirty = true;
    }

    verbose("client::bind enter: server=%s port=%u iface=%s ipv6=%u timeout=%u cache_timeout=%u dirty=%u\n",
        hostname.c_str(), port, interface.c_str(), proto == stun::protocol::af_inet6, bind_timeout, cache_timeout, dirty);

    if(m_cache_timeout > 0        /*asking if caching is enabled*/
    && !dirty                     /*if dirty then the caller has changed our settings and wants a new bind no matter what*/
    && m_last_result.is_valid())  /*we actually have a valid cached response*/
    {
        auto time_in_cache = std::chrono::duration_cast<std::chrono::seconds>(
            std::chrono::steady_clock::now() - m_last_cache_time);

        verbose("client::bind cache time=%lld\n", time_in_cache.count());

        if(time_in_cache.count() < m_cache_timeout)
        {
            result = m_last_result;

            verbose("client::bind returning cached result: %s\n", result.public_ip.c_str());
            return true;
        }
        else
        {
            verbose("client::client::bind cached result expired\n");
        }
    }

    #ifdef __cpp_exceptions
    try
    #endif
    {
        int interval_wait_time = STUN_DEFAULT_WAIT_INTERVAL;
        int num_attempts = STUN_DEFAULT_MAX_ATTEMPTS;
        int total_time = 0;
        int sleep_time = 1;

        std::chrono::milliseconds wait_time(interval_wait_time);
        for (int i = 0; i < num_attempts && total_time < m_bind_timeout; ++i)
        {
            verbose("client::bind sending bind request\n");

            std::unique_ptr<stun::message> binding_response = send_binding_request(wait_time);

            total_time += interval_wait_time;/*FXIME should do a clock delta and not use wait_time which is the max*/
            /*do a multiple of 2 sleep -- FIXIME should use cond variable so user to cancel this*/
            if (i > 0)
            {
                if (total_time + sleep_time > m_bind_timeout)
                    sleep_time = m_bind_timeout - total_time;
                if (sleep_time < 0)
                    break;
                sleep(sleep_time);
                total_time += sleep_time;
                if (sleep_time < 32)/*put some max limit on how long we wait*/
                    sleep_time *= 2;
            }
            if (binding_response)
            {
                stun::attribute const * mapped_address = binding_response->find_attribute(stun::attribute_type::mapped_address);
                if (mapped_address)
                {
                    sockaddr_storage addr = stun::attributes::mapped_address(*mapped_address).addr();

                    result.public_ip = m_last_result.public_ip = stun::sockaddr_to_string(addr);

                    m_last_cache_time = std::chrono::steady_clock::now();

                    verbose("client::bind success: public_ip=%s\n", result.public_ip.c_str());

                    ret_ok = true;
                }
                else
                {
                    verbose("client::bind failed: ip missing from binding response\n");
                }
            }
            else
            {
                verbose("client::bind failed: no response received from server\n");
            }
        }
    }
#ifdef __cpp_exceptions
    catch (std::exception const & err)
    {
        verbose("client::bind failed: %s\n", err.what());
    }
#endif

    if (m_fd != -1)
    {
        close(m_fd);
        m_fd = -1;
    }

    if(!ret_ok)
      result.invalidate();

    return ret_ok;
}

void client::create_udp_socket(int inet_family)
{
  if (inet_family != AF_INET && inet_family != AF_INET6)
    details::throw_error("invalid inet family:%d", inet_family);

  verbose("creating udp/%s socket\n", details::family_to_string(inet_family));

  int soc = socket(inet_family, SOCK_DGRAM | SOCK_CLOEXEC, 0);
  if (soc < 0)
    details::throw_error("error creating socket. %s", strerror(errno));

  #ifdef _SUN_USE_MSGHDR
  int optval = 1;
  setsockopt(soc, IPPROTO_IP, IP_PKTINFO, &optval, sizeof(int));
  #endif

  if (!m_interface.empty()) {
    sockaddr_storage local_addr = details::get_interface_address(m_interface, inet_family);

    verbose("binding to local interface %s/%s\n", m_interface.c_str(),
      sockaddr_to_string(local_addr).c_str());

    int ret = ::bind(soc, reinterpret_cast<sockaddr const *>(&local_addr), details::socket_length(local_addr));
    if (ret < 0) {
      int err = errno;
      close(soc);
      details::throw_error("failed to bind socket to local address '%s'. %s",
          sockaddr_to_string(local_addr).c_str(), strerror(err));
    }
    else {
      if (m_verbose) {
        sockaddr_storage local_endpoint;
        socklen_t socklen = sizeof(sockaddr_storage);
        int ret = getsockname(soc, reinterpret_cast<sockaddr *>(&local_endpoint), &socklen);
        if (ret == 0)
          verbose("local endpoint %s/%d\n", sockaddr_to_string(local_endpoint).c_str(),
            details::sockaddr_get_port(local_endpoint));
      }
    }
  }
  else
    verbose("no local interface supplied to bind to\n");

  if (m_fd != -1)
    close(m_fd);
  m_fd = soc;
  return;
}

message * client::send_message(sockaddr_storage const & remote_addr, message const & req,
  std::chrono::milliseconds wait_time, int * local_iface_index)
{
  if (m_fd < 0)
      return nullptr;

  buffer bytes = req.encode();

  STUN_TRACE("remote_addr:%s\n", sockaddr_to_string(remote_addr).c_str());

  #ifdef _STUN_DEBUG
  details::dump_buffer("STUN >>> ", bytes);
  #endif

  verbose("sending messsage\n");

  ssize_t n = sendto(m_fd, &bytes[0], bytes.size(), 0, (sockaddr *) &remote_addr, details::socket_length(remote_addr));
  if (n < 0)
    details::throw_error("failed to send packet. %s", strerror(errno));

  bytes.resize(0);
  bytes.reserve(256);
  bytes.resize(256);

  sockaddr_storage from_addr = {};

  fd_set rfds;
  FD_ZERO(&rfds);
  FD_SET(m_fd, &rfds);

  timeval timeout;
  timeout.tv_usec = 1000 * wait_time.count();
  timeout.tv_sec = 0;

  constexpr decltype(timeout.tv_sec) kMicrosecondsPerSecond = 1000000;
  if (timeout.tv_usec > kMicrosecondsPerSecond) {
    timeout.tv_sec = (timeout.tv_usec / kMicrosecondsPerSecond);
    timeout.tv_usec -= (timeout.tv_sec * kMicrosecondsPerSecond);
  }
  verbose("waiting for response, timeout set to %lus - %luus\n", timeout.tv_sec, timeout.tv_usec);
  int ret = select(m_fd + 1, &rfds, nullptr, nullptr, &timeout);
  if (ret == 0) {
    STUN_TRACE("select timeout out\n");
    return nullptr;
  }

  //
  // XXX: For discovering the network type, the first test is to run a binding request and
  // compare the response to the local address/port combo. I was attempting to find the
  // local address/port without an explicit bind() on the local socket fd. You can usethe
  // recvmsg() to get the interface index where the UDP packets come in, but you can't get
  // the port. For now, in order for the discovery to work, you have to choose a local
  // interface name, and call bind() on the addr.
  //
  // At some point, I'll come back to this and check whether you can find the port without
  // an explicit bind, possibly using sendmsg() or another option.
  // @see [in this file] client::create_udp_socket(), there's a call to setsockopt() which
  // enables the retrieval of the IP_PKTINFO
  //
  #ifdef _STUN_USE_MSGHDR
  do {
    //buffer control( CMSG_SPACE(sizeof(struct in_addr)) + CMSG_SPACE(sizeof(struct in_pktinfo)) + 
    //  sizeof(struct cmsghdr) );
    uint8_t control_data[256];

    struct msghdr msg = {};
    struct iovec iov = {};

    iov.iov_base = &bytes[0];
    iov.iov_len = bytes.size();

    msg.msg_flags = 0;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    msg.msg_control = control_data;
    msg.msg_controllen = sizeof(control_data);
    msg.msg_name = &from_addr;
    msg.msg_namelen = sizeof(from_addr);

    n = recvmsg(m_fd, &msg, 0);
    if ((n > 0) && local_iface_index) {
      for (cmsghdr * cptr = CMSG_FIRSTHDR(&msg); cptr; cptr = CMSG_NXTHDR(&msg, cptr)) {
        if (cptr->cmsg_level == IPPROTO_IP) {
          if (cptr->cmsg_type == IP_PKTINFO)
            *local_iface_index = reinterpret_cast<in_pktinfo *>(CMSG_DATA(cptr))->ipi_ifindex;
          else if (cptr->cmsg_type == IPV6_PKTINFO)
            *local_iface_index =  reinterpret_cast<in6_pktinfo *>(CMSG_DATA(cptr))->ipi6_ifindex;
        }
      }
    }
  } while (n < 0 && errno == EINTR);
  #else
  do {
    socklen_t len = sizeof(sockaddr_storage);
    n = recvfrom(m_fd, &bytes[0], bytes.size(), MSG_WAITALL, (sockaddr *) &from_addr, &len);
  } while (n == -2 && errno == EINTR);
  #endif

  if (n < 0)
    details::throw_error("error receiving on socket. %s", strerror(errno));
  else
    bytes.resize(n);

  #ifdef _STUN_DEBUG
  details::dump_buffer("STUN <<< ", bytes);
  #endif

  return decoder::decode_message(bytes, nullptr);
}

void client::verbose(char const * format, ...)
{
  if (!m_verbose)
    return;
  va_list ap;
  va_start(ap, format);
  printf("STUN:");
  vprintf(format, ap);
  va_end(ap);
  return;
}

network_access_type client::discover_network_access_type(server const & srv)
{
  std::chrono::milliseconds wait_time(250);
  std::vector<sockaddr_storage> addrs = details::resolve_hostname(srv.hostname, srv.port, m_protocol);

  sockaddr_storage server_addr = {};

  std::unique_ptr<message> binding_response;
  for (sockaddr_storage const & addr : addrs) {
    binding_response = this->send_binding_request(addr, wait_time);
    if (binding_response) {
      server_addr = addr;
      break;
    }
    else
      wait_time = std::min(wait_time * 2, details::binding_requests_wait_time_max);
  }

  if (!binding_response)
    return network_access_type::udp_blocked;

  // get endpoint binding_request was sent from and compare to the binding_response
  // if they're the same, run "test II".
  sockaddr_storage local_endpoint;
  socklen_t socklen = sizeof(sockaddr_storage);
  int ret = getsockname(m_fd, reinterpret_cast<sockaddr *>(&local_endpoint), &socklen);
  if (ret == -1)
    details::throw_error("failed to get local socket name:%s", strerror(errno));

  local_endpoint.ss_family = AF_INET;
  std::string s = sockaddr_to_string(local_endpoint);

  return network_access_type::unknown;
}

std::unique_ptr<message> client::send_binding_request(std::chrono::milliseconds wait_time)
{
  std::unique_ptr<message> binding_response;
  std::vector<sockaddr_storage> addrs = details::resolve_hostname(m_server.hostname, m_server.port, m_protocol);
  for (sockaddr_storage const & addr : addrs) {
    binding_response = this->send_binding_request(addr, wait_time);
    if (binding_response)
      break;
    else
      wait_time = std::min(wait_time * 2, details::binding_requests_wait_time_max);
  }
  return binding_response;
}

std::unique_ptr<message> client::send_binding_request(sockaddr_storage const & addr, 
  std::chrono::milliseconds wait_time)
{
  this->verbose("sending binding request with wait time:%lld ms\n", wait_time.count());
  this->create_udp_socket(addr.ss_family);
  std::unique_ptr<message> binding_request(message_factory::create_binding_request());
  std::unique_ptr<message> binding_response(this->send_message(addr, *binding_request, wait_time));
  return binding_response;
}

attributes::address::address(attribute const & attr)
{
  size_t offset = 0;

  // the family is actually 8-bits, but the pkt has a 1 byte padding
  // for alignment
  uint16_t family = decoder::decode_u16(attr.value, &offset);
  if (family == 1) {
    sockaddr_in * v4 = reinterpret_cast<sockaddr_in *>(&m_addr);
    v4->sin_port = decoder::decode_u16(attr.value, &offset);
    v4->sin_addr.s_addr = htonl(decoder::decode_u32(attr.value, &offset));
    m_addr = * reinterpret_cast<sockaddr_storage *>(v4);
    m_addr.ss_family = AF_INET;
  }
  else if (family == 2) {
    sockaddr_in6 * v6 = reinterpret_cast<sockaddr_in6 *>(&m_addr);
    v6->sin6_port = decoder::decode_u16(attr.value, &offset);
    for (int i = 0; i < 16; ++i)
      v6->sin6_addr.s6_addr[i] = attr.value[offset + i];
    m_addr = * reinterpret_cast<sockaddr_storage *>(v6);
    m_addr.ss_family = AF_INET6;
  }
  else
    details::throw_error("invalid mapped address family:%d", family);
}

uint32_t decoder::decode_u32(buffer const & buff, size_t * offset)
{
  uint32_t const * p = reinterpret_cast<uint32_t const *>(&buff[*offset]);
  uint32_t value = ntohl(*p);
  *offset += 4;
  return value;
}

uint16_t decoder::decode_u16(buffer const & buff, size_t * offset)
{
  uint16_t const * p = reinterpret_cast<uint16_t const *>(&buff[*offset]);
  uint16_t value = ntohs(*p);
  *offset += 2;
  return value;
}

message * decoder::decode_message(buffer const & buff, size_t * offset)
{
  size_t temp_offset = 0;
  if (offset)
    temp_offset = *offset;

  // TODO: use a factory
  // create  a map[ message_type ] = message_factory_method

  message * new_message = nullptr;
  message_header header;
  header.message_type = decoder::decode_u16(buff, &temp_offset);
  header.message_length = decoder::decode_u16(buff, &temp_offset);
  if (header.message_type == message_type::binding_response) {
    for (size_t i = 0, n = header.transaction_id.size(); i < n; ++i)
      header.transaction_id[i] = buff[temp_offset++ + i];
    new_message = new message();
    new_message->m_header = header;
    while (temp_offset < buff.size())
      new_message->m_attrs.push_back(decoder::decode_attr(buff, &temp_offset)); 
  }
  else {
    // TODO: unsupported message type
  }

  if (offset)
    *offset = temp_offset;

  return new_message;
}

attribute decoder::decode_attr(buffer const & buff, size_t * offset)
{
  attribute t = {};
  t.type = decoder::decode_u16(buff, offset);
  t.length = decoder::decode_u16(buff, offset);
  t.value.insert(std::end(t.value), std::begin(buff) + *offset,
      std::begin(buff) + *offset + t.length);
  *offset += t.value.size();
  return t;
}

void encoder::encode_u16(buffer & buff, uint16_t n)
{
  uint16_t temp = htons(n);
  uint8_t * p = reinterpret_cast< uint8_t * >(&temp);
  buff.push_back(p[0]);
  buff.push_back(p[1]);
  return;
}

void encoder::encode_u32(buffer & buff, uint32_t n)
{
  uint32_t temp = htons(n);
  uint8_t * p = reinterpret_cast<uint8_t *>(&temp);
  buff.push_back(p[0]);
  buff.push_back(p[1]);
  buff.push_back(p[2]);
  buff.push_back(p[3]);
  return;
}

std::string sockaddr_to_string(sockaddr_storage const & addr)
{
  sockaddr const * temp = reinterpret_cast<sockaddr const *>(&addr);
  return details::sockaddr_to_string2(temp, addr.ss_family);
}

} // end namespace stun
