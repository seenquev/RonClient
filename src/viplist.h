/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#ifndef __VIPLIST_H_
#define __VIPLIST_H_

#include <map>
#include <string>

#include "mthread.h"


class WindowElementContainer;


class VIPList {
private:
	WindowElementContainer* container;
	
public:
	std::map<unsigned int, std::string> viplist_offline;
	std::map<unsigned int, std::string> viplist_online;
	
	MUTEX lockVIPList;
	
public:
	VIPList();
	~VIPList();
	
	void AddCreature(unsigned int creatureID, std::string creatureName, bool online);
	void LoginCreature(unsigned int creatureID);
	void LogoutCreature(unsigned int creatureID);
	void RemoveCreature(unsigned int creatureID);
	void ClearVIPList();
	
	unsigned int GetCreatureID(std::string creatureName);
	std::string GetCreatureName(unsigned int creatureID);
	
	void SetContainer(WindowElementContainer* container);
	void UpdateContainer();
	
	int GetSize();
};

#endif //__VIPLIST_H_
