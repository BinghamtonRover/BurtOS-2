# Controls
Utilities for processing human input devices and binding them to actions. Currently, this only supports controller/gamepad axes but will be extended to process gamepad buttons and keyboard keys as well.
## Controller Input
This library offers flexible C++ wrappers to the [GLFW Joystick Input](https://www.glfw.org/docs/3.3/input_guide.html#joystick) interface. GLFW and Controls use somewhat different terminology. Where different, the Controls name attemps to clarify a potentially misleading name. Controls Terminology:
* Controller: One specific input device. Controllers will have some number of buttons and axes on them. The GLFW name is Joystick.
* Axis: One variable axis on a controller. An axis has an input in the range [-1.0, 1.0]. GLFW uses the same name, but does not provide the extensive calibration and action binding options proivded by Controls.
* Gamepad: A specific type of Controller with a hardware layout compatible with Microsoft Xbox controllers ([GLFW source](https://www.glfw.org/docs/3.3/input_guide.html#gamepad)). Gamepad axes are recognizable by function (Left Joystick X, Right Trigger, etc.) rather than an uninformative, platform-specific index. Mappings are provided by the crowd-sourced [SDL_GameControllerDB](https://github.com/gabomdq/SDL_GameControllerDB).

### Usage
#### ControllerManager
##### Program flow
Use a `class ControllerManager` for Controls to maintain all GLFW controller events. Only one ControllerManager is allowed per program. A ControllerManager becomes the main controller manager and registers its GLFW callbacks upon calling `ControllerManager::init()`. Each main loop, call `ControllerManager::update_controls()` to process events and call action handlers.
```C++
#include "controls/controller_manager.hpp"

ControllerManager mgr;
int main() {
  // Initialize GLFW first:
  // ...
  // After GLFW initialization:
  
  mgr.init();
  
  // Program:
  while (true) {
    // Program stuff ...
    
    // Somewhere:
    mgr.update_controls();
    
    // More program stuff...
  }
  
  return 0;
}
```
##### Actions
A ControllerManager can optionally manage actions. Actions do not need to be registered with a manager, but this allows recalling them from a central location with a human-readable name. Each action has:
* Human-readable name
* Callback returning void and accepting a float
* Final/default value

When the action is bound to an axis, the callback will be called repeatedly with the calibrated input value. This value is guaranteed to be in range [-1.0, 1.0], excepting the final value: the final value is propagated when the controller disconnects or the action is otherwise unbound. Example:

```C++
// An action named "accelerate" that sets the rover forward speed.
// The final value is -1.0F so the speed is set to minimum upon disconnect
mgr.add_axis_action("accelerate", [&rover](float x) {
  // Translate from the axis range [-1.0, 1.0] to a percentage
  float speed_percent = JoystickAxis::axis_to_percent(x);
  rover.set_speed(speed_percent);
}, -1.0F);
```
Actions cannot be removed. An immutable array of action structs can be returned with `ControllerManager::actions()`, or an immutable action struct can be located by name with `ControllerManager::find_action(const std::string& name)`.

The purpose of registering actions in this manner is for loading axis bindings to and from config files. If there was no central location to add action types, loading and saving the actions would be difficult/impossible.
##### Controller
ControllerManager maintains a list of Controllers accessible via `ControllerManager::devices()`. Each device is a `class Controller`. Each controller has a joystick ID (`Controller::joystick_id()`) and set of `class JoystickAxis` (accessible via `Controller::axes()`).

If a gamepad mapping is available for this Controller, then `Controller::is_gamepad()` will return true. The gamepad axes can be retrieved with `Controller::get_gamepad_axis(...)`. You can use GLFW names (`mgr.get_gamepad_axis(GLFW_GAMEPAD_AXIS_LEFT_TRIGGER)`) or Controls wrapper names (`mgr.get_gamepad_axis(gamepad::left_trigger)`). These functions raise exceptions if the Controller is not a gamepad.

##### JoystickAxis
Each `JoystickAxis` has a calibration and an action. Actions need not be registered with a ControllerManager. Example:
```C++
Controller& ctrl_0 = mgr.devices().at(0);
if (ctrl_0.is_gamepad()) {
  JoystickAxis& target = ctrl_0.get_gamepad_axis(gamepad::right_trigger);
  // equivalent to:
  // target = ctrl_0.axes()[gamepad::right_trigger.IDX];
  // except get_gamepad_axis can raise exceptions
  target.set_action(mgr.find_action("accelerate"));
} else {
  JoystickAxis& target = ctrl_0.axes().at(5);
  target.set_action(mgr.find_action("accelerate"));
}
```
When calling `start_calibration()`, the axis should be in "resting" position. Then, move the axis around the full range. When calibration stops (`end_calibration()`), the "resting" position is compared to minimum and maximum. If they are not equal, then the center is set to resting position and an 8% deadzone is applied (for X/Y joysticks). If they are equal, then center is set to 0 and no deadzone is applied (for triggers).
## Unimplemented
* Controller buttons
* Keyboard input
