/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#include "protocol.h"

#include "allocator.h"
#include "iniloader.h"
#include "luascript.h"
#include "text.h"
#include "tools.h"
#include "window.h"

#ifndef NO_SOUND
	#include "adal.h"
	#include "sound.h"
#endif


// ---- Protocol ---- //

Protocol::Protocol(Game* game) {
	XTEAkey[0] = 0;
	XTEAkey[1] = 0;
	XTEAkey[2] = 0;
	XTEAkey[3] = 0;

	this->game = game;

	std::string p = "14299623962416399520070177382898895550795403345466153217470516082934737582776038882967213386204600674145392845853859217990626450972452084065728686565928113";
	std::string q = "7630979195970404721891201847792002125535401292779123937207447574596692788513647179235335529307251350570728407373705564708871762033017096809910315212884101";
	std::string d = "46730330223584118622160180015036832148732986808519344675210555262940258739805766860224610646919605860206328024326703361630109888417839241959507572247284807035235569619173792292786907845791904955103601652822519121908367187885509270025388641700821735345222087940578381210879116823013776808975766851829020659073";

	if (Game::adminOptions.rsa_p != "") p = Game::adminOptions.rsa_p;
	if (Game::adminOptions.rsa_q != "") q = Game::adminOptions.rsa_q;
	if (Game::adminOptions.rsa_d != "") d = Game::adminOptions.rsa_d;

	rsa.setKey(p.c_str(), q.c_str(), d.c_str());
}

Protocol::~Protocol() {
	if (client.Connected())
		client.doClose();

    if (receiveLoop.joinable())
        receiveLoop.join();
}

void Protocol::RandomizeXTEA() {
	XTEAkey[0] = rand() % 0xFFFFFFFF;
	XTEAkey[1] = rand() % 0xFFFFFFFF;
	XTEAkey[2] = rand() % 0xFFFFFFFF;
	XTEAkey[3] = rand() % 0xFFFFFFFF;
}


void Protocol::CloseConnection() {
	if (client.Connected())
		client.doClose();
}


bool Protocol::LoginServer() {
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

	msg.AddU32(atol(host.account.c_str()));
	msg.AddString(host.password);

	msg.RSA_encrypt(17);

	msg.SendMessage(&client, true);

	return true;
}

bool Protocol::LoginGame() {
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

	msg.AddU8(0x0A);		//Login to server info
	msg.AddU16(APP_OS);		//Client OS
	msg.AddU16(GetVersion());		//Version

	RandomizeXTEA();
	msg.AddU32(XTEAkey[0]);
	msg.AddU32(XTEAkey[1]);
	msg.AddU32(XTEAkey[2]);
	msg.AddU32(XTEAkey[3]);

	msg.AddU8(0x00);

	msg.AddU32(atol(host.account.c_str()));
	msg.AddString(character.name);
	msg.AddString(host.password);

	msg.RSA_encrypt(5);

	msg.SendMessage(&client, true);

	return true;
}


void Protocol::SendLogout() {
	LOCKCLASS lockClass(lockProtocol);

	NetworkMessage msg(GetVersion(), XTEAkey);
	msg.AddU8(0x14);

	msg.SendMessage(&client);
}

void Protocol::SendAutoWalk(std::list<Direction>& list) {
	LOCKCLASS lockClass(lockProtocol);

	NetworkMessage msg(GetVersion(), XTEAkey);

	msg.AddU8(0x64);
	msg.AddU8(list.size());

	std::list<Direction>::iterator it = list.begin();
	for (it; it != list.end(); it++) {
		Direction dir = *it;
		if (dir == EAST) msg.AddU8(1);
		else if (dir == NORTHEAST) msg.AddU8(2);
		else if (dir == NORTH) msg.AddU8(3);
		else if (dir == NORTHWEST) msg.AddU8(4);
		else if (dir == WEST) msg.AddU8(5);
		else if (dir == SOUTHWEST) msg.AddU8(6);
		else if (dir == SOUTH) msg.AddU8(7);
		else if (dir == SOUTHEAST) msg.AddU8(8);
	}

	msg.SendMessage(&client);
}

void Protocol::SendWalk(Direction direction) {
	LOCKCLASS lockClass(lockProtocol);

	NetworkMessage msg(GetVersion(), XTEAkey);

	if (direction == NORTH)
		msg.AddU8(0x65);
	else if (direction == SOUTH)
		msg.AddU8(0x67);
	else if (direction == WEST)
		msg.AddU8(0x68);
	else if (direction == EAST)
		msg.AddU8(0x66);
	else if (direction == NORTHWEST)
		msg.AddU8(0x6D);
	else if (direction == NORTHEAST)
		msg.AddU8(0x6A);
	else if (direction == SOUTHWEST)
		msg.AddU8(0x6C);
	else if (direction == SOUTHEAST)
		msg.AddU8(0x6B);
	else
		msg.AddU8(0x69);

	msg.SendMessage(&client);
}

void Protocol::SendTurn(Direction direction) {
	LOCKCLASS lockClass(lockProtocol);

	NetworkMessage msg(GetVersion(), XTEAkey);

	if (direction == NORTH)
		msg.AddU8(0x6F);
	else if (direction == SOUTH)
		msg.AddU8(0x71);
	else if (direction == WEST)
		msg.AddU8(0x72);
	else if (direction == EAST)
		msg.AddU8(0x70);

	msg.SendMessage(&client);
}

void Protocol::SendStopWalk() {
	LOCKCLASS lockClass(lockProtocol);

	NetworkMessage msg(GetVersion(), XTEAkey);

	msg.AddU8(0x69);

	msg.SendMessage(&client);
}

void Protocol::SendSay(unsigned char speakClass, Channel* channel, std::string message) {
	LOCKCLASS lockClass(lockProtocol);

	if (!channel) {
		Logger::AddLog("Protocol::SendSay()", "Pointer to channel is NULL!", LOG_WARNING);
		return;
	}

	NetworkMessage msg(GetVersion(), XTEAkey);

	msg.AddU8(0x96);
	msg.AddU8(speakClass);
	switch(speakClass) {
		case SPEAK822_PRIVATE:
		case SPEAK822_PRIVATE_RED:
			msg.AddString(channel->GetName());
			break;
		case SPEAK822_CHANNEL_Y:
		case SPEAK822_CHANNEL_W:
		case SPEAK822_CHANNEL_O:
		case SPEAK822_CHANNEL_R1:
			msg.AddU16(channel->GetID());
			break;
		default:
			break;
	};
	msg.AddString(message);

	msg.SendMessage(&client);
}

void Protocol::SendLookAt(Position pos, unsigned short itemId, unsigned char stackPos) {
	LOCKCLASS lockClass(lockProtocol);

	NetworkMessage msg(GetVersion(), XTEAkey);

	msg.AddU8(0x8C);
	msg.AddU16(pos.x);
	msg.AddU16(pos.y);
	msg.AddU8(pos.z);
	msg.AddU16(itemId);
	msg.AddU8(stackPos);

	msg.SendMessage(&client);
}

void Protocol::SendMoveThing(Position fromPos, unsigned short itemId, unsigned char stackPos, Position toPos, unsigned char count) {
	LOCKCLASS lockClass(lockProtocol);

	NetworkMessage msg(GetVersion(), XTEAkey);

	msg.AddU8(0x78);
	msg.AddU16(fromPos.x);
	msg.AddU16(fromPos.y);
	msg.AddU8(fromPos.z);
	msg.AddU16(itemId);
	msg.AddU8(stackPos);
	msg.AddU16(toPos.x);
	msg.AddU16(toPos.y);
	msg.AddU8(toPos.z);
	msg.AddU8(count);

	msg.SendMessage(&client);
}

void Protocol::SendUseThing(Position pos, unsigned short itemId, unsigned char stackPos, unsigned char index) {
	LOCKCLASS lockClass(lockProtocol);

	NetworkMessage msg(GetVersion(), XTEAkey);

	msg.AddU8(0x82);
	msg.AddU16(pos.x);
	msg.AddU16(pos.y);
	msg.AddU8(pos.z);
	msg.AddU16(itemId);
	msg.AddU8(stackPos);
	msg.AddU8(index);

	msg.SendMessage(&client);
}

void Protocol::SendUseWithThing(Position fromPos, unsigned short fromItemId, unsigned char fromStackPos, Position toPos, unsigned short toItemId, unsigned char toStackPos) {
	LOCKCLASS lockClass(lockProtocol);

	NetworkMessage msg(GetVersion(), XTEAkey);

	msg.AddU8(0x83);
	msg.AddU16(fromPos.x);
	msg.AddU16(fromPos.y);
	msg.AddU8(fromPos.z);
	msg.AddU16(fromItemId);
	msg.AddU8(fromStackPos);
	msg.AddU16(toPos.x);
	msg.AddU16(toPos.y);
	msg.AddU8(toPos.z);
	msg.AddU16(toItemId);
	msg.AddU8(toStackPos);

	msg.SendMessage(&client);
}

void Protocol::SendUseBattleThing(Position pos, unsigned short itemId, unsigned char stackPos, unsigned int creatureID) {
	LOCKCLASS lockClass(lockProtocol);

	NetworkMessage msg(GetVersion(), XTEAkey);

	msg.AddU8(0x84);
	msg.AddU16(pos.x);
	msg.AddU16(pos.y);
	msg.AddU8(pos.z);
	msg.AddU16(itemId);
	msg.AddU8(stackPos);
	msg.AddU32(creatureID);

	msg.SendMessage(&client);
}

void Protocol::SendRotateItem(Position pos, unsigned short itemId, unsigned char stackPos) {
	LOCKCLASS lockClass(lockProtocol);

	NetworkMessage msg(GetVersion(), XTEAkey);

	msg.AddU8(0x85);
	msg.AddU16(pos.x);
	msg.AddU16(pos.y);
	msg.AddU8(pos.z);
	msg.AddU16(itemId);
	msg.AddU8(stackPos);

	msg.SendMessage(&client);
}

void Protocol::SendRequestTrade(Position pos, unsigned short itemId, unsigned char stackPos, unsigned int creatureID) {
	LOCKCLASS lockClass(lockProtocol);

	NetworkMessage msg(GetVersion(), XTEAkey);

	msg.AddU8(0x7D);
	msg.AddU16(pos.x);
	msg.AddU16(pos.y);
	msg.AddU8(pos.z);
	msg.AddU16(itemId);
	msg.AddU8(stackPos);
	msg.AddU32(creatureID);

	msg.SendMessage(&client);
}

void Protocol::SendLookInTrade(unsigned char counterOffer, unsigned char index) {
	LOCKCLASS lockClass(lockProtocol);

	NetworkMessage msg(GetVersion(), XTEAkey);

	msg.AddU8(0x7E);
	msg.AddU8(counterOffer);
	msg.AddU8(index);

	msg.SendMessage(&client);
}

void Protocol::SendAcceptTrade() {
	LOCKCLASS lockClass(lockProtocol);

	NetworkMessage msg(GetVersion(), XTEAkey);

	msg.AddU8(0x7F);

	msg.SendMessage(&client);
}

void Protocol::SendCancelTrade() {
	LOCKCLASS lockClass(lockProtocol);

	NetworkMessage msg(GetVersion(), XTEAkey);

	msg.AddU8(0x80);

	msg.SendMessage(&client);
}

void Protocol::SendLookInShop(unsigned short itemID, unsigned char type) {
	LOCKCLASS lockClass(lockProtocol);

	NetworkMessage msg(GetVersion(), XTEAkey);

	msg.AddU8(0x79);
	msg.AddU16(itemID);
	msg.AddU8(type);

	msg.SendMessage(&client);
}

void Protocol::SendPurchaseShop(unsigned short itemID, unsigned char type, unsigned char count, bool ignoreCap, bool inBackpack) {
	LOCKCLASS lockClass(lockProtocol);

	NetworkMessage msg(GetVersion(), XTEAkey);

	msg.AddU8(0x7A);
	msg.AddU16(itemID);
	msg.AddU8(type);
	msg.AddU8(count);

	msg.SendMessage(&client);
}

void Protocol::SendSaleShop(unsigned short itemID, unsigned char type, unsigned char count) {
	LOCKCLASS lockClass(lockProtocol);

	NetworkMessage msg(GetVersion(), XTEAkey);

	msg.AddU8(0x7B);
	msg.AddU16(itemID);
	msg.AddU8(type);
	msg.AddU8(count);

	msg.SendMessage(&client);
}

