/*
	Support for one-way data streams using UDP. Intended for video streaming
	
	Expects that stream data can be broken into frames
*/

#pragma once

#include <cstdint>

namespace net {

class StreamSender {
public:
	void send_frame(unsigned stream, void* data, std::size_t len);
};

class StreamReceiver {

};

} // end namespace net