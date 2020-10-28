/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#include "player.h"

#include "allocator.h"
#include "luascript.h"
#include "map.h"
#include "protocol.h"
#include "text.h"


// ---- Player ---- //

unsigned int	Player::creatureID = 0;
unsigned int	Player::attackID = 0;
unsigned int	Player::followID = 0;
unsigned int	Player::selectID = 0;

bool Player::requestStop = false;
bool Player::walking = false;


Player::Player() : inventory(CONTAINER_INVENTORY, 0, "Inventory", 10, false) {
	for (int i = 0; i < 10; i++)
		inventory.AddItem(NULL);

	memset(&statistics, 0, sizeof(statistics));

	fightModes = 0x29;

	directionPos = Position(0, 0, 0);
	targetPos = Position(0, 0, 0);
}

Player::~Player() { }


void Player::SetCreatureID(unsigned int ID) {
	creatureID = ID;
	attackID = 0;
	followID = 0;
	selectID = 0;
}

unsigned int Player::GetCreatureID() {
	return creatureID;
}

void Player::SetAttackID(unsigned int creatureID) {
	attackID = creatureID;
}

unsigned int Player::GetAttackID() {
	return attackID;
}

void Player::SetFollowID(unsigned int creatureID) {
	followID = creatureID;
}

unsigned int Player::GetFollowID() {
	return followID;
}

void Player::SetSelectID(unsigned int creatureID) {
	selectID = creatureID;
}

unsigned int Player::GetSelectID() {
	return selectID;
}

void Player::SetAutoPath(std::list<Direction>& list) {
	LOCKCLASS lockClass(lockPlayer);

	Position pos = GetPosition();

	autoPath.clear();
	autoPath.insert(autoPath.begin(), list.begin(), list.end());
}

std::list<Direction> Player::GetAutoPath() {
	LOCKCLASS lockClass(lockPlayer);

	return autoPath;
}

void Player::PopAutoPath() {
	LOCKCLASS lockClass(lockPlayer);

	if (autoPath.size() > 0)
		autoPath.pop_front();
}

int Player::GetAutoPathSize() {
	LOCKCLASS lockClass(lockPlayer);

	return autoPath.size();
}

void Player::SetNextMove(Direction dir) {
	LOCKCLASS lockClass(lockPlayer);

	if (nextMove.empty())
		nextMove.push_back(dir);
	else if (nextMove.size() < 2) {
		if (nextMove[nextMove.size() - 1] != dir)
			nextMove.push_back(dir);
	}
	else {
		nextMove.clear();
		nextMove.push_back(dir);
	}
}

Direction Player::GetNextMove() {
	LOCKCLASS lockClass(lockPlayer);

	if (!nextMove.empty()) {
		Direction dir = nextMove[0];
		return dir;
	}

	return NONE;
}

Direction Player::PopNextMove() {
	LOCKCLASS lockClass(lockPlayer);

	if (!nextMove.empty()) {
		Direction dir = nextMove[0];
		nextMove.erase(nextMove.begin());
		return dir;
	}

	return NONE;
}

void Player::ClearMoves() {
	LOCKCLASS lockClass(lockPlayer);

	nextMove.clear();
}


void Player::SetFightMode(bool state, unsigned char num) {
	if (state) fightModes |= (0x01 << num);
	else fightModes &= ~(0x01 << num);
}

bool Player::GetFightMode(unsigned char num) {
	bool state = (fightModes & (0x01 << num) ? true : false);
	return state;
}

void Player::SetFightModes(unsigned char fightModes) {
	this->fightModes = fightModes;
}

unsigned char Player::GetFightModes() {
	return fightModes;
}


Statistics* Player::GetStatistics() {
	return &statistics;
}

Container* Player::GetInventory() {
	return &inventory;
}

Creature* Player::GetCreature() {
	if (creatureID == 0)
		return NULL;

	return Creature::GetFromKnown(creatureID);
}


void Player::SetDirectionPos(Position pos) {
	directionPos = pos;
}

Position Player::GetDirectionPos() {
	return directionPos;
}


void Player::SetTargetPos(Position pos) {
	targetPos = pos;
}

Position Player::GetTargetPos() {
	return targetPos;
}

Position Player::GetPosition() {
	Creature* creature = Creature::GetFromKnown(creatureID);
	if (creature)
		return creature->pos;

	return Position(0, 0, 0);
}

void Player::GetStepOffset(float& x, float& y) {
	Creature* creature = Creature::GetFromKnown(creatureID);
	if (creature) {
		if (creature->newPos == Position(0, 0, 0)) {
			x = (creature->pos.x - creature->oldPos.x) * creature->GetStep();
			y = (creature->pos.y - creature->oldPos.y) * creature->GetStep();
		}
		else {
			x = (creature->newPos.x - creature->pos.x) * (1.0f - creature->GetStep());
			y = (creature->newPos.y - creature->pos.y) * (1.0f - creature->GetStep());
		}
	}
	else {
		x = 0.0f;
		y = 0.0f;
	}
}


