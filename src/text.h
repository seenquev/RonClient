/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#ifndef __TEXT_H_
#define __TEXT_H_

#include <string>
#include <list>

#include "iniloader.h"
#include "luascript.h"
#include "mthread.h"

class Languages {
private:
	static std::list<std::string>	languages;
	static bool			loaded;

	static MUTEX		languagesLock;

public:
	static std::list<std::string> GetLanguages();
};

class Text {
private:
	static INILoader	iniLoader;
	static bool			loaded;

	static MUTEX		textLock;

public:
	static std::string	GetText(std::string type, unsigned char lang);

	//Lua functions
	static int LuaGetText(lua_State* L);

	static void LuaRegisterFunctions(lua_State* L);
};

#endif //__TEXT_H_
