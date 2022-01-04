/*
	A chat application that uses the rover network library
	Great for validating changes to the network library. Not particularly useful otherwise.

	Also serves as an example for a multi-threaded network program with command-line options.
*/

#include <iostream>
#include <string>
#include <cstring>
#include <string>
#include <thread>

#include <boost/program_options.hpp>
#include <network.hpp>
#include <chat_messages.pb.h>

DEFINE_MESSAGE_TYPE(Chat, apps::Chat)
DEFINE_MESSAGE_TYPE(Join, apps::Join)
DEFINE_MESSAGE_TYPE(Leave, apps::Leave)
DEFINE_MESSAGE_TYPE(Here, apps::Here)

namespace opt = boost::program_options;

uint16_t listen_port;
uint16_t destination_port;
std::string destination_ip_str;
std::string name;
bool allow_send;

int read_options(int argc, char* argv[]) {
	try {
		opt::options_description options("Usage");
		options.add_options()
			("help,h", "detail program usage")
			("to,t", opt::value<std::string>(&destination_ip_str), "ip address of the destination")
			("name,n", opt::value<std::string>(&name)->default_value("Mysterious Stranger"), "your name")
			("to_port", opt::value<uint16_t>(&destination_port)->default_value(41000), "port of the destination")
			("listen_port", opt::value<uint16_t>(&listen_port)->default_value(41000), "port to receive messages on")
			("no-send,x", "do not listen for messages on stdin")
		;
		opt::positional_options_description pos;
		pos.add("to", 1);

		opt::variables_map vm;
		opt::store(opt::command_line_parser(argc, argv).options(options).positional(pos).run(), vm);
		opt::notify(vm);

		if (vm.count("help") || argc == 1 || vm.count("to") != 1) {
			std::cout << "Basic chat application using the rover network library\n";
			std::cout << "General usage: run 'chat [other ip address]' to open chat with another instance\n";
			std::cout << options << '\n';
			return 1;
		}

		allow_send = !vm.count("no-send");
		return 0;
	} catch (const std::exception& e) {
		std::cerr << "Invalid options provided: " << e.what() << '\n';
		std::cerr << "See 'chat -h' for help\n";
		return 1;
	}
}

void register_message_types() {
	msg::register_message_type<Chat>();
	msg::register_message_type<Join>();
	msg::register_message_type<Leave>();
	msg::register_message_type<Here>();
}

int main(int argc, char* argv[]) {

	int r = read_options(argc, argv);
	if (r != 0) {
		return r;
	}

	register_message_types();
	const char* phase = "initializing socket";

	try {
		boost::asio::io_context ctx;

		net::Destination d(boost::asio::ip::address::from_string(destination_ip_str), destination_port);

		static net::MessageSender client(ctx, d);
		net::MessageReceiver receiver(listen_port, ctx);

		receiver.register_handler<Chat>([](const uint8_t buf[], std::size_t len) {
			apps::Chat message_in;
			if (message_in.ParseFromArray(buf, len)) {
				std::cout << message_in.text() << '\n';
			}
		});

		receiver.register_handler<Join>([](const uint8_t buf[], std::size_t len) {
			apps::Join join_message;
			if (join_message.ParseFromArray(buf, len)) {
				std::cout << '\'' << join_message.name() << "' joined the chat.\n";

				Here reply_msg;
				reply_msg.data.set_name(name);
				client.send_message(reply_msg);
			}
		});

		receiver.register_handler<Here>([](const uint8_t buf[], std::size_t len) {
			apps::Here here_message;
			if (here_message.ParseFromArray(buf, len)) {
				std::cout << '\'' << here_message.name() << "' is here.\n";
			}
		});

		receiver.register_handler<Leave>([](const uint8_t buf[], std::size_t len) {
			apps::Leave leave_message;
			if (leave_message.ParseFromArray(buf, len)) {
				std::cout << '\'' << leave_message.name() << "' left the chat";
				if (leave_message.reason().size()) {
					std::cout << " (" << leave_message.reason() << ")\n";
				} else {
					std::cout << ".\n";
				}
			}
		});

		phase = "operating";

		if (!allow_send) {
			ctx.run();
			return 0;
		}

		std::thread io_thread([&ctx](void) {
			ctx.run();
		});

		Join joined_message;
		joined_message.data.set_name(name);
		client.send_message(joined_message);

		std::string line;
		while (std::getline(std::cin, line)) {
			if (line.substr(0, 5) == "/quit") {
				Leave leave_msg;
				leave_msg.data.set_name(name);
				if (line.size() > 6)
					leave_msg.data.set_reason(line.substr(6));
				client.send_message(leave_msg);
				break;
			} else {
				Chat write_msg;
				write_msg.data.set_text(line);
				client.send_message(write_msg);
			}
		}

		ctx.stop();
		io_thread.join();

	} catch (const std::exception& e) {
		std::cerr << "Error while " << phase << ": " << e.what() << std::endl;
		return 1;
	}
	return 0;
} 