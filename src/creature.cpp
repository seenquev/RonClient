/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#include "creature.h"

#include "allocator.h"
#include "game.h"
#include "icons.h"
#include "logger.h"
#include "luascript.h"
#include "realtime.h"

// ---- OutfitType ---- //

OutfitType::OutfitType() {
	lookType = 0;
	lookAddons = 0;
	name = "";
}

OutfitType::~OutfitType() {
}


// ---- Outfit ---- //

Outfit::Outfit() {
	lookType = 0;
	lookTypeEx = 0;
	lookHead = 0;
	lookBody = 0;
	lookLegs = 0;
	lookFeet = 0;
	lookAddons = 0;
}

Outfit::~Outfit() {
}

void Outfit::SetTemplateColor(COLOR currentColor, int index) {
	int x = index % 19;
	int y = index / 19;

	int a = x;
	int b = y + 2;
	int c = 0;

	if (y <= 2) c = y;
	else if (y >= 3 && y < 5) c = (4 - y);
	else if (y >= 5) c = (y - 4) * (y - 4);

	float R=0.0f;
	float G=0.0f;
	float B=0.0f;

	if (a==0)		{	R=(float)(255-(b-2)*36)/(255);	G=(float)(255-(b-2)*36)/(255);	B=(float)(255-(b-2)*36)/(255);	}
	else if (a==1)	{	R=(float)(255-b*0)/(255+c*50);	G=(float)(255-b*24)/(255+c*50);	B=(float)(255-b*36)/(255+c*50);	}
	else if (a==2)	{	R=(float)(255-b*0)/(255+c*50);	G=(float)(255-b*12)/(255+c*50);	B=(float)(255-b*36)/(255+c*50);	}
	else if (a==3)	{	R=(float)(255-b*0)/(255+c*50);	G=(float)(255-b*0)/(255+c*50);	B=(float)(255-b*36)/(255+c*50);	}
	else if (a==4)	{	R=(float)(255-b*12)/(255+c*50);	G=(float)(255-b*0)/(255+c*50);	B=(float)(255-b*36)/(255+c*50);	}
	else if (a==5)	{	R=(float)(255-b*24)/(255+c*50);	G=(float)(255-b*0)/(255+c*50);	B=(float)(255-b*36)/(255+c*50);	}
	else if (a==6)	{	R=(float)(255-b*36)/(255+c*50);	G=(float)(255-b*0)/(255+c*50);	B=(float)(255-b*36)/(255+c*50);	}
	else if (a==7)	{	R=(float)(255-b*36)/(255+c*50);	G=(float)(255-b*0)/(255+c*50);	B=(float)(255-b*24)/(255+c*50);	}
	else if (a==8)	{	R=(float)(255-b*36)/(255+c*50);	G=(float)(255-b*0)/(255+c*50);	B=(float)(255-b*12)/(255+c*50);	}
	else if (a==9)	{	R=(float)(255-b*36)/(255+c*50);	G=(float)(255-b*0)/(255+c*50);	B=(float)(255-b*0)/(255+c*50);	}
	else if (a==10)	{	R=(float)(255-b*36)/(255+c*50);	G=(float)(255-b*12)/(255+c*50);	B=(float)(255-b*0)/(255+c*50);	}
	else if (a==11)	{	R=(float)(255-b*36)/(255+c*50);	G=(float)(255-b*24)/(255+c*50);	B=(float)(255-b*0)/(255+c*50);	}
	else if (a==12)	{	R=(float)(255-b*36)/(255+c*50);	G=(float)(255-b*36)/(255+c*50);	B=(float)(255-b*0)/(255+c*50);	}
	else if (a==13)	{	R=(float)(255-b*24)/(255+c*50);	G=(float)(255-b*36)/(255+c*50);	B=(float)(255-b*0)/(255+c*50);	}
	else if (a==14)	{	R=(float)(255-b*12)/(255+c*50);	G=(float)(255-b*36)/(255+c*50);	B=(float)(255-b*0)/(255+c*50);	}
	else if (a==15)	{	R=(float)(255-b*0)/(255+c*50);	G=(float)(255-b*36)/(255+c*50);	B=(float)(255-b*0)/(255+c*50);	}
	else if (a==16)	{	R=(float)(255-b*0)/(255+c*50);	G=(float)(255-b*36)/(255+c*50);	B=(float)(255-b*12)/(255+c*50);	}
	else if (a==17)	{	R=(float)(255-b*0)/(255+c*50);	G=(float)(255-b*36)/(255+c*50);	B=(float)(255-b*24)/(255+c*50);	}
	else if (a==18)	{	R=(float)(255-b*0)/(255+c*50);	G=(float)(255-b*36)/(255+c*50);	B=(float)(255-b*36)/(255+c*50);	}

	AD2D_Window::SetColor(currentColor.red * R, currentColor.green * G, currentColor.blue * B, currentColor.alpha);
}


