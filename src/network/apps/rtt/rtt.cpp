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
#include <vector>
#include <algorithm>
#include <limits>

namespace opt = boost::program_options;

namespace apps {
	DEFINE_MESSAGE_TYPE(RttMessage, apps::Rtt)
}

unsigned int timeout;
unsigned int max_trips;
unsigned int trips;
unsigned short int port;
unsigned short int reply_port;
bool verbose_print = true;
std::string ip_str;
std::string reply_ip_str;

void host_server() {
	static boost::asio::io_context ctx;
	static net::MessageReceiver m(port, ctx);
	static net::MessageSender reply_rd(ctx);
	m.register_handler<apps::RttMessage>([](const uint8_t buf[], std::size_t len) {
		try {
			apps::Rtt rtt_request;
			if (rtt_request.ParseFromArray(buf, len)) {
				apps::RttMessage reply;
				reply.data.set_reply_port(0);
				reply_rd.set_destination_endpoint(net::Destination(m.remote_sender().address(), rtt_request.reply_port()));
				reply_rd.send_message(reply);
			}
		} catch (const std::exception& e) {
			std::cerr << "Error while hosting server: " << e.what() << '\n';
		}
	});
	m.open();
	ctx.run();
}

void run_tests() {
	trips = max_trips;
	if (trips == 0) return;
	static boost::asio::io_context ctx;
	static net::MessageSender rd(ctx, net::Destination(boost::asio::ip::address::from_string(ip_str), port));
	static apps::RttMessage rtt_request;
	static std::chrono::high_resolution_clock::time_point send_time;
	static std::chrono::high_resolution_clock::time_point reply_time;
	static net::MessageReceiver m(reply_port, ctx);
	static std::vector<double> all_times;
	all_times.reserve(max_trips);

	m.register_handler<apps::RttMessage>([](const uint8_t buf[], std::size_t len) {
		reply_time = std::chrono::high_resolution_clock::now();
		double time_ms = (std::chrono::duration_cast<std::chrono::microseconds>(reply_time - send_time).count()) / 1000.0;
		all_times.push_back(time_ms);
		if (verbose_print) std::cout << time_ms << " ms\n";

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
		if (trips == 0) {
			double min = std::numeric_limits<double>::infinity();
			double max = -min;
			double sum = 0.0;
			for (auto& x : all_times) {
				if (x < min) min = x;
				if (x > max) max = x;
				sum += x;
			}
			std::sort(all_times.begin(), all_times.end());
			double median;
			if (all_times.size() % 2 == 0) {
				median = (all_times[all_times.size() / 2] + all_times[all_times.size() / 2 - 1]) / 2.0;
			} else {
				median = all_times[all_times.size() / 2];
			}

			std::cout << "Average: " << (sum / all_times.size()) << " ms\n";
			std::cout << "Median: " << median << " ms\n";
			std::cout << "Min: " << min << " ms\n";
			std::cout << "Max: " << max << " ms\n";

			break;
		}
	}
	if (trips != 0) {
		std::cout << "Timed out while waiting for response.\n";
	}
}

int main(int argc, char* argv[]) {
	opt::options_description options("Options");
	options.add_options()
		("help,h", "detail program usage")
		("count,c", opt::value<unsigned int>(&max_trips)->default_value(5), "number of trips")
		("timeout,t", opt::value<unsigned int>(&timeout)->default_value(5), "how long to wait before cancelling (client)")
		("server,s", "host the RTT server")
		("ip", opt::value<std::string>(&ip_str), "server to send to")
		("port", opt::value<unsigned short int>(&port)->default_value(40005), "port to send to (client) or bind to (server)")
		("replyport", opt::value<unsigned short int>(&reply_port)->default_value(40006), "port for client to listen for response")
		("stats-only", "only print the summary stats")
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

	if (vm.count("stats-only")) {
		verbose_print = false;
	}

	msg::register_message_type<apps::RttMessage>();
	if (vm.count("server")) {
		host_server();
	} else {
		run_tests();
	}

	return 0;
}
