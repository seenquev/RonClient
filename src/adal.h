#ifndef NO_SOUND

#ifndef _ADAL_H
#define _ADAL_H

#include <al\al.h>
#include <al\alc.h>
#include <al\alu.h>
#include <al\alut.h>
#include <map>
#include <mpg123.h>
#include <string>
#include <windows.h>

class ADAL_Sample;
class ADAL_Sound;

class ADAL_System {
private:
	ALfloat gain;
	ALfloat listenerPosition[3];
	ALfloat listenerVelocity[3];
	ALfloat	listenerOrientation[6];

public:
	ADAL_System();
	~ADAL_System();

	void SetListenerGain(ALfloat gain);
	void SetListenerPosition(ALfloat x, ALfloat y, ALfloat z);
	void SetListenerVelocity(ALfloat x, ALfloat y, ALfloat z);
	void SetListenerOrientation(ALfloat ax, ALfloat ay, ALfloat az, ALfloat bx, ALfloat by, ALfloat bz);
};


class ADAL_Sample {
private:
	ALuint buffer;

public:
	ADAL_Sample();
	~ADAL_Sample();

	void Create(ALenum format, ALsizei size, ALsizei freq, ALvoid* data);
	void CreateWAV(const char* path);
	void CreateWAV_(unsigned char* data, long int size);
	void CreateMP3(const char* path);
	void CreateMP3_(unsigned char* data, long int size);

	friend class ADAL_Sound;
};


class ADAL_Sound {
private:
	ALuint source;

	ALfloat position[3];
	ALfloat velocity[3];

public:
	ADAL_Sound();
	~ADAL_Sound();

	bool Create(ADAL_Sample sample, ALfloat pitch, ALfloat gain, ALboolean loop);

	void SetGain(ALfloat gain);
	void SetPosition(ALfloat x, ALfloat y, ALfloat z);
	void SetVelocity(ALfloat x, ALfloat y, ALfloat z);

	ALuint GetBuffer();
	bool IsPlaying();

	void Play();
	void Stop();

	void WaitTillPlaying(int checkInterval);
};

#endif

#endif //NO_SOUND
