/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#include "text.h"

#include "allocator.h"
#include "game.h"
#include "luascript.h"
#include "tools.h"


// ---- Languages ---- //

std::list<std::string>	Languages::languages;
bool					Languages::loaded = false;

MUTEX					Languages::languagesLock;


std::list<std::string> Languages::GetLanguages() {
	LOCKCLASS lockClass(languagesLock);

	if (!loaded) {
		languages.clear();

		INILoader iniLoader;
		iniLoader.OpenFile("text.ini");

		unsigned char num = atoi(iniLoader.GetValue("LANGUAGES").c_str());
		for (int i = 0; i < num; i++) {
			std::string type = "LANGUAGE_" + value2str(i);
			std::string language = iniLoader.GetValue(type);
			if (language != "")
				languages.push_back(language);
		}

		loaded = true;
	}

	return languages;
}


// ---- Text ---- //

INILoader	Text::iniLoader;
bool		Text::loaded = false;

MUTEX		Text::textLock;


std::string Text::GetText(std::string type, unsigned char lang) {
	LOCKCLASS lockClass(textLock);

	if (!loaded) {
		iniLoader.OpenFile("text.ini");
		loaded = true;
	}

	std::string ret = iniLoader.GetValue(type, lang);

	return ret;
}


//Lua functions

int Text::LuaGetText(lua_State* L) {
	std::string type = LuaScript::PopString(L);

	std::string text = GetText(type, Game::options.language);

	LuaScript::PushString(L, text);
	return 1;
}

void Text::LuaRegisterFunctions(lua_State* L) {
	//getText(keyStr) : textStr
	lua_register(L, "getText", Text::LuaGetText);
}
