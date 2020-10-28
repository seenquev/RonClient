/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#ifndef __PROTOCOL850_H_
#define __PROTOCOL850_H_

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
#include "protocol842.h"
#include "rsa.h"

class Channel;
class Game;

class Protocol850 : public Protocol842 {
public:
	Protocol850(Game* game);
	virtual ~Protocol850();

	virtual unsigned short GetVersion();

	virtual void ParseViolation(NetworkMessage* msg);
};

#endif //__PROTOCOL870_H_
