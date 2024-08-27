#include "texture/texture.hpp"

#include "egg/texture_header.hpp"

#include <json/json.h>
#include <fstream>
#include <filesystem>
#include <unordered_set>
#include <algorithm>
#include "utils.hpp"
#include "app.hpp"
#include "types.hpp"

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
static_assert(sizeof(PalleteColor) == 4);

union PalleteColor16
{
	struct
	{
		uint16_t red : 5;
		uint16_t green : 5;
		uint16_t blue : 5;
		uint16_t alpha : 1;
	};
	uint16_t color;
};
static_assert(sizeof(PalleteColor16) == 2);

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

static bool writePalette()
{
	return true;
}

// Collects the palette into palette_colors, sets the PSM and CLUT on texture_header
static bool collectPalette(const Magick::Image& image, std::vector<uint32_t>& palette_colors, TextureFileHeader& texture_header, bool alpha)
{
	using namespace Magick;
	std::unordered_set<PalleteColor> colors(256);

	for (size_t x = 0; x < image.columns(); ++x)
	{
		for (size_t y = 0; y < image.rows(); ++y)
		{
			// The pixel read doesn't work without this line. I don't understand why
			const void* pixel = image.getConstPixels(x, y, 1, 1);

			ColorRGB c = image.pixelColor(x, y);

			PalleteColor new_color;
			new_color.alpha = alpha ? c.alpha() * 255 : 255;
			new_color.red   = c.red() * 255;
			new_color.green = c.green() * 255;
			new_color.blue  = c.blue() * 255;

			colors.insert(new_color);
		}
	}

	print("Num colors in image: %lu", colors.size());
	if (colors.size() <= 16)
	{
		print("16 or less colors, choosing 4-bit texture palette");
		texture_header.psm = GS_PSM_4;
		palette_colors.resize(16);
	}
	else if (colors.size() <= 256)
	{
		print("More than 16 colors, choosing 8-bit texture palette");
		texture_header.psm = GS_PSM_8;
		palette_colors.resize(256);
	}
	else
	{
		print_error("Failed to palletize texture! More than 256 colors! "
		            "Consider changing to non-palletized");
		return false;
	}

	// Write the palette
	{
		size_t i = 0;
		for (const PalleteColor& c : colors)
		{
			palette_colors[i] = c.color;
			i++;
		}

		texture_header.clut.set(palette_colors);
	}

	return true;
}

static bool writePalletizedImageData(const Magick::Image& image, const std::vector<uint32_t>& palette_colors, std::vector<std::byte>& image_data, bool alpha)
{
	using namespace Magick;

	assert(palette_colors.size() == 16 || palette_colors.size() == 256);

	const uint32_t num_pixels = image.columns() * image.rows();
	if (palette_colors.size() == 16)
	{
		image_data.resize(num_pixels / 2 + (num_pixels % 2));
	}
	else
	{
		image_data.resize(num_pixels);
	}

	// Write the data
	int i = 0;
	for (size_t x = 0; x < image.columns(); ++x)
	{
		for (size_t y = 0; y < image.rows(); ++y)
		{
			// The pixel read doesn't work without this line. I don't understand why
			const void* pixel = image.getConstPixels(x, y, 1, 1);

			ColorRGB c = image.pixelColor(x, y);

			PalleteColor new_color;
			new_color.alpha = alpha ? c.alpha() * 255 : 255;
			new_color.red   = c.red() * 255;
			new_color.green = c.green() * 255;
			new_color.blue  = c.blue() * 255;

			ptrdiff_t new_index = std::find(palette_colors.begin(), palette_colors.end(), new_color.color) - palette_colors.begin();
			assert(new_index < palette_colors.size());

			switch (palette_colors.size())
			{
				case 16: {
					assert(new_index < 16);

					std::byte byte_index = (std::byte)new_index;
					if (i % 2 == 1)
					{
						byte_index <<= 4;
					}

					image_data[i / 2] |= byte_index;
				}
				break;

				case 256: {
					assert(new_index < 256);

					image_data[i] = (std::byte)new_index;
				}
				break;

				default:
					break;
			}

			i++;
		}
	}

	assert(i == num_pixels);

	return true;
}

