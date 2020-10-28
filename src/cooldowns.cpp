/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#include "cooldowns.h"

#include "allocator.h"
#include "icons.h"
#include "logger.h"
#include "luascript.h"
#include "realtime.h"
#include "text.h"
#include "window.h"


// ---- Cooldown ---- //

Cooldown::Cooldown() {
	castTime = 0;
	period = 1;
}

Cooldown::~Cooldown() {
}

void Cooldown::Cast(unsigned int period) {
	castTime = RealTime::getTime();
	this->period = period;
}

bool Cooldown::CanCast() {
	if (RealTime::getTime() < castTime + period)
		return false;

	return true;
}

void Cooldown::SetComment(std::string text) {
	this->comment = text;
}

std::string Cooldown::GetComment() {
	return this->comment;
}


// ---- Cooldowns ---- //

CooldownMap	Cooldowns::groupCooldowns;
CooldownMap	Cooldowns::spellCooldowns;

int Cooldowns::threads = 0;
MUTEX Cooldowns::lockCooldowns;


Cooldowns::Cooldowns() {
	containerGroup = NULL;
	containerSpell = NULL;

	groupCooldowns[1] = new(M_PLACE) Cooldown;
	groupCooldowns[2] = new(M_PLACE) Cooldown;
	groupCooldowns[3] = new(M_PLACE) Cooldown;
	groupCooldowns[4] = new(M_PLACE) Cooldown;

	groupCooldowns[1]->SetComment(Text::GetText("COMMENT_2", Game::options.language));
	groupCooldowns[2]->SetComment(Text::GetText("COMMENT_3", Game::options.language));
	groupCooldowns[3]->SetComment(Text::GetText("COMMENT_4", Game::options.language));
	groupCooldowns[4]->SetComment(Text::GetText("COMMENT_5", Game::options.language));
}

Cooldowns::~Cooldowns() {
	LOCKCLASS lockClass(lockCooldowns);

	ClearCooldowns();
}

void Cooldowns::CastGroup(unsigned char id, unsigned int period) {
	LOCKCLASS lockClass(lockCooldowns);

	CooldownMap::iterator it = groupCooldowns.find(id);
	if (it != groupCooldowns.end()) {
		Cooldown* cooldown = it->second;
		if (cooldown)
			cooldown->Cast(period);
	}

	UpdateContainerGroup();
}

void Cooldowns::CastSpell(unsigned char id, unsigned int period) {
	LOCKCLASS lockClass(lockCooldowns);

	CooldownMap::iterator it = groupCooldowns.find(id);
	if (it != groupCooldowns.end())
		RemoveCooldown(id);

	Cooldown* cooldown = new(M_PLACE) Cooldown;
	if (cooldown)
		cooldown->Cast(period);

	AddCooldown(id, cooldown);
	UpdateContainerSpell();
}

void Cooldowns::AddCooldown(unsigned char id, Cooldown* cooldown) {
	LOCKCLASS lockClass(lockCooldowns);

	CooldownMap::iterator it = spellCooldowns.find(id);
	if (it == spellCooldowns.end())
		spellCooldowns[id] = cooldown;
	else {
		RemoveCooldown(id);
		spellCooldowns[id] = cooldown;
	}
}

void Cooldowns::RemoveCooldown(unsigned char id) {
	LOCKCLASS lockClass(lockCooldowns);

	CooldownMap::iterator it = spellCooldowns.find(id);
	if (it != spellCooldowns.end()) {
		Cooldown* cooldown = it->second;
		if (cooldown)
			delete_debug(cooldown, M_PLACE);

		spellCooldowns.erase(it);
	}
}

