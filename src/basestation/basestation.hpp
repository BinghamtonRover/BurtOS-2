#pragma once

#include <vector>
#include <mutex>
#include <functional>

#include <rover_lua.hpp>
#include <basestation_screen.hpp>
#include <controls/controller_manager.hpp>
#include "../network/stream.hpp"
#include <iostream>


/*
	Container class for the main instance of the base station
*/
class Basestation {
	public:
		Basestation();
		~Basestation();

		Basestation(const Basestation&) = delete;
		Basestation(Basestation&&) = delete;

		inline ControllerManager& controller_manager() {
			return controller_mgr;
		}

		void mainloop();

		void add_screen(BasestationScreen*);

		// Return the focused screen. Guaranteed to return a valid screen.
		// If there are no valid screens, throws a runtime error
		BasestationScreen* get_focused_screen() const;
		
		inline const std::vector<BasestationScreen*>& get_screens() const {
			return screens;
		}

		void schedule(const std::function<void(Basestation&)>& callback);
		
		inline static Basestation& get() {
			return *main_instance;
		}

		// Schedule an event to run in the main thread (async to the calling thread)
		// Short for Basestation::get().schedule(..)
		inline static void async(const std::function<void(Basestation&)>& callback) {
			get().schedule(callback);
		}

		struct lua_basestation_lib {
			static const struct luaL_Reg lib[];
			static int shutdown(lua_State*);
			static int new_screen(lua_State*);

			static void open(lua_State*);
		};

        inline void set_video_callback(std::function<void(int stream, net::Frame& frame)> handler) {
            std::cout << "In callback setter" << std::endl;
            video_feed_receiver.on_frame_received(handler);
        }

	private:
		ControllerManager controller_mgr;
		boost::asio::io_context main_thread_ctx;
		net::StreamReceiver video_feed_receiver;


		std::vector<BasestationScreen*> screens;

		std::mutex schedule_lock;
		std::vector<std::function<void(Basestation&)>> async_callbacks;

		bool continue_operating = true;

		static Basestation* main_instance;

};
