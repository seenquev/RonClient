/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#ifndef __ITEM_H_
#define __ITEM_H_

#include <map>
#include <set>
#include <string>

#include "ad2d.h"
#include "logger.h"
#include "luascript.h"
#include "mthread.h"
#include "position.h"
#include "thing.h"


typedef std::set<Item*> ItemsSet;


struct ThingData {
	Position fromPos;
	Position toPos;
	unsigned char fromStackPos;
	unsigned short itemId;
	unsigned char count;
	unsigned char* scroll;

	ThingData() { }
	ThingData(ThingData* thing) { *this = *thing; }
};

struct ColorTemplate {
	AD2D_Image* all;
	AD2D_Image* head;
	AD2D_Image* body;
	AD2D_Image* legs;
	AD2D_Image* feet;

	ColorTemplate() {
		all = NULL;
		head = NULL;
		body = NULL;
		legs = NULL;
		feet = NULL;
	}
};

class Sprites {
private:
	static unsigned char* spritesData;

	static std::map<unsigned short, unsigned long>	handlers;
	static std::map<unsigned short, AD2D_Image*>	sprites;
	static std::map<unsigned short, ColorTemplate*>	templates;

public:
	static std::string version;

public:
	static bool LoadSpritesFromSpr(std::string path, std::string ver);
	static bool LoadSprite(unsigned short number, bool colorTemplate = false, bool updateOutfit = false);
	static bool UpdateOutfit(AD2D_Image* outfit, ColorTemplate* ct, ColorTemplate* ct_outfit);
	static void CreateColorTemplate(unsigned char* data, ColorTemplate* ct);
	static void ReleaseSpritesData();
	static void ReleaseSprites();

	static AD2D_Image* GetSprite(unsigned short number);
	static ColorTemplate* GetColorTemplate(unsigned short number, AD2D_Image* outfit = NULL, ColorTemplate* ct = NULL);
};

class ItemType {
public:
	bool	ground;
	bool	topItem1;
	bool	topItem2;
	bool	topItem3;
	bool	container;
	bool	stackable;
	bool	ladder;
	bool	useable;
	bool	writeable1;
	bool	writeable2;
	bool	fluid;
	bool	multiType;
	bool	blocking;
	bool	notMoveable;
	bool	blockMissiles;
	bool	blockPathFind;
	bool	pickupable;
	bool	wall;
	bool	horizontal;
	bool	vertical;
	bool	rotateable;
	bool	light;
	bool	hole;
	bool	hasOffset;
	bool	hasHeight;
	bool	drawWithHeightOffset;
	bool	idleAnimated;
	bool	minimap;
	bool	action;
	bool	floorChange;
	bool	animated;

	unsigned short	speed;
	unsigned short	height;
	unsigned short	xOffset;
	unsigned short	yOffset;
	unsigned short	lightColor;
	unsigned short	lightRadius;
	unsigned short	minimapColor;
	unsigned short	textLength;
	unsigned short	actionType;

	unsigned char	m_width;
	unsigned char	m_height;
	unsigned char	m_skip;
	unsigned char	m_blend;
	unsigned char	m_xdiv;
	unsigned char	m_ydiv;
	unsigned char	m_pos;
	unsigned char	m_anim;
	unsigned short	m_sprNum;
	unsigned short*	m_sprPtr;

public:
	ItemType();
	~ItemType();

	friend class Item;
};

class Item : public Thing {
public:
	unsigned short	id;
	unsigned char	count;
	unsigned int	shine;

private:
	static std::map<unsigned short, ItemType*>		items;

	static int itemsCount;
	static int creaturesCount;
	static int effectsCount;
	static int distancesCount;

public:
	static MUTEX			lockItem;

	static ItemsSet			movingItems;

	static std::string		version;

public:
	Item();
	Item(Item* item);
	virtual ~Item();

	void SetID(unsigned short id);
	unsigned short GetID();
	void SetCount(unsigned char count);
	unsigned char GetCount();
	void SetShine(unsigned int shine);
	unsigned int GetShine();
	void SetLight();

	bool IsGround();
	unsigned char GetTopOrder();

	ItemType* operator()();

	static bool LoadItemsFromDatFIRST(std::string path, std::string ver);
	static bool LoadItemsFromDatSECOND(std::string path, std::string ver);
	static bool LoadItemsFromDatTHIRD(std::string path, std::string ver);
	static bool ReleaseItems();

	static ItemType* GetItemType(unsigned short number);

	static int GetItemsCount() { return itemsCount; };
	static int GetCreaturesCount() { return creaturesCount; };
	static int GetEffectsCount() { return effectsCount; };
	static int GetDistancesCount() { return distancesCount; };

	void GetStepOffset(float& x, float& y);

	static void AddMovingItem(Item* item);
	static void RemoveMovingItem(Item* item);
	static void ClearMovingItems();

	static void CheckMovingItems(float factor);

	void PrintItem(AD2D_Window* gfx, Position pos, float x, float y, float width, float height, int direction);
	void PrintItemShine(AD2D_Window* gfx, Position pos, float x, float y, float width, float height, int direction, unsigned int shine);

	//Lua  functions
	static int LuaSetItemShine(lua_State* L);
	static int LuaGetItemShine(lua_State* L);

	static void LuaRegisterFunctions(lua_State* L);
};

#endif //__ITEM_H_
