/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#include <stdio.h>

#include "bot.h"

#include "allocator.h"
#include "game.h"
#include "iniloader.h"
#include "luascript.h"
#include "text.h"
#include "tools.h"


// ---- Bot ---- //

PlayersList Bot::friends;
PlayersList Bot::enemies;

HealingStruct Bot::dataHealing;
ManaRefillStruct Bot::dataManaRefill;
AimBotStruct Bot::dataAimBot;
CaveBotStruct Bot::dataCaveBot;
ScriptsBotStruct Bot::dataScriptsBot;

LuaScript Bot::luaScriptHealing;
LuaScript Bot::luaScriptManaRefill;
LuaScript Bot::luaScriptAimBot;
LuaScript Bot::luaScriptCaveBot;
LuaScript Bot::luaScriptsBot[8];

bool Bot::enabled = true;
MUTEX Bot::lockBot;


void Bot::SaveBot(std::string path) {
	LOCKCLASS lockClass(lockBot);

	CreateDirectory(path.c_str(), NULL);

	INILoader iniPlayers;
	int i = 0;
	for (PlayersList::iterator it = friends.begin(); it != friends.end(); it++, i++) {
		std::string name = *it;
		iniPlayers.SetValue("FRIENDS", name, i);
	}
	i = 0;
	for (PlayersList::iterator it = enemies.begin(); it != enemies.end(); it++, i++) {
		std::string name = *it;
		iniPlayers.SetValue("ENEMIES", name, i);
	}
	iniPlayers.SaveFile(path + "players.ini");

	INILoader iniHealing;
	iniHealing.SetValue("MINSELFHEALTH", value2str(dataHealing.minSelfHealth));
	iniHealing.SetValue("MINFRIENDHEALTH", value2str(dataHealing.minFriendHealth));
	iniHealing.SetValue("ITEMID", value2str(dataHealing.itemID));
	iniHealing.SetValue("ITEMSUBTYPE", value2str(dataHealing.itemSubType));
	iniHealing.SetValue("SELFHEALWORDS", dataHealing.selfHealWords);
	iniHealing.SetValue("FRIENDHEALWORDS", dataHealing.friendHealWords);
	iniHealing.SaveFile(path + "healing.ini");

	INILoader iniManaRefill;
	iniManaRefill.SetValue("MANASTART", value2str(dataManaRefill.manaStart));
	iniManaRefill.SetValue("MANAFINISH", value2str(dataManaRefill.manaFinish));
	iniManaRefill.SetValue("ITEMID", value2str(dataManaRefill.itemID));
	iniManaRefill.SetValue("ITEMSUBTYPE", value2str(dataManaRefill.itemSubType));
	iniManaRefill.SaveFile(path + "manarefill.ini");

	INILoader iniAimBot;
	iniAimBot.SetValue("AUTOTARGETING", value2str((int)dataAimBot.autoTargeting));
	iniAimBot.SetValue("ITEMID", value2str(dataAimBot.itemID));
	iniAimBot.SetValue("ITEMSUBTYPE", value2str(dataAimBot.itemSubType));
	iniAimBot.SetValue("OFFENSIVEWORDS", dataAimBot.offensiveWords);
	iniAimBot.SaveFile(path + "aimbot.ini");

	INILoader iniCaveBot;
	iniCaveBot.SetValue("CHECKBODY", value2str((int)dataCaveBot.checkBody));
	iniCaveBot.SetValue("MOVETOBODY", value2str((int)dataCaveBot.moveToBody));
	iniCaveBot.SetValue("EATFOOD", value2str((int)dataCaveBot.eatFood));
	iniCaveBot.SetValue("TAKELOOT", value2str((int)dataCaveBot.takeLoot));
	iniCaveBot.SetValue("LOOTBAG", value2str(dataCaveBot.lootBag));
	i = 0;
	for (std::list<TextString>::iterator it = dataCaveBot.foodList.begin(); it != dataCaveBot.foodList.end(); it++, i++) {
		std::string value = it->text;
		iniCaveBot.SetValue("FOODLIST", value, i);
	}
	i = 0;
	for (std::list<TextString>::iterator it = dataCaveBot.lootList.begin(); it != dataCaveBot.lootList.end(); it++, i++) {
		std::string value = it->text;
		iniCaveBot.SetValue("LOOTLIST", value, i);
	}
	iniCaveBot.SaveFile(path + "cavebot.ini");

	INILoader iniScriptsBot;
	for (int i = 0; i < 8; i++) {
		std::string name = std::string("SCRIPT") + value2str(i + 1);
		iniScriptsBot.SetValue(name, dataScriptsBot.filenames[i]);
	}
	iniScriptsBot.SaveFile(path + "scriptsbot.ini");
}

