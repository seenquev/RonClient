/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#ifndef __PROTOCOL_H_
#define __PROTOCOL_H_

#include "channel.h"
#include "const.h"
#include "creature.h"
#include "distance.h"
#include "game.h"
#include "item.h"
#include "light.h"
#include "logger.h"
#include "luascript.h"
#include "magiceffect.h"
#include "map.h"
#include "mthread.h"
#include "network.h"
#include "rsa.h"


class Channel;
class Creature;
class Game;
class Map;
class Outfit;
class Tile;


static const char* protsStr[] = {"8", "822", "840", "842", "850", "854", "860", "870", "910"};


class Protocol {
protected:
	Game*			game;

	RSA				rsa;
	unsigned int	XTEAkey[4];

	Client		client;

	THREAD	receiveLoop;
	MUTEX	lockProtocol;

	Signal	sig;

public:
	Protocol(Game* game);
	virtual ~Protocol();

	virtual unsigned short GetVersion() = 0;

	void SetRSA(RSA* rsa);
	void RandomizeXTEA();

	void CloseConnection();

	virtual bool LoginServer();
	virtual bool LoginGame();

	virtual void SendLogout();
	virtual void SendAutoWalk(std::list<Direction>& list);
	virtual void SendWalk(Direction direction);
	virtual void SendTurn(Direction direction);
	virtual void SendStopWalk();
	virtual void SendSay(unsigned char speakClass, Channel* channel, std::string message);
	virtual void SendLookAt(Position pos, unsigned short itemId, unsigned char stackPos);
	virtual void SendMoveThing(Position fromPos, unsigned short itemId, unsigned char stackPos, Position toPos, unsigned char count);
	virtual void SendUseThing(Position pos, unsigned short itemId, unsigned char stackPos, unsigned char index);
	virtual void SendUseWithThing(Position fromPos, unsigned short fromItemId, unsigned char fromStackPos, Position toPos, unsigned short toItemId, unsigned char toStackPos);
	virtual void SendUseBattleThing(Position fromPos, unsigned short fromItemId, unsigned char fromStackPos, unsigned int creatureID);
	virtual void SendRotateItem(Position pos, unsigned short itemId, unsigned char stackPos);
	virtual void SendRequestTrade(Position pos, unsigned short itemId, unsigned char stackPos, unsigned int creatureID);
	virtual void SendLookInTrade(unsigned char counterOffer, unsigned char index);
	virtual void SendAcceptTrade();
	virtual void SendCancelTrade();
	virtual void SendLookInShop(unsigned short itemID, unsigned char type);
	virtual void SendPurchaseShop(unsigned short itemID, unsigned char type, unsigned char count, bool ignoreCap, bool inBackpack);
	virtual void SendSaleShop(unsigned short itemID, unsigned char type, unsigned char count);
	virtual void SendCloseShop();
	virtual void SendContainerClose(unsigned char index);
	virtual void SendContainerMoveUp(unsigned char index);
	virtual void SendToggleMount(bool mount);
	virtual void SendFightModes(unsigned char fight, unsigned char follow, unsigned char attack);
	virtual void SendAttack(unsigned int creatureID);
	virtual void SendFollow(unsigned int creatureID);
	virtual void SendCreatePrivateChannel();
	virtual void SendChannelInvite(std::string creatureName);
	virtual void SendChannelExclude(std::string creatureName);
	virtual void SendRequestChannels();
	virtual void SendOpenChannel(unsigned short channelID);
	virtual void SendCloseChannel(unsigned short channelID);
	virtual void SendOpenPrivateChannel(std::string channelName);
	virtual void SendProcessRuleViolation(std::string reporter);
	virtual void SendCloseRuleViolation(std::string reporter);
	virtual void SendCancelRuleViolation();
	virtual void SendAddVIP(std::string creatureName);
	virtual void SendRemoveVIP(unsigned int creatureID);
	virtual void SendTextWindow(unsigned int textID, std::string text);
	virtual void SendHouseWindow(unsigned char listID, unsigned int textID, std::string text);
	virtual void SendRequestOutfit();
	virtual void SendSetOutfit(Outfit outfit);
	virtual void SendPartyInvite(unsigned int creatureID);
	virtual void SendPartyJoin(unsigned int creatureID);
	virtual void SendPartyRevoke(unsigned int creatureID);
	virtual void SendPartyPassLeadership(unsigned int creatureID);
	virtual void SendPartyLeave();
	virtual void SendPartyEnableShared(unsigned char active);
	virtual void SendRequestQuestLog();
	virtual void SendRequestQuestLine(unsigned short questID);
	virtual void SendCloseNPC();
	virtual void SendError(std::string debug, std::string date, std::string description, std::string comment);
	//Extended Protocol
	virtual void SendRequestSpells();

