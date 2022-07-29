# Task Scheduler Manual
## Overview
The task scheduler is for running multiple tasks in the same thread in a controlled and predictable manner. Simply put, with this task scheduler, functions can schedule a time to be called again. When no functions are ready to dispatch, the program can sleep instead of busy wait.

Tasks are vaguely inspired by FreeRTOS Tasks and Boost.Fiber but are much, much simpler. A task is only a function callback that may be dispatched synchronously when calling `TaskScheduler::poll()`.

## Problems Addressed

Formerly, the Main Event Loop in the subsystem computer checked for work by polling. A polling-style loop basically looks like:
  1. Check if any network packets have arrived
  2. Check if the drive controller should update the motors
  3. Check if a CAN heartbeat should be sent
  4. Check if a system status update should be sent
  5. Repeat (no delay)

On most iterations, this program does no work: it wastes nearly 100% of its CPU time *checking* for work. This method scales poorly. On small microcontrollers, busy-waiting is standard. On Raspberry Pi, spinning hogs valuable CPU time and reduces the system's overall capacity. Busy-waiting on a PC also wastes a ton of electricity.

## Usage
### Including
Link to the CMake target `task_scheduler`. The target has no dependencies other than the C++ STL.

In source files, include the header `task_scheduler.hpp`
### Code
Create a `TaskScheduler()` to manage tasks.

For each task, create a `Task(const std::function<void(Task&)>& callback)` for desired tasks. The `Task&` is a reference to the task itself. Providing a null callback and then scheduling the task will result in undefined behavior.
```C++
// Prints "Fizz" every 3 seconds
Task fizz([] (Task& self) {
	// Tasks must reschedule themselves
	self.set_next_occurrence(std::chrono::steady_clock::now() + std::chrono::seconds(3));
	std::cout << "Fizz\n";
}));
```
After creating the task, add it to the scheduler with `TaskScheduler::add_task(Task&)`.
```C++
sched.add_task(fizz);
```
Tasks must be allocated as long as they are scheduled. If a scheduled task is destructed, it will be canceled safely and automatically.

To dispatch all tasks which are ready, call `TaskScheduler::poll()`. This can be combined with `TaskScheduler::get_next_dispatch_time()` to sleep during inactivity.
```C++
while (!sched.empty()) {
	std::this_thread::sleep_until(sched.get_next_dispatch_time());
	sched.poll();
}
```
#### Complete Example
```C++
#include <task_scheduler.hpp>
#include <iostream>
#include <thread>

TaskScheduler sched;

void print_buzz_forever(Task& self) {
	self.set_next_occurrence(std::chrono::steady_clock::now() + std::chrono::seconds(5));
	std::cout << "Buzz\n";
}

int main() {
	// Prints "Fizz" every 3 seconds
	Task fizz([] (Task& self) {
		// Tasks must reschedule themselves
		self.set_next_occurrence(std::chrono::steady_clock::now() + std::chrono::seconds(3));
		std::cout << "Fizz\n";
	}));
	sched.add_task(fizz);

	// Another way to add a task by binding a function
	Task buzz_task(print_buzz_forever);
	sched.add_task(buzz_task);

	while (!sched.empty()) {
		std::this_thread::sleep_until(sched.get_next_dispatch_time());
		sched.poll();
	}
	return 0;
}
```