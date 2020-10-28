/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#include "protocol870.h"

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

Protocol870::Protocol870(Game* game) : Protocol860(game) { }

Protocol870::~Protocol870() {
	if (client.Connected())
		client.doClose();

	if (receiveLoop.joinable())
        receiveLoop.join();
}


unsigned short Protocol870::GetVersion() {
	return 870;
}


void Protocol870::SendSay(unsigned char speakClass, Channel* channel, std::string message) {
	LOCKCLASS lockClass(lockProtocol);

	if (!channel) {
		Logger::AddLog("Protocol::SendSay()", "Pointer to channel is NULL!", LOG_WARNING);
		return;
	}

	NetworkMessage msg(GetVersion(), XTEAkey);

	msg.AddU8(0x96);
	msg.AddU8(speakClass);
	switch(speakClass) {
		case SPEAK870_PRIVATE:
		case SPEAK870_PRIVATE_RED:
			msg.AddString(channel->GetName());
			break;
		case SPEAK870_CHANNEL_Y:
		case SPEAK870_CHANNEL_W:
		case SPEAK870_CHANNEL_O:
		case SPEAK870_CHANNEL_R1:
			msg.AddU16(channel->GetID());
			break;
		default:
			break;
	};
	msg.AddString(message);

	msg.SendMessage(&client);
}

void Protocol870::SendSetOutfit(Outfit outfit) {
	LOCKCLASS lockClass(lockProtocol);

	NetworkMessage msg(GetVersion(), XTEAkey);

	msg.AddU8(0xD3);
	msg.AddU16(outfit.lookType);
	msg.AddU8(outfit.lookHead);
	msg.AddU8(outfit.lookBody);
	msg.AddU8(outfit.lookLegs);
	msg.AddU8(outfit.lookFeet);
	msg.AddU8(outfit.lookAddons);
	msg.AddU16(outfit.lookMount);

	msg.SendMessage(&client);
}


void Protocol870::ParsePlayerStats(NetworkMessage* msg) {
	Player* player = game->GetPlayer();
	Statistics* statistics = player->GetStatistics();

	statistics->health = msg->GetU16();
	statistics->maxHealth = msg->GetU16();

	statistics->capacity = (double)(msg->GetU32()) / 100;
	statistics->experience = msg->GetU64();

	statistics->level = msg->GetU16();
	statistics->level_p = msg->GetU8();

	statistics->mana = msg->GetU16();
	statistics->maxMana = msg->GetU16();

	statistics->magicLevel = msg->GetU8();
	statistics->magicLevel_p = msg->GetU8();

	statistics->soul = msg->GetU8();

	statistics->stamina = msg->GetU16();
}

void Protocol870::ParseSpellCooldown(NetworkMessage* msg) {
	unsigned char spellID = msg->GetU8();
	unsigned int time = msg->GetU32();

	Cooldowns* cooldowns = game->GetCooldowns();
	if (cooldowns) {
		Cooldown* cooldown = new(M_PLACE) Cooldown;
		cooldown->Cast(time);
		cooldown->SetComment(Text::GetText("COMMENT_0", Game::options.language) + " " + value2str(spellID));

		cooldowns->AddCooldown(spellID, cooldown);
		cooldowns->UpdateContainerSpell();
	}
}

void Protocol870::ParseSpellGroupCooldown(NetworkMessage* msg) {
	unsigned char groupID = msg->GetU8();
	unsigned int time = msg->GetU32();

	Cooldowns* cooldowns = game->GetCooldowns();
	if (cooldowns) {
		Cooldown* cooldown = cooldowns->GetGroupCooldown(groupID);
		if (cooldown)
			cooldown->Cast(time);

		cooldowns->UpdateContainerGroup();
	}
}

