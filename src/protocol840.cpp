/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#include "protocol840.h"

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

Protocol840::Protocol840(Game* game) : Protocol822(game) { }

Protocol840::~Protocol840() {
	if (client.Connected())
		client.doClose();

	if (receiveLoop.joinable())
        receiveLoop.join();
}


unsigned short Protocol840::GetVersion() {
	return 840;
}


bool Protocol840::LoginServer() {
	LOCKCLASS lockClass(lockProtocol);

	Host host = game->GetHost();

	CloseConnection();

	client.Connect(host.host.c_str(), host.port.c_str());
	if (!client.Connected()) {
		Windows* wnds = game->GetWindows();
		wnds->OpenWindow(WND_MESSAGE, Text::GetText("ERROR_MESSAGE_2", Game::options.language).c_str());
		return false;
	}

	NetworkMessage msg(GetVersion());
	msg.SetRSA(&rsa);

	msg.AddU8(0x01);			//Login to server info
	msg.AddU16(APP_OS);			//Client OS
	msg.AddU16(GetVersion());	//Version

	msg.AddU32(0);
	msg.AddU32(0);
	msg.AddU32(0);

	RandomizeXTEA();
	msg.AddU32(XTEAkey[0]);
	msg.AddU32(XTEAkey[1]);
	msg.AddU32(XTEAkey[2]);
	msg.AddU32(XTEAkey[3]);

	msg.AddString(host.account);
	msg.AddString(host.password);

	msg.RSA_encrypt(17);

	msg.SendMessage(&client, true);

	return true;
}

bool Protocol840::LoginGame() {
	LOCKCLASS lockClass(lockProtocol);

	Host host = game->GetHost();
	Character character = game->GetCharacter();

	CloseConnection();

	std::string shost = ConvertIP(character.servIP);
	std::string sport = value2str(character.servPort);

	client.Connect(shost.c_str(), sport.c_str());
	if (!client.Connected()) {
		Windows* wnds = game->GetWindows();
		wnds->CloseWindows(WND_MESSAGE, true);
		wnds->OpenWindow(WND_MESSAGE, Text::GetText("ERROR_MESSAGE_3", Game::options.language).c_str());
		return false;
	}

	NetworkMessage msg(GetVersion());
	msg.SetRSA(&rsa);

	msg.AddU8(0x0A);			//Login to server info
	msg.AddU16(APP_OS);			//Client OS
	msg.AddU16(GetVersion());	//Version

	RandomizeXTEA();
	msg.AddU32(XTEAkey[0]);
	msg.AddU32(XTEAkey[1]);
	msg.AddU32(XTEAkey[2]);
	msg.AddU32(XTEAkey[3]);

	msg.AddU8(0x00);

	msg.AddString(host.account);
	msg.AddString(character.name);
	msg.AddString(host.password);

	msg.RSA_encrypt(5);

	msg.SendMessage(&client, true);

	return true;
}


void Protocol840::SendSay(unsigned char speakClass, Channel* channel, std::string message) {
	LOCKCLASS lockClass(lockProtocol);

	if (!channel) {
		Logger::AddLog("Protocol::SendSay()", "Pointer to channel is NULL!", LOG_WARNING);
		return;
	}

	NetworkMessage msg(GetVersion(), XTEAkey);

	msg.AddU8(0x96);
	msg.AddU8(speakClass);
	switch(speakClass) {
		case SPEAK840_PRIVATE:
		case SPEAK840_PRIVATE_RED:
			msg.AddString(channel->GetName());
			break;
		case SPEAK840_CHANNEL_Y:
		case SPEAK840_CHANNEL_W:
		case SPEAK840_CHANNEL_O:
		case SPEAK840_CHANNEL_R1:
			msg.AddU16(channel->GetID());
			break;
		default:
			break;
	};
	msg.AddString(message);

	msg.SendMessage(&client);
}

void Protocol840::SendPurchaseShop(unsigned short itemID, unsigned char type, unsigned char count, bool ignoreCap, bool inBackpack) {
	LOCKCLASS lockClass(lockProtocol);

	NetworkMessage msg(GetVersion(), XTEAkey);

	msg.AddU8(0x7A);
	msg.AddU16(itemID);
	msg.AddU8(type);
	msg.AddU8(count);
	msg.AddU8((int)ignoreCap);
	msg.AddU8((int)inBackpack);

	msg.SendMessage(&client);
}


