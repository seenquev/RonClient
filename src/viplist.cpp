/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#include "viplist.h"

#include "allocator.h"
#include "creature.h"
#include "window.h"
#include "tools.h"


// ---- VIPList ---- //

VIPList::VIPList() {
	container = NULL;
}

VIPList::~VIPList() { }

void VIPList::AddCreature(unsigned int creatureID, std::string creatureName, bool online) {
	LOCKCLASS lockClass(lockVIPList);

	if (online) {
		std::map<unsigned int, std::string>::iterator it = viplist_online.find(creatureID);
		if (it == viplist_online.end())
			viplist_online[creatureID] = creatureName;
	}
	else {
		std::map<unsigned int, std::string>::iterator it = viplist_offline.find(creatureID);
		if (it == viplist_offline.end())
			viplist_offline[creatureID] = creatureName;
	}
}

void VIPList::LoginCreature(unsigned int creatureID) {
	LOCKCLASS lockClass(lockVIPList);

	std::map<unsigned int, std::string>::iterator it = viplist_offline.find(creatureID);
	if (it != viplist_offline.end()) {
		AddCreature(it->first, it->second, true);
		viplist_offline.erase(it);
	}
}

void VIPList::LogoutCreature(unsigned int creatureID) {
	LOCKCLASS lockClass(lockVIPList);

	std::map<unsigned int, std::string>::iterator it = viplist_online.find(creatureID);
	if (it != viplist_online.end()) {
		AddCreature(it->first, it->second, false);
		viplist_online.erase(it);
	}
}

void VIPList::RemoveCreature(unsigned int creatureID) {
	LOCKCLASS lockClass(lockVIPList);

	std::map<unsigned int, std::string>::iterator it = viplist_offline.find(creatureID);
	if (it != viplist_offline.end())
		viplist_offline.erase(it);

	it = viplist_online.find(creatureID);
	if (it != viplist_online.end())
		viplist_online.erase(it);
}

void VIPList::ClearVIPList() {
	LOCKCLASS lockClass(lockVIPList);

	viplist_offline.clear();
	viplist_online.clear();
}

unsigned int VIPList::GetCreatureID(std::string creatureName) {
	LOCKCLASS lockClass(lockVIPList);

	std::map<unsigned int, std::string>::iterator it = viplist_offline.begin();
	for (it; it != viplist_offline.end(); it++) {
		if (it->second == creatureName)
			return it->first;
	}

	it = viplist_online.begin();
	for (it; it != viplist_online.end(); it++) {
		if (it->second == creatureName)
			return it->first;
	}

	return 0;
}

std::string VIPList::GetCreatureName(unsigned int creatureID) {
	LOCKCLASS lockClass(lockVIPList);

	std::map<unsigned int, std::string>::iterator it = viplist_offline.find(creatureID);
	if (it != viplist_offline.end())
		return it->second;

	it = viplist_online.find(creatureID);
	if (it != viplist_online.end())
		return it->second;

	return "";
}

void VIPList::SetContainer(WindowElementContainer* container) {
	LOCKCLASS lockClass(lockVIPList);

	this->container = container;
}

void VIPList::UpdateContainer() {
	LOCKCLASS lockClass1(Windows::lockWindows);
	LOCKCLASS lockClass2(lockVIPList);

	if (!container)
		return;

	container->DeleteAllElements();

	Window* window = container->GetWindow();
	window->SetActiveElement(NULL);

	POINT size = window->GetSize(true);

	WindowElementVIP* vip = new(M_PLACE) WindowElementVIP;
	vip->Create(0, 0, 0, 0xFFFF, 0xFFFF, false, false, window->GetWindowTemplate());
	vip->SetVIPList(this);
	vip->SetCreatureID(0x00);
	window->AddElement(vip);

	std::map<std::string, unsigned int> s_online;
	std::map<std::string, unsigned int> s_offline;

	std::map<unsigned int, std::string>::iterator _it = viplist_online.begin();
	for (_it; _it != viplist_online.end(); _it++)
		s_online[_it->second] = _it->first;

	_it = viplist_offline.begin();
	for (_it; _it != viplist_offline.end(); _it++)
		s_offline[_it->second] = _it->first;

	int posY = 0;
	std::map<std::string, unsigned int>::iterator it = s_online.begin();
	for (it; it != s_online.end(); it++) {
		std::string creatureName = it->first;
		unsigned int creatureID = it->second;

		WindowElementText* text = new(M_PLACE) WindowElementText;
		text->Create(0, 5, posY, 0xFFFF, window->GetWindowTemplate());
		text->SetText(creatureName);
		text->SetColor(0.0f, 1.0f, 0.0f);
		text->SetBorder(1);
		posY += 15;

		WindowElementVIP* vip = new(M_PLACE) WindowElementVIP;
		vip->Create(0, 0, posY - 15, 0xFFFF, 15, false, false, window->GetWindowTemplate());
		vip->SetVIPList(this);
		vip->SetCreatureID(creatureID);

		container->AddElement(text);
		container->AddElement(vip);
	}

	it = s_offline.begin();
	for (it; it != s_offline.end(); it++) {
		std::string creatureName = it->first;
		unsigned int creatureID = it->second;

		WindowElementText* text = new(M_PLACE) WindowElementText;
		text->Create(0, 5, posY, 0xFFFF, window->GetWindowTemplate());
		text->SetText(creatureName);
		text->SetColor(0.8f, 0.0f, 0.0f);
		text->SetBorder(1);
		posY += 15;

		WindowElementVIP* vip = new(M_PLACE) WindowElementVIP;
		vip->Create(0, 0, posY - 15, 0xFFFF, 15, false, false, window->GetWindowTemplate());
		vip->SetVIPList(this);
		vip->SetCreatureID(creatureID);

		container->AddElement(text);
		container->AddElement(vip);
	}

	container->SetIntSize(0, posY);
}
