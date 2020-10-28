/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#ifndef __PROTOCOL840_H_
#define __PROTOCOL840_H_

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
#include "protocol822.h"
#include "rsa.h"

class Channel;
class Game;

class Protocol840 : public Protocol822 {
public:
	Protocol840(Game* game);
	virtual ~Protocol840();

	virtual unsigned short GetVersion();

	virtual bool LoginServer();
	virtual bool LoginGame();

	virtual void SendSay(unsigned char speakClass, Channel* channel, std::string message);
	virtual void SendPurchaseShop(unsigned short itemID, unsigned char type, unsigned char count, bool ignoreCap, bool inBackpack);
	virtual void ParseShop(NetworkMessage* msg);
	virtual void ParseShopSaleItemList(NetworkMessage* msg);
	virtual void ParsePlayerStats(NetworkMessage* msg);
	virtual void ParseCreatureSpeak(NetworkMessage* msg);
	virtual void ParseTextMessage(NetworkMessage* msg);
};

#endif //__PROTOCOL870_H_
