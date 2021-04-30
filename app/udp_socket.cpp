#include <memory.h>
#include <set>
#include <iostream>
#include "udp_socket.hpp"

// submodules
#include "spdlog/spdlog.h"

using namespace std;
using shared_udp = shared_ptr<socket_udp>;

#ifndef _WIN32
#define NET_ERROR errno
#else
#define NET_ERROR WSAGetLastError()
#endif

#define LOG_SOCK_UDP "[UDP] "

extern const set<string> false_names = { "0", "no", "off", "false" };

sockaddr_any CreateAddr(const string& name, unsigned short port, int pref_family = AF_UNSPEC)
{
	// Handle empty name.
	// If family is specified, empty string resolves to ANY of that family.
	// If not, it resolves to IPv4 ANY (to specify IPv6 any, use [::]).
	if (name == "")
	{
		sockaddr_any result(pref_family == AF_INET6 ? pref_family : AF_INET);
		result.hport(port);
		return result;
	}

	bool first6 = pref_family != AF_INET;
	int families[2] = {AF_INET6, AF_INET};
	if (!first6)
	{
		families[0] = AF_INET;
		families[1] = AF_INET6;
	}

	for (int i = 0; i < 2; ++i)
	{
		int family = families[i];
		sockaddr_any result (family);

		// Try to resolve the name by pton first
		if (inet_pton(family, name.c_str(), result.get_addr()) == 1)
		{
			result.hport(port); // same addr location in ipv4 and ipv6
			return result;
		}
	}

	// If not, try to resolve by getaddrinfo
	// This time, use the exact value of pref_family

	sockaddr_any result;
	addrinfo fo = {
		0,
		pref_family,
		0, 0,
		0, 0,
		NULL, NULL
	};

	addrinfo* val = nullptr;
	int erc = getaddrinfo(name.c_str(), nullptr, &fo, &val);
	if (erc == 0)
	{
		result.set(val->ai_addr);
		result.len = result.size();
		result.hport(port); // same addr location in ipv4 and ipv6
	}
	freeaddrinfo(val);

	return result;
}

socket_udp::socket_udp(const UriParser &src_uri)
	: m_host(src_uri.host())
	, m_port(src_uri.portno())
	, m_options(src_uri.parameters())
	, m_dst_addr(sockaddr_any())
{
	sockaddr_in sa     = sockaddr_in();
	sa.sin_family      = AF_INET;
	sa.sin_addr.s_addr = INADDR_ANY;
	m_bind_socket      = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (m_bind_socket == INVALID_SOCKET)
		throw runtime_error("Failed to create a UDP socket. Error code: " + to_string(NET_ERROR));

	int yes = 1;
	::setsockopt(m_bind_socket, SOL_SOCKET, SO_REUSEADDR, (const char *)&yes, sizeof yes);

#if defined(_WIN32)
	unsigned long ulyes = 1;
	if (ioctlsocket(m_bind_socket, FIONBIO, &ulyes) == SOCKET_ERROR)
#else
	if (ioctl(m_bind_socket, FIONBIO, (const char *)&yes) < 0)
#endif
	{
		throw runtime_error("UdpCommon::Setup: ioctl FIONBIO");
	}

	sockaddr_any sa_requested;
	try
	{
		sa_requested = CreateAddr(m_host, m_port);
	}
	catch (const std::invalid_argument&)
	{
		throw runtime_error("create_addr_inet failed");
	}

	const auto bind_me = [&](const sockaddr_any& sa) {
		const int       bind_res = ::bind(m_bind_socket, sa.get(), sa.size());
		if (bind_res < 0)
		{
			throw runtime_error("UDP binding has failed");
		}
	};

	bool ip_bonded = false;
	if (m_options.count("bind"))
	{
		string bindipport = m_options.at("bind");
		transform(bindipport.begin(), bindipport.end(), bindipport.begin(), [](char c) { return tolower(c); });
		const size_t idx = bindipport.find(":");
		const string bindip = bindipport.substr(0, idx);
		const int bindport = idx != string::npos
			? stoi(bindipport.substr(idx + 1, bindipport.size() - (idx + 1)))
			: m_port;
		m_options.erase("bind");

		sockaddr_any sa_bind;
		try
		{
			sa_bind = CreateAddr(bindip, bindport);
		}
		catch (const std::invalid_argument&)
		{
			throw runtime_error("create_addr_inet failed");
		}

		bind_me(sa_bind);
		ip_bonded = true;
	}

	if (m_host != "" || ip_bonded)
	{
		m_dst_addr = sa_requested;
		spdlog::info("{}, destination address {}", ip_bonded ? "Bound" : "Not bound", m_dst_addr.load().str());
	}
	else
	{
		bind_me(reinterpret_cast<const sockaddr*>(&sa_requested));
		spdlog::info("Binding to {}", sa_requested.str());
	}
}

