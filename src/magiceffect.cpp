/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#include "magiceffect.h"

#include "allocator.h"
#include "ad2d.h"


// ---- MagicEffect ---- //

MagicEffectsList MagicEffect::magicEffects;

int MagicEffect::threads = 0;
MUTEX MagicEffect::lockMagicEffect;

MagicEffect::MagicEffect() {
	this->iType = NULL;
}

MagicEffect::~MagicEffect() {
	if (light)
		delete_debug(light, M_PLACE);
}

bool MagicEffect::Create(unsigned short lookType, Position pos) {
	LOCKCLASS lockClass(lockMagicEffect);

	this->pos = pos;
	this->anim = 0;
	ItemType* iType = Item::GetItemType(Item::GetItemsCount() + Item::GetCreaturesCount() + lookType);
	if (iType && iType->light && iType->lightColor > 0 && iType->lightRadius > 0)
		light = new(M_PLACE) StaticMagicEffectLight(this, iType->lightColor, iType->lightRadius);

	this->iType = iType;

	bool exist = false;
	MagicEffectsList::iterator it = magicEffects.begin();
	for (it; it != magicEffects.end() && !exist; it++) {
		MagicEffect* me = *it;
		if (me->pos == pos && iType == me->iType && me->anim < 3)
			exist = true;
	}

	if (!exist) {
		magicEffects.push_back(this);
		return true;
	}

	return false;
}

ItemType* MagicEffect::GetItemType() {
	return iType;
}

Position MagicEffect::GetPosition() {
	return pos;
}

void MagicEffect::PrintMagicEffect(AD2D_Window* gfx, Position pos, float x, float y, float width, float height) {
	if (!iType)
		return;

	int xOffset = 0;
	int yOffset = 0;
	if (iType->m_xdiv > 1 || iType->m_ydiv > 1) {
		xOffset = pos.x % iType->m_xdiv;
		yOffset = pos.y % iType->m_ydiv;
	}

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

void MagicEffect::RemoveMagicEffect(MagicEffect* me) {
	LOCKCLASS lockClass(lockMagicEffect);

	if (!me) {
		Logger::AddLog("MagicEffect::RemoveMagicEffect()", "Pointer to magic effect is NULL!", LOG_WARNING);
		return;
	}

	MagicEffectsList::iterator it = std::find(magicEffects.begin(), magicEffects.end(), me);
	if (it != magicEffects.end()) {
		magicEffects.erase(it);
		delete_debug(me, M_PLACE);
	}
	else
		Logger::AddLog("MagicEffect::RemoveMagicEffect()", "Magic effect not in list!", LOG_WARNING);
}

void MagicEffect::ClearMagicEffects() {
	LOCKCLASS lockClass(lockMagicEffect);

	MagicEffectsList::iterator it = magicEffects.begin();
	for (it; it != magicEffects.end(); it++) {
		MagicEffect* me = *it;
		delete_debug(me, M_PLACE);
	}
	magicEffects.clear();
}

void MagicEffect::CheckMagicEffects(Game* game) {
	if (threads > 0)
		return;

	threads++;

	int last_size = 0;

	while(game->GetGameState() == GAME_LOGGEDTOGAME) {
		lockMagicEffect.lock();

		MagicEffectsList toDelete;

		int size_before = magicEffects.size();

		MagicEffectsList::iterator it = magicEffects.begin();
		for (it; it != magicEffects.end(); it++) {
			MagicEffect* me = *it;
			if (me) {
				me->anim++;
				if (me->iType && me->anim >= me->iType->m_anim)
					toDelete.push_back(me);
			}
		}

		it = toDelete.begin();
		for (it; it != toDelete.end(); it++) {
			MagicEffect* me = *it;
			RemoveMagicEffect(me);
		}

		int size_after = magicEffects.size();

		lockMagicEffect.unlock();

		if (size_before != size_after || size_after != last_size)
			Lights::UpdateStaticLightMap(game->GetMap());

		last_size = size_after;

		Sleep(100);
	}

	ClearMagicEffects();

	threads--;
}

void MagicEffect::PrintMagicEffects(AD2D_Window* gfx, Position pos, float x, float y, float width, float height) {
	LOCKCLASS lockClass(lockMagicEffect);

	MagicEffectsList::iterator it = magicEffects.begin();
	for (it; it != magicEffects.end(); it++) {
		MagicEffect* me = *it;
		if (me && me->pos == pos)
			me->PrintMagicEffect(gfx, pos, x, y, width, height);
	}
}