void Protocol840::ParseShop(NetworkMessage* msg) {
	unsigned char size = msg->GetU8();

	Shop* shop = game->GetShop();
	if (!shop) {
		Logger::AddLog("Protocol::ParseShop()", "Pointer to shop is NULL!", LOG_ERROR, msg);
		return;
	}

	shop->ClearShopDetailItems();
	shop->ClearPurchaseItems();
	shop->ClearSaleItems();
	for (int n = 0; n < size; n++) {
		ShopDetailItem item;
		item.itemID = msg->GetU16();
		item.count = msg->GetU8();
		item.name = msg->GetString();
		item.weight = msg->GetU32();
		item.buyPrice = (int)msg->GetU32();
		item.sellPrice = (int)msg->GetU32();

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
		Logger::AddLog("Protocol::ParseShop()", "Pointer to windows is NULL!", LOG_WARNING);
		return;
	}

	wnds->CloseWindows(WND_SHOP);
	wnds->OpenWindow(WND_SHOP, game);
}

void Protocol840::ParseShopSaleItemList(NetworkMessage* msg) {
	unsigned int money = msg->GetU32();
	unsigned char size = msg->GetU8();

	Shop* shop = game->GetShop();
	if (!shop) {
		Logger::AddLog("Protocol::ParseShopSaleItemList()", "Pointer to shop is NULL!", LOG_ERROR, msg);
		return;
	}

	shop->SetMoney(money);

	shop->ClearSaleItemsAmount();
	for (int n = 0; n < size; n++) {
		ShopItem item;
		item.itemID = msg->GetU16();

		ItemType* iType = Item::GetItemType(item.itemID);
		if (!iType)
			Logger::AddLog("Protocol::ParseShopSellItemList()", "Unknown item id!", LOG_WARNING);

		item.amount = msg->GetU8();

		shop->UpdateSaleItem(item);
	}

	Windows* wnds = game->GetWindows();
	if (!wnds) {
		Logger::AddLog("Protocol840::ParseShop()", "Pointer to windows is NULL!", LOG_WARNING);
		return;
	}

	Window* wnd = wnds->FindWindow(WND_SHOP);
	if (wnd)
		shop->UpdateMemo();
}

void Protocol840::ParsePlayerStats(NetworkMessage* msg) {
	Player* player = game->GetPlayer();
	Statistics* statistics = player->GetStatistics();

	statistics->health = msg->GetU16();
	statistics->maxHealth = msg->GetU16();

	statistics->capacity = (double)(msg->GetU32()) / 100;
	statistics->experience = msg->GetU32();

	statistics->level = msg->GetU16();
	statistics->level_p = msg->GetU8();

	statistics->mana = msg->GetU16();
	statistics->maxMana = msg->GetU16();

	statistics->magicLevel = msg->GetU8();
	statistics->magicLevel_p = msg->GetU8();

	statistics->soul = msg->GetU8();

	statistics->stamina = msg->GetU16();
}

