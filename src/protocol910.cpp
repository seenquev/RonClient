/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#include "protocol910.h"

#include "allocator.h"


// ---- Protocol ---- //

Protocol910::Protocol910(Game* game) : Protocol870(game) { }

Protocol910::~Protocol910() {
	if (client.Connected())
		client.doClose();

	if (receiveLoop.joinable())
        receiveLoop.join();
}


unsigned short Protocol910::GetVersion() {
	return 910;
}


void Protocol910::SendSay(unsigned char speakClass, Channel* channel, std::string message) {
	LOCKCLASS lockClass(lockProtocol);

	if (!channel) {
		Logger::AddLog("Protocol910::::SendSay()", "Pointer to channel is NULL!", LOG_WARNING);
		return;
	}

	NetworkMessage msg(GetVersion(), XTEAkey);

	msg.AddU8(0x96);
	msg.AddU8(speakClass);
	switch(speakClass) {
		case SPEAK910_PRIVATE_FROM:
		case SPEAK910_PRIVATE_TO:
		case SPEAK910_PRIVATE_RED_FROM:
		case SPEAK910_PRIVATE_RED_TO:
			msg.AddString(channel->GetName());
			break;
		case SPEAK910_CHANNEL_Y:
		case SPEAK910_CHANNEL_W:
		case SPEAK910_CHANNEL_O:
		case SPEAK910_CHANNEL_R1:
			msg.AddU16(channel->GetID());
			break;
		default:
			break;
	};
	msg.AddString(message);

	msg.SendMessage(&client);
}


void Protocol910::ParseShop(NetworkMessage* msg) {
	std::string name = msg->GetString();
	unsigned char size = msg->GetU8();

	Shop* shop = game->GetShop();
	if (!shop) {
		Logger::AddLog("Protocol910::ParseShop()", "Pointer to shop is NULL!", LOG_ERROR, msg);
		return;
	}

	shop->SetName(name);

	shop->ClearShopDetailItems();
	shop->ClearPurchaseItems();
	shop->ClearSaleItems();
	for (int n = 0; n < size; n++) {
		ShopDetailItem item;
		item.itemID = msg->GetU16();
		item.count = msg->GetU8();
		item.name = msg->GetString();
		item.weight = msg->GetU32();
		item.buyPrice = msg->GetU32();
		item.sellPrice = msg->GetU32();

		shop->AddShopDetailItem(item);
		if (item.buyPrice > 0) {
			ShopItem pitem;
			pitem.itemID = item.itemID;
			pitem.count = item.count;
			pitem.amount = 100;

			shop->AddPurchaseItem(pitem);
		}
		if (item.sellPrice > 0) {
			ShopItem sitem;
			sitem.itemID = item.itemID;
			sitem.count = item.count;
			sitem.amount = 100;

			shop->AddSaleItem(sitem);
		}
	}

	Windows* wnds = game->GetWindows();
	if (!wnds) {
		Logger::AddLog("Protocol910::ParseShop()", "Pointer to windows is NULL!", LOG_WARNING);
		return;
	}

	wnds->CloseWindows(WND_SHOP);
	wnds->OpenWindow(WND_SHOP, game);
}

void Protocol910::ParsePlayerStats(NetworkMessage* msg) {
	Player* player = game->GetPlayer();
	Statistics* statistics = player->GetStatistics();

	statistics->health = msg->GetU16();
	statistics->maxHealth = msg->GetU16();

	statistics->capacity = (double)(msg->GetU32()) / 100;
	msg->GetU32(); //msx capacity

	statistics->experience = msg->GetU64();

	statistics->level = msg->GetU16();
	statistics->level_p = msg->GetU8();

	statistics->mana = msg->GetU16();
	statistics->maxMana = msg->GetU16();

	statistics->magicLevel = msg->GetU8();
	msg->GetU8(); //base magic level
	statistics->magicLevel_p = msg->GetU8();

	statistics->soul = msg->GetU8();

	statistics->stamina = msg->GetU16();

	msg->GetU16(); //base speed
	msg->GetU16(); //regeneration
}