// ---- Creature ---- //

LoadedSet							Creature::loadedLooktypes;
std::map<std::string, int>			Creature::ignoredCreatures;
std::map<unsigned int, Creature*>	Creature::knownCreatures;
MUTEX								Creature::lockCreature;


Creature::Creature() {
	Clear();
}

Creature::Creature(Creature* creature) {
	Clear();

	id = creature->id;
	shine = creature->shine;
	name = creature->name;
	direction = creature->direction;
	outfit = creature->outfit;
	pos = creature->pos;
	skull = creature->skull;
	shield = creature->shield;
	squareTime = creature->squareTime;
	squareColor = creature->squareColor;
}

Creature::~Creature() {
	if (light)
		delete_debug(light, M_PLACE);
    if (equipment)
        delete_debug(equipment, M_PLACE);
}

void Creature::Clear() {
	id = 0;
	name = "";

	shine = 0;

	health = 0;
	direction = NORTH;
	lastMove = NONE;

	lightLevel = 0;
	lightColor = 0;

	speed = 100;

	skull = 0;
	shield = 0;

	war = 0;

	blocking = true;
	ignored = false;

	step = 0.0f;

	squareTime = 0;
	squareColor = 0;

    equipmentTime = 0;
	equipment = NULL;
}

void Creature::SetID(unsigned int id) {
	this->id = id;
}

unsigned int Creature::GetID() {
	return this->id;
}

void Creature::SetShine(unsigned int shine) {
	this->shine = shine;
}

unsigned int Creature::GetShine() {
	return this->shine;
}

bool Creature::IsPlayer() {
	return ((unsigned int)(id & 0x10000000) == 0x10000000);
}

bool Creature::IsNPC() {
	return ((unsigned int)(id & 0x30000000) == 0x30000000);
}

bool Creature::IsMonster() {
	return ((unsigned int)(id & 0x40000000) == 0x40000000);
}

void Creature::SetName(std::string name) {
	this->name = name;
}

std::string Creature::GetName() {
	return this->name;
}

std::string* Creature::GetNamePtr() {
	return &this->name;
}

void Creature::SetHealth(unsigned char health) {
	this->health = health;
}

unsigned char Creature::GetHealth() {
	return this->health;
}

unsigned char* Creature::GetHealthPtr() {
	return &this->health;
}

void Creature::SetDirection(Direction direction) {
	if (direction == NORTHWEST) direction = WEST;
	else if (direction == NORTHEAST) direction = EAST;
	else if (direction == SOUTHWEST) direction = WEST;
	else if (direction == SOUTHEAST) direction = EAST;
	this->direction = direction;
}

Direction Creature::GetDirection() {
	return this->direction;
}

void Creature::SetLastMove(Direction direction) {
	this->lastMove = direction;
}

Direction Creature::GetLastMove() {
	return this->lastMove;
}

void Creature::SetOutfit(Outfit outfit) {
	this->outfit = outfit;
}

Outfit Creature::GetOutfit() {
	return this->outfit;
}

void Creature::SetLight(unsigned char lightColor, unsigned char lightLevel) {
	this->lightColor = lightColor;
	this->lightLevel = lightLevel;

	if (lightColor > 0 && lightLevel > 0) {
		if (light)
			delete_debug(light, M_PLACE);

		light = new(M_PLACE) DynamicCreatureLight(this, lightColor, lightLevel);
	}
	else {
		if (light)
			delete_debug(light, M_PLACE);

		light = NULL;
	}
}

unsigned char Creature::GetLightLevel() {
	return this->lightLevel;
}

unsigned char Creature::GetLightColor() {
	return this->lightColor;
}

void Creature::SetSpeed(unsigned short speed) {
	this->speed = speed;
}

unsigned short Creature::GetSpeed() {
	return this->speed;
}

void Creature::SetSkull(unsigned char skull) {
	this->skull = skull;
}

unsigned char Creature::GetSkull() {
	return this->skull;
}

void Creature::SetShield(unsigned char shield) {
	this->shield = shield;
}

unsigned char Creature::GetShield() {
	return this->shield;
}

void Creature::SetWar(unsigned char war) {
	this->war = war;
}

unsigned char Creature::GetWar() {
	return this->war;
}

void Creature::SetBlocking(bool blocking) {
	this->blocking = blocking;
}

bool Creature::GetBlocking() {
	return this->blocking;
}

void Creature::SetStep(float step) {
	this->step = step;
}

float Creature::GetStep() {
	return step;
}

void Creature::SetSquare(time_lt time, unsigned char color) {
	squareTime = time;
	squareColor = color;
}

time_lt Creature::GetSquareTime() {
	return squareTime;
}

unsigned char Creature::GetSquareColor() {
	return squareColor;
}