void Protocol::SendCloseShop() {
	LOCKCLASS lockClass(lockProtocol);

	NetworkMessage msg(GetVersion(), XTEAkey);

	msg.AddU8(0x7C);

	msg.SendMessage(&client);
}

void Protocol::SendContainerClose(unsigned char index) {
	LOCKCLASS lockClass(lockProtocol);

	NetworkMessage msg(GetVersion(), XTEAkey);

	msg.AddU8(0x87);
	msg.AddU8(index);

	msg.SendMessage(&client);
}

void Protocol::SendContainerMoveUp(unsigned char index) {
	LOCKCLASS lockClass(lockProtocol);

	NetworkMessage msg(GetVersion(), XTEAkey);

	msg.AddU8(0x88);
	msg.AddU8(index);

	msg.SendMessage(&client);
}

void Protocol::SendToggleMount(bool mount) {
	LOCKCLASS lockClass(lockProtocol);

	NetworkMessage msg(GetVersion(), XTEAkey);

	msg.AddU8(0xD4);
	msg.AddU8((unsigned char)mount);

	msg.SendMessage(&client);
}

void Protocol::SendFightModes(unsigned char fight, unsigned char follow, unsigned char attack) {
	LOCKCLASS lockClass(lockProtocol);

	NetworkMessage msg(GetVersion(), XTEAkey);

	msg.AddU8(0xA0);
	msg.AddU8(fight);
	msg.AddU8(follow);
	msg.AddU8(attack);

	msg.SendMessage(&client);
}

void Protocol::SendAttack(unsigned int creatureID) {
	LOCKCLASS lockClass(lockProtocol);

	NetworkMessage msg(GetVersion(), XTEAkey);

	msg.AddU8(0xA1);
	msg.AddU32(creatureID);

	msg.SendMessage(&client);
}

void Protocol::SendFollow(unsigned int creatureID) {
	LOCKCLASS lockClass(lockProtocol);

	NetworkMessage msg(GetVersion(), XTEAkey);

	msg.AddU8(0xA2);
	msg.AddU32(creatureID);

	msg.SendMessage(&client);
}

void Protocol::SendCreatePrivateChannel() {
	LOCKCLASS lockClass(lockProtocol);

	NetworkMessage msg(GetVersion(), XTEAkey);

	msg.AddU8(0xAA);

	msg.SendMessage(&client);
}

void Protocol::SendChannelInvite(std::string creatureName) {
	LOCKCLASS lockClass(lockProtocol);

	NetworkMessage msg(GetVersion(), XTEAkey);

	msg.AddU8(0xAB);
	msg.AddString(creatureName);

	msg.SendMessage(&client);
}

void Protocol::SendChannelExclude(std::string creatureName) {
	LOCKCLASS lockClass(lockProtocol);

	NetworkMessage msg(GetVersion(), XTEAkey);

	msg.AddU8(0xAC);
	msg.AddString(creatureName);

	msg.SendMessage(&client);
}

void Protocol::SendRequestChannels() {
	LOCKCLASS lockClass(lockProtocol);

	NetworkMessage msg(GetVersion(), XTEAkey);

	msg.AddU8(0x97);

	msg.SendMessage(&client);
}

void Protocol::SendOpenChannel(unsigned short channelID) {
	LOCKCLASS lockClass(lockProtocol);

	NetworkMessage msg(GetVersion(), XTEAkey);

	msg.AddU8(0x98);
	msg.AddU16(channelID);

	msg.SendMessage(&client);
}

void Protocol::SendCloseChannel(unsigned short channelID) {
	LOCKCLASS lockClass(lockProtocol);

	NetworkMessage msg(GetVersion(), XTEAkey);

	msg.AddU8(0x99);
	msg.AddU16(channelID);

	msg.SendMessage(&client);
}

void Protocol::SendOpenPrivateChannel(std::string channelName) {
	LOCKCLASS lockClass(lockProtocol);

	NetworkMessage msg(GetVersion(), XTEAkey);

	msg.AddU8(0x9A);
	msg.AddString(channelName);

	msg.SendMessage(&client);
}

void Protocol::SendProcessRuleViolation(std::string reporter) {
	LOCKCLASS lockClass(lockProtocol);

	NetworkMessage msg(GetVersion(), XTEAkey);

	msg.AddU8(0x9B);
	msg.AddString(reporter);

	msg.SendMessage(&client);
}

void Protocol::SendCloseRuleViolation(std::string reporter) {
	LOCKCLASS lockClass(lockProtocol);

	NetworkMessage msg(GetVersion(), XTEAkey);

	msg.AddU8(0x9C);
	msg.AddString(reporter);

	msg.SendMessage(&client);
}

void Protocol::SendCancelRuleViolation() {
	LOCKCLASS lockClass(lockProtocol);

	NetworkMessage msg(GetVersion(), XTEAkey);

	msg.AddU8(0x9D);

	msg.SendMessage(&client);
}

void Protocol::SendAddVIP(std::string creatureName) {
	LOCKCLASS lockClass(lockProtocol);

	NetworkMessage msg(GetVersion(), XTEAkey);

	msg.AddU8(0xDC);
	msg.AddString(creatureName);

	msg.SendMessage(&client);
}

void Protocol::SendRemoveVIP(unsigned int creatureID) {
	LOCKCLASS lockClass(lockProtocol);

	NetworkMessage msg(GetVersion(), XTEAkey);

	msg.AddU8(0xDD);
	msg.AddU32(creatureID);

	msg.SendMessage(&client);
}

void Protocol::SendTextWindow(unsigned int textID, std::string text) {
	LOCKCLASS lockClass(lockProtocol);

	NetworkMessage msg(GetVersion(), XTEAkey);

	msg.AddU8(0x89);
	msg.AddU32(textID);
	msg.AddString(text);

	msg.SendMessage(&client);
}

void Protocol::SendHouseWindow(unsigned char listID, unsigned int textID, std::string text) {
	LOCKCLASS lockClass(lockProtocol);

	NetworkMessage msg(GetVersion(), XTEAkey);

	msg.AddU8(0x8A);
	msg.AddU8(listID);
	msg.AddU32(textID);
	msg.AddString(text);

	msg.SendMessage(&client);
}

void Protocol::SendRequestOutfit() {
	LOCKCLASS lockClass(lockProtocol);

	NetworkMessage msg(GetVersion(), XTEAkey);

	msg.AddU8(0xD2);

	msg.SendMessage(&client);
}

void Protocol::SendSetOutfit(Outfit outfit) {
	LOCKCLASS lockClass(lockProtocol);

	NetworkMessage msg(GetVersion(), XTEAkey);

	msg.AddU8(0xD3);
	msg.AddU16(outfit.lookType);
	msg.AddU8(outfit.lookHead);
	msg.AddU8(outfit.lookBody);
	msg.AddU8(outfit.lookLegs);
	msg.AddU8(outfit.lookFeet);
	msg.AddU8(outfit.lookAddons);

	msg.SendMessage(&client);
}

void Protocol::SendPartyInvite(unsigned int creatureID) {
	LOCKCLASS lockClass(lockProtocol);

	NetworkMessage msg(GetVersion(), XTEAkey);

	msg.AddU8(0xA3);
	msg.AddU32(creatureID);

	msg.SendMessage(&client);
}

void Protocol::SendPartyJoin(unsigned int creatureID) {
	LOCKCLASS lockClass(lockProtocol);

	NetworkMessage msg(GetVersion(), XTEAkey);

	msg.AddU8(0xA4);
	msg.AddU32(creatureID);

	msg.SendMessage(&client);
}

void Protocol::SendPartyRevoke(unsigned int creatureID) {
	LOCKCLASS lockClass(lockProtocol);

	NetworkMessage msg(GetVersion(), XTEAkey);

	msg.AddU8(0xA5);
	msg.AddU32(creatureID);

	msg.SendMessage(&client);
}

void Protocol::SendPartyPassLeadership(unsigned int creatureID) {
	LOCKCLASS lockClass(lockProtocol);

	NetworkMessage msg(GetVersion(), XTEAkey);

	msg.AddU8(0xA6);
	msg.AddU32(creatureID);

	msg.SendMessage(&client);
}

void Protocol::SendPartyLeave()  {
	LOCKCLASS lockClass(lockProtocol);

	NetworkMessage msg(GetVersion(), XTEAkey);

	msg.AddU8(0xA7);

	msg.SendMessage(&client);
}

void Protocol::SendPartyEnableShared(unsigned char active) {
	LOCKCLASS lockClass(lockProtocol);

	NetworkMessage msg(GetVersion(), XTEAkey);

	msg.AddU8(0xA8);
	msg.AddU8(active);
	msg.AddU8(0x00);

	msg.SendMessage(&client);
}

void Protocol::SendRequestQuestLog() {
	LOCKCLASS lockClass(lockProtocol);

	NetworkMessage msg(GetVersion(), XTEAkey);

	msg.AddU8(0xF0);

	msg.SendMessage(&client);
}

void Protocol::SendRequestQuestLine(unsigned short questID) {
	LOCKCLASS lockClass(lockProtocol);

	NetworkMessage msg(GetVersion(), XTEAkey);

	msg.AddU8(0xF1);
	msg.AddU16(questID);

	msg.SendMessage(&client);
}

void Protocol::SendCloseNPC() {
	LOCKCLASS lockClass(lockProtocol);

	NetworkMessage msg(GetVersion(), XTEAkey);

	msg.AddU8(0x9E);

	msg.SendMessage(&client);
}

void Protocol::SendError(std::string debug, std::string date, std::string description, std::string comment) {
	LOCKCLASS lockClass(lockProtocol);

	NetworkMessage msg(GetVersion(), XTEAkey);

	msg.AddU8(0xE8);
	msg.AddString(debug);
	msg.AddString(date);
	msg.AddString(description);
	msg.AddString(comment);

	msg.SendMessage(&client);
}

//Extended Protocol
void Protocol::SendRequestSpells() {
	LOCKCLASS lockClass(lockProtocol);

	NetworkMessage msg(GetVersion(), XTEAkey);

	msg.AddU8(0x00); //Use extended protocol
	msg.AddU8(0x01);

	msg.SendMessage(&client);
}


void Protocol::ParseMessageBox(NetworkMessage* msg) {
	std::string message = msg->GetString();

	Windows* wnds = game->GetWindows();
	if (!wnds) {
		Logger::AddLog("Protocol::ParseMessageBox()", "Pointer to windows is NULL!", LOG_WARNING);
		return;
	}
	wnds->CloseWindows(WND_MESSAGE);
	wnds->OpenWindow(WND_MESSAGE, message.c_str());
}

void Protocol::ParseMotd(NetworkMessage* msg) {
	std::string message = msg->GetString();
}

void Protocol::ParseFYIBox(NetworkMessage* msg) {
	std::string message = msg->GetString();
}

void Protocol::ParsePlayerAppear(NetworkMessage* msg) {
	unsigned int creatureID = msg->GetU32();
	unsigned short value = msg->GetU16();
	unsigned char canReport = msg->GetU8();

	Player::SetCreatureID(creatureID);

	Windows* wnds = game->GetWindows();
	if (!wnds) {
		Logger::AddLog("Protocol::ParsePlayerAppear()", "Pointer to windows is NULL!", LOG_WARNING);
		return;
	}

	wnds->CloseWindows(WND_ALL, true);

	VIPList* viplist = game->GetVIPList();
	if (viplist) {
		viplist->ClearVIPList();
		viplist->UpdateContainer();
	}

	Window* wnd = wnds->OpenWindow(WND_OPTIONS, game);
	wnd->SetMinimizeAbility(false);

	wnds->LoadGUIWindows();
	game->LoadChannels();

	game->SetGameState(GAME_LOGGEDTOGAME);
	THREAD threadMessages(&Messages::CheckMessages, game);
	THREAD threadEffects(&MagicEffect::CheckMagicEffects, game);
	THREAD threadParticles(&Particles::CheckParticles, game);
	THREAD threadDistances(&Distance::CheckDistances, game);
	THREAD threadCooldowns(&Cooldowns::CheckCooldowns, game);
	THREAD threadLuaScripts(&LuaScript::CheckLuaScripts, game);
#ifndef NO_SOUND
	THREAD threadSounds(&SFX_System::CheckSounds, game);
#endif
}

