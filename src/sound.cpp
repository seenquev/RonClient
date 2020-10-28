/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#ifndef NO_SOUND

#include "sound.h"

#include "allocator.h"
#include "filemanager.h"
#include "game.h"
#include "luascript.h"
#include "realtime.h"


// ---- SFX_System ---- //

SoundsList SFX_System::sounds;

SoundsCount SFX_System::counter;

int SFX_System::threads = 0;
MUTEX SFX_System::lockSounds;


SFX_System::SFX_System() {
	sfx = new(M_PLACE) ADAL_System;

	backgroundSample = 0;

	musicGain = 1.0f;
	soundGain = 1.0f;
}

SFX_System::~SFX_System() {
	StopBackgroundSound();

	ReleaseSamples();

	if (sfx)
		delete_debug(sfx, M_PLACE);
}

void SFX_System::SetGains(float music, float sound) {
	musicGain = music;
	soundGain = sound;

	SetBackgroundSoundGain(music);
}

void SFX_System::LoadSample(SAMPLE_CLASS sClass, unsigned int id) {
	LOCKCLASS lockClass(lockSounds);

	std::string filename = Game::dataLocation + "/sounds/";
	if (sClass == SAMPLE_BACKGROUND)
		filename += "background" + value2str(id);
	else if (sClass == SAMPLE_EFFECT)
		filename += "effect" + value2str(id);
	else if (sClass == SAMPLE_DISTANCE)
		filename += "distance" + value2str(id);
	else if (sClass == SAMPLE_SYSTEM)
		filename += "system" + value2str(id);
	else if (sClass == SAMPLE_CUSTOM)
		filename += "custom" + value2str(id);

	FileManager* files = FileManager::fileManager;
	if (!files)
		return;

	ADAL_Sample* sample = NULL;

	std::string filenameMP3 = filename + ".mp3";
	std::string filenameWAV = filename + ".wav";
	unsigned char* dataMP3 = files->GetFileData(filenameMP3);
	unsigned char* dataWAV = (!dataMP3 ? files->GetFileData(filenameWAV) : NULL);
	if (dataMP3) {
		sample = new(M_PLACE) ADAL_Sample;
		sample->CreateMP3_(dataMP3, files->GetFileSize(filenameMP3));
	}
	else if (dataWAV) {
		sample = new(M_PLACE) ADAL_Sample;
		sample->CreateWAV_(dataWAV, files->GetFileSize(filenameWAV));
	}

	if (sClass == SAMPLE_BACKGROUND)
		backgroundSamples[id] = sample;
	else if (sClass == SAMPLE_EFFECT)
		effectSamples[id] = sample;
	else if (sClass == SAMPLE_DISTANCE)
		distanceSamples[id] = sample;
	else if (sClass == SAMPLE_SYSTEM)
		systemSamples[id] = sample;
	else if (sClass == SAMPLE_CUSTOM)
		customSamples[id] = sample;
}

void SFX_System::ReleaseSamples() {
	LOCKCLASS lockClass(lockSounds);

	if (!backgroundSamples.empty()) {
		SamplesMap::iterator it = backgroundSamples.begin();
		for (it; it != backgroundSamples.end(); it++) {
			ADAL_Sample* sample = it->second;
			delete_debug(sample, M_PLACE);
		}
		backgroundSamples.clear();
	}

	if (!effectSamples.empty()) {
		SamplesMap::iterator it = effectSamples.begin();
		for (it; it != effectSamples.end(); it++) {
			ADAL_Sample* sample = it->second;
			delete_debug(sample, M_PLACE);
		}
		effectSamples.clear();
	}

	if (!distanceSamples.empty()) {
		SamplesMap::iterator it = distanceSamples.begin();
		for (it; it != distanceSamples.end(); it++) {
			ADAL_Sample* sample = it->second;
			delete_debug(sample, M_PLACE);
		}
		distanceSamples.clear();
	}

	if (!systemSamples.empty()) {
		SamplesMap::iterator it = systemSamples.begin();
		for (it; it != systemSamples.end(); it++) {
			ADAL_Sample* sample = it->second;
			delete_debug(sample, M_PLACE);
		}
		systemSamples.clear();
	}

	if (!customSamples.empty()) {
		SamplesMap::iterator it = customSamples.begin();
		for (it; it != customSamples.end(); it++) {
			ADAL_Sample* sample = it->second;
			delete_debug(sample, M_PLACE);
		}
		customSamples.clear();
	}
}