void Protocol870::ParseCreatureSpeak(NetworkMessage* msg) {
	unsigned int creatureID = msg->GetU32();
	std::string creatureName = msg->GetString();
	unsigned short level = msg->GetU16();

	unsigned char type = msg->GetU8();

	Position pos(0, 0, 0);
	Channel* channel = NULL;
	unsigned char color = 215;
	switch(type) {
		case SPEAK870_SAY:
			GetPosition(msg, pos);
			color = 210;
			break;
		case SPEAK870_WHISPER:
			GetPosition(msg, pos);
			color = 200;
			break;
		case SPEAK870_YELL:
			GetPosition(msg, pos);
			color = 210;
			break;
		case SPEAK870_PRIVATE_PN:
			channel = Channel::GetChannel(CHANNEL_NPC);
			break;
		case SPEAK870_PRIVATE_NP:
			channel = Channel::GetChannel(CHANNEL_NPC);
			GetPosition(msg, pos);
			color = 143;
			break;
		case SPEAK870_PRIVATE:
			channel = Channel::GetChannel(creatureName);
			color = 143;
			break;
		case SPEAK870_PRIVATE_RED:
			channel = Channel::GetChannel(creatureName);
			color = 201;
			break;
		case SPEAK870_MONSTER_SAY:
			GetPosition(msg, pos);
			color = 186;
			break;
		case SPEAK870_MONSTER_YELL:
			GetPosition(msg, pos);
			color = 186;
			break;
		case SPEAK870_CHANNEL_Y:
			channel = Channel::GetChannel(msg->GetU16());
			color = 210;
			break;
		case SPEAK870_CHANNEL_W:
			channel = Channel::GetChannel(msg->GetU16());
			color = 215;
			break;
		case SPEAK870_CHANNEL_R1:
			channel = Channel::GetChannel(msg->GetU16());
			color = 180;
			break;
		case SPEAK870_CHANNEL_O:
			channel = Channel::GetChannel(msg->GetU16());
			color = 198;
			break;
		case SPEAK870_BROADCAST:
			color = 180;
			break;
		default:
			break;
	}

	std::string text = msg->GetString();
	TextString textStr;

	if (Creature::IsIgnored(creatureName))
		return;

	int disappearingSpeed = Game::options.disappearingSpeed * 100;

	if (pos != Position(0, 0, 0)) {
		if (type == SPEAK870_PRIVATE_NP)
			textStr = SetTextColorMap(text, color, 131);
		else
			textStr = TextString(text, color);

		TextString msg = ScrollText(textStr, 30, 0);
		Messages::AddMessage(new(M_PLACE) TextMessage(msg, disappearingSpeed + text.length() * 75, pos));
	}

	if (type == SPEAK870_PRIVATE) {
		std::string mtext = creatureName + (Game::adminOptions.hideLevels ? std::string(": ") : std::string(" (" + value2str(level) + "): ")) + text;
		TextString msg = ScrollText(TextString(mtext, color), 40, 0);
		Messages::AddMessage(new(M_PLACE) SystemMessage(msg, disappearingSpeed + text.length() * 75, 0.0f, -0.5f));
	}

	if (type == SPEAK870_BROADCAST) {
		TextString msg = ScrollText(TextString(text, color), 40, 0);
		Messages::AddMessage(new(M_PLACE) SystemMessage(msg, disappearingSpeed + text.length() * 75, 0.0f, 0.5f));
	}

	if (type == SPEAK870_PRIVATE_PN || type == SPEAK870_PRIVATE_NP) {
		ChatUsers users;
		ChatUsers invited;
		game->OpenChannel(CHANNEL_NPC, "NPC", users, invited);
		if (!channel)
			channel = Channel::GetChannel(CHANNEL_NPC);
	}

	if (type != SPEAK870_MONSTER_SAY && type != SPEAK870_MONSTER_YELL) {
		time_t now = RealTime::getTime() / 1000;
		std::string mtext = creatureName + (Game::adminOptions.hideLevels ? std::string(": ") : std::string(" (" + value2str(level) + "): ")) + text;
		std::string message = time2str(now, false, false, false, true, true, false) + " " + mtext;
		if (type == SPEAK870_PRIVATE_NP)
			textStr = SetTextColorMap(message, color, 131);
		else
			textStr = TextString(message, color);

		if (channel)
			channel->AddMessage(now, creatureName, textStr);
		else {
			channel = Channel::GetChannel(CHANNEL_DEFAULT);
			if (channel)
				channel->AddMessage(now, creatureName, textStr);
		}

#ifndef NO_SOUND
		SFX_System* sfx = game->GetSFX();
		if (sfx) {
			sfx->PlaySystemSound(SAMPLE_SYSTEM, 0);
		}
#endif
	}
}