void Creature::SetEquipment(time_lt time, Container* container) {
    if (equipment)
        delete_debug(equipment, M_PLACE);

    equipmentTime = time;
    equipment = container;
}

time_lt Creature::GetEquipmentTime() {
    return equipmentTime;
}

Container* Creature::GetEquipment() {
    return equipment;
}

ItemType* Creature::operator()() {
	int lookType = (outfit.lookType != 0 ? Item::GetItemsCount() + outfit.lookType : outfit.lookTypeEx);
	ItemType* iType = Item::GetItemType(lookType);

	return iType;
}

void Creature::GetStepOffset(float& x, float& y) {
	if (this->newPos == Position(0, 0, 0)) {
		x = (this->pos.x - this->oldPos.x) * this->GetStep();
		y = (this->pos.y - this->oldPos.y) * this->GetStep();
	}
	else {
		x = (this->newPos.x - this->pos.x) * (1.0f - this->GetStep());
		y = (this->newPos.y - this->pos.y) * (1.0f - this->GetStep());
	}
}

void Creature::PrintCreature(AD2D_Window* gfx, Position pos, float x, float y, float width, float height, bool ignoreColors) {
	int lookType = (outfit.lookType != 0 ? Item::GetItemsCount() + outfit.lookType : outfit.lookTypeEx);
	if (lookType == 0)
		return;

	ItemType* iType = Item::GetItemType(lookType);
	if (!iType)
		return;

	int direction = (step < 0.0f ? this->lastMove : this->direction) % iType->m_xdiv;

	int anim = 0;
	if (iType->idleAnimated)
		anim = (RealTime::getTime() / 250) % iType->m_anim;
	else
		anim = (step == 0.0f ? 0 : (iType->m_anim > 1 ? 1 + abs((int)(step * 4) % (iType->m_anim - 1)) : 0));

	float xOffset = (iType->hasOffset ? iType->xOffset : 0);
	float yOffset = (iType->hasOffset ? iType->yOffset : 0);

	bool mount = (outfit.lookMount != 0 ? true : false);
	if (mount) {
		Creature mountCreature;
		mountCreature = (*this);
		mountCreature.light = NULL;
		mountCreature.equipment = NULL;
		mountCreature.outfit.lookType = outfit.lookMount;
		mountCreature.outfit.lookAddons = 0;
		mountCreature.outfit.lookMount = 0;
		mountCreature.PrintCreature(gfx, pos, x, y, width, height);
		xOffset = 0;
		yOffset = 0;
	}

	LoadedSet::iterator it = loadedLooktypes.find(lookType);
	if (it == loadedLooktypes.end() && iType->m_blend == 2) {
		for (int anim = 0; anim < iType->m_anim; anim++)
		for (int mount = 0; mount < iType->m_pos; mount++)
		for (int dir = 0; dir < iType->m_xdiv; dir++)
		for (int addon = 0; addon < iType->m_ydiv; addon++)
		for (int px = 0; px < iType->m_width; px++)
		for (int py = 0; py < iType->m_height; py++) {
			int offsetOutfit = px +	(py * iType->m_width) +
									(dir * iType->m_width * iType->m_height * iType->m_blend) +
									(addon * iType->m_width * iType->m_height * iType->m_blend * iType->m_xdiv) +
									(mount * iType->m_width * iType->m_height * iType->m_blend * iType->m_xdiv * iType->m_ydiv) +
									(anim * iType->m_width * iType->m_height * iType->m_blend * iType->m_xdiv * iType->m_ydiv * iType->m_pos);

			int offset = offsetOutfit + (iType->m_width * iType->m_height);

			AD2D_Image* outfitImage = NULL;
			if (offsetOutfit < iType->m_sprNum)
				outfitImage = Sprites::GetSprite(iType->m_sprPtr[offsetOutfit]);

			if (offset < iType->m_sprNum) {
				ColorTemplate* ct = Sprites::GetColorTemplate(iType->m_sprPtr[offset], outfitImage);
				Sprites::GetColorTemplate(iType->m_sprPtr[offsetOutfit], outfitImage, ct);
			}
		}

		loadedLooktypes.insert(lookType);
	}

	int blend = (ignoreColors ? 1 : iType->m_blend);
	for (int addon = 0; addon < iType->m_ydiv; addon++) {
		if (addon == 0 || (addon > 0 && outfit.lookAddons & 0x01 << (addon - 1))) {
			for (int pb = 0; pb < blend; pb++)
			for (int px = 0; px < iType->m_width; px++)
			for (int py = 0; py < iType->m_height; py++) {
				float mx = x - px * width - (float)(xOffset * (width / 32));
				float my = y - py * height - (float)(yOffset * (height / 32));

				int offset = px +	(py * iType->m_width) +
									(pb * iType->m_width * iType->m_height) +
									(direction * iType->m_width * iType->m_height * iType->m_blend) +
									(addon * iType->m_width * iType->m_height * iType->m_blend * iType->m_xdiv) +
									(mount * iType->m_width * iType->m_height * iType->m_blend * iType->m_xdiv * iType->m_ydiv) +
									(anim * iType->m_width * iType->m_height * iType->m_blend * iType->m_xdiv * iType->m_ydiv * iType->m_pos);

				AD2D_Image* image = NULL;
				ColorTemplate* ct = NULL;
				if (offset < iType->m_sprNum) {
					if (pb == 0) image = Sprites::GetSprite(iType->m_sprPtr[offset]);
					if (blend > 1) ct = Sprites::GetColorTemplate(iType->m_sprPtr[offset]);
				}

				if (image) {
					if (ct) gfx->PutImage(mx, my, mx + width, my + height, *ct->all);
					else gfx->PutImage(mx, my, mx + width, my + height, *image);
				}
				else if (ct) {
					COLOR currentColor = AD2D_Window::GetColor();

					gfx->SetBlendFunc(BLEND_DSTCOLOR, BLEND_INVSRCALPHA);
					outfit.SetTemplateColor(currentColor, outfit.lookHead);	gfx->PutImage(mx, my, mx + width, my + height, *ct->head);
					outfit.SetTemplateColor(currentColor, outfit.lookBody);	gfx->PutImage(mx, my, mx + width, my + height, *ct->body);
					outfit.SetTemplateColor(currentColor, outfit.lookLegs);	gfx->PutImage(mx, my, mx + width, my + height, *ct->legs);
					outfit.SetTemplateColor(currentColor, outfit.lookFeet);	gfx->PutImage(mx, my, mx + width, my + height, *ct->feet);
					gfx->SetBlendFunc(BLEND_SRCALPHA, BLEND_INVSRCALPHA);

					AD2D_Window::SetColor(currentColor.red, currentColor.green, currentColor.blue, currentColor.alpha);
				}
			}
		}
	}
}

