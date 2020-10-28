/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#include "map.h"

#include "allocator.h"
#include "ad2d.h"
#include "channel.h"
#include "const.h"
#include "distance.h"
#include "game.h"
#include "item.h"
#include "luascript.h"
#include "magiceffect.h"
#include "player.h"
#include "tools.h"


// ---- Tile ---- //

MUTEX Tile::lockTile;


Tile::Tile() {
	horizontal = false;
	vertical = false;
	pos = Position(0, 0, 0);
}

Tile::~Tile() {
	std::list<Thing*>::iterator it = things.begin();
	for (it; it != things.end(); it++) {
		Thing* thing = (*it);

		Creature* creature = dynamic_cast<Creature*>(thing);

		if (!creature || !Creature::GetFromKnown(creature->GetID()))
			delete_debug(thing, M_PLACE);
		else {
			creature->SetLight(0, 0);
			creature->pos = Position(0, 0, 0);
		}
	}
	things.clear();
}

void Tile::AddThing(Thing* thing) {
	LOCKCLASS lockClass(lockTile);

	Item* item = dynamic_cast<Item*>(thing);
	Creature* creature = dynamic_cast<Creature*>(thing);

	if (!item && !creature) {
		Logger::AddLog("Tile::AddThing()", "Neither item nor creature!", LOG_WARNING);
		return;
	}

	std::list<Thing*>::iterator it = std::find(things.begin(), things.end(), thing);
	if (it == things.end()) {
		int order;

		if (item && item->IsGround()) order = 0;
		else if (item && item->GetTopOrder()) order = item->GetTopOrder();
		else if (creature) order = 4;
		else order = 5;

		unsigned char stackPos = 0;
		for (it = things.begin(); it != things.end(); it++, stackPos++) {
			Item* Titem = dynamic_cast<Item*>(*it);
			Creature* Tcreature = dynamic_cast<Creature*>(*it);

			if (!Titem && !Tcreature)
				continue;

			int Torder;

			if (Titem && Titem->IsGround()) Torder = 0;
			else if (Titem && Titem->GetTopOrder()) Torder = Titem->GetTopOrder();
			else if (Tcreature) Torder = 4;
			else Torder = 5;

			if (order == 0 && Torder >= 0) {
				things.insert(it, thing);
				thing->pos = this->pos;

				return;
			}
			else if (order == 1 && Torder > 1) {
				things.insert(it, thing);
				thing->pos = this->pos;

				return;
			}
			else if (order == 2 && Torder >= 2) {
				things.insert(it, thing);
				thing->pos = this->pos;

				return;
			}
			else if (order == 3 && Torder >= 3) {
				things.insert(it, thing);
				thing->pos = this->pos;

				return;
			}
			else if (order == 4 && ((Game::options.protocol > 842 && Torder > 4) || (Game::options.protocol <= 842 && Torder >= 4))) {
				things.insert(it, thing);
				thing->pos = this->pos;

				return;
			}
			else if (order == 5 && Torder >= 5) {
				things.insert(it, thing);
				thing->pos = this->pos;

				return;
			}
		}
		things.insert(it, thing);
		thing->pos = this->pos;

		return;
	}
	else
		Logger::AddLog("Tile::AddThing()", "Thing duplicate!", LOG_WARNING);
}

void Tile::InsertThing(Thing* thing, unsigned char stackPos) {
	LOCKCLASS lockClass(lockTile);

	if (!thing) {
		Logger::AddLog("Tile::InsertThing()", "Thing is NULL!", LOG_WARNING);
		return;
	}

	std::list<Thing*>::iterator it = std::find(things.begin(), things.end(), thing);
	if (it == things.end()) {
		if (stackPos != 0xFF) {
			it = things.begin();
			for (it; it != things.end(); it++, stackPos--) {
				if (stackPos == 0)
					break;
			}
		}

		things.insert(it, thing);
		thing->pos = this->pos;
	}
	else
		Logger::AddLog("Tile::InsertThing()", "Thing duplicate!", LOG_WARNING);
}

void Tile::RemoveThing(Thing* thing) {
	LOCKCLASS lockClass(lockTile);

	std::list<Thing*>::iterator it = std::find(things.begin(), things.end(), thing);
	if (it != things.end()) {
		things.erase(it);
		thing->oldPos = thing->pos;
		thing->pos = Position(0, 0, 0);
	}
	else
		Logger::AddLog("Tile::RemoveThing()", "Thing does not exist!", LOG_WARNING);
}

void Tile::TransformThing(Thing* thing, unsigned char stackPos) {
	LOCKCLASS lockClass(lockTile);

	std::list<Thing*>::iterator it = things.begin();
	for (it; it != things.end(); it++, stackPos--) {
		if (stackPos == 0) {
			Thing* oldThing = *it;

			thing->newPos = oldThing->newPos;
			thing->pos = oldThing->pos;
			thing->oldPos = oldThing->oldPos;

			delete_debug(oldThing, M_PLACE);
			*it = thing;

			return;
		}
	}

	Logger::AddLog("Tile::TransformThing()", "Wrong stackPos, thing not found!", LOG_WARNING);
}

void Tile::AddTempCreature(Creature* creature) {
	LOCKCLASS lockClass(lockTile);

	std::list<Creature*>::iterator it = std::find(tempCreatures.begin(), tempCreatures.end(), creature);
	if (it == tempCreatures.end())
		tempCreatures.push_back(creature);
}

void Tile::RemoveTempCreature(Creature* creature) {
	LOCKCLASS lockClass(lockTile);

	std::list<Creature*>::iterator it = std::find(tempCreatures.begin(), tempCreatures.end(), creature);
	if (it != tempCreatures.end())
		tempCreatures.erase(it);
}

unsigned char Tile::GetSize() {
	LOCKCLASS lockClass(lockTile);

	return things.size();
}

unsigned char Tile::GetDownSize() {
	LOCKCLASS lockClass(lockTile);

	return downThings.size();
}

unsigned char Tile::GetCreaturesSize() {
	LOCKCLASS lockClass(lockTile);

	return creatureThings.size();
}

unsigned char Tile::GetTopSize() {
	LOCKCLASS lockClass(lockTile);

	return topThings.size();
}

unsigned char Tile::GetTempCreaturesSize() {
	LOCKCLASS lockClass(lockTile);

	return tempCreatures.size();
}

int Tile::GetDownHeight() {
	LOCKCLASS lockClass(lockTile);

	int height = 0;

	std::list<Thing*>::iterator it = downThings.begin();
	for (it; it != downThings.end(); it++) {
		Thing* thing = (*it);
		Item* item = dynamic_cast<Item*>(thing);
		if (item) {
			if ((*item)() && (*item)()->hasHeight)
				height += (*item)()->height;
		}
	}

	return height;
}

unsigned char Tile::GetThingStackPos(Thing* thing) {
	LOCKCLASS lockClass(lockTile);

	unsigned char stackPos = 0;

	std::list<Thing*>::iterator it = things.begin();
	for (it; it != things.end(); it++, stackPos++) {
		if (*it == thing)
			return stackPos;
	}

	return stackPos;
}

Thing* Tile::GetThingByStackPos(unsigned char stackPos) {
	LOCKCLASS lockClass(lockTile);

	std::list<Thing*>::iterator it = things.begin();
	for (it; it != things.end(); it++, stackPos--) {
		if (stackPos == 0)
			return *it;
	}

	std::stringstream ss;
	ss << "Wrong stackPos, thing not found! [" << pos.x << "/" << pos.y << "/" << (int)pos.z << ":" << (int)stackPos << "]";
	Logger::AddLog("Tile::GetThingByStackPos()", ss.str().c_str(), LOG_WARNING);
	return NULL;
}

Thing* Tile::GetDownThingByStackPos(unsigned char stackPos) {
	LOCKCLASS lockClass(lockTile);

	std::list<Thing*>::iterator it = downThings.begin();
	for (it; it != downThings.end(); it++, stackPos--) {
		if (stackPos == 0)
			return *it;
	}

	Logger::AddLog("Tile::GetDownThingByStackPos()", "Wrong stackPos, thing not found!", LOG_WARNING);
	return NULL;
}

Thing* Tile::GetCreatureThingByStackPos(unsigned char stackPos) {
	LOCKCLASS lockClass(lockTile);

	std::list<Thing*>::iterator it = creatureThings.begin();
	for (it; it != downThings.end(); it++, stackPos--) {
		if (stackPos == 0)
			return *it;
	}

	Logger::AddLog("Tile::GetCreatureThingByStackPos()", "Wrong stackPos, thing not found!", LOG_WARNING);
	return NULL;
}

Thing* Tile::GetTopThingByStackPos(unsigned char stackPos) {
	LOCKCLASS lockClass(lockTile);

	std::list<Thing*>::iterator it = topThings.begin();
	for (it; it != topThings.end(); it++, stackPos--) {
		if (stackPos == 0)
			return *it;
	}

	Logger::AddLog("Tile::GetTopThingByStackPos()", "Wrong stackPos, thing not found!", LOG_WARNING);
	return NULL;
}

Creature* Tile::GetTempCreatureByStackPos(unsigned char stackPos) {
	LOCKCLASS lockClass(lockTile);

	std::list<Creature*>::iterator it = tempCreatures.begin();
	for (it; it != tempCreatures.end(); it++, stackPos--) {
		if (stackPos == 0)
			return *it;
	}

	Logger::AddLog("Tile::GetTempCreatureByStackPos()", "Wrong stackPos, creature not found!", LOG_WARNING);
	return NULL;
}

