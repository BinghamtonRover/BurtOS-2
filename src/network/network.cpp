#include "network.hpp"

#include <boost/bind/bind.hpp>

net::MessageHeader::MessageHeader(const uint8_t* arr) {
	read(arr);
}

net::MessageHeader::MessageHeader(MessageType type, int size) : type(type), size(size) { }

void net::MessageHeader::write(uint8_t* arr) const {
	arr[0] = size;
	arr[1] = size >> 8;
	arr[2] = static_cast<uint8_t>(type);
}

void net::MessageHeader::read(const uint8_t* arr) {
	size = arr[0];
	size |= arr[1] << 8;
	type = static_cast<MessageType>(arr[2]);
	if (type > MessageType::COUNT) {
		type = MessageType::NONE;
	}
}

void net::RemoteDevice::send_message(google::protobuf::Message& msg, MessageType type) {
	// Do not allow sending to start while writing to the active buffer
	// A positive side effect is thread safety for concurrent send_message() calls
	async_start_lock.lock();

	auto& buf = msg_buffer.write_buffer(); 
	bool success = false;

	// Ensure size isn't larger than supported by the header
	if (msg.ByteSizeLong() <= MessageHeader::MAX_MSG_SIZE) {
		uint8_t* block = buf.create_block(msg.GetCachedSize() + MessageHeader::HDR_SIZE);
		
		success = msg.SerializeWithCachedSizesToArray(&block[MessageHeader::HDR_SIZE]);

		// On failure, mark the type as NONE and send the invalid data
		// Block deallocation be impossible if we allow concurrent writers (planned feature)
		// Failure should never happen, but this solution handles it
		if (!success) type = MessageType::NONE;

		MessageHeader hdr(type, msg.GetCachedSize());
		hdr.write(block);

	}

	async_start_lock.unlock();
	if (success) data_available();

}

void net::RemoteDevice::send_finished_handler(const boost::system::error_code& error, std::size_t bytes_transferred) {
	msg_buffer.read_only_buffer().clear();
	async_send_active = false;

	if (msg_buffer.write_buffer().usage() > 0) {
		data_available();
	}

}

void net::RemoteDevice::data_available() {
	if (!async_send_active && async_start_lock.try_lock()) {
		async_send_active = true;
		msg_buffer.swap();
		async_start_lock.unlock();

		auto& to_send = msg_buffer.read_only_buffer();
		socket.async_send_to(boost::asio::buffer(to_send.data(), to_send.usage()), dest, [this](auto error, auto bytes_transferred) {
			send_finished_handler(error, bytes_transferred);
		});
	}
}

net::RemoteDevice::RemoteDevice(const Destination& device_ip, boost::asio::io_context& io_context)
		: dest(device_ip), socket(io_context) {

	socket.open(boost::asio::ip::udp::v4());
}

net::MessageReceiver::MessageReceiver(uint_least16_t listen_port, boost::asio::io_context& io_context)
		: socket(io_context, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), listen_port)) {
	
	for (int i = 0; i < (int)MessageType::COUNT; i++) {
		handlers[i] = nullptr;
	}
}

void net::MessageReceiver::open() {
	socket.async_receive_from(boost::asio::buffer(recv_buffer), remote, [this](auto error, auto bytes_transferred) {
		std::size_t i = 0;
		while (i + MessageHeader::HDR_SIZE <= bytes_transferred) {
			MessageHeader hdr(&recv_buffer[i]);
			i += MessageHeader::HDR_SIZE;

			if (hdr.type != MessageType::NONE && i + hdr.size <= bytes_transferred) {
				Handler h = handlers[(std::size_t)hdr.type];
				if (h != nullptr) {
					h(&recv_buffer[i], hdr.size, remote);
				}
				i += hdr.size;
			} else {
				// Buffer too short
				break;
			}

		}

		// Reopen socket to receive more
		open();
	});
}

void net::MessageReceiver::close() {
	socket.close();
}

void net::MessageReceiver::register_handler(MessageType type, Handler handler) {
	if (type < MessageType::COUNT) {
		handlers[(std::size_t)type] = handler;
	}
}