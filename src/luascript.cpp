/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#include "luascript.h"

#include "allocator.h"
#include "bot.h"
#include "channel.h"
#include "cooldowns.h"
#include "container.h"
#include "filemanager.h"
#include "game.h"
#include "guimanager.h"
#include "icons.h"
#include "iniloader.h"
#include "input.h"
#include "item.h"
#include "logger.h"
#include "map.h"
#include "minimap.h"
#include "protocol.h"
#include "realtime.h"
#include "sound.h"
#include "text.h"
#include "window.h"


// ---- LuaScript ---- //

LuaFiles LuaScript::files;
LuaStates LuaScript::states;
LuaScripts LuaScript::scripts;
MUTEX LuaScript::lockLuaScripts;
MUTEX LuaScript::lockLuaStates;
MUTEX LuaScript::lockLuaData;

LuaGlobals LuaScript::globals;
LuaLocks LuaScript::locks;


LuaScript::LuaScript() {
	data = LuaData(NULL, 0);
	fileName = "";

	luaState = NULL;
	running = false;
	loaded = false;
}

LuaScript::~LuaScript() {
	SetFlag(LUAFLAG_QUIT);
	if (scriptThread.joinable())
        scriptThread.join();

	CleanUp();
}


void LuaScript::SetFlag(LUA_FLAGS flag) {
	LOCKCLASS lockClass(lockLuaScript);

	flags.insert(flag);
}

void LuaScript::ClearFlag(LUA_FLAGS flag) {
	LOCKCLASS lockClass(lockLuaScript);

	LuaFlags::iterator it = flags.find(flag);
	if (it != flags.end())
		flags.erase(it);
}

bool LuaScript::HasFlag(LUA_FLAGS flag) {
	LOCKCLASS lockClass(lockLuaScript);

	LuaFlags::iterator it = flags.find(flag);
	if (it != flags.end())
		return true;

	return false;
}


bool LuaScript::OpenScript(std::string path) {
	LOCKCLASS lockClass(lockLuaScript);

	path = FileManager::FixFileName(path);
	data = GetFile(path);
	if (data.first == NULL) {
		FileManager* files = FileManager::fileManager;
		if (!files)
			return false;

		unsigned char* buffer = files->GetFileData(path);
		if (!buffer)
			return false;

		long int size = files->GetFileSize(path);

		data.first = buffer;
		data.second = size;
		AddFile(path, buffer, size);
	}

	fileName = path;
	loaded = true;
	return true;
}

bool LuaScript::Execute(std::string func, LuaArguments args, bool threaded) {
	running = true;

	if (threaded) {
		AddLuaScript(this);

		THREAD thr(boost::bind(&LuaScript::Execute, this, func, args, false));
		scriptThread.swap(thr);

		return true;
	}

	lockLuaScript.lock();

	if (!data.first) {
		running = false;
		lockLuaScript.unlock();
		return false;
	}

	if (!luaState) {
		luaState = lua_open();

		luaL_openlibs(luaState);

		lua_pushinteger(luaState, LUAFLAG_QUIT);
		lua_setglobal(luaState, "LUAFLAG_QUIT");

		RegisterFunctions(luaState);

		if (luaL_loadbuffer(luaState, (char*)data.first, data.second, fileName.c_str()) || lua_pcall(luaState, 0, LUA_MULTRET, 0)) {
			std::string error = lua_tostring(luaState, -1);
			lua_pop(luaState, 1);

			Logger::AddLog("LuaScript::Execute()", "LuaScript error: " + error, LOG_WARNING);

			if (luaState) {
				lua_close(luaState);
				luaState = NULL;
			}

			running = false;
			lockLuaScript.unlock();
			return false;
		}

		if (!luaState) {
			Logger::AddLog("LuaScript::Execute()", "luaState is NULL", LOG_WARNING);

			running = false;
			lockLuaScript.unlock();
			return false;
		}

		flags.clear();
	}

	SetLuaScriptState(luaState, this);

	lua_getfield(luaState, LUA_GLOBALSINDEX, func.c_str());
	lua_newtable(luaState);
	int i = 1;
	for (LuaArguments::iterator it = args.begin(); it != args.end(); it++, i++) {
		lua_pushnumber(luaState, i);
		lua_pushstring(luaState, (*it).c_str());
		lua_settable(luaState, -3);
	}
	if (lua_pcall(luaState, 1, 1, 0)) {
		std::string error = lua_tostring(luaState, -1);
		lua_pop(luaState, 1);

		Logger::AddLog("LuaScript::Execute()", "LuaScript file: " + fileName + "\nLuaScript error: " + error, LOG_WARNING);

		running = false;
		lockLuaScript.unlock();
		return false;
	}
	lua_pop(luaState, 1);
	bool ret = (bool)lua_tonumber(luaState, 0);

	running = false;
	lockLuaScript.unlock();
	return ret;
}