void Creature::PrintDetails(AD2D_Window* gfx, AD2D_Font* font, Position pos, float x, float y, float width, float height, int name, int hp, int mana, bool attacked, bool followed, bool selected) {
	float stepOffsetX;
	float stepOffsetY;
	GetStepOffset(stepOffsetX, stepOffsetY);
	stepOffsetX *= width;
	stepOffsetY *= height;

	float fontSize = Game::options.textSize;

	int posY = 0;
	if (name >= 0) posY += (int)fontSize;
	if (hp >= 0) posY += 5;
	if (mana >= 0) posY += 5;

	float period = (float)(RealTime::getTime() % 1000) / 1000;
	float offset = (width / 8) * sin(2 * 3.141592654 * period);

	if (name >= 0) {
		if (name > 100) name = 100;
		float nx = ((float)name / 100);
		float r = (nx < 0.5f ? 0.3 + 0.7f * (2 * nx) : 1.0f - 2 * (nx - 0.5f));
		float g = nx;
		float b = 0.3f * nx;

		float px = x + width / 2 - font->GetTextWidth(this->name, fontSize) / 2 + stepOffsetX;
		float py = y /*- (height / 4)*/ + stepOffsetY - posY;
		AD2D_Window::SetColor(r, g, b, 1.0f);
		gfx->Print(px, py, fontSize, *font, this->name.c_str(), true);
		AD2D_Window::SetColor(1.0f, 1.0f, 1.0f, 1.0f);
		posY -= (int)fontSize;
	}
	if (hp >= 0) {
		if (hp > 100) hp = 100;
		float nx = ((float)hp / 100);
		float r = (nx < 0.5f ? 0.3 + 0.7f * (2 * nx) : 1.0f - 2 * (nx - 0.5f));
		float g = nx;
		float b = 0.3f * nx;

		float size = width / 1.5f;
		float px = x + (width / 2) - (size / 2) + stepOffsetX;
		float py = y /*- (height / 4)*/ + stepOffsetY - posY;
		AD2D_Window::SetColor(0.0f, 0.0f, 0.0f, 1.0f);
		gfx->PutRect(px - 1, py - 1, px + size + 1, py + 2 + 1);
		AD2D_Window::SetColor(r, g, b, 1.0f);
		gfx->PutRect(px, py, px + (size * (float)hp / 100), py + 2);
		AD2D_Window::SetColor(1.0f, 1.0f, 1.0f, 1.0f);
		posY -= 5;
	}
	if (mana >= 0) {
		if (mana > 100) mana = 100;
		float size = width / 1.5f;
		float px = x + (width / 2) - (size / 2) + stepOffsetX;
		float py = y /*- (height / 4)*/ + stepOffsetY - posY;
		AD2D_Window::SetColor(0.0f, 0.0f, 0.0f, 1.0f);
		gfx->PutRect(px - 1, py - 1, px + size + 1, py + 2 + 1);
		AD2D_Window::SetColor(0.3f, 0.3f, 1.0f, 1.0f);
		gfx->PutRect(px, py, px + (size * (float)mana / 100), py + 2);
		AD2D_Window::SetColor(1.0f, 1.0f, 1.0f, 1.0f);
		posY -= 5;
	}
	if (attacked) {
		AD2D_Window::SetColor(1.0f, 1.0f, 1.0f, 1.0f);
		//gfx->PutRect(x + stepOffsetX, y + stepOffsetY, x + width + stepOffsetX, y + height + stepOffsetY);
		gfx->PutImage(x + stepOffsetX - offset, y + stepOffsetY - offset, x + width + stepOffsetX + offset, y + height + stepOffsetY + offset, *Icons::GetAttackIcon());
		AD2D_Window::SetColor(1.0f, 1.0f, 1.0f, 1.0f);
	}
	if (followed) {
		AD2D_Window::SetColor(1.0f, 1.0f, 1.0f, 1.0f);
		//gfx->PutRect(x + stepOffsetX, y + stepOffsetY, x + width + stepOffsetX, y + height + stepOffsetY);
		gfx->PutImage(x + stepOffsetX - offset, y + stepOffsetY - offset, x + width + stepOffsetX + offset, y + height + stepOffsetY + offset, *Icons::GetFollowIcon());
		AD2D_Window::SetColor(1.0f, 1.0f, 1.0f, 1.0f);
	}
	if (selected) {
		AD2D_Window::SetColor(1.0f, 1.0f, 1.0f, 1.0f);
		//gfx->PutRect(x + stepOffsetX, y + stepOffsetY, x + width + stepOffsetX, y + height + stepOffsetY);
		gfx->PutImage(x + stepOffsetX - offset, y + stepOffsetY - offset, x + width + stepOffsetX + offset, y + height + stepOffsetY + offset, *Icons::GetSelectIcon());
		AD2D_Window::SetColor(1.0f, 1.0f, 1.0f, 1.0f);
	}
	if (squareTime && (RealTime::getTime() - squareTime) < 1000) {
		float alpha = 0.6f * (float)(1000 - (RealTime::getTime() - squareTime)) / 1000;
		gfx->SetColorSTD(squareColor, alpha);
		gfx->PutRect(x + stepOffsetX, y + stepOffsetY, x + width + stepOffsetX, y + height + stepOffsetY);
		AD2D_Window::SetColor(1.0f, 1.0f, 1.0f, 1.0f);
	}
}

