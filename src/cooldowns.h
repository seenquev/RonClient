/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#ifndef __COOLDOWNS_H_
#define __COOLDOWNS_H_

#include <list>
#include <map>

#include "game.h"
#include "luascript.h"
#include "mthread.h"
#include "realtime.h"


class Game;

class WindowElementContainer;


class Cooldown {
private:
	time_lt			castTime;
	unsigned int	period;

	std::string		comment;

public:
	Cooldown();
	~Cooldown();

	void Cast(unsigned int period);
	bool CanCast();

	void SetComment(std::string text);
	std::string GetComment();

	friend class Cooldowns;
};


typedef std::map<unsigned char, Cooldown*> CooldownMap;

class Cooldowns {
private:
	WindowElementContainer* containerGroup;
	WindowElementContainer* containerSpell;

	static CooldownMap	groupCooldowns;
	static CooldownMap	spellCooldowns;

	static int		threads;
	static MUTEX	lockCooldowns;

public:
	Cooldowns();
	~Cooldowns();

	void CastGroup(unsigned char id, unsigned int period);
	void CastSpell(unsigned char id, unsigned int period);

	void AddCooldown(unsigned char id, Cooldown* cooldown);
	void RemoveCooldown(unsigned char id);
	void ClearCooldowns();
	Cooldown* GetGroupCooldown(unsigned char id);
	Cooldown* GetCooldown(unsigned char id);

	static void CheckCooldowns(Game* game);

	void SetContainerGroup(WindowElementContainer* container);
	void SetContainerSpell(WindowElementContainer* container);
	void UpdateContainerGroup();
	void UpdateContainerSpell();

	//Lua functions
	static int LuaNewCooldown(lua_State* L);
	static int LuaDeleteCooldown(lua_State* L);
	static int LuaAddCooldown(lua_State* L);
	static int LuaRemoveCooldown(lua_State* L);
	static int LuaCastCooldown(lua_State* L);
	static int LuaCastCooldownGroup(lua_State* L);

	static void LuaRegisterFunctions(lua_State* L);
};

#endif //__COOLDOWNS_H_