bool LuaScript::CleanUp() {
	LOCKCLASS lockClass(lockLuaScript);

	if (luaState) {
		flags.clear();
		ClearLuaScriptState(luaState);
		lua_close(luaState);
		luaState = NULL;
		running = false;
		return true;
	}

	return false;
}

bool LuaScript::IsExecuting() {
	return running;
}

bool LuaScript::IsLoaded() {
	return loaded;
}


LuaArguments LuaScript::ConvertArgs(std::string args) {
	LuaArguments arguments;

	size_t pos = 0;
	std::string temp;
	bool str = false;
	while(pos < args.length()) {
		char oldCh = (pos > 0 ? args[pos - 1] : 0);
		char ch = args[pos++];

		if (ch == '\"' && oldCh != '\\')
			str = !str;
		else if (ch != ';' || str)
			temp.push_back(ch);
		else if (ch == ';') {
			arguments.push_back(temp);
			temp.clear();
		}
	}
	if (temp != "")
		arguments.push_back(temp);

	return arguments;
}

std::string LuaScript::GetFilePath(std::string scriptName, bool localDir) {
	size_t pos = 0;
	if ((pos = scriptName.find(".lua", 0)) != std::string::npos)
		scriptName = scriptName.substr(0, pos);

	return (localDir ? Game::dataLocation : std::string(".")) + "/scripts/" + scriptName + ".lua";
}

bool LuaScript::RunScript(std::string scriptname, std::string function, LuaArguments args, bool threaded, LuaScript* script) {
	std::string filename = scriptname;

	if (threaded) {
		LuaScript* script = new(M_PLACE) LuaScript;

		if (script && script->OpenScript(filename)) {
			script->Execute(function, args, threaded);
			return true;
		}
		else if (script) {
			delete_debug(script, M_PLACE);
			return false;
		}
	}
	else {
		if (script)
			script->Execute(function, args, threaded);
		else {
			LuaScript script;
			if (script.OpenScript(filename)) {
				script.Execute(function, args, threaded);
				return true;
			}
			script.CleanUp();
		}
	}

	return false;
}

bool LuaScript::RunScript(std::string execution, bool threaded, bool local, LuaScript* script) {
	std::string name;
	std::string args;

	size_t pos = execution.find(';', 0);
	name = execution.substr(0, pos);
	if (pos != std::string::npos)
		args = execution.substr(pos + 1, std::string::npos);

	pos = 0;
	if ((pos = name.find(".lua", 0)) != std::string::npos)
		name = name.substr(0, pos);

	if (name != "") {
		std::string filename = (local ? Game::dataLocation : std::string(".")) + "/scripts/" + name + ".lua";

		if (threaded) {
			LuaScript* script = new(M_PLACE) LuaScript;

			if (script && script->OpenScript(filename)) {
				script->Execute("main", ConvertArgs(args), threaded);
				return true;
			}
			else if (script) {
				delete_debug(script, M_PLACE);
				return false;
			}
		}
		else {
			if (script)
				script->Execute("main", ConvertArgs(args), threaded);
			else {
				LuaScript script;
				if (script.OpenScript(filename)) {
					script.Execute("main", ConvertArgs(args), threaded);
					return true;
				}
			}
		}
	}

	return false;
}


//Static globals functions
void LuaScript::SetGlobal(std::string key, std::string value) {
	LOCKCLASS lockClass(lockLuaData);

	globals[key] = value;
}