void Protocol::ParseViolation(NetworkMessage* msg) {
	for (int i = 0; i < 23; i++)
		msg->GetU8();
}

void Protocol::ParsePlacedInQueue(NetworkMessage* msg) {
	std::string message = msg->GetString();
	unsigned char seconds = msg->GetU8();

	game->SetGameState(GAME_INQUEUE);

	Windows* wnds = game->GetWindows();
	if (!wnds) {
		Logger::AddLog("Protocol::ParsePlacedInQueue()", "Pointer to windows is NULL!", LOG_WARNING);
		return;
	}

	wnds->CloseWindows(WND_MESSAGE);
	wnds->OpenWindow(WND_QUEUE, game, message.c_str(), seconds);
}

void Protocol::ParsePing(NetworkMessage* msg) {
	NetworkMessage nmsg(GetVersion(), XTEAkey);
	nmsg.AddU8(0x1E);
	nmsg.SendMessage(&client);
}

void Protocol::ParseReLogin(NetworkMessage* msg) {
	msg->GetU8();
	CloseConnection();
	if (game) {
		Windows* wnds = game->GetWindows();
		Game::AddTask(boost::bind(&Game::onEnterGame, game, wnds, (Window*)NULL, (unsigned short*)NULL));
	}
}

void Protocol::ParseCharactersList(NetworkMessage* msg) {
	game->ClearCharactersList();
	unsigned char characters = msg->GetU8();
	for (int n = 0; n < characters; n++) {
		Character character;
		character.name = msg->GetString();
		character.serv = msg->GetString();
		character.servIP = msg->GetU32();
		character.servPort = msg->GetU16();
		game->AddCharacter(character);
	}
	unsigned short PACC = msg->GetU16();

	Host host = game->GetHost();
	host.PACC = PACC;
	game->SetHost(host);

	game->SetGameState(GAME_LOGGEDTOSERVER);

	Windows* wnds = game->GetWindows();
	if (!wnds) {
		Logger::AddLog("Protocol::ParseCharactersList()", "Pointer to windows is NULL!", LOG_WARNING);
		return;
	}

	wnds->CloseWindows(WND_MESSAGE);
	wnds->OpenWindow(WND_CHARACTERSLIST, game);
}

void Protocol::ParseMapDescriptor(NetworkMessage* msg) {
	Position pos;
	GetPosition(msg, pos);

	Map* map = game->GetMap();
	if (!map) {
		Logger::AddLog("Protocol::ParseMapDescriptor()", "Pointer to map is NULL!", LOG_ERROR, msg);
		return;
	}

	LOCKCLASS lockClass1(Map::lockMap);
	LOCKCLASS lockClass2(Tile::lockTile);

	GetMapDescription(msg, 0, 0, 18, 14, pos, map);
	map->ReadMiniMap();
	map->CalculateZ(pos);

	Player* player = game->GetPlayer();
	Creature* creature = (player ? player->GetCreature() : NULL);

	if (player) {
		player->SetTargetPos(Position(0, 0, 0));
		player->SetFollowID(0x00);
		Player::walking = false;
		Player::requestStop = false;
		player->SetTargetPos(Position(0, 0, 0));
	}

	if (!creature)
		return;

	creature->newPos = Position(0, 0, 0);
	creature->SetStep(0.0f);

#ifndef NO_SOUND
	game->UpdateBackgroundSound(creature->pos.z, creature->oldPos == Position(0, 0, 0));
#endif

	Lights::UpdateStaticLightMap(map);

	Game::releaseSprites = true;
}

void Protocol::ParseMoveMapUp(NetworkMessage* msg) {
	Map* map = game->GetMap();
	if (!map) {
		Logger::AddLog("Protocol::ParseMapDescriptor()", "Pointer to map is NULL!", LOG_ERROR, msg);
		return;
	}

	LOCKCLASS lockClass1(Map::lockMap);
	LOCKCLASS lockClass2(Tile::lockTile);

	map->MoveMap(0, 1);

	Position pos = map->GetCorner();
	pos.x += 8;
	pos.y += 6;

	GetMapDescription(msg, 0, 0, 18, 1, pos, map);
	map->ReadMiniMap(0, 0, 256, 1);
	map->CalculateZ(pos);

	Lights::UpdateStaticLightMap(map);
}

void Protocol::ParseMoveMapRight(NetworkMessage* msg) {
	Map* map = game->GetMap();
	if (!map) {
		Logger::AddLog("Protocol::ParseMapDescriptor()", "Pointer to map is NULL!", LOG_ERROR, msg);
		return;
	}

	LOCKCLASS lockClass1(Map::lockMap);
	LOCKCLASS lockClass2(Tile::lockTile);

	map->MoveMap(-1, 0);

	Position pos = map->GetCorner();
	pos.x += 8;
	pos.y += 6;

	GetMapDescription(msg, 17, 0, 1, 14, pos, map);
	map->ReadMiniMap(255, 0, 1, 256);
	map->CalculateZ(pos);

	Lights::UpdateStaticLightMap(map);
}

void Protocol::ParseMoveMapDown(NetworkMessage* msg) {
	Map* map = game->GetMap();
	if (!map) {
		Logger::AddLog("Protocol::ParseMapDescriptor()", "Pointer to map is NULL!", LOG_ERROR, msg);
		return;
	}

	LOCKCLASS lockClass1(Map::lockMap);
	LOCKCLASS lockClass2(Tile::lockTile);

	map->MoveMap(0, -1);

	Position pos = map->GetCorner();
	pos.x += 8;
	pos.y += 6;

	GetMapDescription(msg, 0, 13, 18, 1, pos, map);
	map->ReadMiniMap(0, 255, 256, 1);
	map->CalculateZ(pos);

	Lights::UpdateStaticLightMap(map);
}

void Protocol::ParseMoveMapLeft(NetworkMessage* msg) {
	Map* map = game->GetMap();
	if (!map) {
		Logger::AddLog("Protocol::ParseMapDescriptor()", "Pointer to map is NULL!", LOG_ERROR, msg);
		return;
	}

	LOCKCLASS lockClass1(Map::lockMap);
	LOCKCLASS lockClass2(Tile::lockTile);

	map->MoveMap(1, 0);

	Position pos = map->GetCorner();
	pos.x += 8;
	pos.y += 6;

	GetMapDescription(msg, 0, 0, 1, 14, pos, map);
	map->ReadMiniMap(0, 0, 1, 256);
	map->CalculateZ(pos);

	Lights::UpdateStaticLightMap(map);
}

void Protocol::ParseTileDescription(NetworkMessage* msg) {
	Position pos;
	GetPosition(msg, pos);

	Map* map = game->GetMap();
	if (!map) {
		Logger::AddLog("Protocol::ParseMapDescriptor()", "Pointer to map is NULL!", LOG_ERROR, msg);
		return;
	}

	LOCKCLASS lockClass1(Map::lockMap);
	LOCKCLASS lockClass2(Tile::lockTile);

	Tile* tile = map->GetTile(pos);
	if (tile)
		delete_debug(tile, M_PLACE);

	int skip = 0;
	tile = NULL;

	GetTileDescription(msg, pos, tile, skip);

	map->SetTile(pos, tile);
	sig.PushFunction(boost::bind(&Map::UpdateBattleContainer, map));

	Lights::UpdateStaticLightMap(map);
}

void Protocol::ParseAddThing(NetworkMessage* msg) {
	Position pos;
	GetPosition(msg, pos);

	Thing* thing = NULL;
	int skip = 0;
	GetThing(msg, thing, skip);

	Map* map = game->GetMap();
	if (!map) {
		Logger::AddLog("Protocol::ParseAddThing()", "Pointer to map is NULL!", LOG_ERROR, msg);
		return;
	}

	LOCKCLASS lockClass1(Map::lockMap);
	LOCKCLASS lockClass2(Tile::lockTile);

	Tile* tile = map->GetTile(pos);
	if (tile) {
		if (thing) {
			tile->AddThing(thing);

			Item* item = dynamic_cast<Item*>(thing);
			Creature* creature = dynamic_cast<Creature*>(thing);
			if (item) {
				item->SetLight();

				if ((*item)()) {
					if ((*item)()->light)
						Lights::UpdateStaticLightMap(map);
					if ((*item)()->blocking || (*item)()->blockPathFind)
						map->SetMiniMapPix(pos, tile);
				}
			}
			else if (creature) {
				Position corner = map->GetCorner();
				Battle* battle = map->GetBattle();
				if (battle && corner.z == pos.z)
					battle->AddCreature(creature);
				sig.PushFunction(boost::bind(&Map::UpdateBattle, map));
			}
		}
		tile->SortThings();
	}
	else {
		Logger::AddLog("Protocol::ParseAddThing()", "Tile is NULL!", LOG_ERROR, msg);
		return;
	}
}

void Protocol::ParseTransformThing(NetworkMessage* msg) {
	Position pos;
	GetPosition(msg, pos);
	unsigned char stackPos = msg->GetU8();

	Thing* thing = NULL;
	int skip = 0;
	GetThing(msg, thing, skip);

	Map* map = game->GetMap();
	if (!map) {
		Logger::AddLog("Protocol::ParseTransformThing()", "Pointer to map is NULL!", LOG_ERROR, msg);
		return;
	}

	LOCKCLASS lockClass1(Map::lockMap);
	LOCKCLASS lockClass2(Tile::lockTile);

	Tile* tile = map->GetTile(pos);
	if (tile) {
		if (thing) {
			tile->TransformThing(thing, stackPos);

			Item* item = dynamic_cast<Item*>(thing);
			if (item) {
				item->SetLight();

				Lights::UpdateStaticLightMap(map);
			}

			map->SetMiniMapPix(pos, tile);
		}
		tile->SortThings();
	}
	else {
		Logger::AddLog("Protocol::ParseTransformThing()", "Tile is NULL!", LOG_ERROR, msg);
		return;
	}
}

void Protocol::ParseRemoveThing(NetworkMessage* msg) {
	Position pos;
	GetPosition(msg, pos);
	unsigned char stackPos = msg->GetU8();

	Map* map = game->GetMap();
	if (!map) {
		Logger::AddLog("Protocol::ParseRemoveThing()", "Pointer to map is NULL!", LOG_ERROR, msg);
		return;
	}

	LOCKCLASS lockClass1(Map::lockMap);
	LOCKCLASS lockClass2(Tile::lockTile);

	Tile* tile = map->GetTile(pos);
	if (tile) {
		Thing* thing = tile->GetThingByStackPos(stackPos);
		Item* item = dynamic_cast<Item*>(thing);
		Creature* creature = dynamic_cast<Creature*>(thing);
		if (thing) {
			Position oldPos = thing->oldPos;
			tile->RemoveThing(thing);

			if (item) {
				if ((*item)() && ((*item)()->blocking || (*item)()->blockPathFind))
					map->SetMiniMapPix(pos, tile);

				bool light = false;
				if (item->light)
					light = true;

				delete_debug(thing, M_PLACE);

				if (light)
					Lights::UpdateStaticLightMap(map);
			}
			else if (creature) {
				map->SetMiniMapPix(pos, tile);

				Position corner = map->GetCorner();
				Battle* battle = map->GetBattle();
				if (battle && corner.z == pos.z)
					battle->RemoveCreature(creature);
				sig.PushFunction(boost::bind(&Map::UpdateBattleContainer, map));

				creature->SetLight(0, 0);

				Player* player = game->GetPlayer();
				if (player && player->GetAttackID() == creature->GetID()) {
					player->SetAttackID(0x00);

					Player::walking = false;
					Player::requestStop = false;
					player->SetTargetPos(Position(0, 0, 0));
				}
				if (player && player->GetFollowID() == creature->GetID())
					player->SetFollowID(0x00);

				map->RemoveTempCreature(oldPos, creature);
				if (creature->GetLastMove() == SOUTHWEST)
					map->RemoveTempCreature(Position(oldPos.x, oldPos.y + 1, oldPos.z), creature);
				else if (creature->GetLastMove() == NORTHEAST)
					map->RemoveTempCreature(Position(oldPos.x + 1, oldPos.y, oldPos.z), creature);
				creature->pos = Position(0, 0, 0);
			}
		}
		tile->SortThings();
	}
	else {
		Logger::AddLog("Protocol::ParseRemoveThing()", "Tile is NULL!", LOG_ERROR, msg);
		return;
	}
}

