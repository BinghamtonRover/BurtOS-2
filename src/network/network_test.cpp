#include <iostream>
#include <string>
#include <cstring>
#include <chrono>
#include "hello.pb.h"

#include "network.hpp"

using namespace Network;

constexpr boost::asio::ip::port_type sender_port = 1799;
constexpr boost::asio::ip::port_type receiver_port = 1800;

int main(int argc, char* argv[]) {
	try {
		boost::asio::io_context ctx;
		if (argc >= 2 && 0 == strcmp(argv[1], "-send")) {
			if (argc != 3) {
				std::cout << "Error: send expects one text message to send.\n";
			} else {
				static std::chrono::high_resolution_clock::time_point start;
				static MessageReceiver m(sender_port, ctx);
				static RemoteDevice client(Destination(boost::asio::ip::address::from_string("127.0.0.1"), receiver_port), ctx);
				static rover_message::Hello msg;
				static int count = 0;
				msg.set_hello_text_message(argv[2]);
				m.register_handler(MessageType::STATUS, [](const uint8_t* buf, std::size_t len) {
					auto end = std::chrono::high_resolution_clock::now();
					rover_message::Hello retourn;
					retourn.ParseFromArray(buf, len);
					std::cout << "rtt = " << std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / 1000.0 << " ms\n";
					
					count++;
					if (count == 5) {
						m.close();
						return;
					}
					start = std::chrono::high_resolution_clock::now();
					client.send_message(msg, MessageType::STATUS);
				});
				m.open();

				start = std::chrono::high_resolution_clock::now();
				client.send_message(msg, MessageType::STATUS);

				ctx.run();
				
			}
		} else if (argc == 1 || 0 == strcmp(argv[1], "-receive")) {
			MessageReceiver m(receiver_port, ctx);
			static RemoteDevice rd(Destination(boost::asio::ip::address::from_string("127.0.0.1"), sender_port), ctx);
			m.register_handler(MessageType::STATUS, [](const uint8_t* buf, std::size_t len) {
				rover_message::Hello msg;
				if (msg.ParseFromArray(buf, len)) {
					//std::cout << "STATUS: " << msg.hello_text_message() << '\n';
				} else {
					std::cout << "STATUS: Received invalid message\n";
				}
				rd.send_message(msg, MessageType::STATUS);
			});
			m.open();
			ctx.run();
		}

	} catch (const std::exception& e) {
		std::cerr << "That didn't go so well: " << e.what() << std::endl;
	}
	return 0;
} 