std::string LuaScript::GetGlobal(std::string key) {
	LOCKCLASS lockClass(lockLuaData);

	LuaGlobals::iterator it = globals.find(key);
	if (it != globals.end())
		return it->second;

	return "";
}


//Static files functions
void LuaScript::AddFile(std::string path, unsigned char* buffer, long int size) {
	LOCKCLASS lockClass(lockLuaScripts);

	RemoveFile(path);
	files[path] = LuaData(buffer, size);
}

LuaData LuaScript::GetFile(std::string path) {
	LOCKCLASS lockClass(lockLuaScripts);

	LuaData data(NULL, 0);
	LuaFiles::iterator it = files.find(path);
	if (it != files.end())
		data = it->second;

	return data;
}

bool LuaScript::RemoveFile(std::string path) {
	LOCKCLASS lockClass(lockLuaScripts);

	LuaFiles::iterator it = files.find(path);
	if (it != files.end()) {
		unsigned char* buffer = it->second.first;
		delete buffer;
		files.erase(it);

		return true;
	}

	return false;
}

void LuaScript::ClearFiles() {
	LOCKCLASS lockClass(lockLuaScripts);

	LuaFiles::iterator it = files.begin();
	for (it; it != files.end(); it++) {
		unsigned char* buffer = it->second.first;
		delete buffer;
	}

	files.clear();
}


//Static scripts functions

void LuaScript::AddLuaScript(LuaScript* script) {
	LOCKCLASS lockClass(lockLuaScripts);

	scripts.insert(script);
}

void LuaScript::RemoveLuaScript(LuaScript* script) {
	LOCKCLASS lockClass(lockLuaScripts);

	LuaScripts::iterator it = scripts.find(script);
	if (it != scripts.end()) {
		delete_debug(script, M_PLACE);
		scripts.erase(it);
	}
}

void LuaScript::ClearLuaScripts() {
	LOCKCLASS lockClass(lockLuaScripts);

	LuaScripts::iterator it = scripts.begin();
	for (it; it != scripts.end(); it++) {
		LuaScript* luaScript = *it;
		luaScript->SetFlag(LUAFLAG_QUIT);
	}

	it = scripts.begin();
	for (it; it != scripts.end(); it++) {
		LuaScript* luaScript = *it;
		delete_debug(luaScript, M_PLACE);
	}

	scripts.clear();
}

void LuaScript::CheckLuaScripts(Game* game) {
	while(game->GetGameState() == GAME_LOGGEDTOGAME) {
		lockLuaScripts.lock();

		LuaScripts toDelete;

		LuaScripts::iterator it = scripts.begin();
		for (it; it != scripts.end(); it++) {
			LuaScript* script = *it;
			if (script && !script->IsExecuting())
				toDelete.insert(script);
		}

		LuaScripts::iterator sit = toDelete.begin();
		for (sit; sit != toDelete.end(); sit++)
			RemoveLuaScript(*sit);

		lockLuaScripts.unlock();
		Sleep(50);
	}

	ClearLuaScripts();
}


void LuaScript::SetLuaScriptState(lua_State* L, LuaScript* script) {
	LOCKCLASS lockClass(lockLuaStates);

	states[L] = script;
}

void LuaScript::ClearLuaScriptState(lua_State* L) {
	LOCKCLASS lockClass(lockLuaStates);

	LuaStates::iterator it = states.find(L);
	if (it != states.end())
		states.erase(it);
}

LuaScript* LuaScript::GetLuaScriptState(lua_State* L) {
	LOCKCLASS lockClass(lockLuaStates);

	LuaStates::iterator it = states.find(L);
	if (it != states.end())
		return it->second;

	return NULL;
}

void LuaScript::ClearLuaScriptStates() {
	lockLuaStates.lock();
	LuaStates _states = states;
	lockLuaStates.unlock();

	LuaStates::iterator it = _states.begin();
	for (it; it != _states.end(); it++) {
		LuaScript* luaScript = it->second;
		luaScript->SetFlag(LUAFLAG_QUIT);
	}

	it = _states.begin();
	for (it; it != _states.end(); it++) {
		LuaScript* luaScript = it->second;
		if (luaScript->scriptThread.joinable())
            luaScript->scriptThread.join();
		delete_debug(luaScript, M_PLACE);
	}
}


