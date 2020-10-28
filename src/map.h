/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#ifndef __MAP_H_
#define __MAP_H_

#include <list>

#include "ad2d.h"
#include "battle.h"
#include "creature.h"
#include "light.h"
#include "logger.h"
#include "luascript.h"
#include "minimap.h"
#include "mthread.h"
#include "position.h"
#include "thing.h"
#include "window.h"

class Creature;
class Player;

class Map;


struct AStarNode {
	AStarNode* parent;
	int F;
	int x;
	int y;
	Direction dir;
};


typedef std::set<Position> PositionArrowsSet;


class Tile {
private:
	std::list<Thing*>	things;
	std::list<Thing*>	downThings;
	std::list<Thing*>	creatureThings;
	std::list<Thing*>	topThings;

	std::list<Creature*>	tempCreatures;

public:
	bool			horizontal;
	bool			vertical;
	Position		pos;

	static MUTEX	lockTile;

public:
	Tile();
	~Tile();

	void AddThing(Thing* thing);
	void InsertThing(Thing* thing, unsigned char stackPos = 255);
	void RemoveThing(Thing* thing);
	void TransformThing(Thing* thing, unsigned char stackPos);
	void AddTempCreature(Creature* creature);
	void RemoveTempCreature(Creature* creature);

	unsigned char GetSize();
	unsigned char GetDownSize();
	unsigned char GetCreaturesSize();
	unsigned char GetTopSize();
	unsigned char GetTempCreaturesSize();

	int GetDownHeight();

	unsigned char GetThingStackPos(Thing* thing);
	Thing* GetThingByStackPos(unsigned char stackPos);
	Thing* GetDownThingByStackPos(unsigned char stackPos);
	Thing* GetCreatureThingByStackPos(unsigned char stackPos);
	Thing* GetTopThingByStackPos(unsigned char stackPos);
	Creature* GetTempCreatureByStackPos(unsigned char stackPos);

	Thing* GetGroundThing();
	Thing* GetTopTopThing();
	Thing* GetTopCreatureThing(bool tempCreature = false);
	Thing* GetTopDownThing();
	Thing* GetMoveableThing();
	Thing* GetUseableThing(bool useWith = false);

	unsigned short GetSpeed();
	unsigned char GetMiniMapColor();
	bool IsBlocking(bool ignoreCreatures = false, bool ignorePathBlock = false);

	bool IsHorizontal();
	bool IsVertical();

	void SortThings();

	friend class Map;
};


class Map {
private:
	Tile*		tiles[18][14][16];
	Position	corner;

	MiniMap*	minimap;
	Battle*		battle;

	bool			fullZ;
	unsigned char	fromZ;
	unsigned char	toZ;

public:
	static MUTEX	lockMap;

	static AD2D_Image	mapImage;

	static PositionArrowsSet		posArrows;
	static std::list<Creature*>		creatures;

	static bool	mapUpdated;

public:
	Map();
	~Map();

	void SetMiniMap(MiniMap* minimap);
	MiniMap* GetMiniMap();
	void SetBattle(Battle* battle);
	Battle* GetBattle();

	void ChangeMiniMapZoom(float multiplier);
	void ResetMiniMapZoom();
	void MoveMiniMapLevel(char step);
	void ResetMiniMapLevel();
	void SetMiniMapPix(Position tilePos, Tile* tile);
	void UpdateMiniMap();
	void ReadMiniMap(int x = 0, int y = 0, int w = 256, int h = 256);
	void WriteMiniMap();
	void CloseMiniMapFile();

	void AddBattle(Tile* tile);
	void RemoveBattle(Tile* tile);
	unsigned int GetBattleID(int number);
	void ClearBattle();
	void UpdateBattle();
	void UpdateBattleContainer();

	void SetCorner(Position corner);
	Position GetCorner();
	void ClearMap(int sx = 0, int sy = 0, int width = 18, int height = 14, int zFrom = 0, int zTo = 15);
	void DeleteMap(int sx = 0, int sy = 0, int width = 18, int height = 14, int zFrom = 0, int zTo = 15);
	void MoveMap(int offsetX, int offsetY);

	void SetTile(int x, int y, int z, Tile* tile);
	void SetTile(Position pos, Tile* tile);
	Tile* GetTile(int x, int y, int z);
	Tile* GetTile(Position pos);

	static void SetDirectionPosition(Position pos);
	static void AddPositionArrow(Position pos);
	static void RemovePositionArrow(Position pos);
	static void ClearPositionArrows();

	void ToggleZ();
	void CalculateZ(Position pos);
	POINT GetZ();

	Tile* GetTopTile(int x, int y, int z_from, int z_to);
	bool IsHiddenTile(int x, int y, int z, int z_to);

	void AddTempCreature(Position pos, Creature* creature);
	void RemoveTempCreature(Position pos, Creature* creature);

	void FindWay(Player* player, Position to, std::list<Direction>& list);

	void SortThings(int sx = 0, int sy = 0, int width = 18, int height = 14);

	void RenderMap(AD2D_Window* gfx, Player* player);
	void PrintMap(AD2D_Window* gfx, AD2D_Font* font, Player* player, int x, int y, int width, int height);

	//Lua functions
	static int LuaGetPathTo(lua_State* L);
	static int LuaGetWaypoint(lua_State* L);
	static int LuaGetWaypointsSize(lua_State* L);
	static int LuaGetItemByStackPos(lua_State* L);
	static int LuaGetTopTopItem(lua_State* L);
	static int LuaGetTopDownItem(lua_State* L);
	static int LuaGetUseableItem(lua_State* L);
	static int LuaSetDirectionPosition(lua_State* L);
	static int LuaAddPositionArrow(lua_State* L);
	static int LuaRemovePositionArrow(lua_State* L);
	static int LuaClearPositionArrows(lua_State* L);

	static void LuaRegisterFunctions(lua_State* L);
};

#endif //__MAP_H_
