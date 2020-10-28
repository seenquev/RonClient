/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#include "protocol854.h"

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

Protocol854::Protocol854(Game* game) : Protocol850(game) { }

Protocol854::~Protocol854() {
	if (client.Connected())
		client.doClose();

	if (receiveLoop.joinable())
        receiveLoop.join();
}


unsigned short Protocol854::GetVersion() {
	return 854;
}


void Protocol854::GetCreature(NetworkMessage* msg, Creature*& creature, bool known) {
	if (known) {
		unsigned int creatureID = msg->GetU32();

		creature = Creature::GetFromKnown(creatureID);
		if (!creature) {
			Logger::AddLog("Protocol::GetCreature()", "Known creature pointer is NULL!", LOG_ERROR, msg);
			return;
		}
	}
	else {
		if (!creature) {
			Logger::AddLog("Protocol::GetCreature()", "Unknown creature pointer is NULL!", LOG_ERROR, msg);
			return;
		}

		unsigned int removeID = msg->GetU32();
		unsigned int creatureID = msg->GetU32();
		std::string creatureName = msg->GetString();

		Creature::RemoveFromKnown(removeID);
		creature->SetID(creatureID);
		creature->SetName(creatureName);
		Creature::AddToKnown(creature);
	}

	unsigned char health = msg->GetU8();
	Direction direction = (Direction)msg->GetU8();

	creature->SetHealth(health);
	creature->SetDirection(direction);

	Outfit outfit;
	GetOutfit(msg, &outfit);

	creature->SetOutfit(outfit);

	unsigned char lightLevel = msg->GetU8();
	unsigned char lightColor = msg->GetU8();

	creature->SetLight(lightColor, lightLevel);

	unsigned short speed = msg->GetU16();

	creature->SetSpeed(speed);

	unsigned char skull = msg->GetU8();
	unsigned char shield = msg->GetU8();

	creature->SetSkull(skull);
	creature->SetShield(shield);

	if (!known) {
		unsigned char war = msg->GetU8();
		creature->SetWar(war);
	}

	bool blocking = (bool)msg->GetU8();

	creature->SetBlocking(blocking);
}
