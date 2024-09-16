
#include <lua.h>
#include <lualib.h>

#include <Luau/BytecodeBuilder.h>
#include <Luau/Compiler.h>
#include <Luau/CodeGen.h>

int main() {
    Luau::BytecodeBuilder bcb;
	bcb.setDumpFlags(Luau::BytecodeBuilder::Dump_Code);
	bcb.setDumpSource("stdin");
	Luau::CompileOptions compile_options;
	Luau::compileOrThrow(bcb, "print('Hello, world!')");

	Luau::CodeGen::AssemblyOptions assembly_options;
	assembly_options.annotatorContext = &bcb;

	std::string bytecode = bcb.getBytecode();
	std::unique_ptr<lua_State, void (*)(lua_State*)> globalState(luaL_newstate(), lua_close);
	lua_State* L = globalState.get();
	luaL_openlibs(L);
	int result = luau_load(L, "=stdin", bytecode.c_str(), bytecode.size(), 0);
	lua_call(L, 0, 0);
}