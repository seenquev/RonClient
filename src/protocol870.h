/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#ifndef __PROTOCOL870_H_
#define __PROTOCOL870_H_

#include "channel.h"
#include "const.h"
#include "creature.h"
#include "distance.h"
#include "game.h"
#include "item.h"
#include "light.h"
#include "logger.h"
#include "magiceffect.h"
#include "map.h"
#include "mthread.h"
#include "network.h"
#include "protocol.h"
#include "protocol860.h"
#include "rsa.h"

class Channel;
class Game;
class Outfit;

class Protocol870 : public Protocol860 {
public:
	Protocol870(Game* game);
	virtual ~Protocol870();

	virtual unsigned short GetVersion();

	virtual void SendSay(unsigned char speakClass, Channel* channel, std::string message);
	virtual void SendSetOutfit(Outfit outfit);

	virtual void ParsePlayerStats(NetworkMessage* msg);
	virtual void ParseSpellCooldown(NetworkMessage* msg);
	virtual void ParseSpellGroupCooldown(NetworkMessage* msg);
	virtual void ParseCreatureSpeak(NetworkMessage* msg);
	virtual void ParseTextMessage(NetworkMessage* msg);
	virtual void ParseOutfitWindow(NetworkMessage* msg);

	virtual void GetOutfit(NetworkMessage* msg, Outfit* outfit);
};

#endif //__PROTOCOL870_H_
