/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#include "item.h"

#include <iostream>
#include <algorithm>

#include "allocator.h"
#include "const.h"
#include "creature.h"
#include "filemanager.h"
#include "luascript.h"
#include "realtime.h"
#include "tools.h"


// ---- Sprites ---- //

unsigned char*	Sprites::spritesData = NULL;

std::map<unsigned short, unsigned long>		Sprites::handlers;
std::map<unsigned short, AD2D_Image*>		Sprites::sprites;
std::map<unsigned short, ColorTemplate*>	Sprites::templates;

std::string Sprites::version = "";


bool Sprites::LoadSpritesFromSpr(std::string path, std::string ver) {
	FileManager* files = FileManager::fileManager;
	if (!files)
		return false;

    spritesData = files->GetFileData(path);
    if (!spritesData)
    	return false;

    unsigned long fPos = 0;

	unsigned short HDRA = *((unsigned short*)(spritesData + fPos));		fPos += 2;
	unsigned short HDRB = *((unsigned short*)(spritesData + fPos));		fPos += 2;
	unsigned short SIZE = *((unsigned short*)(spritesData + fPos));		fPos += 2;

	for (int n = 1; n < SIZE; n++)
	{
		unsigned int handler = 0;

		fPos = 2 + n * 4;
		handler = *((unsigned int*)(spritesData + fPos));		fPos += 4;

		if (handler != 0)
			handlers[n] = handler;
	}

	version = ver;

	return true;
}

bool Sprites::LoadSprite(unsigned short number, bool colorTemplate, bool updateOutfit) {
	if (!spritesData)
		return false;

	std::map<unsigned short, unsigned long>::iterator it = handlers.find(number);
	if (it != handlers.end() && it->second != 0)
	{
		unsigned int wsk = 0;
		unsigned int wskMax = 0;
		unsigned int transp = 0;
		unsigned int pixels = 0;

		unsigned long fPos = it->second;
		unsigned char rt = *((unsigned char*)(spritesData + fPos));		fPos++;
		unsigned char gt = *((unsigned char*)(spritesData + fPos));		fPos++;
		unsigned char bt = *((unsigned char*)(spritesData + fPos));		fPos++;
		wskMax = *((unsigned short*)(spritesData + fPos));		fPos += 2;

		unsigned char data[4096];
		for (int y = 0; y < 32; y++)
		for (int x = 0; x < 32; x++)
		{
			unsigned char pixR = 0;
			unsigned char pixG = 0;
			unsigned char pixB = 0;
			if (transp == 0 && pixels == 0)
			{
				transp = *((unsigned short*)(spritesData + fPos));		fPos += 2;
				pixels = *((unsigned short*)(spritesData + fPos));		fPos += 2;
				wsk += 4;
			}
			if (transp > 0) transp--;
			else if (transp == 0 && wsk < wskMax)
			{
				if (pixels > 0)
				{
					unsigned char r = *((unsigned char*)(spritesData + fPos));		fPos++;
					unsigned char g = *((unsigned char*)(spritesData + fPos));		fPos++;
					unsigned char b = *((unsigned char*)(spritesData + fPos));		fPos++;
					pixR = std::max(1, (int)r);
					pixG = std::max(1, (int)g);
					pixB = std::max(1, (int)b);
					if (colorTemplate) {
						pixR = (pixR == 255 ? 255 : 0);
						pixG = (pixG == 255 ? 255 : 0);
						pixB = (pixB == 255 ? 255 : 0);
					}
					wsk += 3;
					pixels--;
				}
			}

			data[(x + y * 32) * 4 + 0] = pixR;
			data[(x + y * 32) * 4 + 1] = pixG;
			data[(x + y * 32) * 4 + 2] = pixB;
			data[(x + y * 32) * 4 + 3] = (pixR == 0 && pixG == 0 && pixB == 0 ? 0 : 255);
		}

		if (colorTemplate) {
			ColorTemplate* ct = new ColorTemplate;
			if (!updateOutfit)
				CreateColorTemplate(data, ct);
			templates[number] = ct;
		}
		else {
			AD2D_Image* image = new AD2D_Image;
			image->Create(32, 32, data);

			sprites[number] = image;
		}
	}

	return true;
}

