/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#include "protocol860.h"

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

Protocol860::Protocol860(Game* game) : Protocol854(game) { }

Protocol860::~Protocol860() {
	if (client.Connected())
		client.doClose();

	if (receiveLoop.joinable())
        receiveLoop.join();
}


unsigned short Protocol860::GetVersion() {
	return 860;
}


void Protocol860::ParseCancelAttack(NetworkMessage* msg) {
	unsigned int creatureID = msg->GetU32();

	Player* player = game->GetPlayer();
	if (player) {
		player->SetAttackID(0);

		Player::walking = false;
		Player::requestStop = false;
		player->SetTargetPos(Position(0, 0, 0));
	}
}