void Bot::LoadBot(std::string path) {
	LOCKCLASS lockClass(lockBot);

	INILoader iniPlayers;
	if (iniPlayers.OpenFile(path + "players.ini")) {
		std::string value;

		int i = 0;
		while((value = iniPlayers.GetValue("FRIENDS", i++)) != "")
			friends.push_back(value);

		i = 0;
		while((value = iniPlayers.GetValue("ENEMIES", i++)) != "")
			enemies.push_back(value);
	}

	INILoader iniHealing;
	if (iniHealing.OpenFile(path + "healing.ini")) {
		dataHealing.minSelfHealth = atoi(iniHealing.GetValue("MINSELFHEALTH").c_str());
		dataHealing.minFriendHealth = atoi(iniHealing.GetValue("MINFRIENDHEALTH").c_str());
		dataHealing.itemID = atoi(iniHealing.GetValue("ITEMID").c_str());
		dataHealing.itemSubType = atoi(iniHealing.GetValue("ITEMSUBTYPE").c_str());
		dataHealing.selfHealWords = iniHealing.GetValue("SELFHEALWORDS");
		dataHealing.friendHealWords = iniHealing.GetValue("FRIENDHEALWORDS");
	}

	INILoader iniManaRefill;
	if (iniManaRefill.OpenFile(path + "manarefill.ini")) {
		dataManaRefill.manaStart = atoi(iniManaRefill.GetValue("MANASTART").c_str());
		dataManaRefill.manaFinish = atoi(iniManaRefill.GetValue("MANAFINISH").c_str());
		dataManaRefill.itemID = atoi(iniManaRefill.GetValue("ITEMID").c_str());
		dataManaRefill.itemSubType = atoi(iniManaRefill.GetValue("ITEMSUBTYPE").c_str());
	}

	INILoader iniAimBot;
	if (iniAimBot.OpenFile(path + "aimbot.ini")) {
		dataAimBot.autoTargeting = (bool)atoi(iniAimBot.GetValue("AUTOTARGETING").c_str());
		dataAimBot.itemID = atoi(iniAimBot.GetValue("ITEMID").c_str());
		dataAimBot.itemSubType = atoi(iniAimBot.GetValue("ITEMSUBTYPE").c_str());
		dataAimBot.offensiveWords = iniAimBot.GetValue("OFFENSIVEWORDS");
	}

	INILoader iniCaveBot;
	if (iniCaveBot.OpenFile(path + "cavebot.ini")) {
		dataCaveBot.checkBody = (bool)atoi(iniCaveBot.GetValue("CHECKBODY").c_str());
		dataCaveBot.moveToBody = (bool)atoi(iniCaveBot.GetValue("MOVETOBODY").c_str());
		dataCaveBot.eatFood = (bool)atoi(iniCaveBot.GetValue("EATFOOD").c_str());
		dataCaveBot.takeLoot = (bool)atoi(iniCaveBot.GetValue("TAKELOOT").c_str());
		dataCaveBot.lootBag = 0;

		std::string value;
		int num = 0;
		dataCaveBot.foodList.clear();
		while((value = iniCaveBot.GetValue("FOODLIST", num++)) != "") {
			dataCaveBot.foodList.push_back(TextString(value));
		}

		num = 0;
		dataCaveBot.lootList.clear();
		while((value = iniCaveBot.GetValue("LOOTLIST", num++)) != "") {
			dataCaveBot.lootList.push_back(TextString(value));
		}
	}

	INILoader iniScriptsBot;
	if (iniScriptsBot.OpenFile(path + "scriptsbot.ini")) {
		for (int i = 0; i < 8; i++) {
			std::string name = std::string("SCRIPT") + value2str(i + 1);
			dataScriptsBot.filenames[i] = iniScriptsBot.GetValue(name);
		}
	}
}

void Bot::ReleaseBot() {
	LOCKCLASS lockClass(lockBot);

	friends.clear();
	enemies.clear();
	Stop();
}

