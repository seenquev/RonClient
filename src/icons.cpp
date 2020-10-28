/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#include "icons.h"

#include <string>

#include "allocator.h"
#include "filemanager.h"
#include "game.h"
#include "tools.h"


// ---- Icons ---- //

AD2D_Image Icons::logoIcon;

AD2D_Image Icons::shinerIcon;

AD2D_Image Icons::attackIcon;
AD2D_Image Icons::followIcon;
AD2D_Image Icons::selectIcon;

AD2D_Image Icons::dirArrowIcon;
AD2D_Image Icons::posArrowIcon;

AD2D_Image Icons::slotIcons[10];
AD2D_Image Icons::skullIcons[5];
AD2D_Image Icons::shieldIcons[10];
AD2D_Image Icons::fightIcons[6];
AD2D_Image Icons::statusIcons[15];
AD2D_Image Icons::buttonIcons[1];
AD2D_Image*	Icons::minimapIcons[256];
AD2D_Image*	Icons::waypointIcons[256];

AD2D_Image Icons::spellGroupIcons[4];
AD2D_Image*	Icons::spellIcons[256];

IconsMap	Icons::customIcons;

MUTEX Icons::lockIcons;

std::string Icons::logoFilename = "icons/logo.png";


Icons::Icons() { }

Icons::~Icons() { }

void Icons::LoadIcons(const char* path) {
	LOCKCLASS lockClass(lockIcons);

	std::string filename;

	FileManager* files = FileManager::fileManager;
	if (!files)
		return;

	logoIcon.CreatePNG_(files->GetFileData(logoFilename), files->GetFileSize(logoFilename));

	shinerIcon.CreatePNG_(files->GetFileData(std::string(path) + "/shiner.png"), files->GetFileSize(std::string(path) + "/shiner.png"));

	attackIcon.CreatePNG_(files->GetFileData(std::string(path) + "/attack.png"), files->GetFileSize(std::string(path) + "/attack.png"));
	followIcon.CreatePNG_(files->GetFileData(std::string(path) + "/follow.png"), files->GetFileSize(std::string(path) + "/follow.png"));
	selectIcon.CreatePNG_(files->GetFileData(std::string(path) + "/select.png"), files->GetFileSize(std::string(path) + "/select.png"));

	dirArrowIcon.CreatePNG_(files->GetFileData(std::string(path) + "/dirarrow.png"), files->GetFileSize(std::string(path) + "/dirarrow.png"));
	posArrowIcon.CreatePNG_(files->GetFileData(std::string(path) + "/posarrow.png"), files->GetFileSize(std::string(path) + "/posarrow.png"));

	for (int i = 0; i < 10; i++) {
		filename = std::string(path) + "/slot" + value2str(i + 1) + ".png";
		slotIcons[i].CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
	}

	for (int i = 0; i < 5; i++) {
		filename = std::string(path) + "/skull" + value2str(i + 1) + ".png";
		skullIcons[i].CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
	}

	for (int i = 0; i < 10; i++) {
		filename = std::string(path) + "/shield" + value2str(i + 1) + ".png";
		shieldIcons[i].CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
	}

	for (int i = 0; i < 6; i++) {
		filename = std::string(path) + "/fight" + value2str(i + 1) + ".png";
		fightIcons[i].CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
	}

	for (int i = 0; i < 15; i++) {
		filename = std::string(path) + "/status" + value2str(i + 1) + ".png";
		statusIcons[i].CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
	}

	for (int i = 0; i < 1; i++) {
		filename = std::string(path) + "/button" + value2str(i + 1) + ".png";
		buttonIcons[i].CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
	}

	for (int i = 0; i < 256; i++) {
		filename = std::string(path) + "/minimap" + value2str(i + 1) + ".png";

		unsigned char* data = files->GetFileData(filename);
		if (data) {
			minimapIcons[i] = new AD2D_Image;
			minimapIcons[i]->CreatePNG_(data, files->GetFileSize(filename));
		}
		else
			minimapIcons[i] = NULL;
	}

	for (int i = 0; i < 256; i++) {
		filename = std::string(path) + "/waypoint" + value2str(i + 1) + ".png";

		unsigned char* data = files->GetFileData(filename);
		if (data) {
			waypointIcons[i] = new AD2D_Image;
			waypointIcons[i]->CreatePNG_(data, files->GetFileSize(filename));
		}
		else
			waypointIcons[i] = NULL;
	}

	for (int i = 0; i < 4; i++) {
		filename = std::string(path) + "/spellG" + value2str(i + 1) + ".png";
		spellGroupIcons[i].CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
	}

	for (int i = 0; i < 256; i++) {
		filename = std::string(path) + "/spell" + value2str(i + 1) + ".png";

		unsigned char* data = files->GetFileData(filename);
		if (data) {
			spellIcons[i] = new AD2D_Image;
			spellIcons[i]->CreatePNG_(data, files->GetFileSize(filename));
		}
		else
			spellIcons[i] = NULL;
	}

	for (int i = 1; i <= 15000; i++)
		LoadCustomIcon(i);
}

