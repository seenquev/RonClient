/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#include "iniloader.h"

#include <windows.h>

#include "allocator.h"
#include "filemanager.h"
#include "luascript.h"
#include "tools.h"


// ---- INILoader ---- //

INILoader::INILoader() { }

INILoader::~INILoader() { }

bool INILoader::OpenFile(std::string path) {
	FileManager* files = FileManager::fileManager;
	if (!files)
		return false;

	unsigned char* buffer = files->GetFileData(path);
	if (!buffer)
		return false;

	long int size = files->GetFileSize(path);

	std::string iniString;
	for (long int i = 0; i < size; i++)
		iniString.push_back(buffer[i]);

	delete[] buffer;

	return OpenString(iniString);
}

bool INILoader::OpenString(std::string iniString) {
	bool bvalue = false;
	bool bstr = false;
	bool perm = false;
	int index = 0;
	std::string name;
	std::map<int, std::string> value;

	size_t pos = 0;
	while(pos < iniString.length()) {
		char c = iniString[pos++];

		if (c == 13 || c == '\n' || pos == iniString.length()) {
			bvalue = false;
			bstr = false;
			perm = false;
			index = 0;

			int i;
			for (i = 0; name.length() > 0 && name[0] == ' '; i++)
				name.erase(0, 1);
			for (i = name.length() - 1; name.length() > 0 && name[i] == ' '; i--)
				name.erase(i, 1);

			if (name != "")
				entry.push_back(std::pair<std::string, std::map<int, std::string> >(name, value));

			name.clear();
			value.clear();
		}
		else if (c == '=' && !bvalue) bvalue = true;
		else if (!perm && c == '"' && bvalue) {
			if (bstr && value[index].length() == 0)
				perm = true;
			else
				bstr = !bstr;
		}
		else if (c == ',' && bvalue && !bstr) index++;
		else {
			if (!bvalue) name.push_back(c);
			else if (bstr) value[index].push_back(c);
		}
	}

	return true;
}

bool INILoader::SaveFile(std::string path) {
	FILE* file = fopen(path.c_str(), "wb");
	if (!file)
		return false;

	bool first = true;

	INIEntry::iterator it = entry.begin();
	for (it; it != entry.end(); it++) {
		if (first) first = false;
		else fputc('\n', file);

		if (!it->first.empty())
			fwrite(it->first.c_str(), it->first.length(), 1, file);

		if (!it->second.empty()) {
			fputc(' ', file);
			fputc('=', file);
			fputc(' ', file);

			std::map<int, std::string>::iterator itm = it->second.begin();
			for (itm; itm != it->second.end(); itm++) {
				if (itm != it->second.begin()) {
					fputc(',', file);
					fputc(' ', file);
				}
				fputc('"', file);
				fwrite(itm->second.c_str(), itm->second.length(), 1, file);
				if (!(itm->second.length() > 0 && itm->second[0] == '\"'))
					fputc('"', file);
			}
		}
	}

	fclose(file);

	return true;
}

std::string INILoader::GetValue(std::string varname, int index) {
	INIEntry::iterator it = entry.begin();

	std::string value = "";
	for (it; it != entry.end(); it++) {
		if (it->first == varname) {
			std::map<int, std::string>::iterator itm = it->second.find(index);
			if (itm != it->second.end())
				value = itm->second;
		}
	}

	return value;
}

std::string INILoader::GetValue(int pos, int index) {
	INIEntry::iterator it = entry.begin();

	std::string value = "";
	int n = 0;
	for (it; it != entry.end(); it++, n++) {
		if (n == pos) {
			std::map<int, std::string>::iterator itm = it->second.find(index);
			if (itm != it->second.end())
				value = itm->second;

			break;
		}
	}

	return value;
}

void INILoader::SetValue(std::string varname, std::string value, int index) {
	INIEntry::iterator it = entry.begin();

	bool enter = false;
	for (it; it != entry.end(); it++) {
		if (it->first == varname) {
			it->second[index] = value;
			enter = true;
		}
	}

	if (!enter) {
		std::map<int, std::string> valuesMap;
		valuesMap[index] = value;

		entry.push_back(std::pair<std::string, std::map<int, std::string> >(varname, valuesMap));
	}
}


unsigned int INILoader::GetSize() {
	return entry.size();
}

std::string INILoader::GetVarName(unsigned int pos) {
	INIEntry::iterator it = entry.begin();

	unsigned int _pos = 0;
	for (it; it != entry.end() && _pos <= pos; _pos++, it++) {
		if (_pos == pos)
			return it->first;
	}

	return "";
}


//Lua functions