	virtual void ParseMessageBox(NetworkMessage* msg);
	virtual void ParseMotd(NetworkMessage* msg);
	virtual void ParseFYIBox(NetworkMessage* msg);
	virtual void ParsePlayerAppear(NetworkMessage* msg);
	virtual void ParseViolation(NetworkMessage* msg);
	virtual void ParsePlacedInQueue(NetworkMessage* msg);
	virtual void ParsePing(NetworkMessage* msg);
	virtual void ParseReLogin(NetworkMessage* msg);
	virtual void ParseCharactersList(NetworkMessage* msg);
	virtual void ParseMapDescriptor(NetworkMessage* msg);
	virtual void ParseMoveMapUp(NetworkMessage* msg);
	virtual void ParseMoveMapRight(NetworkMessage* msg);
	virtual void ParseMoveMapDown(NetworkMessage* msg);
	virtual void ParseMoveMapLeft(NetworkMessage* msg);
	virtual void ParseTileDescription(NetworkMessage* msg);
	virtual void ParseAddThing(NetworkMessage* msg);
	virtual void ParseTransformThing(NetworkMessage* msg);
	virtual void ParseRemoveThing(NetworkMessage* msg);
	virtual void ParseMoveThing(NetworkMessage* msg);
	virtual void ParseOpenContainer(NetworkMessage* msg);
	virtual void ParseCloseContainer(NetworkMessage* msg);
	virtual void ParseAddItemContainer(NetworkMessage* msg);
	virtual void ParseTransformItemContainer(NetworkMessage* msg);
	virtual void ParseRemoveItemContainer(NetworkMessage* msg);
	virtual void ParseUpdateInventoryItem(NetworkMessage* msg);
	virtual void ParseRemoveInventoryItem(NetworkMessage* msg);
	virtual void ParseShop(NetworkMessage* msg);
	virtual void ParseShopSaleItemList(NetworkMessage* msg);
	virtual void ParseCloseShop(NetworkMessage* msg);
	virtual void ParseAcceptTrade(NetworkMessage* msg);
	virtual void ParseRequestTrade(NetworkMessage* msg);
	virtual void ParseCloseTrade(NetworkMessage* msg);
	virtual void ParseWorldLight(NetworkMessage* msg);
	virtual void ParseMagicEffect(NetworkMessage* msg);
	virtual void ParseAnimatedText(NetworkMessage* msg);
	virtual void ParseDistanceShot(NetworkMessage* msg);
	virtual void ParseCreatureSquare(NetworkMessage* msg);
	virtual void ParseCreatureHealth(NetworkMessage* msg);
	virtual void ParseCreatureLight(NetworkMessage* msg);
	virtual void ParseCreatureOutfit(NetworkMessage* msg);
	virtual void ParseCreatureSpeed(NetworkMessage* msg);
	virtual void ParseCreatureSkull(NetworkMessage* msg);
	virtual void ParseCreatureShield(NetworkMessage* msg);
	virtual void ParseCreaturePZSwitch(NetworkMessage* msg);
	virtual void ParseTextWindow(NetworkMessage* msg);
	virtual void ParseHouseWindow(NetworkMessage* msg);
	virtual void ParsePlayerStats(NetworkMessage* msg);
	virtual void ParsePlayerSkills(NetworkMessage* msg);
	virtual void ParsePlayerIcons(NetworkMessage* msg);
	virtual void ParseCancelAttack(NetworkMessage* msg);
	virtual void ParseSpellCooldown(NetworkMessage* msg);
	virtual void ParseSpellGroupCooldown(NetworkMessage* msg);
	virtual void ParseCreatureSpeak(NetworkMessage* msg);
	virtual void ParseChannels(NetworkMessage* msg);
	virtual void ParseOpenChannel(NetworkMessage* msg);
	virtual void ParseOpenPrivate(NetworkMessage* msg);
	virtual void ParseCreatePrivateChannel(NetworkMessage* msg);
	virtual void ParseCloseChannel(NetworkMessage* msg);
	virtual void ParseTextMessage(NetworkMessage* msg);
	virtual void ParseCancelWalk(NetworkMessage* msg);
	virtual void ParseMovePlayerUp(NetworkMessage* msg);
	virtual void ParseMovePlayerDown(NetworkMessage* msg);
	virtual void ParseOutfitWindow(NetworkMessage* msg);
	virtual void ParseVIPList(NetworkMessage* msg);
	virtual void ParseVIPLogin(NetworkMessage* msg);
	virtual void ParseVIPLogout(NetworkMessage* msg);
	virtual void ParseTutorial(NetworkMessage* msg);
	virtual void ParseMarker(NetworkMessage* msg);
	virtual void ParseQuestLog(NetworkMessage* msg);
	virtual void ParseQuestLine(NetworkMessage* msg);
	//Extended Protocol
	virtual void ParseStatusPeriods(NetworkMessage* msg);
	virtual void ParseSpellsWindow(NetworkMessage* msg);
	virtual void ParseScreenShot(NetworkMessage* msg);
	virtual void ParseCreatureEquipment(NetworkMessage* msg);
	virtual void ParseSound(NetworkMessage* msg);
	virtual void ParseManageBot(NetworkMessage* msg);
	virtual void ParseDirectionArrow(NetworkMessage* msg);
	virtual void ParsePositionArrow(NetworkMessage* msg);
	virtual void ParseScriptPacket(NetworkMessage* msg);

