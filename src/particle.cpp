/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#include <math.h>

#include "particle.h"

#include "allocator.h"
#include "filemanager.h"
#include "tools.h"


// ---- Particle ---- //

Particle::Particle(ParticleCondition* condition, Position pos) {
	this->pos = pos;

	if (condition) {
		this->lookType = condition->lookType;
		this->maxLiveTime = (rand() % condition->liveTime);
		this->speed = (float)(rand() % condition->speed) / 100;
		this->color.red = (float)condition->colorR / 255;
		this->color.green = (float)condition->colorG / 255;
		this->color.blue = (float)condition->colorB / 255;
		this->color.alpha = (float)condition->colorA / 255;
	}
	else {
		this->lookType = 0;
		this->maxLiveTime = 0;
		this->speed = 0;
	}

	this->liveTime = 0;
	this->direction = (float)(rand() % 36000) / 100;
	this->posX = 0;
	this->posY = 0;
}

Particle::~Particle() { }

void Particle::SetPosition(float x, float y) {
	posX = x;
	posY = y;
}

bool Particle::CheckParticle() {
	if (liveTime < maxLiveTime) {
		posX += speed * sin(direction * 3.1415 / 180) * 2.5f;
		posY -= speed * cos(direction * 3.1415 / 180) * 2.5f;

		liveTime += 25;
		speed *= (0.99f - 0.025f);
	}

	if (liveTime >= maxLiveTime)
		return true;

	return false;
}


// ---- Particles ---- //

ParticleImagesMap		Particles::images;
ParticleConditionsMap	Particles::conditions;
ParticlesMap			Particles::particles;

int Particles::threads = 0;
MUTEX Particles::lockParticles;


Particles::Particles() { }

Particles::~Particles() {
	ReleaseParticles();
}

void Particles::LoadParticles(std::string path, unsigned short count) {
	LOCKCLASS lockClass(lockParticles);

	FileManager* files = FileManager::fileManager;
	if (!files)
		return;

	for (int i = 0; i < count; i++) {
		AD2D_Image* image = NULL;

		std::string filename = path + std::string("particle") + value2str(i) + std::string(".png");
		unsigned char* data = files->GetFileData(filename);
		if (data) {
			image = new(M_PLACE) AD2D_Image;
			image->CreatePNG_(data, files->GetFileSize(filename));
			images[i] = image;
		}
	}

	for (int i = 0; i < count; i++) {
		std::string filename = path + std::string("particle") + value2str(i) + std::string(".ini");
		INILoader iniParticle;
		if (iniParticle.OpenFile(filename)) {
			ParticleCondition* condition = new(M_PLACE) ParticleCondition;

			condition->count = atoi(iniParticle.GetValue("COUNT").c_str());
			condition->lookType = atoi(iniParticle.GetValue("LOOKTYPE").c_str());
			condition->colorR = atoi(iniParticle.GetValue("COLOR", 0).c_str());
			condition->colorG = atoi(iniParticle.GetValue("COLOR", 1).c_str());
			condition->colorB = atoi(iniParticle.GetValue("COLOR", 2).c_str());
			condition->colorA = atoi(iniParticle.GetValue("COLOR", 3).c_str());
			condition->liveTime = atoi(iniParticle.GetValue("LIVETIME").c_str());
			condition->speed = atoi(iniParticle.GetValue("SPEED").c_str());

			conditions[i] = condition;
		}
	}
}

void Particles::ReleaseParticles() {
	LOCKCLASS lockClass(lockParticles);

	if (!images.empty()) {
		ParticleImagesMap::iterator it = images.begin();
		for (it; it != images.end(); it++) {
			AD2D_Image* image = it->second;
			delete_debug(image, M_PLACE);
		}
		images.clear();
	}

	if (!conditions.empty()) {
		ParticleConditionsMap::iterator it = conditions.begin();
		for (it; it != conditions.end(); it++) {
			ParticleCondition* condition = it->second;
			delete_debug(condition, M_PLACE);
		}
		conditions.clear();
	}

	ClearParticles();
}

AD2D_Image* Particles::GetParticleImage(unsigned short lookType) {
	LOCKCLASS lockClass(lockParticles);

	ParticleImagesMap::iterator it = images.find(lookType);
	if (it != images.end())
		return it->second;

	return NULL;
}

