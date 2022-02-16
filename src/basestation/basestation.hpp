#pragma once

#include <vector>
#include <mutex>
#include <functional>

#include <boost/property_tree/ptree.hpp>

#include <network.hpp>

#include <rover_lua.hpp>
#include <basestation_screen.hpp>
#include <controls/controller_manager.hpp>
#include <controls/drive_input.hpp>

/*
	Container class for the main instance of the base station
*/
class Basestation {
	public:
		Basestation();
		Basestation(const boost::property_tree::ptree& config);
		~Basestation();

		Basestation(const Basestation&) = delete;
		Basestation(Basestation&&) = delete;

		inline ControllerManager& controller_manager() {
			return controller_mgr;
		}

		void mainloop();

		void add_screen(BasestationScreen*);
		void write_settings(boost::property_tree::ptree&);

		// Return the focused screen. Guaranteed to return a valid screen.
		// If there are no valid screens, throws a runtime error
		BasestationScreen* get_focused_screen() const;
		
		inline const std::vector<BasestationScreen*>& get_screens() const {
			return screens;
		}
		inline net::MessageSender& subsystem_sender() {
			return m_subsystem_sender;
		}
		inline net::MessageReceiver& subsystem_feed() {
			return m_subsystem_feed;
		}
		inline DriveInput& remote_drive() {
			return m_remote_drive;
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
			static int open_module(lua_State*);
			static int set_throttle(lua_State*);

			static void open(lua_State*);
		};

	private:
		boost::asio::io_context main_thread_ctx;
		net::MessageSender m_subsystem_sender;
		net::MessageReceiver m_subsystem_feed;
		DriveInput m_remote_drive;

		event::Handler log_feed_error;
		event::Handler log_sender_error;

		ControllerManager controller_mgr;

		std::vector<BasestationScreen*> screens;

		std::mutex schedule_lock;
		std::vector<std::function<void(Basestation&)>> async_callbacks;

		bool continue_operating = true;

		static Basestation* main_instance;

};
