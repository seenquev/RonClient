/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#ifndef __ICONS_H_
#define __ICONS_H_

#include <string>

#include "ad2d.h"
#include "luascript.h"
#include "mthread.h"

typedef std::map<unsigned int, AD2D_Image*> IconsMap;

class Icons {
private:
	static AD2D_Image		logoIcon;

	static AD2D_Image		shinerIcon;

	static AD2D_Image		attackIcon;
	static AD2D_Image		followIcon;
	static AD2D_Image		selectIcon;

	static AD2D_Image		dirArrowIcon;
	static AD2D_Image		posArrowIcon;

	static AD2D_Image		slotIcons[10];
	static AD2D_Image		skullIcons[5];
	static AD2D_Image		shieldIcons[10];
	static AD2D_Image		fightIcons[6];
	static AD2D_Image		statusIcons[15];
	static AD2D_Image		buttonIcons[1];
	static AD2D_Image*		minimapIcons[256];
	static AD2D_Image*		waypointIcons[256];

	static AD2D_Image		spellGroupIcons[4];
	static AD2D_Image*		spellIcons[256];

	static IconsMap			customIcons;

	static MUTEX			lockIcons;

public:
	static std::string		logoFilename;

public:
	Icons();
	~Icons();

	static void LoadCustomIcon(unsigned int id);
	void LoadIcons(const char* path);

	static AD2D_Image* GetLogoIcon();
	static AD2D_Image* GetShinerIcon();
	static AD2D_Image* GetAttackIcon();
	static AD2D_Image* GetFollowIcon();
	static AD2D_Image* GetSelectIcon();
	static AD2D_Image* GetDirArrowIcon();
	static AD2D_Image* GetPosArrowIcon();
	static AD2D_Image* GetSlotIcon(unsigned char slot);
	static AD2D_Image* GetSkullIcon(unsigned char skull);
	static AD2D_Image* GetShieldIcon(unsigned char shield);
	static AD2D_Image* GetFightIcon(unsigned char fight);
	static AD2D_Image* GetStatusIcon(unsigned char status);
	static AD2D_Image* GetButtonIcon(unsigned char button);
	static AD2D_Image* GetMinimapIcon(unsigned char mmap);
	static AD2D_Image* GetWaypointIcon(unsigned char wayp);
	static AD2D_Image* GetSpellGroupIcon(unsigned char group);
	static AD2D_Image* GetSpellIcon(unsigned char spell);

	static AD2D_Image* GetCustomIcon(unsigned int icon);

	//Lua functions
	static int LuaGetLogoIcon(lua_State* L);
	static int LuaGetAttackIcon(lua_State* L);
	static int LuaGetFollowIcon(lua_State* L);
	static int LuaGetSelectIcon(lua_State* L);
	static int LuaGetDirArrowIcon(lua_State* L);
	static int LuaGetPosArrowIcon(lua_State* L);
	static int LuaGetSlotIcon(lua_State* L);
	static int LuaGetSkullIcon(lua_State* L);
	static int LuaGetShieldIcon(lua_State* L);
	static int LuaGetFightIcon(lua_State* L);
	static int LuaGetStatusIcon(lua_State* L);
	static int LuaGetButtonIcon(lua_State* L);
	static int LuaGetMinimapIcon(lua_State* L);
	static int LuaGetWaypointIcon(lua_State* L);
	static int LuaGetSpellGroupIcon(lua_State* L);
	static int LuaGetSpellIcon(lua_State* L);
	static int LuaGetCustomIcon(lua_State* L);

	static void LuaRegisterFunctions(lua_State* L);
};

#endif //__ICONS_H_
