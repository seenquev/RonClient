/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#ifndef __INILOADER_H_
#define __INILOADER_H_

#include <list>
#include <map>
#include <stdio.h>
#include <string>

#include "luascript.h"


typedef std::list<std::pair<std::string, std::map<int, std::string> > > INIEntry;

class INILoader {
private:
	INIEntry	entry;

public:
	INILoader();
	~INILoader();

	bool OpenFile(std::string path);
	bool OpenString(std::string iniString);
	bool SaveFile(std::string path);
	std::string GetValue(std::string varname, int index = 0);
	std::string GetValue(int pos, int index = 0);
	void SetValue(std::string varname, std::string value, int index = 0);

	unsigned int GetSize();
	std::string GetVarName(unsigned int pos);


	//Lua functions
	static int LuaNewINILoader(lua_State* L);
	static int LuaDeleteINILoader(lua_State* L);
	static int LuaOpenFile(lua_State* L);
	static int LuaOpenString(lua_State* L);
	static int LuaSaveFile(lua_State* L);
	static int LuaGetValue(lua_State* L);
	static int LuaSetValue(lua_State* L);
	static int LuaGetSize(lua_State* L);
	static int LuaGetVarName(lua_State* L);

	static void LuaRegisterFunctions(lua_State* L);
};


#endif //__INILOADER_H_
