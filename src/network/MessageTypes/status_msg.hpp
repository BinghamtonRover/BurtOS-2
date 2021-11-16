#ifndef STATUSMSG_H
#define STATUSMSG_H

//probably need an include for network for serialization methods 

//Holds data about the status of the rover at any given time
struct StatusMessage{
	static const auto TYPE = MessageType::STATUS;

	//From old Location message
	enum class FixStatus : uint8_t {
		NONE,
		STABILIZING,
		FIXED
	};

	FixStatus fix_status;
	float latitude;
	float longitude;

	//From old Tick message
	float ticks_per_second

	//Serialize and deserialize are guesses from old messages, have to check how these are implemented in old system
	void serialize(Buffer* buffer){
		network::serialize(buffer, static_cast<uint8_t>(this->fix_status));
		network::serialize(buffer, this->latitude);
		network::serialize(buffer, this->longitude);
		network::serialize(buffer, this->ticks_per_second);
	}

	void deserialize(Buffer* buffer){
		network::serialize(buffer, &(static_cast<uint8_t>(this->fix_status)));
		network::serialize(buffer, &(this->latitude));
		network::serialize(buffer, &(this->longitude));
		network::serialize(buffer, &(this->ticks_per_second));
	}
};

#endif