socket_udp::~socket_udp() { closesocket(m_bind_socket); }

sockaddr_any socket_udp::get_sockaddr() const
{
	// The getsockname function requires only to have enough target
	// space to copy the socket name, it doesn't have to be correlated
	// with the address family. So the maximum space for any name,
	// regardless of the family, does the job.
	sockaddr_any addr;
	socklen_t namelen = addr.storage_size();
	::getsockname(m_bind_socket, (addr.get()), (&namelen));
	addr.len = namelen;
	std::cerr << "UDP address " << addr.str() << std::endl;
	return addr;
}

size_t socket_udp::recv(const mut_bufv& buffer, int timeout_ms)
{
	while (!m_blocking_mode)
	{
		fd_set set;
		timeval tv;
		FD_ZERO(&set);
		FD_SET(m_bind_socket, &set);
		tv.tv_sec = 0;
		tv.tv_usec = 10000;
		const int select_ret = ::select((int)m_bind_socket + 1, &set, nullptr, &set, &tv);

		if (select_ret != 0)    // ready
			break;

		if (timeout_ms >= 0)   // timeout
			return 0;
	}

	const int res =
		::recvfrom(m_bind_socket, reinterpret_cast<char*>(buffer.data()), (int)buffer.size()
			, 0, nullptr, nullptr);

	if (res == -1)
	{
#ifndef _WIN32
#define NET_ERROR errno
#else
#define NET_ERROR WSAGetLastError()
#endif
		const int err = NET_ERROR;
		if (err != EAGAIN && err != EINTR && err != ECONNREFUSED)
			throw runtime_error("udp::recv::recv");

		spdlog::info("UDP reading failed: error {0}. Again.", err);
		return 0;
	}

	return static_cast<size_t>(res);
}

std::pair<size_t, sockaddr_any> socket_udp::recvfrom(const mut_bufv &buffer, int timeout_ms)
{
	typedef std::pair<size_t, sockaddr_any> return_pair;
	while (!m_blocking_mode)
	{
		fd_set set;
		timeval tv;
		FD_ZERO(&set);
		FD_SET(m_bind_socket, &set);
		tv.tv_sec = 0;
		tv.tv_usec = 10000;
		const int select_ret = ::select((int)m_bind_socket + 1, &set, nullptr, &set, &tv);

		if (select_ret != 0)    // ready
			break;

		if (timeout_ms >= 0)   // timeout
			return return_pair();
	}

	sockaddr_any peer_addr;
	socklen_t addrlen = peer_addr.storage_size();

	const int res =
		::recvfrom(m_bind_socket, reinterpret_cast<char*>(buffer.data()), (int)buffer.size()
			, 0, peer_addr.get(), &addrlen);

	if (res == -1)
	{
#ifndef _WIN32
#define NET_ERROR errno
#else
#define NET_ERROR WSAGetLastError()
#endif
		const int err = NET_ERROR;
		if (err != EAGAIN && err != EINTR && err != ECONNREFUSED)
		{
			spdlog::error("UDP reading failed: error {0}.", err);
			throw runtime_error("udp::recv::recv");
		}

		spdlog::info("UDP reading failed: error {0}. Again.", err);
		return return_pair();
	}

	return return_pair(static_cast<size_t>(res), peer_addr);
}

int socket_udp::sendto(const sockaddr_any& dst_addr, const const_bufv& buffer, int timeout_ms)
{
	while (!m_blocking_mode)
	{
		fd_set set;
		timeval tv;
		FD_ZERO(&set);
		FD_SET(m_bind_socket, &set);
		tv.tv_sec = 0;
		tv.tv_usec = 10000;
		const int select_ret = ::select((int)m_bind_socket + 1, nullptr, &set, &set, &tv);

		if (select_ret != 0)    // ready
			break;

		if (timeout_ms >= 0)   // timeout
			return 0;
	}

	const int res = ::sendto(m_bind_socket,
		reinterpret_cast<const char*>(buffer.data()),
		(int)buffer.size(),
		0,
		dst_addr.get(),
		dst_addr.size());
	if (res == -1)
	{
#ifndef _WIN32
#define NET_ERROR errno
#else
#define NET_ERROR WSAGetLastError()
#endif
		const int err = NET_ERROR;
		spdlog::error("UDP sending failed: error {0}.", err);
		throw runtime_error("udp::send::send");
	}

	return static_cast<size_t>(res);
}

int socket_udp::send(const const_bufv &buffer, int timeout_ms)
{
	return sendto(m_dst_addr, buffer, timeout_ms);
}