Thing* Tile::GetGroundThing() {
	Thing* thing = GetThingByStackPos(0);//(!downThings.empty() ? *downThings.begin() : NULL);
	Item* item = dynamic_cast<Item*>(thing);
	if (item && item->IsGround())
		return thing;

	return NULL;
}

Thing* Tile::GetTopDownThing() {
	LOCKCLASS lockClass(lockTile);

	for (int stackPos = this->GetDownSize() - 1; stackPos >= 0 ; stackPos--) {
		Thing* thing = this->GetDownThingByStackPos(stackPos);
		if (thing)
			return thing;
	}

	return NULL;
}

Thing* Tile::GetTopCreatureThing(bool tempCreature) {
	LOCKCLASS lockClass(lockTile);

	for (int stackPos = this->GetCreaturesSize() - 1; stackPos >= 0 ; stackPos--) {
		Thing* thing = this->GetCreatureThingByStackPos(stackPos);
		if (thing)
			return thing;
	}

	if (tempCreature) {
		for (int stackPos = this->GetTempCreaturesSize() - 1; stackPos >= 0; stackPos--) {
			Creature* creature = this->GetTempCreatureByStackPos(stackPos);
			if (creature && creature->GetStep() != 0.0f)
				return creature;
		}
	}

	return NULL;
}

Thing* Tile::GetTopTopThing() {
	LOCKCLASS lockClass(lockTile);

	for (int stackPos = this->GetTopSize() - 1; stackPos >= 0 ; stackPos--) {
		Thing* thing = this->GetTopThingByStackPos(stackPos);
		if (thing)
			return thing;
	}

	return NULL;
}

Thing* Tile::GetMoveableThing() {
	LOCKCLASS lockClass(lockTile);

	Thing* moveThing = NULL;
	for (int stackPos = 0; stackPos < this->GetDownSize(); stackPos++) {
		Thing* thing = this->GetDownThingByStackPos(stackPos);
		Item* item = dynamic_cast<Item*>(thing);
		if (item) {
			if ((*item)() && !(*item)()->notMoveable)
				moveThing = thing;
		}
	}

	if (!moveThing) {
		for (int stackPos = 0; stackPos < this->GetCreaturesSize(); stackPos++) {
			Thing* thing = this->GetCreatureThingByStackPos(stackPos);
			Creature* creature = dynamic_cast<Creature*>(thing);
			if (creature) {
				moveThing = thing;
				break;
			}
		}
	}

	return moveThing;
}

Thing* Tile::GetUseableThing(bool useWith) {
	LOCKCLASS lockClass(lockTile);

	Thing* useThing = NULL;
	Thing* useAction = NULL;
	Thing* useLadder = NULL;
	Thing* useTopItem = NULL;
	Thing* useGround = NULL;
	for (int stackPos = 0; stackPos < this->GetSize(); stackPos++) {
		Thing* thing = this->GetThingByStackPos(stackPos);
		Item* item = dynamic_cast<Item*>(thing);
		if (item && (*item)()) {
		    if ((*item)()->action)
                useAction = thing;
			if ((*item)()->ground && !useGround)
				useGround = thing;
			if ((*item)()->ladder)
				useLadder = thing;
			if ((item->GetTopOrder() == 1 || item->GetTopOrder() == 2 || item->GetTopOrder() == 3)/* && !useTopItem*/)
				useTopItem = thing;
			if (!(*item)()->ground && item->GetTopOrder() == 0) {
				useThing = thing;
				break;
			}
		}
	}
	if (useWith && useAction)
        useThing = useAction;
	else if (useLadder)
		useThing = useLadder;
	else if (!useThing && useTopItem)
		useThing = useTopItem;
	else if (!useThing && useGround)
		useThing = useGround;

	return useThing;
}

unsigned short Tile::GetSpeed() {
	LOCKCLASS lockClass(lockTile);

	Thing* thing = GetThingByStackPos(0);
	Item* item = dynamic_cast<Item*>(thing);
	if (item && item->IsGround()) {
		if ((*item)())
			return (*item)()->speed;
	}

	return 220;
}

unsigned char Tile::GetMiniMapColor() {
	LOCKCLASS lockClass(lockTile);

	unsigned char color = 0;

	for (int stackPos = 0; stackPos < this->GetSize(); stackPos++) {
		Thing* thing = GetThingByStackPos(stackPos);
		Item* item = dynamic_cast<Item*>(thing);
		if (item && (*item)() && (*item)()->minimap)
			color = (*item)()->minimapColor;
	}

	return color;
}

bool Tile::IsBlocking(bool ignoreCreatures, bool ignorePathBlock) {
	LOCKCLASS lockClass(lockTile);

	for (int stackPos = 0; stackPos < this->GetSize(); stackPos++) {
		Thing* thing = GetThingByStackPos(stackPos);
		Item* item = dynamic_cast<Item*>(thing);
		Creature* creature = dynamic_cast<Creature*>(thing);
		if (item && (*item)()) {
		 	if ((*item)()->blocking)
				return true;
			if (!ignorePathBlock && (*item)()->blockPathFind)
				return true;
		}
		else if (!ignoreCreatures && creature && creature->GetBlocking())
			return true;
	}

	return false;
}

bool Tile::IsHorizontal() {
	return horizontal;
}

bool Tile::IsVertical() {
	return vertical;
}

void Tile::SortThings() {
	LOCKCLASS lockClass(lockTile);

	downThings.clear();
	creatureThings.clear();
	topThings.clear();

	horizontal = false;
	vertical = false;

	std::list<Thing*>::iterator it = things.begin();
	for (it; it != things.end(); it++) {
		int order;

		Thing* thing = *it;
		Item* item = dynamic_cast<Item*>(thing);
		Creature* creature = dynamic_cast<Creature*>(thing);

		if (item && (*item)()) {
			if ((*item)()->horizontal)
				horizontal = true;
			if ((*item)()->vertical)
				vertical = true;
		}

		if (item && item->IsGround()) order = 0;
		else if (item && item->GetTopOrder() == 1) order = 1;
		else if (item && item->GetTopOrder() == 2) order = 2;
		else if (item && item->GetTopOrder() == 0) order = 3;
		else if (creature) order = 4;
		else order = 5;

		std::list<Thing*>::iterator Tit_begin;
		std::list<Thing*>::iterator Tit_end;
		if (order <= 3) {
			Tit_begin = downThings.begin();
			Tit_end = downThings.end();
		}
		else if (order == 4) {
			Tit_begin = creatureThings.begin();
			Tit_end = creatureThings.end();
		}
		else if (order > 4) {
			Tit_begin = topThings.begin();
			Tit_end = topThings.end();
		}

		std::list<Thing*>::iterator Tit = Tit_begin;
		for (Tit; Tit != Tit_end; Tit++) {
			int Torder;

			Thing* Tthing = *Tit;
			Item* Titem = dynamic_cast<Item*>(Tthing);
			Creature* Tcreature = dynamic_cast<Creature*>(Tthing);

			if (Titem && Titem->IsGround()) Torder = 0;
			else if (Titem && Titem->GetTopOrder() == 1) Torder = 1;
			else if (Titem && Titem->GetTopOrder() == 2) Torder = 2;
			else if (Titem && Titem->GetTopOrder() == 0) Torder = 3;
			else if (Tcreature) Torder = 4;
			else Torder = 5;

			if (order == 0 && Torder > 0) {
				downThings.insert(Tit, thing);
				break;
			}
			else if (order == 1 && Torder > 1) {
				downThings.insert(Tit, thing);
				break;
			}
			else if (order == 2 && Torder > 2) {
				downThings.insert(Tit, thing);
				break;
			}
			else if (order == 3 && Torder >= 3) {
				downThings.insert(Tit, thing);
				break;
			}
			else if (order == 4 && ((Game::options.protocol > 822 && Torder > 4) || (Game::options.protocol <= 822 && Torder >= 4))) {
				creatureThings.insert(Tit, thing);
				break;
			}
			else if (order == 5 && Torder >= 5) {
				topThings.insert(Tit, thing);
				break;
			}
		}
		if (Tit == Tit_end) {
			if (order <= 3)
				downThings.insert(Tit, thing);
			else if (order == 4)
				creatureThings.insert(Tit, thing);
			else if (order > 4)
				topThings.insert(Tit, thing);
		}
	}
}


// ---- Map ---- //

MUTEX	Map::lockMap;

AD2D_Image	Map::mapImage;

PositionArrowsSet		Map::posArrows;
std::list<Creature*>	Map::creatures;

bool	Map::mapUpdated = false;


Map::Map() {
	ClearMap();

	unsigned char data[512 * 512 * 4];
	memset(data, 0, 512 * 512 * 4);
	if (AD2D_Window::_mode == API_OPENGL)
		mapImage.Create(512, 512, data);
	else if (AD2D_Window::_mode == API_DIRECT3D) {
		AD2D_Window* gfx = AD2D_Window::_window;
		LPDIRECT3DDEVICE9 D3DDev = (gfx ? gfx->GetD3DDEV() : NULL);

		LPDIRECT3DTEXTURE9 texture;
		D3DXCreateTexture(D3DDev, 512, 512, 1, D3DUSAGE_RENDERTARGET, D3DFMT_A8B8G8R8, D3DPOOL_DEFAULT, &texture);

		mapImage.Create(512, 512, texture);
		mapImage.textureD3D->GetSurfaceLevel(0, &mapImage.surfaceD3D);
	}

	mapImage.AdjustSize(480, 352);
}

