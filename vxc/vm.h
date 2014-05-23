#pragma once

typedef int(*BIND_FUNC)(lua_State*);

struct VirtualMachine
{
	void init();
	void close();
	void proc(const std::string &cmd);
	void reg(const std::string &name, BIND_FUNC func);

	lua_State *L;
};

extern VirtualMachine gVM;