void SFX_System::PlayBackgroundSound(SAMPLE_CLASS sc, unsigned int id) {
	LOCKCLASS lockClass(lockSounds);

	backgroundSound.Stop();

	if (Game::options.musicGain == 0)
		return;

	bool exist = false;
	ADAL_Sample* sample = NULL;
	SamplesMap::iterator it;
	if (sc == SAMPLE_BACKGROUND) {
		it = backgroundSamples.find(id);
		if (it != backgroundSamples.end()) {
			sample = it->second;
			exist = true;
		}
	}
	else if (sc == SAMPLE_CUSTOM) {
		it = customSamples.find(id);
		if (it != customSamples.end()) {
			sample = it->second;
			exist = true;
		}
	}

	if (!exist) {
		LoadSample(sc, id);
		PlayBackgroundSound(sc, id);
	}
	else if (sample) {
		backgroundSample = id;
		if (backgroundSound.Create(*sample, 1.0f, musicGain, AL_TRUE));
			backgroundSound.Play();
	}
}

void SFX_System::StopBackgroundSound() {
	LOCKCLASS lockClass(lockSounds);

	backgroundSound.Stop();
}

void SFX_System::SetBackgroundSoundGain(float gain) {
	LOCKCLASS lockClass(lockSounds);

	backgroundSound.SetGain(gain);
}

unsigned int SFX_System::GetBackgroundSample() {
	return backgroundSample;
}

void SFX_System::PlaySystemSound(SAMPLE_CLASS sc, unsigned int id) {
	LOCKCLASS lockClass(lockSounds);

	if (Game::options.soundGain == 0)
		return;

	bool exist = false;
	ADAL_Sample* sample = NULL;
	SamplesMap::iterator it;
	if (sc == SAMPLE_SYSTEM) {
		it = systemSamples.find(id);
		if (it != systemSamples.end()) {
			sample = it->second;
			exist = true;
		}
	}

	if (!exist) {
		LoadSample(sc, id);
		PlaySystemSound(sc, id);
	}
	else if (sample) {
		ADAL_Sound* sound = new(M_PLACE) ADAL_Sound;
		if (sound->Create(*sample, 1.0f, soundGain, AL_FALSE)) {
			if (AddSound(sound))
				sound->Play();
			else
				delete_debug(sound, M_PLACE);
		}
		else
			delete_debug(sound, M_PLACE);
	}
}

void SFX_System::PlayGameSound(SAMPLE_CLASS sc, unsigned int id, ALfloat x, ALfloat y, ALfloat z) {
	LOCKCLASS lockClass(lockSounds);

	if (Game::options.soundGain == 0)
		return;

	bool exist = false;
	ADAL_Sample* sample = NULL;
	SamplesMap::iterator it;
	if (sc == SAMPLE_EFFECT) {
		it = effectSamples.find(id);
		if (it != effectSamples.end()) {
			sample = it->second;
			exist = true;
		}
	}
	else if (sc == SAMPLE_DISTANCE) {
		it = distanceSamples.find(id);
		if (it != distanceSamples.end()) {
			sample = it->second;
			exist = true;
		}
	}

	if (!exist) {
		LoadSample(sc, id);
		PlayGameSound(sc, id, x, y, z);
	}
	else if (sample) {
		ADAL_Sound* sound = new(M_PLACE) ADAL_Sound;
		if (sound->Create(*sample, 1.0f, soundGain, AL_FALSE)) {
			if (AddSound(sound)) {
				sound->SetPosition(x, y, z);
				sound->Play();
			}
			else
				delete_debug(sound, M_PLACE);
		}
		else
			delete_debug(sound, M_PLACE);
	}
}

