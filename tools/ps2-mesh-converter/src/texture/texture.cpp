#include "texture/texture.hpp"

#include "egg/texture_header.hpp"

#include "app.hpp"
#include <json/json.h>
#include <fstream>
#include <filesystem>
#include <unordered_set>
#include <algorithm>
#include "utils.hpp"
#include "app.hpp"
#include "types.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

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

/** Texture wrapping */
#define WRAP_REPEAT 0
#define WRAP_CLAMP 1
#define WRAP_REGION_CLAMP 2
#define WRAP_REGION_REPEAT 3

struct image
{
	int width, height, channels;
	unsigned char* data;
};

static std::filesystem::path get_file_path(std::string_view json_path, const Json::Value& obj)
{
	std::string input_file_path = obj["file"].asString();

	return std::filesystem::path(json_path).parent_path() / input_file_path;
}

union Color
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
static_assert(sizeof(Color) == 4);

struct Color24
{
	uint8_t red;
	uint8_t green;
	uint8_t blue;
};
static_assert(sizeof(Color24) == 3);

static Color fromColorRGB(const image& img, size_t x, size_t y, bool alpha)
{
	int i = x + y * img.width;

	Color new_color;
	if (img.channels == 1)
	{
		new_color.alpha = 1;
		new_color.red   = img.data[i] / 8;
		new_color.green = img.data[i] / 8;
		new_color.blue  = img.data[i] / 8;
	}
	else if (img.channels == 3)
	{
		const Color24* colors = (const Color24*)img.data;

		new_color.alpha = 255;
		new_color.red   = colors[i].red;
		new_color.green = colors[i].green;
		new_color.blue  = colors[i].blue;
	}
	else if (img.channels == 4)
	{
		const Color* colors = (const Color*)img.data;

		new_color.alpha = alpha ? colors[i].alpha : 255;
		new_color.red   = colors[i].red;
		new_color.green = colors[i].green;
		new_color.blue  = colors[i].blue;
	}
	else
	{
		assert(false);
	}
	return new_color;
}

union Color16
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
static_assert(sizeof(Color16) == 2);

static Color16 fromColorRGB16(const image& img, size_t x, size_t y, bool alpha)
{
	int i = x + y * img.width;

	Color16 new_color;
	if (img.channels == 1)
	{
		new_color.alpha = 1;
		new_color.red   = img.data[i] / 8;
		new_color.green = img.data[i] / 8;
		new_color.blue  = img.data[i] / 8;
	}
	else if (img.channels == 3)
	{
		const Color24* colors = (const Color24*)img.data;

		new_color.alpha = 1;
		new_color.red   = colors[i].red / 8;
		new_color.green = colors[i].green / 8;
		new_color.blue  = colors[i].blue / 8;
	}
	else if (img.channels == 4)
	{
		const Color* colors = (const Color*)img.data;

		new_color.alpha = alpha ? colors[i].alpha > 128 : 1;
		new_color.red   = colors[i].red / 8;
		new_color.green = colors[i].green / 8;
		new_color.blue  = colors[i].blue / 8;
	}
	else
	{
		assert(false);
	}
	return new_color;
}

template <>
struct std::hash<Color>
{
	std::size_t operator()(const Color& k) const
	{
		return k.color;
	}
};

static bool operator==(const Color& lhs, const Color& rhs)
{
	return lhs.color == rhs.color;
}