int INILoader::LuaNewINILoader(lua_State* L) {
	INILoader* iniLoader = new(M_PLACE) INILoader;

	LuaScript::PushNumber(L, (unsigned long)iniLoader);
	return 1;
}

int INILoader::LuaDeleteINILoader(lua_State* L) {
	INILoader* iniLoader = (INILoader*)((unsigned long)LuaScript::PopNumber(L));

	if (iniLoader)
		delete_debug(iniLoader, M_PLACE);

	return 1;
}

int INILoader::LuaOpenFile(lua_State* L) {
	std::string path = LuaScript::PopString(L);
	INILoader* iniLoader = (INILoader*)((unsigned long)LuaScript::PopNumber(L));

	if (iniLoader)
		iniLoader->OpenFile(path);

	return 1;
}

int INILoader::LuaOpenString(lua_State* L) {
	std::string str = LuaScript::PopString(L);
	INILoader* iniLoader = (INILoader*)((unsigned long)LuaScript::PopNumber(L));

	if (iniLoader)
		iniLoader->OpenString(str);

	return 1;
}

int INILoader::LuaSaveFile(lua_State* L) {
	std::string path = LuaScript::PopString(L);
	INILoader* iniLoader = (INILoader*)((unsigned long)LuaScript::PopNumber(L));

	if (iniLoader)
		iniLoader->SaveFile(path);

	return 1;
}

int INILoader::LuaGetValue(lua_State* L) {
	int params = lua_gettop(L);

	int index = 0;
	if (params > 2)
		index = LuaScript::PopNumber(L);
	std::string varname = "";
	int varnum = 0;
	if (lua_type(L, -1) == LUA_TSTRING)
		varname = LuaScript::PopString(L);
	else
		varnum = LuaScript::PopNumber(L);
	INILoader* iniLoader = (INILoader*)((unsigned long)LuaScript::PopNumber(L));

	std::string value;
	if (iniLoader) {
		if (varname == "")
			value = iniLoader->GetValue(varnum, index);
		else
			value = iniLoader->GetValue(varname, index);
	}

	LuaScript::PushString(L, value);
	return 1;
}

int INILoader::LuaSetValue(lua_State* L) {
	int params = lua_gettop(L);

	int index = 0;
	if (params > 3)
		index = LuaScript::PopNumber(L);
	std::string value = LuaScript::PopString(L);
	std::string varname = LuaScript::PopString(L);
	INILoader* iniLoader = (INILoader*)((unsigned long)LuaScript::PopNumber(L));

	if (iniLoader)
		iniLoader->SetValue(varname, value, index);

	return 1;
}

int INILoader::LuaGetSize(lua_State* L) {
	INILoader* iniLoader = (INILoader*)((unsigned long)LuaScript::PopNumber(L));

	int value;
	if (iniLoader)
		value = iniLoader->GetSize();

	LuaScript::PushNumber(L, value);
	return 1;
}

int INILoader::LuaGetVarName(lua_State* L) {
	int varnum = LuaScript::PopNumber(L);
	INILoader* iniLoader = (INILoader*)((unsigned long)LuaScript::PopNumber(L));

	std::string value;
	if (iniLoader)
		value = iniLoader->GetVarName(varnum);

	LuaScript::PushString(L, value);
	return 1;
}


void INILoader::LuaRegisterFunctions(lua_State* L) {
	//newINILoader() : iniLoaderPtr
	lua_register(L, "newINILoader", INILoader::LuaNewINILoader);

	//deleteINILoader(iniLoaderPtr)
	lua_register(L, "deleteINILoader", INILoader::LuaDeleteINILoader);

	//iniOpenFile(iniLoaderPtr, path)
	lua_register(L, "iniOpenFile", INILoader::LuaOpenFile);

	//iniOpenString(iniLoaderPtr, str)
	lua_register(L, "iniOpenString", INILoader::LuaOpenString);

	//iniSaveFile(iniLoaderPtr, path)
	lua_register(L, "iniSaveFile", INILoader::LuaSaveFile);

	//iniGetValue(iniLoaderPtr, varname, index = 0) : valueStr
	//iniGetValue(iniLoaderPtr, varnum, index = 0) : valueStr
	lua_register(L, "iniGetValue", INILoader::LuaGetValue);

	//iniSetValue(iniLoaderPtr, varname, valueStr, index = 0)
	lua_register(L, "iniSetValue", INILoader::LuaSetValue);

	//iniGetSize(iniLoaderPtr) : size
	lua_register(L, "iniGetSize", INILoader::LuaGetSize);

	//iniGetVarName(iniLoaderPtr, varnum) : varname
	lua_register(L, "iniGetVarName", INILoader::LuaGetVarName);
}