Map::~Map() { }

void Map::SetMiniMap(MiniMap* minimap) {
	LOCKCLASS lockClass(lockMap);

	this->minimap = minimap;
}

MiniMap* Map::GetMiniMap() {
	LOCKCLASS lockClass(lockMap);

	return minimap;
}

void Map::SetBattle(Battle* battle) {
	LOCKCLASS lockClass(lockMap);

	this->battle = battle;
}

Battle* Map::GetBattle() {
	LOCKCLASS lockClass(lockMap);

	return battle;
}

void Map::ChangeMiniMapZoom(float multiplier) {
	LOCKCLASS lockClass(lockMap);

	if (minimap)
		minimap->ChangeZoom(multiplier);
}

void Map::ResetMiniMapZoom() {
	LOCKCLASS lockClass(lockMap);

	if (minimap)
		minimap->ResetZoom();
}

void Map::MoveMiniMapLevel(char step) {
	LOCKCLASS lockClass(lockMap);

	if (minimap) {
		if (corner.z + step < 0 || corner.z + step > 15)
			step = 0;

		minimap->MoveLevel(step);
		minimap->FileGetMap(corner, 0, 0, 256, 256);
	}
}

void Map::ResetMiniMapLevel() {
	LOCKCLASS lockClass(lockMap);

	if (minimap) {
		minimap->FileFlush();
		minimap->ResetLevel();
		minimap->FileGetMap(corner, 0, 0, 256, 256);
	}
}

void Map::SetMiniMapPix(Position tilePos, Tile* tile) {
	LOCKCLASS lockClass(lockMap);

	if (minimap && tilePos.z == corner.z) {
		unsigned char color = 0;
		unsigned short speed = 0;
		if (tile) {
			color = tile->GetMiniMapColor();
			if (!tile->IsBlocking())
				speed = tile->GetSpeed();
		}

		minimap->SetPix(corner, tilePos, color, speed);
	}
}

void Map::UpdateMiniMap() {
	LOCKCLASS lockClass(lockMap);

	for (int x = 0; x < 18; x++)
	for (int y = 0; y < 14; y++) {
		Tile* tile = tiles[x][y][corner.z];
		SetMiniMapPix(Position(corner.x + x, corner.y + y, corner.z), tile);
	}
}

void Map::ReadMiniMap(int x, int y, int w, int h) {
	LOCKCLASS lockClass(lockMap);

	if (minimap)
		minimap->FileGetMap(corner, x, y, w, h);
}

void Map::WriteMiniMap() {
	LOCKCLASS lockClass(lockMap);

	if (minimap)
		minimap->FileFlush();
}

void Map::CloseMiniMapFile() {
	LOCKCLASS lockClass(lockMap);

	if (minimap)
		minimap->CloseFile();
}

void Map::AddBattle(Tile* tile) {
	LOCKCLASS lockClass1(lockMap);
	LOCKCLASS lockClass2(Tile::lockTile);

	for (int stackPos = 0; stackPos < tile->GetSize(); stackPos++) {
		Thing* thing = tile->GetThingByStackPos(stackPos);
		Creature* creature = dynamic_cast<Creature*>(thing);

		if (creature && battle)
			battle->AddCreature(creature);
	}
}

void Map::RemoveBattle(Tile* tile) {
	LOCKCLASS lockClass1(lockMap);
	LOCKCLASS lockClass2(Tile::lockTile);

	for (int stackPos = 0; stackPos < tile->GetSize(); stackPos++) {
		Thing* thing = tile->GetThingByStackPos(stackPos);
		Creature* creature = dynamic_cast<Creature*>(thing);

		if (creature && battle)
			battle->RemoveCreature(creature);
	}
}

unsigned int Map::GetBattleID(int number) {
	LOCKCLASS lockClass(lockMap);

	if (battle) {
		Creature* creature = battle->GetCreature(number);
		if (creature)
			return creature->GetID();
	}

	return 0;
}

void Map::ClearBattle() {
	LOCKCLASS lockClass(lockMap);

	if (battle)
		battle->ClearBattle();
}

void Map::UpdateBattle() {
	LOCKCLASS lockClass1(Windows::lockWindows);
	LOCKCLASS lockClass2(lockMap);

	if (battle) {
	    battle->ClearBattle();

	    for (int x = 0; x < 18; x++)
        for (int y = 0; y < 14; y++) {
            Tile* tile = tiles[x][y][corner.z];
            if (tile) {
                for (int stackPos = 0; stackPos < tile->GetCreaturesSize(); stackPos++) {
                    Thing* thing = tile->GetCreatureThingByStackPos(stackPos);
                    Creature* creature = dynamic_cast<Creature*>(thing);
                    if (creature)
                        battle->AddCreature(creature);
                }
            }
        }

		battle->UpdateContainer();
	}
}

void Map::UpdateBattleContainer() {
	LOCKCLASS lockClass1(Windows::lockWindows);
	LOCKCLASS lockClass2(lockMap);

	if (battle)
		battle->UpdateContainer();
}

void Map::SetCorner(Position corner) {
	LOCKCLASS lockClass(lockMap);

	this->corner = corner;
}

Position Map::GetCorner() {
	LOCKCLASS lockClass(lockMap);

	return this->corner;
}

void Map::ClearMap(int sx, int sy, int width, int height, int zFrom, int zTo) {
	LOCKCLASS lockClass(lockMap);

	for (int x = sx; x < sx + width; x++)
	for (int y = sy; y < sy + height; y++)
	for (int z = zFrom; z <= zTo; z++)
		tiles[x][y][z] = NULL;
}

void Map::DeleteMap(int sx, int sy, int width, int height, int zFrom, int zTo) {
	LOCKCLASS lockClass(lockMap);

	for (int x = sx; x < sx + width; x++)
	for (int y = sy; y < sy + height; y++)
	for (int z = zFrom; z <= zTo; z++) {
		if (tiles[x][y][z]) {
			if (corner.z == z)
				RemoveBattle(tiles[x][y][z]);
			delete_debug(tiles[x][y][z], M_PLACE);
		}
		tiles[x][y][z] = NULL;
	}
}

void Map::MoveMap(int offsetX, int offsetY) {
	LOCKCLASS lockClass(lockMap);

	Tile* temp[18][14][16];
	memcpy(temp, tiles, sizeof(Tile*) * 18 * 14 * 16);

	for (int x = 0; x < 18; x++)
	for (int y = 0; y < 14; y++)
	for (int z = 0; z < 16; z++) {
		int tx = x - offsetX;
		int ty = y - offsetY;

		if (tx < 0) tx += 18;
		if (tx >= 18) tx -= 18;
		if (ty < 0) ty += 14;
		if (ty >= 14) ty -= 14;

		tiles[x][y][z] = temp[tx][ty][z];
	}

	corner.x -= offsetX;
	corner.y -= offsetY;

	if (minimap)
		minimap->MoveMiniMap(offsetX, offsetY);
}

void Map::SetTile(int x, int y, int z, Tile* tile) {
	LOCKCLASS lockClass(lockMap);

	tiles[x][y][z] = tile;

	if (tile && corner.z == z)
		AddBattle(tile);
	SetMiniMapPix(Position(corner.x + x, corner.y + y, z), tile);
}

void Map::SetTile(Position pos, Tile* tile) {
	LOCKCLASS lockClass(lockMap);

	int offset = corner.z - pos.z;
	int x = pos.x - corner.x - offset;
	int y = pos.y - corner.y - offset;
	int z = pos.z;

	if (x < 0 || x >= 18 || y < 0 || y >= 14 || z < 0 || z >= 16)
		return;

	tiles[x][y][z] = tile;

	if (tile && corner.z == z)
		AddBattle(tile);
	SetMiniMapPix(pos, tile);
}

Tile* Map::GetTile(int x, int y, int z) {
	LOCKCLASS lockClass(lockMap);

	if (x < 0 || x >= 18 || y < 0 || y >= 14 || z < 0 || z >= 16)
		return NULL;

	Tile* tile = tiles[x][y][z];

	return tile;
}

Tile* Map::GetTile(Position pos) {
	LOCKCLASS lockClass(lockMap);

	int offset = corner.z - pos.z;
	int x = pos.x - corner.x - offset;
	int y = pos.y - corner.y - offset;
	int z = pos.z;

	if (x < 0 || x >= 18 || y < 0 || y >= 14 || z < 0 || z >= 16)
		return NULL;

	Tile* tile = tiles[x][y][z];

	return tile;
}


void Map::SetDirectionPosition(Position pos) {
	Game* game = Game::game;
	Player* player = (game ? game->GetPlayer() : NULL);

	if (player)
		player->SetDirectionPos(pos);
}

void Map::AddPositionArrow(Position pos) {
	LOCKCLASS lockClass(lockMap);

	posArrows.insert(pos);
}

void Map::RemovePositionArrow(Position pos) {
	LOCKCLASS lockClass(lockMap);

	PositionArrowsSet::iterator it = posArrows.find(pos);
	if (it != posArrows.end())
		posArrows.erase(it);
}

void Map::ClearPositionArrows() {
	LOCKCLASS lockClass(lockMap);

	posArrows.clear();
}


void Map::ToggleZ() {
	fullZ = !fullZ;
}