bool Sprites::UpdateOutfit(AD2D_Image* outfit, ColorTemplate* ct, ColorTemplate* ct_outfit) {
	if (!outfit || !ct)
		return false;

	unsigned char* outfitData = new unsigned char[32 * 32 * 4];
	unsigned char* templateData = new unsigned char[32 * 32 * 4];
	outfit->GetData(outfitData);
	ct->all->GetData(templateData);

	for (int x = 0; x < 32; x++)
	for (int y = 0; y < 32; y++) {
		unsigned char* outfitColor = outfitData + ((x + y * 32) * 4);
		unsigned char* templateColor = templateData + ((x + y * 32) * 4);

		if (templateColor[3] == 255) {
			int color = (outfitColor[0] + outfitColor[1] + outfitColor[2]) / 3;
			outfitColor[0] = color;
			outfitColor[1] = color;
			outfitColor[2] = color;
		}
	}

	ct_outfit->all = new AD2D_Image;
	ct_outfit->all->Create(32, 32, outfitData);

	delete[] outfitData;
	delete[] templateData;

	return true;
}

void Sprites::CreateColorTemplate(unsigned char* data, ColorTemplate* ct) {
	unsigned char dataAll[4096];
	unsigned char dataHead[4096];
	unsigned char dataBody[4096];
	unsigned char dataLegs[4096];
	unsigned char dataFeet[4096];

	memset(dataAll, 0, 4096);
	memset(dataHead, 0, 4096);
	memset(dataBody, 0, 4096);
	memset(dataLegs, 0, 4096);
	memset(dataFeet, 0, 4096);

	for (int x = 0; x < 32; x++)
	for (int y = 0; y < 32; y++) {
		int pixR = data[(x + y * 32) * 4 + 0];
		int pixG = data[(x + y * 32) * 4 + 1];
		int pixB = data[(x + y * 32) * 4 + 2];
		int alpha = data[(x + y * 32) * 4 + 3];

		bool temp = false;
		if (pixR == 255 && pixG == 0 && pixB == 0) {
			dataBody[(x + y *32) * 4 + 0] = 255;
			dataBody[(x + y *32) * 4 + 1] = 255;
			dataBody[(x + y *32) * 4 + 2] = 255;
			dataBody[(x + y *32) * 4 + 3] = alpha;
			temp = true;
		}
		else if (pixR == 0 && pixG == 255 && pixB <= 0) {
			dataLegs[(x + y *32) * 4 + 0] = 255;
			dataLegs[(x + y *32) * 4 + 1] = 255;
			dataLegs[(x + y *32) * 4 + 2] = 255;
			dataLegs[(x + y *32) * 4 + 3] = alpha;
			temp = true;
		}
		else if (pixR == 0 && pixG == 0 && pixB == 255) {
			dataFeet[(x + y *32) * 4 + 0] = 255;
			dataFeet[(x + y *32) * 4 + 1] = 255;
			dataFeet[(x + y *32) * 4 + 2] = 255;
			dataFeet[(x + y *32) * 4 + 3] = alpha;
			temp = true;
		}
		else if (pixR == 255 && pixG == 255 && pixB == 0) {
			dataHead[(x + y *32) * 4 + 0] = 255;
			dataHead[(x + y *32) * 4 + 1] = 255;
			dataHead[(x + y *32) * 4 + 2] = 255;
			dataHead[(x + y *32) * 4 + 3] = alpha;
			temp = true;
		}

		dataAll[(x + y * 32) * 4 + 0] = pixR;
		dataAll[(x + y * 32) * 4 + 1] = pixG;
		dataAll[(x + y * 32) * 4 + 2] = pixB;
		dataAll[(x + y * 32) * 4 + 3] = (temp ? 255 : 0);
	}

	ct->all = new AD2D_Image;
	ct->head = new AD2D_Image;
	ct->body = new AD2D_Image;
	ct->legs = new AD2D_Image;
	ct->feet = new AD2D_Image;

	ct->all->Create(32, 32, dataAll);
	ct->head->Create(32, 32, dataHead);
	ct->body->Create(32, 32, dataBody);
	ct->legs->Create(32, 32, dataLegs);
	ct->feet->Create(32, 32, dataFeet);
}

void Sprites::ReleaseSpritesData() {
	if (spritesData)
		delete[] spritesData;

	spritesData = NULL;

	version = "";
}

void Sprites::ReleaseSprites() {
	std::map<unsigned short, AD2D_Image*>::iterator it = sprites.begin();
	for (it; it != sprites.end(); it++) {
		AD2D_Image* image = it->second;
		if (image)
			delete image;
	}

	std::map<unsigned short, ColorTemplate*>::iterator itt = templates.begin();
	for (itt; itt != templates.end(); itt++) {
		ColorTemplate* ct = itt->second;
		if (ct) {
			if (ct->all) delete ct->all;
			if (ct->head) delete ct->head;
			if (ct->body) delete ct->body;
			if (ct->legs) delete ct->legs;
			if (ct->feet) delete ct->feet;
			delete ct;
		}
	}

	sprites.clear();
	templates.clear();
	Creature::loadedLooktypes.clear();
}


