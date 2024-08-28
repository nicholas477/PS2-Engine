#pragma once

#include <tamtypes.h>
#include <graph_vram.h>

namespace egg::ps2::graphics::gs_mem
{
static constexpr u32 num_vram_slots = 10;

// Called during init() to allocate the framebuffers, returns -1 if there isn't
// enough memory. Don't call this yourself
int allocate_framebuffer(int width, int height, int psm, int alignment);

// Tells
void finish_allocating_framebuffers();

// Allocates a texture slot on in GS memory, returns -1 if there isn't enough
// memory
//
// Each page is 8k (or 2k words), so the total memory size in bytes is pages * 8192
int allocate_texture_slot(u32 size_pages);

// Returns an address of an empty texture slot, or -1 if it couldn't find one
// Locks the corresponding texture slot
int get_empty_texture_slot(u32 size_pages);

// Unlocks a texture slot so it can be used by other textures
//
// Returns true if it found the slot with the corresponding address
bool unlock_texture_slot(u32 address);

// Returns the total amount of memory available to be allocated.
// Defaults to 1,048,576 words or 4MB
//
// This can be increased with set_max_vram_words().
u32 get_max_vram_words();

// Sets the limit of allocatable vram.
//
// Don't change this on the real console!
void set_max_vram_words(u32 new_max);

// Returns the pointer pointing to the next unallocated word in vram
//
// This is the current amount of vram allocated (in words)
u32 get_current_vram_pointer();

// Returns how much memory is unallocated (in pages)
static u32 get_unallocated_page_num()
{
	return (get_max_vram_words() - get_current_vram_pointer()) / 2048;
}

// Resets all texture slots (except the framebuffers)
void reset_vram_slots();

void print_vram_slots();

}; // namespace egg::ps2::graphics::gs_mem