//Lua function

int Player::LuaGetPlayerID(lua_State* L) {
	Game* game = Game::game;
	Player* player = (game ? game->GetPlayer() : NULL);

	double value = (player ? Player::GetCreatureID() : 0);

	LuaScript::PushNumber(L, value);
	return 1;
}

int Player::LuaGetPlayerPosition(lua_State* L) {
	Game* game = Game::game;
	Player* player = (game ? game->GetPlayer() : NULL);

	Position pos(0, 0, 0);
	if (player)
		pos = player->GetPosition();

	LuaScript::PushPosition(L, pos);
	return 1;
}

int Player::LuaGetPlayerStep(lua_State* L) {
	Game* game = Game::game;
	Player* player = (game ? game->GetPlayer() : NULL);
	Creature* creature = (player ? player->GetCreature() : NULL);

	double value = 0;
	if (creature)
		value = creature->GetStep();

	LuaScript::PushNumber(L, value);
	return 1;
}

int Player::LuaGetPlayerHealth(lua_State* L) {
	Game* game = Game::game;
	Player* player = (game ? game->GetPlayer() : NULL);
	Statistics* stats = (player ? player->GetStatistics() : NULL);

	double value = 0;
	if (stats)
		value = stats->health;

	LuaScript::PushNumber(L, value);
	return 1;
}

int Player::LuaGetPlayerMaxHealth(lua_State* L) {
	Game* game = Game::game;
	Player* player = (game ? game->GetPlayer() : NULL);
	Statistics* stats = (player ? player->GetStatistics() : NULL);

	double value = 0;
	if (stats)
		value = stats->maxHealth;

	LuaScript::PushNumber(L, value);
	return 1;
}

int Player::LuaGetPlayerHealthP(lua_State* L) {
	Game* game = Game::game;
	Player* player = (game ? game->GetPlayer() : NULL);
	Statistics* stats = (player ? player->GetStatistics() : NULL);

	double value = 0;
	if (stats)
		value = (int)(((double)stats->health / stats->maxHealth) * 100);

	LuaScript::PushNumber(L, value);
	return 1;
}

int Player::LuaGetPlayerMana(lua_State* L) {
	Game* game = Game::game;
	Player* player = (game ? game->GetPlayer() : NULL);
	Statistics* stats = (player ? player->GetStatistics() : NULL);

	double value = 0;
	if (stats)
		value = stats->mana;

	LuaScript::PushNumber(L, value);
	return 1;
}

int Player::LuaGetPlayerMaxMana(lua_State* L) {
	Game* game = Game::game;
	Player* player = (game ? game->GetPlayer() : NULL);
	Statistics* stats = (player ? player->GetStatistics() : NULL);

	double value = 0;
	if (stats)
		value = stats->maxMana;

	LuaScript::PushNumber(L, value);
	return 1;
}

int Player::LuaGetPlayerManaP(lua_State* L) {
	Game* game = Game::game;
	Player* player = (game ? game->GetPlayer() : NULL);
	Statistics* stats = (player ? player->GetStatistics() : NULL);

	double value = 0;
	if (stats)
		value = (int)(((double)stats->mana / stats->maxMana) * 100);

	LuaScript::PushNumber(L, value);
	return 1;
}

int Player::LuaGetPlayerCapacity(lua_State* L) {
	Game* game = Game::game;
	Player* player = (game ? game->GetPlayer() : NULL);
	Statistics* stats = (player ? player->GetStatistics() : NULL);

	double value = 0;
	if (stats)
		value = stats->capacity;

	LuaScript::PushNumber(L, value);
	return 1;
}

int Player::LuaGetPlayerExperience(lua_State* L) {
	Game* game = Game::game;
	Player* player = (game ? game->GetPlayer() : NULL);
	Statistics* stats = (player ? player->GetStatistics() : NULL);

	double value = 0;
	if (stats)
		value = stats->experience;

	LuaScript::PushNumber(L, value);
	return 1;
}

int Player::LuaGetPlayerLevel(lua_State* L) {
	Game* game = Game::game;
	Player* player = (game ? game->GetPlayer() : NULL);
	Statistics* stats = (player ? player->GetStatistics() : NULL);

	double value = 0;
	if (stats)
		value = stats->level;

	LuaScript::PushNumber(L, value);
	return 1;
}

