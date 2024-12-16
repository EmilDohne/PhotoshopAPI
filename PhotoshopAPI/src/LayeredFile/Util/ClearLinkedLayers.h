#pragma once

#include "Macros.h"
#include "Util/Profiling/Perf/Instrumentor.h"
#include "LayeredFile/LayeredFile.h"
#include "LayeredFile/LayerTypes/SmartObjectLayer.h"
#include "LayeredFile/LinkedData/LinkedLayerData.h"

#include <memory>
#include <set>

PSAPI_NAMESPACE_BEGIN

/// Remove all unused linked layers stored on the file by checking whether their hashes are known to us at 
template <typename T>
void clear_unused_linked_layers(LayeredFile<T>& file)
{
	PSAPI_PROFILE_FUNCTION();
	auto& linked_layers = file.linked_layers();
	std::set<std::string> all_hashes = linked_layers.hashes();

	std::set<std::string> hashes_in_file;
	for (const auto& layer_ptr : file.flat_layers())
	{
		if (auto smart_object_ptr = std::dynamic_pointer_cast<SmartObjectLayer<T>>(layer_ptr))
		{
			hashes_in_file.insert(smart_object_ptr->hash());
		}
	}

	// Find hashes in `all_hashes` but not in `hashes_in_file`.
	std::set<std::string> hashes_to_remove;
	std::set_difference(
		all_hashes.begin(), all_hashes.end(),
		hashes_in_file.begin(), hashes_in_file.end(),
		std::inserter(hashes_to_remove, hashes_to_remove.end())
	);

	// Erase the no longer referenced hashes from the linked layers
	for (const auto& hash : hashes_to_remove)
	{
		linked_layers.erase(hash);
	}
}

PSAPI_NAMESPACE_END