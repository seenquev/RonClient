/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#ifndef __PROTOCOL860_H_
#define __PROTOCOL860_H_

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
#include "protocol854.h"
#include "rsa.h"

class Game;

class Protocol860 : public Protocol854 {
public:
	Protocol860(Game* game);
	virtual ~Protocol860();

	virtual unsigned short GetVersion();

	virtual void ParseCancelAttack(NetworkMessage* msg);
};

#endif //__PROTOCOL870_H_
