/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#include "light.h"

#include "allocator.h"
#include "creature.h"
#include "distance.h"
#include "item.h"
#include "magiceffect.h"
#include "map.h"
#include "logger.h"
#include "tools.h"


// ---- Light ---- //

Light::Light(unsigned char color, unsigned char level) {
	this->color = color;
	this->level = level;
}

Light::~Light() { }


// ---- StaticItemLight ---- //

StaticItemLight::StaticItemLight(Item* item, unsigned char color, unsigned char level) : Light(color, level) {
	this->item = item;

	Lights::AddStaticLight(this);
}

StaticItemLight::~StaticItemLight() {
	Lights::RemoveStaticLight(this);
}


// ---- StaticMagicEffectLight ---- //

StaticMagicEffectLight::StaticMagicEffectLight(MagicEffect* magicEffect, unsigned char color, unsigned char level) : Light(color, level) {
	this->magicEffect = magicEffect;

	Lights::AddStaticLight(this);
}

StaticMagicEffectLight::~StaticMagicEffectLight() {
	Lights::RemoveStaticLight(this);
}


// ---- DynamicCreatureLight ---- //

DynamicCreatureLight::DynamicCreatureLight(Creature* creature, unsigned char color, unsigned char level) : Light(color, level) {
	this->creature = creature;

	Lights::AddDynamicLight(this);
}

DynamicCreatureLight::~DynamicCreatureLight() {
	Lights::RemoveDynamicLight(this);
}


// ---- DynamicDistanceLight ---- //

DynamicDistanceLight::DynamicDistanceLight(Distance* distance, unsigned char color, unsigned char level) : Light(color, level) {
	this->distance = distance;

	Lights::AddDynamicLight(this);
}

DynamicDistanceLight::~DynamicDistanceLight() {
	Lights::RemoveDynamicLight(this);
}


// ---- Lights ---- //

unsigned char Lights::globalColor;
unsigned char Lights::globalLevel;

LightMap Lights::staticMap;
LightMap Lights::dynamicMap;

LightsList Lights::staticLights;
LightsList Lights::dynamicLights;

MUTEX Lights::lockLights;


void Lights::SetGlobalColor(unsigned char color) {
	LOCKCLASS lockClass(lockLights);

	globalColor = color;
}

unsigned char Lights::GetGlobalColor() {
	LOCKCLASS lockClass(lockLights);

	return globalColor;
}

void Lights::SetGlobalLevel(unsigned char level) {
	LOCKCLASS lockClass(lockLights);

	globalLevel = level;
}

unsigned char Lights::GetGlobalLevel() {
	LOCKCLASS lockClass(lockLights);

	return globalLevel;
}

void Lights::AddStaticLight(Light* light) {
	LOCKCLASS lockClass(lockLights);

	LightsList::iterator it = std::find(staticLights.begin(), staticLights.end(), light);
	if (it == staticLights.end())
		staticLights.push_back(light);
	else
		Logger::AddLog("Lights::AddStaticLight()", "Static light already on list!", LOG_ERROR);
}

void Lights::RemoveStaticLight(Light* light) {
	LOCKCLASS lockClass(lockLights);

	LightsList::iterator it = std::find(staticLights.begin(), staticLights.end(), light);
	if (it != staticLights.end())
		staticLights.erase(it);
	else
		Logger::AddLog("Lights::RemoveStaticLight()", "Static light not on list!", LOG_ERROR);
}


void Lights::AddDynamicLight(Light* light) {
	LOCKCLASS lockClass(lockLights);

	LightsList::iterator it = std::find(dynamicLights.begin(), dynamicLights.end(), light);
	if (it == dynamicLights.end())
		dynamicLights.push_back(light);
	else
		Logger::AddLog("Lights::AddDynamicLight()", "Dynamic light already on list!", LOG_ERROR);
}

void Lights::RemoveDynamicLight(Light* light) {
	LOCKCLASS lockClass(lockLights);

	LightsList::iterator it = std::find(dynamicLights.begin(), dynamicLights.end(), light);
	if (it != dynamicLights.end())
		dynamicLights.erase(it);
	else
		Logger::AddLog("Lights::RemoveDynamicLight()", "Dynamic light not on list!", LOG_ERROR);
}


int Lights::GetStaticLightsCount() {
	LOCKCLASS lockClass(lockLights);

	return staticLights.size();
}

int Lights::GetDynamicLightsCount() {
	LOCKCLASS lockClass(lockLights);

	return dynamicLights.size();
}


