#include "network.hpp"

#include <boost/bind/bind.hpp>

void net::RemoteDevice::send_message(msg::Message& message) {
	// Do not allow sending to start while writing to the active buffer
	// A positive side effect is thread safety for concurrent send_message() calls
	async_start_lock.lock();

	auto& buf = msg_buffer.write_buffer(); 
	bool success = false;

	// Ensure size isn't larger than supported by the header
	if (message.data_p->ByteSizeLong() <= msg::Header::MAX_MSG_SIZE) {
		uint8_t* block = buf.create_block(message.data_p->GetCachedSize() + msg::Header::HDR_SIZE);
		
		success = message.data_p->SerializeWithCachedSizesToArray(&block[msg::Header::HDR_SIZE]);

		// On failure, mark the type as NONE and send the invalid data
		// Block deallocation will be impossible if we allow concurrent writers (planned feature)
		// Failure should never happen, but this solution handles it
		msg::type_t use_type = message.type;
		if (!success) use_type = msg::TYPE_NONE;

		msg::Header hdr(use_type, message.data_p->GetCachedSize());
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

net::MessageReceiver::MessageReceiver(uint_least16_t listen_port, boost::asio::io_context& io_context, bool open)
		: socket(io_context, boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), listen_port)) {
	
	if (open) this->open();
}

void net::MessageReceiver::open() {
	socket.async_receive_from(boost::asio::buffer(recv_buffer), remote, [this](auto error, auto bytes_transferred) {
		read_messages(recv_buffer.data(), bytes_transferred);
		open();
	});
}

void net::MessageReceiver::close() {
	socket.close();
}
