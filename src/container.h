/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#ifndef __CONTAINER_H_
#define __CONTAINER_H_

#include <map>
#include <string>
#include <vector>

#include "item.h"
#include "logger.h"
#include "luascript.h"
#include "mthread.h"


enum CONTAINER {
	CONTAINER_FREE = 0xFC,
    CONTAINER_EQUIPMENT = 0xFD,
	CONTAINER_TRADE = 0xFE,
	CONTAINER_INVENTORY = 0xFF,
};


class Container {
private:
	unsigned char		index;

	unsigned short		id;
	std::string			name;

	unsigned short		places;
	std::vector<Item*>	items;

	bool				child;

	static std::map<unsigned char, Container*>		containers;
	static unsigned char							lastIndex;

public:
	static MUTEX	lockContainer;

public:
	Container(unsigned char index, unsigned short id, std::string name, unsigned short places, bool child);
	~Container();

	void AddItem(Item* item, bool pushBack = false);
	void TransformItem(unsigned short slot, Item* item);
	void RemoveItem(unsigned short slot);
	Item* GetItem(unsigned short slot);
	bool IsItemInContainer(unsigned short itemID, unsigned char itemSubType);

	unsigned char GetIndex();
	std::string GetName();
	unsigned short GetID();
	unsigned short GetPlaces();
	unsigned short GetSize();
	bool IsChild();

	static unsigned char GetFreeIndex();

	static void AddContainer(Container* container);
	static Container* GetContainer(unsigned char index);
	static std::list<Container*> GetContainers();
	static void RemoveContainer(unsigned char index);

	static unsigned char GetLastIndex();
	static void ResetLastIndex();

	//Lua functions
	static int LuaGetContainer(lua_State* L);
	static int LuaContainerGetLastIndex(lua_State* L);
	static int LuaContainerResetLastIndex(lua_State* L);
	static int LuaContainerGetPlaces(lua_State* L);
	static int LuaContainerGetSize(lua_State* L);
	static int LuaContainerGetItem(lua_State* L);
	static int LuaIsContainer(lua_State* L);
	static int LuaHasParent(lua_State* L);

	static void LuaRegisterFunctions(lua_State* L);
};

#endif //__CONTAINER_H_