//Debug functions

void LuaScript::CheckLocks() {
	LOCKCLASS lockClass(lockLuaData);

	LuaLocks::iterator it = locks.begin();
	for (it; it != locks.end(); it++)
		std::cout << "(LOCK) " << it->first << " -> " << it->second.first << "(" << it->second.second << ")" << "\n";
}

void LuaScript::CheckGlobals() {
	LOCKCLASS lockClass(lockLuaData);

	LuaGlobals::iterator it = globals.begin();
	for (it; it != globals.end(); it++)
		std::cout << "(GLOBAL) " << it->first << " -> " << it->second << "\n";
}


//Static data functions

double LuaScript::PopNumber(lua_State* L) {
	double value = lua_tonumber(L, -1);
	lua_pop(L, 1);
	return value;
}

std::string LuaScript::PopString(lua_State* L) {
	std::string value = lua_tostring(L, -1);
	lua_pop(L, 1);
	return value;
}

Position LuaScript::PopPosition(lua_State* L) {
	Position value;
	lua_pushstring(L, "x");
	lua_gettable(L, -2);
	value.x = lua_tonumber(L, -1);
	lua_pop(L, 1);
	lua_pushstring(L, "y");
	lua_gettable(L, -2);
	value.y = lua_tonumber(L, -1);
	lua_pop(L, 1);
	lua_pushstring(L, "z");
	lua_gettable(L, -2);
	value.z = lua_tonumber(L, -1);
	lua_pop(L, 1);
	lua_pop(L, 1);
	return value;
}

TypePointer LuaScript::PopPointer(lua_State* L) {
	TypePointer value;
	lua_pushstring(L, "type");
	lua_gettable(L, -2);
	value.type = lua_tostring(L, -1);
	lua_pop(L, 1);
	lua_pushstring(L, "ptr");
	lua_gettable(L, -2);
	value.ptr = (void*)((unsigned long)lua_tonumber(L, -1));
	lua_pop(L, 1);
	lua_pop(L, 1);
	return value;
}

TypePointer LuaScript::ReadPointer(lua_State* L, int p) {
	TypePointer value;
	lua_pushstring(L, "type");
	lua_gettable(L, p - 1);
	value.type = lua_tostring(L, -1);
	lua_pop(L, 1);
	lua_pushstring(L, "ptr");
	lua_gettable(L, p - 1);
	value.ptr = (void*)((unsigned long)lua_tonumber(L, -1));
	lua_pop(L, 1);
	return value;
}


void LuaScript::PushNumber(lua_State* L, double value) {
	lua_pushnumber(L, value);
}

void LuaScript::PushString(lua_State* L, std::string value) {
	lua_pushstring(L, value.c_str());
}

void LuaScript::PushPosition(lua_State* L, Position value) {
	lua_newtable(L);
	lua_pushstring(L, "x");
	lua_pushnumber(L, value.x);
	lua_settable(L, -3);
	lua_pushstring(L, "y");
	lua_pushnumber(L, value.y);
	lua_settable(L, -3);
	lua_pushstring(L, "z");
	lua_pushnumber(L, value.z);
	lua_settable(L, -3);
}

void LuaScript::PushPointer(lua_State* L, TypePointer value) {
	lua_newtable(L);
	lua_pushstring(L, "type");
	lua_pushstring(L, value.type.c_str());
	lua_settable(L, -3);
	lua_pushstring(L, "ptr");
	lua_pushnumber(L, (double)((unsigned long)value.ptr));
	lua_settable(L, -3);
}


//Static functions

int LuaScript::LuaSleep(lua_State* L) {
	unsigned int mseconds = (unsigned int)PopNumber(L);

	LuaScript* script = GetLuaScriptState(L);
	script->lockLuaScript.unlock();
	while(mseconds && !script->HasFlag(LUAFLAG_QUIT)) {
		if (mseconds > 100) {
			Sleep(100);
			mseconds -= 100;
		}
		else {
			Sleep(mseconds);
			mseconds = 0;
		}
	}
	script->lockLuaScript.lock();

	return 1;
}

