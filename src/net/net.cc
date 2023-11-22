#include "net/net.hpp"

#include "egg/filesystem.hpp"

#include <string.h>
#include <stdio.h>
#include <kernel.h>
#include <iopcontrol.h>
#include <iopheap.h>
#include <debug.h>

extern "C" {
#include <netman.h>
#include <ps2ip.h>
}

#include <sifrpc.h>
#include <loadfile.h>
#include <sbv_patches.h>

static int sock_fd = -1;

static void set_sockaddr(sockaddr_in& address, u8 a, u8 b, u8 c, u8 d)
{
	address.sin_addr.s_addr = htonl(((u32)(a & 0xff) << 24) | ((u32)(b & 0xff) << 16) | ((u32)(c & 0xff) << 8) | (u32)(d & 0xff));
}

static void send_shit()
{
	sock_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	check(sock_fd >= 0);

	sockaddr_in serv_addr;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port   = htons(1938);
	set_sockaddr(serv_addr, 10, 0, 6, 10);
	check(serv_addr.sin_addr.s_addr != INADDR_NONE);

	printf("trying to connect!\n");
	if (connect(sock_fd, (sockaddr*)&serv_addr, sizeof(serv_addr)) >= 0)
	{
		for (int i = 0; i < 100; ++i)
		{
			printf("sending new message!!!!!!!!!!!\n");
			send(sock_fd, (void*)"hello!\n", strlen("hello!\n"), 0);
		}
	}
	printf("Failed!\n");
	closesocket(sock_fd);
}

extern unsigned char DEV9_irx[];
extern unsigned int size_DEV9_irx;

extern unsigned char SMAP_irx[];
extern unsigned int size_SMAP_irx;

extern unsigned char NETMAN_irx[];
extern unsigned int size_NETMAN_irx;

static int ethApplyNetIFConfig(int mode)
{
	int result;
	//By default, auto-negotiation is used.
	static int CurrentMode = NETMAN_NETIF_ETH_LINK_MODE_AUTO;

	if (CurrentMode != mode)
	{ //Change the setting, only if different.
		if ((result = NetManSetLinkMode(mode)) == 0)
			CurrentMode = mode;
	}
	else
		result = 0;

	return result;
}

static void EthStatusCheckCb(s32 alarm_id, u16 time, void* common)
{
	iWakeupThread(*(int*)common);
}

static int WaitValidNetState(int (*checkingFunction)(void))
{
	int ThreadID, retry_cycles;

	// Wait for a valid network status;
	ThreadID = GetThreadId();
	for (retry_cycles = 0; checkingFunction() == 0; retry_cycles++)
	{ //Sleep for 1000ms.
		SetAlarm(1000 * 16, &EthStatusCheckCb, &ThreadID);
		SleepThread();

		if (retry_cycles >= 10) //10s = 10*1000ms
			return -1;
	}

	return 0;
}

static int ethGetNetIFLinkStatus(void)
{
	return (NetManIoctl(NETMAN_NETIF_IOCTL_GET_LINK_STATUS, NULL, 0, NULL, 0) == NETMAN_NETIF_ETH_LINK_STATE_UP);
}

static int ethWaitValidNetIFLinkState(void)
{
	return WaitValidNetState(&ethGetNetIFLinkStatus);
}

static int ethApplyIPConfig(int use_dhcp, const struct ip4_addr* ip, const struct ip4_addr* netmask, const struct ip4_addr* gateway, const struct ip4_addr* dns)
{
	t_ip_info ip_info;
	int result;

	//SMAP is registered as the "sm0" device to the TCP/IP stack.
	if ((result = ps2ip_getconfig("sm0", &ip_info)) >= 0)
	{
		const ip_addr_t* dns_curr;

		//Obtain the current DNS server settings.
		dns_curr = dns_getserver(0);

		//Check if it's the same. Otherwise, apply the new configuration.
		if ((use_dhcp != ip_info.dhcp_enabled) || (!use_dhcp &&
		                                           (!ip_addr_cmp(ip, (struct ip4_addr*)&ip_info.ipaddr) ||
		                                            !ip_addr_cmp(netmask, (struct ip4_addr*)&ip_info.netmask) ||
		                                            !ip_addr_cmp(gateway, (struct ip4_addr*)&ip_info.gw) ||
		                                            !ip_addr_cmp(dns, dns_curr))))
		{
			if (use_dhcp)
			{
				ip_info.dhcp_enabled = 1;
			}
			else
			{ //Copy over new settings if DHCP is not used.
				ip_addr_set((struct ip4_addr*)&ip_info.ipaddr, ip);
				ip_addr_set((struct ip4_addr*)&ip_info.netmask, netmask);
				ip_addr_set((struct ip4_addr*)&ip_info.gw, gateway);

				ip_info.dhcp_enabled = 0;
			}

			//Update settings.
			result = ps2ip_setconfig(&ip_info);
			if (!use_dhcp)
				dns_setserver(0, dns);
		}
		else
			result = 0;
	}

	return result;
}