void Map::CalculateZ(Position pos) {
	if (!Game::options.printOneLevel) {
		unsigned char z_from = (pos.z <= 7 ? 7 : (pos.z + 2 < 16 ? pos.z + 2 : 15));
		unsigned char z_to = pos.z;

		int tx = pos.x - corner.x;
		int ty = pos.y - corner.y;
		for (int z = z_to - 1; z >= 0; z--) {
			bool grounded = false;
			for (int x = tx - 1; x <= tx; x++)
			for (int y = ty - 1; y <= ty; y++) {
				Tile* tile = GetTile(x, y, z);
				if (tile && tile->GetSize() > 0)
					grounded = true;
			}

			if (!grounded)
				z_to = z;
			else
				break;
		}

		fromZ = z_from;
		toZ = z_to;
	}
	else {
		fromZ = pos.z;
		toZ = pos.z;
	}
}

POINT Map::GetZ() {
	POINT ret = { fromZ, toZ };
	return ret;
}

Tile* Map::GetTopTile(int x, int y, int z_from, int z_to) {
	LOCKCLASS lockClass(lockMap);

	for (int z = z_to; z <= z_from; z++) {
		Tile* tile = GetTile(x, y, z);
		if (tile && tile->GetGroundThing())
			return tile;
	}

	return NULL;
}

bool Map::IsHiddenTile(int x, int y, int z, int z_to) {
	LOCKCLASS lockClass(lockMap);

	int z_from = z;
	for (z = z_from - 1; z >= z_to; z--) {
		Tile* tile = GetTile(x, y, z);
		if (tile && tile->GetGroundThing())
			return true;
	}

	return false;
}

void Map::AddTempCreature(Position pos, Creature* creature) {
	LOCKCLASS lockClass(lockMap);

	Tile* tile = GetTile(pos);
	if (tile)
		tile->AddTempCreature(creature);
}

void Map::RemoveTempCreature(Position pos, Creature* creature) {
	LOCKCLASS lockClass(lockMap);

	Tile* tile = GetTile(pos);
	if (tile)
		tile->RemoveTempCreature(creature);
}

void Map::FindWay(Player* player, Position to, std::list<Direction>& list) {
	LOCKCLASS lockClass1(lockMap);
	LOCKCLASS lockClass2(Tile::lockTile);
	LOCKCLASS lockClass3(MiniMap::lockMiniMap);

	Position from = player->GetPosition();

	list.clear();

	if (!minimap)
		return;

	if (from.z != to.z)
		return;

	int fx = 119 + from.x - corner.x;
	int fy = 121 + from.y - corner.y;

	int tx = 119 + to.x - corner.x;
	int ty = 121 + to.y - corner.y;

	if (fx < 0 || fx > 255 || fy < 0 || fy > 255 ||
		tx < 0 || tx > 255 || ty < 0 || ty > 255)
		return;

	std::map<POINT, AStarNode*> openNodes;
	std::map<POINT, AStarNode*> closedNodes;

	AStarNode* startNode = new(M_PLACE) AStarNode;
	startNode->parent = NULL;
	startNode->F = 0;
	startNode->x = fx;
	startNode->y = fy;
	startNode->dir = NONE;
	openNodes[doPOINT(fx, fy)] = startNode;

	POINT dirs[] = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}, {-1, -1}, {1, -1}, {1, 1}, {-1, 1}};
	int steps = 1000;
	while(openNodes.size() > 0 && steps > 0) {
		AStarNode* Q = openNodes.begin()->second;
		steps--;

		std::map<POINT, AStarNode*>::iterator itQ = openNodes.begin();
		std::map<POINT, AStarNode*>::iterator it;
		for (it = openNodes.begin(); it != openNodes.end(); it++) {
			if (it->second->F < itQ->second->F) {
				itQ = it;
				Q = itQ->second;
			}
		}

		closedNodes[itQ->first] = Q;
		openNodes.erase(itQ);

		if (Q->x == tx && Q->y == ty)
			break;

		for (int dir = NORTH; dir <= SOUTHWEST; dir++) {
			int cx = Q->x + dirs[dir].x;
			int cy = Q->y + dirs[dir].y;

			if (cx < 0 || cx > 255 || cy < 0 || cy > 255)
				continue;

			std::map<POINT, AStarNode*>::iterator it;

			it = closedNodes.find(doPOINT(cx, cy));

			unsigned short speed = minimap->GetSpeed(cx, cy);
			if (cx == tx && cy == ty) {
				Tile* tile = GetTile(cx - 119, cy - 121, corner.z);
				if (!tile || (tile && !tile->IsBlocking(true, true)))
					speed = 100;
			}

			if (it == closedNodes.end() && speed > 0) {
				it = openNodes.find(doPOINT(cx, cy));

				int F = (speed * (dir < NORTHWEST ? 1 : 3)) + 10 * (int)sqrt((tx - cx) * (tx - cx) + (ty - cy) * (ty - cy));
				if (it != openNodes.end()) {
					if (it->second->F > F) {
						it->second->parent = Q;
						it->second->F = F;
						it->second->dir = (Direction)dir;
					}
				}
				else {
					AStarNode* node = new(M_PLACE) AStarNode;
					node->parent = Q;
					node->F = F;
					node->x = cx;
					node->y = cy;
					node->dir = (Direction)dir;
					openNodes[doPOINT(cx, cy)] = node;
				}
			}
		}
	}

	std::map<POINT, AStarNode*>::iterator it;

	it = closedNodes.find(doPOINT(tx, ty));
	if (it != closedNodes.end()) {
		AStarNode* node = it->second;
		while(node) {
			if (node->dir != NONE)
				list.push_front(node->dir);
			node = node->parent;
		}
	}

	for (it = openNodes.begin(); it != openNodes.end(); it++) {
		AStarNode* node = it->second;
		delete_debug(node, M_PLACE);
	}

	for (it = closedNodes.begin(); it != closedNodes.end(); it++) {
		AStarNode* node = it->second;
		delete_debug(node, M_PLACE);
	}
}

void Map::SortThings(int sx, int sy, int width, int height) {
	LOCKCLASS lockClass1(lockMap);
	LOCKCLASS lockClass2(Tile::lockTile);

	for (int x = sx; x < sx + width; x++)
	for (int y = sy; y < sy + height; y++)
	for (int z = 0; z < 16; z++) {
		Tile* tile = tiles[x][y][z];
		if (tile)
			tile->SortThings();
	}
}

