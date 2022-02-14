#pragma once

#include "network_util.hpp"

#include <vector>
#include <mutex>
#include <cstdint>
#include <limits>
#include <google/protobuf/message.h>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <messages.hpp>
#include <events.hpp>

namespace net {

typedef boost::asio::ip::udp::endpoint Destination;

/*
	Maintains a socket and a double-buffered queue for one specific remote device

	Intended for devices with continuous message exchange (ex. rover and base
	station) rather than temporary connections (ex. true client/server programs)
*/
class MessageSender {
	public:
		MessageSender(boost::asio::io_context& io_context, const Destination& device_ip);
		MessageSender(boost::asio::io_context& io_context);
		void send_message(msg::Message& message);

		// Change the destination. If the device was constructed without a destination, this enables the device.
		// Otherwise, enabled/disabled state is unchanged.
		void set_destination_endpoint(const Destination& endpoint);
		inline const Destination& destination_endpoint() const { return dest; }

		// Disable device and block until the remaining operations have finished
		// Continues dispatching other jobs on io_context until return
		void wait_finish(boost::asio::io_context& io_context);

		// Stops messages from being queued by send_message(). Clears queues but does not stop dispatched jobs
		void disable();
		inline void enable() { _disable = false; }
		inline bool enabled() const { return !_disable; }
		inline event::Emitter<const boost::system::error_code&>& event_send_error() { return error_emitter; }

		// Close and reopen the socket
		void reset();
	private:
		boost::asio::ip::udp::socket socket;
		Destination dest;
		DoubleBuffer<uint8_t> msg_buffer;
		std::mutex async_start_lock;
		event::Emitter<const boost::system::error_code&> error_emitter;
		bool async_send_active = false;
		bool _disable = false;
		bool destination_provided;

		void begin_sending();

};

class MessageReceiver : public msg::Receiver {
	public:
		MessageReceiver(boost::asio::io_context& io_context);
		MessageReceiver(boost::asio::io_context& io_context, uint_least16_t listen_port, bool open = true);
		MessageReceiver(boost::asio::io_context& io_context, const boost::asio::ip::udp::endpoint& mcast_feed, bool open = true);

		void set_listen_port(uint_least16_t port);
		void subscribe(const boost::asio::ip::udp::endpoint& mcast_feed);
		inline int listen_port() const { return listen_ep.port(); }
		inline const Destination& listen_endpoint() const { return listen_ep; }

		void open();
		void close();
		inline bool opened() const { return socket.is_open(); }
		inline Destination& remote_sender() { return remote; }
		inline event::Emitter<const boost::system::error_code&>& event_receive_error() { return error_emitter; }
	private:
		boost::asio::ip::udp::socket socket;
		Destination remote;
		boost::asio::ip::udp::endpoint listen_ep;
		event::Emitter<const boost::system::error_code&> error_emitter;
		bool use_multicast;
		bool listening = false;
		
		boost::array<uint8_t, 2048> recv_buffer;

		void listen();
		
};

} // end namespace Network