static void ethPrintIPConfig(void)
{
	t_ip_info ip_info;
	u8 ip_address[4], netmask[4], gateway[4], dns[4];

	//SMAP is registered as the "sm0" device to the TCP/IP stack.
	if (ps2ip_getconfig("sm0", &ip_info) >= 0)
	{
		const ip_addr_t* dns_curr;

		//Obtain the current DNS server settings.
		dns_curr = dns_getserver(0);

		ip_address[0] = ip4_addr1((struct ip4_addr*)&ip_info.ipaddr);
		ip_address[1] = ip4_addr2((struct ip4_addr*)&ip_info.ipaddr);
		ip_address[2] = ip4_addr3((struct ip4_addr*)&ip_info.ipaddr);
		ip_address[3] = ip4_addr4((struct ip4_addr*)&ip_info.ipaddr);

		netmask[0] = ip4_addr1((struct ip4_addr*)&ip_info.netmask);
		netmask[1] = ip4_addr2((struct ip4_addr*)&ip_info.netmask);
		netmask[2] = ip4_addr3((struct ip4_addr*)&ip_info.netmask);
		netmask[3] = ip4_addr4((struct ip4_addr*)&ip_info.netmask);

		gateway[0] = ip4_addr1((struct ip4_addr*)&ip_info.gw);
		gateway[1] = ip4_addr2((struct ip4_addr*)&ip_info.gw);
		gateway[2] = ip4_addr3((struct ip4_addr*)&ip_info.gw);
		gateway[3] = ip4_addr4((struct ip4_addr*)&ip_info.gw);

		dns[0] = ip4_addr1(dns_curr);
		dns[1] = ip4_addr2(dns_curr);
		dns[2] = ip4_addr3(dns_curr);
		dns[3] = ip4_addr4(dns_curr);

		printf("IP:\t%d.%d.%d.%d\n"
		       "NM:\t%d.%d.%d.%d\n"
		       "GW:\t%d.%d.%d.%d\n"
		       "DNS:\t%d.%d.%d.%d\n",
		       ip_address[0], ip_address[1], ip_address[2], ip_address[3],
		       netmask[0], netmask[1], netmask[2], netmask[3],
		       gateway[0], gateway[1], gateway[2], gateway[3],
		       dns[0], dns[1], dns[2], dns[3]);
	}
	else
	{
		printf("Unable to read IP address.\n");
	}
}

static void ethPrintLinkStatus(void)
{
	int mode, baseMode;

	//SMAP is registered as the "sm0" device to the TCP/IP stack.
	printf("Link:\t");
	if (NetManIoctl(NETMAN_NETIF_IOCTL_GET_LINK_STATUS, NULL, 0, NULL, 0) == NETMAN_NETIF_ETH_LINK_STATE_UP)
		printf("Up\n");
	else
		printf("Down\n");

	printf("Mode:\t");
	mode = NetManIoctl(NETMAN_NETIF_IOCTL_ETH_GET_LINK_MODE, NULL, 0, NULL, 0);

	//NETMAN_NETIF_ETH_LINK_MODE_PAUSE is a flag, so file it off first.
	baseMode = mode & (~NETMAN_NETIF_ETH_LINK_DISABLE_PAUSE);
	switch (baseMode)
	{
		case NETMAN_NETIF_ETH_LINK_MODE_10M_HDX:
			printf("10M HDX");
			break;
		case NETMAN_NETIF_ETH_LINK_MODE_10M_FDX:
			printf("10M FDX");
			break;
		case NETMAN_NETIF_ETH_LINK_MODE_100M_HDX:
			printf("100M HDX");
			break;
		case NETMAN_NETIF_ETH_LINK_MODE_100M_FDX:
			printf("100M FDX");
			break;
		default:
			printf("Unknown");
	}
	if (!(mode & NETMAN_NETIF_ETH_LINK_DISABLE_PAUSE))
		printf(" with ");
	else
		printf(" without ");
	printf("Flow Control\n");
}

