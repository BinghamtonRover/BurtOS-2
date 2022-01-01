/*
	Calculate the round trip time from this device to another using the network library
*/

#include <chrono>
#include <network.hpp>
#include <messages.hpp>
#include <iostream>
#include <string>
#include <boost/program_options.hpp>

namespace opt = boost::program_options;

unsigned int trips;
unsigned short int port;
unsigned short int reply_port;
std::string ip_str;
std::string reply_ip_str;

void host_server() {
	static boost::asio::io_context ctx;
	static net::RemoteDevice* rd = nullptr;
	net::MessageReceiver m(port, ctx);
	m.register_handler(net::MessageType::RTT, [](const uint8_t buf[], std::size_t len) {
		try {
			extras::Rtt rtt_request;
			if (rtt_request.ParseFromArray(buf, len)) {
				extras::Rtt reply;
				reply.set_reply_port(-1);
				reply.set_reply_ip(0);
				if (rtt_request.reply_port() == -1) {
					rd->send_message(reply, net::MessageType::RTT);
				} else {
					delete rd;
					rd = new net::RemoteDevice(net::Destination(boost::asio::ip::address_v4(rtt_request.reply_ip()), rtt_request.reply_port()), ctx);
					rd->send_message(reply, net::MessageType::RTT);
				}
			}
		} catch (const std::exception& e) {
			std::cerr << "Error while hosting server: " << e.what() << '\n';
		}
	});
	m.open();
	ctx.run();
}

void run_tests() {
	static boost::asio::io_context ctx;
	static net::RemoteDevice rd(net::Destination(boost::asio::ip::address::from_string(ip_str), port), ctx);
	static extras::Rtt rtt_request;
	static std::chrono::high_resolution_clock::time_point send_time;
	static std::chrono::high_resolution_clock::time_point reply_time;
	static net::MessageReceiver m(reply_port, ctx);

	m.register_handler(net::MessageType::RTT, [](const uint8_t buf[], std::size_t len) {
		reply_time = std::chrono::high_resolution_clock::now();
		std::cout << (std::chrono::duration_cast<std::chrono::microseconds>(reply_time - send_time).count()) / 1000.0 << " ms\n";

		trips -= 1;
		if (trips == 0) {
			m.close();
			ctx.stop();
		}

		rtt_request.set_reply_ip(-1);
		rtt_request.set_reply_port(-1);
		send_time = std::chrono::high_resolution_clock::now();
		rd.send_message(rtt_request, net::MessageType::RTT);
	});
	m.open();

	rtt_request.set_reply_ip(boost::asio::ip::address_v4::from_string(reply_ip_str).to_uint());
	rtt_request.set_reply_port(reply_port);

	send_time = std::chrono::high_resolution_clock::now();
	rd.send_message(rtt_request, net::MessageType::RTT);
	ctx.run();
}

int main(int argc, char* argv[]) {
	opt::options_description options("Options");
	options.add_options()
		("help,h", "detail program usage")
		("count,c", opt::value<unsigned int>(&trips)->default_value(5), "number of trips")
		("server,s", "host the RTT server")
		("ip", opt::value<std::string>(&ip_str), "server to send to")
		("reply_ip", opt::value<std::string>(&reply_ip_str), "should be the local ip of the client machine")
		("reply_port", opt::value<unsigned short int>(&reply_port)->default_value(40006))
		("port", opt::value<unsigned short int>(&port)->default_value(40005), "port to send to (client) or bind to (server)")
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

	if (vm.count("server")) {
		host_server();
	} else {
		run_tests();
	}

	return 0;
}
