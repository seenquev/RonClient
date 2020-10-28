/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#include "battle.h"

#include "allocator.h"
#include "creature.h"
#include "icons.h"
#include "logger.h"
#include "player.h"
#include "window.h"
#include "tools.h"


// ---- Battle ---- //

MUTEX Battle::lockBattle;



Battle::Battle() {
	container = NULL;
}

Battle::~Battle() {
}

void Battle::AddCreature(Creature* creature) {
	LOCKCLASS lockClass(lockBattle);

	Creature* playerCreature = Creature::GetFromKnown(Player::GetCreatureID());
	if (!creature || (creature == playerCreature))
		return;

	std::list<Creature*>::iterator it = std::find(battle.begin(), battle.end(), creature);
	if (it == battle.end())
		battle.push_back(creature);
}

void Battle::RemoveCreature(Creature* creature) {
	LOCKCLASS lockClass(lockBattle);

	Creature* playerCreature = Creature::GetFromKnown(Player::GetCreatureID());
	if (!creature || (creature == playerCreature))
		return;

	std::list<Creature*>::iterator it = std::find(battle.begin(), battle.end(), creature);
	if (it != battle.end())
		battle.erase(it);
}

Creature* Battle::GetCreature(int number) {
	LOCKCLASS lockClass(lockBattle);

	Creature* creature = NULL;
	if (number >= battle.size())
		return creature;

	std::list<Creature*>::iterator it = battle.begin();
	for (it; it != battle.end() && number > 0; it++, number--);

	if (it != battle.end())
		creature = *it;

	return creature;
}

std::list<Creature*> Battle::GetCreatures() {
	LOCKCLASS lockClass(lockBattle);

	return battle;
}

void Battle::ClearBattle() {
	LOCKCLASS lockClass(lockBattle);

	battle.clear();
}

int Battle::GetSize() {
	LOCKCLASS lockClass(lockBattle);

	return battle.size();
}


void Battle::SetContainer(WindowElementContainer* container) {
	LOCKCLASS lockClass(lockBattle);

	this->container = container;
}

void Battle::UpdateContainer() {
	LOCKCLASS lockClass(lockBattle);

	if (!container)
		return;

	container->DeleteAllElements();

	Window* window = container->GetWindow();
	WindowTemplate* wndTemplate = window->GetWindowTemplate();
	window->SetActiveElement(NULL);

	POINT size = window->GetSize(true);

	int posY = 0;
	std::list<Creature*>::iterator it = battle.begin();
	for (it; it != battle.end(); it++) {
		Creature* creature = *it;

		Creature* newCreature = new(M_PLACE) Creature(creature);
		newCreature->direction = SOUTH;
		newCreature->step = 0.0f;

		WindowElementItemContainer* itemCont = new(M_PLACE) WindowElementItemContainer;
		itemCont->Create(0, 5, posY, 20, 20, NULL, wndTemplate);
		itemCont->SetCreature(newCreature);

		int offset = 0;
		int sk = newCreature->GetSkull();
		int sh = newCreature->GetShield();
		AD2D_Image* skull = (sk != 0 ? Icons::GetSkullIcon(sk) : NULL);
		AD2D_Image* shield = (sh != 0 ? Icons::GetShieldIcon(sh) : NULL);

		if (skull) {
			WindowElementImage* skullImage = new(M_PLACE) WindowElementImage;
			skullImage->Create(0, 25 + offset, posY, 12, 12, skull, wndTemplate);
			offset += 14;

			container->AddElement(skullImage);
		}

		if (shield) {
			WindowElementImage* shieldImage = new(M_PLACE) WindowElementImage;
			shieldImage->Create(0, 25 + offset, posY, 12, 12, shield, wndTemplate);
			offset += 14;

			container->AddElement(shieldImage);
		}


		WindowElementText* text = new(M_PLACE) WindowElementText;
		text->Create(0, 25 + offset, posY, 0xFFFF, wndTemplate);
		text->SetText(creature->GetName());
		text->SetColor(0.8f, 0.8f, 0.8f);
		text->SetBorder(1);
		posY += 15;

		WindowElementSkillBar* bar = new(M_PLACE) WindowElementSkillBar;
		bar->Create(0, 25, posY, size.x - 51, 5, TypePointer("uint8", (void*)creature->GetHealthPtr()), wndTemplate);
		bar->SetColor(boost::bind(RT_ConvertColorHP, creature->GetHealthPtr()));
		bar->SetLocks(true, false);
		posY += 15;

		WindowElementBattle* batt = new(M_PLACE) WindowElementBattle;
		batt->Create(0, 0, posY - 30, size.x, 30, false, false, wndTemplate);
		batt->SetCreatureID(creature->GetID());
		batt->SetLocks(true, false);

		container->AddElement(itemCont);
		container->AddElement(text);
		container->AddElement(bar);
		container->AddElement(batt);
	}

	container->SetIntSize(0, posY);
}
