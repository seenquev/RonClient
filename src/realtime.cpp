/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#include "realtime.h"

#include <iostream>
#include <sys/types.h>
#include <sys/timeb.h>

#include "luascript.h"


time_lt	RealTime::time = 0;
MUTEX	RealTime::lockRealTime;


// ---- FPS ---- //

FPS::FPS() {
	length = 0;
}

FPS::~FPS() {
	data.clear();
}

void FPS::Create(unsigned short size) {
	length = size;
}

void FPS::Insert(float fps) {
	if (data.size() >= length)
		data.erase(data.begin());
	data.push_back((unsigned int)(fps * 1000));
}

float FPS::Return() {
	if (data.size() == 0)
		return 0;

	unsigned int fps = 0;
	for (std::list<unsigned int>::iterator it = data.begin(); it != data.end(); it++)
		fps += *it;
	fps /= data.size();

	return float((float)fps / 1000);
}

float FPS::GetPeriod() {
	return float(1000.0f / Return());
}


// ---- RealTime ---- //

RealTime::RealTime() {
	currTime = 0;
	prevTime = 0;

	Calculate();
}

RealTime::~RealTime() {
}

void RealTime::Calculate() {
	_timeb t;
	_ftime(&t);

	prevTime = currTime;
	currTime = (time_lt)t.time * 1000 + (time_t)t.millitm;

	factor = float(currTime - prevTime) / float(1000);
	fps = 1000.0f / (float)(currTime - prevTime);

	LOCKCLASS lockClass(lockRealTime);

	RealTime::time = currTime;
}

int RealTime::getPeriod() {
	time_lt period = currTime - prevTime;
	return (int)period;
}

float RealTime::getFactor() {
	return factor;
}

float RealTime::getFPS() {
	return fps;
}

time_lt RealTime::getTime() {
	LOCKCLASS lockClass(lockRealTime);

	return RealTime::time;
}


//Lua functions

int RealTime::LuaGetTime(lua_State* L) {
	LuaScript::PushNumber(L, getTime());
	return 1;
}


void RealTime::LuaRegisterFunctions(lua_State* L) {
	//getTime()
	lua_register(L, "getTime", RealTime::LuaGetTime);
}