AD2D_Image* Sprites::GetSprite(unsigned short number) {
	std::map<unsigned short, AD2D_Image*>::iterator it = sprites.find(number);
	if (it != sprites.end())
		return it->second;
	else {
		LoadSprite(number, false);
		it = sprites.find(number);
		if (it != sprites.end())
			return it->second;
	}

	return NULL;
}

ColorTemplate* Sprites::GetColorTemplate(unsigned short number, AD2D_Image* outfit, ColorTemplate* ct) {
	std::map<unsigned short, ColorTemplate*>::iterator it = templates.find(number);
	if (it != templates.end())
		return it->second;
	else if (outfit) {
		LoadSprite(number, true, (ct ? true : false));
		it = templates.find(number);
		if (it != templates.end()) {
			if (ct)
				UpdateOutfit(outfit, ct, it->second);
			return it->second;
		}
	}

	return NULL;
}


// ---- ItemType ---- //

ItemType::ItemType() {
	ground = false;
	topItem1 = false;
	topItem2 = false;
	topItem3 = false;
	container = false;
	stackable = false;
	ladder = false;
	useable = false;
	writeable1 = false;
	writeable2 = false;
	fluid = false;
	multiType = false;
	blocking = false;
	notMoveable = false;
	blockMissiles = false;
	blockPathFind = false;
	pickupable = false;
	wall = false;
	horizontal = false;
	vertical = false;
	rotateable = false;
	light = false;
	hole = false;
	hasHeight = false;
	hasOffset = false;
	drawWithHeightOffset = false;
	idleAnimated = false;
	minimap = false;
	action = false;
	floorChange = false;
	animated = false;

	speed = 0;
	height = 0;
	xOffset = 0;
	yOffset = 0;
	lightColor = 0;
	lightRadius = 0;
	minimapColor = 0;
	textLength = 0;
	actionType = 0;

	m_width = 0;
	m_height = 0;
	m_skip = 0;
	m_blend = 0;
	m_xdiv = 0;
	m_ydiv = 0;
	m_pos = 0;
	m_anim = 0;
	m_sprNum = 0;
	m_sprPtr = NULL;
}

ItemType::~ItemType() {
	if (m_sprPtr)
		delete[] m_sprPtr;
}

// ---- Item ---- //

std::map<unsigned short, ItemType*>			Item::items;

int Item::itemsCount = 0;
int Item::creaturesCount = 0;
int Item::effectsCount = 0;
int Item::distancesCount = 0;

std::string Item::version = "";

MUTEX Item::lockItem;

ItemsSet Item::movingItems;

Item::Item() {
	id = 0;
	count = 0;
	shine = 0;
}

Item::Item(Item* item) {
	id = item->id;
	count = item->count;
	shine = item->shine;
	pos = item->pos;
}

Item::~Item() {
	Item::RemoveMovingItem(this);
	if (light)
		delete_debug(light, M_PLACE);
}


void Item::SetID(unsigned short id) {
	this->id = id;
}

unsigned short Item::GetID() {
	return id;
}

void Item::SetCount(unsigned char count) {
	this->count = count;
}

unsigned char Item::GetCount() {
	return count;
}

void Item::SetShine(unsigned int shine) {
	this->shine = shine;
}

unsigned int Item::GetShine() {
	return shine;
}

void Item::SetLight() {
	ItemType* iType = GetItemType(id);
	if (iType && iType->light && iType->lightColor > 0 && iType->lightRadius > 0) {
		if (light)
			delete_debug(light, M_PLACE);

		light = new(M_PLACE) StaticItemLight(this, iType->lightColor, iType->lightRadius);
	}
	else {
		if (light)
			delete_debug(light, M_PLACE);

		light = NULL;
	}
}


bool Item::IsGround() {
	ItemType* iType = GetItemType(id);

	if (iType && iType->ground)
		return true;

	return false;
}

unsigned char Item::GetTopOrder() {
	ItemType* iType = GetItemType(id);

	if (iType && iType->topItem1) return 1;
	else if (iType && iType->topItem2) return 2;
	else if (iType && iType->topItem3) return 3;

	return 0;
}

ItemType* Item::operator()() {
	ItemType* iType = GetItemType(this->GetID());

	return iType;
}