void Cooldowns::ClearCooldowns() {
	LOCKCLASS lockClass(lockCooldowns);

	CooldownMap::iterator it = groupCooldowns.begin();
	for (it; it != groupCooldowns.end(); it++) {
		Cooldown* cooldown = it->second;
		if (cooldown)
			delete_debug(cooldown, M_PLACE);
	}

	it = spellCooldowns.begin();
	for (it; it != spellCooldowns.end(); it++) {
		Cooldown* cooldown = it->second;
		if (cooldown)
			delete_debug(cooldown, M_PLACE);
	}

	groupCooldowns.clear();
	spellCooldowns.clear();
}

Cooldown* Cooldowns::GetGroupCooldown(unsigned char id) {
	LOCKCLASS lockClass(lockCooldowns);

	CooldownMap::iterator it = groupCooldowns.find(id);
	if (it != groupCooldowns.end())
		return it->second;

	return NULL;
}

Cooldown* Cooldowns::GetCooldown(unsigned char id) {
	LOCKCLASS lockClass(lockCooldowns);

	CooldownMap::iterator it = spellCooldowns.find(id);
	if (it != spellCooldowns.end())
		return it->second;

	return NULL;
}

void Cooldowns::CheckCooldowns(Game* game) {
	if (threads > 0)
		return;

	threads++;

	while(game->GetGameState() == GAME_LOGGEDTOGAME) {
		bool update = false;

		lockCooldowns.lock();

		Cooldowns* cooldowns = game->GetCooldowns();
		if (cooldowns) {
			std::list<unsigned char> toDelete;

			CooldownMap::iterator it = spellCooldowns.begin();
			for (it; it != spellCooldowns.end(); it++) {
				Cooldown* cooldown = it->second;

				if (cooldown->CanCast())
					toDelete.push_back(it->first);
			}

			if (!toDelete.empty()) {
				std::list<unsigned char>::iterator it = toDelete.begin();
				for (it; it != toDelete.end(); it++) {
					unsigned char spellID = *it;
					cooldowns->RemoveCooldown(spellID);
				}

				update = true;
			}
		}

		lockCooldowns.unlock();

		if (update)
			cooldowns->UpdateContainerSpell();

		Sleep(50);
	}

	threads--;
}

void Cooldowns::SetContainerGroup(WindowElementContainer* container) {
	LOCKCLASS lockClass(lockCooldowns);

	this->containerGroup = container;
}

void Cooldowns::SetContainerSpell(WindowElementContainer* container) {
	LOCKCLASS lockClass(lockCooldowns);

	this->containerSpell = container;
}

void Cooldowns::UpdateContainerGroup() {
	LOCKCLASS lockClass1(Windows::lockWindows);
	LOCKCLASS lockClass2(lockCooldowns);

	if (!containerGroup)
		return;

	containerGroup->DeleteAllElements();

	Window* window = containerGroup->GetWindow();
	WindowTemplate* wndTemplate = window->GetWindowTemplate();
	window->SetActiveElement(NULL);

	POINT size_ext = containerGroup->GetIntSize();

	int i = 0;
	CooldownMap::iterator it = groupCooldowns.begin();
	for (it; it != groupCooldowns.end(); it++, i++) {
		unsigned char groupID = it->first;
		Cooldown* cooldown = it->second;

		if (!cooldown)
			continue;

		WindowElementCooldown* wcooldown = new(M_PLACE) WindowElementCooldown;
		wcooldown->Create(0, 0, i * 32, 32, 32, wndTemplate);
		wcooldown->SetGroupID(groupID);
		wcooldown->SetCast(cooldown->castTime, cooldown->period);
		wcooldown->SetIcon(Icons::GetSpellGroupIcon(groupID));
		wcooldown->SetComment(cooldown->GetComment());

		containerGroup->AddElement(wcooldown);
	}
}

