#include "interactive_lua.hpp"

#include <iostream>
#include <stdexcept>

#define lua_initreadline(L)  ((void)L)
#define lua_readline(L,b,p) \
        ((void)L, fputs(p, stdout), fflush(stdout),  /* show prompt */ \
        fgets(b, LUA_MAXINPUT, stdin) != NULL)  /* get line */

#define lua_freeline(L,b)	{ (void)L; (void)b; }
#define lua_saveline(L,line)	{ (void)L; (void)line; }

#define EOFMARK		"<eof>"
#define marklen		(sizeof(EOFMARK)/sizeof(char) - 1)

int rover_lua::InteractivePrompt::Builtin::print(lua_State* L) {
	std::string line;
	int n = lua_gettop(L);  /* number of arguments */
	int i;
	for (i = 1; i <= n; i++) {  /* for each argument */
		size_t l;
		const char *s = luaL_tolstring(L, i, &l);  /* convert it to string */
		if (i > 1)  /* not the first element? */
		line.append("\t");  /* add a tab before it */
		line.append(s);  /* print it */
		lua_pop(L, 1);  /* pop result */
	}
	auto p = reinterpret_cast<InteractivePrompt*>(rover_lua::get_custom_ptr(L));
	p->_c_write_line(line.c_str());
	return 0;
}

void rover_lua::InteractivePrompt::Default::write_line(const char* line) {
	std::cout << line << "\n";
}

rover_lua::InteractivePrompt::InteractivePrompt() : _c_write_line(Default::write_line) {
	L = luaL_newstate();
	rover_lua::set_custom_ptr(L, this);

	luaL_openlibs(L);

	add_function("print", Builtin::print);
}

rover_lua::InteractivePrompt::~InteractivePrompt() {
		rover_lua::del_custom_ptr(L);
		lua_close(L);
}

void rover_lua::InteractivePrompt::stop() {
		std::unique_lock lock(execute_stream_lock);
		should_close = true;
		cv_line_available.notify_all();
}

void rover_lua::InteractivePrompt::add_function(const char* lua_name, int(*lua_c_function)(lua_State*)) {
	lua_pushcfunction(L, lua_c_function);
	lua_setglobal(L, lua_name);
}

void rover_lua::InteractivePrompt::load_library(const char* lua_name, const std::function<void(lua_State*)>& open_lib) {
	open_lib(L);
	lua_setglobal(L, lua_name);
}

int rover_lua::InteractivePrompt::report_error(int status) {
	if (status != LUA_OK) {
		const char *msg = lua_tostring(L, -1);
		_c_write_line(msg);
		lua_pop(L, 1);  /* remove message */
	}
	return status;
}

void rover_lua::InteractivePrompt::lua_print() {
	int n = lua_gettop(L);
	if (n > 0) {  /* any result to be printed? */
		luaL_checkstack(L, LUA_MINSTACK, "too many results to print");
		lua_getglobal(L, "print");
		lua_insert(L, 1);
		if (lua_pcall(L, n, 0, 0) != LUA_OK) {
			constexpr const char* error_msg = "error calling 'print' (%s)";
			constexpr std::size_t error_msg_len = strlen(error_msg);

			const char* error_desc = lua_tostring(L, -1);
			std::size_t error_desc_len = strlen(error_desc);

			char buf[error_desc_len + error_msg_len + 1];
			snprintf(buf, error_desc_len + error_msg_len, error_msg, error_msg);

			_c_write_line(buf);
		}
	}
}

 int rover_lua::InteractivePrompt::msghandler(lua_State* L) {
	const char *msg = lua_tostring(L, 1);
	if (msg == NULL) {  /* is error object not a string? */
		if (luaL_callmeta(L, 1, "__tostring") &&  /* does it have a metamethod */
			lua_type(L, -1) == LUA_TSTRING)  /* that produces a string? */
		return 1;  /* that is the message */
		else
		msg = lua_pushfstring(L, "(error object is a %s value)",
								luaL_typename(L, 1));
	}
	luaL_traceback(L, L, msg, 1);  /* append a standard traceback */
	return 1;  /* return the traceback */
}

int rover_lua::InteractivePrompt::docall(int narg, int nres) {
	int status;
	int base = lua_gettop(L) - narg;  /* function index */
	lua_pushcfunction(L, msghandler);  /* push message handler */
	lua_insert(L, base);  /* put it under function and args */
	status = lua_pcall(L, narg, nres, base);
	lua_remove(L, base);  /* remove message handler from the stack */
	return status;
}

int rover_lua::InteractivePrompt::incomplete(int status) {
	if (status == LUA_ERRSYNTAX) {
		size_t lmsg;
		const char *msg = lua_tolstring(L, -1, &lmsg);
		if (lmsg >= marklen && strcmp(msg + lmsg - marklen, EOFMARK) == 0) {
		lua_pop(L, 1);
		return 1;
		}
	}
	return 0;  /* else... */
}