void Map::RenderMap(AD2D_Window* gfx, Player* player) {
	LOCKCLASS lockClass1(lockMap);
	LOCKCLASS lockClass2(Tile::lockTile);

	creatures.clear();

	if (!player)
		return;

	Creature* playerCreature = player->GetCreature();
	if (!playerCreature)
		return;

	Position playerPos = Position(corner.x + 8, corner.y + 6, corner.z);

	POINT pointZ = GetZ();
	unsigned char z_from = pointZ.x;
	unsigned char z_to = pointZ.y;

	float stepOffsetX;
	float stepOffsetY;
	player->GetStepOffset(stepOffsetX, stepOffsetY);

	POINT wndSize = gfx->GetWindowSize();

	AD2D_Viewport currentVP = gfx->GetCurrentViewport();

	if (AD2D_Window::_mode == API_OPENGL) {
		glViewport(0, 0, 512, 512);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluOrtho2D(0.0f, (GLfloat)512, 0.0f, (GLfloat)512);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
	}
	else if (AD2D_Window::_mode == API_DIRECT3D) {
		LPDIRECT3DDEVICE9 D3DDev = gfx->GetD3DDEV();

		D3DDev->EndScene();
		D3DDev->SetRenderTarget(0, mapImage.surfaceD3D);
		D3DDev->BeginScene();
		gfx->ClearBuffers();
	}

	for (int z = z_from; z >= z_to; z--) {
		int zOffset = playerPos.z - z;

		for (int x = 0; x < 18; x++)
		for (int y = 0; y < 14; y++) {
			Position pos;

			pos.x = playerPos.x - 8 + x + zOffset;
			pos.y = playerPos.y - 6 + y + zOffset;
			pos.z = z;

			Tile* tile = GetTile(x, y, z);
			if (tile) {
				int direction = (tile->IsHorizontal() ? 1 : (tile->IsVertical() ? 2 : 0));
				int height = 0;
				std::list<POINT> topRepeats;
				for (int stackPos = 0; stackPos < tile->GetDownSize(); stackPos++) {
					Thing* thing = tile->GetDownThingByStackPos(stackPos);
					Item* item = dynamic_cast<Item*>(thing);
					if (item && item->step == 0.0f) {
						int xOffset = ((*item)() && (*item)()->hasOffset ? (*item)()->xOffset : 0);
						int yOffset = ((*item)() && (*item)()->hasOffset ? (*item)()->yOffset : 0);

						if (item->GetShine())
							item->PrintItemShine(gfx, pos, (int)(x * 32 - (xOffset + height) - (int)(stepOffsetX * 32) - 32), (int)(y * 32 - (yOffset + height) - (int)(stepOffsetY * 32) - 32), 32, 32, direction, item->GetShine());

						item->PrintItem(gfx, pos, (int)(x * 32 - (xOffset + height) - (int)(stepOffsetX * 32) - 32), (int)(y * 32 - (yOffset + height) - (int)(stepOffsetY * 32) - 32), 32, 32, direction);
						if ((*item)()->hasHeight)
							height += (*item)()->height;

						if ((*item)()->drawWithHeightOffset) {
							if ((*item)()->m_width > 1 || (*item)()->m_height > 1) {
								for (int i = 0; i < (*item)()->m_width; i++)
								for (int j = 0; j < (*item)()->m_height; j++)
									if (i != 0 || j != 0)
										topRepeats.push_back(doPOINT(x - i, y - j));
							}
						}
					}
				}

				topRepeats.sort();
				std::list<POINT>::iterator it = topRepeats.begin();
				while(it != topRepeats.end()) {
					int x = (*it).x;
					int y = (*it).y;

					Position pos;
					pos.x = playerPos.x - 8 + x + zOffset;
					pos.y = playerPos.y - 6 + y + zOffset;
					pos.z = z;

					Tile* tile = GetTile(x, y, z);
					if (tile) {
						int height = tile->GetDownHeight();
						for (int stackPos = 0; stackPos < tile->GetCreaturesSize(); stackPos++) {
							Thing* thing = tile->GetCreatureThingByStackPos(stackPos);
							Creature* creature = dynamic_cast<Creature*>(thing);
							if (creature) {
								Outfit outfit = creature->GetOutfit();
								unsigned short id = (outfit.lookType != 0 ? Item::GetItemsCount() + outfit.lookType : outfit.lookTypeEx);

								ItemType* iType = Item::GetItemType(id);
								int xOffset = (iType && iType->hasOffset ? iType->xOffset : 0);
								int yOffset = (iType && iType->hasOffset ? iType->yOffset : 0);

								float _stepOffsetX;
								float _stepOffsetY;
								creature->GetStepOffset(_stepOffsetX, _stepOffsetY);

								if (creature->GetShine())
									creature->PrintCreatureShine(gfx, pos, (int)(x * 32 - height - (int)(stepOffsetX * 32) + (int)(_stepOffsetX * 32) - 32), (int)(y * 32 - height - (int)(stepOffsetY * 32) + (int)(_stepOffsetY * 32) - 32), 32, 32, creature->GetShine());
								creature->PrintCreature(gfx, pos, (int)(x * 32 - height - (int)(stepOffsetX * 32) + (int)(_stepOffsetX * 32) - 32), (int)(y * 32 - height - (int)(stepOffsetY * 32) + (int)(_stepOffsetY * 32) - 32), 32, 32);
							}
						}
						for (int stackPos = 0; stackPos < tile->GetTopSize(); stackPos++) {
							Thing* thing = tile->GetTopThingByStackPos(stackPos);
							Item* item = dynamic_cast<Item*>(thing);
							if (item) {
								int xOffset = ((*item)() && (*item)()->hasOffset ? (*item)()->xOffset : 0);
								int yOffset = ((*item)() && (*item)()->hasOffset ? (*item)()->yOffset : 0);
								item->PrintItem(gfx, pos, (int)(x * 32 - xOffset - (int)(stepOffsetX * 32) - 32), (int)(y * 32 - yOffset - (int)(stepOffsetY * 32) - 32), 32, 32, direction);
							}
						}
					}

					it++;
				}
				topRepeats.clear();

				for (int stackPos = 0; stackPos < tile->GetTempCreaturesSize(); stackPos++) {
					Creature* creature = tile->GetTempCreatureByStackPos(stackPos);
					if (creature && (creature->GetStep() < 0.0f || creature->newPos != Position(0, 0, 0))) {
						int height = 0;

						Tile* posTile = GetTile(creature->pos);
						if (posTile)
							height = posTile->GetDownHeight();

						int dx = creature->pos.x - pos.x;
						int dy = creature->pos.y - pos.y;

						x += dx;
						y += dy;

						Outfit outfit = creature->GetOutfit();
						unsigned short id = (outfit.lookType != 0 ? Item::GetItemsCount() + outfit.lookType : outfit.lookTypeEx);

						ItemType* iType = Item::GetItemType(id);
						int xOffset = (iType && iType->hasOffset ? iType->xOffset : 0);
						int yOffset = (iType && iType->hasOffset ? iType->yOffset : 0);

						float _stepOffsetX;
						float _stepOffsetY;
						creature->GetStepOffset(_stepOffsetX, _stepOffsetY);

						if (creature->GetShine()) {
							creature->PrintCreatureShine(gfx, pos,
													(int)(x * 32 - height - (int)(stepOffsetX * 32) + (int)(_stepOffsetX * 32) - 32),
													(int)(y * 32 - height - (int)(stepOffsetY * 32) + (int)(_stepOffsetY * 32) - 32),
													32, 32, creature->GetShine());
						}
						creature->PrintCreature(gfx, pos,
												(int)(x * 32 - height - (int)(stepOffsetX * 32) + (int)(_stepOffsetX * 32) - 32),
												(int)(y * 32 - height - (int)(stepOffsetY * 32) + (int)(_stepOffsetY * 32) - 32),
												32, 32);

						x -= dx;
						y -= dy;
					}
				}

				for (int stackPos = 0; stackPos < tile->GetCreaturesSize(); stackPos++) {
					Thing* thing = tile->GetCreatureThingByStackPos(stackPos);
					Creature* creature = dynamic_cast<Creature*>(thing);
					if (creature) {
						Outfit outfit = creature->GetOutfit();
						unsigned short id = (outfit.lookType != 0 ? Item::GetItemsCount() + outfit.lookType : outfit.lookTypeEx);

						ItemType* iType = Item::GetItemType(id);
						int xOffset = (iType && iType->hasOffset ? iType->xOffset : 0);
						int yOffset = (iType && iType->hasOffset ? iType->yOffset : 0);
						float _stepOffsetX;
						float _stepOffsetY;
						creature->GetStepOffset(_stepOffsetX, _stepOffsetY);

						if (creature->GetShine())
							creature->PrintCreatureShine(gfx, pos, (int)(x * 32 - height - (int)(stepOffsetX * 32) + (int)(_stepOffsetX * 32) - 32), (int)(y * 32 - height - 32) - (int)(stepOffsetY * 32) + (int)(_stepOffsetY * 32), 32, 32, creature->GetShine());
						creature->PrintCreature(gfx, pos, (int)(x * 32 - height - (int)(stepOffsetX * 32) + (int)(_stepOffsetX * 32) - 32), (int)(y * 32 - height - 32) - (int)(stepOffsetY * 32) + (int)(_stepOffsetY * 32), 32, 32);

						creatures.push_back(creature);
					}
				}

				MagicEffect::PrintMagicEffects(gfx, pos,
												(int)(x * 32 - height - (int)(stepOffsetX * 32) - 32),
												(int)(y * 32 - height - (int)(stepOffsetY * 32) - 32),
												32, 32);

				for (int stackPos = 0; stackPos < tile->GetTopSize(); stackPos++) {
					Thing* thing = tile->GetTopThingByStackPos(stackPos);
					Item* item = dynamic_cast<Item*>(thing);
					Creature* creature = dynamic_cast<Creature*>(thing);
					if (item) {
						int xOffset = ((*item)() && (*item)()->hasOffset ? (*item)()->xOffset : 0);
						int yOffset = ((*item)() && (*item)()->hasOffset ? (*item)()->yOffset : 0);
						item->PrintItem(gfx, pos, (int)(x * 32 - xOffset - (int)(stepOffsetX * 32) - 32), (int)(y * 32 - yOffset - (int)(stepOffsetY * 32) - 32), 32, 32, direction);
					}
				}
			}
		}

		//Moving Items
		Item::lockItem.lock();
		ItemsSet::iterator it = Item::movingItems.begin();
		for (it; it != Item::movingItems.end(); it++) {
			Item* item = (*it);
			int x = item->pos.x - playerPos.x + 8 - zOffset;
			int y = item->pos.y - playerPos.y + 6 - zOffset;
			if (item && item->step < 0.0f && item->pos.z == z) {
				float _stepOffsetX;
				float _stepOffsetY;
				item->GetStepOffset(_stepOffsetX, _stepOffsetY);

				int xOffset = ((*item)() && (*item)()->hasOffset ? (*item)()->xOffset : 0);
				int yOffset = ((*item)() && (*item)()->hasOffset ? (*item)()->yOffset : 0);

				if (item->GetShine())
					item->PrintItemShine(gfx, item->pos, (int)(x * 32 - (xOffset) - (int)(stepOffsetX * 32) + (int)(_stepOffsetX * 32) - 32), (int)(y * 32 - (yOffset) - (int)(stepOffsetY * 32) + (int)(_stepOffsetY * 32) - 32), 32, 32, 0, item->GetShine());

				item->PrintItem(gfx, item->pos, (int)(x * 32 - (xOffset) - (int)(stepOffsetX * 32) + (int)(_stepOffsetX * 32) - 32), (int)(y * 32 - (yOffset) - (int)(stepOffsetY * 32) + (int)(_stepOffsetY * 32) - 32), 32, 32, 0);
			}
		}
		Item::lockItem.unlock();

		Particles::PrintParticles(gfx, corner, z, (int)( - stepOffsetX * 32 - 32), (int)( - stepOffsetY * 32 - 32), 1);
		Distance::PrintDistances(gfx, corner, z, (int)( - stepOffsetX * 32 - 32), (int)( - stepOffsetY * 32 - 32), 32, 32);
	}

	if (AD2D_Window::_mode == API_OPENGL) {
		glBindTexture(GL_TEXTURE_2D, mapImage.texture);
		glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, 480, 352);
	}
	else if (AD2D_Window::_mode == API_DIRECT3D) {
		LPDIRECT3DDEVICE9 D3DDev = gfx->GetD3DDEV();
		LPDIRECT3DSURFACE9 backBuffer = gfx->GetD3DBackBuffer();

		D3DDev->EndScene();
		D3DDev->SetRenderTarget(0, backBuffer);
		D3DDev->BeginScene();
	}

	mapUpdated = true;

	gfx->SetViewport(currentVP);
}

