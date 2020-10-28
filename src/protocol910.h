/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#ifndef __PROTOCOL910_H_
#define __PROTOCOL910_H_

#include "game.h"
#include "mthread.h"
#include "network.h"
#include "protocol.h"
#include "protocol870.h"

class Game;

class Protocol910 : public Protocol870 {
public:
	Protocol910(Game* game);
	virtual ~Protocol910();

	virtual unsigned short GetVersion();

	virtual void SendSay(unsigned char speakClass, Channel* channel, std::string message);

	virtual void ParseShop(NetworkMessage* msg);
	virtual void ParsePlayerStats(NetworkMessage* msg);
	virtual void ParsePlayerSkills(NetworkMessage* msg);
	virtual void ParseOpenChannel(NetworkMessage* msg);
	virtual void ParseCreatePrivateChannel(NetworkMessage* msg);
	virtual void ParseCreatureSpeak(NetworkMessage* msg);
	virtual void ParseTextMessage(NetworkMessage* msg);

	virtual void GetThing(NetworkMessage* msg, Thing*& thing, int& skip);
	virtual void GetTileDescription(NetworkMessage* msg, Position pos, Tile*& tile, int& skip);
	virtual void GetCreature(NetworkMessage* msg, Creature*& creature, bool known);
};

#endif //__PROTOCOL870_H_