static std::vector<std::byte> serialize_texture(const TextureFileHeader& texture_header)
{
	std::vector<std::byte> out_data;
	Serializer texture_serializer(out_data);
	serialize(texture_serializer, texture_header);
	texture_serializer.finish_serialization();
	return out_data;
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
	texture_header.size_x     = my_image.columns();
	texture_header.size_y     = my_image.rows();
	texture_header.function   = TEXTURE_FUNCTION_DECAL;
	texture_header.components = obj["alpha"].asBool() ? TEXTURE_COMPONENTS_RGBA : TEXTURE_COMPONENTS_RGB;

	const std::string color_type = obj["psm"].asString();

	print("Texture size: x: %u y: %u", texture_header.size_x, texture_header.size_y);

	if (iequals(color_type, "palette"))
	{
		print("Texture is palettized, collecting palette");

		std::vector<uint32_t> palette_colors;
		if (!collectPalette(my_image, palette_colors, texture_header, obj["alpha"].asBool()))
		{
			print_error("Failed to collect color palette!");
			return false;
		}

		std::vector<std::byte> image_data;
		if (!writePalletizedImageData(my_image, palette_colors, image_data, obj["alpha"].asBool()))
		{
			print_error("Failed to write image data!");
			return false;
		}

		texture_header.data.set(image_data);

		print("Bytes in palletized image: %lu", image_data.size());
		print("Pixels in palletized image: %lu", my_image.columns() * my_image.rows());

		out_data = serialize_texture(texture_header);
		return true;
	}
	else if (iequals(color_type, "32") || iequals(color_type, "24"))
	{
		texture_header.psm = iequals(color_type, "32") ? GS_PSM_32 : GS_PSM_24;

		std::vector<std::byte> image_data;
		image_data.resize(texture_header.size_x * texture_header.size_y * 4);

		uint32_t i = 0;
		for (size_t x = 0; x < my_image.columns(); ++x)
		{
			for (size_t y = 0; y < my_image.rows(); ++y)
			{
				// The pixel read doesn't work without this line. I don't understand why
				const void* pixel = my_image.getConstPixels(x, y, 1, 1);

				ColorRGB c = my_image.pixelColor(x, y);

				PalleteColor* new_color = (PalleteColor*)image_data.data();
				new_color[i].alpha      = obj["alpha"].asBool() ? c.alpha() * 255 : 255;
				new_color[i].red        = c.red() * 255;
				new_color[i].green      = c.green() * 255;
				new_color[i].blue       = c.blue() * 255;

				i++;
			}
		}
		assert(i == (image_data.size() / 4));

		texture_header.data.set(image_data);

		out_data = serialize_texture(texture_header);
		return true;
	}
	else if (iequals(color_type, "16"))
	{
		texture_header.psm = GS_PSM_16;

		std::vector<std::byte> image_data;
		image_data.resize(texture_header.size_x * texture_header.size_y * 2);

		uint32_t i = 0;
		for (size_t x = 0; x < my_image.columns(); ++x)
		{
			for (size_t y = 0; y < my_image.rows(); ++y)
			{
				// The pixel read doesn't work without this line. I don't understand why
				const void* pixel = my_image.getConstPixels(x, y, 1, 1);

				ColorRGB c = my_image.pixelColor(x, y);

				PalleteColor16* new_color = (PalleteColor16*)image_data.data();
				new_color[i].alpha        = obj["alpha"].asBool() ? c.alpha() >= 0.5 : 1;
				new_color[i].red          = c.red() * 31;
				new_color[i].green        = c.green() * 31;
				new_color[i].blue         = c.blue() * 31;

				i++;
			}
		}
		assert(i == (image_data.size() / 2));

		texture_header.data.set(image_data);

		out_data = serialize_texture(texture_header);
		return true;
	}
	else
	{
		print_error("Unrecognized color type!: \"%s\"", color_type.c_str());
		return false;
	}
}