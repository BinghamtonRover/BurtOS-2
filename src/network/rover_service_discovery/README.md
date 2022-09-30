# Rover Service Discovery Protocol (RSDP)
## Overview
Rover Service Discovery Protocol (RSDP) is a method for automatically discovering network devices which provide services. It allows the base station to find the rover services (subsystem/video) without preconfigured knowledge of their IP addresses and ports. RSDP is inspired by Simple Service Discovery Protocol (SSDP) but is extremely minimal.

## Definitions and Specification
### General Rules
* RSDP shall use IPv4 multicast address 239.255.11.2 and port 22100
* RSDP messages are comprised of serialized Protobuf messages
    * Each message has an enumeration to select the function and an array of bytes comprising the function data.
	* The function may use any format for data, but using another serialized Protobuf message is suggested.
* Only one message may be sent per UDP packet
### Functions
* The `inquire` function is used when a device wants to know what services are available. Inquiries require no extra data. Service providers should listen for inquiries and `advertise` in response.
* The `advertise` function is used when a service provider wants to notify clients that some service is available. Advertisements require the extra data to be populated with a serialized `rsdp::Service` message.
    * `rsdp::Service` fields:
	    * `service_name`: use to identify the type of service
		* `provider_hostname`: the hostname or any name which identifies the specific host
		* `provider_ip`: the IP address at which the service is available
		* `provider_port`: the port at which the service is available

## Library
The C++ library can be used by linking to the CMake target `rover_service_discovery` and including `<rover_service_discovery.hpp>`. The CMake target requires Protobuf and Boost (header-only libraries) to build. Docs are provided in the header file.

The protocol depends only on the Protobuf format, so this library may be re-written in another language rather quickly.

### C++ Example
This example makes a provider which will advertise a service, *Hello Service*, in response to inquiries. An inquirer casts an inquiry and shows the response.

```C++
#include <iostream>
#include <rover_service_discovery.hpp>

boost::asio::io_context ctx;
rsdp::ServiceProvider prov;
rsdp::ServiceInquirer enq;

int main() {

	rsdp::Service hello;

	// Service names can be anything, but if they aren't unique, it defeats
	// the purpose.
	hello.set_service_name("Hello Service");

	// Hostname may be set to any string which helps users to identify
	// what system this is in case multiple hosts provide the same service
	hello.set_provider_hostname("my-computer");

	// The IP is string based. This library does no validation whatsoever,
	// so use it for something else if you really want to
	hello.set_provider_ip("127.0.0.1");

	// Same with port, except it is a 32-bit unsigned integer.
	hello.set_provider_port(16000);

	// The services vector may be changed at any time
	prov.get_services().push_back(hello);

	// The provider will echo back the hello service in response to inquiries
	prov.handle_inquiries(ctx);

	// Our "print service" lambda will be called when a service is advertised
	enq.handle_advertisements(ctx, [](const rsdp::Service& new_service) {
		std::cout << "Found a service:"
			<< "\n\tName: " << new_service.service_name()
			<< "\n\tProvider: {host: " << new_service.provider_hostname() << ", addr: "
			<< new_service.provider_ip() << ":" << new_service.provider_port() << "}\n";
	});

	enq.cast_inquiry();

	// Wait to see the results
	ctx.run_for(std::chrono::duration<int, std::milli>(1000));

	prov.stop_inquiries();
	enq.stop_advertisements();

	return 0;

}
```

