#ifndef LOGMSG_H
#define LOGMSG_H

//Ported from old netwrok
struct LogMessage {
	static const auto TYPE = MessageType::LOG;

	uint8_t size;
	uint8_t* log_string;

	void serialize(Buffer* buffer) {
		network::serialize(buffer, this->size);
		network::serialize(buffer, this->log_string, this->size);
	}

	void deserialize(Buffer* buffer) {
		network::deserialize(buffer, &(this->size));
		network::deserialize(buffer, this->log_string, this->size);
	}
};

#endif