#include "network.hpp"

#include <iostream>
#include <limits>
#include <boost/bind/bind.hpp>

void Network::RemoteDevice::send_bytes(void* data, std::size_t size_bytes) {
	// Do not allow sending to start while writing to the active buffer
	// A positive side effect is thread safety for concurrent send_message() calls
	async_start_lock.lock();

	std::size_t available_space = b.get_active_buffer().size() - b.usage[b.active];

	if (size_bytes > available_space) {
		b.get_active_buffer().resize(b.usage[b.active] + size_bytes);
	}

	for (int i = 0; i < size_bytes; i++) {
		b.get_active_buffer()[b.usage[b.active] + i] = ((const uint8_t*)data)[i];
	}
	b.usage[b.active] += size_bytes;

	async_start_lock.unlock();

	data_available();
}

void Network::RemoteDevice::send_message(google::protobuf::Message& msg, MessageType type) {
	// Do not allow sending to start while writing to the active buffer
	// A positive side effect is thread safety for concurrent send_message() calls
	async_start_lock.lock();

	auto& buf = b.get_active_buffer();
	std::size_t usage = b.usage[b.active];
	std::size_t available_space = buf.size() - usage;
	bool success = false;

	// Headers use 16-bit sizes. Packets shouldn't be larger than that anyway
	if (msg.ByteSizeLong() <= std::numeric_limits<uint16_t>::max()) {
		// Reallocate if there isn't enough space. 3 bytes needed for message header
		if (available_space < msg.GetCachedSize() + 3) {
			// Only resize to the minimum necessary since control message sizes should be constant
			buf.resize(usage + msg.GetCachedSize() + 3);
		}
		
		success = msg.SerializeWithCachedSizesToArray(&buf[usage + 3]);

		if (success) {
			// Header format: Little-endian 16-bit size followed by the message type
			buf[usage] = msg.GetCachedSize();
			buf[usage + 1] = msg.GetCachedSize() >> 8;
			buf[usage + 2] = static_cast<uint8_t>(type);
			b.usage[b.active] += msg.GetCachedSize() + 3;
		}

	}

	async_start_lock.unlock();
	if (success) data_available();

}

void Network::RemoteDevice::send_finished_handler(const boost::system::error_code& error, std::size_t bytes_transferred) {
	std::cout << "event: send_finished<transferred=" << bytes_transferred << " B, error=" << error << ">\n";
	b.usage[!b.active] = 0;
	async_send_active = false;

	if (b.usage[b.active] > 0) {
		data_available();
	}

}

void Network::RemoteDevice::data_available() {
	if (!async_send_active && async_start_lock.try_lock()) {
		std::cout << "Starting data transmission\n";
		async_send_active = true;
		b.swap();
		async_start_lock.unlock();

		auto& to_send = b.get_locked_buffer();
		std::cout << "Data available to send: " << b.usage[!b.active] << std::endl;
		socket.async_send_to(boost::asio::buffer(to_send.data(), b.usage[!b.active]), dest, [this](auto error, auto bytes_transferred) {
			send_finished_handler(error, bytes_transferred);
		});
	}
}

Network::RemoteDevice::RemoteDevice(const Destination& device_ip, boost::asio::io_context& io_context)
		: dest(device_ip), socket(io_context) {

	socket.open(boost::asio::ip::udp::v4());
	// Start with buffer length of 100
	b.buf[0].resize(100);
	b.buf[1].resize(100);
}

Network::MessageReceiver::MessageReceiver(boost::asio::ip::port_type listen_port, boost::asio::io_context& io_context)
		: socket(io_context, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), listen_port)) {
	
	for (int i = 0; i < (int)MessageType::COUNT; i++) {
		handlers[i] = nullptr;
	}
}

void Network::MessageReceiver::open() {
	socket.async_receive_from(boost::asio::buffer(recv_buffer), remote, [this](auto error, auto bytes_transferred) {
		std::cout << "received in " << bytes_transferred << "B\n";
		std::size_t i = 0;
		while (i + 3 <= bytes_transferred) {
			uint16_t sz = *(uint16_t*)(&recv_buffer[i]);
			MessageType t = static_cast<MessageType>(recv_buffer[i + 2]);
			i += 3;

			std::cout << "message<t=" << (int)t << ", sz=" << sz << ">\n";

			if (t < MessageType::COUNT && i + sz <= bytes_transferred) {
				Handler h = handlers[(std::size_t)t];
				if (h != nullptr) {
					h(&recv_buffer[i], sz);
				}
				i += sz;
			} else {
				// Buffer too short
				break;
			}

		}

		// Reopen socket to receive more
		open();
	});
}

void Network::MessageReceiver::close() {
	socket.close();
}

void Network::MessageReceiver::register_handler(MessageType type, Handler handler) {
	if (type < MessageType::COUNT) {
		handlers[(std::size_t)type] = handler;
	}
}