#pragma once

#include "offset_pointer.hpp"
#include "serialization.hpp"
#include "math_types.hpp"
#include "egg/asset.hpp"
#include <cstdint>

#pragma pack(push, 1)
struct LevelMeshes
{
	OffsetArray<Matrix> mesh_transforms;
	OffsetArray<Asset::Reference> mesh_files;
};

static void collect_references(std::unordered_set<Asset::Reference>& references, const LevelMeshes& meshes)
{
	collect_references(references, meshes.mesh_files);
}

static size_t serialize(Serializer& serializer, const LevelMeshes& meshes, size_t alignment)
{
	size_t base = serialize(serializer, meshes.mesh_transforms, alignment);
	serialize(serializer, meshes.mesh_files, alignment);

	return base;
}

struct LevelFileHeader
{
	LevelMeshes meshes;
};

static void collect_references(std::unordered_set<Asset::Reference>& references, const LevelFileHeader& level)
{
	collect_references(references, level.meshes);
}

static size_t serialize(Serializer& serializer, const LevelFileHeader& level, size_t alignment)
{
	return serialize(serializer, level.meshes, alignment);
}


#pragma pack(pop)