void Cooldowns::UpdateContainerSpell() {
	LOCKCLASS lockClass1(Windows::lockWindows);
	LOCKCLASS lockClass2(lockCooldowns);

	if (!containerSpell)
		return;

	containerSpell->DeleteAllElements();

	Window* window = containerSpell->GetWindow();
	WindowTemplate* wndTemplate = window->GetWindowTemplate();
	window->SetActiveElement(NULL);

	POINT size_ext = containerSpell->GetIntSize();

	int i = 0;
	CooldownMap::iterator it = spellCooldowns.begin();
	for (it; it != spellCooldowns.end(); it++, i++) {
		unsigned char spellID = it->first;
		Cooldown* cooldown = it->second;

		if (!cooldown)
			continue;

		WindowElementCooldown* wcooldown = new(M_PLACE) WindowElementCooldown;
		wcooldown->Create(0, 0, i * 32, 32, 32, wndTemplate);
		wcooldown->SetSpellID(spellID);
		wcooldown->SetCast(cooldown->castTime, cooldown->period);
		wcooldown->SetIcon(Icons::GetSpellIcon(spellID));
		wcooldown->SetComment(cooldown->GetComment());

		containerSpell->AddElement(wcooldown);
	}
}


//Lua functions

int Cooldowns::LuaNewCooldown(lua_State* L) {
	Cooldown* cooldown = new(M_PLACE) Cooldown;

	LuaScript::PushNumber(L, (unsigned long)cooldown);
	return 1;
}

int Cooldowns::LuaDeleteCooldown(lua_State* L) {
	Cooldown* cooldown = (Cooldown*)((unsigned long)LuaScript::PopNumber(L));
	if (cooldown)
		delete_debug(cooldown, M_PLACE);

	return 1;
}

int Cooldowns::LuaAddCooldown(lua_State* L) {
	Cooldown* cooldown = (Cooldown*)((unsigned long)LuaScript::PopNumber(L));
	int id = LuaScript::PopNumber(L);

	Game* game = Game::game;
	Cooldowns* cooldowns = (game ? game->GetCooldowns() : NULL);
	if (cooldowns && cooldown)
		cooldowns->AddCooldown(id, cooldown);

	return 1;
}

int Cooldowns::LuaRemoveCooldown(lua_State* L) {
	int id = LuaScript::PopNumber(L);

	Game* game = Game::game;
	Cooldowns* cooldowns = (game ? game->GetCooldowns() : NULL);
	if (cooldowns)
		cooldowns->RemoveCooldown(id);

	return 1;
}

int Cooldowns::LuaCastCooldown(lua_State* L) {
	unsigned long period = LuaScript::PopNumber(L);
	int id = LuaScript::PopNumber(L);

	Game* game = Game::game;
	Cooldowns* cooldowns = (game ? game->GetCooldowns() : NULL);

	if (cooldowns)
		cooldowns->CastSpell(id, period);

	return 1;
}

int Cooldowns::LuaCastCooldownGroup(lua_State* L) {
	unsigned long period = LuaScript::PopNumber(L);
	int groupid = LuaScript::PopNumber(L);

	Game* game = Game::game;
	Cooldowns* cooldowns = (game ? game->GetCooldowns() : NULL);

	if (cooldowns)
		cooldowns->CastGroup(groupid, period);

	return 1;
}


void Cooldowns::LuaRegisterFunctions(lua_State* L) {
	//newCooldown() : cooldownPtr
	lua_register(L, "newCooldown", Cooldowns::LuaNewCooldown);

	//deleteCooldown(cooldownPtr)
	lua_register(L, "deleteCooldown", Cooldowns::LuaDeleteCooldown);

	//addCooldown(id, cooldownPtr)
	lua_register(L, "addCooldown", Cooldowns::LuaAddCooldown);

	//removeCooldown(id)
	lua_register(L, "removeCooldown", Cooldowns::LuaRemoveCooldown);

	//castCooldown(id, period)
	lua_register(L, "castCooldown", Cooldowns::LuaCastCooldown);

	//castCooldownGroup(groupid, period)
	lua_register(L, "castCooldownGroup", Cooldowns::LuaCastCooldownGroup);
}