ParticleCondition* Particles::GetParticleCondition(unsigned short type) {
	LOCKCLASS lockClass(lockParticles);

	ParticleConditionsMap::iterator it = conditions.find(type);
	if (it != conditions.end())
		return it->second;

	return NULL;
}

void Particles::AddParticle(Particle* particle) {
	LOCKCLASS lockClass(lockParticles);

	if (!particle) {
		Logger::AddLog("Particles::AddParticle()", "Pointer to particle is NULL!", LOG_WARNING);
		return;
	}

	unsigned char level = particle->pos.z;
	ParticlesList::iterator itl = std::find(particles[level].begin(), particles[level].end(), particle);
	if (itl != particles[level].end()) {
		Logger::AddLog("Particles::AddParticle()", "Particle already in map!", LOG_WARNING);
		return;
	}
	else
		particles[level].push_back(particle);
}

void Particles::RemoveParticle(Particle* particle) {
	LOCKCLASS lockClass(lockParticles);

	if (!particle) {
		Logger::AddLog("Particles::RemoveParticle()", "Pointer to particle is NULL!", LOG_WARNING);
		return;
	}

	unsigned char level = particle->pos.z;
	ParticlesMap::iterator it = particles.find(level);
	if (it != particles.end()) {
		ParticlesList::iterator itl = std::find(it->second.begin(), it->second.end(), particle);
		if (itl != it->second.end()) {
			it->second.erase(itl);
			delete_debug(particle, M_PLACE);
		}
		else
			Logger::AddLog("Particles::RemoveParticle()", "Particle not in map!", LOG_WARNING);
	}
	else
		Logger::AddLog("Particles::RemoveParticle()", "Particle not in map!", LOG_WARNING);
}

void Particles::ClearParticles() {
	LOCKCLASS lockClass(lockParticles);

	if (!particles.empty()) {
		ParticlesMap::iterator it = particles.begin();
		for (it; it != particles.end(); it++) {
			ParticlesList::iterator itl = it->second.begin();
			for (itl; itl != it->second.end(); itl++) {
				Particle* particle = *itl;
				delete_debug(particle, M_PLACE);
			}
			it->second.clear();
		}
		particles.clear();
	}
}

void Particles::CheckParticles(Game* game) {
	if (threads > 0)
		return;

	threads++;

	while(game->GetGameState() == GAME_LOGGEDTOGAME) {
		lockParticles.lock();

		ParticlesMap::iterator it = particles.begin();
		for (it; it != particles.end(); it++) {
			ParticlesList toDelete;

			ParticlesList::iterator itl = it->second.begin();
			for (itl; itl != it->second.end(); itl++) {
				Particle* particle = *itl;
				if (particle->CheckParticle())
					toDelete.push_back(particle);
			}

			itl = toDelete.begin();
			for (itl; itl != toDelete.end(); itl++) {
				Particle* particle = *itl;
				RemoveParticle(particle);
			}
		}

		lockParticles.unlock();
		Sleep(25);
	}

	ClearParticles();

	threads--;
}

void Particles::PrintParticles(AD2D_Window* gfx, Position pos, unsigned char level, float x, float y, float zoom) {
	LOCKCLASS lockClass(lockParticles);

	ParticlesMap::iterator it = particles.find(level);
	if (it != particles.end()) {
		COLOR currentColor = AD2D_Window::GetColor();

		ParticlesList::iterator itl = it->second.begin();
		for (itl; itl != it->second.end(); itl++) {
			Particle* particle = *itl;

			int offset = particle->pos.z - pos.z;

			Position relativePos;
			relativePos.x = particle->pos.x - pos.x + offset;
			relativePos.y = particle->pos.y - pos.y + offset;
			relativePos.z = particle->pos.z;

			float px = x + (particle->posX + (relativePos.x * 32 + 16)) * zoom;
			float py = y + (particle->posY + (relativePos.y * 32 + 16)) * zoom;
			float alpha = (float)(particle->maxLiveTime - particle->liveTime) / particle->maxLiveTime;

			AD2D_Window::SetColor(particle->color.red, particle->color.green, particle->color.blue, particle->color.alpha * alpha);
			AD2D_Image* image = GetParticleImage(particle->lookType);
			if (image)
				gfx->RotImage(px, py, particle->direction, zoom, *image);
		}

		AD2D_Window::SetColor(currentColor.red, currentColor.green, currentColor.blue, currentColor.alpha);
	}
}