void SFX_System::PlayCustomSound(SAMPLE_CLASS sc, unsigned int id, float volume, float offset) {
	LOCKCLASS lockClass(lockSounds);

	if (Game::options.soundGain == 0)
		return;

	bool exist = false;
	ADAL_Sample* sample = NULL;
	SamplesMap::iterator it;
	if (sc == SAMPLE_BACKGROUND) {
		it = backgroundSamples.find(id);
		if (it != backgroundSamples.end()) {
			sample = it->second;
			exist = true;
		}
	}
	else if (sc == SAMPLE_CUSTOM) {
		it = customSamples.find(id);
		if (it != customSamples.end()) {
			sample = it->second;
			exist = true;
		}
	}

	if (!exist) {
		LoadSample(sc, id);
		PlayCustomSound(sc, id, volume, offset);
	}
	else if (sample) {
		ADAL_Sound* sound = new(M_PLACE) ADAL_Sound;
		if (sound->Create(*sample, 1.0f, soundGain * volume, AL_FALSE)) {
			if (AddSound(sound)) {
				sound->SetPosition(offset * 5, 0.0f, 0.0f);
				sound->Play();
			}
			else
				delete_debug(sound, M_PLACE);
		}
		else
			delete_debug(sound, M_PLACE);
	}
}


bool SFX_System::AddSound(ADAL_Sound* sound) {
	LOCKCLASS lockClass(lockSounds);

	time_lt time = RealTime::getTime();
	unsigned int buffer = sound->GetBuffer();
	if (time < counter[buffer] + 50)
		return false;

	sounds.push_back(sound);
	counter[buffer] = time;

	return true;
}

void SFX_System::RemoveSound(ADAL_Sound* sound) {
	LOCKCLASS lockClass(lockSounds);

	SoundsList::iterator it = std::find(sounds.begin(), sounds.end(), sound);
	if (it != sounds.end()) {
		unsigned int buffer = sound->GetBuffer();
		counter[buffer] = 0;

		sounds.erase(it);
		delete_debug(sound, M_PLACE);
	}
}

void SFX_System::ClearSounds() {
	LOCKCLASS lockClass(lockSounds);

	SoundsList::iterator it = sounds.begin();
	for (it; it != sounds.end(); it++) {
		ADAL_Sound* sound = *it;
		if (sound) {
			sound->Stop();
			delete_debug(sound, M_PLACE);
		}
	}

	sounds.clear();
	counter.clear();
}

void SFX_System::CheckSounds(Game* game) {
	if (threads > 0)
		return;

	threads++;

	while(game->GetGameState() == GAME_LOGGEDTOGAME) {
		lockSounds.lock();

		SoundsList toDelete;

		SoundsList::iterator it = sounds.begin();
		for (it; it != sounds.end(); it++) {
			ADAL_Sound* sound = *it;
			if (sound && !sound->IsPlaying())
				toDelete.push_back(sound);
		}

		it = toDelete.begin();
		for (it; it != toDelete.end(); it++) {
			ADAL_Sound* sound = *it;
			if (sound)
				RemoveSound(sound);
		}

		lockSounds.unlock();

		Sleep(100);
	}

	ClearSounds();

	threads--;
}


//Lua functions

int SFX_System::LuaPlayBackgroundSound(lua_State* L) {
	int params = lua_gettop(L);

	SAMPLE_CLASS sampleClass = SAMPLE_BACKGROUND;
	if (params == 2)
		sampleClass = (SAMPLE_CLASS)LuaScript::PopNumber(L);
	int id = LuaScript::PopNumber(L);

	Game* game = Game::game;
	SFX_System* sfx = (game ? game->GetSFX() : NULL);
	if (sfx)
		sfx->PlayBackgroundSound(sampleClass, id);

	return 1;
}