bool Item::LoadItemsFromDatFIRST(std::string path, std::string ver) {
	FileManager* files = FileManager::fileManager;
	if (!files)
		return false;

	unsigned char* data = files->GetFileData(path);
	if (!data) {
		Logger::AddLog("Item::LoadItemsFromDat()", "Cannot open file!", LOG_WARNING);
		return false;
	}

	unsigned long fsize = files->GetFileSize(path);
	unsigned long pos = 0;

	unsigned short VER1 = ((uint16_t*)(data + pos))[0];
	unsigned short VER2 = ((uint16_t*)(data + pos))[1];
	itemsCount = ((uint16_t*)(data + pos))[2];
	creaturesCount = ((uint16_t*)(data + pos))[3];
	effectsCount = ((uint16_t*)(data + pos))[4];
	distancesCount = ((uint16_t*)(data + pos))[5];
	pos += 12;

	unsigned char ch;
	unsigned short id = 100;

	while(pos < fsize)
	{
		ch = 0;

		ItemType* iType = new ItemType;
		while(ch != 255) {
			ch=data[pos++];

			if (ch == 0xFF)
				break;

			switch(ch) {
				// Ground
				case 0:
					iType->ground = true;
					memcpy(&iType->speed, data + pos, 2);
					pos += 2;
					break;
				// TopItem 1
				case 1:
					iType->topItem1 = true;
					break;
				// TopItem 2 - CanWalkThrough
				case 2:
					iType->topItem2 = true;
					break;
				// TopItem 3
				case 3:
					iType->topItem3 = true;
					break;
				// Container
				case 4:
					iType->container = true;
					break;
				// Stackable
				case 5:
					iType->stackable = true;
					break;
				// Ladder
				case 6:
					iType->ladder = true;
					break;
				// Useable
				case 7:
					iType->useable = true;
					break;
				// Rune
				case 8:
					iType->stackable = true;
					break;
				// Writeable
				case 9:
					iType->writeable1 = true;
					memcpy(&iType->textLength, data + pos, 2);
					pos += 2;
					break;
				// Writeable ed
				case 10:
					iType->writeable2 = true;
					memcpy(&iType->textLength, data + pos, 2);
					pos += 2;
					break;
				// Fluid
				case 11:
					iType->fluid = true;
					break;
				// Multitype
				case 12:
					iType->multiType = true;
					break;
				// Blocking
				case 13:
					iType->blocking = true;
					break;
				// Not moveable
				case 14:
					iType->notMoveable = true;
					break;
				// Block missiles
				case 15:
					iType->blockMissiles = true;
					break;
				// Block path find
				case 16:
					iType->blockPathFind = true;
					break;
				// Pickupable
				case 17:
					iType->pickupable = true;
					break;
				// Wall
				case 18:
					iType->wall = true;
					break;
				// Horizontal
				case 19:
					iType->horizontal = true;
					break;
				// Vertical
				case 20:
					iType->vertical = true;
					break;
				// Rotateable
				case 21:
					iType->rotateable = true;
					break;
				// Light
				case 22:
					iType->light = true;
					memcpy(&iType->lightRadius, data + pos, 2);
					pos += 2;
					memcpy(&iType->lightColor, data + pos, 2);
					pos += 2;
					break;
				// Unknown
				case 23:
					break;
				// Hole
				case 24:
					iType->hole = true;
					break;
				// Offset
				case 25:
					iType->hasOffset = true;
					memcpy(&iType->xOffset, data + pos, 2);
					pos += 2;
					memcpy(&iType->yOffset, data + pos, 2);
					pos += 2;
					break;
				// Height
				case 26:
					iType->hasHeight = true;
					memcpy(&iType->height, data + pos, 2);
					pos += 2;
					break;
				// Draw with height offset
				case 27:
					iType->drawWithHeightOffset = true;
					break;
				// Idle animated
				case 28:
					iType->idleAnimated = true;
					break;
				// Minimap
				case 29:
					iType->minimap = true;
					memcpy(&iType->minimapColor, data + pos, 2);
					pos += 2;
					break;
				// Actioned
				case 30:
					iType->action = true;
					memcpy(&iType->actionType, data + pos, 2);
					pos += 2;
					break;
				// Not floor change
				case 31:
					iType->floorChange = true;
					break;
				case 32:
					break;

				default:
					Logger::AddLog("Item::LoadItemsFromDat()", std::string("Unknown item property! [" + value2str(ch) + "]"), LOG_WARNING);
					break;
			}
		}

		iType->m_width = data[pos++];
		iType->m_height = data[pos++];
		if (iType->m_width * iType->m_height > 1) iType->m_skip = data[pos++];
		else iType->m_skip = 0;
		iType->m_blend = data[pos++];
		iType->m_xdiv = data[pos++];
		iType->m_ydiv = data[pos++];
		iType->m_pos = data[pos++];
		iType->m_anim = data[pos++];

		iType->m_sprNum = iType->m_width * iType->m_height * iType->m_blend * iType->m_xdiv * iType->m_ydiv * iType->m_pos * iType->m_anim;
		iType->m_sprPtr = new unsigned short[iType->m_sprNum];

		for (int i = 0; i < iType->m_sprNum; i++) {
			memcpy(&iType->m_sprPtr[i], data + pos, 2);
			pos += 2;
		}

		items[id] = iType;

		id++;
	}

	delete[] data;

	version = ver;

	return true;
}

