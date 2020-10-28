/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#ifndef __MAGICEFFECT_H_
#define __MAGICEFFECT_H_

#include <list>

#include "ad2d.h"
#include "game.h"
#include "item.h"
#include "mthread.h"
#include "position.h"
#include "thing.h"


class Game;

class MagicEffect;


typedef std::list<MagicEffect*> MagicEffectsList;

class MagicEffect : public Thing {
private:
	ItemType*		iType;
	unsigned short	anim;

	static MagicEffectsList		magicEffects;

	static int					threads;
	static MUTEX				lockMagicEffect;

public:
	MagicEffect();
	~MagicEffect();

	bool Create(unsigned short lookType, Position pos);

	ItemType* GetItemType();
	Position GetPosition();

	void PrintMagicEffect(AD2D_Window* gfx, Position pos, float x, float y, float width, float height);

	static void RemoveMagicEffect(MagicEffect* me);
	static void ClearMagicEffects();

	static void CheckMagicEffects(Game* game);
	static void PrintMagicEffects(AD2D_Window* gfx, Position pos, float x, float y, float width, float height);
};

#endif //__MAGICEFFECT_H_
