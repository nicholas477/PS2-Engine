#include "net_rpc_server.h"

#include <thbase.h>
#include <sifcmd.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>


#include "ps2ip.h"

static void* netRpcHandler(int cmd, void* buffer, int size)
{
	NetRpcBuffer* rpc = (NetRpcBuffer*)buffer;

	printf("sizeof NetRpcBuffer: %u\n", sizeof(NetRpcBuffer));
	printf("sizeof buffer: %u\n", size);

	switch (cmd)
	{
		case PS2LINK_IOP_RPC_FUNC_SOCKET:
			printf("socket rpc call\n");
			rpc->result = socket(rpc->socketArgs.domain, rpc->socketArgs.type, rpc->socketArgs.protocol);
			break;
		case PS2LINK_IOP_RPC_FUNC_BIND:
			printf("bind rpc call\n");
			rpc->result = bind(rpc->bindArgs.sockfd, &rpc->bindArgs.addr, rpc->bindArgs.addrlen);
			break;
		case PS2LINK_IOP_RPC_FUNC_LISTEN:
			printf("listen rpc call\n");
			rpc->result = listen(rpc->listenArgs.sockfd, rpc->listenArgs.backlog);
			break;
		case PS2LINK_IOP_RPC_FUNC_ACCEPT:
			printf("accept rpc call\n");
			rpc->result = accept(rpc->acceptArgs.sockfd, &rpc->acceptArgs.addr, &rpc->acceptArgs.addrlen);
			break;
		case PS2LINK_IOP_RPC_FUNC_CONNECT:
			printf("connect rpc call\n");
			rpc->result = connect(rpc->connectArgs.sockfd, (struct sockaddr*)&rpc->connectArgs.addr, sizeof(rpc->connectArgs.addr));
			break;
		case PS2LINK_IOP_RPC_FUNC_SEND:
			printf("send rpc call\n");
			rpc->result = send(rpc->sendArgs.sockfd, (void*)rpc->sendArgs.buf, rpc->sendArgs.len, rpc->sendArgs.flags);
			break;
		case PS2LINK_IOP_RPC_FUNC_RECV:
			printf("recv rpc call\n");
			rpc->result = recv(rpc->recvArgs.sockfd, rpc->recvArgs.buf, rpc->recvArgs.len, rpc->recvArgs.flags);
			break;
		default:
			printf("unknown rpc call\n");
	}

	if (rpc->result < 0)
	{
		printf("RPC call failed with result: %d\n", rpc->result);
		printf("Err: %s\n", error_to_string(rpc->result));
	}

	return buffer;
}

static SifRpcServerData_t server __attribute((aligned(16)));
static SifRpcDataQueue_t queue __attribute((aligned(16)));
static unsigned char rpc_buffer[512] __attribute((aligned(16)));

static void netrpcThread(void* arg)
{
	int pid;

	SifInitRpc(0);
	pid = GetThreadId();
	SifSetRpcQueue(&queue, pid);
	SifRegisterRpc(&server, PS2LINK_NET_RPC_NUMBER, netRpcHandler, rpc_buffer, 0, 0, &queue);
	SifRpcLoop(&queue); // Never exits
	ExitDeleteThread();
}

int initNetRPCServer(void)
{
	struct _iop_thread th_attr;
	int ret;
	int pid;

	th_attr.attr      = 0x02000000;
	th_attr.option    = 0;
	th_attr.thread    = netrpcThread;
	th_attr.stacksize = 0x800;
	th_attr.priority  = 79;

	pid = CreateThread(&th_attr);
	if (pid < 0)
	{
		printf("IOP: netRpc createThread failed %d\n", pid);
		return -1;
	}

	ret = StartThread(pid, 0);
	if (ret < 0)
	{
		printf("IOP: netRpc startThread failed %d\n", ret);
		DeleteThread(pid);
		return -1;
	}
	return 0;
}