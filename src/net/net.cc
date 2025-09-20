#include "net/net.hpp"

#include <cstdlib>
#include "engine.hpp"
extern "C" {
#include <netman.h>
#include <ps2ip.h>
#include <sifrpc.h>
}

namespace Net
{
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

void init()
{
	struct ip4_addr IP, NM, GW, DNS;

	Engine::sif_load_module("ps2dev9.irx");
	Engine::sif_load_module("netman.irx");
	Engine::sif_load_module("smap.irx");

	NetManInit();

	if (ethApplyNetIFConfig(NETMAN_NETIF_ETH_LINK_MODE_AUTO) != 0)
	{
		std::exit(-1);
	}
}
} // namespace Net