void Map::PrintMap(AD2D_Window* gfx, AD2D_Font* font, Player* player, int oX, int oY, int width, int height) {
	LOCKCLASS lockClass1(lockMap);
	LOCKCLASS lockClass2(Tile::lockTile);

	if (!player)
		return;

	Creature* playerCreature = player->GetCreature();
	if (!playerCreature)
		return;

	Position playerPos = Position(corner.x + 8, corner.y + 6, corner.z);

	float ratio = (Game::options.dontStrech ? 32 : (float)width / 15);
	float zoom = (Game::options.dontStrech ? 1 : ratio / 32);
	float wndOffsetX = (Game::options.dontStrech ? (int)(width - 480) / 2 : 0);
	float wndOffsetY = (Game::options.dontStrech ? (int)(height - 352) / 2 : 0);

	POINT pointZ = GetZ();
	unsigned char z_from = pointZ.x;
	unsigned char z_to = pointZ.y;

	float stepOffsetX;
	float stepOffsetY;
	player->GetStepOffset(stepOffsetX, stepOffsetY);

	AD2D_Image* dir_arrow = Icons::GetDirArrowIcon();
	AD2D_Image* pos_arrow = Icons::GetPosArrowIcon();

	Lights::UpdateDynamicLightMap(this);

	if (Game::options.renderToTexture) {
		AD2D_Window::SetColor(1.0f, 1.0f, 1.0f, 1.0f);
		if (mapUpdated)
			gfx->PutImage(oX, oY, width, height, mapImage);
		mapUpdated = false;
	}
	else {
		creatures.clear();
		for (int z = z_from; z >= z_to; z--) {
			int zOffset = playerPos.z - z;

			for (int x = 0; x < 18; x++)
			for (int y = 0; y < 14; y++) {
				Position pos;

				pos.x = playerPos.x - 8 + x + zOffset;
				pos.y = playerPos.y - 6 + y + zOffset;
				pos.z = z;

				Tile* tile = GetTile(x, y, z);
				if (tile) {
					int direction = (tile->IsHorizontal() ? 1 : (tile->IsVertical() ? 2 : 0));
					int height = 0;
					std::list<POINT> topRepeats;
					for (int stackPos = 0; stackPos < tile->GetDownSize(); stackPos++) {
						Thing* thing = tile->GetDownThingByStackPos(stackPos);
						Item* item = dynamic_cast<Item*>(thing);
						if (item && item->step == 0.0f) {
							int xOffset = ((*item)() && (*item)()->hasOffset ? (*item)()->xOffset : 0);
							int yOffset = ((*item)() && (*item)()->hasOffset ? (*item)()->yOffset : 0);

							if (item->GetShine())
								item->PrintItemShine(gfx, pos, oX + wndOffsetX + (x * ratio - ((float)(xOffset + height) / 32) * ratio - ratio * stepOffsetX - ratio), wndOffsetY + (y * ratio - ((float)(yOffset + height) / 32) * ratio - ratio * stepOffsetY - ratio), ratio, ratio, direction, item->GetShine());

							item->PrintItem(gfx, pos, oX + wndOffsetX + (x * ratio - ((float)(xOffset + height) / 32) * ratio - ratio * stepOffsetX - ratio), oY + wndOffsetY + (y * ratio - ((float)(yOffset + height) / 32) * ratio - ratio * stepOffsetY - ratio), ratio, ratio, direction);
							if ((*item)()->hasHeight)
								height += (*item)()->height;

							if (!(*item)()->notMoveable) {
								if ((*item)()->m_width > 1 || (*item)()->m_height > 1) {
								for (int i = 0; i < (*item)()->m_width; i++)
								for (int j = 0; j < (*item)()->m_height; j++)
									if (i != 0 || j != 0)
										topRepeats.push_back(doPOINT(x - i, y - j));
								}
							}
						}
					}

					topRepeats.sort();
					std::list<POINT>::iterator it = topRepeats.begin();
					while(it != topRepeats.end()) {
						int x = (*it).x;
						int y = (*it).y;

						Position pos;
						pos.x = playerPos.x - 8 + x + zOffset;
						pos.y = playerPos.y - 6 + y + zOffset;
						pos.z = z;

						Tile* tile = GetTile(x, y, z);
						if (tile) {
							int height = tile->GetDownHeight();
							for (int stackPos = 0; stackPos < tile->GetCreaturesSize(); stackPos++) {
								Thing* thing = tile->GetCreatureThingByStackPos(stackPos);
								Creature* creature = dynamic_cast<Creature*>(thing);
								if (creature) {
									Outfit outfit = creature->GetOutfit();
									unsigned short id = (outfit.lookType != 0 ? Item::GetItemsCount() + outfit.lookType : outfit.lookTypeEx);

									ItemType* iType = Item::GetItemType(id);
									int xOffset = (iType && iType->hasOffset ? iType->xOffset : 0);
									int yOffset = (iType && iType->hasOffset ? iType->yOffset : 0);

									float _stepOffsetX;
									float _stepOffsetY;
									creature->GetStepOffset(_stepOffsetX, _stepOffsetY);

									if (creature->GetShine())
										creature->PrintCreatureShine(gfx, pos, oX + wndOffsetX + (x * ratio - ((float)(height) / 32) * ratio - ratio * stepOffsetX + ratio * _stepOffsetX - ratio), oY + wndOffsetY + (y * ratio - ((float)(height) / 32) * ratio - ratio * stepOffsetY + ratio * _stepOffsetY - ratio), ratio, ratio, creature->GetShine());
									creature->PrintCreature(gfx, pos, oX + wndOffsetX + (x * ratio - ((float)(height) / 32) * ratio - ratio * stepOffsetX + ratio * _stepOffsetX - ratio), oY + wndOffsetY + (y * ratio - ((float)(height) / 32) * ratio - ratio * stepOffsetY + ratio * _stepOffsetY - ratio), ratio, ratio);
								}
							}
							for (int stackPos = 0; stackPos < tile->GetTopSize(); stackPos++) {
								Thing* thing = tile->GetTopThingByStackPos(stackPos);
								Item* item = dynamic_cast<Item*>(thing);
								if (item) {
									int xOffset = ((*item)() && (*item)()->hasOffset ? (*item)()->xOffset : 0);
									int yOffset = ((*item)() && (*item)()->hasOffset ? (*item)()->yOffset : 0);
									item->PrintItem(gfx, pos, oX + wndOffsetX + (x * ratio - ((float)(xOffset) / 32) * ratio - ratio * stepOffsetX - ratio), oY + wndOffsetY + (y * ratio - ((float)(yOffset) / 32) * ratio - ratio * stepOffsetY - ratio), ratio, ratio, direction);
								}
							}
						}

						it++;
					}
					topRepeats.clear();

					for (int stackPos = 0; stackPos < tile->GetTempCreaturesSize(); stackPos++) {
						Creature* creature = tile->GetTempCreatureByStackPos(stackPos);
						if (creature && (creature->GetStep() < 0.0f || creature->newPos != Position(0, 0, 0))) {
							int height = 0;

							Tile* posTile = GetTile(creature->pos);
							if (posTile)
								height = posTile->GetDownHeight();

							int dx = creature->pos.x - pos.x;
							int dy = creature->pos.y - pos.y;

							x += dx;
							y += dy;

							Outfit outfit = creature->GetOutfit();
							unsigned short id = (outfit.lookType != 0 ? Item::GetItemsCount() + outfit.lookType : outfit.lookTypeEx);

							ItemType* iType = Item::GetItemType(id);
							int xOffset = (iType && iType->hasOffset ? iType->xOffset : 0);
							int yOffset = (iType && iType->hasOffset ? iType->yOffset : 0);

							float _stepOffsetX;
							float _stepOffsetY;
							creature->GetStepOffset(_stepOffsetX, _stepOffsetY);

							if (creature->GetShine()) {
								creature->PrintCreatureShine(gfx, pos,
														oX + wndOffsetX + (x * ratio - ((float)(height) / 32) * ratio - ratio * stepOffsetX + ratio * _stepOffsetX - ratio),
														oY + wndOffsetY + (y * ratio - ((float)(height) / 32) * ratio - ratio * stepOffsetY + ratio * _stepOffsetY - ratio),
														ratio, ratio, creature->GetShine());
							}
							creature->PrintCreature(gfx, pos,
													oX + wndOffsetX + (x * ratio - ((float)(height) / 32) * ratio - ratio * stepOffsetX + ratio * _stepOffsetX - ratio),
													oY + wndOffsetY + (y * ratio - ((float)(height) / 32) * ratio - ratio * stepOffsetY + ratio * _stepOffsetY - ratio),
													ratio, ratio);

							x -= dx;
							y -= dy;
						}
					}

					for (int stackPos = 0; stackPos < tile->GetCreaturesSize(); stackPos++) {
						Thing* thing = tile->GetCreatureThingByStackPos(stackPos);
						Creature* creature = dynamic_cast<Creature*>(thing);
						if (creature) {
							Outfit outfit = creature->GetOutfit();
							unsigned short id = (outfit.lookType != 0 ? Item::GetItemsCount() + outfit.lookType : outfit.lookTypeEx);

							ItemType* iType = Item::GetItemType(id);
							int xOffset = (iType && iType->hasOffset ? iType->xOffset : 0);
							int yOffset = (iType && iType->hasOffset ? iType->yOffset : 0);

							float _stepOffsetX;
							float _stepOffsetY;
							creature->GetStepOffset(_stepOffsetX, _stepOffsetY);

							if (creature->GetShine())
								creature->PrintCreatureShine(gfx, pos, oX + wndOffsetX + (x * ratio - ((float)(height) / 32) * ratio - ratio * stepOffsetX + ratio * _stepOffsetX - ratio), oY + wndOffsetY + (y * ratio - ((float)(height) / 32) * ratio - ratio * stepOffsetY + ratio * _stepOffsetY - ratio), ratio, ratio, creature->GetShine());
							creature->PrintCreature(gfx, pos, oX + wndOffsetX + (x * ratio - ((float)(height) / 32) * ratio - ratio * stepOffsetX + ratio * _stepOffsetX - ratio), oY + wndOffsetY + (y * ratio - ((float)(height) / 32) * ratio - ratio * stepOffsetY + ratio * _stepOffsetY - ratio), ratio, ratio);

							creatures.push_back(creature);
						}
					}

					MagicEffect::PrintMagicEffects(gfx, pos,
													oX + wndOffsetX + (x * ratio - ((float)(height) / 32) * ratio - ratio * stepOffsetX - ratio),
													oY + wndOffsetY + (y * ratio - ((float)(height) / 32) * ratio - ratio * stepOffsetY - ratio),
													ratio, ratio);

					for (int stackPos = 0; stackPos < tile->GetTopSize(); stackPos++) {
						Thing* thing = tile->GetTopThingByStackPos(stackPos);
						Item* item = dynamic_cast<Item*>(thing);
						Creature* creature = dynamic_cast<Creature*>(thing);
						if (item) {
							int xOffset = ((*item)() && (*item)()->hasOffset ? (*item)()->xOffset : 0);
							int yOffset = ((*item)() && (*item)()->hasOffset ? (*item)()->yOffset : 0);
							item->PrintItem(gfx, pos, oX + wndOffsetX + (x * ratio - ((float)(xOffset) / 32) * ratio - ratio * stepOffsetX - ratio), oY + wndOffsetY + (y * ratio - ((float)(yOffset) / 32) * ratio - ratio * stepOffsetY - ratio), ratio, ratio, direction);
						}
					}
				}
			}

			//Moving Items
			Item::lockItem.lock();
			ItemsSet::iterator it = Item::movingItems.begin();
			for (it; it != Item::movingItems.end(); it++) {
				Item* item = (*it);
				int x = item->pos.x - playerPos.x + 8 - zOffset;
				int y = item->pos.y - playerPos.y + 6 - zOffset;
				if (item && item->step < 0.0f && item->pos.z == z) {
					float _stepOffsetX;
					float _stepOffsetY;
					item->GetStepOffset(_stepOffsetX, _stepOffsetY);

					int xOffset = ((*item)() && (*item)()->hasOffset ? (*item)()->xOffset : 0);
					int yOffset = ((*item)() && (*item)()->hasOffset ? (*item)()->yOffset : 0);

					if (item->GetShine())
						item->PrintItemShine(gfx, item->pos, oX + (x * ratio - (xOffset) - (stepOffsetX * ratio) + (_stepOffsetX * ratio) - 32), oY + (y * ratio - (yOffset) - (stepOffsetY * ratio) + (_stepOffsetY * ratio) - 32), ratio, ratio, 0, item->GetShine());

					item->PrintItem(gfx, item->pos, oX + (x * ratio - (xOffset) - (stepOffsetX * ratio) + (_stepOffsetX * ratio) - 32), oY + (y * ratio - (yOffset) - (stepOffsetY * ratio) + (_stepOffsetY * ratio) - 32), ratio, ratio, 0);
				}
			}
			Item::lockItem.unlock();

			Particles::PrintParticles(gfx, corner, z, oX + wndOffsetX - stepOffsetX * ratio - ratio, oY + wndOffsetY - stepOffsetY * ratio - ratio, zoom);
			Distance::PrintDistances(gfx, corner, z, oX + wndOffsetX - stepOffsetX * ratio - ratio, oY + wndOffsetY - stepOffsetY * ratio - ratio, ratio, ratio);
		}
	}

	//Position arrows
	if (!posArrows.empty() && pos_arrow) {
		PositionArrowsSet::iterator ait = posArrows.begin();
		for (ait; ait != posArrows.end(); ait++) {
			Position pos = *ait;

			if (pos.z == playerPos.z) {
				int zOffset = playerPos.z - pos.z;
				int x = 8 + pos.x - playerPos.x - zOffset;
				int y = 6 + pos.y - playerPos.y - zOffset;

				float period = (float)(RealTime::getTime() % 1000) / 1000;
				float factor = sin(period * 2 * 3.141592654);
				float factorX = (ratio / 8) * factor;
				float factorY = (ratio / 4) * factor;

				float px = wndOffsetX + x * ratio - stepOffsetX * ratio - ratio;
				float py = wndOffsetY + y * ratio - stepOffsetY * ratio - (ratio * 5) / 2;

				gfx->PutImage(oX + px - factorX, oY + py - 2 * factorY, oX + px + factorX + ratio, oY + py + 2 * ratio, *pos_arrow);
			}
		}
	}

	std::list<Creature*>::iterator it = creatures.begin();
	for (it; it != creatures.end(); it++) {
		Creature* creature = *it;
		ItemType* iType = (*creature)();

		int zOffset = playerPos.z - creature->pos.z;
		int x = 8 + creature->pos.x - playerPos.x - zOffset;
		int y = 6 + creature->pos.y - playerPos.y - zOffset;

		int height = 0;
		Tile* posTile = GetTile(creature->pos);
		if (posTile)
			height = posTile->GetDownHeight();

		float pyOffset = (iType ? iType->yOffset : 0);

		float px = wndOffsetX + x * ratio - ((float)height / 32) * ratio - stepOffsetX * ratio - ratio;
		float py = wndOffsetY + y * ratio - ((float)(height + pyOffset) / 32) * ratio - stepOffsetY * ratio - ratio;

		if (creature->pos.z == playerPos.z)
			creature->PrintEquipment(gfx, creature->pos, oX + px, oY + py, ratio, ratio);

		if (creature->pos.z == playerPos.z || !IsHiddenTile(x, y, creature->pos.z, z_to))
			creature->PrintIcons(gfx, creature->pos, oX + px, oY + py, ratio, ratio);

		if (creature->pos.z == playerPos.z) {
			int name = (iType && Game::options.showNames ? creature->GetHealth() : -1);
			int hp = (iType && Game::options.showBars ? creature->GetHealth() : -1);
			int mana = (iType && Game::options.showBars && playerCreature == creature ? (int)((float)(player->GetStatistics()->mana * 100) / player->GetStatistics()->maxMana) : -1);
			bool attacked = (Player::GetAttackID() != 0 && creature->GetID() == Player::GetAttackID() ? true : false);
			bool followed = (Player::GetFollowID() != 0 && creature->GetID() == Player::GetFollowID() ? true : false);
			bool selected = (Player::GetSelectID() != 0 && creature->GetID() == Player::GetSelectID() ? true : false);

			creature->PrintDetails(gfx, font, creature->pos, oX + px, oY + py, ratio, ratio, name, hp, mana, attacked, followed, selected);
		}
	}

	Position dirPos = player->GetDirectionPos();
	if (dirPos != Position(0, 0, 0) && dirPos != playerPos && dir_arrow) {
		Position pos = playerCreature->pos;

		float wdh = (width - (oX*2) - dir_arrow->GetWidth()) / 2;
		float hgt = (height - (oY*2) - dir_arrow->GetHeight()) / 2;

		float xDist = (dirPos.x - pos.x) * 32 - stepOffsetX * 32;
		float yDist = (dirPos.y - pos.y) * 32 - stepOffsetY * 32;
		float rad = 0.0f;
		if (yDist != 0.0f)
			rad = atan(-xDist / yDist) + (yDist > 0.0f ? 3.141592654 : 0);
		else
			rad = (xDist < 0 ? 3 * (3.141592654 / 2) : (3.141592654 / 2));

		float xP = sin(rad);
		float yP = -cos(rad);

		float dist;
		if (abs(yP) > 0.75f)
			dist = abs(hgt / yP);
		else if (abs(xP) > 0.75f)
			dist = abs(wdh / xP);
		else
			dist = std::min(abs(wdh / xP), abs(hgt / yP));

		float pixDist = zoom * (sqrt(pow(xDist, 2) + pow(yDist, 2)) - 16);
		if (dist > pixDist)
			dist = pixDist;

		if (dirPos.z < playerPos.z && RealTime::getTime() % 1000 < 700)
			gfx->RotImage(oX + (width / 2), oY + (height / 2), 0, 0.7f * zoom, *dir_arrow);
		if (dirPos.z > playerPos.z && RealTime::getTime() % 1000 < 700)
			gfx->RotImage(oX + (width / 2), oY + (height / 2), 180, 0.7f * zoom, *dir_arrow);

		gfx->RotImage(oX + (width / 2) + xP * dist, oY + (height / 2) + yP * dist, 180 * rad / 3.141592654, 1, *dir_arrow);
	}

	Lights::PrintLightMap(gfx, corner.z, oX + wndOffsetX - stepOffsetX * ratio - ratio, oY + wndOffsetY - stepOffsetY * ratio - ratio, ratio, ratio);
	Messages::PrintMessages(gfx, font, corner, playerPos.z, oX + wndOffsetX - stepOffsetX * ratio - ratio, oY + wndOffsetY - stepOffsetY * ratio - ratio, ratio);
}


