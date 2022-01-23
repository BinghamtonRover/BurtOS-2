#pragma once

#include <vector>
#include <string>
#include <functional>

struct AxisAction {
	std::string name = UNBOUND_NAME;
	std::function<void(float)> callback;
	float final_value;

	// Compare actions based on their names (alphabetical order)
	inline bool operator<(const AxisAction& other) const {
		return name < other.name;
	}
	constexpr static const char* UNBOUND_NAME = "<unbound>";
};


// Two-sided scaling: joysticks usually don't center exactly at 0, so scale around a calibrated center
// There are two separate scales since one side will cover a larger range of values
struct AxisCalibration {
	float left_scale = 1.0F;
	float right_scale = 1.0F;
	float center = 0.0F;
	float dead_zone = 0.0F;
	float min = -1.0F;
	float max = 1.0F;
	void set_center(float);
	void set_dead_zone(float);
	void rescale();
	void restore_defaults();
};

// Helper class during calibration
struct AxisCalibrator : public AxisCalibration {
	inline bool active() const {
		return calibration_active;
	}
	private:
		friend class JoystickAxis;

		AxisAction displaced_action;
		bool calibration_active = false;
};

class JoystickAxis {
	public:

		constexpr static float AXIS_MIN = -1.0F;
		constexpr static float AXIS_MAX = 1.0F;
		constexpr static float AXIS_RANGE = AXIS_MAX - AXIS_MIN;

		inline const AxisAction& action() const {
			return _action;
		}
		inline const AxisCalibration& calibration() const {
			return cal;
		}
		inline float value() const { 
			return last_value_translated;
		}
		inline float raw_value() const {
			return last_value_raw;
		}

		// Scale the raw controller input to the calibrated range for this joystick
		float translate(float) const;
		void set_action(const AxisAction& act);
		void update(float x);
		void unbind();

		/*
			Calibration process:
			- When starting calibration, the new values are written to an AxisCalibrator
				owned by the caller. The old (current) config is not changed.
				The displaced action is saved privately in the Calibrator
			- When calibration ends (either by calling end or set_action), the displaced
				action is restored. The old config is still not overwritten
			- The caller will presumably call apply_calibration() if desired
		*/

		void apply_calibration(const AxisCalibration&);	
		// Begin calibrating and save the new config in an AxisCalibrator
		// AxisCalibrator must be valid until calling end_calibration()
		void start_calibration(AxisCalibrator& save_to);
		// Finish calibrating and restore the displaced action
		// Does not apply the new config: Must call apply_calibration()
		void end_calibration(AxisCalibrator& to_end);

	private:
		
		AxisCalibration cal;

		float last_value_raw;
		float last_value_translated;

		AxisAction _action;
};

class Controller {
	private:
		std::string name;
		std::vector<JoystickAxis> _axes;
		int _joystick_id = -1;
		bool _present = false;
	public:

		// Read hardware information for this device
		void update_device();

		// Read axis positions and call event handlers
		void update_axes();

		const char* device_name() const;
	
		inline void set_joystick_id(int id) {
			_joystick_id = id;
		}
		inline int joystick_id() const {
			return _joystick_id;
		}
		inline bool present() const {
			return _present;
		}
		inline decltype(_axes)& axes() {
			return _axes;
		}
};