void Bot::Stop() {
	luaScriptHealing.SetFlag(LUAFLAG_QUIT);
	luaScriptManaRefill.SetFlag(LUAFLAG_QUIT);
	luaScriptAimBot.SetFlag(LUAFLAG_QUIT);
	luaScriptCaveBot.SetFlag(LUAFLAG_QUIT);
	for (int i = 0; i < 8; i++)
		luaScriptsBot[i].SetFlag(LUAFLAG_QUIT);

	if (luaScriptHealing.scriptThread.joinable())
        luaScriptHealing.scriptThread.join();
    if (luaScriptManaRefill.scriptThread.joinable())
        luaScriptManaRefill.scriptThread.join();
    if (luaScriptAimBot.scriptThread.joinable())
        luaScriptAimBot.scriptThread.join();
    if (luaScriptCaveBot.scriptThread.joinable())
        luaScriptCaveBot.scriptThread.join();
	for (int i = 0; i < 8; i++)
        if (luaScriptsBot[i].scriptThread.joinable())
            luaScriptsBot[i].scriptThread.join();
}

void Bot::Run() {
	if (!enabled || Game::adminOptions.disableBot) {
		Game* game = Game::game;
		Windows* wnds = (game ? game->GetWindows() : NULL);
		if (wnds)
			wnds->OpenWindow(WND_MESSAGE, Text::GetText("ERROR_MESSAGE_6", Game::options.language).c_str());
		return;
	}

	if (dataHealing.selfHealing || dataHealing.friendHealing) {
		BotArguments args;
		args.push_back(value2str((dataHealing.selfHealing ? dataHealing.minSelfHealth : 0)).c_str());
		args.push_back(value2str((dataHealing.friendHealing ? dataHealing.minFriendHealth : 0)).c_str());
		args.push_back(value2str(dataHealing.itemID).c_str());
		args.push_back(value2str(dataHealing.itemSubType).c_str());
		args.push_back(std::string(dataHealing.selfHealWords).c_str());
		args.push_back(std::string(dataHealing.friendHealWords).c_str());

		if (luaScriptHealing.OpenScript("scripts/healing.lua"))
			luaScriptHealing.Execute("main", args, true);
	}

	if (dataManaRefill.manaRefill) {
		BotArguments args;
		args.push_back(value2str(dataManaRefill.manaStart));
		args.push_back(value2str(dataManaRefill.manaFinish));
		args.push_back(value2str(dataManaRefill.itemID));
		args.push_back(value2str(dataManaRefill.itemSubType));

		if (luaScriptManaRefill.OpenScript("scripts/manarefill.lua"))
			luaScriptManaRefill.Execute("main", args, true);
	}

	if (dataAimBot.aimBot) {
		BotArguments args;
		args.push_back(value2str((int)dataAimBot.autoTargeting));
		args.push_back(value2str(dataAimBot.itemID));
		args.push_back(value2str(dataAimBot.itemSubType));
		args.push_back(std::string(dataAimBot.offensiveWords));

		if (luaScriptAimBot.OpenScript("scripts/aimbot.lua"))
			luaScriptAimBot.Execute("main", args, true);
	}

	if (dataCaveBot.caveBot) {
		BotArguments args;
		args.push_back(value2str((int)dataCaveBot.checkBody));
		args.push_back(value2str((int)dataCaveBot.moveToBody));
		args.push_back(value2str((int)dataCaveBot.eatFood));
		args.push_back(value2str((int)dataCaveBot.takeLoot));
		args.push_back(value2str((int)dataCaveBot.lootBag));
		std::string foodList;
		for (std::list<TextString>::iterator it = dataCaveBot.foodList.begin(); it != dataCaveBot.foodList.end(); it++) {
			std::string value = it->text;
			foodList += (foodList != "" ? " " : "") + value;
		}
		std::string lootList;
		for (std::list<TextString>::iterator it = dataCaveBot.lootList.begin(); it != dataCaveBot.lootList.end(); it++) {
			std::string value = it->text;
			lootList += (lootList != "" ? " " : "") + value;
		}
		args.push_back(foodList);
		args.push_back(lootList);

		if (luaScriptCaveBot.OpenScript("scripts/cavebot.lua"))
			luaScriptCaveBot.Execute("main", args, true);
	}

	for (int i = 0; i < 8; i++) {
		if (dataScriptsBot.scriptsState[i] && dataScriptsBot.filenames[i] != "") {
			std::string name;
			std::string args;

			size_t pos = dataScriptsBot.filenames[i].find(';', 0);
			name = dataScriptsBot.filenames[i].substr(0, pos);
			if (pos != std::string::npos)
				args = dataScriptsBot.filenames[i].substr(pos + 1, std::string::npos);

			pos = 0;
			if ((pos = name.find(".lua", 0)) != std::string::npos)
				name = name.substr(0, pos);

			if (luaScriptsBot[i].OpenScript(LuaScript::GetFilePath(name)))
				luaScriptsBot[i].Execute("main", LuaScript::ConvertArgs(args), true);
		}
	}
}