	virtual void ParsePacketLogin(NetworkMessage* msg);
	virtual void ParsePacketGame(NetworkMessage* msg);

	virtual void GetPosition(NetworkMessage* msg, Position& position);
	virtual void GetThing(NetworkMessage* msg, Thing*& thing, int& skip);
	virtual void GetMapDescription(NetworkMessage* msg, int x, int y, int width, int height, Position pos, Map* map);
	virtual void GetFloorDescription(NetworkMessage* msg, int x, int y, int z, int width, int height, Position pos, Map* map, int& skip);
	virtual void GetTileDescription(NetworkMessage* msg, Position pos, Tile*& tile, int& skip);
	virtual void GetCreature(NetworkMessage* msg, Creature*& creature, bool known);
	virtual void GetOutfit(NetworkMessage* msg, Outfit* outfit);

	void StartReceiveLoop();
	void ReceiveLoop(NetworkMessage* msg);
	void onReceiveLoopEnd();

	//Lua functions
	static int LuaGetVersion(lua_State* L);
	static int LuaNewNetworkMessage(lua_State* L);
	static int LuaDeleteNetworkMessage(lua_State* L);
	static int LuaClearNetworkMessage(lua_State* L);
	static int LuaGetNetworkMessageSize(lua_State* L);
	static int LuaSendNetworkMessage(lua_State* L);
	static int LuaGetNetworkMessageData(lua_State* L);
	static int LuaAddNetworkMessageData(lua_State* L);
	static int LuaEndNetworkMessage(lua_State* L);

	static void LuaRegisterFunctions(lua_State* L);

	friend class Game;
};

#endif //__PROTOCOL_H_