int LuaScript::LuaRun(lua_State* L) {
	int params = lua_gettop(L);
	bool localDir = (bool)LuaScript::PopNumber(L); params--;
	bool threaded = (bool)LuaScript::PopNumber(L); params--;
	LuaArguments args;
	for (params; params > 2; params--)
		args.push_front(LuaScript::PopString(L));
	std::string functionName = LuaScript::PopString(L);
	std::string scriptName = LuaScript::PopString(L);

	size_t pos = 0;
	if ((pos = scriptName.find(".lua", 0)) != std::string::npos)
		scriptName = scriptName.substr(0, pos);

	std::string fileName = (localDir ? Game::dataLocation : std::string(".")) + "/scripts/" + scriptName + ".lua";
	RunScript(fileName, functionName, args, threaded);

	return 1;
}

int LuaScript::LuaLock(lua_State* L) {
	lockLuaData.lock();

	LuaScript* script = GetLuaScriptState(L);
	std::string lock = LuaScript::PopString(L);

	LuaLocks::iterator it = locks.find(lock);
	if (it == locks.end())
		locks[lock] = std::pair<boost::thread::id, int>(boost::this_thread::get_id(), 1);
	else {
		if (it->second.first == boost::this_thread::get_id())
			it->second.second++;
		else {
			bool locked = true;
			while(locked) {
				lockLuaData.unlock();
				script->lockLuaScript.unlock();
				Sleep(50);
				script->lockLuaScript.lock();
				lockLuaData.lock();
				locked = (locks.find(lock) != locks.end());
			}
		}
	}

	lockLuaData.unlock();

	return 1;
}

int LuaScript::LuaUnlock(lua_State* L) {
	LOCKCLASS lockClass(lockLuaData);

	std::string lock = LuaScript::PopString(L);

	LuaLocks::iterator it = locks.find(lock);
	if (it != locks.end()) {
		if (it->second.first == boost::this_thread::get_id())
			it->second.second--;

		if (it->second.second == 0)
			locks.erase(it);
	}

	return 1;
}

int LuaScript::LuaBitTest(lua_State* L) {
	unsigned long bit = LuaScript::PopNumber(L);
	unsigned long value = LuaScript::PopNumber(L);

	bool test = false;
	if (value & (1 << bit))
		test = true;

	LuaScript::PushNumber(L, (int)test);
	return 1;
}

int LuaScript::LuaSetFlag(lua_State* L) {
	LUA_FLAGS flag = (LUA_FLAGS)PopNumber(L);

	LuaScript* script = GetLuaScriptState(L);
	script->SetFlag(flag);

	return 1;
}

int LuaScript::LuaClearFlag(lua_State* L) {
	LUA_FLAGS flag = (LUA_FLAGS)PopNumber(L);

	LuaScript* script = GetLuaScriptState(L);
	script->ClearFlag(flag);

	return 1;
}

int LuaScript::LuaHasFlag(lua_State* L) {
	LUA_FLAGS flag = (LUA_FLAGS)PopNumber(L);

	LuaScript* script = GetLuaScriptState(L);
	bool ret = script->HasFlag(flag);

	PushNumber(L, (int)ret);
	return 1;
}

int LuaScript::LuaNew(lua_State* L) {
	std::string type = PopString(L);

	void* ptr = NULL;
	if (type == "float")
		ptr = (void*)new(M_PLACE) double;
	else if (type == "uint8")
		ptr = (void*)new(M_PLACE) unsigned char;
	else if (type == "uint16")
		ptr = (void*)new(M_PLACE) unsigned short;
	else if (type == "uint32")
		ptr = (void*)new(M_PLACE) unsigned int;
	else if (type == "uint64")
		ptr = (void*)new(M_PLACE) unsigned long;
	else if (type == "string")
		ptr = (void*)new(M_PLACE) std::string;
	else {
		lua_pushnil(L);
		return 1;
	}

	PushPointer(L, TypePointer(type, ptr));
	return 1;
}

