#include "utils/rendering.hpp"

#include "draw.h"

#include <dma_tags.h>
#include <gif_tags.h>
#include <gs_psm.h>

#include <malloc.h>
#include <stdio.h>

#include <font.h>
#include <dma.h>

void idk()
{
	dma_channel_send_chain(0, 0, 0, 0, 0);
	dma_channel_send_normal(0, 0, 0, 0, 0);
	dma_channel_wait(0, 0);
}