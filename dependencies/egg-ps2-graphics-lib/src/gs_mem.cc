#include "egg-ps2-graphics-lib/gs_mem.hpp"

#include <stdio.h>
#include <gs_psm.h>
#include <array>
#include <algorithm>

namespace egg::ps2::graphics::gs_mem
{

struct vram_slot
{
	u32 address;
	u32 size_pages;
	bool is_locked;

	vram_slot()
	{
		address    = 0;
		size_pages = 0;
		is_locked  = false;
	}
};

static bool compare_slots(const vram_slot& lhs, const vram_slot& rhs)
{
	return (lhs.size_pages < rhs.size_pages) || (lhs.address < rhs.address);
}

static void print_slot(const vram_slot& slot)
{
	printf("------ Slot ------\n");
	printf("size in pages: %u\n", slot.size_pages);
	printf("address: %u\n", slot.address);
	printf("is locked: %s\n", slot.is_locked ? "true" : "false");
}

std::array<vram_slot, num_vram_slots> vram_slots;

static void sort_vram_slots()
{
	std::sort(vram_slots.begin(), vram_slots.end(), compare_slots);
}

static u32 graph_vram_pointer = 0;

// Maybe change this later for emulators?
static u32 graph_vram_max_words = GRAPH_VRAM_MAX_WORDS;

static u32 graph_vram_frambuffer_end_pointer = 0;

int allocate_framebuffer(int width, int height, int psm, int alignment)
{
	int size;

	// Calculate the size and increment the pointer
	size = graph_vram_size(width, height, psm, alignment);

	graph_vram_pointer += size;

	// If the pointer overflows the vram size
	if (graph_vram_pointer > graph_vram_max_words)
	{
		graph_vram_pointer -= size;
		return -1;
	}

	return graph_vram_pointer - size;
}

void finish_allocating_framebuffers()
{
	graph_vram_frambuffer_end_pointer = graph_vram_pointer;
}

int allocate_texture_slot(u32 pages)
{
	int size;

	// Calculate the size and increment the pointer
	size = graph_vram_size(64, 32 * pages, GS_PSM_32, GRAPH_ALIGN_BLOCK);

	graph_vram_pointer += size;

	// If the pointer overflows the vram size
	if (graph_vram_pointer > graph_vram_max_words)
	{

		graph_vram_pointer -= size;
		return -1;
	}

	const u32 address = graph_vram_pointer - size;

	bool found_slot = false;
	for (size_t i = 0; i < vram_slots.size(); ++i)
	{
		if (vram_slots[i].size_pages == 0)
		{
			vram_slots[i].address    = address;
			vram_slots[i].size_pages = pages;
			found_slot               = true;
			break;
		}
	}

	if (found_slot)
	{
		sort_vram_slots();
		return address;
	}
	else
	{
		graph_vram_pointer -= size;
		return -1;
	}
}

int get_empty_texture_slot(u32 size_pages)
{
	for (size_t i = 0; i < vram_slots.size(); ++i)
	{
		if (vram_slots[i].size_pages >= size_pages && vram_slots[i].is_locked == false)
		{
			vram_slots[i].is_locked = true;
			return vram_slots[i].address;
		}
	}
	return -1;
}

bool unlock_texture_slot(u32 address)
{
	for (size_t i = 0; i < vram_slots.size(); ++i)
	{
		if (vram_slots[i].address == address)
		{
			vram_slots[i].is_locked = false;
			return true;
		}
	}

	return false;
}

u32 get_max_vram_words()
{
	return graph_vram_max_words;
}

void set_max_vram_words(u32 new_max)
{
	graph_vram_max_words = new_max;
}

u32 get_current_vram_pointer()
{
	return graph_vram_pointer;
}

void reset_vram_slots()
{
	for (size_t i = 0; i < vram_slots.size(); ++i)
	{
		vram_slots[i] = vram_slot();
	}

	graph_vram_pointer = graph_vram_frambuffer_end_pointer;
}

void print_vram_slots()
{
	for (size_t i = 0; i < vram_slots.size(); ++i)
	{
		print_slot(vram_slots[i]);
	}
}

}; // namespace egg::ps2::graphics::gs_mem