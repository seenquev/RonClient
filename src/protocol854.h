/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#ifndef __PROTOCOL854_H_
#define __PROTOCOL854_H_

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
#include "protocol850.h"
#include "rsa.h"

class Creature;
class Game;

class Protocol854 : public Protocol850 {
public:
	Protocol854(Game* game);
	virtual ~Protocol854();

	virtual unsigned short GetVersion();

	virtual void GetCreature(NetworkMessage* msg, Creature*& creature, bool known);
};

#endif //__PROTOCOL870_H_