void Protocol910::ParsePlayerSkills(NetworkMessage* msg) {
	Player* player = game->GetPlayer();
	Statistics* statistics = player->GetStatistics();

	for (int i = 0; i < 7; i++) {
		statistics->skill[i] = msg->GetU8();
		msg->GetU8(); //base skill
		statistics->skill_p[i] = msg->GetU8();
	}
}

void Protocol910::ParseOpenChannel(NetworkMessage* msg) {
	unsigned short channelID = msg->GetU16();
	std::string channelName = msg->GetString();

	ChatUsers users;
	ChatUsers invited;

	unsigned short usersSize = msg->GetU16();
	for (int i = 0; i < usersSize; i++)
		users.push_back(msg->GetString());

	unsigned short invitedSize = msg->GetU16();
	for (int i = 0; i < invitedSize; i++)
		invited.push_back(msg->GetString());

	game->OpenChannel(channelID, channelName, users, invited);
}

void Protocol910::ParseCreatePrivateChannel(NetworkMessage* msg) {
	unsigned short channelID = msg->GetU16();
	std::string channelName = msg->GetString();

	ChatUsers users;
	ChatUsers invited;

	unsigned short usersSize = msg->GetU16();
	for (int i = 0; i < usersSize; i++)
		users.push_back(msg->GetString());

	unsigned short invitedSize = msg->GetU16();
	for (int i = 0; i < invitedSize; i++)
		invited.push_back(msg->GetString());

	game->OpenChannel(channelID, channelName, users, invited);
}

