/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#include "protocol842.h"

#include "allocator.h"
#include "position.h"
#include "player.h"
#include "text.h"
#include "tools.h"
#include "window.h"


// ---- Protocol ---- //

Protocol842::Protocol842(Game* game) : Protocol840(game) { }

Protocol842::~Protocol842() {
	if (client.Connected())
		client.doClose();

	if (receiveLoop.joinable())
        receiveLoop.join();
}


unsigned short Protocol842::GetVersion() {
	return 842;
}


bool Protocol842::LoginGame() {
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

	NetworkMessage nmsg(GetVersion());
	nmsg.ReceiveMessage(&client);

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


void Protocol842::ParseAddThing(NetworkMessage* msg) {
	Position pos;
	GetPosition(msg, pos);
	unsigned char stackPos = msg->GetU8();

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
			if (stackPos != 255)
				tile->InsertThing(thing, stackPos);
			else
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