void Bot::Restart() {
	Stop();
	Run();
}


void Bot::SetHealingData(HealingStruct hs) {
	LOCKCLASS lockClass(lockBot);

	dataHealing = hs;
}

HealingStruct Bot::GetHealingData() {
	LOCKCLASS lockClass(lockBot);

	return dataHealing;
}


void Bot::SetManaRefillData(ManaRefillStruct ms) {
	LOCKCLASS lockClass(lockBot);

	dataManaRefill = ms;
}

ManaRefillStruct Bot::GetManaRefillData() {
	LOCKCLASS lockClass(lockBot);

	return dataManaRefill;
}


void Bot::SetAimBotData(AimBotStruct as) {
	LOCKCLASS lockClass(lockBot);

	dataAimBot = as;
}

AimBotStruct Bot::GetAimBotData() {
	LOCKCLASS lockClass(lockBot);

	return dataAimBot;
}


void Bot::SetCaveBotData(CaveBotStruct cs) {
	LOCKCLASS lockClass(lockBot);

	dataCaveBot = cs;
}

CaveBotStruct Bot::GetCaveBotData() {
	LOCKCLASS lockClass(lockBot);

	return dataCaveBot;
}


void Bot::SetScriptsBotData(ScriptsBotStruct ss) {
	LOCKCLASS lockClass(lockBot);

	dataScriptsBot = ss;
}

ScriptsBotStruct Bot::GetScriptsBotData() {
	LOCKCLASS lockClass(lockBot);

	return dataScriptsBot;
}


void Bot::AddFriend(std::string name) {
	LOCKCLASS lockClass(lockBot);

	friends.push_back(name);
}

PlayersList Bot::GetFriends() {
	LOCKCLASS lockClass(lockBot);

	return friends;
}

void Bot::RemoveFriend(std::string name) {
	LOCKCLASS lockClass(lockBot);

	PlayersList::iterator it = friends.begin();
	while(it != friends.end()) {
		if (*it == name) {
			friends.erase(it);
			it = friends.begin();
		}
		else
			it++;
	}
}

void Bot::ClearFriends() {
	LOCKCLASS lockClass(lockBot);

	friends.clear();
}

void Bot::AddEnemie(std::string name) {
	LOCKCLASS lockClass(lockBot);

	enemies.push_back(name);
}

PlayersList Bot::GetEnemies() {
	LOCKCLASS lockClass(lockBot);

	return enemies;
}

void Bot::RemoveEnemie(std::string name) {
	LOCKCLASS lockClass(lockBot);

	PlayersList::iterator it = enemies.begin();
	while(it != enemies.end()) {
		if (*it == name) {
			enemies.erase(it);
			it = enemies.begin();
		}
		else
			it++;
	}
}

void Bot::ClearEnemies() {
	LOCKCLASS lockClass(lockBot);

	enemies.clear();
}


//Lua functions

int Bot::LuaGetFriends(lua_State* L) {
	LOCKCLASS lockClass(lockBot);

	lua_newtable(L);
	int pos = 1;
	for (PlayersList::reverse_iterator it = friends.rbegin(); it != friends.rend(); it++, pos++) {
		std::string name = *it;

		lua_pushnumber(L, pos);
		lua_pushstring(L, name.c_str());
		lua_settable(L, -3);
	}

	return 1;
}

int Bot::LuaGetEnemies(lua_State* L) {
	LOCKCLASS lockClass(lockBot);

	lua_newtable(L);
	int pos = 1;
	for (PlayersList::reverse_iterator it = enemies.rbegin(); it != enemies.rend(); it++, pos++) {
		std::string name = *it;

		lua_pushnumber(L, pos);
		lua_pushstring(L, name.c_str());
		lua_settable(L, -3);
	}

	return 1;
}


void Bot::LuaRegisterFunctions(lua_State* L) {
	//getBotFriends() : friends{nameStr, ...}
	lua_register(L, "getBotFriends", Bot::LuaGetFriends);

	//getBotEnemies() : enemies{nameStr, ...}
	lua_register(L, "getBotEnemies", Bot::LuaGetEnemies);
}
