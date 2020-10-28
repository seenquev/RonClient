/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#ifndef __LIGHT_H_
#define __LIGHT_H_

#include <iostream>
#include <list>

#include "ad2d.h"
#include "mthread.h"
#include "position.h"


class Item;
class Creature;
class MagicEffect;
class Distance;

class Map;

class Lights;


class Light {
private:
	unsigned char	color;
	unsigned char	level;
	
public:
	Light(unsigned char color, unsigned char level);
	virtual ~Light();
	
	friend class Lights;
};


class StaticItemLight : public Light {
private:
	Item*	item;
	
public:
	StaticItemLight(Item* item, unsigned char color, unsigned char level);
	~StaticItemLight();
	
	friend class Lights;
};


class StaticMagicEffectLight : public Light {
private:
	MagicEffect*	magicEffect;
	
public:
	StaticMagicEffectLight(MagicEffect* magicEffect, unsigned char color, unsigned char level);
	~StaticMagicEffectLight();
	
	friend class Lights;
};


class DynamicCreatureLight : public Light {
private:
	Creature*		creature;
	
public:
	DynamicCreatureLight(Creature* creature, unsigned char color, unsigned char level);
	~DynamicCreatureLight();
	
	friend class Lights;
};


class DynamicDistanceLight : public Light {
private:
	Distance*		distance;
	
public:
	DynamicDistanceLight(Distance* distance, unsigned char color, unsigned char level);
	~DynamicDistanceLight();
	
	friend class Lights;
};


typedef unsigned char LightMap[19][15][3];

typedef std::list<Light*> LightsList;

class Lights {
private:
	static unsigned char	globalColor;
	static unsigned char	globalLevel;
	
	static LightMap		staticMap;
	static LightMap		dynamicMap;
	
	static LightsList	staticLights;
	static LightsList	dynamicLights;
	
	static MUTEX		lockLights;
	
public:
	static void SetGlobalColor(unsigned char color);
	static unsigned char GetGlobalColor();
	
	static void SetGlobalLevel(unsigned char level);
	static unsigned char GetGlobalLevel();
	
	static void AddStaticLight(Light* light);
	static void RemoveStaticLight(Light* light);
	
	static void AddDynamicLight(Light* light);
	static void RemoveDynamicLight(Light* light);
	
	static int GetStaticLightsCount();
	static int GetDynamicLightsCount();
	
	static void UpdateStaticLightMap(Map* map);
	static void UpdateDynamicLightMap(Map* map);
	
	static void PrintLightMap(AD2D_Window* gfx, unsigned char z, float x, float y, float width, float height);
};

#endif //__LIGHT_H_