bool Item::LoadItemsFromDatSECOND(std::string path, std::string ver) {
	FileManager* files = FileManager::fileManager;
	if (!files)
		return false;

	unsigned char* data = files->GetFileData(path);
	if (!data) {
		Logger::AddLog("Item::LoadItemsFromDat()", "Cannot open file!", LOG_WARNING);
		return false;
	}

	unsigned long fsize = files->GetFileSize(path);
	unsigned long pos = 0;

	unsigned short VER1 = ((uint16_t*)(data + pos))[0];
	unsigned short VER2 = ((uint16_t*)(data + pos))[1];
	itemsCount = ((uint16_t*)(data + pos))[2];
	creaturesCount = ((uint16_t*)(data + pos))[3];
	effectsCount = ((uint16_t*)(data + pos))[4];
	distancesCount = ((uint16_t*)(data + pos))[5];
	pos += 12;

	unsigned char ch;
	unsigned short id = 100;

	while(pos < fsize)
	{
		ch = 0;

		ItemType* iType = new ItemType;
		while(ch != 255) {
			ch=data[pos++];

			if (ch == 0xFF)
				break;

			switch(ch) {
				// Ground
				case 0:
					iType->ground = true;
					memcpy(&iType->speed, data + pos, 2);
					pos += 2;
					break;
				// TopItem 1
				case 1:
					iType->topItem1 = true;
					break;
				// TopItem 2 - CanWalkThrough
				case 2:
					iType->topItem2 = true;
					break;
				// TopItem 3
				case 3:
					iType->topItem3 = true;
					break;
				// Container
				case 4:
					iType->container = true;
					break;
				// Stackable
				case 5:
					iType->stackable = true;
					break;
				// Ladder
				case 6:
					iType->ladder = true;
					break;
				// Useable
				case 7:
					iType->useable = true;
					break;
				// Writeable
				case 8:
					iType->writeable1 = true;
					memcpy(&iType->textLength, data + pos, 2);
					pos += 2;
					break;
				// Writeable ed
				case 9:
					iType->writeable2 = true;
					memcpy(&iType->textLength, data + pos, 2);
					pos += 2;
					break;
				// Fluid
				case 10:
					iType->fluid = true;
					break;
				// Multitype
				case 11:
					iType->multiType = true;
					break;
				// Blocking
				case 12:
					iType->blocking = true;
					break;
				// Not moveable
				case 13:
					iType->notMoveable = true;
					break;
				// Block missiles
				case 14:
					iType->blockMissiles = true;
					break;
				// Block path find
				case 15:
					iType->blockPathFind = true;
					break;
				// Pickupable
				case 16:
					iType->pickupable = true;
					break;
				// Wall
				case 17:
					iType->wall = true;
					break;
				// Horizontal
				case 18:
					iType->horizontal = true;
					break;
				// Vertical
				case 19:
					iType->vertical = true;
					break;
				// Rotateable
				case 20:
					iType->rotateable = true;
					break;
				// Light
				case 21:
					iType->light = true;
					memcpy(&iType->lightRadius, data + pos, 2);
					pos += 2;
					memcpy(&iType->lightColor, data + pos, 2);
					pos += 2;
					break;
				// Unknown
				case 22:
					break;
				// Hole
				case 23:
					iType->hole = true;
					break;
				// Offset
				case 24:
					iType->hasOffset = true;
					memcpy(&iType->xOffset, data + pos, 2);
					pos += 2;
					memcpy(&iType->yOffset, data + pos, 2);
					pos += 2;
					break;
				// Height
				case 25:
					iType->hasHeight = true;
					memcpy(&iType->height, data + pos, 2);
					pos += 2;
					break;
				// Draw with height offset
				case 26:
					iType->drawWithHeightOffset = true;
					break;
				// Idle animated
				case 27:
					iType->idleAnimated = true;
					break;
				// Minimap
				case 28:
					iType->minimap = true;
					memcpy(&iType->minimapColor, data + pos, 2);
					pos += 2;
					break;
				// Actioned
				case 29:
					iType->action = true;
					memcpy(&iType->actionType, data + pos, 2);
					pos += 2;
					break;
				// Not floor change
				case 30:
					iType->floorChange = true;
					break;
				case 31:
					break;

				default:
					Logger::AddLog("Item::LoadItemsFromDat()", std::string("Unknown item property! [" + value2str(ch) + "]"), LOG_WARNING);
					break;
			}
		}

		iType->m_width = data[pos++];
		iType->m_height = data[pos++];
		if (iType->m_width * iType->m_height > 1) iType->m_skip = data[pos++];
		else iType->m_skip = 0;
		iType->m_blend = data[pos++];
		iType->m_xdiv = data[pos++];
		iType->m_ydiv = data[pos++];
		iType->m_pos = data[pos++];
		iType->m_anim = data[pos++];

		iType->m_sprNum = iType->m_width * iType->m_height * iType->m_blend * iType->m_xdiv * iType->m_ydiv * iType->m_pos * iType->m_anim;
		iType->m_sprPtr = new unsigned short[iType->m_sprNum];

		for (int i = 0; i < iType->m_sprNum; i++) {
			memcpy(&iType->m_sprPtr[i], data + pos, 2);
			pos += 2;
		}

		items[id] = iType;

		id++;
	}

	delete[] data;

	version = ver;

	return true;
}

