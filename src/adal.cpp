#ifndef NO_SOUND

#include "adal.h"

#include "allocator.h"

#include "tools.h"


// ---- ADAL_System ---- //

ADAL_System::ADAL_System() {
	alutInit(NULL,0);
	alGetError();

	SetListenerGain(1.0f);
	SetListenerPosition(0.0f, 0.0f, 0.0f);
	SetListenerVelocity(0.0f, 0.0f, 0.0f);
	SetListenerOrientation(0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f);
}

ADAL_System::~ADAL_System() {
	alutExit();
}

void ADAL_System::SetListenerGain(ALfloat g) {
	gain = g;

	alListenerf(AL_GAIN, gain);
}

void ADAL_System::SetListenerPosition(ALfloat x, ALfloat y, ALfloat z) {
	listenerPosition[0] = x;
	listenerPosition[1] = y;
	listenerPosition[2] = z;

	alListenerfv(AL_POSITION, listenerPosition);
}

void ADAL_System::SetListenerVelocity(ALfloat x, ALfloat y, ALfloat z) {
	listenerVelocity[0] = x;
	listenerVelocity[1] = y;
	listenerVelocity[2] = z;

	alListenerfv(AL_VELOCITY, listenerVelocity);
}

void ADAL_System::SetListenerOrientation(ALfloat ax, ALfloat ay, ALfloat az, ALfloat bx, ALfloat by, ALfloat bz) {
	listenerOrientation[0] = ax;
	listenerOrientation[1] = ay;
	listenerOrientation[2] = az;
	listenerOrientation[3] = bx;
	listenerOrientation[4] = by;
	listenerOrientation[5] = bz;

	alListenerfv(AL_ORIENTATION, listenerOrientation);
}


// ---- ADAL_Sample ---- //

ADAL_Sample::ADAL_Sample() {
	buffer = 0;
}

ADAL_Sample::~ADAL_Sample() {
	if (buffer)
		alDeleteBuffers(1, &buffer);
}

void ADAL_Sample::Create(ALenum format, ALsizei size, ALsizei freq, ALvoid* data) {
	if (buffer)
		alDeleteBuffers(1, &buffer);

	alGenBuffers(1, &buffer);
	alBufferData(buffer, format, data, size, freq);
}

void ADAL_Sample::CreateWAV(const char* path) {
	ALenum format;
	ALsizei size;
	ALsizei freq;
	ALvoid* data;
	ALboolean loop;

	alutLoadWAVFile((ALbyte*)path, &format, &data, &size, &freq, &loop);
	Create(format, size, freq, data);
	alutUnloadWAV(format, data, size, freq);
}

void ADAL_Sample::CreateWAV_(unsigned char* data, long int size) {
	if (!data)
		return;

	ALenum format;
	ALsizei wsize;
	ALsizei freq;
	ALvoid* wdata;
	ALboolean loop;

	alutLoadWAVMemory((ALbyte*)buffer, &format, &wdata, &wsize, &freq, &loop);
	Create(format, wsize, freq, wdata);
	alutUnloadWAV(format, wdata, wsize, freq);

	delete[] data;
}

void ADAL_Sample::CreateMP3(const char* path) {
	int error = MPG123_OK;
	mpg123_handle *mh = NULL;

	error = mpg123_init();
	mh = mpg123_new(NULL, &error);
	error = mpg123_open(mh, path);

	int channels;
	int encoding;
	long freq;
	mpg123_getformat(mh, &freq, &channels, &encoding);
	size_t size = mpg123_length(mh);

	size_t blocksize = mpg123_outblock(mh);

	unsigned char* data = new unsigned char[size * channels * 2];
	unsigned char* block = new unsigned char[blocksize];

	size_t done = 0;
	size_t read = 0;
	do {
		error = mpg123_read(mh, block, blocksize, &read);
		if (done + read > size * channels * 2)
			read = (size * channels * 2) - done;
		memcpy(data + done, block, read);
		done += read;
	} while(error != MPG123_DONE);
	size = done;

	delete[] block;

	ALenum format;
	if (channels == 1)
		format = AL_FORMAT_MONO16;
	else
		format = AL_FORMAT_STEREO16;

	Create(format, (ALsizei)size, (ALsizei)freq, (ALvoid*)data);

	delete[] data;

	mpg123_close(mh);
	mpg123_delete(mh);
	mpg123_exit();
}

