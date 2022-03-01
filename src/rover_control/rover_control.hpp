#include <network.hpp>
#include <rover_system_messages.hpp>
#include <events.hpp>

namespace rc {

class Drive {
	public:
		Drive(net::MessageSender& ms);

		void set_interval(int milliseconds);
		int get_interval();
		void poll_events();
		bool update_ready();
		void send_update();

		void set_drive_mode(::drive::DriveMode_Mode mode);
		void halt();

		void set_speed(float speed);
		void set_angle(float angle);
		void set_movement(float speed, float angle);

		void register_listen_handlers(net::MessageReceiver& m);

		float get_actual_left_speed();
		float get_actual_right_speed();
		::drive::DriveMode_Mode get_actual_drive_mode();
		inline const std::chrono::steady_clock::time_point& get_last_update_received() const { return last_update_received; }

		// public events
		event::Emitter<drive::DriveMode_Mode> EVENT_DRIVEMODE;
		event::Emitter<float, float> EVENT_SPEED;

	private:
		int interval = 100;
		net::MessageSender& sender;
		drive_msg::Velocity movement_message;

		std::chrono::steady_clock::time_point last_message_sent{};

		float actual_left_speed = 0.0F;
		float actual_right_speed = 0.0F;
		::drive::DriveMode_Mode actual_drive_mode = drive::DriveMode_Mode::DriveMode_Mode_NEUTRAL;
		drive::DriveMode_Mode requested_drive_mode = drive::DriveMode_Mode::DriveMode_Mode_NEUTRAL;
		
		std::chrono::steady_clock::time_point last_update_received{};
};

class Sensor {
	public:
		Sensor(net::MessageSender& ms);

		void register_listen_handlers(net::MessageReceiver& m);

		float get_battery_voltage();
		float get_battery_current();
		float get_v12_supply_voltage();
		float get_v12_supply_current();
		float get_v12_supply_temperature();
		float get_v5_supply_voltage();
		float get_v5_supply_current();
		float get_v5_supply_temperature();
		float get_odrive0_current();
		float get_odrive1_current();
		float get_odrive2_current();

		inline const std::chrono::steady_clock::time_point& get_last_update_received() const { return last_update_received; }

		event::Emitter<float, float> EVENT_BATTERY_SENSOR;
		event::Emitter<float, float, float> EVENT_POWERSUPPLY12V_SENSOR;
		event::Emitter<float, float, float> EVENT_POWERSUPPLY5V_SENSOR;
		event::Emitter<float, float, float> EVENT_ODRIVE_SENSOR;

	private:
		net::MessageSender& sender;

		std::chrono::steady_clock::time_point last_update_received{};

		float battery_voltage = 0.0F;
		float battery_current = 0.0F;
		float v12_supply_voltage = 0.0F;
		float v12_supply_current = 0.0F;
		float v12_supply_temperature = 0.0F;
		float v5_supply_voltage = 0.0F;
		float v5_supply_current = 0.0F;
		float v5_supply_temperature = 0.0F;
		float odrive0_current = 0.0F;
		float odrive1_current = 0.0F;
		float odrive2_current = 0.0F;
};

} // namespace rc