void Lights::UpdateStaticLightMap(Map* map) {
	LOCKCLASS lockClass1(Map::lockMap);
	LOCKCLASS lockClass2(Tile::lockTile);
	LOCKCLASS lockClass3(lockLights);

	if (Game::adminOptions.fullLight)
		return;

	memset(&staticMap, 0, 19 * 15 * 3);

	if (!map) {
		Logger::AddLog("Lights::UpdateStaticLightMap()", "Pointer to map is NULL!", LOG_WARNING);
		return;
	}

	Position corner = map->GetCorner();

	POINT pointZ = map->GetZ();
	unsigned char z_from = pointZ.x;
	unsigned char z_to = pointZ.y;

	for (int z = z_from; z >= z_to; z--) {
		int offset = z - corner.z;

		LightsList::iterator it = staticLights.begin();
		for (it; it != staticLights.end(); it++) {
			Light* light = *it;
			StaticItemLight* itemLight = dynamic_cast<StaticItemLight*>(light);
			StaticMagicEffectLight* magicEffectLight = dynamic_cast<StaticMagicEffectLight*>(light);

			Position pos;
			if (itemLight && itemLight->item) pos = itemLight->item->pos;
			else if (magicEffectLight && magicEffectLight->magicEffect) pos = magicEffectLight->magicEffect->pos;

			if (light && pos.z == z) {
				COLOR color = ConvertColorSTD(light->color);
				unsigned char radius = light->level;

				int lx = pos.x - corner.x + offset + 1;
				int ly = pos.y - corner.y + offset + 1;
				for (int x = lx - radius; x <= lx + radius; x++)
				for (int y = ly - radius; y <= ly + radius; y++) {
					if (map->IsHiddenTile(x, y, z, z_to)) {
						staticMap[x][y][0] = 0;
						staticMap[x][y][1] = 0;
						staticMap[x][y][2] = 0;
						continue;
					}

					if (x >= 0 && x < 19 && y >= 0 && y < 15) {
						float dist;
						dist = ((float)radius - sqrt(pow(x - lx, 2) + pow(y - ly, 2))) / 6.0f;

						if (dist < 0.0f) dist = 0.0f;
						else if (dist > 1.0f) dist = 1.0f;

						int r = (int)(255.0f * dist * color.red);
						int g = (int)(255.0f * dist * color.green);
						int b = (int)(255.0f * dist * color.blue);

						r = std::max(r, (int)staticMap[x][y][0]);
						g = std::max(g, (int)staticMap[x][y][1]);
						b = std::max(b, (int)staticMap[x][y][2]);

						staticMap[x][y][0] = (unsigned char)r;
						staticMap[x][y][1] = (unsigned char)g;
						staticMap[x][y][2] = (unsigned char)b;
					}
				}
			}
		}
	}
}

