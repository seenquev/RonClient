/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#ifndef __PLAYER_H_
#define __PLAYER_H_

#include <string>
#include <vector>

#include "const.h"
#include "container.h"
#include "creature.h"
#include "logger.h"
#include "luascript.h"
#include "mthread.h"
#include "thing.h"


class Creature;
class Map;


struct Statistics {
	unsigned short health;
	unsigned short maxHealth;
	unsigned short mana;
	unsigned short maxMana;

	double capacity;
	unsigned long experience;

	unsigned short level;
	unsigned char level_p;

	unsigned short magicLevel;
	unsigned char magicLevel_p;

	unsigned char soul;
	unsigned short stamina;

	unsigned char skill[7];
	unsigned char skill_p[7];
};


class Player {
private:
	std::list<Direction>	autoPath;
	std::vector<Direction>	nextMove;

	unsigned char			fightModes;
	Statistics				statistics;
	Container				inventory;

	Position				directionPos;
	Position				targetPos;

	MUTEX					lockPlayer;

public:
	static unsigned int		creatureID;
	static unsigned int		attackID;
	static unsigned int		followID;
	static unsigned int		selectID;

	static bool				requestStop;
	static bool				walking;

public:
	Player();
	~Player();

	static void SetCreatureID(unsigned int ID);
	static unsigned int GetCreatureID();
	static void SetAttackID(unsigned int creatureID);
	static unsigned int GetAttackID();
	static void SetFollowID(unsigned int creatureID);
	static unsigned int GetFollowID();
	static void SetSelectID(unsigned int creatureID);
	static unsigned int GetSelectID();

	void SetAutoPath(std::list<Direction>& list);
	std::list<Direction> GetAutoPath();
	void PopAutoPath();
	int GetAutoPathSize();

	void SetNextMove(Direction dir);
	Direction GetNextMove();
	Direction PopNextMove();
	void ClearMoves();

	void SetFightMode(bool state, unsigned char num);
	bool GetFightMode(unsigned char num);
	void SetFightModes(unsigned char fightModes);
	unsigned char GetFightModes();
	Statistics* GetStatistics();
	Container* GetInventory();
	Creature* GetCreature();

	void SetDirectionPos(Position pos);
	Position GetDirectionPos();

	void SetTargetPos(Position pos);
	Position GetTargetPos();

	Position GetPosition();
	void GetStepOffset(float& x, float& y);

	//Lua function
	static int LuaGetPlayerPosition(lua_State* L);
	static int LuaGetPlayerHealth(lua_State* L);
	static int LuaGetPlayerMaxHealth(lua_State* L);
	static int LuaGetPlayerHealthP(lua_State* L);
	static int LuaGetPlayerMana(lua_State* L);
	static int LuaGetPlayerMaxMana(lua_State* L);
	static int LuaGetPlayerManaP(lua_State* L);
	static int LuaGetPlayerCapacity(lua_State* L);
	static int LuaGetPlayerExperience(lua_State* L);
	static int LuaGetPlayerLevel(lua_State* L);
	static int LuaGetPlayerLevelP(lua_State* L);
	static int LuaGetPlayerMagicLevel(lua_State* L);
	static int LuaGetPlayerMagicLevelP(lua_State* L);
	static int LuaGetPlayerSoul(lua_State* L);
	static int LuaGetPlayerStamina(lua_State* L);
	static int LuaGetPlayerSkill(lua_State* L);
	static int LuaGetPlayerSkillP(lua_State* L);
	static int LuaGetPlayerID(lua_State* L);
	static int LuaGetPlayerAttackID(lua_State* L);
	static int LuaGetPlayerFollowID(lua_State* L);
	static int LuaGetPlayerSelectID(lua_State* L);
	static int LuaGetPlayerStep(lua_State* L);
	static int LuaGetPlayerBattle(lua_State* L);
	static int LuaGetPlayerInventory(lua_State* L);
	static int LuaGetPlayerInventoryItem(lua_State* L);
	static int LuaPlayerCheckItemPresence(lua_State* L);

	static void LuaRegisterFunctions(lua_State* L);
};

#endif //__PLAYER_H_
