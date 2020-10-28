/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#ifndef __THING_H_
#define __THING_H_

#include "light.h"
#include "logger.h"
#include "position.h"


class Thing {
public:
	Light*			light;
	Position		newPos;
	Position		pos;
	Position		oldPos;

	float			step;

public:
	Thing();
	virtual ~Thing();
};

#endif //__THING_H_
