#ifndef NET_RPC_SERVER_H
#define NET_RPC_SERVER_H

#include <tcpip.h>
#include <stddef.h>

#define PS2LINK_NET_RPC_NUMBER 0x45004239

enum PS2LINK_IOP_RPC_FUNC_NUMS {
	PS2LINK_IOP_RPC_FUNC_SOCKET  = 0x00,
	PS2LINK_IOP_RPC_FUNC_BIND    = 0x01,
	PS2LINK_IOP_RPC_FUNC_LISTEN  = 0x02,
	PS2LINK_IOP_RPC_FUNC_ACCEPT  = 0x03,
	PS2LINK_IOP_RPC_FUNC_CONNECT = 0x04,
	PS2LINK_IOP_RPC_FUNC_SEND    = 0x05,
	PS2LINK_IOP_RPC_FUNC_RECV    = 0x06,
};

typedef struct
{
	int result;

	union
	{
		struct
		{
			int domain;
			int type;
			int protocol;
		} socketArgs;

		struct
		{
			int sockfd;
			struct sockaddr addr;
			socklen_t addrlen;
		} bindArgs;

		struct
		{
			int sockfd;
			int backlog;
		} listenArgs;

		struct
		{
			int sockfd;
			struct sockaddr addr;
			socklen_t addrlen;
		} acceptArgs;

		struct
		{
			int sockfd;
			struct sockaddr_in addr;
		} connectArgs;

		struct
		{
			int sockfd;
			const void* buf;
			size_t len;
			int flags;
		} sendArgs;

		struct
		{
			int sockfd;
			void* buf;
			size_t len;
			int flags;
		} recvArgs;
	};
} NetRpcBuffer;

int initNetRPCServer(void);
#endif // NET_RPC_SERVER_H