void Protocol::ParseMoveThing(NetworkMessage* msg) {
	Position posFrom;
	GetPosition(msg, posFrom);
	unsigned char stackPosFrom = msg->GetU8();

	Position posTo;
	GetPosition(msg, posTo);

	Map* map = game->GetMap();
	if (!map) {
		Logger::AddLog("Protocol::ParseMoveThing()", "Pointer to map is NULL!", LOG_ERROR, msg);
		return;
	}

	LOCKCLASS lockClass1(Map::lockMap);
	LOCKCLASS lockClass2(Tile::lockTile);

	Tile* tileFrom = map->GetTile(posFrom);
	Tile* tileTo = map->GetTile(posTo);
	if (tileFrom && tileTo) {
		Thing* thing = tileFrom->GetThingByStackPos(stackPosFrom);
		Position oldPos = Position(0, 0, 0);
		if (thing) {
			oldPos = thing->oldPos;
			tileFrom->RemoveThing(thing);
			tileTo->AddThing(thing);
		}
		tileFrom->SortThings();
		tileTo->SortThings();

		Player* player = game->GetPlayer();
		Item* item = dynamic_cast<Item*>(thing);
		Creature* creature = dynamic_cast<Creature*>(thing);
		if (item) {
			if ((*item)()) {
				if( (*item)()->light)
					Lights::UpdateStaticLightMap(map);

				if ((*item)()->blocking || (*item)()->blockPathFind) {
					map->SetMiniMapPix(posFrom, tileFrom);
					map->SetMiniMapPix(posTo, tileTo);
				}
			}

			item->step = -1.0f;
			Item::AddMovingItem(item);
		}
		else if (creature) {
			map->SetMiniMapPix(posFrom, tileFrom);
			map->SetMiniMapPix(posTo, tileTo);

			Direction direction = NORTH;
			if (posFrom.x == posTo.x && posFrom.y < posTo.y)
				direction = SOUTH;
			else if (posFrom.x == posTo.x && posFrom.y > posTo.y)
				direction = NORTH;
			else if (posFrom.x < posTo.x && posFrom.y == posTo.y)
				direction = EAST;
			else if (posFrom.x > posTo.x && posFrom.y == posTo.y)
				direction = WEST;
			else if (posFrom.x < posTo.x && posFrom.y < posTo.y)
				direction = SOUTHEAST;
			else if (posFrom.x > posTo.x && posFrom.y < posTo.y)
				direction = SOUTHWEST;
			else if (posFrom.x < posTo.x && posFrom.y > posTo.y)
				direction = NORTHEAST;
			else if (posFrom.x > posTo.x && posFrom.y > posTo.y)
				direction = NORTHWEST;

			map->RemoveTempCreature(oldPos, creature);
			if (creature->GetLastMove() == SOUTHWEST)
				map->RemoveTempCreature(Position(oldPos.x, oldPos.y + 1, oldPos.z), creature);
			else if (creature->GetLastMove() == NORTHEAST)
				map->RemoveTempCreature(Position(oldPos.x + 1, oldPos.y, oldPos.z), creature);

			tileTo->RemoveTempCreature(creature);

			tileFrom->AddTempCreature(creature);
			if (direction == SOUTHWEST)
				map->AddTempCreature(Position(creature->oldPos.x, creature->oldPos.y + 1, creature->oldPos.z), creature);
			else if (direction == NORTHEAST)
				map->AddTempCreature(Position(creature->oldPos.x + 1, creature->oldPos.y, creature->oldPos.z), creature);

			if (creature->newPos != Position(0, 0, 0)) {
				map->RemoveTempCreature(creature->newPos, creature);

				float step = creature->GetStep();
				creature->newPos = Position(0, 0, 0);
				creature->SetDirection(direction);
				creature->SetStep(-step);
			}
			else {
				creature->SetDirection(direction);
				if (posFrom.z == posTo.z)
					creature->SetStep(-1.0f);
				else
					creature->SetStep(0.0f);
			}

			creature->SetLastMove(direction);

			if (player) {
				Creature* playerCreature = player->GetCreature();
				if (creature == playerCreature) {
					Position targetPos = player->GetTargetPos();

					if (targetPos != Position(0, 0, 0)) {
						player->PopAutoPath();
						if (player->GetAutoPathSize() == 0) {
							Player::walking = false;
							Player::requestStop = false;
							player->SetTargetPos(Position(0, 0, 0));
						}
					}

					if (targetPos == creature->pos || targetPos == Position(0, 0, 0)) {
						Player::walking = false;
						Player::requestStop = false;
						player->SetTargetPos(Position(0, 0, 0));
					}

					MiniMap* minimap = map->GetMiniMap();
					if (minimap) {
						float offsetX;
						float offsetY;
						minimap->GetOffsetPos(offsetX, offsetY);
						char offsetZ = minimap->GetOffsetLevel();

						if (offsetX != 0.0f || offsetY != 0.0f)
							minimap->SetOffsetPos(0.0f, 0.0f);

						if (offsetZ != 0)
							map->ResetMiniMapLevel();
					}
				}
			}
		}
	}
	else {
		if (!tileFrom) {
			std::string error = "TileFrom(" + value2str(posFrom.x) + "/" + value2str(posFrom.y) + "/" + value2str(posFrom.z) + ") is NULL!";
			Logger::AddLog("Protocol::ParseMoveThing()", error.c_str(), LOG_ERROR, msg);
		}
		if (!tileTo) {
			std::string error = "TileTo(" + value2str(posTo.x) + "/" + value2str(posTo.y) + "/" + value2str(posTo.z) + ") is NULL!";
			Logger::AddLog("Protocol::ParseMoveThing()", error.c_str(), LOG_ERROR, msg);
		}
		return;
	}
}

void Protocol::ParseOpenContainer(NetworkMessage* msg) {
	unsigned char index = msg->GetU8();

	unsigned short containerID = msg->GetU16();
	std::string containerName = msg->GetString();

	unsigned char capacity = msg->GetU8();
	unsigned char child = msg->GetU8();
	unsigned char size = msg->GetU8();

	Container* container = new(M_PLACE) Container(index, containerID, containerName, capacity, (bool)child);

	for (int n = 0; n < size; n++) {
		Thing* thing = NULL;
		int skip = 0;
		GetThing(msg, thing, skip);
		Item* item = dynamic_cast<Item*>(thing);

		container->AddItem(item, true);
	}

	Windows* wnds = game->GetWindows();
	if (!wnds) {
		Logger::AddLog("Protocol::ParseOpenContainer()", "Pointer to windows is NULL!", LOG_WARNING);
		return;
	}

	wnds->OpenWindow(WND_CONTAINER, game, container);
}

void Protocol::ParseCloseContainer(NetworkMessage* msg) {
	unsigned char index = msg->GetU8();

	Windows* wnds = game->GetWindows();
	if (!wnds) {
		Logger::AddLog("Protocol::ParseCloseContainer()", "Pointer to windows is NULL!", LOG_WARNING);
		return;
	}

	ContainerWindow* cntWindow = ContainerWindow::GetContainerWindow(index);
	if (cntWindow)
		wnds->KillWindow(cntWindow);
}

void Protocol::ParseAddItemContainer(NetworkMessage* msg) {
	unsigned char index = msg->GetU8();

	Thing* thing = NULL;
	int skip = 0;
	GetThing(msg, thing, skip);
	Item* item = dynamic_cast<Item*>(thing);

	Container* container = Container::GetContainer(index);
	if (!container) {
		delete_debug(item, M_PLACE);
		//Logger::AddLog("Protocol::ParseAddItemContainer()", "Pointer to container is NULL!", LOG_WARNING);
		return;
	}

	container->AddItem(item);
}

void Protocol::ParseTransformItemContainer(NetworkMessage* msg) {
	unsigned char index = msg->GetU8();

	unsigned char slot = msg->GetU8();

	Thing* thing = NULL;
	int skip = 0;
	GetThing(msg, thing, skip);
	Item* item = dynamic_cast<Item*>(thing);

	Container* container = Container::GetContainer(index);
	if (!container) {
		delete_debug(item, M_PLACE);
		//Logger::AddLog("Protocol::ParseTransformItemContainer()", "Pointer to container is NULL!", LOG_WARNING);
		return;
	}

	container->TransformItem(slot, item);
}

void Protocol::ParseRemoveItemContainer(NetworkMessage* msg) {
	unsigned char index = msg->GetU8();

	unsigned char slot = msg->GetU8();

	Container* container = Container::GetContainer(index);
	if (!container) {
		//Logger::AddLog("Protocol::ParseRemoveItemContainer()", "Pointer to container is NULL!", LOG_WARNING);
		return;
	}

	container->RemoveItem(slot);
}

void Protocol::ParseUpdateInventoryItem(NetworkMessage* msg) {
	unsigned char slot = msg->GetU8();

	Thing* thing = NULL;
	int skip = 0;
	GetThing(msg, thing, skip);
	Item* item = dynamic_cast<Item*>(thing);

	Player* player = game->GetPlayer();
	if (player) {
		Container* container = player->GetInventory();
		container->TransformItem(slot - 1, item);
	}
	else
		delete_debug(item, M_PLACE);
}

void Protocol::ParseRemoveInventoryItem(NetworkMessage* msg) {
	unsigned char slot = msg->GetU8();

	Player* player = game->GetPlayer();
	if (player) {
		Container* container = player->GetInventory();
		container->TransformItem(slot - 1, NULL);
	}
}

void Protocol::ParseShop(NetworkMessage* msg) {
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
		item.weight = 0;
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
		Logger::AddLog("Protocol::ParseShop()", "Pointer to windows is NULL!", LOG_WARNING);
		return;
	}

	wnds->CloseWindows(WND_SHOP);
	wnds->OpenWindow(WND_SHOP, game);
}

void Protocol::ParseShopSaleItemList(NetworkMessage* msg) {
	unsigned int money = msg->GetU32();

	Shop* shop = game->GetShop();
	if (!shop) {
		Logger::AddLog("Protocol::ParseShopSaleItemList()", "Pointer to shop is NULL!", LOG_ERROR, msg);
		return;
	}

	shop->SetMoney(money);

	Windows* wnds = game->GetWindows();
	if (!wnds) {
		Logger::AddLog("Protocol::ParseShop()", "Pointer to windows is NULL!", LOG_WARNING);
		return;
	}

	Window* wnd = wnds->FindWindow(WND_SHOP);
	if (wnd)
		shop->UpdateMemo();
}

void Protocol::ParseCloseShop(NetworkMessage* msg) {
	Windows* wnds = game->GetWindows();
	if (!wnds) {
		Logger::AddLog("Protocol::ParseCloseShop()", "Pointer to windows is NULL!", LOG_WARNING);
		return;
	}

	wnds->CloseWindows(WND_SHOP);
}

void Protocol::ParseAcceptTrade(NetworkMessage* msg) {
	std::string name = msg->GetString();

	unsigned char size = msg->GetU8();

	Container* container = new(M_PLACE) Container(CONTAINER_TRADE, 0, name, size, false);
	for (int n = 0; n < size; n++) {
		Thing* thing = NULL;
		int skip = 0;
		GetThing(msg, thing, skip);
		Item* item = dynamic_cast<Item*>(thing);

		if (container)
			container->AddItem(item, true);
		else
			delete_debug(item, M_PLACE);
	}

	Trade* trade = game->GetTrade();
	if (trade) {
		Windows* wnds = game->GetWindows();
		if (wnds)
			wnds->OpenWindow(WND_TRADE, game);

		trade->SetContainerP1(container);
		trade->UpdateContainer();
	}
	else
		delete_debug(container, M_PLACE);
}

void Protocol::ParseRequestTrade(NetworkMessage* msg) {
	std::string name = msg->GetString();

	unsigned char size = msg->GetU8();

	Container* container = new(M_PLACE) Container(CONTAINER_TRADE, 1, name, size, false);
	for (int n = 0; n < size; n++) {
		Thing* thing = NULL;
		int skip = 0;
		GetThing(msg, thing, skip);
		Item* item = dynamic_cast<Item*>(thing);

		if (container)
			container->AddItem(item, true);
		else
			delete_debug(item, M_PLACE);
	}

	Trade* trade = game->GetTrade();
	if (trade) {
		trade->SetContainerP2(container);
		trade->UpdateContainer();
	}
	else
		delete_debug(container, M_PLACE);
}