int LuaScript::LuaDelete(lua_State* L) {
	TypePointer pointer = PopPointer(L);

	if (pointer.type == "float")
		delete_debug((double*)pointer.ptr, M_PLACE);
	else if (pointer.type == "uint8")
		delete_debug((unsigned char*)pointer.ptr, M_PLACE);
	else if (pointer.type == "uint16")
		delete_debug((unsigned short*)pointer.ptr, M_PLACE);
	else if (pointer.type == "uint32")
		delete_debug((unsigned int*)pointer.ptr, M_PLACE);
	else if (pointer.type == "uint64")
		delete_debug((unsigned long*)pointer.ptr, M_PLACE);
	else if (pointer.type == "string")
		delete_debug((std::string*)pointer.ptr, M_PLACE);

	return 1;
}

int LuaScript::LuaSetDynamic(lua_State* L) {
	std::string value = PopString(L);
	TypePointer pointer = PopPointer(L);

	pointer.SetValue(value);

	return 1;
}

int LuaScript::LuaGetDynamic(lua_State* L) {
	int params = lua_gettop(L);

	int precision = 6;
	if (params > 1)
		precision = (int)PopNumber(L);
	TypePointer pointer = PopPointer(L);

	std::string value = pointer.GetValue(precision);

	PushString(L, value);
	return 1;
}

int LuaScript::LuaSetGlobal(lua_State* L) {
	std::string value;
	if (lua_type(L, -1) == LUA_TNUMBER)
		value = value2str(PopNumber(L));
	else
		value = PopString(L);
	std::string key = PopString(L);

	SetGlobal(key, value);

	return 1;
}

int LuaScript::LuaGetGlobal(lua_State* L) {
	std::string key = PopString(L);

	std::string value = GetGlobal(key);

	PushString(L, value);
	return 1;
}


void LuaScript::RegisterFunctions(lua_State* L) {
	//sleep(miliseconds)
	lua_register(L, "sleep", LuaScript::LuaSleep);

	//run(scriptName, functionName, arg1, arg2, ..., threaded = 0, localDir = 0)
	lua_register(L, "run", LuaScript::LuaRun);

	//lock(lockName)
	lua_register(L, "lock", LuaScript::LuaLock);

	//unlock(lockName)
	lua_register(L, "unlock", LuaScript::LuaUnlock);

	//bitTest(value, bit)
	lua_register(L, "bitTest", LuaScript::LuaBitTest);

	//setFlag(flag)
	lua_register(L, "setFlag", LuaScript::LuaSetFlag);

	//clearFlag(flag)
	lua_register(L, "clearFlag", LuaScript::LuaClearFlag);

	//hasFlag(flag) : state
	lua_register(L, "hasFlag", LuaScript::LuaHasFlag);

	//new(type) : valuePtrType
	lua_register(L, "new", LuaScript::LuaNew);

	//delete(valuePtr)
	lua_register(L, "delete", LuaScript::LuaDelete);

	//setDynamic(valuePtr, valueStr)
	lua_register(L, "setDynamic", LuaScript::LuaSetDynamic);

	//getDynamic(valuePtr, precision = 6) : valueStr
	lua_register(L, "getDynamic", LuaScript::LuaGetDynamic);

	//setDynamic(key, value)
	lua_register(L, "setGlobal", LuaScript::LuaSetGlobal);

	//getDynamic(key) : valueStr
	lua_register(L, "getGlobal", LuaScript::LuaGetGlobal);

	RealTime::LuaRegisterFunctions(L);
	Protocol::LuaRegisterFunctions(L);
	INILoader::LuaRegisterFunctions(L);
	Mouse::LuaRegisterFunctions(L);
	Keyboard::LuaRegisterFunctions(L);
	Windows::LuaRegisterFunctions(L);
	GUIManager::LuaRegisterFunctions(L);
	Game::LuaRegisterFunctions(L);
	Item::LuaRegisterFunctions(L);
	Map::LuaRegisterFunctions(L);
	MiniMap::LuaRegisterFunctions(L);
	Cooldowns::LuaRegisterFunctions(L);
	Icons::LuaRegisterFunctions(L);
	Channel::LuaRegisterFunctions(L);
	Container::LuaRegisterFunctions(L);
	Player::LuaRegisterFunctions(L);
	Creature::LuaRegisterFunctions(L);
	SFX_System::LuaRegisterFunctions(L);
	Bot::LuaRegisterFunctions(L);
	Text::LuaRegisterFunctions(L);
}