void Icons::LoadCustomIcon(unsigned int id) {
	LOCKCLASS lockClass(lockIcons);

	FileManager* files = FileManager::fileManager;
	if (!files)
		return;

	std::string filenamePNG = "icons/custom" + value2str(id) + ".png";

	AD2D_Image* image = NULL;

	unsigned char* data = files->GetFileData(filenamePNG);
	if (data) {
		image = new AD2D_Image;
		image->CreatePNG_(data, files->GetFileSize(filenamePNG));
	}

	customIcons[id] = image;
}

AD2D_Image* Icons::GetLogoIcon() {
	LOCKCLASS lockClass(lockIcons);

	return &logoIcon;
}

AD2D_Image* Icons::GetShinerIcon() {
	LOCKCLASS lockClass(lockIcons);

	return &shinerIcon;
}

AD2D_Image* Icons::GetAttackIcon() {
	LOCKCLASS lockClass(lockIcons);

	return &attackIcon;
}

AD2D_Image* Icons::GetFollowIcon() {
	LOCKCLASS lockClass(lockIcons);

	return &followIcon;
}

AD2D_Image* Icons::GetSelectIcon() {
	LOCKCLASS lockClass(lockIcons);

	return &selectIcon;
}

AD2D_Image* Icons::GetDirArrowIcon() {
	LOCKCLASS lockClass(lockIcons);

	return &dirArrowIcon;
}

AD2D_Image* Icons::GetPosArrowIcon() {
	LOCKCLASS lockClass(lockIcons);

	return &posArrowIcon;
}


AD2D_Image* Icons::GetSlotIcon(unsigned char slot) {
	LOCKCLASS lockClass(lockIcons);

	int number = slot - 1;

	if (number >= 0 && number < 10)
		return &slotIcons[number];

	return NULL;
}

AD2D_Image* Icons::GetSkullIcon(unsigned char skull) {
	LOCKCLASS lockClass(lockIcons);

	int number = skull - 1;

	if (number >= 0 && number < 5)
		return &skullIcons[number];

	return NULL;
}

AD2D_Image* Icons::GetShieldIcon(unsigned char shield) {
	LOCKCLASS lockClass(lockIcons);

	int number = shield - 1;

	if (number >= 0 && number < 10)
		return &shieldIcons[number];

	return NULL;
}

AD2D_Image* Icons::GetFightIcon(unsigned char fight) {
	LOCKCLASS lockClass(lockIcons);

	int number = fight - 1;

	if (number >= 0 && number < 6)
		return &fightIcons[number];

	return NULL;
}

AD2D_Image* Icons::GetStatusIcon(unsigned char status) {
	LOCKCLASS lockClass(lockIcons);

	int number = status - 1;

	if (number >= 0 && number < 15)
		return &statusIcons[number];

	return NULL;
}

AD2D_Image* Icons::GetButtonIcon(unsigned char button) {
	LOCKCLASS lockClass(lockIcons);

	int number = button - 1;

	if (number >= 0 && number < 1)
		return &buttonIcons[number];

	return NULL;
}

