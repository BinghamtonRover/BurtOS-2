#include "network.hpp"

#include <boost/bind/bind.hpp>
#include <iostream>

void net::MessageSender::send_message(msg::Message& message) {
	// Do not allow sending to start while writing to the active buffer
	// A positive side effect is thread safety for concurrent send_message() calls
	std::lock_guard lock(async_start_lock);

	if (pause_sending || !destination_provided) {
		return;
	}

	// Ensure the serialized data size can fit in the header.
	if (message.data_p->ByteSizeLong() <= msg::Header::MAX_MSG_SIZE) {
		auto& buf = msg_buffer.write_buffer(); 

		uint8_t* block = buf.create_block(message.data_p->GetCachedSize() + msg::Header::HDR_SIZE);
		
		bool success = message.data_p->SerializeWithCachedSizesToArray(&block[msg::Header::HDR_SIZE]);

		// On failure, mark the type as NONE and send the invalid data
		// Block deallocation will be impossible if we allow concurrent writers (planned feature)
		// Failure should never happen, but this solution handles it
		msg::type_t use_type = message.type;
		if (!success) use_type = msg::TYPE_NONE;

		// Write the message header at the beginning of the block
		msg::Header hdr(use_type, message.data_p->GetCachedSize());
		hdr.write(block);

	}

	if (!async_send_active) {
		begin_sending();
	}

}

// Function assumes that caller has acquired async_start_lock
void net::MessageSender::begin_sending() {

	async_send_active = true;
	msg_buffer.swap();
	auto& to_send = msg_buffer.read_only_buffer();
	socket.async_send_to(boost::asio::buffer(to_send.data(), to_send.usage()), dest, [this](boost::system::error_code ec, std::size_t) {
		// On send finished:

		msg_buffer.read_only_buffer().clear();
		{
			std::lock_guard lock(async_start_lock);
			async_send_active = false;
			if (msg_buffer.write_buffer().usage() > 0) {
				begin_sending();
			}
		}

		if (ec) {
			error_emitter(ec);
		}

	});
}

void net::MessageSender::wait_finish(boost::asio::io_context& io_context) {
	disable();
	// At this point, another send cannot start until enable() is called
	// If there are any remaining jobs, wait

	while (async_send_active) {
		io_context.poll();
	}
}

void net::MessageSender::disable() {
	std::lock_guard lock(async_start_lock);

	pause_sending = true;
	msg_buffer.write_buffer().clear();

}

net::MessageSender::MessageSender(boost::asio::io_context& io_context, const Destination& device_ip)
		: socket(io_context), dest(device_ip), destination_provided(true) {

	socket.open(device_ip.protocol());
}

net::MessageSender::MessageSender(boost::asio::io_context& io_context)
		: socket(io_context), destination_provided(false) {}

void net::MessageSender::set_destination_endpoint(const Destination& endpoint) {
	// Reopen the socket with the new protocol if they don't match
	bool reopen = dest.protocol() != endpoint.protocol();

	// Endpoint assignment is not atomic, so make sure sending cannot restart until this is finished
	std::lock_guard lock(async_start_lock);
	dest = endpoint;
	destination_provided = true;

	if (reopen || !socket.is_open()) {

		if (socket.is_open())
			socket.close();

		socket.open(dest.protocol());
	}

}


net::MessageReceiver::MessageReceiver(boost::asio::io_context& io_context)
	: socket(io_context),
	use_multicast(false) {

}

net::MessageReceiver::MessageReceiver(boost::asio::io_context& io_context, uint_least16_t listen_port, bool open)
	: socket(io_context),
	listen_ep(boost::asio::ip::address_v4(), listen_port),
	use_multicast(false) {
	
	if (open) bind();
}

net::MessageReceiver::MessageReceiver(boost::asio::io_context& io_context, const boost::asio::ip::udp::endpoint& mcast_feed, bool open)
	: socket(io_context),
	listen_ep(mcast_feed),
	use_multicast(true) {
	
	if (open) bind();
}

void net::MessageReceiver::set_listen_endpoint(const boost::asio::ip::udp::endpoint& ep) {
	listen_ep = ep;
	bind();
}

void net::MessageReceiver::set_multicast(bool on) {
	if (on != use_multicast) {
		use_multicast = on;
		bind();
	}
}

void net::MessageReceiver::bind() {
	if (!enable_open_socket)
		return;
	
	socket.close();

	if (use_multicast) {
		throw std::runtime_error("MessageReceiver::bind: multicast not implemented");
	} else {
		socket.open(listen_ep.protocol());
		socket.bind(listen_ep);
		std::cout << "bind complete\n";
	}
	listen();
}

void net::MessageReceiver::open() {
	enable_open_socket = true;
	bind();
}

void net::MessageReceiver::close() {
	enable_open_socket = false;
	socket.close();
}

void net::MessageReceiver::listen() {
	if (!socket.is_open()) {
		listening = false;
		std::cout << "Not listening on socket (socket is closed)\n";
		return;
	}
	if (!listening) {
		std::cout << "Starting to listen on socket\n";
		socket.async_receive_from(boost::asio::buffer(recv_buffer), remote, [this](boost::system::error_code ec, std::size_t bytes_transferred) {
			read_messages(recv_buffer.data(), bytes_transferred);
			std::cout << "Read " << bytes_transferred << " bytes (error code: " << ec.message() << ")\n";

			if (ec) {
				if (ec != boost::asio::error::operation_aborted)
					error_emitter(ec);
			} else {
				last_activity = std::chrono::system_clock::now();
			}

			listening = false;
			listen();
		});
		listening = true;
	}
}
