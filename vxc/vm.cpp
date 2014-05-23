#include "base.h"
#include "vm.h"
#include "log.h"

VirtualMachine gVM;

void VirtualMachine::init()
{
	L = luaL_newstate();
	luaopen_base(L);
	luaopen_table(L);
	luaopen_string(L);
	luaopen_math(L);
}

void VirtualMachine::close()
{
	lua_close(L);
}

void VirtualMachine::reg(const std::string &name, BIND_FUNC func)
{
	lua_register(L, name.c_str(), func);
}

void VirtualMachine::proc(const std::string &cmd)
{
	luaL_dostring(L, cmd.c_str());
}