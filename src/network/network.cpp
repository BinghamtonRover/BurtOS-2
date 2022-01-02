#include "network.hpp"

#include <boost/bind/bind.hpp>

void net::RemoteDevice::send_message(msg::Message& message) {
	// Do not allow sending to start while writing to the active buffer
	// A positive side effect is thread safety for concurrent send_message() calls
	async_start_lock.lock();

	if (_disable) {
		async_start_lock.unlock();
		return;
	}

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

	if (!async_send_active) {
		begin_sending();
	}

	async_start_lock.unlock();

}

// Function assumes that caller has acquired async_start_lock
void net::RemoteDevice::begin_sending() {

	async_send_active = true;
	msg_buffer.swap();
	auto& to_send = msg_buffer.read_only_buffer();
	socket.async_send_to(boost::asio::buffer(to_send.data(), to_send.usage()), dest, [this](auto error, auto bytes_transferred) {
		// On send finished:

		msg_buffer.read_only_buffer().clear();
		async_start_lock.lock();
		async_send_active = false;
		if (msg_buffer.write_buffer().usage() > 0) {
			begin_sending();
		}
		async_start_lock.unlock();

	});
}

void net::RemoteDevice::wait_finish(boost::asio::io_context& io_context) {
	disable();
	// At this point, another send cannot start until enable() is called
	// If there are any remaining jobs, wait

	while (async_send_active) {
		io_context.poll();
	}
}

void net::RemoteDevice::disable() {
	async_start_lock.lock();

	_disable = true;
	msg_buffer.write_buffer().clear();

	async_start_lock.unlock();
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
