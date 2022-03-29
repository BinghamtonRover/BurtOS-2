#include "network.hpp"

#include <boost/bind/bind.hpp>

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

	socket.open(boost::asio::ip::udp::v4());
}

net::MessageSender::MessageSender(boost::asio::io_context& io_context)
		: socket(io_context), destination_provided(false) {

	socket.open(boost::asio::ip::udp::v4());
}

void net::MessageSender::set_destination_endpoint(const Destination& endpoint) {
	dest = endpoint;
	destination_provided = true;
}

void net::MessageSender::reset() {
	disable();
	socket.close();
	socket.open(boost::asio::ip::udp::v4());
}

net::MessageReceiver::MessageReceiver(boost::asio::io_context& io_context)
	: socket(io_context),
	use_multicast(false) {

}

net::MessageReceiver::MessageReceiver(boost::asio::io_context& io_context, uint_least16_t listen_port, bool open)
	: socket(io_context),
	listen_ep(boost::asio::ip::address_v4(), listen_port),
	use_multicast(false) {
	
	if (open) this->open();
}

net::MessageReceiver::MessageReceiver(boost::asio::io_context& io_context, const boost::asio::ip::udp::endpoint& mcast_feed, bool open)
	: socket(io_context),
	listen_ep(mcast_feed),
	use_multicast(true) {
	
	if (open) this->open();
}

void net::MessageReceiver::set_listen_port(uint_least16_t port) {
	use_multicast = false;
	listen_ep.port(port);

	if (socket.is_open()) {
		open();
	}
}

void net::MessageReceiver::subscribe(const boost::asio::ip::udp::endpoint& mcast_feed) {
	use_multicast = true;
	listen_ep = mcast_feed;

	if (socket.is_open()) {
		open();
	}
}

void net::MessageReceiver::set_listen_endpoint(const boost::asio::ip::udp::endpoint& ep) {
	listen_ep = ep;

	if (socket.is_open()) {
		open();
	}
}

void net::MessageReceiver::set_multicast(bool on) {
	if (on != use_multicast) {
		use_multicast = on;
		if (socket.is_open()) {
			open();
		}
	}
}

void net::MessageReceiver::open() {
	close();

	if (use_multicast) {
		socket.open(listen_ep.protocol());
		socket.set_option(boost::asio::ip::udp::socket::reuse_address(true));
		socket.bind(boost::asio::ip::udp::endpoint(
			boost::asio::ip::address_v4::from_string("0.0.0.0"),
			listen_ep.port()
		));
		socket.set_option(boost::asio::ip::multicast::join_group(listen_ep.address()));
	} else {
		socket.open(boost::asio::ip::udp::v4());
		socket.bind(boost::asio::ip::udp::endpoint(boost::asio::ip::udp::v4(), listen_ep.port()));
	}
	if (!listening)
		listen();
}

void net::MessageReceiver::listen() {
	if (!socket.is_open()) {
		listening = false;
		return;
	}
	listening = true;
	socket.async_receive_from(boost::asio::buffer(recv_buffer), remote, [this](boost::system::error_code ec, std::size_t bytes_transferred) {
		read_messages(recv_buffer.data(), bytes_transferred);

		if (ec) {
			if (ec != boost::asio::error::operation_aborted)
				error_emitter(ec);
		} else {
			last_activity = std::chrono::system_clock::now();
		}

		listen();
	});
}

void net::MessageReceiver::close() {
	if (socket.is_open()) {
		socket.close();
	}
}
