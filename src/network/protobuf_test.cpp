#include "hello.pb.h"
#include <iostream>
#include <string>

void g_serialize(google::protobuf::Message& msg, std::string& to) {
	msg.SerializeToString(&to);
}

int main() {
	rover_message::Hello h;
	h.set_hello_text_message("hello world");
	rover_message::Henlo he;
	he.set_henlo_text("henlo lizer. helllo you STINKY LIZARD. go eat a fly ugly");

	std::string serialized;
	std::string he_serialized;
	g_serialize(h, serialized);
	g_serialize(he, he_serialized);

	rover_message::Hello deserialized_message;
	rover_message::Henlo d_henlo;
	deserialized_message.ParseFromString(serialized);
	d_henlo.ParseFromString(he_serialized);
	std::cout << "After cycle, received message: " << deserialized_message.hello_text_message() << std::endl;
	std::cout << "Transmission took: " << serialized.size() << " bytes" << std::endl;
	std::cout << "After cycle, received message: " << d_henlo.henlo_text() << std::endl;
	std::cout << "Transmission took: " << he_serialized.size() << " bytes" << std::endl;
	return 0;
}
