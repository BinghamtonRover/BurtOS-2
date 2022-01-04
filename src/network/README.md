# Rover Network Library
This is a message-passing library that uses UDP, which performs better than TCP on the rover system since it does not require establishing two-way connections. The library uses Google Protocol Buffers for serializing and deserializing messages.
## Usage
View the provided examples in `apps` for additional help. They are not built by default: set `BUILD_NETWORK_APPS=ON` with CMake to enable. This can be done without regenerating all build files by running `cmake -B build -DBUILD_NETWORK_APPS=ON` in the repo root (assuming BurtOS-2/build is the desired build directory). The executables are placed in `build/bin`.
### Using in a new program
#### Defining message types
Messages must be defined using [Google Protocol Buffers](https://developers.google.com/protocol-buffers/docs/cpptutorial). They can be defined wherever you choose. Use as many or few `.proto` files as you like. CMake can automatically run the protobuf compiler if you add it to your build files (see CMake help section). There should be other examples to follow to help with Protobuf. Most rover members will not need to create new `.proto` files and will add to existing ones.

Each message type must be registered at runtime. Protobuf does not provide any means of distinguishing message types once they are serialized, so the definition/registration process generates a unique ID for each type.

Each protobuf message turns into its own class type. The network library requires defining a wrapper class that supports type differentiation. `message.hpp` provides a macro to do this easily. In a file of your choice (typically a header file), use the macro to define wrapper classes:
```c++
// File: my_project_messages.hpp
#pragma once

#include <messages.hpp>
#include <my_protobuf_example_messages.pb.h>

//Format: DEFINE_MESSAGE_TYPE(NameOfNewWrapperClass, package_name::ProtobufMessageTypeName)

//Examples
DEFINE_MESSAGE_TYPE(HelloMessage, example::Hello)
DEFINE_MESSAGE_TYPE(GoodbyeMessage, example::Goodbye)

// You can wrap them in namespaces if you prefer
namespace example {
	DEFINE_MESSAGE_TYPE(Hello1Message, example::Hello1)

	// Remember that classes can't have the same name. Don't do this:
	// DEFINE_MESSAGE_TYPE(Hello2, example::Hello2)
}

// If you are curious, here is how each macro expands:
// DEFINE_MESSAGE_TYPE(Hello3Message, example::Hello3)
// expands to
struct Hello3Message : public msg::Message {
	example::Hello3 data;
	inline static msg::type_t TYPE;
	Hello3Message() {
		data_p = &data;
		type = Hello3Message::TYPE;
	}
};

```

#### Registering message types
In the example, see that each message type's struct has a static member variable for TYPE. TYPE has no default value as it is given during registration. Somewhere near the beginning of your program, register each type. This should be done before constructing any `MessageReceiver`s, but doesn't have to be.
```c++
// A continuation from the example above
#include <messages.hpp>
#include "my_project_messags.hpp"

void register_my_messages() {
	msg::register_message_type<HelloMessage>();
	msg::register_message_type<GoodbyeMessage>();
	msg::register_message_type<example::Hello1Message>();
	msg::register_message_type<Hello3Message>();
}

int main() {
	// Okay, it doesn't have to be the first thing in your program.
	// But it is imperative you do it before registering any handlers or sending any messages
	// Registering handlers or sending messages before registering the message type is undefined behavior.
	register_my_messages();
}

```

#### Sending messages
Use the `net::RemoteDevice` object for sending messages to a specific device. This documentation is not a [Boost.Asio](https://www.boost.org/doc/libs/1_76_0/doc/html/boost_asio/overview.html) tutorial substitute.
```c++
boost::asio::io_context ctx;
net::RemoteDevice rover_subsystem(net::Destination(boost::asio::ip::address::from_string("192.168.1.10"), 12308), ctx); // No, the ip/port are not accurate to our system

HelloMessage outgoing_message;
// Use the <MessageWrapperType>.data member to access the original protobuf class
outgoing_message.data.set_garbage_value(10);

// Send is asynchronous: it serializes the message and returns. Boost handles actually sending messages
rover_subsystem.send_message(outgoing_message);

ctx.run();
```

#### Receiving messages
Use `net::MessageReceiver` for receiving messages. The handler uses `std::function`, so handlers may use lambdas, `std::bind`, or regular function pointers (with reinterpret_cast).
```c++
boost::asio::io_context ctx;
net::MessageReceiver onboard_rover_receiver(12308, ctx);

onboard_rover_receiver.register_handler<HelloMessage>([] (const uint8_t buffer[], std::size_t len) {
	// For deserializing, we do not need the wrapper. You can use the original protobuf class
	// You could also use the wrapper, just using the "data" member to get the protobuf class
	example::Hello message;
	if (message.ParseFromArray(buffer, len)) {
		// Valid message received
		std::cout << "Received a hello with garbage value: " << message.garbage_value() << '\n';
	}
	
});
```


#### CMake files
Add `network` to the include directories and link libraries for your target. `network` will already include Boost and Protobuf. Example following from the scheme above:
```CMake
protobuf_generate_cpp(PROTO_SRC PROTO_HDR my_protobuf_examples_messages.proto)

add_executable(example_program ${PROTO_HDR} ${PROTO_SRC} your_other_source_files...)
target_include_directories(example_program PUBLIC network ${CMAKE_CURRENT_BINARY_DIR})	# current binary directory is necessary to find `*.pb.h` files
target_link_libraries(example_program PUBLIC network)
```
## Planned features
* Error recovery options to prevent having to restart the program or delete/reconstruct the object on errors.
* Separate messages.hpp/cpp from network since they function as a universal message passing utility.
