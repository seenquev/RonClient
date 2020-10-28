/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#ifndef __BOT_H_
#define __BOT_H_

#include <list>
#include <string>

#include "mthread.h"
#include "luascript.h"
#include "tools.h"


typedef std::list<std::string> BotArguments;
typedef std::list<std::string> PlayersList;


struct HealingStruct {
	bool selfHealing;
	bool friendHealing;
	unsigned char minSelfHealth;
	unsigned char minFriendHealth;

	unsigned short itemID;
	unsigned char itemSubType;
	std::string selfHealWords;
	std::string friendHealWords;

	HealingStruct() {
		selfHealing = false;
		friendHealing = false;
		minSelfHealth = 0;
		minFriendHealth = 0;
		itemID = 0;
		itemSubType = 0;
	}
};

struct ManaRefillStruct {
	bool manaRefill;
	unsigned char manaStart;
	unsigned char manaFinish;

	unsigned short itemID;
	unsigned char itemSubType;

	ManaRefillStruct() {
		manaRefill = false;
		manaStart = 0;
		manaFinish = 100;
		itemID = 0;
		itemSubType = 0;
	}
};

struct AimBotStruct {
	bool aimBot;

	bool autoTargeting;

	unsigned short itemID;
	unsigned char itemSubType;
	std::string offensiveWords;

	AimBotStruct() {
		aimBot = false;
		autoTargeting = false;
		itemID = 0;
		itemSubType = 0;
	}
};

struct CaveBotStruct {
	bool caveBot;
	bool checkBody;
	bool moveToBody;
	bool eatFood;
	bool takeLoot;

	std::list<TextString> foodList;
	std::list<TextString> lootList;

	unsigned char lootBag;

	CaveBotStruct() {
		caveBot = false;
		checkBody = false;
		moveToBody = false;
		eatFood = false;
		takeLoot = false;
		lootBag = 0;
	}
};

struct ScriptsBotStruct {
	bool scriptsState[8];
	std::string filenames[8];

	ScriptsBotStruct() {
		memset(scriptsState, 0, 8);
	}
};


class Bot {
private:
	static PlayersList		friends;
	static PlayersList		enemies;

	static HealingStruct	dataHealing;
	static ManaRefillStruct	dataManaRefill;
	static AimBotStruct		dataAimBot;
	static CaveBotStruct	dataCaveBot;
	static ScriptsBotStruct	dataScriptsBot;

	static LuaScript		luaScriptHealing;
	static LuaScript		luaScriptManaRefill;
	static LuaScript		luaScriptAimBot;
	static LuaScript		luaScriptCaveBot;
	static LuaScript		luaScriptsBot[8];

public:
	static bool		enabled;
	static MUTEX	lockBot;

public:
	static void SaveBot(std::string path);
	static void LoadBot(std::string path);
	static void ReleaseBot();

	static void Stop();
	static void Run();
	static void Restart();

	static void SetHealingData(HealingStruct hs);
	static HealingStruct GetHealingData();

	static void SetManaRefillData(ManaRefillStruct ms);
	static ManaRefillStruct GetManaRefillData();

	static void SetAimBotData(AimBotStruct as);
	static AimBotStruct GetAimBotData();

	static void SetCaveBotData(CaveBotStruct cs);
	static CaveBotStruct GetCaveBotData();

	static void SetScriptsBotData(ScriptsBotStruct cs);
	static ScriptsBotStruct GetScriptsBotData();

	static void AddFriend(std::string name);
	static PlayersList GetFriends();
	static void RemoveFriend(std::string name);
	static void ClearFriends();

	static void AddEnemie(std::string name);
	static PlayersList GetEnemies();
	static void RemoveEnemie(std::string name);
	static void ClearEnemies();

	//Lua functions
	static int LuaGetFriends(lua_State* L);
	static int LuaGetEnemies(lua_State* L);

	static void LuaRegisterFunctions(lua_State* L);
};

#endif //__BOT_H_