void Creature::PrintIcons(AD2D_Window* gfx, Position pos, float x, float y, float width, float height) {
	float stepOffsetX;
	float stepOffsetY;
	GetStepOffset(stepOffsetX, stepOffsetY);

	unsigned char sk = GetSkull();
	unsigned char sh = GetShield();
	AD2D_Image* skull = (sk != 0 ? Icons::GetSkullIcon(sk) : NULL);
	AD2D_Image* shield = (sh != 0 ? Icons::GetShieldIcon(sh) : NULL);

	int i = (sk != 0 && sh != 0 ? 2 : 1);
	float px = x + (width - width / 2) - (width / 4) * i + stepOffsetX * width;
	float py = y + (height - height / 2) + stepOffsetY * height;

	if (skull) {
		AD2D_Window::SetColor(1.0f, 1.0f, 1.0f, 1.0f);
		gfx->PutImage(px + (width / 2) * (i - 1), py, px + (width / 2) * i, py + (height / 2), *skull);
		AD2D_Window::SetColor(1.0f, 1.0f, 1.0f, 1.0f);
	}

	if (shield) {
		if (sh > 8) {
			float period = (float)(RealTime::getTime() % 1000) / 1000;
			float alpha = (float)(1.0 + sin(2 * 3.141592654 * period)) / 2;
			AD2D_Window::SetColor(1.0f, 1.0f, 1.0f, alpha);
		}
		else
			AD2D_Window::SetColor(1.0f, 1.0f, 1.0f, 1.0f);
		gfx->PutImage(px, py, px + (width / 2), py + (height / 2), *shield);
		AD2D_Window::SetColor(1.0f, 1.0f, 1.0f, 1.0f);
	}
}

