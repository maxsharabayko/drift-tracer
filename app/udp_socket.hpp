#pragma once
#include <map>
#include <future>
#include <string>

// srtx
#include "platform_sys.hpp"
#include "buf_view.hpp"
#include "netinet_any.hpp"
#include "uri_parser.hpp"

#if !defined(_WIN32)
#include <sys/ioctl.h>
typedef int SOCKET;
#define INVALID_SOCKET ((SOCKET)-1)
#define closesocket close
#endif

class socket_udp
	: public std::enable_shared_from_this<socket_udp>
{
	using shared_udp = std::shared_ptr<socket_udp>;
	using string     = std::string;

public:
	socket_udp(const UriParser& src_uri);
	~socket_udp();

public:
	void listen();

public:
	int id() const { return static_cast<int>(m_bind_socket); }

	sockaddr_any get_sockaddr() const;
	sockaddr_any dst_addr() const { return m_dst_addr; }
	void set_dst_addr(const sockaddr_any& addr) { m_dst_addr = addr; }

public:
	/**
	 * @returns The number of bytes received.
	 *
	 * @throws socket_exception Thrown on failure.
	 */
	std::pair<size_t, sockaddr_any>
	       recvfrom(const mut_bufv &buffer, int timeout_ms = -1);

	size_t recv  (const mut_bufv& buffer, int timeout_ms);
	int    send  (const const_bufv &buffer, int timeout_ms = -1);
	int    sendto(const sockaddr_any& dst_addr, const const_bufv& buffer, int timeout_ms = -1);

private:
	SOCKET m_bind_socket = -1; // INVALID_SOCK;

	std::atomic<sockaddr_any> m_dst_addr;
	const bool m_blocking_mode = false;

	string                   m_host;
	int                      m_port;
	std::map<string, string> m_options; // All other options, as provided in the URI
};

