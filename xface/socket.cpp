#include "socket.h"

#pragma comment(lib, "Ws2_32.lib")

namespace {
enum sock_type_s {
	SOCK_STREAM = 1, /* stream socket */
	SOCK_DGRAM = 2, /* datagram socket */
	SOCK_RAW = 3, /* raw-protocol interface */
	SOCK_RDM = 4, /* reliably-delivered message */
	SOCK_SEQPACKET = 5, /* sequenced packet stream */
};
enum sock_shutdown_s { SD_RECEIVE, SD_SEND, SD_BOTH };
// Although AF_UNSPEC is defined for backwards compatibility, using
// AF_UNSPEC for the "af" parameter when creating a socket is STRONGLY
// DISCOURAGED.  The interpretation of the "protocol" parameter
// depends on the actual address family chosen.  As environments grow
// to include more and more address families that use overlapping
// protocol values there is more and more chance of choosing an
// undesired address family when AF_UNSPEC is used.
enum adressfamily {
	AF_UNSPEC, // unspecified
	AF_UNIX, // local to host (pipes, portals)
	AF_INET, // internetwork: UDP, TCP, etc.
	AF_IMPLINK, // arpanet imp addresses
	AF_PUP, // pup protocols: e.g. BSP
	AF_CHAOS, // mit CHAOS protocols
	AF_NS, // XEROX NS protocols
	AF_IPX = AF_NS, // IPX protocols: IPX, SPX, etc.
	AF_ISO, // ISO protocols
	AF_OSI = AF_ISO, // OSI is ISO
	AF_ECMA, // european computer
	AF_DATAKIT, // datakit protocols
	AF_CCITT, // CCITT protocols, X.25
	AF_SNA, // IBM SNA
	AF_DECnet, // DECnet
	AF_DLI, // Direct data link interface
	AF_LAT, // LAT
	AF_HYLINK, // NSC Hyperchannel
	AF_APPLETALK, // AppleTalk
	AF_NETBIOS, // NetBios-style addresses
	AF_VOICEVIEW, // VoiceView
	AF_FIREFOX, // Protocols from Firefox
	AF_UNKNOWN1, // Somebody is using this!
	AF_BAN, // Banyan
	AF_ATM, // Native ATM Services
	AF_INET6, // Internetwork Version 6
	AF_CLUSTER, // Microsoft Wolfpack
	AF_12844, // IEEE 1284.4 WG AF
	AF_IRDA, // IrDA
	AF_NETDES = 28 // Network Designers OSI & gateway
};
enum protocolformat {
	PF_UNSPEC = AF_UNSPEC,
	PF_UNIX = AF_UNIX,
	PF_INET = AF_INET,
	PF_IMPLINK = AF_IMPLINK,
	PF_PUP = AF_PUP,
	PF_CHAOS = AF_CHAOS,
	PF_NS = AF_NS,
	PF_IPX = AF_IPX,
	PF_ISO = AF_ISO,
	PF_HYLINK = AF_HYLINK,
	PF_APPLETALK = AF_APPLETALK,
	PF_FIREFOX = AF_FIREFOX,
	PF_INET6 = AF_INET6,
};
enum protocol_s {
	IPPROTO_TCP = 6
};
enum adress_s {
	AI_PASSIVE = 0x00000001, // Socket address will be used in bind() call
	AI_CANONNAME = 0x00000002,  // Return canonical name in first ai_canonname
	AI_NUMERICHOST = 0x00000004,  // Nodename must be a numeric address string
	AI_NUMERICSERV = 0x00000008,  // Servicename must be a numeric port number
	AI_DNS_ONLY = 0x00000010,  // Restrict queries to unicast DNS only (no LLMNR, netbios, etc.)
};
struct addrinfo {
	int                     ai_flags;       // AI_PASSIVE, AI_CANONNAME, AI_NUMERICHOST
	int                     ai_family;      // PF_xxx
	int                     ai_socktype;    // SOCK_xxx
	int                     ai_protocol;    // 0 or IPPROTO_xxx for IPv4 and IPv6
	size_t                  ai_addrlen;     // Length of ai_addr
	char *                  ai_canonname;   // Canonical name for nodename
	struct sockaddr *       ai_addr;        // Binary address
	struct addrinfo *       ai_next;        // Next structure in linked list
};
struct WSAData {
	unsigned short			wVersion;
	unsigned short			wHighVersion;
#ifdef _WIN64
	unsigned short          iMaxSockets;
	unsigned short          iMaxUdpDg;
	char FAR *              lpVendorInfo;
	char                    szDescription[WSADESCRIPTION_LEN + 1];
	char                    szSystemStatus[WSASYS_STATUS_LEN + 1];
#else
	char                    szDescription[256 + 1];
	char                    szSystemStatus[128 + 1];
	unsigned short          iMaxSockets;
	unsigned short          iMaxUdpDg;
	char*					lpVendorInfo;
#endif
};
}
extern "C" {
	int __stdcall accept(int s, struct sockaddr* addr, int* addrlen);
	int	atexit(void(*func)(void));
	int __stdcall bind(int s, const struct sockaddr* name, int namelen);
	int __stdcall closesocket(int s);
	int __stdcall connect(int s, const struct sockaddr* name, int namelen);
	void __stdcall freeaddrinfo(addrinfo* pAddrInfo);
	int __stdcall getaddrinfo(const char* pNodeName, const char* pServiceName, const addrinfo* pHints, addrinfo** ppREsult);
	int __stdcall listen(int s, int backlog);
	int __stdcall recv(int s, char* buffer, int len, int flags);
	int __stdcall send(int s, const char* buf, int len, int flags);
	int __stdcall shutdown(int s, int how);
	int __stdcall socket(int af, int type, int protocol);
	int __stdcall WSAGetLastError();
	int __stdcall WSACleanup();
	int __stdcall WSAStartup(unsigned short wVersionRequested, WSAData* lpWSAData);
	int errno;
}

