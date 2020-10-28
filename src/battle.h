/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#ifndef __BATTLE_H_
#define __BATTLE_H_

#include <list>

#include "mthread.h"


class Creature;
class WindowElementContainer;


class Battle {
private:
	WindowElementContainer* container;

public:
	std::list<Creature*> battle;

	static MUTEX lockBattle;

public:
	Battle();
	~Battle();

	void AddCreature(Creature* creature);
	void RemoveCreature(Creature* creature);
	Creature* GetCreature(int number);
	std::list<Creature*> GetCreatures();
	void ClearBattle();

	void SetContainer(WindowElementContainer* container);
	void UpdateContainer();

	int GetSize();
};

#endif //__BATTLE_H_
