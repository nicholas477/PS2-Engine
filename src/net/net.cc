#include "net/net.hpp"

#include "egg/assert.hpp"
#include "egg/filesystem.hpp"
#include <cstdlib>
#include "engine.hpp"
#include "threading.hpp"

extern "C" {
#include <netman.h>
#include <ps2ip.h>
#include <sifrpc.h>
#include <ps2link/net_rpc_server.h>
#include <sifcmd.h>
#include <arpa/inet.h>
}

namespace Net
{

static SifRpcClientData_t client_data __attribute__((aligned(64)));
static union
{
	NetRpcBuffer RpcBuffer;
	char buffer[128];
} RpcBuffer __attribute__((aligned(64)));

static int sockHandle;
static bool isConnected = false;

void init()
{
	struct _local
	{
		~_local()
		{
			Threading::sleep(20 * 1000);
		}
	} __local;

	if (SifBindRpc(&client_data, PS2LINK_NET_RPC_NUMBER, 0) < 0)
	{
		printf("Failed to bind ps2link RPC client\n");
		scr_printf("Failed to bind ps2link RPC client\n");
		return;
	}

	// Create the socket
	{
		RpcBuffer.RpcBuffer.socketArgs.domain   = AF_INET;
		RpcBuffer.RpcBuffer.socketArgs.type     = SOCK_STREAM;
		RpcBuffer.RpcBuffer.socketArgs.protocol = 0;
		int res                                 = SifCallRpc(&client_data, PS2LINK_IOP_RPC_FUNC_SOCKET, 0, &RpcBuffer, sizeof(RpcBuffer), &RpcBuffer, sizeof(RpcBuffer), NULL, NULL);
		sockHandle                              = RpcBuffer.RpcBuffer.result;
		check(res == 0);

		if (sockHandle < 0)
		{
			printf("Failed to create socket\n");
			scr_printf("Failed to create socket\n");
			return;
		}
		else
		{
			printf("Socket created successfully, id: %d\n", sockHandle);
			scr_printf("Socket created successfully, id: %d\n", sockHandle);
		}
	}

	// Connect to my computer
	{
		RpcBuffer.RpcBuffer.connectArgs.sockfd          = sockHandle;
		RpcBuffer.RpcBuffer.connectArgs.addr.sin_family = AF_INET;
		RpcBuffer.RpcBuffer.connectArgs.addr.sin_port   = htons(12345); // Replace with your port number
		if ((RpcBuffer.RpcBuffer.connectArgs.addr.sin_addr.s_addr = inet_addr("10.0.1.9")) == INADDR_NONE)
		{
			scr_printf("Invalid IP address\n");
			return;
		}

		int res = SifCallRpc(&client_data, PS2LINK_IOP_RPC_FUNC_CONNECT, 0, &RpcBuffer, sizeof(RpcBuffer), &RpcBuffer, sizeof(RpcBuffer), NULL, NULL);
		check(res == 0);

		if (RpcBuffer.RpcBuffer.result < 0)
		{
			printf("Failed to connect to server, error: %d\n", RpcBuffer.RpcBuffer.result);
			scr_printf("Failed to connect to server, error: %d\n", RpcBuffer.RpcBuffer.result);
			isConnected = false;

			exit(1);
			return;
		}
		else
		{
			printf("Connected to server successfully\n");
			scr_printf("Connected!!!\n");
			isConnected = true;
		}
	}
}

void tick()
{
	if (!isConnected)
		return;

	RpcBuffer.RpcBuffer.sendArgs.sockfd = sockHandle;
	RpcBuffer.RpcBuffer.sendArgs.flags  = 0;
	RpcBuffer.RpcBuffer.sendArgs.buf    = "Hello, world!\n";
	RpcBuffer.RpcBuffer.sendArgs.len    = 15;

	int res = SifCallRpc(&client_data, PS2LINK_IOP_RPC_FUNC_SEND, 0, &RpcBuffer, sizeof(RpcBuffer), &RpcBuffer, sizeof(RpcBuffer), NULL, NULL);
	(void)res;
}
} // namespace Net