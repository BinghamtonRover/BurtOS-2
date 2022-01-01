#include <iostream>
#include <string>
#include <cstring>
#include "extras.pb.h"

#include "network.hpp"

using namespace net;

constexpr boost::asio::ip::port_type sender_port = 1799;
constexpr boost::asio::ip::port_type receiver_port = 1800;

int main(int argc, char* argv[]) {
	try {
		boost::asio::io_context ctx;
		if (argc >= 2 && 0 == strcmp(argv[1], "-send")) {
			if (argc < 3) {
				std::cout << "Error: send expects one text message to send.\n";
			} else {
				Destination d(boost::asio::ip::address::from_string("127.0.0.1"), receiver_port);
				if (argc > 3 && 0 == strcmp(argv[3], "-ip")) {
					if (argc != 5) {
						std::cout << "Error: ip option expects a valid IP endpoint\n";
						exit(0);
					} else {
						std::cout << "Using provided IP address\n";
						d = Destination(boost::asio::ip::address::from_string(argv[4]), receiver_port);
					}
				}
				RemoteDevice client(d, ctx);
				extras::StringMessage msg;
				msg.set_text(argv[2]);

				client.send_message(msg, MessageType::STRING_MESSAGE);

				ctx.run();
				
			}
		} else if (argc == 1 || 0 == strcmp(argv[1], "-receive")) {
			MessageReceiver m(receiver_port, ctx);
			m.register_handler(MessageType::STRING_MESSAGE, [](const uint8_t* buf, std::size_t len, net::Destination& sender) {
				extras::StringMessage msg;
				if (msg.ParseFromArray(buf, len)) {
					std::cout << "STATUS: " << msg.text() << '\n';
				} else {
					std::cout << "STATUS: Received invalid message\n";
				}
			});
			m.open();
			ctx.run();
		}

	} catch (const std::exception& e) {
		std::cerr << "That didn't go so well: " << e.what() << std::endl;
	}
	return 0;
} 