void Creature::PrintEquipment(AD2D_Window* gfx, Position pos, float x, float y, float width, float height) {
    if (!equipment || equipmentTime + 8000 < RealTime::getTime())
        return;

	Game* game = Game::game;
	Keyboard* keyboard = (game ? game->GetKeyboard() : NULL);

	if (!keyboard || !keyboard->key[VK_SHIFT]) {
		if (equipmentTime + 500 < RealTime::getTime() && equipmentTime + 7000 > RealTime::getTime())
			equipmentTime = RealTime::getTime() - 7000;
	}

    float stepOffsetX;
	float stepOffsetY;
	GetStepOffset(stepOffsetX, stepOffsetY);

    time_lt relative = RealTime::getTime() - equipmentTime;
    float radius = - (float)exp(- relative * 0.005f) + 1.0f;

    int places = equipment->GetPlaces();
	for (int i = 0; i < places; i++) {
	    Item* item = equipment->GetItem(i);

        float ex = 0.0f;
        float ey = 0.0f;
        if (i == 0) { ex = 1.0f;   ey = 0.0f; }
        else if (i == 1) { ex = 0.0f;   ey = 0.5f; }
        else if (i == 2) { ex = 2.0f;   ey = 0.5f; }
        else if (i == 3) { ex = 1.0f;   ey = 1.0f; }
        else if (i == 4) { ex = 2.0f;   ey = 1.5f; }
        else if (i == 5) { ex = 0.0f;   ey = 1.5f; }
        else if (i == 6) { ex = 1.0f;   ey = 2.0f; }
        else if (i == 7) { ex = 1.0f;   ey = 3.0f; }
        else if (i == 8) { ex = 0.0f;   ey = 2.5f; }
        else if (i == 9) { ex = 2.0f;   ey = 2.5f; }

        if (item) {
            float px = x + stepOffsetX * width + ((ex - 1.0f) * width) * radius;
            float py = y + stepOffsetY * height + ((ey - 1.5f) * height) * radius;

            float alpha = 1.0f;
            if (relative > 7000) alpha = 1.0f - (float)(relative - 7000) * 0.001f;
            else if (relative < 500) alpha = (float)relative * 0.002f;

            AD2D_Window::SetColor(1.0f, 1.0f, 1.0f, alpha);
            if (item->GetShine())
				item->PrintItemShine(gfx, pos, px, py, width, height, 0, item->GetShine());

            item->PrintItem(gfx, pos, px, py, width, height, 0);
            AD2D_Window::SetColor(1.0f, 1.0f, 1.0f, 1.0f);
        }
	}
}

void Creature::PrintCreatureShine(AD2D_Window* gfx, Position pos, float x, float y, float width, float height, unsigned int shine) {
	unsigned char* ch_shine = (unsigned char*)&shine;
	COLOR color((float)(ch_shine[0]) / 255,
				(float)(ch_shine[1]) / 255,
				(float)(ch_shine[2]) / 255, 1.0f);
	int strength = ch_shine[3] % 4;
	int mode = ch_shine[3] / 4;

	if (strength == 0)
		return;

    COLOR currentColor = AD2D_Window::GetColor();

    float rand_shine = 0.0f;
    if ((int)(color.alpha * 1000) != 0)rand_shine = (float)(rand() % (int)(color.alpha * 1000)) / 2000;

    if (mode == 0)
    	gfx->SetBlendFunc(BLEND_SRCALPHA, BLEND_ONE);
	else
		gfx->SetBlendFunc(BLEND_SRCALPHA, BLEND_INVSRCALPHA);

    AD2D_Window::SetColor(color.red, color.green, color.blue, currentColor.alpha * (color.alpha / 2 + rand_shine));
    for (int i = 0; i < 8 * strength; i++) {
        int pt = 1 + i / 8;
        float sx = 0.0f;
        float sy = 0.0f;
        if (i % 8 == 0) { sx = 0.0f;         sy = -0.05f; }
        else if (i % 8 == 1) { sx = 0.03f;   sy = -0.03f; }
        else if (i % 8 == 2) { sx = 0.05f;   sy = 0.0f; }
        else if (i % 8 == 3) { sx = 0.03f;   sy = 0.03f; }
        else if (i % 8 == 4) { sx = 0.0f;    sy = 0.05f; }
        else if (i % 8 == 5) { sx = -0.03f;  sy = 0.03f; }
        else if (i % 8 == 6) { sx = -0.05f;  sy = 0.0f; }
        else if (i % 8 == 7) { sx = -0.03f;  sy = -0.03f; }

        PrintCreature(gfx, pos, x + sx * width * pt, y + sy * height * pt, width, height, true);
    }
    gfx->SetBlendFunc(BLEND_SRCALPHA, BLEND_INVSRCALPHA);

    AD2D_Window::SetColor(currentColor.red, currentColor.green, currentColor.blue, currentColor.alpha);
}


void Creature::Ignore(std::string creatureName) {
	LOCKCLASS lockClass(lockCreature);

	ignoredCreatures[creatureName] = 0;
}

void Creature::Unignore(std::string creatureName) {
	LOCKCLASS lockClass(lockCreature);

	std::map<std::string, int>::iterator it = ignoredCreatures.find(creatureName);
	if (it != ignoredCreatures.end())
		ignoredCreatures.erase(it);
}

