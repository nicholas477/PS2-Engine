#include "texture/texture.hpp"

#include "egg/texture_header.hpp"

#include <json/json.h>
#include <fstream>
#include <filesystem>
#include <unordered_set>
#include <algorithm>
#include "utils.hpp"
#include "app.hpp"

#define MAGICKCORE_QUANTUM_DEPTH 8
#define MAGICKCORE_HDRI_ENABLE false

#include <Magick++.h>

// Copied from gs_psm.h

// Pixel-Storage Methods
/** 32 bits per pixel. */
#define GS_PSM_32 0x00
/** 24 bits per pixel. */
#define GS_PSM_24 0x01
/** 16 bits per pixel. */
#define GS_PSM_16 0x02
/** 16 bits per pixel. */
#define GS_PSM_16S 0x0A
/** 24 bits per pixel. */
#define GS_PSM_PS24 0x12
/** 8 bits per pixel, palettized. */
#define GS_PSM_8 0x13
/** 4 bits per pixel, palettized. */
#define GS_PSM_4 0x14
/** 8 bits per pixel, 24 to 32 */
#define GS_PSM_8H 0x1B
/** 4 bits per pixel, 28 to 32 */
#define GS_PSM_4HL 0x24
/** 4 bits per pixel, 24 to 27 */
#define GS_PSM_4HH 0x2C
/** 32 bits per pixel. */
#define GS_PSMZ_32 0x30
/** 24 bits per pixel. */
#define GS_PSMZ_24 0x31
/** 16 bits per pixel. */
#define GS_PSMZ_16 0x32
/** 16 bits per pixel. */
#define GS_PSMZ_16S 0x3A

/** Texture Color Components */
#define TEXTURE_COMPONENTS_RGB 0
#define TEXTURE_COMPONENTS_RGBA 1

/** Texture Function */
#define TEXTURE_FUNCTION_MODULATE 0
#define TEXTURE_FUNCTION_DECAL 1
#define TEXTURE_FUNCTION_HIGHLIGHT 2
#define TEXTURE_FUNCTION_HIGHLIGHT2 3

static std::filesystem::path get_file_path(std::string_view json_path, const Json::Value& obj)
{
	std::string input_file_path = obj["file"].asString();

	return std::filesystem::path(json_path).parent_path() / input_file_path;
}

union PalleteColor
{
	struct
	{
		uint8_t red;
		uint8_t green;
		uint8_t blue;
		uint8_t alpha;
	};
	uint32_t color;
};

template <>
struct std::hash<PalleteColor>
{
	std::size_t operator()(const PalleteColor& k) const
	{
		return k.color;
	}
};

static bool operator==(const PalleteColor& lhs, const PalleteColor& rhs)
{
	return lhs.color == rhs.color;
}

bool parseTexture(std::string_view path, const Json::Value& obj, std::vector<std::byte>& out_data)
{
	using namespace Magick;

	print("Opening texture!");

	std::filesystem::path texture_file_path = get_file_path(path, obj);

	print("Texture file path: %s", texture_file_path.c_str());

	Magick::Image my_image;
	my_image.read(texture_file_path);

	TextureFileHeader texture_header;
	texture_header.size_x   = my_image.columns();
	texture_header.size_y   = my_image.rows();
	texture_header.psm      = GS_PSM_32;
	texture_header.function = TEXTURE_FUNCTION_DECAL;

	print("x: %u y: %u", texture_header.size_x, texture_header.size_y);

	std::unordered_set<PalleteColor> colors;
	for (size_t x = 0; x < my_image.columns(); ++x)
	{
		for (size_t y = 0; y < my_image.rows(); ++y)
		{
			// The pixel read doesn't work without this line. I don't understand why
			const void* pixel = my_image.getConstPixels(x, y, 1, 1);

			ColorRGB c = my_image.pixelColor(x, y);

			PalleteColor new_color;
			new_color.alpha = c.alpha() * 255;
			new_color.red   = c.red() * 255;
			new_color.green = c.green() * 255;
			new_color.blue  = c.blue() * 255;

			colors.insert(new_color);
		}
	}

	print("Num colors in image: %lu", colors.size());

	for (const PalleteColor& c : colors)
	{
		print("r: %u, g: %u, b: %u, a: %u", c.red, c.green, c.blue, c.alpha);
	}

	return true;
}