// Collects the palette into palette_colors, sets the PSM and CLUT on texture_header
static bool collectPalette(const image& image, std::vector<uint32_t>& palette_colors, TextureFileHeader& texture_header, bool alpha)
{
	std::unordered_set<::Color> colors(256);

	for (size_t x = 0; x < image.width; ++x)
	{
		for (size_t y = 0; y < image.height; ++y)
		{
			::Color new_color = fromColorRGB(image, x, y, alpha);
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
		for (const Color& c : colors)
		{
			palette_colors[i] = c.color;
			print("Palette color: r: %u, g: %u, b: %u, a: %u", c.red, c.green, c.blue, c.alpha);

			i++;
		}

		texture_header.clut.set(palette_colors);
	}

	return true;
}

static bool writePalletizedImageData(const image& image, const std::vector<uint32_t>& palette_colors, std::vector<std::byte>& image_data, bool alpha)
{
	assert(palette_colors.size() == 16 || palette_colors.size() == 256);

	const uint32_t num_pixels = image.width * image.height;
	if (palette_colors.size() == 16)
	{
		image_data.resize((num_pixels / 2) + (num_pixels % 2));
	}
	else
	{
		image_data.resize(num_pixels);
	}

	// Write the data
	int i = 0;
	for (size_t x = 0; x < image.width; ++x)
	{
		for (size_t y = 0; y < image.height; ++y)
		{
			Color new_color = fromColorRGB(image, x, y, alpha);

			ptrdiff_t new_index = std::find(palette_colors.begin(), palette_colors.end(), new_color.color) - palette_colors.begin();
			assert(new_index < palette_colors.size());
			assert(new_index >= 0);

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
					assert(false);
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
	assert(serialize(texture_serializer, texture_header) == 0);
	texture_serializer.finish_serialization();
	return out_data;
}

void checkTextureHeader(const TextureFileHeader& texture_header, const std::vector<std::byte>& data)
{
	const TextureFileHeader& new_texture_header = *(const TextureFileHeader*)data.data();
	assert(texture_header.size_x == new_texture_header.size_x);
	assert(texture_header.size_y == new_texture_header.size_y);
	assert(texture_header.psm == new_texture_header.psm);
	assert(texture_header.function == new_texture_header.function);
	assert(texture_header.components == new_texture_header.components);
	assert(texture_header.horizontal == new_texture_header.horizontal);
	assert(texture_header.vertical == new_texture_header.vertical);
	assert(texture_header.minu == new_texture_header.minu);
	assert(texture_header.maxu == new_texture_header.maxu);
	assert(texture_header.minv == new_texture_header.minv);
	assert(texture_header.maxv == new_texture_header.maxv);

	print("Texture header length: %lu", texture_header.data.length);
	assert(memcmp(texture_header.data.get_ptr(), new_texture_header.data.get_ptr(), texture_header.data.length) == 0);

	assert(memcmp(texture_header.clut.get_ptr(), new_texture_header.clut.get_ptr(), texture_header.clut.length) == 0);
}

bool parseTexture(std::string_view path, const Json::Value& obj, std::vector<std::byte>& out_data)
{
	print("Opening texture!");

	const std::filesystem::path texture_file_path = get_file_path(path, obj);

	print("Texture file path: %s", texture_file_path.c_str());

	image img;
	img.data = stbi_load(texture_file_path.c_str(), &img.width, &img.height, &img.channels, STBI_rgb);
	if (img.data == nullptr)
	{
		print_error("Failed to load image!");
		return false;
	}
	print("Num channels in image: %lu", img.channels);


	TextureFileHeader texture_header;
	texture_header.size_x     = img.width;
	texture_header.size_y     = img.height;
	texture_header.function   = TEXTURE_FUNCTION_DECAL;
	texture_header.components = TEXTURE_COMPONENTS_RGBA; //obj["alpha"].asBool() ? TEXTURE_COMPONENTS_RGBA : TEXTURE_COMPONENTS_RGB;

	print("Setting texture to repeat mapping");
	texture_header.horizontal = WRAP_REPEAT;
	texture_header.vertical   = WRAP_REPEAT;

	texture_header.minu = 0;
	texture_header.maxu = texture_header.size_x;
	texture_header.minv = 0;
	texture_header.maxv = texture_header.size_y;

	const std::string color_type = obj["psm"].asString();

	print("Texture size: x: %u y: %u", texture_header.size_x, texture_header.size_y);

	if (iequals(color_type, "palette"))
	{
		print("Texture is palettized, collecting palette");

		std::vector<uint32_t> palette_colors;
		if (!collectPalette(img, palette_colors, texture_header, obj["alpha"].asBool()))
		{
			print_error("Failed to collect color palette!");
			return false;
		}

		std::vector<std::byte> image_data;
		if (!writePalletizedImageData(img, palette_colors, image_data, obj["alpha"].asBool()))
		{
			print_error("Failed to write image data!");
			return false;
		}

		texture_header.data.set(image_data);

		print("Bytes in palletized image: %lu", image_data.size());
		print("Pages in palletized image: %lu", image_data.size() / 8192);
		print("Pixels in palletized image: %lu", img.width * img.height);

		out_data = serialize_texture(texture_header);
		checkTextureHeader(texture_header, out_data);
	}
	else if (iequals(color_type, "32")) // || iequals(color_type, "24"))
	{
		texture_header.psm = iequals(color_type, "32") ? GS_PSM_32 : GS_PSM_24;

		std::vector<std::byte> image_data;
		image_data.resize(texture_header.size_x * texture_header.size_y * 4);

		uint32_t i = 0;
		for (size_t x = 0; x < img.width; ++x)
		{
			for (size_t y = 0; y < img.height; ++y)
			{
				Color* palette_data = (Color*)image_data.data();

				palette_data[i] = fromColorRGB(img, x, y, obj["alpha"].asBool());

				i++;
			}
		}
		assert(i == (image_data.size() / 4));

		texture_header.data.set(image_data);

		out_data = serialize_texture(texture_header);
		checkTextureHeader(texture_header, out_data);
	}
	else if (iequals(color_type, "16"))
	{
		texture_header.psm = GS_PSM_16;

		std::vector<std::byte> image_data;
		image_data.resize(texture_header.size_x * texture_header.size_y * 2);

		uint32_t i = 0;
		for (size_t x = 0; x < img.width; ++x)
		{
			for (size_t y = 0; y < img.height; ++y)
			{
				Color16* new_color = (Color16*)image_data.data();

				new_color[i] = fromColorRGB16(img, x, y, obj["alpha"].asBool());

				i++;
			}
		}
		assert(i == (image_data.size() / 2));

		texture_header.data.set(image_data);

		out_data = serialize_texture(texture_header);
		checkTextureHeader(texture_header, out_data);
	}
	else
	{
		print_error("Unrecognized color type!: \"%s\"", color_type.c_str());
		print_error("Recognized color types are: \"palette\", \"32\", \"24\", \"16\"");
		return false;
	}

	return true;
}