static void deinitialize() {
	WSACleanup();
}

int io::sock::initialize() {
	WSAData wsaData;
	auto error = WSAStartup(0x0202, &wsaData);
	if(error)
		return error;
	atexit(deinitialize);
	return 0;
}

void io::sock::close() {
	if(s != -1)
		closesocket(s);
	s = -1;
}

int io::sock::createserver(const char* port) {
	struct addrinfo *result = 0, hints = {0};
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;
	// Resolve the server address and port
	auto error = getaddrinfo(0, port, &hints, &result);
	if(error != 0)
		return error;
	// Create a SOCKET for connecting to server
	s = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if(s == -1) {
		error = getlasterror();
		freeaddrinfo(result);
		return error;
	}
	// Setup the TCP listening socket
	error = bind(s, result->ai_addr, (int)result->ai_addrlen);
	if(error == -1) {
		error = getlasterror();
		freeaddrinfo(result);
		close();
		return error;
	}
	freeaddrinfo(result);
	error = listen(s, 0x7fffffff); // SOMAXCONN
	if(error == -1) {
		error = getlasterror();
		close();
		return error;
	}
	return 0;
}

int	io::sock::waitclient(sock& client) const {
	client.close();
	client.s = accept(s, 0, 0);
	if(client.s == -1)
		return getlasterror();
	return 0;
}

int io::sock::open(const char* url, const char* port) {
	struct addrinfo *result = 0, hints = {0};
	hints.ai_family = 0; //AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = 6; // IPPROTO_TCP;
	// Resolve the server address and port
	auto error = getaddrinfo(url, port, &hints, &result);
	if(error)
		return error;
	// Attempt to connect to an address until one succeeds
	for(auto ptr = result; ptr != 0; ptr = ptr->ai_next) {
		// Create a SOCKET for connecting to server
		s = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
		if(s == -1)
			return WSAGetLastError();
		// Connect to server.
		auto result = connect(s, ptr->ai_addr, (int)ptr->ai_addrlen);
		if(result == -1) {
			close();
			continue;
		}
		break;
	}
	freeaddrinfo(result);
	return 0;
}

int io::sock::write(const char* buffer, unsigned size) {
	return send(s, buffer, size, 0);
}

int io::sock::read(char* buffer, unsigned size) {
	return recv(s, buffer, size, 0);
}

int io::sock::stopread() {
	return shutdown(s, SD_RECEIVE);
}

int io::sock::stopwrite() {
	return shutdown(s, SD_SEND);
}

int	io::sock::getlasterror() {
	return errno;
}