void Lights::UpdateDynamicLightMap(Map* map) {
	LOCKCLASS lockClass1(Map::lockMap);
	LOCKCLASS lockClass2(Tile::lockTile);
	LOCKCLASS lockClass3(lockLights);

	if (Game::adminOptions.fullLight)
		return;

	memset(&dynamicMap, 0, 19 * 15 * 3);

	if (!map) {
		Logger::AddLog("Lights::UpdateDynamicLightMap()", "Pointer to map is NULL!", LOG_WARNING);
		return;
	}

	Position corner = map->GetCorner();

	POINT pointZ = map->GetZ();
	unsigned char z_from = pointZ.x;
	unsigned char z_to = pointZ.y;

	for (int z = z_from; z >= z_to; z--) {
		int offset = z - corner.z;

		LightsList::iterator it = dynamicLights.begin();
		for (it; it != dynamicLights.end(); it++) {
			Light* light = *it;
			DynamicCreatureLight* creatureLight = dynamic_cast<DynamicCreatureLight*>(light);
			DynamicDistanceLight* distanceLight = dynamic_cast<DynamicDistanceLight*>(light);

			Position pos;
			if (creatureLight && creatureLight->creature) pos = creatureLight->creature->pos;
			else if (distanceLight && distanceLight->distance) pos = distanceLight->distance->pos;

			if (light && pos.z == z) {
				COLOR color = ConvertColorSTD(light->color);
				unsigned char radius = light->level;

				float px = 0.0f;
				float py = 0.0f;
				if (creatureLight) {
					Creature* creature = creatureLight->creature;
					if (creature->newPos == Position(0, 0, 0)) {
						px = 32.0f * (creature->pos.x - creature->oldPos.x) * creature->GetStep();
						py = 32.0f * (creature->pos.y - creature->oldPos.y) * creature->GetStep();
					}
					else {
						px = 32.0f * (creature->newPos.x - creature->pos.x) * (1.0f - creature->GetStep());
						py = 32.0f * (creature->newPos.y - creature->pos.y) * (1.0f - creature->GetStep());
					}
				}
				else if (distanceLight) {
					Distance* distance = distanceLight->distance;

					int dx = (distance->newPos.x - distance->pos.x);
					int dy = (distance->newPos.y - distance->pos.y);

					float step = 1.0f - (float)(distance->GetMaxLiveTime() - distance->GetLiveTime()) / distance->GetMaxLiveTime();
					px = 32.0f * ((float)(distance->pos.x - pos.x) + (float)dx * step);
					py = 32.0f * ((float)(distance->pos.y - pos.y) + (float)dy * step);
				}

				float _lx = ((float)(pos.x - corner.x + offset + 1) * 32 + px);
				float _ly = ((float)(pos.y - corner.y + offset + 1) * 32 + py);
				int lx = (int)(_lx / 32);
				int ly = (int)(_ly / 32);
				float lx_off = (float)lx - (_lx / 32);
				float ly_off = (float)ly - (_ly / 32);

				int startX = lx - radius - 1;
				int startY = ly - radius - 1;
				int stopX = lx + radius + 1;
				int stopY = ly + radius + 1;
				if (startX < 0) startX = 0;
				if (startY < 0) startY = 0;
				if (stopX > 18) stopX = 18;
				if (stopY > 14) stopY = 14;

				for (int x = startX; x <= stopX; x++)
				for (int y = startY; y <= stopY; y++) {
					if (map->IsHiddenTile(x, y, z, z_to)) {
						dynamicMap[x][y][0] = 0;
						dynamicMap[x][y][1] = 0;
						dynamicMap[x][y][2] = 0;
						continue;
					}

					if (x >= 0 && x < 19 && y >= 0 && y < 15) {
						float dist;

						if (radius == 255)
							dist = 1.0f;
						else {
							dist = ((float)radius - sqrt(pow((float)(x - lx) + lx_off, 2) + pow((float)(y - ly) + ly_off, 2))) / 6.0f;

							if (dist < 0.0f) dist = 0.0f;
							else if (dist > 1.0f) dist = 1.0f;
						}

						int r = (int)(255.0f * dist * color.red);
						int g = (int)(255.0f * dist * color.green);
						int b = (int)(255.0f * dist * color.blue);

						r = std::max(r, (int)dynamicMap[x][y][0]);
						g = std::max(g, (int)dynamicMap[x][y][1]);
						b = std::max(b, (int)dynamicMap[x][y][2]);

						dynamicMap[x][y][0] = (unsigned char)r;
						dynamicMap[x][y][1] = (unsigned char)g;
						dynamicMap[x][y][2] = (unsigned char)b;
					}
				}
			}
		}
	}
}

