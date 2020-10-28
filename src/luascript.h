/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#ifndef __LUASCRIPT_H_
#define __LUASCRIPT_H_


extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

#include <set>
#include <string>

#include "mthread.h"
#include "position.h"
#include "tools.h"

class Game;

class LuaScript;

typedef std::set<LuaScript*> LuaScripts;
typedef std::map<lua_State*, LuaScript*> LuaStates;
typedef std::list<std::string> LuaArguments;
typedef std::map<std::string, std::string> LuaGlobals;
typedef std::map<std::string, std::pair<boost::thread::id, int> > LuaLocks;
typedef std::set<unsigned int> LuaFlags;

typedef std::pair<unsigned char*, long int> LuaData;
typedef std::map<std::string, LuaData> LuaFiles;


enum LUA_FLAGS {
	LUAFLAG_QUIT = 0x01,
};

class LuaScript {
private:
	LuaData			data;
	std::string 	fileName;

	lua_State*		luaState;
	bool			running;
	bool			loaded;

	LuaFlags			flags;
	static LuaGlobals	globals;
	static LuaLocks		locks;

	static LuaFiles 	files;
	static LuaStates	states;
	static LuaScripts	scripts;

public:
	THREAD			scriptThread;
	MUTEX			lockLuaScript;
	static MUTEX	lockLuaScripts;
	static MUTEX	lockLuaStates;
	static MUTEX	lockLuaData;

public:
	LuaScript();
	~LuaScript();

	std::string GetFileName() { return fileName; }

	void SetFlag(LUA_FLAGS flag);
	void ClearFlag(LUA_FLAGS flag);
	bool HasFlag(LUA_FLAGS flag);

	bool OpenScript(std::string path);
	bool Execute(std::string func, LuaArguments args, bool threaded = false);
	bool CleanUp();
	bool IsExecuting();
	bool IsLoaded();

	static LuaArguments ConvertArgs(std::string args);
	static std::string GetFilePath(std::string scriptName, bool localDir = false);
	static bool RunScript(std::string scriptname, std::string function, LuaArguments args, bool threaded, LuaScript* script = NULL);
	static bool RunScript(std::string execution, bool threaded, bool local = false, LuaScript* script = NULL);

	//Static globals functions
	static void SetGlobal(std::string key, std::string value);
	static std::string GetGlobal(std::string key);

	//Static files functions
	static void AddFile(std::string path, unsigned char* buffer, long int size);
	static LuaData GetFile(std::string path);
	static bool RemoveFile(std::string path);
	static void ClearFiles();

	//Static scripts functions
	static void AddLuaScript(LuaScript* script);
	static void RemoveLuaScript(LuaScript* script);
	static void ClearLuaScripts();
	static void CheckLuaScripts(Game* game);

	static void SetLuaScriptState(lua_State* L, LuaScript* script);
	static void ClearLuaScriptState(lua_State* L);
	static LuaScript* GetLuaScriptState(lua_State* L);
	static void ClearLuaScriptStates();

	//Debug functions
	static void CheckLocks();
	static void CheckGlobals();

	//Static data functions
	static double PopNumber(lua_State* L);
	static std::string PopString(lua_State* L);
	static Position PopPosition(lua_State* L);
	static TypePointer PopPointer(lua_State* L);
	static TypePointer ReadPointer(lua_State* L, int p);

	static void PushNumber(lua_State* L, double value);
	static void PushString(lua_State* L, std::string value);
	static void PushPosition(lua_State* L, Position value);
	static void PushPointer(lua_State* L, TypePointer value);

	//Static functions
	static int LuaSleep(lua_State* L);
	static int LuaRun(lua_State* L);
	static int LuaLock(lua_State* L);
	static int LuaUnlock(lua_State* L);
	static int LuaBitTest(lua_State* L);
	static int LuaSetFlag(lua_State* L);
	static int LuaClearFlag(lua_State* L);
	static int LuaHasFlag(lua_State* L);
	static int LuaNew(lua_State* L);
	static int LuaDelete(lua_State* L);
	static int LuaSetDynamic(lua_State* L);
	static int LuaGetDynamic(lua_State* L);
	static int LuaSetGlobal(lua_State* L);
	static int LuaGetGlobal(lua_State* L);

	static void RegisterFunctions(lua_State* L);
};


#endif //__LUASCRIPT_H_
