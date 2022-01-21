#include "rover_lua.hpp"

#include <vector>
#include <shared_mutex>
#include <mutex>
#include <algorithm>

typedef std::pair<lua_State*, void*> map_entry;

// Map using a vector for contiguous blocks
// N should remain small, and insertion/deletion will be rare
static std::vector<map_entry> mapping;

static std::shared_mutex map_rw;

static bool comp(const map_entry& i, const map_entry& j) {
	return i.first < j.first;
}


void rover_lua::set_custom_ptr(lua_State* L, void* ptr) {
	map_entry new_pair(L, ptr);

	std::unique_lock writer(map_rw);

	// If this lua_State already has a custom ptr, update that one.
	// Do not create a duplicate entry
	auto entry = std::lower_bound(mapping.begin(), mapping.end(), new_pair, comp);
	if (entry != mapping.end() && entry->first == L) {
		entry->second = ptr;
	} else {
		// Not found; Create a new entry
		mapping.push_back(new_pair);
		std::sort(mapping.begin(), mapping.end(), comp);
	}

}

void* rover_lua::get_custom_ptr(lua_State* L) {
	map_entry search_pair(L, nullptr);
	std::shared_lock reader(map_rw);

	auto entry = std::lower_bound(mapping.begin(), mapping.end(), search_pair, comp);
	if (entry != mapping.end() && entry->first == L) {
		return entry->second;
	} else {
		return nullptr;
	}

}

void rover_lua::del_custom_ptr(lua_State* L) {
	map_entry search_pair(L, nullptr);

	std::unique_lock writer(map_rw);

	// If this lua_State already has a custom ptr, update that one.
	// Do not create a duplicate entry
	auto entry = std::lower_bound(mapping.begin(), mapping.end(), search_pair, comp);
	if (entry != mapping.end() && entry->first == L) {
		mapping.erase(entry);
		std::sort(mapping.begin(), mapping.end(), comp);
	}

}