void Lights::PrintLightMap(AD2D_Window* gfx, unsigned char z, float x, float y, float width, float height) {
	LOCKCLASS lockClass(lockLights);

	if (Game::adminOptions.fullLight)
		return;

	float ratio = (float)Game::options.ambientLight / 100;
	int gR = (int)(ratio * 255);
	int gG = (int)(ratio * 255);
	int gB = (int)(ratio * 255);
	if (z <= 7) {
		COLOR global = ConvertColorSTD(globalColor);
		gR = (int)(global.red * globalLevel * (1.0f - ratio) + ratio * 255);
		gG = (int)(global.green * globalLevel * (1.0f - ratio) + ratio * 255);
		gB = (int)(global.blue * globalLevel * (1.0f - ratio) + ratio * 255);
	}

	LightMap printMap;
	for (int a = 0; a < 19; a++)
	for (int b = 0; b < 15; b++) {
		int cr = std::max(gR, (int)staticMap[a][b][0]);
		int cg = std::max(gG, (int)staticMap[a][b][1]);
		int cb = std::max(gB, (int)staticMap[a][b][2]);
		cr = std::max(cr, (int)dynamicMap[a][b][0]);
		cg = std::max(cg, (int)dynamicMap[a][b][1]);
		cb = std::max(cb, (int)dynamicMap[a][b][2]);
		printMap[a][b][0] = cr;
		printMap[a][b][1] = cg;
		printMap[a][b][2] = cb;
	}

	x -= width / 2;
	y -= height / 2;

	AD2D_Window::SetBlendFunc(BLEND_ZERO, BLEND_SRCCOLOR);
	if (AD2D_Window::_mode == API_OPENGL) {
		glDisable(GL_TEXTURE_2D);
		glBegin(GL_QUADS);

		for (int lx = 0; lx < 19 - 1; lx++)
		for (int ly = 0; ly < 15 - 1; ly++) {
			AD2D_Window::SetColor((float)printMap[lx][ly][0] / 255, (float)printMap[lx][ly][1] / 255, (float)printMap[lx][ly][2] / 255);				glVertex2f(x + width * lx, y + height * ly);
			AD2D_Window::SetColor((float)printMap[lx+1][ly][0] / 255, (float)printMap[lx+1][ly][1] / 255, (float)printMap[lx+1][ly][2] / 255);			glVertex2f(x + width * lx + width, y + height * ly);
			AD2D_Window::SetColor((float)printMap[lx+1][ly+1][0] / 255, (float)printMap[lx+1][ly+1][1] / 255, (float)printMap[lx+1][ly+1][2] / 255);	glVertex2f(x + width * lx + width, y + height * ly + height);
			AD2D_Window::SetColor((float)printMap[lx][ly+1][0] / 255, (float)printMap[lx][ly+1][1] / 255, (float)printMap[lx][ly+1][2] / 255);			glVertex2f(x + width * lx, y + height * ly + height);
		}

		glEnd();
		glEnable(GL_TEXTURE_2D);
	}
	else if (AD2D_Window::_mode == API_DIRECT3D) {
		LPDIRECT3DDEVICE9 D3DDev = gfx->GetD3DDEV();
		LPDIRECT3DVERTEXBUFFER9 D3DVB = gfx->GetD3DVB();
		D3DDev->SetTexture(0, NULL);

		COLOR currentColor = gfx->GetColor();
		AD2D_Viewport currentViewport = gfx->GetCurrentViewport();

		D3DCUSTOMVERTEX* pVertices;
		D3DVB->Lock(0, (18 * 14) * 6 * sizeof(D3DCUSTOMVERTEX), (void**)&pVertices, 0);
		int vNum = 0;
		for (int lx = 0; lx < 19 - 1; lx++)
		for (int ly = 0; ly < 15 - 1; ly++) {
			pVertices[vNum++] = {currentViewport.startX + x + width * lx, currentViewport.startY + y + height * ly, 0.0f, 1.0f, D3DCOLOR_COLORVALUE((float)printMap[lx][ly][0] / 255, (float)printMap[lx][ly][1] / 255, (float)printMap[lx][ly][2] / 255, currentColor.alpha), 0.0f, 0.0f };
			pVertices[vNum++] = {currentViewport.startX + x + width * lx + width, currentViewport.startY + y + height * ly, 0.0f, 1.0f, D3DCOLOR_COLORVALUE((float)printMap[lx+1][ly][0] / 255, (float)printMap[lx+1][ly][1] / 255, (float)printMap[lx+1][ly][2] / 255, currentColor.alpha), 0.0f, 0.0f };
			pVertices[vNum++] = {currentViewport.startX + x + width * lx + width, currentViewport.startY + y + height * ly + height, 0.0f, 1.0f, D3DCOLOR_COLORVALUE((float)printMap[lx+1][ly+1][0] / 255, (float)printMap[lx+1][ly+1][1] / 255, (float)printMap[lx+1][ly+1][2] / 255, currentColor.alpha), 0.0f, 0.0f };

			pVertices[vNum++] = {currentViewport.startX + x + width * lx, currentViewport.startY + y + height * ly, 0.0f, 1.0f, D3DCOLOR_COLORVALUE((float)printMap[lx][ly][0] / 255, (float)printMap[lx][ly][1] / 255, (float)printMap[lx][ly][2] / 255, currentColor.alpha), 0.0f, 0.0f };
			pVertices[vNum++] = {currentViewport.startX + x + width * lx + width, currentViewport.startY + y + height * ly + height, 0.0f, 1.0f, D3DCOLOR_COLORVALUE((float)printMap[lx+1][ly+1][0] / 255, (float)printMap[lx+1][ly+1][1] / 255, (float)printMap[lx+1][ly+1][2] / 255, currentColor.alpha), 0.0f, 0.0f };
			pVertices[vNum++] = {currentViewport.startX + x + width * lx, currentViewport.startY + y + height * ly + height, 0.0f, 1.0f, D3DCOLOR_COLORVALUE((float)printMap[lx][ly+1][0] / 255, (float)printMap[lx][ly+1][1] / 255, (float)printMap[lx][ly+1][2] / 255, currentColor.alpha), 0.0f, 0.0f };
		}
		D3DVB->Unlock();

		D3DDev->DrawPrimitive(D3DPT_TRIANGLELIST, 0, vNum / 3);
	}
	AD2D_Window::SetBlendFunc(BLEND_SRCALPHA, BLEND_INVSRCALPHA);
}