int SFX_System::LuaStopBackgroundSound(lua_State* L) {
	Game* game = Game::game;
	SFX_System* sfx = (game ? game->GetSFX() : NULL);
	if (sfx)
		sfx->StopBackgroundSound();

	return 1;
}

int SFX_System::LuaPlaySystemSound(lua_State* L) {
	int id = LuaScript::PopNumber(L);

	Game* game = Game::game;
	SFX_System* sfx = (game ? game->GetSFX() : NULL);
	if (sfx)
		sfx->PlaySystemSound(SAMPLE_SYSTEM, id);

	return 1;
}

int SFX_System::LuaPlayEffectSound(lua_State* L) {
	float z = (float)LuaScript::PopNumber(L);
	float y = (float)LuaScript::PopNumber(L);
	float x = (float)LuaScript::PopNumber(L);
	int id = LuaScript::PopNumber(L);

	Game* game = Game::game;
	SFX_System* sfx = (game ? game->GetSFX() : NULL);
	if (sfx)
		sfx->PlayGameSound(SAMPLE_EFFECT, id, x, y, z);

	return 1;
}

int SFX_System::LuaPlayDistanceSound(lua_State* L) {
	float z = (float)LuaScript::PopNumber(L);
	float y = (float)LuaScript::PopNumber(L);
	float x = (float)LuaScript::PopNumber(L);
	int id = LuaScript::PopNumber(L);

	Game* game = Game::game;
	SFX_System* sfx = (game ? game->GetSFX() : NULL);
	if (sfx)
		sfx->PlayGameSound(SAMPLE_DISTANCE, id, x, y, z);

	return 1;
}

int SFX_System::LuaPlayCustomSound(lua_State* L) {
	float offset = (float)LuaScript::PopNumber(L);
	float volume = (float)LuaScript::PopNumber(L);
	int id = LuaScript::PopNumber(L);

	Game* game = Game::game;
	SFX_System* sfx = (game ? game->GetSFX() : NULL);
	if (sfx)
		sfx->PlayCustomSound(SAMPLE_CUSTOM, id, volume, offset);

	return 1;
}


void SFX_System::LuaRegisterFunctions(lua_State* L) {
	lua_pushinteger(L, SAMPLE_BACKGROUND);
	lua_setglobal(L, "SAMPLE_BACKGROUND");
	lua_pushinteger(L, SAMPLE_EFFECT);
	lua_setglobal(L, "SAMPLE_EFFECT");
	lua_pushinteger(L, SAMPLE_DISTANCE);
	lua_setglobal(L, "SAMPLE_DISTANCE");
	lua_pushinteger(L, SAMPLE_SYSTEM);
	lua_setglobal(L, "SAMPLE_SYSTEM");
	lua_pushinteger(L, SAMPLE_CUSTOM);
	lua_setglobal(L, "SAMPLE_CUSTOM");

	//playBackgroundSound(id, sampleClass = SAMPLE_BACKGROUND)
	lua_register(L, "playBackgroundSound", SFX_System::LuaPlayBackgroundSound);

	//stopBackgroundSound()
	lua_register(L, "stopBackgroundSound", SFX_System::LuaStopBackgroundSound);

	//playSystemSound(id)
	lua_register(L, "playSystemSound", SFX_System::LuaPlaySystemSound);

	//playEffectSound(id, x, y, z)
	lua_register(L, "playEffectSound", SFX_System::LuaPlayEffectSound);

	//playDistanceSound(id, x, y, z)
	lua_register(L, "playDistanceSound", SFX_System::LuaPlayDistanceSound);

	//playCustomSound(id, volume, offset)
	lua_register(L, "playCustomSound", SFX_System::LuaPlayCustomSound);
}

#endif //NO_SOUND
