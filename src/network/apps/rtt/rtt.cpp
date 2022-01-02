/*
	Calculate the round trip time from this device to another using the network library
	Not a particularly stable or polished program
*/

#include <chrono>
#include <network.hpp>
#include <messages.hpp>
#include <iostream>
#include <string>
#include <boost/program_options.hpp>
#include <rtt_messages.pb.h>

namespace opt = boost::program_options;

namespace apps {
	DEFINE_MESSAGE_TYPE(RttMessage, apps::Rtt)
}

unsigned int timeout;
unsigned int trips;
unsigned short int port;
unsigned short int reply_port;
std::string ip_str;
std::string reply_ip_str;

void host_server() {
	static boost::asio::io_context ctx;
	static net::MessageReceiver m(port, ctx);
	m.register_handler<apps::RttMessage>([](const uint8_t buf[], std::size_t len) {
		try {
			apps::Rtt rtt_request;
			if (rtt_request.ParseFromArray(buf, len)) {
				apps::RttMessage reply;
				reply.data.set_reply_port(0);
				net::RemoteDevice reply_rd(net::Destination(m.remote_sender().address(), rtt_request.reply_port()), ctx);
				reply_rd.send_message(reply);
				reply_rd.wait_finish(ctx);
			}
		} catch (const std::exception& e) {
			std::cerr << "Error while hosting server: " << e.what() << '\n';
		}
	});
	m.open();
	ctx.run();
}

void run_tests() {
	if (trips == 0) return;
	static boost::asio::io_context ctx;
	static net::RemoteDevice rd(net::Destination(boost::asio::ip::address::from_string(ip_str), port), ctx);
	static apps::RttMessage rtt_request;
	static std::chrono::high_resolution_clock::time_point send_time;
	static std::chrono::high_resolution_clock::time_point reply_time;
	static net::MessageReceiver m(reply_port, ctx);

	m.register_handler<apps::RttMessage>([](const uint8_t buf[], std::size_t len) {
		reply_time = std::chrono::high_resolution_clock::now();
		std::cout << (std::chrono::duration_cast<std::chrono::microseconds>(reply_time - send_time).count()) / 1000.0 << " ms\n";

		trips -= 1;
		if (trips == 0) {
			m.close();
			ctx.stop();
		}

		send_time = std::chrono::high_resolution_clock::now();
		rd.send_message(rtt_request);
	});
	m.open();

	rtt_request.data.set_reply_port(reply_port);

	send_time = std::chrono::high_resolution_clock::now();
	rd.send_message(rtt_request);

	while (std::chrono::duration_cast<std::chrono::seconds>(std::chrono::high_resolution_clock::now() - send_time).count() < timeout) {
		ctx.run_for(std::chrono::seconds(timeout));
		if (trips == 0) break;
	}
	if (trips != 0) {
		std::cout << "Timed out while waiting for response.\n";
	}
}

int main(int argc, char* argv[]) {
	opt::options_description options("Options");
	options.add_options()
		("help,h", "detail program usage")
		("count,c", opt::value<unsigned int>(&trips)->default_value(5), "number of trips")
		("timeout,t", opt::value<unsigned int>(&timeout)->default_value(10), "how long to wait before cancelling (client)")
		("server,s", "host the RTT server")
		("ip", opt::value<std::string>(&ip_str), "server to send to")
		("port", opt::value<unsigned short int>(&port)->default_value(40005), "port to send to (client) or bind to (server)")
		("replyport", opt::value<unsigned short int>(&reply_port)->default_value(40006), "port for client to listen for response")
	;

	opt::positional_options_description positional;
	positional.add("ip", 1);

	opt::variables_map vm;
	opt::store(opt::command_line_parser(argc, argv).options(options).positional(positional).run(), vm);
	opt::notify(vm);

	if (vm.count("help") || argc == 1) {
		std::cout << "Calculates round-trip time using the rover network library\n";
		std::cout << "General usage: run 'rtt -s' on the host device and 'rtt [host ip address]' to calculate the round-trip time to that host.\n";
		std::cout << options << '\n';
		return 0;
	}

	msg::register_message_type<apps::RttMessage>();
	if (vm.count("server")) {
		host_server();
	} else {
		run_tests();
	}

	return 0;
}
