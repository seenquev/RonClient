/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#include "container.h"

#include "allocator.h"
#include "logger.h"
#include "luascript.h"


// ---- Container ---- //

std::map<unsigned char, Container*> Container::containers;
unsigned char Container::lastIndex = 0xFF;

MUTEX Container::lockContainer;


Container::Container(unsigned char index, unsigned short id, std::string name, unsigned short places, bool child) {
	this->index = index;
	this->id = id;
	this->name = name;
	this->places = places;
	this->child = child;
}

Container::~Container() {
	LOCKCLASS lockClass(lockContainer);

	std::vector<Item*>::iterator it = items.begin();
	for (it; it != items.end(); it++) {
		Item* item = *it;
		if (item)
			delete_debug(item, M_PLACE);
	}

	items.clear();
}


void Container::AddItem(Item* item, bool pushBack) {
	LOCKCLASS lockClass(lockContainer);

	if (items.size() >= places) {
		Logger::AddLog("Container::AddItem()", "Too many items!", LOG_WARNING);
		return;
	}

	if (pushBack)
		items.push_back(item);
	else
		items.insert(items.begin(), item);
}

void Container::TransformItem(unsigned short slot, Item* item) {
	LOCKCLASS lockClass(lockContainer);

	std::vector<Item*>::iterator it = items.begin() + slot;
	if (it >= items.end()) {
		Logger::AddLog("Container::TransformItem()", "No item at given slot!", LOG_WARNING);
		return;
	}

	Item* _item = items[slot];
	if (_item)
		delete_debug(_item, M_PLACE);

	items[slot] = item;
}

void Container::RemoveItem(unsigned short slot) {
	LOCKCLASS lockClass(lockContainer);

	std::vector<Item*>::iterator it = items.begin() + slot;
	if (it >= items.end()) {
		Logger::AddLog("Container::RemoveItem()", "No item at given slot!", LOG_WARNING);
		return;
	}

	Item* item = items[slot];
	if (item)
		delete_debug(item, M_PLACE);

	items.erase(items.begin() + slot);
}

Item* Container::GetItem(unsigned short slot) {
	LOCKCLASS lockClass(lockContainer);

	Item* item = NULL;

	std::vector<Item*>::iterator it = items.begin() + slot;
	if (it < items.end())
		item = items[slot];

	return item;
}

bool Container::IsItemInContainer(unsigned short itemID, unsigned char itemSubType) {
	LOCKCLASS lockClass(lockContainer);

	for (int i = 0; i < places; i++) {
		Item* item = GetItem(i);
		if (item && item->GetID() == itemID && ((*item)() && ((*item)()->stackable || item->GetCount() == itemSubType)))
			return true;
	}

	return false;
}

unsigned char Container::GetIndex() {
	LOCKCLASS lockClass(lockContainer);

	return index;
}

std::string Container::GetName() {
	LOCKCLASS lockClass(lockContainer);

	return name;
}

unsigned short Container::GetID() {
	LOCKCLASS lockClass(lockContainer);

	return id;
}

unsigned short Container::GetPlaces() {
	LOCKCLASS lockClass(lockContainer);

	return places;
}

unsigned short Container::GetSize() {
	LOCKCLASS lockClass(lockContainer);

	return items.size();
}

bool Container::IsChild() {
	LOCKCLASS lockClass(lockContainer);

	return child;
}


unsigned char Container::GetFreeIndex() {
	LOCKCLASS lockClass(lockContainer);

	int lastIndex = -1;
	int index = 0;

	std::map<unsigned char, Container*>::iterator it = containers.begin();
	for (it; it != containers.end(); it++) {
		index = it->first;
		if (index - lastIndex > 1)
			return (unsigned char)(lastIndex + 1);

		lastIndex = index;
		index++;
	}

	return (unsigned char)index;
}


void Container::AddContainer(Container* container) {
	LOCKCLASS lockClass(lockContainer);

	if (!container) {
		Logger::AddLog("Container::AddContainer()", "Pointer to container is NULL!", LOG_ERROR);
		return;
	}

	std::map<unsigned char, Container*>::iterator it = containers.find(container->index);
	if (it != containers.end()) {
		Logger::AddLog("Container::AddContainer()", "Container with given index already exist!", LOG_WARNING);
		RemoveContainer(container->index);
	}

	containers[container->index] = container;
	lastIndex = container->index;
}

Container* Container::GetContainer(unsigned char index) {
	LOCKCLASS lockClass(lockContainer);

	std::map<unsigned char, Container*>::iterator it = containers.find(index);
	if (it == containers.end())
		return NULL;

	return it->second;
}

std::list<Container*> Container::GetContainers() {
	LOCKCLASS lockClass(lockContainer);

	std::list<Container*> retContainers;
	std::map<unsigned char, Container*>::iterator it = containers.begin();
	for (it; it != containers.end(); it++) {
		Container* container = it->second;
		if (container)
			retContainers.push_back(container);
	}

	return retContainers;
}