void ADAL_Sample::CreateMP3_(unsigned char* data, long int size) {
	if (!data)
		return;

	int error = MPG123_OK;
	mpg123_handle *mh = NULL;

	error = mpg123_init();
	mh = mpg123_new(NULL, &error);
	mpg123_open_feed(mh);
	mpg123_feed(mh, data, size);

	size_t bufferSize = 1048576;
	unsigned char* wdata = new unsigned char[bufferSize];
	unsigned char* block = new unsigned char[4096];

	size_t done = 0;
	size_t read = 0;
	do {
		error = mpg123_decode(mh, NULL, 0, block, 4096, &read);
		if (done + read > bufferSize) {
			bufferSize *= 10;
			unsigned char* newData = new unsigned char[bufferSize];
			memcpy(newData, wdata, done);
			delete[] wdata;
			wdata = newData;
		}
		memcpy(wdata + done, block, read);
		done += read;
	} while(error != MPG123_NEED_MORE);

	delete[] block;

	delete[] data;

	int channels;
	int encoding;
	long freq;
	mpg123_getformat(mh, &freq, &channels, &encoding);

	ALenum format;
	if (channels == 1)
		format = AL_FORMAT_MONO16;
	else
		format = AL_FORMAT_STEREO16;

	Create(format, (ALsizei)done, (ALsizei)freq, (ALvoid*)wdata);

	delete[] wdata;

	mpg123_close(mh);
	mpg123_delete(mh);
	mpg123_exit();
}


// ---- ADAL_Sound ---- //

ADAL_Sound::ADAL_Sound() {
	source = 0;

	position[0] = 0.0f;
	position[1] = 0.0f;
	position[2] = 0.0f;

	velocity[0] = 0.0f;
	velocity[1] = 0.0f;
	velocity[2] = 0.0f;
}

ADAL_Sound::~ADAL_Sound() {
	if (source)
		alDeleteSources(1, &source);
}

bool ADAL_Sound::Create(ADAL_Sample sample, ALfloat pitch, ALfloat gain, ALboolean loop) {
	if (source)
		alDeleteSources(1, &source);

	alGenSources(1, &source);
	if (!source)
		return false;

	alSourcei(source, AL_BUFFER, sample.buffer);
	alSourcef(source, AL_PITCH, pitch);
	alSourcef(source, AL_GAIN, gain);
	alSourcefv(source, AL_POSITION, position);
	alSourcefv(source, AL_VELOCITY, velocity);
	alSourcef(source, AL_ROLLOFF_FACTOR, 0.5f);
	alSourcei(source, AL_LOOPING, loop);

	return true;
}

void ADAL_Sound::SetGain(ALfloat gain) {
	alSourcef(source, AL_GAIN, gain);
}

void ADAL_Sound::SetPosition(ALfloat x, ALfloat y, ALfloat z) {
	position[0] = x;
	position[1] = y;
	position[2] = z;

	alSourcefv(source, AL_POSITION, position);
}

void ADAL_Sound::SetVelocity(ALfloat x, ALfloat y, ALfloat z) {
	velocity[0] = x;
	velocity[1] = y;
	velocity[2] = z;

	alSourcefv(source, AL_VELOCITY, velocity);
}

ALuint ADAL_Sound::GetBuffer() {
	ALint buffer;
	alGetSourcei(source, AL_BUFFER, &buffer);

	return buffer;
}

bool ADAL_Sound::IsPlaying() {
	ALint state;
	alGetSourcei(source, AL_SOURCE_STATE, &state);

	if (state == AL_PLAYING)
		return true;

	return false;
}

void ADAL_Sound::Play() {
	alSourcePlay(source);
}

void ADAL_Sound::Stop() {
	alSourceStop(source);
}

void ADAL_Sound::WaitTillPlaying(int checkInterval) {
	ALint state = AL_PLAYING;
	do {
		Sleep(checkInterval);
		alGetSourcei(source, AL_SOURCE_STATE, &state);
	} while(state == AL_PLAYING);
}

#endif //NO_SOUND
