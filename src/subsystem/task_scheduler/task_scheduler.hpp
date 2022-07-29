#pragma once

#include <functional>
#include <chrono>
#include <queue>

// The scheduler and tasks are closely related and almost function as one unit
class TaskScheduler;
class Task;

/// Manage and dispatch synchronous tasks in one thread
///
/// This is ideal for procedures that must be scheduled on a regular basis but
/// need to run synchronously in a single thread. Using a scheduler allows the
/// process to sleep when nothing needs to run.
class TaskScheduler {
	friend class Task;

	public:
		/// Add a new task to this scheduler
		///
		/// The scheduler saves a reference to the task, but the Task container
		/// is safe. The Task will be automatically removed from the scheduler
		/// at the end of its lifetime.
		void add_task(Task&);

		/// Run any tasks which are ready for dispatch and return
		void poll();

		inline bool empty() const {
			return tasks.empty();
		}

		std::chrono::steady_clock::time_point get_next_dispatch_time() const;

	private:

		// Wraps the task pointer to provide a comparator based on what is being
		// pointed to rather than the address. This also allows invalidating
		// entries in the tasks queue since elements can't be removed
		struct TaskEntry {
			TaskEntry(Task&);
			TaskEntry(const TaskEntry&) = default;
			TaskEntry(TaskEntry&&);
			~TaskEntry();
			Task* ptr;
			bool entry_active;

			bool operator<(const TaskEntry& other) const;
			bool operator>(const TaskEntry& other) const;
			Task& operator*() const;

			TaskEntry& operator=(TaskEntry&) = default;
			TaskEntry& operator=(const TaskEntry&) = default;
		};
		// The soonest tasks will be at the top
		std::priority_queue<TaskEntry, std::vector<TaskEntry>, std::greater<TaskEntry>> tasks;

};

/// Control structure for an individual task for the TaskScheduler
class Task {
	friend class TaskScheduler;

	public:
		Task(const std::function<void(Task&)>& callback);
		~Task();

		void set_next_occurrence(const std::chrono::steady_clock::time_point& p);
		void cancel();

		inline const std::chrono::steady_clock::time_point& get_next_occurrence() const {
			return next_event;
		}

		inline bool operator<(const Task& other) const {
			return next_event < other.next_event;
		}
		inline bool operator>(const Task& other) const {
			return next_event > other.next_event;
		}

	private:
		std::chrono::steady_clock::time_point next_event = {};
		TaskScheduler* parent_scheduler = nullptr;
		TaskScheduler::TaskEntry* latest_entry = nullptr;

		std::function<void(Task&)> event_callback;

};