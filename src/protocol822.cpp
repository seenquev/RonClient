/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#include "protocol822.h"

#include "allocator.h"
#include "channel.h"
#include "cooldowns.h"
#include "messages.h"
#include "particle.h"
#include "position.h"
#include "player.h"
#include "questlog.h"
#include "shop.h"
#include "status.h"
#include "text.h"
#include "tools.h"
#include "window.h"

#ifndef NO_SOUND
	#include "adal.h"
	#include "sound.h"
#endif


// ---- Protocol ---- //

Protocol822::Protocol822(Game* game) : Protocol(game) { }

Protocol822::~Protocol822() {
	if (client.Connected())
		client.doClose();

	if (receiveLoop.joinable())
        receiveLoop.join();
}


unsigned short Protocol822::GetVersion() {
	return 822;
}