AD2D_Image* Icons::GetMinimapIcon(unsigned char mmap) {
	LOCKCLASS lockClass(lockIcons);

	int number = mmap - 1;

	if (number >= 0 && number < 256)
		return minimapIcons[number];

	return NULL;
}

AD2D_Image* Icons::GetWaypointIcon(unsigned char wayp) {
	LOCKCLASS lockClass(lockIcons);

	int number = wayp - 1;

	if (number >= 0 && number < 256)
		return waypointIcons[number];

	return NULL;
}

AD2D_Image* Icons::GetSpellGroupIcon(unsigned char group) {
	LOCKCLASS lockClass(lockIcons);

	int number = group - 1;

	if (number >= 0 && number < 4)
		return &spellGroupIcons[number];

	return NULL;
}

AD2D_Image* Icons::GetSpellIcon(unsigned char spell) {
	LOCKCLASS lockClass(lockIcons);

	int number = spell - 1;

	if (number >= 0 && number < 256)
		return spellIcons[number];

	return NULL;
}

AD2D_Image* Icons::GetCustomIcon(unsigned int icon) {
	LOCKCLASS lockClass(lockIcons);

	bool exist = false;
	AD2D_Image* image = NULL;

	IconsMap::iterator it = customIcons.find(icon);
	if (it != customIcons.end()) {
		image = it->second;
		exist = true;
	}

	if (!exist) {
		LoadCustomIcon(icon);
		return GetCustomIcon(icon);
	}

	return image;
}


//Lua functions

int Icons::LuaGetLogoIcon(lua_State* L) {
	AD2D_Image* icon = GetLogoIcon();

	LuaScript::PushNumber(L, (unsigned long)icon);
	return 1;
}

int Icons::LuaGetAttackIcon(lua_State* L) {
	AD2D_Image* icon = GetAttackIcon();

	LuaScript::PushNumber(L, (unsigned long)icon);
	return 1;
}

int Icons::LuaGetFollowIcon(lua_State* L) {
	AD2D_Image* icon = GetFollowIcon();

	LuaScript::PushNumber(L, (unsigned long)icon);
	return 1;
}

int Icons::LuaGetSelectIcon(lua_State* L) {
	AD2D_Image* icon = GetSelectIcon();

	LuaScript::PushNumber(L, (unsigned long)icon);
	return 1;
}

int Icons::LuaGetDirArrowIcon(lua_State* L) {
	AD2D_Image* icon = GetDirArrowIcon();

	LuaScript::PushNumber(L, (unsigned long)icon);
	return 1;
}

int Icons::LuaGetPosArrowIcon(lua_State* L) {
	AD2D_Image* icon = GetPosArrowIcon();

	LuaScript::PushNumber(L, (unsigned long)icon);
	return 1;
}

int Icons::LuaGetSlotIcon(lua_State* L) {
	int id = LuaScript::PopNumber(L);

	AD2D_Image* icon = GetSlotIcon(id);

	LuaScript::PushNumber(L, (unsigned long)icon);
	return 1;
}

int Icons::LuaGetSkullIcon(lua_State* L) {
	int id = LuaScript::PopNumber(L);

	AD2D_Image* icon = GetSkullIcon(id);

	LuaScript::PushNumber(L, (unsigned long)icon);
	return 1;
}

int Icons::LuaGetShieldIcon(lua_State* L) {
	int id = LuaScript::PopNumber(L);

	AD2D_Image* icon = GetShieldIcon(id);

	LuaScript::PushNumber(L, (unsigned long)icon);
	return 1;
}

int Icons::LuaGetFightIcon(lua_State* L) {
	int id = LuaScript::PopNumber(L);

	AD2D_Image* icon = GetFightIcon(id);

	LuaScript::PushNumber(L, (unsigned long)icon);
	return 1;
}

int Icons::LuaGetStatusIcon(lua_State* L) {
	int id = LuaScript::PopNumber(L);

	AD2D_Image* icon = GetStatusIcon(id);

	LuaScript::PushNumber(L, (unsigned long)icon);
	return 1;
}