bool Item::LoadItemsFromDatTHIRD(std::string path, std::string ver) {
	FileManager* files = FileManager::fileManager;
	if (!files)
		return false;

	unsigned char* data = files->GetFileData(path);
	if (!data) {
		Logger::AddLog("Item::LoadItemsFromDat()", "Cannot open file!", LOG_WARNING);
		return false;
	}

	unsigned long fsize = files->GetFileSize(path);
	unsigned long pos = 0;

	unsigned short VER1 = ((uint16_t*)(data + pos))[0];
	unsigned short VER2 = ((uint16_t*)(data + pos))[1];
	itemsCount = ((uint16_t*)(data + pos))[2];
	creaturesCount = ((uint16_t*)(data + pos))[3];
	effectsCount = ((uint16_t*)(data + pos))[4];
	distancesCount = ((uint16_t*)(data + pos))[5];
	pos += 12;

	unsigned char ch;
	unsigned short id = 100;

	while(pos < fsize)
	{
		ch = 0;

		ItemType* iType = new ItemType;
		while(ch != 255) {
			ch=data[pos++];

			if (ch == 0xFF)
				break;

			switch(ch) {
				// Ground
				case 0:
					iType->ground = true;
					memcpy(&iType->speed, data + pos, 2);
					pos += 2;
					break;
				// TopItem 1
				case 1:
					iType->topItem1 = true;
					break;
				// TopItem 2 - CanWalkThrough
				case 2:
					iType->topItem2 = true;
					break;
				// TopItem 3
				case 3:
					iType->topItem3 = true;
					break;
				// Container
				case 4:
					iType->container = true;
					break;
				// Stackable
				case 5:
					iType->stackable = true;
					break;
				// Ladder
				case 6:
					iType->ladder = true;
					break;
				// Useable
				case 7:
					iType->useable = true;
					break;
				// Writeable
				case 8:
					iType->writeable1 = true;
					memcpy(&iType->textLength, data + pos, 2);
					pos += 2;
					break;
				// Writeable ed
				case 9:
					iType->writeable2 = true;
					memcpy(&iType->textLength, data + pos, 2);
					pos += 2;
					break;
				// Fluid
				case 10:
					iType->fluid = true;
					break;
				// Multitype
				case 11:
					iType->multiType = true;
					break;
				// Blocking
				case 12:
					iType->blocking = true;
					break;
				// Not moveable
				case 13:
					iType->notMoveable = true;
					break;
				// Block missiles
				case 14:
					iType->blockMissiles = true;
					break;
				// Block path find
				case 15:
					iType->blockPathFind = true;
					break;
				// Pickupable
				case 16:
					iType->pickupable = true;
					break;
				// Wall
				case 17:
					iType->wall = true;
					break;
				// Horizontal
				case 18:
					iType->horizontal = true;
					break;
				// Vertical
				case 19:
					iType->vertical = true;
					break;
				// Rotateable
				case 20:
					iType->rotateable = true;
					break;
				// Light
				case 21:
					iType->light = true;
					memcpy(&iType->lightRadius, data + pos, 2);
					pos += 2;
					memcpy(&iType->lightColor, data + pos, 2);
					pos += 2;
					break;
				// Unknown
				case 22:
					break;
				// Hole
				case 23:
					iType->hole = true;
					break;
				// Offset
				case 24:
					iType->hasOffset = true;
					memcpy(&iType->xOffset, data + pos, 2);
					pos += 2;
					memcpy(&iType->yOffset, data + pos, 2);
					pos += 2;
					break;
				// Height
				case 25:
					iType->hasHeight = true;
					memcpy(&iType->height, data + pos, 2);
					pos += 2;
					break;
				// Draw with height offset
				case 26:
					iType->drawWithHeightOffset = true;
					break;
				// Idle animated
				case 27:
					iType->idleAnimated = true;
					break;
				// Minimap
				case 28:
					iType->minimap = true;
					memcpy(&iType->minimapColor, data + pos, 2);
					pos += 2;
					break;
				// Actioned
				case 29:
					iType->action = true;
					memcpy(&iType->actionType, data + pos, 2);
					pos += 2;
					break;
				// Not floor change
				case 30:
					iType->floorChange = true;
					break;
				// Border corner
				case 31:
					break;
				// Element??
				case 32:
					data[pos++];
					data[pos++];
					break;

				default:
					Logger::AddLog("Item::LoadItemsFromDat()", std::string("Unknown item property! [" + value2str(ch) + "]"), LOG_WARNING);
					break;
			}
		}

		iType->m_width = data[pos++];
		iType->m_height = data[pos++];
		if (iType->m_width * iType->m_height > 1) iType->m_skip = data[pos++];
		else iType->m_skip = 0;
		iType->m_blend = data[pos++];
		iType->m_xdiv = data[pos++];
		iType->m_ydiv = data[pos++];
		iType->m_pos = data[pos++];
		iType->m_anim = data[pos++];
		if (iType->m_anim > 1)
			iType->animated = true;

		iType->m_sprNum = iType->m_width * iType->m_height * iType->m_blend * iType->m_xdiv * iType->m_ydiv * iType->m_pos * iType->m_anim;
		iType->m_sprPtr = new unsigned short[iType->m_sprNum];

		for (int i = 0; i < iType->m_sprNum; i++) {
			memcpy(&iType->m_sprPtr[i], data + pos, 2);
			pos += 2;
		}

		items[id] = iType;

		id++;
	}

	delete[] data;

	version = ver;

	return true;
}

