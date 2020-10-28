/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#include <math.h>

#include "distance.h"

#include "allocator.h"
#include "logger.h"


// ---- Distance ---- //

DistancesList Distance::distances;

int Distance::threads = 0;
MUTEX Distance::lockDistance;


Distance::Distance(unsigned short lookType, Position fromPos, Position toPos) {
	this->pos = fromPos;
	this->newPos = toPos;

	this->maxLiveTime = (int)(sqrt(pow(toPos.x - fromPos.x, 2) + pow(toPos.y - fromPos.y, 2)) * 50);
	this->liveTime = 0;

	ItemType* iType = Item::GetItemType(Item::GetItemsCount() + Item::GetCreaturesCount() + Item::GetEffectsCount() + lookType);
	if (iType && iType->light && iType->lightColor > 0 && iType->lightRadius > 0)
		light = new(M_PLACE) DynamicDistanceLight(this, iType->lightColor, iType->lightRadius);

	this->iType = iType;

	LOCKCLASS lockClass(lockDistance);

	distances.push_back(this);
}

Distance::~Distance() {
	if (light)
		delete_debug(light, M_PLACE);
}

unsigned short Distance::GetMaxLiveTime() {
	return maxLiveTime;
}

unsigned short Distance::GetLiveTime() {
	return liveTime;
}

ItemType* Distance::GetItemType() {
	return iType;
}

Position Distance::GetFromPosition() {
	return pos;
}

Position Distance::GetToPosition() {
	return newPos;
}

void Distance::PrintDistance(AD2D_Window* gfx, float x, float y, float width, float height) {
	if (!iType)
		return;

	int dx = (newPos.x - pos.x);
	int dy = (newPos.y - pos.y);

	int direction;
	if (dx < 0 && dy < 0) direction = 0;
	else if (dx < 0 && dy == 0) direction = 1;
	else if (dx < 0 && dy > 0) direction = 2;
	else if (dx == 0 && dy < 0) direction = 3;
	else if (dx == 0 && dy == 0) direction = 4;
	else if (dx == 0 && dy > 0) direction = 5;
	else if (dx > 0 && dy < 0) direction = 6;
	else if (dx > 0 && dy == 0) direction = 7;
	else if (dx > 0 && dy > 0) direction = 8;

	int xOffset = (direction / iType->m_ydiv) % iType->m_xdiv;
	int yOffset = direction % iType->m_xdiv;

	int anim = (RealTime::getTime() / 50) % iType->m_anim;

	AD2D_Image* image = NULL;
	for (int pb = 0; pb < iType->m_blend; pb++)
	for (int px = 0; px < iType->m_width; px++)
	for (int py = 0; py < iType->m_height; py++) {
		float mx = x - px * width;
		float my = y - py * height;

		int offset = px +	(py * iType->m_width) +
							(pb * iType->m_width * iType->m_height) +
							(xOffset * iType->m_width * iType->m_height * iType->m_blend) +
							(yOffset * iType->m_width * iType->m_height * iType->m_blend * iType->m_xdiv) +
							(anim * iType->m_width * iType->m_height * iType->m_blend * iType->m_xdiv * iType->m_ydiv);
		if (offset < iType->m_sprNum)
			image = Sprites::GetSprite(iType->m_sprPtr[offset]);

		if (image) {
			gfx->PutImage(mx, my, mx + width, my + height, *image);
		}
	}
}


void Distance::RemoveDistance(Distance* dist) {
	LOCKCLASS lockClass(lockDistance);

	if (!dist) {
		Logger::AddLog("Distance::RemoveDistance()", "Pointer to distance is NULL!", LOG_WARNING);
		return;
	}

	DistancesList::iterator it = std::find(distances.begin(), distances.end(), dist);
	if (it != distances.end()) {
		distances.erase(it);
		delete_debug(dist, M_PLACE);
	}
	else
		Logger::AddLog("Distance::RemoveDistance()", "Distance not in list!", LOG_WARNING);
}

void Distance::ClearDistances() {
	LOCKCLASS lockClass(lockDistance);

	DistancesList::iterator it = distances.begin();
	for (it; it != distances.end(); it++) {
		Distance* dist = *it;
		delete_debug(dist, M_PLACE);
	}
	distances.clear();
}

void Distance::CheckDistances(Game* game) {
	if (threads > 0)
		return;

	threads++;

	while(game->GetGameState() == GAME_LOGGEDTOGAME) {
		lockDistance.lock();

		DistancesList toDelete;

		DistancesList::iterator it = distances.begin();
		for (it; it != distances.end(); it++) {
			Distance* dist = *it;
			if (dist) {
				dist->liveTime += 25;
				if (dist->liveTime >= dist->maxLiveTime)
					toDelete.push_back(dist);
			}
		}

		it = toDelete.begin();
		for (it; it != toDelete.end(); it++) {
			Distance* dist = *it;
			RemoveDistance(dist);
		}

		lockDistance.unlock();
		Sleep(25);
	}

	ClearDistances();

	threads--;
}

void Distance::PrintDistances(AD2D_Window* gfx, Position pos, unsigned char z, float x, float y, float width, float height) {
	LOCKCLASS lockClass(lockDistance);

	DistancesList::iterator it = distances.begin();
	for (it; it != distances.end(); it++) {
		Distance* dist = *it;
		if (dist && dist->pos.z == z) {
			int offset = dist->pos.z - pos.z;

			int dx = (dist->newPos.x - dist->pos.x);
			int dy = (dist->newPos.y - dist->pos.y);

			float step = 1.0f - (float)(dist->maxLiveTime - dist->liveTime) / dist->maxLiveTime;
			float px = x + (((float)(dist->pos.x - pos.x + offset) + (float)dx * step) * width);
			float py = y + (((float)(dist->pos.y - pos.y + offset) + (float)dy * step) * height);

			dist->PrintDistance(gfx, px, py, width, height);
		}
	}
}