void Container::RemoveContainer(unsigned char index) {
	LOCKCLASS lockClass(lockContainer);

	std::map<unsigned char, Container*>::iterator it = containers.find(index);
	if (it == containers.end()) {
		Logger::AddLog("Container::RemoveContainer()", "Container with given index not exist!", LOG_ERROR);
		return;
	}

	Container* container = it->second;
	if (container)
		delete_debug(container, M_PLACE);

	containers.erase(it);
}


unsigned char Container::GetLastIndex() {
	LOCKCLASS lockClass(lockContainer);

	return lastIndex;
}

void Container::ResetLastIndex() {
	LOCKCLASS lockClass(lockContainer);

	lastIndex = 0xFF;
}


//Lua functions

int Container::LuaGetContainer(lua_State* L) {
	int index = LuaScript::PopNumber(L);

	Container* container = Container::GetContainer(index);

	LuaScript::PushNumber(L, (unsigned long)container);
	return 1;
}

int Container::LuaContainerGetLastIndex(lua_State* L) {
	int index = Container::GetLastIndex();

	LuaScript::PushNumber(L, index);
	return 1;
}

int Container::LuaContainerResetLastIndex(lua_State* L) {
	Container::ResetLastIndex();

	return 1;
}

int Container::LuaContainerGetPlaces(lua_State* L) {
	int index = LuaScript::PopNumber(L);

	int places = 0;
	Container* container = Container::GetContainer(index);
	if (container)
		places = container->GetPlaces();

	LuaScript::PushNumber(L, places);
	return 1;
}

int Container::LuaContainerGetSize(lua_State* L) {
	int index = LuaScript::PopNumber(L);

	int size = 0;
	Container* container = Container::GetContainer(index);
	if (container)
		size = container->GetSize();

	LuaScript::PushNumber(L, size);
	return 1;
}

int Container::LuaContainerGetItem(lua_State* L) {
	int slot = LuaScript::PopNumber(L);
	int index = LuaScript::PopNumber(L);

	Item* item = NULL;
	Container* container = Container::GetContainer(index);
	if (container)
		item = container->GetItem(slot);

	int id = 0;
	int count = 0;
	int stack = 0;
	if (item) {
		id = item->GetID();
		count = item->GetCount();
		stack = slot;
	}

	lua_newtable(L);
	lua_pushstring(L, "id");
	lua_pushnumber(L, id);
	lua_settable(L, -3);
	lua_pushstring(L, "count");
	lua_pushnumber(L, count);
	lua_settable(L, -3);
	lua_pushstring(L, "stack");
	lua_pushnumber(L, stack);
	lua_settable(L, -3);
	lua_pushstring(L, "pointer");
	lua_pushnumber(L, (unsigned long)item);
	lua_settable(L, -3);
	return 1;
}

int Container::LuaIsContainer(lua_State* L) {
	int itemid = LuaScript::PopNumber(L);

	ItemType* iType = Item::GetItemType(itemid);
	bool isContainer = (iType && iType->container);

	LuaScript::PushNumber(L, (int)isContainer);
	return 1;
}

int Container::LuaHasParent(lua_State* L) {
	int index = LuaScript::PopNumber(L);

	bool hasParent = false;
	Container* container = Container::GetContainer(index);
	if (container)
		hasParent = container->IsChild();

	LuaScript::PushNumber(L, (int)hasParent);
	return 1;
}


void Container::LuaRegisterFunctions(lua_State* L) {
	lua_pushinteger(L, CONTAINER_FREE);
	lua_setglobal(L, "CONTAINER_FREE");
    lua_pushinteger(L, CONTAINER_EQUIPMENT);
    lua_setglobal(L, "CONTAINER_EQUIPMENT");
	lua_pushinteger(L, CONTAINER_TRADE);
	lua_setglobal(L, "CONTAINER_TRADE");
	lua_pushinteger(L, CONTAINER_INVENTORY);
	lua_setglobal(L, "CONTAINER_INVENTORY");

	//getContainer(index) : containerPtr
	lua_register(L, "getContainer", Container::LuaGetContainer);

	//containerGetLastIndex()
	lua_register(L, "containerGetLastIndex", Container::LuaContainerGetLastIndex);

	//containerResetLastIndex()
	lua_register(L, "containerResetLastIndex", Container::LuaContainerResetLastIndex);

	//containerGetPlaces(index) : places
	lua_register(L, "containerGetPlaces", Container::LuaContainerGetPlaces);

	//containerGetSize(index) : size
	lua_register(L, "containerGetSize", Container::LuaContainerGetSize);

	//containerGetItem(index, slot) : item{id, count, stack, pointer}
	lua_register(L, "containerGetItem", Container::LuaContainerGetItem);

	//isContainer(itemid) : state
	lua_register(L, "isContainer", Container::LuaIsContainer);

	//hasParent(index) : state
	lua_register(L, "hasParent", Container::LuaHasParent);
}
