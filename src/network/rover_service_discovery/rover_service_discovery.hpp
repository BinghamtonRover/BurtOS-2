#pragma once

#include <vector>
#include <string_view>
#include <memory>
#include <array>
#include <functional>

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/udp.hpp>

#include <rsdp_functions.pb.h>

namespace rsdp {

/// Standard IPv4 Multicast address for RSDP
constexpr std::string_view V4_ADDRESS = "239.255.11.2";

/// Standard port for RSDP
constexpr uint16_t PORT = 22100;

/// ServiceProvider is for declaring a set of RSDP Services and advertising them
/// or listening to inquiries
///
/// This object is for communicating using the Rover Service Discovery Protocol
/// (RSDP) and doesn't actually provide or handle any services. It is only for
/// \a declaring that some named services are available.
/// See the README for RSDP for more info.
class ServiceProvider {
	public:

		/// Send RSDP Advertise messages
		///
		///	This method is synchronous. All network resources are allocated and
		/// cleaned before the call returns.
		void advertise_services() const;

		/// Enable listening for RSDP Inquire messages
		///
		/// Callers must provide the Boost IO Context for asynchronous events.
		/// Inquiries are handled continuously until stop_inquiries() is called.
		void handle_inquiries(boost::asio::io_context&);
		void stop_inquiries();


		/// Get the vector containing all service descriptions.
		///
		/// The vector is mutable, so services can be added and removed freely
		inline std::vector<rsdp::Service>& get_services() { return services; }

	private:

		void advertise(const boost::asio::ip::udp::endpoint&) const;
		void receive();

		std::vector<rsdp::Service> services;

		std::unique_ptr<boost::asio::ip::udp::socket> inquiry_listen_socket;
		std::unique_ptr<char[]> inquiry_read_buf;
		boost::asio::ip::udp::endpoint remote_sender;

		constexpr static std::size_t RD_SIZE = 1024;


};

/// Send service inquiries and receive advertisements
///
/// The ServiceInquirer can be used to cast an inquiry, which requests that all
/// service providers on the network advertise their services. Use the
/// set_advertisement_received_callback() method to receive the available
/// service descriptions.
class ServiceInquirer {
	public:

		/// Read RSDP Service Advertisements asynchronously
		void handle_advertisements(boost::asio::io_context&);

		/// Read RSDP Service Advertisements asynchronously and provide the receipt callback
		///
		/// The callback replaces the one set with set_advertisement_received_callback()
		void handle_advertisements(boost::asio::io_context&, const std::function<void(const rsdp::Service&)>& callback);
		void stop_advertisements();

		/// Set the function to be called when new services are discovered
		inline void set_advertisement_received_callback(const std::function<void(const rsdp::Service&)>& f) {
			new_service_callback = f;
		}
	
		void cast_inquiry() const;
	private:
		void receive();

		std::function<void(const rsdp::Service&)> new_service_callback;

		std::unique_ptr<boost::asio::ip::udp::socket> listen_socket;
		std::array<char, 1024> service_rd_buf;
		boost::asio::ip::udp::endpoint remote_sender;

};

} // end namespace RSDP
