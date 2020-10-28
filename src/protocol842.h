/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#ifndef __PROTOCOL842_H_
#define __PROTOCOL842_H_

#include "const.h"
#include "creature.h"
#include "game.h"
#include "item.h"
#include "light.h"
#include "logger.h"
#include "map.h"
#include "mthread.h"
#include "network.h"
#include "protocol.h"
#include "protocol840.h"
#include "rsa.h"

class Game;

class Protocol842 : public Protocol840 {
public:
	Protocol842(Game* game);
	virtual ~Protocol842();

	virtual unsigned short GetVersion();

	virtual bool LoginGame();

	virtual void ParseAddThing(NetworkMessage* msg);
};

#endif //__PROTOCOL870_H_
