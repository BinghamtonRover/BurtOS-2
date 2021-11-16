#ifndef CAMERAMSG_H
#define CAMERAMSG_H

//Most likely need network import

//ported from old network
struct CameraMessage {
	static const auto TYPE = MessageType::CAMERA;

	static const int HEADER_SIZE = 7;

	uint8_t stream_index;
	uint16_t frame_index;
	uint8_t section_index;
	uint8_t section_count;
	uint16_t size;

	uint8_t* data;

	void serialize(Buffer* buffer) {
		network::serialize(buffer, this->stream_index);
		network::serialize(buffer, this->frame_index);
		network::serialize(buffer, this->section_index);
		network::serialize(buffer, this->section_count);
		network::serialize(buffer, this->size);
		network::serialize(buffer, this->data, this->size);
	}

	void deserialize(Buffer* buffer) {
		network::deserialize(buffer, &(this->stream_index));
		network::deserialize(buffer, &(this->frame_index));
		network::deserialize(buffer, &(this->section_index));
		network::deserialize(buffer, &(this->section_count));
		network::deserialize(buffer, &(this->size));
		network::deserialize(buffer, this->data, this->size);
	}
};

#endif