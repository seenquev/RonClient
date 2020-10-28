/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#ifndef NO_SOUND

#ifndef __SOUNDS_H_
#define __SOUNDS_H_

#include <map>

#include "adal.h"
#include "luascript.h"
#include "mthread.h"
#include "realtime.h"
#include "tools.h"

class Game;


enum SAMPLE_CLASS {
	SAMPLE_BACKGROUND = 0,
	SAMPLE_EFFECT = 1,
	SAMPLE_DISTANCE = 2,
	SAMPLE_SYSTEM = 3,
	SAMPLE_CUSTOM = 4,
};

typedef std::map<unsigned int, time_lt> SoundsCount;
typedef std::map<unsigned int, ADAL_Sample*> SamplesMap;
typedef std::list<ADAL_Sound*> SoundsList;

class SFX_System {
private:
	ADAL_System* sfx;

	float	musicGain;
	float	soundGain;

	SamplesMap backgroundSamples;
	SamplesMap effectSamples;
	SamplesMap distanceSamples;
	SamplesMap systemSamples;
	SamplesMap customSamples;

	unsigned int backgroundSample;
	ADAL_Sound backgroundSound;

	static SoundsList sounds;

public:
	static SoundsCount counter;

	static int threads;
	static MUTEX lockSounds;

public:
	SFX_System();
	~SFX_System();

	void SetGains(float music, float sound);

	void LoadSample(SAMPLE_CLASS sClass, unsigned int id);
	void ReleaseSamples();

	void PlayBackgroundSound(SAMPLE_CLASS sc, unsigned int id);
	void StopBackgroundSound();
	void SetBackgroundSoundGain(float gain);
	unsigned int GetBackgroundSample();

	void PlaySystemSound(SAMPLE_CLASS sc, unsigned int id);
	void PlayGameSound(SAMPLE_CLASS sc, unsigned int id, float x, float y, float z);
	void PlayCustomSound(SAMPLE_CLASS sc, unsigned int id, float volume, float offset);

	static bool AddSound(ADAL_Sound* sound);
	static void RemoveSound(ADAL_Sound* sound);
	static void ClearSounds();
	static void CheckSounds(Game* game);

	//Lua functions
	static int LuaPlayBackgroundSound(lua_State* L);
	static int LuaStopBackgroundSound(lua_State* L);
	static int LuaPlaySystemSound(lua_State* L);
	static int LuaPlayEffectSound(lua_State* L);
	static int LuaPlayDistanceSound(lua_State* L);
	static int LuaPlayCustomSound(lua_State* L);

	static void LuaRegisterFunctions(lua_State* L);
};

#endif //__SOUNDS_H_

#endif //NO_SOUND