int Player::LuaGetPlayerLevelP(lua_State* L) {
	Game* game = Game::game;
	Player* player = (game ? game->GetPlayer() : NULL);
	Statistics* stats = (player ? player->GetStatistics() : NULL);

	double value = 0;
	if (stats)
		value = stats->level_p;

	LuaScript::PushNumber(L, value);
	return 1;
}

int Player::LuaGetPlayerMagicLevel(lua_State* L) {
	Game* game = Game::game;
	Player* player = (game ? game->GetPlayer() : NULL);
	Statistics* stats = (player ? player->GetStatistics() : NULL);

	double value = 0;
	if (stats)
		value = stats->magicLevel;

	LuaScript::PushNumber(L, value);
	return 1;
}

int Player::LuaGetPlayerMagicLevelP(lua_State* L) {
	Game* game = Game::game;
	Player* player = (game ? game->GetPlayer() : NULL);
	Statistics* stats = (player ? player->GetStatistics() : NULL);

	double value = 0;
	if (stats)
		value = stats->magicLevel_p;

	LuaScript::PushNumber(L, value);
	return 1;
}

int Player::LuaGetPlayerSoul(lua_State* L) {
	Game* game = Game::game;
	Player* player = (game ? game->GetPlayer() : NULL);
	Statistics* stats = (player ? player->GetStatistics() : NULL);

	double value = 0;
	if (stats)
		value = stats->soul;

	LuaScript::PushNumber(L, value);
	return 1;
}

int Player::LuaGetPlayerStamina(lua_State* L) {
	Game* game = Game::game;
	Player* player = (game ? game->GetPlayer() : NULL);
	Statistics* stats = (player ? player->GetStatistics() : NULL);

	double value = 0;
	if (stats)
		value = stats->stamina;

	LuaScript::PushNumber(L, value);
	return 1;
}

int Player::LuaGetPlayerSkill(lua_State* L) {
	int skill = LuaScript::PopNumber(L);

	Game* game = Game::game;
	Player* player = (game ? game->GetPlayer() : NULL);
	Statistics* stats = (player ? player->GetStatistics() : NULL);

	double value = 0;
	if (stats)
		value = stats->skill[skill];

	LuaScript::PushNumber(L, value);
	return 1;
}

int Player::LuaGetPlayerSkillP(lua_State* L) {
	int skill = LuaScript::PopNumber(L);

	Game* game = Game::game;
	Player* player = (game ? game->GetPlayer() : NULL);
	Statistics* stats = (player ? player->GetStatistics() : NULL);

	double value = 0;
	if (stats)
		value = stats->skill_p[skill];

	LuaScript::PushNumber(L, value);
	return 1;
}

int Player::LuaGetPlayerAttackID(lua_State* L) {
	Game* game = Game::game;
	Player* player = (game ? game->GetPlayer() : NULL);

	double value = (player ? Player::GetAttackID() : 0);

	LuaScript::PushNumber(L, value);
	return 1;
}

int Player::LuaGetPlayerFollowID(lua_State* L) {
	Game* game = Game::game;
	Player* player = (game ? game->GetPlayer() : NULL);

	double value = (player ? Player::GetFollowID() : 0);

	LuaScript::PushNumber(L, value);
	return 1;
}

int Player::LuaGetPlayerSelectID(lua_State* L) {
	Game* game = Game::game;
	Player* player = (game ? game->GetPlayer() : NULL);

	double value = (player ? Player::GetSelectID() : 0);

	LuaScript::PushNumber(L, value);
	return 1;
}

int Player::LuaGetPlayerBattle(lua_State* L) {
	Game* game = Game::game;
	Player* player = (game ? game->GetPlayer() : NULL);
	Map* map = (game ? game->GetMap() : NULL);
	Battle* battle = (map ? map->GetBattle() : NULL);

	std::string list;
	std::list<Creature*> creatures;
	if (player && battle) {
		creatures = battle->GetCreatures();

		lua_newtable(L);
		std::list<Creature*>::iterator it = creatures.begin();
		int i = 1;
		for (it; it != creatures.end(); it++, i++) {
			Creature* creature = *it;
			lua_pushnumber(L, i);
			lua_pushnumber(L, creature->GetID());
			lua_settable(L, -3);
		}
		return 1;
	}

	lua_pushnil(L);
	return 1;
}

int Player::LuaGetPlayerInventory(lua_State* L) {
	Game* game = Game::game;
	Player* player = (game ? game->GetPlayer() : NULL);
	Container* inventory = (player ? player->GetInventory() : NULL);

	LuaScript::PushNumber(L, (unsigned long)inventory);
	return 1;
}

