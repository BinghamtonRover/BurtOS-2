#pragma once

#include "network_util.hpp"

#include <vector>
#include <mutex>
#include <cstdint>
#include <google/protobuf/message.h>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <messages.hpp>

namespace Network {

typedef boost::asio::ip::udp::endpoint Destination;

/*
	Maintains a socket and a double-buffered queue for one specific remote device
*/
class RemoteDevice {
	public:
		RemoteDevice(const Destination& device_ip, boost::asio::io_context& io_context);
		void send_message(google::protobuf::Message& msg, MessageType type);
		void send_bytes(void* data, std::size_t size_bytes);
	private:
		boost::asio::ip::udp::socket socket;
		Destination dest;
		DoubleBuffer<uint8_t> b;
		std::mutex async_start_lock;
		bool async_send_active = false;

		void data_available();
		void send_finished_handler(const boost::system::error_code& error, std::size_t bytes_transferred);

};

class MessageReceiver {
	public:
		MessageReceiver(uint_least16_t listen_port, boost::asio::io_context& io_context);
		void open();
		void close();
		typedef void(*Handler)(const uint8_t*, std::size_t);
		void register_handler(MessageType type, Handler handler);
	private:
		boost::asio::ip::udp::socket socket;
		Destination remote;
		boost::array<uint8_t, 2048> recv_buffer;

		Handler handlers[(std::size_t)MessageType::COUNT];
		
};

} // end namespace Network
