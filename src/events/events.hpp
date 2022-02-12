/*
	Header-only (and extremely simple) event dispatch library
*/

#pragma once

#include <vector>
#include <functional>
#include <utility>

namespace event {

class Handler;

/*
	The event emitter can be used to distribute custom events to any number of listeners

	Listeners can subscribe with an event::Handler, which will automatically unsubscribe
	at the end of its lifetime.

	The template parameters are the types emitted with this event.
	
	Example:

	event::Emitter<int, int> expects handler functions that accepts two ints, like:

	void my_handler(int a, int b);

*/
template<typename... S>
class Emitter {
	public:

		// Notify listeners that this event occurred
		void operator()(S... args) {
			for (auto& listener_pair : listeners) {
				listener_pair.second(args...);
			}
		}

		typedef std::function<void(S...)> HandlerType;

	private:

		friend class Handler;

		// Add a handler to listeners and return a function that can be called to unsubscribe it
		// Returning this "deleter" means that handlers do not need to save the emitter and thus 
		// Handlers do not have to be template classes
		std::function<void()> subscribe_handler(Handler* h_ptr, const std::function<void(S...)>& h_f1) {
			auto it = std::find_if(listeners.begin(), listeners.end(), [h_ptr] (const std::pair<Handler*, std::function<void(S...)>>& h) {
				return h.first == h_ptr;
			});
			if (it != listeners.end()) {
				*it = std::pair(h_ptr, h_f1);
			} else {
				listeners.emplace_back(std::pair(h_ptr, h_f1));
			}

			return [this, h_ptr] () {
				remove_handler(h_ptr);
			};
		}

		void remove_handler(Handler* h_ptr) {
			auto it = std::find_if(listeners.begin(), listeners.end(), [h_ptr] (const std::pair<Handler*, std::function<void(S...)>>& h) {
				return h.first == h_ptr;
			});
			if (it != listeners.end()) {
				listeners.erase(it);
			}
		}

		std::vector< std::pair<Handler*, std::function<void(S...)>> > listeners;
};

/*
	Generic handler for any type of event
*/
class Handler {
	public:
		template<class EmitterT>
		void subscribe(EmitterT& ev, const typename EmitterT::HandlerType& handler) {
			deleter = ev.subscribe_handler(this, handler);
		}

		void unsubscribe() {
			if (deleter)
				deleter();
		}

		Handler() {}

		template<class EmitterT>
		Handler(EmitterT& ev, const typename EmitterT::HandlerType& handler) {
			subscribe(ev, handler);
		}

		~Handler() {
			unsubscribe();
		}

	private:
		// To avoid requiring the handler to be a template class, 
		// the emitter will provide an unsubscribe function
		std::function<void()> deleter;
};

}	// namespace event
