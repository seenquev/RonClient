/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#ifndef __DISTANCE_H_
#define __DISTANCE_H_

#include <list>

#include "ad2d.h"
#include "game.h"
#include "item.h"
#include "mthread.h"
#include "position.h"
#include "thing.h"


class Game;

class Distance;


typedef std::list<Distance*> DistancesList;

class Distance : public Thing {
private:
	ItemType*		iType;

	unsigned short	maxLiveTime;
	unsigned short	liveTime;

	static DistancesList		distances;

	static int					threads;
	static MUTEX				lockDistance;

public:
	Distance(unsigned short lookType, Position fromPos, Position toPos);
	~Distance();

	unsigned short GetMaxLiveTime();
	unsigned short GetLiveTime();

	ItemType* GetItemType();
	Position GetFromPosition();
	Position GetToPosition();

	void PrintDistance(AD2D_Window* gfx, float x, float y, float width, float height);

	static void RemoveDistance(Distance* dist);
	static void ClearDistances();

	static void CheckDistances(Game* game);
	static void PrintDistances(AD2D_Window* gfx, Position pos, unsigned char z, float x, float y, float width, float height);
};

#endif //__DISTANCE_H_
