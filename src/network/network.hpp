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

namespace net {

typedef boost::asio::ip::udp::endpoint Destination;

struct MessageHeader {
	constexpr static std::size_t HDR_SIZE = 3;
	constexpr static uint16_t MAX_MSG_SIZE = std::numeric_limits<uint16_t>::max();
	MessageType type;
	int size;	// Protobuf uses int type for size
	void write(uint8_t* arr) const;
	void read(const uint8_t* arr);
	MessageHeader(const uint8_t* arr);
	MessageHeader(MessageType type, int size);
};

/*
	Maintains a socket and a double-buffered queue for one specific remote device

	Intended for devices with continuous message exchange (ex. rover and base
	station) rather than temporary connections (ex. true client/server programs)
*/
class RemoteDevice {
	public:
		RemoteDevice(const Destination& device_ip, boost::asio::io_context& io_context);
		void send_message(google::protobuf::Message& msg, MessageType type);
	private:
		boost::asio::ip::udp::socket socket;
		Destination dest;
		DoubleBuffer<uint8_t> msg_buffer;
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