void Protocol840::ParseCreatureSpeak(NetworkMessage* msg) {
	unsigned int creatureID = msg->GetU32();
	std::string creatureName = msg->GetString();
	unsigned short level = msg->GetU16();

	unsigned char type = msg->GetU8();

	Position pos(0, 0, 0);
	Channel* channel = NULL;
	unsigned char color = 215;
	switch(type) {
		case SPEAK840_SAY:
			GetPosition(msg, pos);
			color = 210;
			break;
		case SPEAK840_WHISPER:
			GetPosition(msg, pos);
			color = 200;
			break;
		case SPEAK840_YELL:
			GetPosition(msg, pos);
			color = 210;
			break;
		case SPEAK840_PRIVATE_PN:
			channel = Channel::GetChannel(CHANNEL_NPC);
			break;
		case SPEAK840_PRIVATE_NP:
			channel = Channel::GetChannel(CHANNEL_NPC);
			GetPosition(msg, pos);
			color = 143;
			break;
		case SPEAK840_PRIVATE:
			channel = Channel::GetChannel(creatureName);
			color = 143;
			break;
		case SPEAK840_PRIVATE_RED:
			channel = Channel::GetChannel(creatureName);
			color = 201;
			break;
		case SPEAK840_MONSTER_SAY:
			GetPosition(msg, pos);
			color = 186;
			break;
		case SPEAK840_MONSTER_YELL:
			GetPosition(msg, pos);
			color = 186;
			break;
		case SPEAK840_CHANNEL_Y:
			channel = Channel::GetChannel(msg->GetU16());
			color = 210;
			break;
		case SPEAK840_CHANNEL_W:
			channel = Channel::GetChannel(msg->GetU16());
			color = 215;
			break;
		case SPEAK840_CHANNEL_R1:
		case SPEAK840_CHANNEL_R2:
			channel = Channel::GetChannel(msg->GetU16());
			color = 180;
			break;
		case SPEAK840_CHANNEL_O:
			channel = Channel::GetChannel(msg->GetU16());
			color = 198;
			break;
		case SPEAK840_BROADCAST:
			color = 180;
			break;
		case SPEAK840_RVR_CHANNEL:
			msg->GetU32();
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
		if (type == SPEAK840_PRIVATE_NP)
			textStr = SetTextColorMap(text, color, 131);
		else
			textStr = TextString(text, color);

		TextString msg = ScrollText(textStr, 30, 0);
		Messages::AddMessage(new(M_PLACE) TextMessage(msg, disappearingSpeed + text.length() * 75, pos));
	}

	if (type == SPEAK840_PRIVATE) {
		std::string mtext = creatureName + (Game::adminOptions.hideLevels ? std::string(": ") : std::string(" (" + value2str(level) + "): ")) + text;
		TextString msg = ScrollText(TextString(mtext, color), 40, 0);
		Messages::AddMessage(new(M_PLACE) SystemMessage(msg, disappearingSpeed + text.length() * 75, 0.0f, -0.5f));
	}

	if (type == SPEAK840_BROADCAST) {
		TextString msg = ScrollText(TextString(text, color), 40, 0);
		Messages::AddMessage(new(M_PLACE) SystemMessage(msg, disappearingSpeed + text.length() * 75, 0.0f, 0.5f));
	}

	if (type == SPEAK840_PRIVATE_PN || type == SPEAK840_PRIVATE_NP) {
		ChatUsers users;
		ChatUsers invited;
		game->OpenChannel(CHANNEL_NPC, "NPC", users, invited);
		if (!channel)
			channel = Channel::GetChannel(CHANNEL_NPC);
	}

	if (type != SPEAK840_MONSTER_SAY && type != SPEAK840_MONSTER_YELL) {
		time_t now = RealTime::getTime() / 1000;
		std::string mtext = creatureName + (Game::adminOptions.hideLevels ? std::string(": ") : std::string(" (" + value2str(level) + "): ")) + text;
		std::string message = time2str(now, false, false, false, true, true, false) + " " + mtext;
		if (type == SPEAK840_PRIVATE_NP)
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

void Protocol840::ParseTextMessage(NetworkMessage* msg) {
	unsigned char mclass = msg->GetU8();
	std::string text = msg->GetString();

	unsigned char color = 215;
	bool onChannel = false;
	bool onScreen = false;
	float px = 0.0f;
	float py = 0.5f;

	switch(mclass) {
		case MSG840_ORANGE:
		case MSG840_ORANGE2:
			color = 198;
			onChannel = true;
			break;
		case MSG840_RED_TEXT:
			color = 180;
			onChannel = true;
			onScreen = true;
			break;
		case MSG840_ADVANCE:
			color = 215;
			onChannel = true;
			onScreen = true;
			break;
		case MSG840_EVENT:
		case MSG840_STATUS_DEFAULT:
			color = 215;
			onChannel = true;
			onScreen = true;
			py = 1.0f;
			break;
		case MSG840_INFO:
			color = 30;
			onChannel = true;
			onScreen = true;
			break;
		case MSG840_SMALLINFO:
			color = 215;
			onScreen = true;
			py = 1.0f;
			break;
		case MSG840_BLUE_TEXT:
			color = 89;
			onChannel = true;
			break;
		case MSG840_RED_INFO:
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