int Icons::LuaGetButtonIcon(lua_State* L) {
	int id = LuaScript::PopNumber(L);

	AD2D_Image* icon = GetButtonIcon(id);

	LuaScript::PushNumber(L, (unsigned long)icon);
	return 1;
}

int Icons::LuaGetMinimapIcon(lua_State* L) {
	int id = LuaScript::PopNumber(L);

	AD2D_Image* icon = GetMinimapIcon(id);

	LuaScript::PushNumber(L, (unsigned long)icon);
	return 1;
}

int Icons::LuaGetWaypointIcon(lua_State* L) {
	int id = LuaScript::PopNumber(L);

	AD2D_Image* icon = GetWaypointIcon(id);

	LuaScript::PushNumber(L, (unsigned long)icon);
	return 1;
}

int Icons::LuaGetSpellGroupIcon(lua_State* L) {
	int id = LuaScript::PopNumber(L);

	AD2D_Image* icon = GetSpellGroupIcon(id);

	LuaScript::PushNumber(L, (unsigned long)icon);
	return 1;
}

int Icons::LuaGetSpellIcon(lua_State* L) {
	int id = LuaScript::PopNumber(L);

	AD2D_Image* icon = GetSpellIcon(id);

	LuaScript::PushNumber(L, (unsigned long)icon);
	return 1;
}

int Icons::LuaGetCustomIcon(lua_State* L) {
	int id = LuaScript::PopNumber(L);

	AD2D_Image* icon = GetCustomIcon(id);

	LuaScript::PushNumber(L, (unsigned long)icon);
	return 1;
}


void Icons::LuaRegisterFunctions(lua_State* L) {
	//getLogoIcon() : imagePtr
	lua_register(L, "getLogoIcon", Icons::LuaGetLogoIcon);

	//getAttackIcon() : imagePtr
	lua_register(L, "getAttackIcon", Icons::LuaGetAttackIcon);

	//getFollowIcon() : imagePtr
	lua_register(L, "getFollowIcon", Icons::LuaGetFollowIcon);

	//getSelectIcon() : imagePtr
	lua_register(L, "getSelectIcon", Icons::LuaGetSelectIcon);

	//getDirArrowIcon() : imagePtr
	lua_register(L, "getDirArrowIcon", Icons::LuaGetDirArrowIcon);

	//getPosArrowIcon() : imagePtr
	lua_register(L, "getPosArrowIcon", Icons::LuaGetPosArrowIcon);

	//getSlotIcon(id) : imagePtr
	lua_register(L, "getSlotIcon", Icons::LuaGetSlotIcon);

	//getSkullIcon(id) : imagePtr
	lua_register(L, "getSkullIcon", Icons::LuaGetSkullIcon);

	//getShieldIcon(id) : imagePtr
	lua_register(L, "getShieldIcon", Icons::LuaGetShieldIcon);

	//getFightIcon(id) : imagePtr
	lua_register(L, "getFightIcon", Icons::LuaGetFightIcon);

	//getStatusIcon(id) : imagePtr
	lua_register(L, "getStatusIcon", Icons::LuaGetStatusIcon);

	//getButtonIcon(id) : imagePtr
	lua_register(L, "getButtonIcon", Icons::LuaGetButtonIcon);

	//getMinimapIcon(id) : imagePtr
	lua_register(L, "getMinimapIcon", Icons::LuaGetMinimapIcon);

	//getWaypointIcon(id) : imagePtr
	lua_register(L, "getWaypointIcon", Icons::LuaGetWaypointIcon);

	//getSpellGroupIcon(id) : imagePtr
	lua_register(L, "getSpellGroupIcon", Icons::LuaGetSpellGroupIcon);

	//getSpellIcon(id) : imagePtr
	lua_register(L, "getSpellIcon", Icons::LuaGetSpellIcon);

	//getCustomIcon(id) : imagePtr
	lua_register(L, "getCustomIcon", Icons::LuaGetCustomIcon);
}
