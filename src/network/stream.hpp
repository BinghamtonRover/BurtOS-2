/*
	Support for one-way data streams using UDP. Intended for video streaming
	Frames and metadata are sent using vectored IO to reduce copies
*/

#pragma once

#include <cstdint>
#include <vector>
#include <mutex>
#include <shared_mutex>
#include <functional>
#include <stdexcept>
#include <boost/asio.hpp>
#include <boost/array.hpp>

namespace net {

// Stream object may be more useful in the future if we
// want to support named streams and camera UUID
struct StreamMetadata {
	uint8_t frame_index = 0;
};

class StreamSender {
public:
	StreamSender(boost::asio::io_context& io_context);
	void set_destination_endpoint(const boost::asio::ip::udp::endpoint& endpoint);
	void send_frame(int stream, uint8_t* data, std::size_t len);
	void create_streams(int stream_count);
	void set_max_section_size(uint32_t max);
	inline uint32_t get_max_section_size() const { return max_section_size; }
private:
	std::vector<StreamMetadata> stream_info;
	boost::asio::io_context& ctx;
	boost::asio::ip::udp::socket socket;
	boost::asio::ip::udp::endpoint destination;
	uint32_t max_section_size = 1024;
};

// Forward declaration for Frame (small cross-dependency for "friend" declaration)
class StreamReceiver;
// Provides thread-safe, RAII-style access to completed frames
class Frame {
public:
	Frame() = default;
	Frame(Frame&& src);
	Frame(const Frame&) = delete;
	~Frame();

	void release();

	inline std::size_t size() const { return len; }
	inline uint8_t* data() { return _data; }
	inline const uint8_t* data() const { return _data; }
	inline uint8_t& operator[](std::size_t i) { return _data[i]; }
	inline uint8_t operator[](std::size_t i) const { return _data[i]; }
private:
	uint8_t* _data = nullptr;
	std::size_t len = 0;
	std::mutex* completion_lock = nullptr;

	friend StreamReceiver;
	void bind(std::mutex* completion_lock, uint8_t* data, std::size_t len);
};

// Partial thread-safety:
//	- io_context handlers may be dispatched concurrently
//	- control functions (open_stream, close_stream, etc.) may not be called concurrently
// Intended design: 1 main thread, 1 io thread, 1 frame processing thread
class StreamReceiver {
private:
	// Private: Internal organizational structures

	// Metadata and pointers for frame buffers allocated by Stream
	struct FrameBuf {
		uint8_t* data;
		std::size_t received_size;
		uint8_t received_sections;
		uint8_t frame_index;
	};
	
	// Hold buffers and other metadata necessary for reconstructing a single stream
	struct Stream {
		// Allocate and use n buffers for reconstructing this stream. Any previously buffered data is lost
		void alloc_buffers(std::size_t size, int n);
		void free_buffers();
		Stream(Stream&& src);
		Stream();
		~Stream();

		// All buffer sizes are equal, so allocate as one contiguous block
		std::size_t indv_buffer_size;
		uint8_t* all_data = nullptr;
		std::vector<FrameBuf> frame_buffers;

		// Lock when the complete buffer is in use to prevent overwriting when another frame completes
		std::mutex completion_lock;

		// Which buffer is complete?
		int complete_buffer = -1;
		bool open = false;
	};
public:

	// Placeholder aliases for frame receipt callback
	struct args {
		typedef decltype(std::placeholders::_1) stream_index;
	};

	StreamReceiver(boost::asio::io_context& io_context);
	void set_listen_port(uint16_t port);
	void begin(uint16_t port);
	void open_stream(int stream);
	void close_stream(int stream);
	void destroy_stream(int stream);

	// Get exclusive access to the latest completed frame.
	// throws std::out_of_range if stream is invalid
	// throws std::range_error if no frame is available
	Frame get_complete_frame(int stream);
	inline void on_frame_received(std::function<void(int stream)> handler) { frame_handler = handler; }

private:
	boost::asio::io_context& ctx;
	boost::asio::ip::udp::socket socket;
	boost::asio::ip::udp::endpoint remote;
	boost::array<uint8_t, 2048> recv_buffer;
	std::vector<Stream> streams;
	// Reader-writer lock: stream vector cannot be reallocated while being read
	std::shared_mutex streams_lock;
	std::function<void(int stream)> frame_handler;
	// Default size: 4MB
	unsigned frame_buffer_size = 4 * 1024 * 1024;
	unsigned frame_buffer_level = 3;
	uint16_t port;
	void receive();

};

// Identifies information needed to reconstruct multiple streams from streams split into sections
// Only 3 LSB of offset
struct FrameHeader {
	static constexpr std::size_t SIZE = 4 * sizeof(int8_t) + 3;
	int8_t stream_index;
	uint8_t frame_index;
	uint8_t section_index;
	uint8_t section_count;
	// Max: 16 MB
	uint32_t offset;
	void write(uint8_t* arr) const;
	void read(const uint8_t* arr);
	void write_new_section(uint8_t* arr) const;
};

} // end namespace net