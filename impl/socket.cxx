// SPDX-License-Identifier: BSD-3-Clause
#include <cstring>
#ifndef _MSC_VER
#	include <sys/socket.h>
#	include <netdb.h>
#	include <arpa/inet.h>
#	include <netinet/in.h>
#endif

#include <substrate/utility>
#include <substrate/socket>

#ifndef _MSC_VER
#	include <unistd.h>
inline int closesocket(const int s) { return close(s); }
#else
#	include <Winsock2.h>
#endif

using namespace substrate;

size_t sockaddrLen(const sockaddr_storage &addr) noexcept
{
	switch (addr.ss_family)
	{
		case AF_INET:
			return sizeof(sockaddr_in);
		case AF_INET6:
			return sizeof(sockaddr_in6);
	}
	return sizeof(sockaddr);
}

socket_t::socket_t(const int family, const int type, const int protocol) noexcept :
	socket(::socket(family, type, protocol)) { }
socket_t::~socket_t() noexcept
	{ if (socket != -1) closesocket(socket); }
bool socket_t::bind(const void *const addr, const size_t len) const noexcept
	{ return ::bind(socket, static_cast<const sockaddr *>(addr), len) == 0; }
bool socket_t::bind(const sockaddr_storage &addr) const noexcept
	{ return bind(static_cast<const void *>(&addr), sockaddrLen(addr)); }
bool socket_t::connect(const void *const addr, const size_t len) const noexcept
	{ return ::connect(socket, static_cast<const sockaddr *>(addr), len) == 0; }
bool socket_t::connect(const sockaddr_storage &addr) const noexcept
	{ return connect(static_cast<const void *>(&addr), sockaddrLen(addr)); }
bool socket_t::listen(const int32_t queueLength) const noexcept
	{ return ::listen(socket, queueLength) == 0; }
socket_t socket_t::accept(sockaddr *peerAddr, socklen_t *peerAddrLen) const noexcept
	{ return ::accept(socket, peerAddr, peerAddrLen); }
#ifndef _MSC_VER
ssize_t socket_t::write(const void *const bufferPtr, const size_t len) const noexcept
	{ return ::write(socket, bufferPtr, len); }
ssize_t socket_t::read(void *const bufferPtr, const size_t len) const noexcept
	{ return ::read(socket, bufferPtr, len); }
#else
ssize_t socket_t::write(const void *const bufferPtr, const size_t len) const noexcept
	{ return ::send(socket, static_cast<const char *const>(bufferPtr), int32_t(len), 0); }
ssize_t socket_t::read(void *const bufferPtr, const size_t len) const noexcept
	{ return ::recv(socket, static_cast<char *const>(bufferPtr), int32_t(len), 0); }
#endif

char socket_t::peek() const noexcept
{
	char buffer{};
	if (::recv(socket, &buffer, 1, MSG_PEEK) != 1)
		return {};
	return buffer;
}

inline int typeToFamily(const socketType_t type) noexcept
{
	if (type == socketType_t::ipv4)
		return AF_INET;
	else if (type == socketType_t::ipv6)
		return AF_INET6;
	return AF_UNSPEC;
}

inline size_t familyToSize(const sa_family_t family) noexcept
{
	if (family == AF_INET)
		return sizeof(sockaddr_in);
	else if (family == AF_INET6)
		return sizeof(sockaddr_in6);
	return sizeof(sockaddr_storage);
}

inline uint16_t toBE(const uint16_t value) noexcept
{
	const std::array<uint8_t, 2> resultBytes
	{
		uint8_t(value >> 8U),
		uint8_t(value)
	};
	uint16_t result{};
	memcpy(&result, resultBytes.data(), resultBytes.size());
	return result;
}

template<size_t offset> inline void *offsetPtr(void *ptr)
{
	const auto addr = reinterpret_cast<std::uintptr_t>(ptr); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast) lgtm[cpp/reinterpret-cast]
	return reinterpret_cast<void *>(addr + offset); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast) lgtm[cpp/reinterpret-cast]
}

template<size_t offset, typename T, typename U> inline void copyToOffset(T &dest, const U value)
	{ memcpy(offsetPtr<offset>(&dest), &value, sizeof(U)); }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
sockaddr_storage substrate::socket::prepare(const socketType_t family, const char *const where,
	const uint16_t port) noexcept
{
	addrinfo hints{};
	hints.ai_family = typeToFamily(family);
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE; // This may not be right/complete..

	addrinfo *results = nullptr;
	if (getaddrinfo(where, nullptr, &hints, &results) || !results)
		return {AF_UNSPEC};

	sockaddr_storage service{};
	memcpy(&service, results->ai_addr, familyToSize(results->ai_addr->sa_family));
	freeaddrinfo(results);

	const auto portBE = toBE(port);
	if (service.ss_family == AF_INET)
		copyToOffset<offsetof(sockaddr_in, sin_port)>(service, portBE);
	else if (service.ss_family == AF_INET6)
		copyToOffset<offsetof(sockaddr_in6, sin6_port)>(service, portBE);
	else
		return {AF_UNSPEC};
	return service;
}
#pragma GCC diagnostic pop
/* vim: set ft=cpp ts=4 sw=4 noexpandtab: */
