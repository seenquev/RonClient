/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#ifndef __PROTOCOL822_H_
#define __PROTOCOL822_H_

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
#include "rsa.h"

class Channel;
class Creature;
class Game;
class Map;
class Outfit;
class Tile;

class Protocol822 : public Protocol {
public:
	Protocol822(Game* game);
	virtual ~Protocol822();

	virtual unsigned short GetVersion();
};

#endif //__PROTOCOL870_H_
