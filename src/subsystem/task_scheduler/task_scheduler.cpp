#include <task_scheduler.hpp>

Task::Task(const std::function<void(Task&)>& callback) : event_callback(callback) {}

std::chrono::steady_clock::time_point TaskScheduler::get_next_dispatch_time() const {
	if (tasks.empty())
		return std::chrono::steady_clock::time_point{};
	else
		return (*tasks.top()).get_next_occurrence();
} 

bool TaskScheduler::TaskEntry::operator<(const TaskEntry& other) const {
	return ptr->next_event < other.ptr->next_event;
}
bool TaskScheduler::TaskEntry::operator>(const TaskEntry& other) const {
	return ptr->next_event > other.ptr->next_event;
}
Task& TaskScheduler::TaskEntry::operator*() const {
	return *ptr;
}

TaskScheduler::TaskEntry::TaskEntry(Task& t) :
	ptr(&t),
	entry_active(true) {

	// Tasks cannot be scheduled multiple times at once: invalidate any previous entries
	if (t.latest_entry != nullptr) {
		t.latest_entry->ptr = nullptr;
		t.latest_entry->entry_active = false;
	}
	t.latest_entry = this;

}

// Tasks point back to their task entry, so when moving an entry, update this pointer
// to the new location
TaskScheduler::TaskEntry::TaskEntry(TaskEntry&& old) :
	ptr(old.ptr),
	entry_active(old.entry_active) {

	old.ptr = nullptr;
	old.entry_active = false;

	ptr->latest_entry = this;

}

// When destructing an entry, update the task's entry pointer to reflect that it no longer
// has an entry with the scheduler. Note that this entry may not point to a task, and if it
// does point to a task, it may have been rescheduled with another entry
TaskScheduler::TaskEntry::~TaskEntry() {
	if (ptr != nullptr && ptr->latest_entry == this) {
		ptr->latest_entry = nullptr;
	}
}

void Task::cancel() {
	if (latest_entry) {
		latest_entry->entry_active = false;
	}
}

void Task::set_next_occurrence(const std::chrono::steady_clock::time_point& time_pt) {
	// If this task is already scheduled, invalidate the latest scheduler entry
	if (latest_entry) {
		latest_entry->entry_active = false;
	}

	next_event = time_pt;
	parent_scheduler->add_task(*this);
}

void TaskScheduler::add_task(Task& task) {
	tasks.emplace(TaskEntry(task));
	task.parent_scheduler = this;
}

void TaskScheduler::poll() {
	// While the soonest task is ready for dispatch, run the event callback
	// Additionally, pop any inactive tasks
	while (!tasks.empty() && (!tasks.top().entry_active || (*tasks.top()).get_next_occurrence() <= std::chrono::steady_clock::now())) {
		// Entries should be de-queued before dispatch, so make a copy of the entry information
		TaskEntry current(tasks.top());
		tasks.pop();
		
		if (current.entry_active) {
			Task& c = *current;
			c.event_callback(c);
		}

	}
}