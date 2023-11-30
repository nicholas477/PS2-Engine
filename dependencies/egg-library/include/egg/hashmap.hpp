#pragma once

#include <array>
#include <cstdint>
#include <cstddef>

#include "egg/assert.hpp"

// Key is not actually stored, just the hashes of the keys
template <size_t size, typename keyT, typename valueT>
class HashMap
{
public:
	std::array<uint32_t, size> keys;
	std::array<valueT, size> values;

	// Returns nullptr if the value doesn't exist
	valueT* lookup(const keyT& key)
	{
		uint32_t hash = (uint32_t)std::hash<keyT>()(key);
		check(hash != 0);

		int32_t index = lookup_by_hash(hash);
		if (index != -1)
		{
			return &values[index];
		}

		return nullptr;
	}

	// Returns nullptr if the value doesn't exist
	const valueT* lookup(const keyT& key) const
	{
		uint32_t hash = (uint32_t)std::hash<keyT>()(key);
		check(hash != 0);

		int32_t index = lookup_by_hash(hash);
		if (index != -1)
		{
			return &values[index];
		}

		return nullptr;
	}

	// Returns the index of the key/value by hash, or -1 if not found
	int32_t lookup_by_hash(const uint32_t hash) const
	{
		uint32_t index = hash % size;
		if (keys[index] == hash)
		{
			return index;
		}
		else
		{
			// Use linear probing to find the value
			index++;
			while ((index % size) != hash)
			{
				if (keys[index] == hash)
				{
					return index;
				}
				index++;
			}
		}

		return -1;
	}

	// Returns the first empty index for this hash
	int32_t find_next_empty_index(const uint32_t hash) const
	{
		const uint32_t start_index = hash % size;
		uint32_t index             = start_index;
		if (keys[index] == 0)
		{
			return index;
		}
		else
		{
			// Use linear probing to find the next empty key
			index++;
			while ((index % size) != start_index)
			{
				if (keys[index] == 0)
				{
					return index;
				}
				index++;
			}
		}

		return -1;
	}

	valueT& operator[](const keyT& key)
	{
		valueT* value = lookup(key);
		check(value != nullptr);
		return *value;
	}

	const valueT& operator[](const keyT& key) const
	{
		const valueT* value = lookup(key);
		check(value != nullptr);
		return *value;
	}

	bool add_key_value(const keyT& key, const valueT& value)
	{
		uint32_t hash = (uint32_t)std::hash<keyT>()(key);
		check(hash != 0);

		int32_t empty_index = find_next_empty_index(hash);
		if (empty_index != -1)
		{
			keys[empty_index]   = hash;
			values[empty_index] = value;

			return true;
		}

		return false;
	}
};