#pragma once
#include <map>
#include <future>
#include <string>

// srtx
#include "platform_sys.hpp"
#include "buf_view.hpp"
#include "netinet_any.hpp"
#include "uri_parser.hpp"


class socket_udp
	: public std::enable_shared_from_this<udp>
{
	using shared_udp = std::shared_ptr<socket_udp>;
	using string     = std::string;

public:
	socket_udp(const UriParser& src_uri);
	~socket_udp();

public:
	void listen();

public:
	int id() const final { return static_cast<int>(m_bind_socket); }

	sockaddr_any get_sockaddr() const;

public:
	/**
	 * @returns The number of bytes received.
	 *
	 * @throws socket_exception Thrown on failure.
	 */
	std::pair<size_t, sockaddr_any>
	       recvfrom(const mut_bufv &buffer, int timeout_ms = -1);

	size_t recv  (const mut_bufv& buffer, int timeout_ms) final;
	int    send  (const const_bufv &buffer, int timeout_ms = -1) final;
	int    sendto(const sockaddr_any& dst_addr, const const_bufv& buffer, int timeout_ms = -1);

private:
	SOCKET m_bind_socket = -1; // INVALID_SOCK;

	sockaddr_any m_dst_addr;

	bool                     m_blocking_mode = true;
	string                   m_host;
	int                      m_port;
	std::map<string, string> m_options; // All other options, as provided in the URI
};