bool Item::ReleaseItems() {
	std::map<unsigned short, ItemType*>::iterator it = items.begin();
	for (it; it != items.end(); it++) {
		ItemType* iType = it->second;
		if (iType)
			delete iType;
	}
	items.clear();

	itemsCount = 0;
	creaturesCount = 0;
	effectsCount = 0;
	distancesCount = 0;

	version = "";
}


ItemType* Item::GetItemType(unsigned short number) {
	std::map<unsigned short, ItemType*>::iterator it = items.find(number);
	if (it != items.end())
		return it->second;

	return NULL;
}


void Item::GetStepOffset(float& x, float& y) {
	x = (this->pos.x - this->oldPos.x) * step;
	y = (this->pos.y - this->oldPos.y) * step;
}


void Item::AddMovingItem(Item* item) {
	LOCKCLASS lockClass(lockItem);

	ItemsSet::iterator it = movingItems.find(item);
	if (it == movingItems.end())
		movingItems.insert(item);
}

void Item::RemoveMovingItem(Item* item) {
	LOCKCLASS lockClass(lockItem);

	ItemsSet::iterator it = movingItems.find(item);
	if (it != movingItems.end())
		movingItems.erase(it);
}

void Item::ClearMovingItems() {
	LOCKCLASS lockClass(lockItem);

	movingItems.clear();
}

void Item::CheckMovingItems(float factor) {
	LOCKCLASS lockClass(lockItem);

	ItemsSet toDelete;

	ItemsSet::iterator it = movingItems.begin();
	for (it; it != movingItems.end(); it++) {
		Item* item = (*it);
		if (item && item->step < 0.0f) {
			item->step += 5 * factor;

			if (item->step >= 0.0f)
				item->step = 0.0f;
		}
		else
			toDelete.insert(item);
	}

	if (!toDelete.empty()) {
		it = toDelete.begin();
		for (it; it != toDelete.end(); it++)
			RemoveMovingItem(*it);
	}
}