//Lua  functions

int Map::LuaGetPathTo(lua_State* L) {
	Position pos = LuaScript::PopPosition(L);

	Game* game = Game::game;
	Map* map = (game ? game->GetMap() : NULL);
	Player* player = (game ? game->GetPlayer() : NULL);

	std::string list;
	std::list<Direction> dirList;

	if (map && player && pos != Position(0, 0, 0)) {
		map->FindWay(player, pos, dirList);

		lua_newtable(L);
		std::list<Direction>::reverse_iterator it = dirList.rbegin();
		int i = 1;
		for (it; it != dirList.rend(); it++, i++) {
			lua_pushnumber(L, i);
			lua_pushnumber(L, (int)(*it));
			lua_settable(L, -3);
		}
		return 1;
	}

	lua_pushnil(L);
	return 1;
}

int Map::LuaGetWaypoint(lua_State* L) {
	int waypoint = LuaScript::PopNumber(L);

	Game* game = Game::game;
	Map* map = (game ? game->GetMap() : NULL);
	MiniMap* mmap = (map ? map->GetMiniMap() : NULL);

	Position pos(0, 0, 0);
	unsigned char type = 0;
	std::string comment;

	if (mmap) {
		int num = waypoint;
		WaypointsList waypoints = mmap->GetWaypoints();
		WaypointsList::iterator it = waypoints.begin();
		for (it; it != waypoints.end() && num > 0; it++, num--);

		if (it != waypoints.end()) {
			pos = it->first;
			type = it->second.first;
			comment = it->second.second;
		}
	}

	lua_newtable(L);
	lua_pushstring(L, "pos");
	LuaScript::PushPosition(L, pos);
	lua_settable(L, -3);
	lua_pushstring(L, "type");
	lua_pushnumber(L, type);
	lua_settable(L, -3);
	lua_pushstring(L, "comment");
	lua_pushstring(L, comment.c_str());
	lua_settable(L, -3);
	return 1;
}