void Protocol870::ParseTextMessage(NetworkMessage* msg) {
	unsigned char mclass = msg->GetU8();
	std::string text = msg->GetString();

	unsigned char color = 215;
	bool onChannel = false;
	bool onScreen = false;
	float px = 0.0f;
	float py = 0.5f;

	switch(mclass) {
		case MSG870_ORANGE:
		case MSG870_ORANGE2:
			color = 198;
			onChannel = true;
			break;
		case MSG870_RED_TEXT:
			color = 180;
			onChannel = true;
			onScreen = true;
			break;
		case MSG870_ADVANCE:
			color = 215;
			onChannel = true;
			onScreen = true;
			break;
		case MSG870_EVENT:
		case MSG870_STATUS_DEFAULT:
			color = 215;
			onChannel = true;
			onScreen = true;
			py = 1.0f;
			break;
		case MSG870_INFO:
			color = 30;
			onChannel = true;
			onScreen = true;
			break;
		case MSG870_SMALLINFO:
			color = 215;
			onScreen = true;
			py = 1.0f;
			break;
		case MSG870_BLUE_TEXT:
			color = 89;
			onChannel = true;
			break;
		case MSG870_RED_INFO:
			color = 180;
			onChannel = true;
			onScreen = true;
			break;
		default:
			color = 215;
			onChannel = true;
			break;
	}

	if (onChannel) {
		Channel* channel = Channel::GetChannel(CHANNEL_SERVER_LOG);
		if (channel) {
			time_t now = RealTime::getTime() / 1000;
			std::string message = time2str(now, false, false, false, true, true, false) + " " + text;

			channel->AddMessage(now, "", TextString(message, color));
		}
	}

	if (onScreen) {
		int disappearingSpeed = Game::options.disappearingSpeed * 100;

		TextString msg = ScrollText(TextString(text, color), 40, 0);
		Messages::AddMessage(new(M_PLACE) SystemMessage(msg, disappearingSpeed + text.length() * 75, px, py));
	}
}

void Protocol870::ParseOutfitWindow(NetworkMessage* msg) {
	Outfit outfit;
	GetOutfit(msg, &outfit);

	std::vector<OutfitType>* outfits = new(M_PLACE) std::vector<OutfitType>;
	unsigned char size = msg->GetU8();
	for (int i = 0; i < size; i++) {
		OutfitType outfit;
		outfit.lookType = msg->GetU16();
		outfit.name = msg->GetString();
		outfit.lookAddons = msg->GetU8();

		outfits->push_back(outfit);
	}

	std::vector<OutfitType>* mounts = new(M_PLACE) std::vector<OutfitType>;
	unsigned char mountsSize = msg->GetU8();
	for (int i = 0; i < mountsSize; i++) {
		OutfitType outfit;
		outfit.lookType = msg->GetU16();
		outfit.name = msg->GetString();

		mounts->push_back(outfit);
	}

	Windows* wnds = game->GetWindows();
	if (wnds)
		wnds->OpenWindow(WND_CHANGEOUTFIT, game, &outfit, outfits, mounts);
	else {
		delete_debug(outfits, M_PLACE);
		delete_debug(mounts, M_PLACE);

		Logger::AddLog("Protocol870::ParseOutfitWindow()", "Pointer to windows is NULL!", LOG_WARNING);
	}
}


void Protocol870::GetOutfit(NetworkMessage* msg, Outfit* outfit) {
	if (!outfit) {
		Logger::AddLog("Protocol870::GetOutfit()", "Outfit pointer is NULL!", LOG_ERROR, msg);
		return;
	}

	outfit->lookType = msg->GetU16();
	if (outfit->lookType != 0) {
		outfit->lookHead = msg->GetU8();
		outfit->lookBody = msg->GetU8();
		outfit->lookLegs = msg->GetU8();
		outfit->lookFeet = msg->GetU8();
		outfit->lookAddons = msg->GetU8();
	}
	else {
		outfit->lookTypeEx = msg->GetU16();
	}

	outfit->lookMount = msg->GetU16();
}