int rover_lua::InteractivePrompt::multiline() {
	for (;;) {  /* repeat until gets a complete statement */
		size_t len;
		const char *line = lua_tolstring(L, 1, &len);  /* get what it has */
		int status = luaL_loadbuffer(L, line, len, "=console");  /* try it */
		if (!incomplete(status) || !pushline(0)) {
			lua_saveline(L, line);  /* keep history */
			return status;  /* cannot or should not try to add continuation line */
		}
		lua_pushliteral(L, "\n");  /* add newline... */
		lua_insert(L, -2);  /* ...between the two lines */
		lua_concat(L, 3);  /* join them */
	}
}

int rover_lua::InteractivePrompt::addreturn() {
	const char *line = lua_tostring(L, -1);  /* original line */
	const char *retline = lua_pushfstring(L, "return %s;", line);
	int status = luaL_loadbuffer(L, retline, strlen(retline), "=console");
	if (status == LUA_OK) {
		lua_remove(L, -2);  /* remove modified line */
		if (line[0] != '\0')  /* non empty? */
		lua_saveline(L, line);  /* keep history */
	}
	else
		lua_pop(L, 2);  /* pop result from 'luaL_loadbuffer' and modified line */
	return status;
}

const char* rover_lua::InteractivePrompt::get_prompt(int firstline) {
	if (lua_getglobal(L, firstline ? "_PROMPT" : "_PROMPT2") == LUA_TNIL)
		return (firstline ? "> " : ">> ");  /* use the default */
	else {  /* apply 'tostring' over the value */
		const char *p = luaL_tolstring(L, -1, NULL);
		lua_remove(L, -2);  /* remove original value */
		return p;
	}
}

void rover_lua::InteractivePrompt::execute_line(const std::string& line) {
	std::unique_lock lock(execute_stream_lock);
	line_in_unread.append(line);
	cv_line_available.notify_all();
}

void rover_lua::InteractivePrompt::get_input() {
	std::unique_lock lock(execute_stream_lock);
	if (line_in_unread.size() > 0) {
		line_in = line_in_unread;
		line_in_unread.clear();
		return;
	}
	while (line_in_unread.empty() && !should_close)
		cv_line_available.wait(lock);
	if (should_close) {
		throw SignalShutdown();
	} else {
		line_in = line_in_unread;
		line_in_unread.clear();
		// Ignore interrupts raised while execution was stopped
		if (should_interrupt) should_interrupt = false;
	}
}

int rover_lua::InteractivePrompt::pushline(int firstline) {
	size_t l;
	get_prompt(firstline);
	get_input();
	if (line_in.size() == 0)
		return 0;
	lua_pop(L, 1);  /* remove prompt */
	l = line_in.size();
	if (l > 0 && line_in[l-1] == '\n')  /* line ends with newline? */
		line_in[--l] = '\0';  /* remove it */
	if (firstline && line_in[0] == '=')  /* for compatibility with 5.2, ... */
		lua_pushfstring(L, "return %s", line_in.c_str() + 1);  /* change '=' to 'return' */
	else
		lua_pushlstring(L, line_in.c_str(), l);
	lua_freeline(L, line_in.c_str());
	return 1;
}

int rover_lua::InteractivePrompt::loadline() {
	int status;
	lua_settop(L, 0);
	if (!pushline(1))
		return -1;  /* no input */
	if ((status = addreturn()) != LUA_OK)  /* 'return ...' did not work? */
		status = multiline();  /* try as command, maybe with continuation lines */
	lua_remove(L, 1);  /* remove line from the stack */
	lua_assert(lua_gettop(L) == 1);
	return status;
}
void rover_lua::InteractivePrompt::check_interrupt(lua_State* L, lua_Debug* db) {
	InteractivePrompt* self = static_cast<InteractivePrompt*>(rover_lua::get_custom_ptr(L));

	if (self->should_interrupt) {
		self->should_interrupt = false;
		luaL_error(L, "interrupted!");
	} else if (self->should_close) {
		throw SignalShutdown();
	}
}

void rover_lua::InteractivePrompt::run() {
	try {
		_prompt_active = true;
		int status;

		// Check for interrupts every so often
		lua_sethook(L, check_interrupt, LUA_MASKCOUNT, 1000);

		while ((status = loadline()) != -1) {
			if (status == LUA_OK)
			status = docall(0, LUA_MULTRET);
			if (status == LUA_OK) lua_print();
			else report_error(status);
		}
		lua_settop(L, 0);  /* clear stack */
		lua_writeline();
	} catch (const SignalShutdown&) {
		// Let the shutdown signal fall through
		_prompt_active = false;
		_normal_exit = true;
	} catch (const std::exception& e) {
		_prompt_active = false;
		// Catch-all for other exceptions: try reporting error on console before closing
		std::string error_msg("lua: caught ");
		error_msg.append(typeid(e).name());
		error_msg.append("\n\tcause: ");
		error_msg.append(e.what());
		error_msg.append("\ninstance terminated");
		_c_write_line(error_msg.c_str());
	}
}