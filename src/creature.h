/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#ifndef __CREATURE_H_
#define __CREATURE_H_

#include <map>
#include <set>
#include <string>

#include "ad2d.h"
#include "const.h"
#include "container.h"
#include "item.h"
#include "logger.h"
#include "luascript.h"
#include "map.h"
#include "mthread.h"
#include "position.h"
#include "realtime.h"
#include "thing.h"

class Battle;
class Map;

typedef std::set<unsigned short> LoadedSet;

class OutfitType {
public:
	unsigned short lookType;
	unsigned char lookAddons;
	std::string name;

public:
	OutfitType();
	~OutfitType();
};

class Outfit {
public:
	unsigned short lookType;
	unsigned short lookTypeEx;
	unsigned char lookHead;
	unsigned char lookBody;
	unsigned char lookLegs;
	unsigned char lookFeet;
	unsigned char lookAddons;
	unsigned short lookMount;

public:
	Outfit();
	~Outfit();

	static void SetTemplateColor(COLOR currentColor, int index);
};


class Creature : public Thing {
private:
	unsigned int	id;
	std::string		name;

	unsigned char	health;
	Direction		direction;
	Direction		lastMove;

	Outfit			outfit;

	unsigned char	lightLevel;
	unsigned char	lightColor;
	unsigned short	speed;

	unsigned char	skull;
	unsigned char	shield;

	unsigned char	war;

	unsigned int	shine;

	bool			blocking;
	bool			ignored;

	time_lt			squareTime;
	unsigned char	squareColor;

    time_lt         equipmentTime;
	Container*      equipment;

	static std::map<std::string, int>			ignoredCreatures;
	static std::map<unsigned int, Creature*>	knownCreatures;

public:
	static MUTEX		lockCreature;

	static LoadedSet	loadedLooktypes;

public:
	Creature();
	Creature(Creature* creature);
	virtual ~Creature();

	void Clear();

	void SetID(unsigned int id);
	unsigned int GetID();
	void SetShine(unsigned int shine);
	unsigned int GetShine();

	bool IsPlayer();
	bool IsNPC();
	bool IsMonster();

	void SetName(std::string name);
	std::string GetName();
	std::string* GetNamePtr();

	void SetHealth(unsigned char health);
	unsigned char GetHealth();
	unsigned char* GetHealthPtr();

	void SetDirection(Direction direction);
	Direction GetDirection();

	void SetLastMove(Direction direction);
	Direction GetLastMove();

	void SetOutfit(Outfit outfit);
	Outfit GetOutfit();

	void SetLight(unsigned char lightColor, unsigned char lightLevel);
	unsigned char GetLightLevel();
	unsigned char GetLightColor();

	void SetSpeed(unsigned short speed);
	unsigned short GetSpeed();

	void SetSkull(unsigned char skull);
	unsigned char GetSkull();

	void SetShield(unsigned char shield);
	unsigned char GetShield();

	void SetWar(unsigned char war);
	unsigned char GetWar();

	void SetBlocking(bool blocking);
	bool GetBlocking();

	void SetStep(float step);
	float GetStep();

	void SetSquare(time_lt time, unsigned char color);
	time_lt GetSquareTime();
	unsigned char GetSquareColor();

	void SetEquipment(time_lt time, Container* container);
	time_lt GetEquipmentTime();
	Container* GetEquipment();

	ItemType* operator()();

	void GetStepOffset(float& x, float& y);

	void PrintCreature(AD2D_Window* gfx, Position pos, float x, float y, float width, float height, bool ignoreColors = false);
	void PrintDetails(AD2D_Window* gfx, AD2D_Font* font, Position pos, float x, float y, float width, float height, int name, int hp, int mana, bool attacked, bool followed, bool selected);
	void PrintIcons(AD2D_Window* gfx, Position pos, float x, float y, float width, float height);
	void PrintEquipment(AD2D_Window* gfx, Position pos, float x, float y, float width, float height);
	void PrintCreatureShine(AD2D_Window* gfx, Position pos, float x, float y, float width, float height, unsigned int shine);

	static void Ignore(std::string creatureName);
	static void Unignore(std::string creatureName);
	static bool IsIgnored(std::string creatureName);

	static void CheckCreatures(Map* map, float factor);
	static void AddToKnown(Creature* creature);
	static Creature* GetFromKnown(unsigned int id);
	static std::list<Creature*> GetCreaturesList();
	static void RemoveFromKnown(unsigned int id);
	static void DeleteKnown();

	//Lua functions
	static int LuaSetCreatureName(lua_State* L);
	static int LuaGetCreatureID(lua_State* L);
	static int LuaSetCreatureShine(lua_State* L);
	static int LuaGetCreatureShine(lua_State* L);
	static int LuaGetCreatureName(lua_State* L);
	static int LuaGetCreaturePosition(lua_State* L);
	static int LuaGetCreatureHealth(lua_State* L);
	static int LuaGetCreatureSkull(lua_State* L);
	static int LuaGetCreatureShield(lua_State* L);
	static int LuaGetCreatureWar(lua_State* L);
	static int LuaGetCreatureStep(lua_State* L);


	static void LuaRegisterFunctions(lua_State* L);

	friend class Battle;
};

#endif //__CREATURE_H_
