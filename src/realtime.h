/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#ifndef __REALTIME_H_
#define __REALTIME_H_

#include <iostream>
#include <list>
#include <windows.h>

#include "luascript.h"
#include "mthread.h"

typedef unsigned long long int time_lt;

class FPS {
private:
	unsigned short			length;
	std::list<unsigned int>	data;

public:
	FPS();
	~FPS();

	void Create(unsigned short size);
	void Insert(float fps);
	float Return();
	float GetPeriod();
};

class RealTime {
private:
	time_lt		currTime;
	time_lt		prevTime;
	float		factor;
	float		fps;

	static time_lt	time;
	static MUTEX	lockRealTime;

public:
	RealTime();
	~RealTime();

	void Calculate();
	int getPeriod();
	float getFactor();
	float getFPS();

	static time_lt getTime();

	//Lua functions
	static int LuaGetTime(lua_State* L);

	static void LuaRegisterFunctions(lua_State* L);
};

#endif //__REALTIME_H_