bool Creature::IsIgnored(std::string creatureName) {
	LOCKCLASS lockClass(lockCreature);

	std::map<std::string, int>::iterator it = ignoredCreatures.find(creatureName);
	if (it != ignoredCreatures.end())
		return true;

	return false;
}


void Creature::CheckCreatures(Map* map, float factor) {
	LOCKCLASS lockClass1(Map::lockMap);
	LOCKCLASS lockClass2(Tile::lockTile);
	LOCKCLASS lockClass3(lockCreature);

	if (!map) {
		Logger::AddLog("Creature::CheckCreatures()", "Map pointer is NULL!", LOG_ERROR);
		return;
	}

	std::map<unsigned int, Creature*>::iterator it = knownCreatures.begin();
	for (it; it != knownCreatures.end(); it++) {
		Creature* creature = it->second;
		if (creature && creature->step < 0.0f) {
			Tile* tile = map->GetTile(creature->pos);
			unsigned short tileSpeed = (tile ? tile->GetSpeed() : 220);
			creature->step += ((float)(creature->speed ? creature->speed : 220) / tileSpeed) * factor;

			if (creature->step >= 0.0f)
				creature->step = 0.0f;
		}
		else if (creature && creature->step > 0.0f) {
			Tile* tile = map->GetTile(creature->newPos);
			unsigned short tileSpeed = (tile ? tile->GetSpeed() : 220);
			creature->step -= ((float)(creature->speed ? creature->speed : 220) / tileSpeed) * factor;

			if (creature->step <= 0.0f)
				creature->step = 0.0f;
		}
	}
}

void Creature::AddToKnown(Creature* creature) {
	LOCKCLASS lockClass(lockCreature);

	if (!creature) {
		Logger::AddLog("Creature::AddToKnown()", "Creature pointer is NULL!", LOG_ERROR);
		return;
	}

	unsigned int id = creature->GetID();

	std::map<unsigned int, Creature*>::iterator it = knownCreatures.find(id);
	if (it == knownCreatures.end())
		knownCreatures[id] = creature;
	else
		Logger::AddLog("Creature::AddToKnown()", "Creature duplicate!", LOG_WARNING);
}

Creature* Creature::GetFromKnown(unsigned int id) {
	LOCKCLASS lockClass(lockCreature);

	std::map<unsigned int, Creature*>::iterator it = knownCreatures.find(id);
	if (it != knownCreatures.end()) {
		return it->second;
	}

	return NULL;
}

std::list<Creature*> Creature::GetCreaturesList() {
	LOCKCLASS lockClass(lockCreature);

	std::list<Creature*> list;

	std::map<unsigned int, Creature*>::iterator it = knownCreatures.begin();
	for (it; it != knownCreatures.end(); it++) {
		list.push_back(it->second);
	}

	return list;
}

void Creature::RemoveFromKnown(unsigned int id) {
	LOCKCLASS lockClass(lockCreature);

	if (id == 0)
		return;

	std::map<unsigned int, Creature*>::iterator it = knownCreatures.find(id);
	if (it != knownCreatures.end()) {
		Creature* creature = it->second;
		delete_debug(creature, M_PLACE);
		knownCreatures.erase(it);
	}
	else
		Logger::AddLog("Creature::RemoveFromKnown()", "Creature not in list!", LOG_WARNING);
}

void Creature::DeleteKnown() {
	LOCKCLASS lockClass(lockCreature);

	std::map<unsigned int, Creature*>::iterator it = knownCreatures.begin();
	for (it; it != knownCreatures.end(); it++) {
		Creature* creature = it->second;
		delete_debug(creature, M_PLACE);
	}

	knownCreatures.clear();
}


//Lua functions
int Creature::LuaSetCreatureName(lua_State* L) {
	std::string name = LuaScript::PopString(L);
	unsigned int creatureID = LuaScript::PopNumber(L);

	Creature* creature = Creature::GetFromKnown(creatureID);
	if (creature)
		creature->SetName(name);

	return 1;
}

int Creature::LuaGetCreatureID(lua_State* L) {
	int params = lua_gettop(L);

	int rad = 0;
	if (params > 1)
		rad = LuaScript::PopNumber(L);
	std::string name = LuaScript::PopString(L);

	Game* game = Game::game;
	Player* player = (game ? game->GetPlayer() : NULL);
	Map* map = (game ? game->GetMap() : NULL);
	Battle* battle = (map ? map->GetBattle() : NULL);

	if (player && battle) {
		std::list<Creature*> creatures = battle->GetCreatures();
		std::list<Creature*>::iterator it = creatures.begin();
		for (it; it != creatures.end(); it++) {
			Creature* creature = *it;
			if (creature && creature->GetName() == name) {
				Position pos1 = player->GetPosition();
				Position pos2 = creature->pos;

				if (abs(pos1.x - pos2.x) > rad || abs(pos1.y - pos2.y) > rad)
					continue;

				LuaScript::PushNumber(L, creature->GetID());
				return 1;
			}
		}
	}

	LuaScript::PushNumber(L, 0);
	return 1;
}

