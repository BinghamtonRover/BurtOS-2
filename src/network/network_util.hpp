#pragma once

#include <vector>
#include <cstdint>
#include <mutex>

namespace Network {

template<typename T>
struct DoubleBuffer {
	std::mutex buffer_swap_lock;
	std::vector<T> buf[2];
	std::size_t usage[2] = {0, 0};
	uint8_t active = 0;

	void swap() {
		buffer_swap_lock.lock();
		active = !active;
		buffer_swap_lock.unlock();
	}
	std::vector<T>& get_active_buffer() {
		return buf[active];
	}
	std::vector<T>& get_locked_buffer() {
		return buf[!active];
	}

	void push_back(const T& t) {
		// swap and push_back cannot happen at the same time or the wrong buffer may be modified
		buffer_swap_lock.lock();
		if (usage[active] + 1 >= buf[active].size()) {
			buf[active].resize(usage[active] + 1);
		}
		buf[usage[active]++] = t;
		buffer_swap_lock.unlock();
	}
};

}