static int ethGetDHCPStatus(void)
{
	t_ip_info ip_info;
	int result;

	if ((result = ps2ip_getconfig("sm0", &ip_info)) >= 0)
	{ //Check for a successful state if DHCP is enabled.
		if (ip_info.dhcp_enabled)
			result = (ip_info.dhcp_status == DHCP_STATE_BOUND || (ip_info.dhcp_status == DHCP_STATE_OFF));
		else
			result = -1;
	}

	return result;
}

static int ethWaitValidDHCPState(void)
{
	return WaitValidNetState(&ethGetDHCPStatus);
}

void Net::init()
{
	struct ip4_addr IP, NM, GW, DNS;
	int EthernetLinkMode;

	// //Reboot IOP
	// SifInitRpc(0);
	// while (!SifIopReset("", 0)) {};
	// while (!SifIopSync()) {};

	// //Initialize SIF services
	// SifInitRpc(0);
	// SifLoadFileInit();
	// SifInitIopHeap();
	// //sbv_patch_enable_lmb();

	{
		int ret = SifLoadModule("DEV9.IRX"_p.c_str(), 0, nullptr);
		printf("ret: %d\n", ret);
		check(ret >= 0);
	}

	{
		int ret = SifLoadModule("NETMAN.IRX"_p.c_str(), 0, nullptr);
		printf("ret: %d\n", ret);
		check(ret >= 0);
	}

	{
		int ret = SifLoadModule("SMAP.IRX"_p.c_str(), 0, nullptr);
		printf("ret: %d\n", ret);
		check(ret >= 0);
	}

	//Initialize NETMAN
	NetManInit();

	//The network interface link mode/duplex can be set.
	EthernetLinkMode = NETMAN_NETIF_ETH_LINK_MODE_AUTO;

	//Attempt to apply the new link setting.
	if (ethApplyNetIFConfig(EthernetLinkMode) != 0)
	{
		printf("Error: failed to set link mode.\n");
		check(false);
		return;
	}

	//Initialize IP address.
	IP4_ADDR(&IP, 10, 0, 6, 51);
	IP4_ADDR(&NM, 255, 255, 0, 0);
	IP4_ADDR(&GW, 10, 0, 1, 1);
	//DNS is not required if the DNS service is not used, but this demo will show how it is done.
	IP4_ADDR(&DNS, 10, 0, 1, 1);


	//Initialize IP address.
	//In this example, DHCP is enabled, hence the IP, NM, GW and DNS fields are cleared to 0..
	// ip4_addr_set_zero(&IP);
	// ip4_addr_set_zero(&NM);
	// ip4_addr_set_zero(&GW);
	// ip4_addr_set_zero(&DNS);

	//Initialize the TCP/IP protocol stack.
	ps2ipInit(&IP, &NM, &GW);

	//Enable DHCP
	ethApplyIPConfig(0, &IP, &NM, &GW, &DNS);

	//Wait for the link to become ready.
	printf("Waiting for connection...\n");
	if (ethWaitValidNetIFLinkState() != 0)
	{
		printf("Error: failed to get valid link status.\n");
		check(false);
		return;
	}

	// printf("Waiting for DHCP lease...");
	// //Wait for DHCP to initialize, if DHCP is enabled.
	// if (ethWaitValidDHCPState() != 0)
	// {
	// 	printf("DHCP failed\n.");
	// 	check(false);
	// 	return;
	// }

	printf("Initialized:\n");
	ethPrintLinkStatus();
	ethPrintIPConfig();

	//At this point, network support has been initialized and the PS2 can be pinged.
	//SleepThread();

	for (int i = 0; i < 10; ++i)
	{
		send_shit();
	}

	// end:
	// 	//To cleanup, just call these functions.
	// 	ps2ipDeinit();
	// 	NetManDeinit();

	// 	//Deinitialize SIF services
	// 	SifExitRpc();
}