int Creature::LuaSetCreatureShine(lua_State* L) {
	unsigned int shine = LuaScript::PopNumber(L);
	unsigned int creatureID = LuaScript::PopNumber(L);

	Creature* creature = Creature::GetFromKnown(creatureID);
	if (creature)
		creature->SetShine(shine);

	return 1;
}

int Creature::LuaGetCreatureShine(lua_State* L) {
	unsigned int creatureID = LuaScript::PopNumber(L);

	Creature* creature = Creature::GetFromKnown(creatureID);
	unsigned int shine = (creature ? creature->GetShine() : 0);

	LuaScript::PushNumber(L, shine);
	return 1;
}

int Creature::LuaGetCreatureName(lua_State* L) {
	unsigned int creatureID = LuaScript::PopNumber(L);

	Creature* creature = Creature::GetFromKnown(creatureID);
	std::string name = (creature ? creature->GetName() : std::string(""));

	LuaScript::PushString(L, name);
	return 1;
}

int Creature::LuaGetCreaturePosition(lua_State* L) {
	unsigned int creatureID = LuaScript::PopNumber(L);

	Creature* creature = Creature::GetFromKnown(creatureID);
	Position position = (creature ? creature->pos : Position(0, 0, 0));

	LuaScript::PushPosition(L, position);
	return 1;
}

int Creature::LuaGetCreatureHealth(lua_State* L) {
	unsigned int creatureID = LuaScript::PopNumber(L);

	Creature* creature = Creature::GetFromKnown(creatureID);
	double value = (creature ? creature->GetHealth() : 0);

	LuaScript::PushNumber(L, value);
	return 1;
}

int Creature::LuaGetCreatureSkull(lua_State* L) {
	unsigned int creatureID = LuaScript::PopNumber(L);

	Creature* creature = Creature::GetFromKnown(creatureID);
	double value = (creature ? creature->GetSkull() : 0);

	LuaScript::PushNumber(L, value);
	return 1;
}

int Creature::LuaGetCreatureShield(lua_State* L) {
	unsigned int creatureID = LuaScript::PopNumber(L);

	Creature* creature = Creature::GetFromKnown(creatureID);
	double value = (creature ? creature->GetShield() : 0);

	LuaScript::PushNumber(L, value);
	return 1;
}

int Creature::LuaGetCreatureWar(lua_State* L) {
	unsigned int creatureID = LuaScript::PopNumber(L);

	Creature* creature = Creature::GetFromKnown(creatureID);
	double value = (creature ? creature->GetWar() : 0);

	LuaScript::PushNumber(L, value);
	return 1;
}

int Creature::LuaGetCreatureStep(lua_State* L) {
	unsigned int creatureID = LuaScript::PopNumber(L);

	Creature* creature = Creature::GetFromKnown(creatureID);
	double value = (creature ? creature->GetStep() : 0);

	LuaScript::PushNumber(L, value);
	return 1;
}


void Creature::LuaRegisterFunctions(lua_State* L) {
	//setCreatureName(creatureID, name)
	lua_register(L, "setCreatureName", Creature::LuaSetCreatureName);

	//getCreatureID(name, radius = 0) : creatureID
	lua_register(L, "getCreatureID", Creature::LuaGetCreatureID);

	//setCreatureShine(creatureID, shine)
	lua_register(L, "setCreatureShine", Creature::LuaSetCreatureShine);

	//getCreatureShine(creatureID) : shine
	lua_register(L, "getCreatureShine", Creature::LuaGetCreatureShine);

	//getCreatureName(creatureID) : name
	lua_register(L, "getCreatureName", Creature::LuaGetCreatureName);

	//getCreaturePosition(creatureID) : position{x, y, z}
	lua_register(L, "getCreaturePosition", Creature::LuaGetCreaturePosition);

	//getCreatureHealth(creatureID) : health
	lua_register(L, "getCreatureHealth", Creature::LuaGetCreatureHealth);

	//getCreatureSkull(creatureID) : skull
	lua_register(L, "getCreatureSkull", Creature::LuaGetCreatureSkull);

	//getCreatureShield(creatureID) : shield
	lua_register(L, "getCreatureShield", Creature::LuaGetCreatureShield);

	//getCreatureWar(creatureID) : war
	lua_register(L, "getCreatureWar", Creature::LuaGetCreatureWar);

	//getCreatureStep(creatureID) : step
	lua_register(L, "getCreatureStep", Creature::LuaGetCreatureStep);
}