int Player::LuaGetPlayerInventoryItem(lua_State* L) {
	int slot = LuaScript::PopNumber(L);

	Game* game = Game::game;
	Player* player = (game ? game->GetPlayer() : NULL);
	Container* inventory = (player ? player->GetInventory() : NULL);

	Item* item = NULL;
	if (inventory)
		item = inventory->GetItem(slot);

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

int Player::LuaPlayerCheckItemPresence(lua_State* L) {
	int params = lua_gettop(L);

	int itemSubType = 0;
	if (params > 1)
		itemSubType = LuaScript::PopNumber(L);
	int itemid = LuaScript::PopNumber(L);

	Game* game = Game::game;
	Player* player = (game ? game->GetPlayer() : NULL);

	Container* inventory = (player ? player->GetInventory() : NULL);

	std::list<Container*> containers = Container::GetContainers();
	if (inventory)
		containers.push_front(inventory);

	std::list<Container*>::iterator it = containers.begin();
	for (it; it != containers.end(); it++) {
		Container* container = *it;
		if (container && container->IsItemInContainer(itemid, itemSubType)) {
			LuaScript::PushNumber(L, 1);
			return 1;
		}
	}

	LuaScript::PushNumber(L, 0);
	return 1;
}


void Player::LuaRegisterFunctions(lua_State* L) {
	//getPlayerID() : playerID
	lua_register(L, "getPlayerID", Player::LuaGetPlayerID);

	//getPlayerPosition() : position{x, y, z}
	lua_register(L, "getPlayerPosition", Player::LuaGetPlayerPosition);

	//getPlayerStep() : step
	lua_register(L, "getPlayerStep", Player::LuaGetPlayerStep);

	//getPlayerHealth() : health
	lua_register(L, "getPlayerHealth", Player::LuaGetPlayerHealth);

	//getPlayerMaxHealth() : healthMax
	lua_register(L, "getPlayerMaxHealth", Player::LuaGetPlayerMaxHealth);

	//getPlayerHealthP() : healthPercent
	lua_register(L, "getPlayerHealthP", Player::LuaGetPlayerHealthP);

	//getPlayerMana() : mana
	lua_register(L, "getPlayerMana", Player::LuaGetPlayerMana);

	//getPlayerMaxMana() : manaMax
	lua_register(L, "getPlayerMaxMana", Player::LuaGetPlayerMaxMana);

	//getPlayerManaP() : manaPercent
	lua_register(L, "getPlayerManaP", Player::LuaGetPlayerManaP);

	//getPlayerCapacity() : capacity
	lua_register(L, "getPlayerCapacity", Player::LuaGetPlayerCapacity);

	//getPlayerExperience() : experience
	lua_register(L, "getPlayerExperience", Player::LuaGetPlayerExperience);

	//getPlayerLevel() : level
	lua_register(L, "getPlayerLevel", Player::LuaGetPlayerLevel);

	//getPlayerLevelP() : levelPercent
	lua_register(L, "getPlayerLevelP", Player::LuaGetPlayerLevelP);

	//getPlayerMagicLevel() : magicLevel
	lua_register(L, "getPlayerMagicLevel", Player::LuaGetPlayerMagicLevel);

	//getPlayerMagicLevelP() : magicLevelPercent
	lua_register(L, "getPlayerMagicLevelP", Player::LuaGetPlayerMagicLevelP);

	//getPlayerSoul() : soul
	lua_register(L, "getPlayerSoul", Player::LuaGetPlayerSoul);

	//getPlayerStamina() : stamina
	lua_register(L, "getPlayerStamina", Player::LuaGetPlayerStamina);

	//getPlayerSkill(num) : skill
	lua_register(L, "getPlayerSkill", Player::LuaGetPlayerSkill);

	//getPlayerSkillP(num) : skillPercent
	lua_register(L, "getPlayerSkillP", Player::LuaGetPlayerSkillP);

	//getPlayerAttackID() : attackID
	lua_register(L, "getPlayerAttackID", Player::LuaGetPlayerAttackID);

	//getPlayerFollowID() : followID
	lua_register(L, "getPlayerFollowID", Player::LuaGetPlayerFollowID);

	//getPlayerSelectID() : selectID
	lua_register(L, "getPlayerSelectID", Player::LuaGetPlayerSelectID);

	//getPlayerBattle() : battle{creatureID, ...}
	lua_register(L, "getPlayerBattle", Player::LuaGetPlayerBattle);

	//getPlayerInventory() : containerPtr
	lua_register(L, "getPlayerInventory", Player::LuaGetPlayerInventory);

	//getPlayerInventoryItem(slot) : item{id, count, stack, pointer}
	lua_register(L, "getPlayerInventoryItem", Player::LuaGetPlayerInventoryItem);

	//playerCheckItemPresence(itemid, itemSubType = 0) : state
	lua_register(L, "playerCheckItemPresence", Player::LuaPlayerCheckItemPresence);
}