void Protocol::ParseCloseTrade(NetworkMessage* msg) {
	Trade* trade = game->GetTrade();
	if (trade) {
		Container* tradeP1 = trade->GetContainerP1();
		Container* tradeP2 = trade->GetContainerP2();
		if (tradeP1)
			delete_debug(tradeP1, M_PLACE);
		if (tradeP2)
			delete_debug(tradeP2, M_PLACE);

		trade->SetContainerP1(NULL);
		trade->SetContainerP2(NULL);
	}

	Windows* wnds = game->GetWindows();
	if (wnds)
		wnds->CloseWindows(WND_TRADE);
	else
		Logger::AddLog("Protocol::ParseCloseTrade()", "Pointer to windows is NULL!", LOG_WARNING);
}

void Protocol::ParseWorldLight(NetworkMessage* msg) {
	unsigned char lightLevel = msg->GetU8();
	unsigned char lightColor = msg->GetU8();

	LOCKCLASS lockClass1(Map::lockMap);
	LOCKCLASS lockClass2(Tile::lockTile);

	game->SetWorldLight(lightLevel, lightColor);

#ifndef NO_SOUND
	Player* player = game->GetPlayer();
	SFX_System* sfx = game->GetSFX();
	if (sfx) {
		Position playerPos = player->GetPosition();
		if (playerPos.z <= 7) {
			if (lightLevel > 80 && sfx->GetBackgroundSample() != 0)
				sfx->PlayBackgroundSound(SAMPLE_BACKGROUND, 0);
			else if (lightLevel <= 80 && sfx->GetBackgroundSample() != 1)
				sfx->PlayBackgroundSound(SAMPLE_BACKGROUND, 1);
		}
	}
#endif
}

void Protocol::ParseMagicEffect(NetworkMessage* msg) {
	Position pos;
	GetPosition(msg, pos);
	unsigned char type = msg->GetU8();

	MagicEffect* me = new(M_PLACE) MagicEffect;
	if (!me->Create(type, pos)) {
		delete_debug(me, M_PLACE);
		return;
	}

	Map* map = game->GetMap();
	if (map) {
		if (Game::options.printParticles) {
			Particles* particles = game->GetParticles();
			if (particles) {
				ParticleCondition* condition = Particles::GetParticleCondition(type - 1);
				if (condition) {
					Tile* tile = map->GetTile(pos);
					int height = (tile ? tile->GetDownHeight() : 0);

					for (int i = 0; i < condition->count; i++) {
						Particle* particle = new(M_PLACE) Particle(condition, pos);
						particle->SetPosition(- height, - height);
						Particles::AddParticle(particle);
					}
				}
			}
			else
				Logger::AddLog("Protocol::ParseMagicEffect()", "Pointer to particles is NULL!", LOG_WARNING);
		}

#ifndef NO_SOUND
		Player* player = game->GetPlayer();
		SFX_System* sfx = game->GetSFX();
		if (sfx) {
			Position playerPos = player->GetPosition();
			float x = 0;//(pos.x - playerPos.x);
			float y = 0;//(pos.y - playerPos.y);
			float z = (pos.z - playerPos.z) * 4;
			sfx->PlayGameSound(SAMPLE_EFFECT, type - 1, x, y, z);
		}
		else
			Logger::AddLog("Protocol::ParseMagicEffect()", "Pointer to sfx is NULL!", LOG_WARNING);
#endif
	}
	else
		Logger::AddLog("Protocol::ParseMagicEffect()", "Pointer to map is NULL!", LOG_WARNING);
}

void Protocol::ParseAnimatedText(NetworkMessage* msg) {
	Position pos;
	GetPosition(msg, pos);
	unsigned char color = msg->GetU8();
	std::string text = msg->GetString();

	Messages::AddMessage(new(M_PLACE) AnimatedMessage(TextString(text, color), 2000, pos));
}

void Protocol::ParseDistanceShot(NetworkMessage* msg) {
	Position posFrom;
	GetPosition(msg, posFrom);
	Position posTo;
	GetPosition(msg, posTo);
	unsigned char type = msg->GetU8();

	Distance* dist = new(M_PLACE) Distance(type, posFrom, posTo);

#ifndef NO_SOUND
	Player* player = game->GetPlayer();
	SFX_System* sfx = game->GetSFX();
	if (sfx) {
		float x = 0;//(posFrom.x - player->GetPosition().x);
		float y = 0;//(posFrom.y - player->GetPosition().y);
		float z = (posFrom.z - player->GetPosition().z) * 4;
		sfx->PlayGameSound(SAMPLE_DISTANCE, type - 1, x, y, z);
	}
#endif
}

void Protocol::ParseCreatureSquare(NetworkMessage* msg) {
	unsigned int creatureID = msg->GetU32();
	unsigned char color = msg->GetU8();

	Creature* creature = Creature::GetFromKnown(creatureID);
	if (creature)
		creature->SetSquare(RealTime::getTime(), color);

	Map* map = game->GetMap();
	if (map)
		sig.PushFunction(boost::bind(&Map::UpdateBattleContainer, map));
}

void Protocol::ParseCreatureHealth(NetworkMessage* msg) {
	unsigned int creatureID = msg->GetU32();
	unsigned char health = msg->GetU8();

	Creature* creature = Creature::GetFromKnown(creatureID);
	if (creature)
		creature->SetHealth(health);
}

void Protocol::ParseCreatureLight(NetworkMessage* msg) {
	unsigned int creatureID = msg->GetU32();
	unsigned char lightLevel = msg->GetU8();
	unsigned char lightColor = msg->GetU8();

	LOCKCLASS lockClass1(Map::lockMap);
	LOCKCLASS lockClass2(Tile::lockTile);

	Creature* creature = Creature::GetFromKnown(creatureID);
	if (creature)
		creature->SetLight(lightColor, lightLevel);
}

void Protocol::ParseCreatureOutfit(NetworkMessage* msg) {
	unsigned int creatureID = msg->GetU32();

	Outfit outfit;
	GetOutfit(msg, &outfit);

	Creature* creature = Creature::GetFromKnown(creatureID);
	if (creature)
		creature->SetOutfit(outfit);

	Map* map = game->GetMap();
	if (map)
		sig.PushFunction(boost::bind(&Map::UpdateBattleContainer, map));
}

void Protocol::ParseCreatureSpeed(NetworkMessage* msg) {
	unsigned int creatureID = msg->GetU32();
	unsigned short speed = msg->GetU16();

	Creature* creature = Creature::GetFromKnown(creatureID);
	if (creature)
		creature->SetSpeed(speed);
}

void Protocol::ParseCreatureSkull(NetworkMessage* msg) {
	unsigned int creatureID = msg->GetU32();
	unsigned char skull = msg->GetU8();

	Creature* creature = Creature::GetFromKnown(creatureID);
	if (creature)
		creature->SetSkull(skull);

	Map* map = game->GetMap();
	if (map)
		sig.PushFunction(boost::bind(&Map::UpdateBattleContainer, map));
}

void Protocol::ParseCreatureShield(NetworkMessage* msg) {
	unsigned int creatureID = msg->GetU32();
	unsigned char shield = msg->GetU8();

	Creature* creature = Creature::GetFromKnown(creatureID);
	if (creature)
		creature->SetShield(shield);

	Map* map = game->GetMap();
	if (map)
		sig.PushFunction(boost::bind(&Map::UpdateBattleContainer, map));
}

void Protocol::ParseCreaturePZSwitch(NetworkMessage* msg) {
	msg->GetU32();
	msg->GetU8();
}

void Protocol::ParseTextWindow(NetworkMessage* msg) {
	unsigned int textWindowID = msg->GetU32();
	unsigned short itemID = msg->GetU16();
	unsigned short maxLength = msg->GetU16();
	std::string message = msg->GetString();
	std::string writer = msg->GetString();
	std::string date = msg->GetString();

	Windows* wnds = game->GetWindows();
	if (!wnds) {
		Logger::AddLog("Protocol::ParseTextWindow()", "Pointer to windows is NULL!", LOG_WARNING);
		return;
	}

	wnds->CloseWindows(WND_TEXT);
	wnds->CloseWindows(WND_HOUSETEXT);
	wnds->OpenWindow(WND_TEXT, game, textWindowID, itemID, &message, &writer, &date, maxLength);
}

void Protocol::ParseHouseWindow(NetworkMessage* msg) {
	unsigned char listID = msg->GetU8();
	unsigned short itemID = msg->GetU16();
	unsigned short maxLength = msg->GetU16();
	std::string message = msg->GetString();

	Windows* wnds = game->GetWindows();
	if (!wnds) {
		Logger::AddLog("Protocol::ParseHouseWindow()", "Pointer to windows is NULL!", LOG_WARNING);
		return;
	}

	wnds->CloseWindows(WND_TEXT);
	wnds->CloseWindows(WND_HOUSETEXT);
	wnds->OpenWindow(WND_HOUSETEXT, game, listID, 1, itemID, &message, maxLength);
}