void Item::PrintItem(AD2D_Window* gfx, Position pos, float x, float y, float width, float height, int direction) {
	ItemType* iType = Item::GetItemType(GetID());
	if (!iType)
		return;

	int anim = (iType->m_anim > 1 ? (RealTime::getTime() / 500) % iType->m_anim : 0);

	int xOffset = 0;
	int yOffset = 0;
	if (iType->m_xdiv > 1 || iType->m_ydiv > 1) {
		if (iType->stackable) {
			unsigned char count = GetCount();
			int countOffset = 0;
			if (count == 0) countOffset = 0;
			else if (count >= 1 && count < 5) countOffset = count - 1;
			else if (count >= 5 && count < 10) countOffset = 4;
			else if (count >= 10 && count < 25) countOffset = 5;
			else if (count >= 25 && count < 50) countOffset = 6;
			else if (count >= 50) countOffset = 7;
			xOffset = countOffset % iType->m_xdiv;
			yOffset = (countOffset / iType->m_xdiv) % iType->m_ydiv;
		}
		else if (iType->fluid || iType->multiType) {
			int type = FluidMap[GetCount()];
			xOffset = type % iType->m_xdiv;
			yOffset = (type / iType->m_xdiv) % iType->m_ydiv;
		}
		else if (iType->wall) {
			xOffset = direction;
			yOffset = 0;
		}
		else {
			xOffset = pos.x % iType->m_xdiv;
			yOffset = pos.y % iType->m_ydiv;
		}
	}

	AD2D_Image* image = NULL;
	for (int pb = 0; pb < iType->m_blend; pb++)
	for (int px = 0; px < iType->m_width; px++)
	for (int py = 0; py < iType->m_height; py++) {
		float mx = x - px * width;
		float my = y - py * height;

		int offset = px +	(py * iType->m_width) +
							(pb * iType->m_width * iType->m_height) +
							(xOffset * iType->m_width * iType->m_height * iType->m_blend) +
							(yOffset * iType->m_width * iType->m_height * iType->m_blend * iType->m_xdiv) +
							(anim * iType->m_width * iType->m_height * iType->m_blend * iType->m_xdiv * iType->m_ydiv);
		if (offset < iType->m_sprNum)
			image = Sprites::GetSprite(iType->m_sprPtr[offset]);

		if (image) {
			gfx->PutImage(mx, my, mx + width, my + height, *image);
		}
	}
}

void Item::PrintItemShine(AD2D_Window* gfx, Position pos, float x, float y, float width, float height, int direction, unsigned int shine) {
	unsigned char* ch_shine = (unsigned char*)&shine;
	COLOR color((float)(ch_shine[0]) / 255,
				(float)(ch_shine[1]) / 255,
				(float)(ch_shine[2]) / 255, 1.0f);
	int strength = ch_shine[3] % 4;
	int mode = ch_shine[3] / 4;

	if (strength == 0)
		return;

    COLOR currentColor = AD2D_Window::GetColor();

    float rand_shine = 0.0f;
    if ((int)(color.alpha * 1000) != 0)rand_shine = (float)(rand() % (int)(color.alpha * 1000)) / 2000;

	if (mode == 0)
    	gfx->SetBlendFunc(BLEND_SRCALPHA, BLEND_ONE);
	else
		gfx->SetBlendFunc(BLEND_SRCALPHA, BLEND_INVSRCALPHA);

    AD2D_Window::SetColor(color.red, color.green, color.blue, currentColor.alpha * (color.alpha / 2 + rand_shine));
    for (int i = 0; i < 8 * strength; i++) {
        int pt = 1 + i / 8;
        float sx = 0.0f;
        float sy = 0.0f;
        if (i % 8 == 0) { sx = 0.0f;         sy = -0.05f; }
        else if (i % 8 == 1) { sx = 0.03f;   sy = -0.03f; }
        else if (i % 8 == 2) { sx = 0.05f;   sy = 0.0f; }
        else if (i % 8 == 3) { sx = 0.03f;   sy = 0.03f; }
        else if (i % 8 == 4) { sx = 0.0f;    sy = 0.05f; }
        else if (i % 8 == 5) { sx = -0.03f;  sy = 0.03f; }
        else if (i % 8 == 6) { sx = -0.05f;  sy = 0.0f; }
        else if (i % 8 == 7) { sx = -0.03f;  sy = -0.03f; }

        PrintItem(gfx, pos, x + sx * width * pt, y + sy * height * pt, width, height, direction);
    }
    gfx->SetBlendFunc(BLEND_SRCALPHA, BLEND_INVSRCALPHA);

    AD2D_Window::SetColor(currentColor.red, currentColor.green, currentColor.blue, currentColor.alpha);
}


//Lua functions

int Item::LuaSetItemShine(lua_State* L) {
	unsigned int shine = LuaScript::PopNumber(L);
	Item* item = (Item*)((unsigned long)LuaScript::PopNumber(L));

	if (item)
		item->SetShine(shine);

	return 1;
}

int Item::LuaGetItemShine(lua_State* L) {
	Item* item = (Item*)((unsigned long)LuaScript::PopNumber(L));

	unsigned int shine = (item ? item->GetShine() : 0);

	LuaScript::PushNumber(L, shine);
	return 1;
}


void Item::LuaRegisterFunctions(lua_State* L) {
	//setItemShine(itemPtr, shine)
	lua_register(L, "setItemShine", Item::LuaSetItemShine);

	//getItemShine(itemPtr) : shine
	lua_register(L, "getItemShine", Item::LuaGetItemShine);
}
