#include <rover_service_discovery.hpp>

#include <array>
#include <cstring>

#include <boost/asio.hpp>
#include <boost/array.hpp>

void rsdp::ServiceProvider::advertise(const boost::asio::ip::udp::endpoint& ep) const {
	using namespace boost::asio;

	io_context ctx;
	ip::udp::socket socket(ctx, ep.protocol());

	std::string serialized_svc_msg;

	rsdp::MsgPacket rsdp_message;
	rsdp_message.set_rsdp_function(rsdp::MsgPacket_Function_Advertise);

	// Send one packet for each service
	for (auto& svc : services) {

		rsdp_message.set_data(svc.SerializeAsString());

		if (rsdp_message.SerializeToString(&serialized_svc_msg)) {

			socket.send_to(buffer(serialized_svc_msg.data(), serialized_svc_msg.size()), ep);

		}

	}
}

void rsdp::ServiceProvider::advertise_services() const {
	using namespace boost::asio;
	advertise(ip::udp::endpoint(ip::address_v4::from_string(std::string(V4_ADDRESS)), PORT));
}

void rsdp::ServiceProvider::handle_inquiries(boost::asio::io_context& ctx) {
	using namespace boost::asio;

	inquiry_listen_socket.reset(new ip::udp::socket(ctx));
	inquiry_read_buf.reset(new char[RD_SIZE]);

	auto ep = ip::udp::endpoint(ip::address_v4::from_string("0.0.0.0"), PORT);

	inquiry_listen_socket->open(ep.protocol());
	inquiry_listen_socket->set_option(ip::udp::socket::reuse_address(true));
	inquiry_listen_socket->bind(ep);

	inquiry_listen_socket->set_option(ip::multicast::join_group(ip::address_v4::from_string(std::string(V4_ADDRESS))));
	receive();

}

void rsdp::ServiceProvider::receive() {
	inquiry_listen_socket->async_receive_from(boost::asio::buffer(inquiry_read_buf.get(), RD_SIZE), remote_sender,
		[this](boost::system::error_code ec, std::size_t size) {
		
		if (!ec) {

			rsdp::MsgPacket msg_received;
			if (msg_received.ParseFromArray(inquiry_read_buf.get(), size) &&
				msg_received.rsdp_function() == rsdp::MsgPacket_Function_Inquire) {

				advertise_services();

			}

		}
		
		receive();
	});
}

void rsdp::ServiceProvider::stop_inquiries() {
	inquiry_listen_socket.release();
}

void rsdp::ServiceInquirer::cast_inquiry() const {
	using namespace boost::asio;

	auto ep = ip::udp::endpoint(ip::address_v4::from_string(std::string(V4_ADDRESS)), PORT);

	io_context ctx;
	ip::udp::socket socket(ctx, ep.protocol());

	rsdp::MsgPacket rsdp_message;
	rsdp_message.set_rsdp_function(rsdp::MsgPacket_Function_Inquire);

	std::string serialized_svc_msg = rsdp_message.SerializeAsString();
	socket.send_to(buffer(serialized_svc_msg.data(), serialized_svc_msg.size()), ep);

}

void rsdp::ServiceInquirer::handle_advertisements(boost::asio::io_context& ctx) {
	using namespace boost::asio;

	listen_socket.reset(new ip::udp::socket(ctx));

	auto ep = ip::udp::endpoint(ip::address_v4::from_string("0.0.0.0"), PORT);

	listen_socket->open(ep.protocol());
	listen_socket->set_option(ip::udp::socket::reuse_address(true));
	listen_socket->bind(ep);

	listen_socket->set_option(ip::multicast::join_group(ip::address_v4::from_string(std::string(V4_ADDRESS))));
	receive();	
}

void rsdp::ServiceInquirer::handle_advertisements(boost::asio::io_context& ctx, const std::function<void(const rsdp::Service&)>& callback) {
	new_service_callback = callback;
	handle_advertisements(ctx);
}

void rsdp::ServiceInquirer::stop_advertisements() {
	listen_socket.release();
}

void rsdp::ServiceInquirer::receive() {
	listen_socket->async_receive_from(boost::asio::buffer(service_rd_buf), remote_sender,
		[this](boost::system::error_code ec, std::size_t size) {

		if (!ec) {

			rsdp::MsgPacket msg_received;
			if (msg_received.ParseFromArray(service_rd_buf.data(), size) &&
				msg_received.rsdp_function() == rsdp::MsgPacket_Function_Advertise) {

				// Deserialize the service description and notify of its receipt
				rsdp::Service svc_received;
				if (svc_received.ParseFromString(msg_received.data())) {
					if (new_service_callback) new_service_callback(svc_received);
				}

			}

		}
		
		receive();
	});
}