void Protocol910::ParseCreatureSpeak(NetworkMessage* msg) {
	unsigned int statementID = msg->GetU32();
	std::string creatureName = msg->GetString();
	unsigned short level = msg->GetU16();

	unsigned char type = msg->GetU8();

	Position pos(0, 0, 0);
	Channel* channel = NULL;
	unsigned char color = 215;
	switch(type) {
		case SPEAK910_SAY:
			GetPosition(msg, pos);
			color = 210;
			break;
		case SPEAK910_WHISPER:
			GetPosition(msg, pos);
			color = 200;
			break;
		case SPEAK910_YELL:
			GetPosition(msg, pos);
			color = 210;
			break;
		case SPEAK910_PRIVATE_PN:
			channel = Channel::GetChannel(CHANNEL_NPC);
			break;
		case SPEAK910_PRIVATE_NP:
			channel = Channel::GetChannel(CHANNEL_NPC);
			GetPosition(msg, pos);
			color = 143;
			break;
		case SPEAK910_PRIVATE_FROM:
			channel = Channel::GetChannel(creatureName);
			color = 143;
			break;
		case SPEAK910_PRIVATE_TO:
			channel = Channel::GetChannel(creatureName);
			color = 143;
			break;
		case SPEAK910_PRIVATE_RED_FROM:
			channel = Channel::GetChannel(creatureName);
			color = 201;
			break;
		case SPEAK910_PRIVATE_RED_TO:
			channel = Channel::GetChannel(creatureName);
			color = 201;
			break;
		case SPEAK910_MONSTER_SAY:
			GetPosition(msg, pos);
			color = 186;
			break;
		case SPEAK910_MONSTER_YELL:
			GetPosition(msg, pos);
			color = 186;
			break;
		case SPEAK910_CHANNEL_Y:
			channel = Channel::GetChannel(msg->GetU16());
			color = 210;
			break;
		case SPEAK910_CHANNEL_W:
			channel = Channel::GetChannel(msg->GetU16());
			color = 215;
			break;
		case SPEAK910_CHANNEL_R1:
			channel = Channel::GetChannel(msg->GetU16());
			color = 180;
			break;
		case SPEAK910_CHANNEL_O:
			channel = Channel::GetChannel(msg->GetU16());
			color = 198;
			break;
		case SPEAK910_BROADCAST:
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
		if (type == SPEAK910_PRIVATE_NP)
			textStr = SetTextColorMap(text, color, 131);
		else
			textStr = TextString(text, color);

		TextString msg = ScrollText(textStr, 30, 0);
		Messages::AddMessage(new(M_PLACE) TextMessage(msg, disappearingSpeed + text.length() * 75, pos));
	}

	if (type == SPEAK910_PRIVATE_FROM || type == SPEAK910_PRIVATE_TO) {
		std::string mtext = creatureName + (Game::adminOptions.hideLevels ? std::string(": ") : std::string(" (" + value2str(level) + "): ")) + text;
		TextString msg = ScrollText(TextString(mtext, color), 40, 0);
		Messages::AddMessage(new(M_PLACE) SystemMessage(msg, disappearingSpeed + text.length() * 75, 0.0f, -0.5f));
	}

	if (type == SPEAK910_PRIVATE_RED_FROM || type == SPEAK910_PRIVATE_RED_TO) {
		std::string mtext = creatureName + (Game::adminOptions.hideLevels ? std::string(": ") : std::string(" (" + value2str(level) + "): ")) + text;
		TextString msg = ScrollText(TextString(mtext, color), 40, 0);
		Messages::AddMessage(new(M_PLACE) SystemMessage(msg, disappearingSpeed + text.length() * 75, 0.0f, -0.5f));
	}

	if (type == SPEAK910_BROADCAST) {
		TextString msg = ScrollText(TextString(text, color), 40, 0);
		Messages::AddMessage(new(M_PLACE) SystemMessage(msg, disappearingSpeed + text.length() * 75, 0.0f, 0.5f));
	}

	if (type == SPEAK910_PRIVATE_PN || type == SPEAK910_PRIVATE_NP) {
		ChatUsers users;
		ChatUsers invited;
		game->OpenChannel(CHANNEL_NPC, "NPC", users, invited);
		if (!channel)
			channel = Channel::GetChannel(CHANNEL_NPC);
	}

	if (type != SPEAK910_MONSTER_SAY && type != SPEAK910_MONSTER_YELL) {
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

void Protocol910::ParseTextMessage(NetworkMessage* msg) {
	unsigned char mclass = msg->GetU8();

	unsigned char color = 215;
	bool onChannel = false;
	bool onScreen = false;
	float px = 0.0f;
	float py = 0.5f;

	switch(mclass) {
		case MSG910_DAMAGE_DEALT:
		case MSG910_DAMAGE_RECEIVED:
		case MSG910_DAMAGE_OTHERS: {
			Position pos;
			GetPosition(msg, pos);
			unsigned int firstDmg = msg->GetU32();
			std::string first = value2str(firstDmg);
			unsigned char fcolor = msg->GetU8();
			unsigned int secondDmg = msg->GetU32();
			std::string second = value2str(secondDmg);
			unsigned char scolor = msg->GetU8();

			Messages::AddMessage(new(M_PLACE) AnimatedMessage(TextString(first, fcolor), 2000, pos, 0.0f, 0.0f));
			if (secondDmg)
				Messages::AddMessage(new(M_PLACE) AnimatedMessage(TextString(second, scolor), 2000, pos, 0.0f, 1.0f));
			break;
		}
		case MSG910_EXPERIENCE:
		case MSG910_EXPERIENCE_OTHERS:
		case MSG910_HEALED:
		case MSG910_HEALED_OTHERS: {
			Position pos;
			GetPosition(msg, pos);
			std::string text = value2str(msg->GetU32());
			unsigned char color = msg->GetU8();
			Messages::AddMessage(new(M_PLACE) AnimatedMessage(TextString(text, color), 2000, pos));
			break;
		}
		case MSG910_STATUS_CONSOLE_BLUE:
			color = 89;
			onChannel = true;
			break;
		case MSG910_STATUS_CONSOLE_RED:
			color = 180;
			onChannel = true;
			break;
		case MSG910_STATUS_DEFAULT:
			color = 215;
			onChannel = true;
			onScreen = true;
			py = 1.0f;
			break;
		case MSG910_STATUS_WARNING:
			color = 180;
			onChannel = true;
			onScreen = true;
			break;
		case MSG910_EVENT_ADVANCE:
			color = 215;
			onChannel = true;
			onScreen = true;
			break;
		case MSG910_STATUS_SMALL:
			color = 215;
			onChannel = true;
			py = 1.0f;
			break;
		case MSG910_INFO_DESCR:
			color = 30;
			onChannel = true;
			onScreen = true;
			break;
		case MSG910_EVENT_DEFAULT:
			color = 215;
			onChannel = true;
			onScreen = true;
			py = 1.0f;
			break;
		case MSG910_LOOT:
		case MSG910_EVENT_ORANGE:
			color = 198;
			onChannel = true;
			break;
		case MSG910_STATUS_CONSOLE_ORANGE:
			color = 215;
			onChannel = true;
			onScreen = true;
			break;
	}

	std::string text = msg->GetString();

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


void Protocol910::GetThing(NetworkMessage* msg, Thing*& thing, int& skip) {
	unsigned short id = msg->GetU16();
	unsigned int shine = 0;

	if (id == 0x0010) {
		id = msg->GetU16();
		shine = msg->GetU32();
	}

	if (id == 0x0000) { //enviromental effect
		thing = NULL;
		skip = -1;
	}
	else if (id >= 0xFF00) {
		thing = NULL;
		skip = id - 0xFF00;
	}
	else if (id == 0x61) {
		Creature* creature = new(M_PLACE) Creature;
		GetCreature(msg, creature, false);
		thing = creature;
	}
	else if (id == 0x62) {
		Creature* creature = NULL;
		GetCreature(msg, creature, true);
		thing = creature;
	}
	else if (id == 0x63) {
		unsigned int creatureID = msg->GetU32();
		Direction direction = (Direction)msg->GetU8();

		Creature* creature = Creature::GetFromKnown(creatureID);
		if (creature)
			creature->SetDirection(direction);
	}
	else if (id >= 100) {
		unsigned char count = 0;

		ItemType* iType = Item::GetItemType(id);
		if (!iType) {
			std::string error = "Unknown item id [" + value2str(id) + "]!";
			Logger::AddLog("Protocol::GetThing()", error.c_str(), LOG_ERROR, msg);
			return;
		}

		if (iType->stackable || iType->fluid || iType->multiType)
			count = msg->GetU8();
		if (iType->animated)
			msg->GetU8();

		Item* item = new(M_PLACE) Item;
		item->SetID(id);
		item->SetCount(count);
		item->SetShine(shine);
		thing = item;
	}
}

void Protocol910::GetTileDescription(NetworkMessage* msg, Position pos, Tile*& tile, int& skip) {
	unsigned char color = 0;

	bool first = true;
	Thing* thing = NULL;
	do {
		thing = NULL;
		skip = 0;
		GetThing(msg, thing, skip);

		if (!thing && skip == -1) {		//enviromantal effect
			skip = 0;
		}
		else if (thing) {
			if (!tile) {
				tile = new(M_PLACE) Tile;
				tile->pos = pos;
			}

			thing->pos = pos;
			tile->InsertThing(thing);

			Item* item = dynamic_cast<Item*>(thing);
			if (item)
				item->SetLight();
		}
		else
			break;
	} while(true);

	if (tile)
		tile->SortThings();
}

void Protocol910::GetCreature(NetworkMessage* msg, Creature*& creature, bool known) {
	if (known) {
		unsigned int creatureID = msg->GetU32();

		creature = Creature::GetFromKnown(creatureID);
		if (!creature) {
			Logger::AddLog("Protocol910::GetCreature()", "Known creature pointer is NULL!", LOG_ERROR, msg);
			return;
		}
	}
	else {
		if (!creature) {
			Logger::AddLog("Protocol910::GetCreature()", "Unknown creature pointer is NULL!", LOG_ERROR, msg);
			return;
		}

		unsigned int removeID = msg->GetU32();
		unsigned int creatureID = msg->GetU32();
		unsigned char type = msg->GetU8();
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
