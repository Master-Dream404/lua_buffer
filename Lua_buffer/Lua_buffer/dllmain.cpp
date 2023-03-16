// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"

#include <lobject.h>
#include <lapi.h>
#include <lua.hpp>
#include <lstate.h>

#include <iostream>
#include <fstream>
#include <string>


#if LUA_VERSION_NUM == 504
#define LUA_INTERNAL_LINKAGE "C++"
#else
#define LUA_INTERNAL_LINKAGE "C"
#endif

// Utility macro for the constexpr if statement
#define LUA_IF_CONSTEXPR if constexpr

// Inline utility
#if !defined(LUA_INLINE)
#ifdef _MSC_VER
#ifndef _DEBUG
#define LUA_INLINE __forceinline
#else
#define LUA_INLINE
#endif
#elif __has_attribute(__always_inline__)
#define LUA_INLINE inline __attribute__((__always_inline__))
#else
#define LUA_INLINE inline
#endif
#endif

static const luaL_Reg lualibs[] = {
    { "_G", luaopen_base },
    { LUA_TABLIBNAME, luaopen_table },
    { LUA_STRLIBNAME, luaopen_string },
    { LUA_MATHLIBNAME, luaopen_math },
    { LUA_DBLIBNAME, luaopen_debug },
    { LUA_COLIBNAME, luaopen_coroutine },
    { LUA_UTF8LIBNAME, luaopen_utf8 },
#ifdef IS_FXSERVER
    { LUA_IOLIBNAME, luaopen_io },
    { LUA_OSLIBNAME, luaopen_os },
#endif
    { NULL, NULL }
};

class LuaStateHolder {
private:
	lua_State* m_state;

#if defined(LUA_USE_RPMALLOC)
	/// <summary>
	/// Create a lua_State instance with a rpmalloc allocator.
	/// </summary>
	static lua_State* lua_rpmalloc_state(void*& opaque);

	/// <summary>
	/// Free/Dispose any additional resources associated with the Lua state.
	/// </summary>
	static void lua_rpmalloc_free(void* opaque);

	/// <summary>
	/// Reference to the heap_t pointer. At the time of destruction lua_getallocf
	/// may point to the profiler allocator hook.
	/// </summary>
	void* rpmalloc_data = nullptr;
#endif

public:
	LuaStateHolder()
	{
#if defined(LUA_USE_RPMALLOC)
		m_state = lua_rpmalloc_state(rpmalloc_data);
#else
		m_state = luaL_newstate();
#endif
#if LUA_VERSION_NUM >= 504
		lua_gc(m_state, LUA_GCGEN, 0, 0);  /* GC in generational mode */
#endif
	}

	~LuaStateHolder()
	{
		Close();
	}

	void Close()
	{
		if (m_state)
		{
			lua_close(m_state);

#if defined(LUA_USE_RPMALLOC)
			lua_rpmalloc_free(rpmalloc_data);
			rpmalloc_data = nullptr;
#endif
			m_state = nullptr;
		}
	}

	operator lua_State* ()
	{
		return m_state;
	}

	LUA_INLINE lua_State* Get()
	{
		return m_state;
	}

};

LUALIB_API void safe_openlibs(lua_State* L)
{
	const luaL_Reg* lib = lualibs;
	for (; lib->func; lib++)
	{
		luaL_requiref(L, lib->name, lib->func, 1);
		lua_pop(L, 1);
	}
}

void Load_Script()
{
/*
	lua_State* m_luaState;

	m_luaState = luaL_newstate();
	assert(m_luaState);
	safe_openlibs(m_luaState);

	lua_pushlightuserdata(m_luaState, nullptr);
	lua_pushcclosure(m_luaState, [](lua_State* L)
		{
			auto loader = const_cast<void*>(lua_topointer(L, lua_upvalueindex(1)));
			auto key = luaL_checkstring(L, 1);
			auto value = luaL_checkstring(L, 2);

			if (stricmp(key, "is_cfxv2") != 0)
			{
				lua_Debug ar;
				if (lua_getstack(L, 2, &ar)) {
					if (lua_getinfo(L, "Sl", &ar))
					{
						if (ar.source && ar.source[0] == '@')
						{
							//location.file = fmt::sprintf("%s/%s", loader->GetComponent()->GetResource()->GetPath(), &ar.source[1]);
						}
					}
				}
			}

		}, 1);

	lua_setglobal(m_luaState, "AddMetaData");
*/



}

DWORD WINAPI Main(LPVOID ps)
{

	if (GetAsyncKeyState(VK_INSERT))
	{
		LuaStateHolder m_state;


		std::string a = "print(\"test\")";
		std::string name = "@dream";
		std::ifstream myfile;
		myfile.open("dream.lua");



		if (luaL_loadbufferx(m_state, a.c_str(), a.length(), "@dream", NULL) != 0)
		{
			MessageBoxA(0, "fail", "", 0);
		}

		fprintf(stderr, "%s\n", lua_tostring(m_state, -1));
	}

	FreeLibraryAndExitThread((HMODULE)ps, 0);

    return 0;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
		CreateThread(0, 0, Main, 0, 0, 0);
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