void Protocol::ParsePlayerStats(NetworkMessage* msg) {
	Player* player = game->GetPlayer();
	Statistics* statistics = player->GetStatistics();

	statistics->health = msg->GetU16();
	statistics->maxHealth = msg->GetU16();

	statistics->capacity = (double)msg->GetU16();
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

void Protocol::ParsePlayerSkills(NetworkMessage* msg) {
	Player* player = game->GetPlayer();
	Statistics* statistics = player->GetStatistics();

	for (int i = 0; i < 7; i++) {
		statistics->skill[i] = msg->GetU8();
		statistics->skill_p[i] = msg->GetU8();
	}
}

void Protocol::ParsePlayerIcons(NetworkMessage* msg) {
	unsigned short icons = msg->GetU16();

	Status* status = game->GetStatus();
	if (status) {
		status->SetIcons(icons);
		status->UpdateContainer();
	}
}

void Protocol::ParseCancelAttack(NetworkMessage* msg) {
	Player* player = game->GetPlayer();
	if (player) {
		player->SetAttackID(0x00);

		Player::walking = false;
		Player::requestStop = false;
		player->SetTargetPos(Position(0, 0, 0));
	}
}

void Protocol::ParseSpellCooldown(NetworkMessage* msg) {
}

void Protocol::ParseSpellGroupCooldown(NetworkMessage* msg) {
}

void Protocol::ParseCreatureSpeak(NetworkMessage* msg) {
	unsigned int creatureID = msg->GetU32();
	std::string creatureName = msg->GetString();
	unsigned short level = msg->GetU16();

	unsigned char type = msg->GetU8();

	Position pos(0, 0, 0);
	Channel* channel = NULL;
	unsigned char color = 215;
	switch(type) {
		case SPEAK822_SAY:
			GetPosition(msg, pos);
			color = 210;
			break;
		case SPEAK822_WHISPER:
			GetPosition(msg, pos);
			color = 200;
			break;
		case SPEAK822_YELL:
			GetPosition(msg, pos);
			color = 210;
			break;
		case SPEAK822_PRIVATE_PN:
			channel = Channel::GetChannel(CHANNEL_NPC);
			break;
		case SPEAK822_PRIVATE_NP:
			channel = Channel::GetChannel(CHANNEL_NPC);
			GetPosition(msg, pos);
			color = 143;
			break;
		case SPEAK822_PRIVATE:
			channel = Channel::GetChannel(creatureName);
			color = 143;
			break;
		case SPEAK822_PRIVATE_RED:
			channel = Channel::GetChannel(creatureName);
			color = 201;
			break;
		case SPEAK822_MONSTER_SAY:
			GetPosition(msg, pos);
			color = 186;
			break;
		case SPEAK822_MONSTER_YELL:
			GetPosition(msg, pos);
			color = 186;
			break;
		case SPEAK822_CHANNEL_Y:
			channel = Channel::GetChannel(msg->GetU16());
			color = 210;
			break;
		case SPEAK822_CHANNEL_W:
			channel = Channel::GetChannel(msg->GetU16());
			color = 215;
			break;
		case SPEAK822_CHANNEL_R1:
		case SPEAK822_CHANNEL_R2:
			channel = Channel::GetChannel(msg->GetU16());
			color = 180;
			break;
		case SPEAK822_CHANNEL_O:
			channel = Channel::GetChannel(msg->GetU16());
			color = 198;
			break;
		case SPEAK822_BROADCAST:
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
		if (type == SPEAK822_PRIVATE_NP)
			textStr = SetTextColorMap(text, color, 131);
		else
			textStr = TextString(text, color);

		TextString msg = ScrollText(textStr, 30, 0);
		Messages::AddMessage(new(M_PLACE) TextMessage(msg, disappearingSpeed + text.length() * 75, pos));
	}

	if (type == SPEAK822_PRIVATE) {
		std::string mtext = creatureName + (Game::adminOptions.hideLevels ? std::string(": ") : std::string(" (" + value2str(level) + "): ")) + text;
		TextString msg = ScrollText(TextString(mtext, color), 40, 0);
		Messages::AddMessage(new(M_PLACE) SystemMessage(msg, disappearingSpeed + text.length() * 75, 0.0f, -0.5f));
	}

	if (type == SPEAK822_BROADCAST) {
		TextString msg = ScrollText(TextString(text, color), 40, 0);
		Messages::AddMessage(new(M_PLACE) SystemMessage(msg, disappearingSpeed + text.length() * 75, 0.0f, 0.5f));
	}

	if (type == SPEAK822_PRIVATE_PN || type == SPEAK822_PRIVATE_NP) {
		ChatUsers users;
		ChatUsers invited;
		game->OpenChannel(CHANNEL_NPC, "NPC", users, invited);
		if (!channel)
			channel = Channel::GetChannel(CHANNEL_NPC);
	}

	if (type != SPEAK822_MONSTER_SAY && type != SPEAK822_MONSTER_YELL) {
		time_t now = RealTime::getTime() / 1000;
		std::string mtext = creatureName + (Game::adminOptions.hideLevels ? std::string(": ") : std::string(" (" + value2str(level) + "): ")) + text;
		std::string message = time2str(now, false, false, false, true, true, false) + " " + mtext;
		if (type == SPEAK822_PRIVATE_NP)
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

void Protocol::ParseChannels(NetworkMessage* msg) {
	unsigned char size = msg->GetU8();

	game->ClearChannelsList();
	for (int n = 0; n < size; n++) {
		unsigned short channelID = msg->GetU16();
		std::string channelName = msg->GetString();

		Channel channel(channelID, channelName);
		game->AddChannel(channel);
	}

	Channel channel(CHANNEL_NPC, "NPC");
	game->AddChannel(channel);

	Windows* wnds = game->GetWindows();
	if (!wnds) {
		Logger::AddLog("Protocol::ParseChannels()", "Pointer to windows is NULL!", LOG_WARNING);
		return;
	}

	wnds->CloseWindows(WND_CHANNELSLIST);
	wnds->OpenWindow(WND_CHANNELSLIST, game);
}

void Protocol::ParseOpenChannel(NetworkMessage* msg) {
	unsigned short channelID = msg->GetU16();
	std::string channelName = msg->GetString();

	ChatUsers users;
	ChatUsers invited;
	game->OpenChannel(channelID, channelName, users, invited);
}

void Protocol::ParseOpenPrivate(NetworkMessage* msg) {
	std::string name = msg->GetString();

	ChatUsers users;
	ChatUsers invited;
	game->OpenChannel(CHANNEL_PRIVATE, name, users, invited);
}

void Protocol::ParseCreatePrivateChannel(NetworkMessage* msg) {
	unsigned short channelID = msg->GetU16();
	std::string channelName = msg->GetString();

	ChatUsers users;
	ChatUsers invited;
	game->OpenChannel(channelID, channelName, users, invited);
}

void Protocol::ParseCloseChannel(NetworkMessage* msg) {
	unsigned short channelID = msg->GetU16();

	//game->CloseChannel(channelID);
}

void Protocol::ParseTextMessage(NetworkMessage* msg) {
	unsigned char mclass = msg->GetU8();
	std::string text = msg->GetString();

	unsigned char color = 215;
	bool onChannel = false;
	bool onScreen = false;
	float px = 0.0f;
	float py = 0.5f;

	switch(mclass) {
		case MSG822_ORANGE:
		case MSG822_ORANGE2:
			color = 198;
			onChannel = true;
			break;
		case MSG822_RED_TEXT:
			color = 180;
			onChannel = true;
			onScreen = true;
			break;
		case MSG822_ADVANCE:
			color = 215;
			onChannel = true;
			onScreen = true;
			break;
		case MSG822_EVENT:
		case MSG822_STATUS_DEFAULT:
			color = 215;
			onChannel = true;
			onScreen = true;
			py = 1.0f;
			break;
		case MSG822_INFO:
			color = 30;
			onChannel = true;
			onScreen = true;
			break;
		case MSG822_SMALLINFO:
			color = 215;
			onScreen = true;
			py = 1.0f;
			break;
		case MSG822_BLUE_TEXT:
			color = 89;
			onChannel = true;
			break;
		case MSG822_RED_INFO:
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

void Protocol::ParseCancelWalk(NetworkMessage* msg) {
	Direction direction = (Direction)msg->GetU8();

	Player* player = game->GetPlayer();
	Creature* creature = (player ? player->GetCreature() : NULL);

	creature->SetDirection(direction);
	if (creature->newPos != Position(0, 0, 0)) {
		Map* map = game->GetMap();
		if (map) {
			Tile* tile = map->GetTile(creature->newPos);
			if (tile)
				tile->RemoveTempCreature(creature);
		}
		else {
			Logger::AddLog("Protocol::ParseCancelWalk()", "Pointer to map is NULL!", LOG_ERROR, msg);
			return;
		}

		creature->newPos = Position(0, 0, 0);
	}

	if (player) {
		Position targetPos = player->GetTargetPos();
		if (!Player::requestStop) {
			player->PopAutoPath();
			if (player->GetAutoPathSize() > 0)
				game->PlayerStopWalk();
			else {
				Player::walking = false;
				Player::requestStop = false;
			}
		}
		else {
			Player::walking = false;
			Player::requestStop = false;
		}

		if (targetPos == Position(0, 0, 0)) {
			LOCKCLASS lockClass1(Map::lockMap);
			LOCKCLASS lockClass2(Tile::lockTile);

			creature->newPos = Position(0, 0, 0);
			creature->SetStep(0.0f);
		}

		player->SetFollowID(0x00);
	}
}

void Protocol::ParseMovePlayerUp(NetworkMessage* msg) {
	Map* map = game->GetMap();
	if (!map) {
		Logger::AddLog("Protocol::ParseMovePlayerUp()", "Pointer to map is NULL!", LOG_ERROR, msg);
		return;
	}

	LOCKCLASS lockClass1(Map::lockMap);
	LOCKCLASS lockClass2(Tile::lockTile);

	Player* player = game->GetPlayer();
	Creature* creature = player->GetCreature();

	Position pos;
	pos.x = creature->oldPos.x;
	pos.y = creature->oldPos.y;
	pos.z = creature->oldPos.z - 1;
	creature->SetStep(0.0f);

	int offset = 0;
	int start_z, stop_z, step_z = 0;
	if (pos.z == 7) {
		start_z = pos.z - 2;
		stop_z = 0;
		step_z = -1;

		offset = -1;
	}
	else if (pos.z > 7) {
		start_z = pos.z - 2;
		stop_z = pos.z - 2;
		step_z = 1;
	}

	if (step_z != 0)
		map->DeleteMap(0, 0, 18, 14, (start_z < stop_z ? start_z : stop_z), (start_z < stop_z ? stop_z : start_z));

	map->SetCorner(Position(pos.x - 8 + 1, pos.y - 6 + 1, pos.z));
	pos.x -= offset;
	pos.y -= offset;

	int skip = 0;
	if (step_z != 0) {
		for (int nz = start_z; nz != stop_z + step_z; nz += step_z)
			GetFloorDescription(msg, 0, 0, nz, 18, 14, pos, map, skip);
	}

	map->ReadMiniMap();
	map->UpdateMiniMap();
	map->CalculateZ(pos);

#ifndef NO_SOUND
	game->UpdateBackgroundSound(creature->pos.z, false);
#endif

	sig.PushFunction(boost::bind(&Map::UpdateBattle, map));
	map->WriteMiniMap();
	map->SortThings();
}

void Protocol::ParseMovePlayerDown(NetworkMessage* msg) {
	Map* map = game->GetMap();
	if (!map) {
		Logger::AddLog("Protocol::ParseMovePlayerDown()", "Pointer to map is NULL!", LOG_ERROR, msg);
		return;
	}

	LOCKCLASS lockClass1(Map::lockMap);
	LOCKCLASS lockClass2(Tile::lockTile);

	Player* player = game->GetPlayer();
	Creature* creature = player->GetCreature();

	Position pos;
	pos.x = creature->oldPos.x;
	pos.y = creature->oldPos.y;
	pos.z = creature->oldPos.z + 1;
	creature->SetStep(0.0f);

	int offset = 0;
	int start_z, stop_z, step_z = 0;
	if (pos.z == 8) {
		start_z = pos.z;
		stop_z = pos.z + 2;
		step_z = 1;

		offset = 1;
	}
	else if (pos.z > 8 && pos.z < 14) {
		start_z = pos.z + 2;
		stop_z = pos.z + 2;
		step_z = -1;
	}

	if (step_z != 0)
		map->DeleteMap(0, 0, 18, 14, (start_z < stop_z ? start_z : stop_z), (start_z < stop_z ? stop_z : start_z));

	map->SetCorner(Position(pos.x - 8 - 1, pos.y - 6 - 1, pos.z));
	pos.x -= offset;
	pos.y -= offset;

	int skip = 0;
	if (step_z != 0) {
		for (int nz = start_z; nz != stop_z + step_z; nz += step_z)
			GetFloorDescription(msg, 0, 0, nz, 18, 14, pos, map, skip);
	}

	map->ReadMiniMap();
	map->UpdateMiniMap();
	map->CalculateZ(pos);

#ifndef NO_SOUND
	game->UpdateBackgroundSound(creature->pos.z, false);
#endif

	sig.PushFunction(boost::bind(&Map::UpdateBattle, map));
	map->WriteMiniMap();
	map->SortThings();
}

void Protocol::ParseOutfitWindow(NetworkMessage* msg) {
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

	std::vector<OutfitType>* mounts = new(M_PLACE) std::vector<OutfitType>;	//Not implemented in 822 version

	Windows* wnds = game->GetWindows();
	if (wnds)
		wnds->OpenWindow(WND_CHANGEOUTFIT, game, &outfit, outfits, mounts);
	else {
		delete_debug(outfits, M_PLACE);
		delete_debug(mounts, M_PLACE);

		Logger::AddLog("Protocol::ParseOutfitWindow()", "Pointer to windows is NULL!", LOG_WARNING);
	}
}

void Protocol::ParseVIPList(NetworkMessage* msg) {
	unsigned int creatureID = msg->GetU32();
	std::string name = msg->GetString();
	unsigned char online = msg->GetU8();

	VIPList* viplist = game->GetVIPList();
	if (viplist) {
		viplist->AddCreature(creatureID, name, (bool)online);
		viplist->UpdateContainer();
	}
}

void Protocol::ParseVIPLogin(NetworkMessage* msg) {
	unsigned int creatureID = msg->GetU32();

	VIPList* viplist = game->GetVIPList();
	if (viplist) {
		viplist->LoginCreature(creatureID);
		viplist->UpdateContainer();
	}
}

void Protocol::ParseVIPLogout(NetworkMessage* msg) {
	unsigned int creatureID = msg->GetU32();

	VIPList* viplist = game->GetVIPList();
	if (viplist) {
		viplist->LogoutCreature(creatureID);
		viplist->UpdateContainer();
	}
}

void Protocol::ParseTutorial(NetworkMessage* msg) {
	unsigned char tutorial = msg->GetU8();
}

void Protocol::ParseMarker(NetworkMessage* msg) {
	Position pos;
	GetPosition(msg, pos);
	unsigned char type = msg->GetU8();
	std::string description = msg->GetString();
}

void Protocol::ParseQuestLog(NetworkMessage* msg) {
	unsigned short size = msg->GetU16();

	QuestLog* questlog = game->GetQuestLog();
	if (!questlog) {
		Logger::AddLog("Protocol::ParseQuestLog()", "Pointer to questlog is NULL!", LOG_ERROR, msg);
		return;
	}

	questlog->ClearQuests();
	for (int n = 0; n < size; n++) {
		unsigned short questID = msg->GetU16();
		Quest quest;
		quest.name = msg->GetString();
		quest.completed = (bool)msg->GetU8();

		questlog->AddQuest(questID, quest);
	}

	Windows* wnds = game->GetWindows();
	if (!wnds) {
		Logger::AddLog("Protocol::ParseQuestLog()", "Pointer to windows is NULL!", LOG_WARNING);
		return;
	}

	wnds->CloseWindows(WND_QUESTS);
	wnds->OpenWindow(WND_QUESTS, game);
}

void Protocol::ParseQuestLine(NetworkMessage* msg) {
	unsigned short questID = msg->GetU16();
	unsigned short size = msg->GetU8();

	QuestLog* questlog = game->GetQuestLog();
	if (!questlog) {
		Logger::AddLog("Protocol::ParseQuestLine()", "Pointer to questlog is NULL!", LOG_ERROR, msg);
		return;
	}

	questlog->ClearMissions();
	for (int n = 0; n < size; n++) {
		Mission mission;
		mission.name = msg->GetString();
		mission.description = msg->GetString();

		questlog->AddMission(mission);
	}

	Windows* wnds = game->GetWindows();
	if (!wnds) {
		Logger::AddLog("Protocol::ParseQuestLine()", "Pointer to windows is NULL!", LOG_WARNING);
		return;
	}

	wnds->CloseWindows(WND_MISSIONS);
	wnds->OpenWindow(WND_MISSIONS, game);
}

//Extended Protocol
void Protocol::ParseStatusPeriods(NetworkMessage* msg) {
	Status* status = game->GetStatus();
	if (!status) {
		Logger::AddLog("Protocol::ParseStatusPeriods()", "Pointer to status is NULL!", LOG_ERROR, msg);
		return;
	}

	unsigned char size = msg->GetU8();
	for (int i = 0; i < size; i++) {
		unsigned char id = msg->GetU8();
		unsigned int period = msg->GetU32();

		status->SetPeriod(id, period);
	}
}

void Protocol::ParseSpellsWindow(NetworkMessage* msg) {
	SpellsList spells;

	unsigned char size = msg->GetU8();
	for (int i = 0; i < size; i++) {
		unsigned short spellID = msg->GetU8();
		std::string words = msg->GetString();
		std::string name = msg->GetString();

		spells.push_back(std::pair<unsigned char, SpellPair>(spellID, SpellPair(words, name)));
	}

	Windows* wnds = game->GetWindows();
	if (!wnds) {
		Logger::AddLog("Protocol::ParseSpellsWindow()", "Pointer to windows is NULL!", LOG_WARNING);
		return;
	}

	wnds->OpenWindow(WND_SPELLS, game, &spells);
}

void Protocol::ParseScreenShot(NetworkMessage* msg) {
	Game::screenshot = true;
}

void Protocol::ParseCreatureEquipment(NetworkMessage* msg) {
    unsigned int creatureID = msg->GetU32();

    Creature* creature = Creature::GetFromKnown(creatureID);
    if (!creature) {
        Logger::AddLog("Protocol::ParseCreatureEquipment()", "Creature not found!", LOG_ERROR);
        return;
    }

    Container* container = new(M_PLACE) Container(CONTAINER_EQUIPMENT, 0, "", 10, false);

    if (container) {
        for (int i = 0; i < 10; i++) {
            Thing* thing = NULL;
            int skip = 0;
            GetThing(msg, thing, skip);
            Item* item = dynamic_cast<Item*>(thing);

			container->AddItem(item, true);
        }

        creature->SetEquipment(RealTime::getTime(), container);
    }
    else
        Logger::AddLog("Protocol::ParseCreatureEquipment()", "Container not exist!", LOG_ERROR);
}

void Protocol::ParseSound(NetworkMessage* msg) {
	unsigned int type = msg->GetU32();
	unsigned char volume = msg->GetU8();
	unsigned char offset = msg->GetU8();

#ifndef NO_SOUND
	Player* player = game->GetPlayer();
	SFX_System* sfx = game->GetSFX();
	if (sfx) {
		Position playerPos = player->GetPosition();
		sfx->PlayCustomSound(SAMPLE_CUSTOM, type, (float)volume / 255, (float)(offset - 100) / 200);
	}
	else
		Logger::AddLog("Protocol::ParseSound()", "Pointer to sfx is NULL!", LOG_WARNING);
#endif
}

void Protocol::ParseManageBot(NetworkMessage* msg) {
	unsigned char enabled = msg->GetU8();

	Bot::enabled = (bool)enabled;
	if (!enabled)
		Bot::Stop();
}

void Protocol::ParseDirectionArrow(NetworkMessage* msg) {
	Position pos;
	GetPosition(msg, pos);
}

void Protocol::ParsePositionArrow(NetworkMessage* msg) {
	Position pos;
	GetPosition(msg, pos);

	unsigned short offset = msg->GetU16();
}

void Protocol::ParseScriptPacket(NetworkMessage* msg) {
	LOCKCLASS lockClass(lockProtocol);

	std::string scriptName = msg->GetString();
	unsigned short size = msg->GetU16();

	NetworkMessage* nmsg = msg->Clone(size);
	if (nmsg) {
		std::string execution = scriptName + ";receive;" + value2str((unsigned int)nmsg);
		LuaScript::RunScript(execution, false, true);
		delete_debug(nmsg, M_PLACE);
	}

	msg->Skip(size);
}


void Protocol::ParsePacketLogin(NetworkMessage* msg) {
	LOCKCLASS lockClass1(game->lockGame);
	LOCKCLASS lockClass2(lockProtocol);

	while(!msg->EndOfPacket()) {
		std::string error;
		unsigned char id = msg->GetU8();
		Logger::AddPacketType(id);

		switch(id) {
			case 0x0A: //MessageBox
				ParseMessageBox(msg);
				break;
			case 0x14: //MessageBox
				ParseMessageBox(msg);
				break;
			case 0x16: //PlaceInQueue
				ParsePlacedInQueue(msg);
				break;
			case 0x1E: //Ping
				ParsePing(msg);
				break;
			case 0x64: //CharactersList
				ParseCharactersList(msg);
				break;
			default:
				error = "Unknown packet type! [" + value2str(id, true) + "]";
				Logger::AddLog("Protocol::ParsePacketLogin()", error.c_str(), LOG_ERROR, msg);
				break;
		}
	}
}

void Protocol::ParsePacketGame(NetworkMessage* msg) {
	LOCKCLASS lockClass1(game->lockGame);
	LOCKCLASS lockClass2(lockProtocol);

	while(!msg->EndOfPacket()) {
		std::string error;
		unsigned short id = msg->GetU8();
		if (id == 0x00)
			id = 0x100 + msg->GetU8();
		Logger::AddPacketType(id);

		switch(id) {
			case 0x0A:
				ParsePlayerAppear(msg);
				break;
			case 0x0B:
				ParseViolation(msg);
				break;
			case 0x14:
				ParseMessageBox(msg);
				break;
			case 0x15:
				ParseFYIBox(msg);
				break;
			case 0x16:
				ParsePlacedInQueue(msg);
				break;
			case 0x1E:
				ParsePing(msg);
				break;
			case 0x28:
				ParseReLogin(msg);
				break;
			case 0x64:
				ParseMapDescriptor(msg);
				break;
			case 0x65:
				ParseMoveMapUp(msg);
				break;
			case 0x66:
				ParseMoveMapRight(msg);
				break;
			case 0x67:
				ParseMoveMapDown(msg);
				break;
			case 0x68:
				ParseMoveMapLeft(msg);
				break;
			case 0x69:
				ParseTileDescription(msg);
				break;
			case 0x6A:
				ParseAddThing(msg);
				break;
			case 0x6B:
				ParseTransformThing(msg);
				break;
			case 0x6C:
				ParseRemoveThing(msg);
				break;
			case 0x6D:
				ParseMoveThing(msg);
				break;
			case 0x6E:
				ParseOpenContainer(msg);
				break;
			case 0x6F:
				ParseCloseContainer(msg);
				break;
			case 0x70:
				ParseAddItemContainer(msg);
				break;
			case 0x71:
				ParseTransformItemContainer(msg);
				break;
			case 0x72:
				ParseRemoveItemContainer(msg);
				break;
			case 0x78:
				ParseUpdateInventoryItem(msg);
				break;
			case 0x79:
				ParseRemoveInventoryItem(msg);
				break;
			case 0x7A:
				ParseShop(msg);
				break;
			case 0x7B:
				ParseShopSaleItemList(msg);
				break;
			case 0x7C:
				ParseCloseShop(msg);
				break;
			case 0x7D:
				ParseAcceptTrade(msg);
				break;
			case 0x7E:
				ParseRequestTrade(msg);
				break;
			case 0x7F:
				ParseCloseTrade(msg);
				break;
			case 0x82:
				ParseWorldLight(msg);
				break;
			case 0x83:
				ParseMagicEffect(msg);
				break;
			case 0x84:
				ParseAnimatedText(msg);
				break;
			case 0x85:
				ParseDistanceShot(msg);
				break;
			case 0x86:
				ParseCreatureSquare(msg);
				break;
			case 0x8C:
				ParseCreatureHealth(msg);
				break;
			case 0x8D:
				ParseCreatureLight(msg);
				break;
			case 0x8E:
				ParseCreatureOutfit(msg);
				break;
			case 0x8F:
				ParseCreatureSpeed(msg);
				break;
			case 0x90:
				ParseCreatureSkull(msg);
				break;
			case 0x91:
				ParseCreatureShield(msg);
				break;
			case 0x92:
				ParseCreaturePZSwitch(msg);
				break;
			case 0x96:
				ParseTextWindow(msg);
				break;
			case 0x97:
				ParseHouseWindow(msg);
				break;
			case 0xA0:
				ParsePlayerStats(msg);
				break;
			case 0xA1:
				ParsePlayerSkills(msg);
				break;
			case 0xA2:
				ParsePlayerIcons(msg);
				break;
			case 0xA3:
				ParseCancelAttack(msg);
				break;
			case 0xA4:
				ParseSpellCooldown(msg);
				break;
			case 0xA5:
				ParseSpellGroupCooldown(msg);
				break;
			case 0xAA:
				ParseCreatureSpeak(msg);
				break;
			case 0xAB:
				ParseChannels(msg);
				break;
			case 0xAC:
				ParseOpenChannel(msg);
				break;
			case 0xAD:
				ParseOpenPrivate(msg);
				break;
			case 0xB2:
				ParseCreatePrivateChannel(msg);
				break;
			case 0xB3:
				ParseCloseChannel(msg);
				break;
			case 0xB4:
				ParseTextMessage(msg);
				break;
			case 0xB5:
				ParseCancelWalk(msg);
				break;
			case 0xBE:
				ParseMovePlayerUp(msg);
				break;
			case 0xBF:
				ParseMovePlayerDown(msg);
				break;
			case 0xC8:
				ParseOutfitWindow(msg);
				break;
			case 0xD2:
				ParseVIPList(msg);
				break;
			case 0xD3:
				ParseVIPLogin(msg);
				break;
			case 0xD4:
				ParseVIPLogout(msg);
				break;
			case 0xDC:
				ParseTutorial(msg);
				break;
			case 0xDD:
				ParseMarker(msg);
				break;
			case 0xF0:
				ParseQuestLog(msg);
				break;
			case 0xF1:
				ParseQuestLine(msg);
				break;
			//Extended protocol
			case 0x101:
				ParseStatusPeriods(msg);
				break;
			case 0x102:
				ParseSpellsWindow(msg);
				break;
			case 0x103:
				ParseScreenShot(msg);
				break;
			case 0x105:
                ParseCreatureEquipment(msg);
                break;
			case 0x106:
				ParseSound(msg);
				break;
			case 0x107:
				ParseManageBot(msg);
				break;
			case 0x108:
				ParseDirectionArrow(msg);
				break;
			case 0x109:
				ParsePositionArrow(msg);
				break;
			case 0x1FF:
				ParseScriptPacket(msg);
				break;
			default:
				error = "Unknown packet type! [" + value2str(id, true) + "]";
				Logger::AddLog("Protocol::ParsePacketGame()", error.c_str(), LOG_ERROR, msg);
				break;
		}
	}

	sig.Execute();
	sig.Clear();
}


void Protocol::GetPosition(NetworkMessage* msg, Position& position) {
	position.x = msg->GetU16();
	position.y = msg->GetU16();
	position.z = msg->GetU8();
}

void Protocol::GetThing(NetworkMessage* msg, Thing*& thing, int& skip) {
	unsigned short id = msg->GetU16();
	unsigned int shine = 0;

	if (id == 0x0000 || id == 0x0010) {
		id = msg->GetU16();
		shine = msg->GetU32();
	}

	if (id >= 0xFF00) {
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

		Item* item = new(M_PLACE) Item;
		item->SetID(id);
		item->SetCount(count);
		item->SetShine(shine);
		thing = item;
	}
}

void Protocol::GetMapDescription(NetworkMessage* msg, int x, int y, int width, int height, Position pos, Map* map) {
	map->DeleteMap(x, y, width, height);

	int start_z, stop_z, step_z;
	if (pos.z > 7) {
		start_z = pos.z - 2;
		stop_z = std::min(15, pos.z + 2);
		step_z = 1;
	}
	else {
		start_z = 7;
		stop_z = 0;
		step_z = -1;
	}

	map->SetCorner(Position(pos.x - 8, pos.y - 6, pos.z));

	int skip = 0;
	for (int nz = start_z; nz != stop_z + step_z; nz += step_z)
		GetFloorDescription(msg, x, y, nz, width, height, pos, map, skip);

	sig.PushFunction(boost::bind(&Map::UpdateBattleContainer, map));
	map->WriteMiniMap();
	//map->SortThings(x, y, width, height);
}

void Protocol::GetFloorDescription(NetworkMessage* msg, int x, int y, int z, int width, int height, Position pos, Map* map, int& skip) {
	for (int nx = x; nx < x + width; nx++)
	for (int ny = y; ny < y + height; ny++) {
		Tile* tile = NULL;

		int offset = z - pos.z;
		Position position(pos.x - 8 + nx - offset, pos.y - 6 + ny - offset, z);

		if (skip == 0)
			GetTileDescription(msg, position, tile, skip);
		else
			skip--;

		map->SetTile(nx, ny, z, tile);
	}
}

void Protocol::GetTileDescription(NetworkMessage* msg, Position pos, Tile*& tile, int& skip) {
	unsigned char color = 0;

	Thing* thing = NULL;
	do {
		thing = NULL;
		skip = 0;
		GetThing(msg, thing, skip);

		if (thing) {
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
	} while(thing);

	if (tile)
		tile->SortThings();
}

void Protocol::GetCreature(NetworkMessage* msg, Creature*& creature, bool known) {
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
}

void Protocol::GetOutfit(NetworkMessage* msg, Outfit* outfit) {
	if (!outfit) {
		Logger::AddLog("Protocol::GetOutfit()", "Outfit pointer is NULL!", LOG_ERROR, msg);
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

	outfit->lookMount = 0;
}


void Protocol::StartReceiveLoop() {
	if (Item::version != Game::dataLocation) {
		Windows* wnds = game->GetWindows();
		wnds->OpenWindow(WND_MESSAGE, Text::GetText("ERROR_MESSAGE_4", Game::options.language).c_str());
		return;
	}
	else if (Sprites::version != Game::dataLocation) {
		Windows* wnds = game->GetWindows();
		wnds->OpenWindow(WND_MESSAGE, Text::GetText("ERROR_MESSAGE_5", Game::options.language).c_str());
		return;
	}

	NetworkMessage* msg = new(M_PLACE) NetworkMessage(GetVersion(), XTEAkey);

	THREAD receiveLoop(&Protocol::ReceiveLoop, this, msg);
	receiveLoop.swap(this->receiveLoop);
}

void Protocol::ReceiveLoop(NetworkMessage* msg) {
	Logger::AddLog("Protocol::ReceiveLoop()", "Receive loop has been started!", LOG_INFO);

	bool endLoop = false;
	while(!endLoop) {
		msg->Reset();
		msg->ReceiveMessage(&client);

		endLoop = !client.Connected();
		if (!endLoop) {
			if (game->GetGameState() >= GAME_LOGGEDTOSERVER) ParsePacketGame(msg);
			else if (game->GetGameState() <= GAME_INQUEUE) ParsePacketLogin(msg);
		}
	}

	Logger::AddLog("Protocol::ReceiveLoop()", "Receive loop has been stoped!", LOG_INFO);

	onReceiveLoopEnd();

	delete_debug(msg, M_PLACE);
}

void Protocol::onReceiveLoopEnd() {
	LOCKCLASS lockClass(game->lockGame);
	if (game->GetGameState() != GAME_LOGGEDTOGAME)
		return;

	Windows* wnds = game->GetWindows();

	Bot::enabled = true;

#ifndef NO_SOUND
	SFX_System* sfx = game->GetSFX();
	if (sfx)
		sfx->StopBackgroundSound();
#endif

	game->SaveChannels();
	game->SaveHotKeys();
	game->SaveMarkers();
	game->SaveWaypoints();

	Bot* bot = game->GetBot();
	if (bot) {
		bot->SaveBot(Game::filesLocation + "/bot/");
		THREAD thread(&Bot::Stop);
	}

	if (wnds) {
		wnds->SaveGUIWindows(true);
		wnds->CloseWindows(WND_ALL, true);

		GUIManager* guiManager = wnds->GetGUIManager();
		guiManager->ClearWindows();
		guiManager->ClearActionsOnResizeMainWindow();
		guiManager->CleanUp();

		Window* wnd = wnds->OpenWindow(WND_OPTIONS, game);
		wnd->SetMinimizeAbility(false);

		wnds->OpenWindow(WND_STARTUP, game);
		wnds->OpenWindow(WND_LOGIN, game);
		wnds->OpenWindow(WND_CHARACTERSLIST, game);
	}
	else
		Logger::AddLog("Protocol::ReceiveLoop()", "Pointer to windows is NULL!", LOG_WARNING);

	game->SetWindowGame(NULL);
	game->SetWindowConsole(NULL);
	Channel::ClearChannels();
	Messages::ClearMessages();

	Map* map = game->GetMap();
	if (map) {
		map->DeleteMap();
		map->CloseMiniMapFile();
		map->UpdateBattle();
	}
	else
		Logger::AddLog("Protocol::ReceiveLoop()", "Pointer to map is NULL!", LOG_WARNING);

	Creature::DeleteKnown();

	game->SetGameState(GAME_LOGGEDTOSERVER);
	Game::releaseSprites = true;
}


//Lua functions

int Protocol::LuaGetVersion(lua_State* L) {
	Game* game = Game::game;
	Protocol* protocol = (game ? game->GetProtocol() : NULL);

	int version = (protocol ? protocol->GetVersion() : 0);

	LuaScript::PushNumber(L, version);
	return 1;
}

int Protocol::LuaNewNetworkMessage(lua_State* L) {
	Game* game = Game::game;
	Protocol* protocol = (game ? game->GetProtocol() : NULL);

	NetworkMessage* msg = NULL;
	if (protocol)
		msg = new(M_PLACE) NetworkMessage(protocol->GetVersion(), protocol->XTEAkey);

	LuaScript::PushNumber(L, (unsigned long)msg);
	return 1;
}

int Protocol::LuaDeleteNetworkMessage(lua_State* L) {
	NetworkMessage* msg = (NetworkMessage*)((unsigned long)LuaScript::PopNumber(L));

	if (msg)
		delete_debug(msg, M_PLACE);

	return 1;
}

int Protocol::LuaClearNetworkMessage(lua_State* L) {
	NetworkMessage* msg = (NetworkMessage*)((unsigned long)LuaScript::PopNumber(L));

	if (msg)
		msg->Reset();

	return 1;
}

int Protocol::LuaGetNetworkMessageSize(lua_State* L) {
	NetworkMessage* msg = (NetworkMessage*)((unsigned long)LuaScript::PopNumber(L));

	size_t size = 0;
	if (msg)
		size = msg->GetSize();

	LuaScript::PushNumber(L, size);
	return 1;
}

int Protocol::LuaSendNetworkMessage(lua_State* L) {
	NetworkMessage* msg = (NetworkMessage*)((unsigned long)LuaScript::PopNumber(L));

	Game* game = Game::game;
	Protocol* protocol = (game ? game->GetProtocol() : NULL);
	if (protocol && protocol->client.Connected() && msg)
		msg->SendMessage(&protocol->client);

	return 1;
}

int Protocol::LuaGetNetworkMessageData(lua_State* L) {
	std::string type = LuaScript::PopString(L);
	NetworkMessage* msg = (NetworkMessage*)((unsigned long)LuaScript::PopNumber(L));

	if (!msg) {
		lua_pushnil(L);
		return 1;
	}

	if (type == "uint8")
		LuaScript::PushNumber(L, msg->GetU8());
	else if (type == "uint16")
		LuaScript::PushNumber(L, msg->GetU16());
	else if (type == "uint32")
		LuaScript::PushNumber(L, msg->GetU32());
	else if (type == "uint64")
		LuaScript::PushNumber(L, msg->GetU64());
	else if (type == "string")
		LuaScript::PushString(L, msg->GetString());
	else
		lua_pushnil(L);

	return 1;
}

int Protocol::LuaAddNetworkMessageData(lua_State* L) {
	double valueNum = 0;
	std::string valueStr = "";
	if (lua_type(L, -1) == LUA_TSTRING)
		valueStr = LuaScript::PopString(L);
	else
		valueNum = LuaScript::PopNumber(L);
	std::string type = LuaScript::PopString(L);
	NetworkMessage* msg = (NetworkMessage*)((unsigned long)LuaScript::PopNumber(L));

	if (!msg)
		return 1;

	if (type == "uint8")
		msg->AddU8(valueNum);
	else if (type == "uint16")
		msg->AddU16(valueNum);
	else if (type == "uint32")
		msg->AddU32(valueNum);
	else if (type == "uint64")
		msg->AddU64(valueNum);
	else if (type == "string")
		msg->AddString(valueStr);

	return 1;
}

int Protocol::LuaEndNetworkMessage(lua_State* L) {
	NetworkMessage* msg = (NetworkMessage*)((unsigned long)LuaScript::PopNumber(L));

	bool endMsg = false;
	if (msg)
		endMsg = msg->EndOfPacket();

	LuaScript::PushNumber(L, (int)endMsg);
	return 1;
}


void Protocol::LuaRegisterFunctions(lua_State* L) {
	//getVersion()
	lua_register(L, "getVersion", Protocol::LuaGetVersion);

	//newNetworkMessage() : netMsgPtr
	lua_register(L, "newNetworkMessage", Protocol::LuaNewNetworkMessage);

	//deleteNetworkMessage(netMsgPtr)
	lua_register(L, "deleteNetworkMessage", Protocol::LuaDeleteNetworkMessage);

	//clearNetworkMessage(netMsgPtr)
	lua_register(L, "clearNetworkMessage", Protocol::LuaClearNetworkMessage);

	//getNetworkMessageSize(netMsgPtr) : size
	lua_register(L, "getNetworkMessageSize", Protocol::LuaGetNetworkMessageSize);

	//sendNetworkMessage(netMsgPtr)
	lua_register(L, "sendNetworkMessage", Protocol::LuaSendNetworkMessage);

	//getNetworkMessageData(netMsgPtr, type) : valueNum - type: uint8, uint16, uint32, uint64
	//getNetworkMessageData(netMsgPtr, type) : valueStr - type: string
	lua_register(L, "getNetworkMessageData", Protocol::LuaGetNetworkMessageData);

	//addNetworkMessageData(netMsgPtr, type, data) - type: uint8, uint16, uint32, uint64, string
	lua_register(L, "addNetworkMessageData", Protocol::LuaAddNetworkMessageData);

	//endNetworkMessage(netMsgPtr)
	lua_register(L, "endNetworkMessage", Protocol::LuaEndNetworkMessage);
}