int Map::LuaGetWaypointsSize(lua_State* L) {
	Game* game = Game::game;
	Map* map = (game ? game->GetMap() : NULL);
	MiniMap* mmap = (map ? map->GetMiniMap() : NULL);

	int size = 0;
	if (mmap) {
		WaypointsList waypoints = mmap->GetWaypoints();
		size = waypoints.size();
	}

	LuaScript::PushNumber(L, size);
	return 1;
}

int Map::LuaGetItemByStackPos(lua_State* L) {
	unsigned char stackPos = LuaScript::PopNumber(L);
	Position pos = LuaScript::PopPosition(L);

	Game* game = Game::game;
	Map* map = (game ? game->GetMap() : NULL);
	Tile* tile = (map ? map->GetTile(pos) : NULL);

	Item* item = NULL;

	if (tile) {
		Thing* thing = tile->GetThingByStackPos(stackPos);
		item = dynamic_cast<Item*>(thing);
	}

	int id = 0;
	int count = 0;
	int stack = 0;
	if (item) {
		id = item->GetID();
		count = item->GetCount();
		stack = stackPos;
	}

	lua_newtable(L);
	lua_pushstring(L, "id");
	lua_pushnumber(L, id);
	lua_settable(L, -3);
	lua_pushstring(L, "count");
	lua_pushnumber(L, count);
	lua_settable(L, -3);
	lua_pushstring(L, "stack");
	lua_pushnumber(L, stack);
	lua_settable(L, -3);
	lua_pushstring(L, "pointer");
	lua_pushnumber(L, (unsigned long)item);
	lua_settable(L, -3);
	return 1;
}

int Map::LuaGetTopTopItem(lua_State* L) {
	Position pos = LuaScript::PopPosition(L);

	Game* game = Game::game;
	Map* map = (game ? game->GetMap() : NULL);
	Tile* tile = (map ? map->GetTile(pos) : NULL);

	Item* item = NULL;
	int stackPos = 0;

	if (tile) {
		Thing* thing = tile->GetTopTopThing();
		item = dynamic_cast<Item*>(thing);
		stackPos = tile->GetThingStackPos(thing);
	}

	int id = 0;
	int count = 0;
	int stack = 0;
	if (item) {
		id = item->GetID();
		count = item->GetCount();
		stack = stackPos;
	}

	lua_newtable(L);
	lua_pushstring(L, "id");
	lua_pushnumber(L, id);
	lua_settable(L, -3);
	lua_pushstring(L, "count");
	lua_pushnumber(L, count);
	lua_settable(L, -3);
	lua_pushstring(L, "stack");
	lua_pushnumber(L, stack);
	lua_settable(L, -3);
	lua_pushstring(L, "pointer");
	lua_pushnumber(L, (unsigned long)item);
	lua_settable(L, -3);
	return 1;
}

int Map::LuaGetTopDownItem(lua_State* L) {
	Position pos = LuaScript::PopPosition(L);

	Game* game = Game::game;
	Map* map = (game ? game->GetMap() : NULL);
	Tile* tile = (map ? map->GetTile(pos) : NULL);

	Item* item = NULL;
	int stackPos = 0;

	if (tile) {
		Thing* thing = tile->GetTopDownThing();
		item = dynamic_cast<Item*>(thing);
		stackPos = tile->GetThingStackPos(thing);
	}

	int id = 0;
	int count = 0;
	int stack = 0;
	if (item) {
		id = item->GetID();
		count = item->GetCount();
		stack = stackPos;
	}

	lua_newtable(L);
	lua_pushstring(L, "id");
	lua_pushnumber(L, id);
	lua_settable(L, -3);
	lua_pushstring(L, "count");
	lua_pushnumber(L, count);
	lua_settable(L, -3);
	lua_pushstring(L, "stack");
	lua_pushnumber(L, stack);
	lua_settable(L, -3);
	lua_pushstring(L, "pointer");
	lua_pushnumber(L, (unsigned long)item);
	lua_settable(L, -3);
	return 1;
}

int Map::LuaGetUseableItem(lua_State* L) {
	int params = lua_gettop(L);

	bool useWith = false;
	if (params > 1)
		useWith = (bool)LuaScript::PopNumber(L);
	Position pos = LuaScript::PopPosition(L);

	Game* game = Game::game;
	Map* map = (game ? game->GetMap() : NULL);
	Tile* tile = (map ? map->GetTile(pos) : NULL);

	Item* item = NULL;
	int stackPos = 0;

	if (tile) {
		Thing* thing = tile->GetUseableThing(useWith);
		item = dynamic_cast<Item*>(thing);
		stackPos = tile->GetThingStackPos(thing);
	}

	int id = 0;
	int count = 0;
	int stack = 0;
	if (item) {
		id = item->GetID();
		count = item->GetCount();
		stack = stackPos;
	}

	lua_newtable(L);
	lua_pushstring(L, "id");
	lua_pushnumber(L, id);
	lua_settable(L, -3);
	lua_pushstring(L, "count");
	lua_pushnumber(L, count);
	lua_settable(L, -3);
	lua_pushstring(L, "stack");
	lua_pushnumber(L, stack);
	lua_settable(L, -3);
	lua_pushstring(L, "pointer");
	lua_pushnumber(L, (unsigned long)item);
	lua_settable(L, -3);
	return 1;
}

int Map::LuaSetDirectionPosition(lua_State* L) {
	Position pos = LuaScript::PopPosition(L);

	SetDirectionPosition(pos);

	return 1;
}

int Map::LuaAddPositionArrow(lua_State* L) {
	Position pos = LuaScript::PopPosition(L);

	AddPositionArrow(pos);

	return 1;
}

int Map::LuaRemovePositionArrow(lua_State* L) {
	Position pos = LuaScript::PopPosition(L);

	RemovePositionArrow(pos);

	return 1;
}

int Map::LuaClearPositionArrows(lua_State* L) {
	ClearPositionArrows();

	return 1;
}


void Map::LuaRegisterFunctions(lua_State* L) {
	//getPathTo(position) : list{direction, ...}
	lua_register(L, "getPathTo", Map::LuaGetPathTo);

	//getWaypoint(index) : waypoint{pos{x, y, z}, type, comment}
	lua_register(L, "getWaypoint", Map::LuaGetWaypoint);

	//getWaypointsSize() : size
	lua_register(L, "getWaypointsSize", Map::LuaGetWaypointsSize);

	//getItemByStackPos(position, stackpos) : item{id, count, stack, pointer}
	lua_register(L, "getItemByStackPos", Map::LuaGetItemByStackPos);

	//getTopTopItem(position) : item{id, count, stack, pointer}
	lua_register(L, "getTopTopItem", Map::LuaGetTopTopItem);

	//getTopDownItem(position) : item{id, count, stack, pointer}
	lua_register(L, "getTopDownItem", Map::LuaGetTopDownItem);

	//getUseableItem(position, useWith = 0) : item{id, count, stack, pointer}
	lua_register(L, "getUseableItem", Map::LuaGetUseableItem);

	//setDirectionPosition(position)
	lua_register(L, "setDirectionPosition", Map::LuaSetDirectionPosition);

	//addPositionArrow(position)
	lua_register(L, "addPositionArrow", Map::LuaAddPositionArrow);

	//removePositionArrow(position)
	lua_register(L, "removePositionArrow", Map::LuaRemovePositionArrow);

	//clearPositionArrows()
	lua_register(L, "removePositionArrow", Map::LuaRemovePositionArrow);
}
