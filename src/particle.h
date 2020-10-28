/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#ifndef __PARTICLE_H_
#define __PARTICLE_H_

#include <list>
#include <map>

#include "ad2d.h"
#include "game.h"
#include "iniloader.h"
#include "logger.h"
#include "mthread.h"
#include "position.h"


class Game;

class Particle;
class Particles;


struct ParticleCondition {
	unsigned short count;
	unsigned short lookType;
	unsigned short liveTime;
	unsigned short speed;
	unsigned char colorR;
	unsigned char colorG;
	unsigned char colorB;
	unsigned char colorA;
};

class Particle {
private:
	unsigned short	lookType;
	Position		pos;

	unsigned short	maxLiveTime;
	unsigned short	liveTime;
	float			speed;
	float			posX;
	float			posY;
	float			direction;
	COLOR			color;

public:
	Particle(ParticleCondition* condition, Position pos);
	~Particle();

	void SetPosition(float x, float y);

	bool CheckParticle();

	friend class Particles;
};


typedef std::map<unsigned short, AD2D_Image*> ParticleImagesMap;
typedef std::map<unsigned short, ParticleCondition*> ParticleConditionsMap;
typedef std::list<Particle*> ParticlesList;
typedef std::map<unsigned char, ParticlesList> ParticlesMap;

class Particles {
private:
	static ParticleImagesMap		images;
	static ParticleConditionsMap	conditions;
	static ParticlesMap				particles;

	static int			threads;
	static MUTEX		lockParticles;

public:
	Particles();
	~Particles();

	void LoadParticles(std::string path, unsigned short count);
	void ReleaseParticles();

	static AD2D_Image* GetParticleImage(unsigned short lookType);
	static ParticleCondition* GetParticleCondition(unsigned short type);

	static void AddParticle(Particle* particle);
	static void RemoveParticle(Particle* particle);
	static void ClearParticles();

	static void CheckParticles(Game* game);
	static void PrintParticles(AD2D_Window* gfx, Position pos, unsigned char level, float x, float y, float zoom);
};

#endif //__PARTICLE_H_
