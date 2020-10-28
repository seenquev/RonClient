/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#include "thing.h"

#include "allocator.h"
#include "magiceffect.h"


// ---- Thing ---- //

Thing::Thing() {
	light = NULL;
	newPos = Position(0, 0, 0);
	pos = Position(0, 0, 0);
	oldPos = Position(0, 0, 0);

	step = 0.0f;
}

Thing::~Thing() { }
