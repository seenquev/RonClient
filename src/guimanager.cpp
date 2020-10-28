/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#include "guimanager.h"

#include "allocator.h"
#include "game.h"
#include "window.h"


// ---- GUIConnection ---- //

GUIConnection::GUIConnection() { }

GUIConnection::GUIConnection(Window* w1, Window* w2) : wnd1(w1), wnd2(w2) { }

GUIConnection::~GUIConnection() { }


Window* GUIConnection::GetFirstWindow() {
	return wnd1;
}

Window* GUIConnection::GetSecondWindow() {
	return wnd2;
}

Window* GUIConnection::GetOtherWindow(Window* wnd) {
	if (wnd == wnd1)
		return wnd2;
	else if (wnd == wnd2)
		return wnd1;

	return NULL;
}

bool GUIConnection::ContainWindow(Window* wnd) {
	return (wnd == wnd1 || wnd == wnd2);
}


void GUIConnection::SetAction(Window* wnd, GUI_ACTIONS actionMain, GUI_ACTIONS actionOther) {
	actions[wnd][actionMain] = actionOther;
}


// ---- GUIManager ---- //

GUIManager::GUIManager() {
	guiMaker = NULL;
	guiKeyChecker = NULL;
}

GUIManager::~GUIManager() {
	CleanUp();
}


void GUIManager::CleanUp() {
	if (guiMaker) {
		delete_debug(guiMaker, M_PLACE);
		guiMaker = NULL;
	}
	if (guiKeyChecker) {
		delete_debug(guiKeyChecker, M_PLACE);
		guiKeyChecker = NULL;
	}
}


void GUIManager::AddWindow(Window* wnd) {
	LOCKCLASS lockClass(lockGUIManager);

	windows.insert(wnd);
}

void GUIManager::RemoveWindow(Window* wnd) {
	LOCKCLASS lockClass(lockGUIManager);

	WindowsSet::iterator it = windows.find(wnd);
	if (it != windows.end())
		windows.erase(it);

	RemoveConnections(wnd);
	ClearActionsOnSetPosition(wnd);
	ClearActionsOnSetSize(wnd);
}

void GUIManager::ClearWindows() {
	LOCKCLASS lockClass(lockGUIManager);

	WindowsSet::iterator it = windows.begin();
	for (it; it != windows.end(); it++)
		RemoveConnections(*it);

	onSetPosition.clear();
	onSetSize.clear();

	windows.clear();
}

WindowsSet GUIManager::GetWindows() {
	LOCKCLASS lockClass(lockGUIManager);

	return windows;
}

bool GUIManager::ContainWindow(Window* wnd) {
	LOCKCLASS lockClass(lockGUIManager);

	WindowsSet::iterator it = windows.find(wnd);
	if (it != windows.end())
		return true;

	return false;
}


GUIConnection* GUIManager::AddConnection(Window* wnd1, Window* wnd2) {
	LOCKCLASS lockClass(lockGUIManager);

	GUIConnectionsMap::iterator it1 = connectionsMap.find(wnd1);
	GUIConnectionsMap::iterator it2 = connectionsMap.find(wnd2);

	GUIConnectionsSet connections1 = (it1 != connectionsMap.end() ? it1->second : GUIConnectionsSet());
	GUIConnectionsSet connections2 = (it2 != connectionsMap.end() ? it2->second : GUIConnectionsSet());

	GUIConnectionsSet::iterator cit1 = connections1.begin();
	for (cit1; cit1 != connections1.end(); cit1++) {
		GUIConnection* connection = *cit1;
		GUIConnectionsSet::iterator cit2 = connections2.find(connection);
		if (cit2 != connections2.end())
			return connection;
	}

	GUIConnection* connection = new(M_PLACE) GUIConnection(wnd1, wnd2);
	connectionsMap[wnd1].insert(connection);
	connectionsMap[wnd2].insert(connection);
	return connection;
}

void GUIManager::RemoveConnection(GUIConnection* connection) {
	LOCKCLASS lockClass(lockGUIManager);

	Window* wnd1 = connection->GetFirstWindow();
	Window* wnd2 = connection->GetSecondWindow();

	GUIConnectionsMap::iterator it1 = connectionsMap.find(wnd1);
	if (it1 != connectionsMap.end()) {
		GUIConnectionsSet connections1 = it1->second;

		GUIConnectionsSet::iterator cit1 = connections1.find(connection);
		if (cit1 != connections1.end())
			connections1.erase(cit1);

		if (connections1.size() == 0)
			connectionsMap.erase(it1);
		else
			it1->second = connections1;
	}

	GUIConnectionsMap::iterator it2 = connectionsMap.find(wnd2);
	if (it2 != connectionsMap.end()) {
		GUIConnectionsSet connections2 = it2->second;

		GUIConnectionsSet::iterator cit2 = connections2.find(connection);
		if (cit2 != connections2.end())
			connections2.erase(cit2);

		if (connections2.size() == 0)
			connectionsMap.erase(it2);
		else
			it2->second = connections2;
	}

	delete_debug(connection, M_PLACE);
}

void GUIManager::RemoveConnections(Window* wnd) {
	LOCKCLASS lockClass(lockGUIManager);

	GUIConnectionsMap::iterator it1 = connectionsMap.find(wnd);
	if (it1 != connectionsMap.end()) {
		GUIConnectionsSet connections = it1->second;
		connectionsMap.erase(it1);

		GUIConnectionsSet::iterator cit1 = connections.begin();
		for (cit1; cit1 != connections.end(); cit1++) {
			GUIConnection* connection = *cit1;

			Window* wnd2 = connection->GetOtherWindow(wnd);
			GUIConnectionsMap::iterator it2 = connectionsMap.find(wnd2);

			if (it2 != connectionsMap.end()) {
				GUIConnectionsSet connections = it2->second;
				GUIConnectionsSet::iterator cit2 = connections.find(connection);
				if (cit2 != connections.end())
					connections.erase(cit2);

				if (connections.size() == 0)
					connectionsMap.erase(it2);
				else
					connectionsMap[wnd2] = connections;
			}

			delete_debug(connection, M_PLACE);
		}
	}
}

GUIConnectionsSet GUIManager::GetConnections(Window* wnd) {
	LOCKCLASS lockClass(lockGUIManager);

	GUIConnectionsMap::iterator it = connectionsMap.find(wnd);
	if (it != connectionsMap.end())
		return it->second;

	GUIConnectionsSet ret;
	return ret;
}


void GUIManager::MakeGUI() {
	Game* game = Game::game;
	Windows* wnds = Windows::wnds;

	if (!game || !wnds)
		return;

	std::string filename = LuaScript::GetFilePath("makegui");
	guiMaker = new(M_PLACE) LuaScript;
	if (guiMaker->OpenScript(filename)) {
		guiMaker->Execute("main", LuaScript::ConvertArgs(""));
		return;
	}

	Window* wstatus = game->GetWindowStatus();
	Window* whotkeys = game->GetWindowHotkeys();
	Window* wgame = game->GetWindowGame();
	Window* wconsole = game->GetWindowConsole();
	Window* winventory = game->GetWindowInventory();
	if (!wstatus) wstatus = wnds->OpenWindow(WND_STATUS, game);
	if (!whotkeys) whotkeys = wnds->OpenWindow(WND_HOTKEYS, game);
	if (!wgame) wgame = wnds->OpenWindow(WND_GAME, game);
	if (!wconsole) wconsole = wnds->OpenWindow(WND_CONSOLE, game);
	if (!winventory) winventory = wnds->OpenWindow(WND_INVENTORY, game);

	GUIConnection* conn = AddConnection(wgame, wconsole);
	if (conn) {
		conn->SetAction(wgame, GUIACTION_RESIZE_BOTTOM, GUIACTION_RESIZE_TOP);
		conn->SetAction(wgame, GUIACTION_RESIZE_LEFT, GUIACTION_RESIZE_LEFT);
		conn->SetAction(wgame, GUIACTION_RESIZE_RIGHT, GUIACTION_RESIZE_RIGHT);
		conn->SetAction(wconsole, GUIACTION_RESIZE_TOP, GUIACTION_RESIZE_BOTTOM);
		conn->SetAction(wconsole, GUIACTION_RESIZE_LEFT, GUIACTION_RESIZE_LEFT);
		conn->SetAction(wconsole, GUIACTION_RESIZE_RIGHT, GUIACTION_RESIZE_RIGHT);
		conn->SetAction(wconsole, GUIACTION_MOVE_TOP, GUIACTION_RESIZE_BOTTOM);
		conn->SetAction(wconsole, GUIACTION_MOVE_BOTTOM, GUIACTION_RESIZE_BOTTOM);
	}

	POINT wndSize = wnds->GetWindowSize();

	wgame->SetMinSize(0, 100);
	wgame->SetMaxSize(0, wndSize.y - 100);
	wconsole->SetMinSize(0, 100);
	wconsole->SetMaxSize(0, wndSize.y - 100);

	AddActionOnResizeMainWindow(boost::bind(&Windows::FitGameWindow, wnds));
	AddActionOnSetSize(wgame, boost::bind(&Windows::MatchGameWindows, wnds));
}

bool GUIManager::CheckGUIKey(Keyboard* keyboard) {
	Game* game = Game::game;
	Windows* wnds = Windows::wnds;

	if (!game || !wnds)
		return false;

	if (!guiKeyChecker) {
		std::string filename = LuaScript::GetFilePath("checkguikey");
		guiKeyChecker = new(M_PLACE) LuaScript;
		if (guiKeyChecker->OpenScript(filename)) {
			LuaArguments args;
			args.push_back(value2str((unsigned int)keyboard));
			return guiKeyChecker->Execute("main", args);
		}
	}
	else {
		if (guiKeyChecker->IsLoaded()) {
			LuaArguments args;
			args.push_back(value2str((unsigned int)keyboard));
			return guiKeyChecker->Execute("main", args);
		}
	}

	if (keyboard->key[VK_CONTROL] && keyboard->keyChar == 13) {
		game->onMakeAction(wnds, NULL, ACTION_OPENWINDOW, (void*)(unsigned int)WND_MINIMAP);
		return true;
	}
	else if (keyboard->key[VK_CONTROL] && keyboard->keyChar == 9) {
		game->onMakeAction(wnds, NULL, ACTION_OPENWINDOW, (void*)(unsigned int)WND_INVENTORY);
		return true;
	}
	else if (!keyboard->capsLock && keyboard->key[VK_CONTROL] && keyboard->keyChar == 19) {
		game->onMakeAction(wnds, NULL, ACTION_OPENWINDOW, (void*)(unsigned int)WND_STATISTICS);
		return true;
	}
	else if (keyboard->key[VK_CONTROL] && keyboard->keyChar == 2) {
		game->onMakeAction(wnds, NULL, ACTION_OPENWINDOW, (void*)(unsigned int)WND_BATTLE);
		return true;
	}
	else if (keyboard->key[VK_CONTROL] && keyboard->keyChar == 14) {
		game->onMakeAction(wnds, NULL, ACTION_OPENWINDOW, (void*)(unsigned int)WND_VIPLIST);
		return true;
	}
	else if (keyboard->key[VK_CONTROL] && keyboard->keyChar == 26) {
		game->onMakeAction(wnds, NULL, ACTION_OPENWINDOW, (void*)(unsigned int)WND_BOT);
		return true;
	}
	else if (keyboard->key[VK_CONTROL] && keyboard->keyChar == 8) {
		game->onMakeAction(wnds, NULL, ACTION_OPENWINDOW, (void*)(unsigned int)WND_OLDHOTKEYS);
		return true;
	}
	else if (keyboard->key[VK_CONTROL] && keyboard->keyChar == 15) {
		game->PlayerRequestChannels();
		return true;
	}
	else if (keyboard->key[VK_CONTROL] && keyboard->keyChar == 17) {
		game->PlayerRequestQuestLog();
		return true;
	}
	else if (keyboard->key[VK_CONTROL] && keyboard->keyChar == 11) {
		game->PlayerRequestSpells();
		return true;
	}

	return false;
}


void GUIManager::AddActionOnResizeMainWindow(boost::function<void()> func) {
	LOCKCLASS lockClass(lockGUIManager);

	onResizeMainWindow.PushFunction(func);
}

void GUIManager::AddActionOnSetPosition(Window* wnd, boost::function<void()> func) {
	if (!ContainWindow(wnd))
		return;

	LOCKCLASS lockClass(lockGUIManager);

	onSetPosition[wnd].PushFunction(func);
}

void GUIManager::AddActionOnSetSize(Window* wnd, boost::function<void()> func) {
	if (!ContainWindow(wnd))
		return;

	LOCKCLASS lockClass(lockGUIManager);

	onSetSize[wnd].PushFunction(func);
}

bool GUIManager::ExecuteOnResizeMainWindow() {
	LOCKCLASS lockClass(lockGUIManager);

	if (onResizeMainWindow.IsExecutable()) {
		onResizeMainWindow.Execute();
		return true;
	}

	return false;
}

bool GUIManager::ExecuteOnSetPosition(Window* wnd) {
	if (!ContainWindow(wnd))
		return false;

	LOCKCLASS lockClass(lockGUIManager);

	if (onSetPosition[wnd].IsExecutable()) {
		onSetPosition[wnd].Execute();
		return true;
	}

	return false;
}

bool GUIManager::ExecuteOnSetSize(Window* wnd) {
	if (!ContainWindow(wnd))
		return false;

	LOCKCLASS lockClass(lockGUIManager);

	if (onSetSize[wnd].IsExecutable()) {
		onSetSize[wnd].Execute();
		return true;
	}

	return false;
}

void GUIManager::ClearActionsOnResizeMainWindow() {
	LOCKCLASS lockClass(lockGUIManager);

	onResizeMainWindow.Clear();
}

void GUIManager::ClearActionsOnSetPosition(Window* wnd) {
	if (!ContainWindow(wnd))
		return;

	LOCKCLASS lockClass(lockGUIManager);

	onSetPosition[wnd].Clear();
}

void GUIManager::ClearActionsOnSetSize(Window* wnd) {
	if (!ContainWindow(wnd))
		return;

	LOCKCLASS lockClass(lockGUIManager);

	onSetSize[wnd].Clear();
}


void GUIManager::OnWindowChangePosition(Window* wnd, int dx, int dy, GUIConnection* excludeConnection) {
	if (!ContainWindow(wnd))
		return;

	LOCKCLASS lockClass(lockGUIManager);

	GUIConnectionsSet connections = GetConnections(wnd);
	GUIConnectionsSet::iterator cit = connections.begin();
	for (cit; cit != connections.end(); cit++) {
		GUIConnection* connection = *cit;
		if (connection == excludeConnection)
			continue;

		Window* wnd2 = connection->GetOtherWindow(wnd);

		GUIWindowActionsMap::iterator it = connection->actions.find(wnd);
		if (it != connection->actions.end()) {
			GUIActionMap::iterator ait = it->second.begin();
			for (ait; ait != it->second.end(); ait++) {
				if (dy < 0 && ait->first == GUIACTION_MOVE_TOP && ait->second == GUIACTION_MOVE_TOP) {
					wnd2->SetPosition(wnd2->posX, wnd2->posY + dy, connection);
				}
				else if (dy < 0 && ait->first == GUIACTION_MOVE_TOP && ait->second == GUIACTION_RESIZE_TOP) {
					wnd2->SetSize(wnd2->width, -(wnd2->height - dy), connection);
					wnd2->SetPosition(wnd2->posX, wnd2->posY + dy, connection);
				}
				else if (dy < 0 && ait->first == GUIACTION_MOVE_TOP && ait->second == GUIACTION_RESIZE_BOTTOM) {
					wnd2->SetSize(wnd2->width, wnd2->height + dy, connection);
				}
				else if (dy > 0 && ait->first == GUIACTION_MOVE_BOTTOM && ait->second == GUIACTION_MOVE_BOTTOM) {
					wnd2->SetPosition(wnd2->posX, wnd2->posY + dy, connection);
				}
				else if (dy > 0 && ait->first == GUIACTION_MOVE_BOTTOM && ait->second == GUIACTION_RESIZE_TOP) {
					wnd2->SetSize(wnd2->width, -(wnd2->height - dy), connection);
					wnd2->SetPosition(wnd2->posX, wnd2->posY + dy, connection);
				}
				else if (dy > 0 && ait->first == GUIACTION_MOVE_BOTTOM && ait->second == GUIACTION_RESIZE_BOTTOM) {
					wnd2->SetSize(wnd2->width, wnd2->height + dy, connection);
				}
				else if (dx < 0 && ait->first == GUIACTION_MOVE_LEFT && ait->second == GUIACTION_MOVE_LEFT) {
					wnd2->SetPosition(wnd2->posX + dx, wnd2->posY, connection);
				}
				else if (dx < 0 && ait->first == GUIACTION_MOVE_LEFT && ait->second == GUIACTION_RESIZE_LEFT) {
					wnd2->SetSize(-(wnd2->width - dx), wnd2->height, connection);
					wnd2->SetPosition(wnd2->posX + dx, wnd2->posY, connection);
				}
				else if (dx < 0 && ait->first == GUIACTION_MOVE_LEFT && ait->second == GUIACTION_RESIZE_RIGHT) {
					wnd2->SetSize(wnd2->width + dx, wnd2->height, connection);
				}
				else if (dx > 0 && ait->first == GUIACTION_MOVE_RIGHT && ait->second == GUIACTION_MOVE_RIGHT) {
					wnd2->SetPosition(wnd2->posX + dx, wnd2->posY, connection);
				}
				else if (dx > 0 && ait->first == GUIACTION_MOVE_RIGHT && ait->second == GUIACTION_RESIZE_LEFT) {
					wnd2->SetSize(-(wnd2->width - dx), wnd2->height, connection);
					wnd2->SetPosition(wnd2->posX + dx, wnd2->posY, connection);
				}
				else if (dx > 0 && ait->first == GUIACTION_MOVE_RIGHT && ait->second == GUIACTION_RESIZE_RIGHT) {
					wnd2->SetSize(wnd2->width + dx, wnd2->height, connection);
				}
			}
		}
	}
}

void GUIManager::OnWindowChangeSize(Window* wnd, int dx, int dy, bool negative, GUIConnection* excludeConnection) {
	if (!ContainWindow(wnd))
		return;

	LOCKCLASS lockClass(lockGUIManager);

	GUIConnectionsSet connections = GetConnections(wnd);
	GUIConnectionsSet::iterator cit = connections.begin();
	for (cit; cit != connections.end(); cit++) {
		GUIConnection* connection = *cit;
		if (connection == excludeConnection)
			continue;

		Window* wnd2 = connection->GetOtherWindow(wnd);

		GUIWindowActionsMap::iterator it = connection->actions.find(wnd);
		if (it != connection->actions.end()) {
			GUIActionMap::iterator ait = it->second.begin();
			for (ait; ait != it->second.end(); ait++) {
				if (dy != 0 && negative && ait->first == GUIACTION_RESIZE_TOP && ait->second == GUIACTION_MOVE_TOP) {
					wnd2->SetPosition(wnd2->posX, wnd2->posY + dy, connection);
				}
				else if (dy != 0 && negative && ait->first == GUIACTION_RESIZE_TOP && ait->second == GUIACTION_RESIZE_TOP) {
					wnd2->SetSize(wnd2->width, -(wnd2->height - dy), connection);
					wnd2->SetPosition(wnd2->posX, wnd2->posY + dy, connection);
				}
				else if (dy != 0 && negative && ait->first == GUIACTION_RESIZE_TOP && ait->second == GUIACTION_RESIZE_BOTTOM) {
					wnd2->SetSize(wnd2->width, wnd2->height + dy, connection);
				}
				else if (dy != 0 && !negative && ait->first == GUIACTION_RESIZE_BOTTOM && ait->second == GUIACTION_MOVE_BOTTOM) {
					wnd2->SetPosition(wnd2->posX, wnd2->posY + dy, connection);
				}
				else if (dy != 0 && !negative && ait->first == GUIACTION_RESIZE_BOTTOM && ait->second == GUIACTION_RESIZE_TOP) {
					wnd2->SetSize(wnd2->width, -(wnd2->height - dy), connection);
					wnd2->SetPosition(wnd2->posX, wnd2->posY + dy, connection);
				}
				else if (dy != 0 && !negative && ait->first == GUIACTION_RESIZE_BOTTOM && ait->second == GUIACTION_RESIZE_BOTTOM) {
					wnd2->SetSize(wnd2->width, wnd2->height + dy, connection);
				}
				else if (dx != 0 && negative && ait->first == GUIACTION_RESIZE_LEFT && ait->second == GUIACTION_MOVE_LEFT) {
					wnd2->SetPosition(wnd2->posX + dx, wnd2->posY, connection);
				}
				else if (dx != 0 && negative && ait->first == GUIACTION_RESIZE_LEFT && ait->second == GUIACTION_RESIZE_LEFT) {
					wnd2->SetSize(-(wnd2->width - dx), wnd2->height, connection);
					wnd2->SetPosition(wnd2->posX + dx, wnd2->posY, connection);
				}
				else if (dx != 0 && negative && ait->first == GUIACTION_RESIZE_LEFT && ait->second == GUIACTION_RESIZE_RIGHT) {
					wnd2->SetSize(wnd2->width + dx, wnd2->height, connection);
				}
				else if (dx != 0 && !negative && ait->first == GUIACTION_RESIZE_RIGHT && ait->second == GUIACTION_MOVE_RIGHT) {
					wnd2->SetPosition(wnd2->posX + dx, wnd2->posY, connection);
				}
				else if (dx != 0 && !negative && ait->first == GUIACTION_RESIZE_RIGHT && ait->second == GUIACTION_RESIZE_LEFT) {
					wnd2->SetSize(-(wnd2->width - dx), wnd2->height, connection);
					wnd2->SetPosition(wnd2->posX + dx, wnd2->posY, connection);
				}
				else if (dx != 0 && !negative && ait->first == GUIACTION_RESIZE_RIGHT && ait->second == GUIACTION_RESIZE_RIGHT) {
					wnd2->SetSize(wnd2->width + dx, wnd2->height, connection);
				}
			}
		}
	}
}


//Lua functions

int GUIManager::LuaGUIAddWindow(lua_State* L) {
	Window* wnd = (Window*)((unsigned long)LuaScript::PopNumber(L));
	GUIManager* guiManager = (GUIManager*)((unsigned long)LuaScript::PopNumber(L));

	if (guiManager && wnd)
		guiManager->AddWindow(wnd);

	return 1;
}

int GUIManager::LuaGUIRemoveWindow(lua_State* L) {
	Window* wnd = (Window*)((unsigned long)LuaScript::PopNumber(L));
	GUIManager* guiManager = (GUIManager*)((unsigned long)LuaScript::PopNumber(L));

	if (guiManager && wnd)
		guiManager->RemoveWindow(wnd);

	return 1;
}

int GUIManager::LuaGUIGetWindows(lua_State* L) {
	GUIManager* guiManager = (GUIManager*)((unsigned long)LuaScript::PopNumber(L));

	WindowsSet windows;
	if (guiManager)
		windows = guiManager->GetWindows();

	lua_newtable(L);
	WindowsSet::iterator it = windows.begin();
	int i = 1;
	for (it; it != windows.end(); it++, i++) {
		Window* wnd = *it;
		lua_pushnumber(L, i);
		lua_pushnumber(L, (unsigned long)wnd);
		lua_settable(L, -3);
	}
	return 1;
}

int GUIManager::LuaGUIClearWindows(lua_State* L) {
	GUIManager* guiManager = (GUIManager*)((unsigned long)LuaScript::PopNumber(L));

	if (guiManager)
		guiManager->ClearWindows();

	return 1;
}

int GUIManager::LuaGUIContainWindow(lua_State* L) {
	Window* wnd = (Window*)((unsigned long)LuaScript::PopNumber(L));
	GUIManager* guiManager = (GUIManager*)((unsigned long)LuaScript::PopNumber(L));

	bool state = false;
	if (guiManager && wnd)
		state = guiManager->ContainWindow(wnd);

	LuaScript::PushNumber(L, (int)state);
	return 1;
}

int GUIManager::LuaGUIAddConnection(lua_State* L) {
	Window* wnd2 = (Window*)((unsigned long)LuaScript::PopNumber(L));
	Window* wnd1 = (Window*)((unsigned long)LuaScript::PopNumber(L));
	GUIManager* guiManager = (GUIManager*)((unsigned long)LuaScript::PopNumber(L));

	GUIConnection* connection = NULL;
	if (guiManager && wnd1 && wnd2)
		connection = guiManager->AddConnection(wnd1, wnd2);

	LuaScript::PushNumber(L, (unsigned long)connection);
	return 1;
}

int GUIManager::LuaGUIRemoveConnection(lua_State* L) {
	GUIConnection* guiConnection = (GUIConnection*)((unsigned long)LuaScript::PopNumber(L));
	GUIManager* guiManager = (GUIManager*)((unsigned long)LuaScript::PopNumber(L));

	if (guiManager && guiConnection)
		guiManager->RemoveConnection(guiConnection);

	return 1;
}

int GUIManager::LuaGUIRemoveConnections(lua_State* L) {
	Window* wnd = (Window*)((unsigned long)LuaScript::PopNumber(L));
	GUIManager* guiManager = (GUIManager*)((unsigned long)LuaScript::PopNumber(L));

	if (guiManager && wnd)
		guiManager->RemoveConnections(wnd);

	return 1;
}

int GUIManager::LuaGUIGetConnections(lua_State* L) {
	Window* wnd = (Window*)((unsigned long)LuaScript::PopNumber(L));
	GUIManager* guiManager = (GUIManager*)((unsigned long)LuaScript::PopNumber(L));

	GUIConnectionsSet connections;
	if (guiManager && wnd)
		connections = guiManager->GetConnections(wnd);

	lua_newtable(L);
	GUIConnectionsSet::iterator it = connections.begin();
	int i = 1;
	for (it; it != connections.end(); it++, i++) {
		GUIConnection* connection = *it;
		lua_pushnumber(L, i);
		lua_pushnumber(L, (unsigned long)connection);
		lua_settable(L, -3);
	}
	return 1;
}

int GUIManager::LuaGUIAddActionOnResizeMainWindow(lua_State* L) {
	int params = lua_gettop(L);
	int p = params;

	GUIManager* guiManager = (GUIManager*)((unsigned long)lua_tonumber(L, -p--));
	std::string function = lua_tostring(L, -p--);
	LuaArguments args;
	for (p; p > 1; p--)
		args.push_back(lua_tostring(L, -p));
	bool sameScript = (bool)lua_tonumber(L, -p--);

	LuaScript* script = LuaScript::GetLuaScriptState(L);

	if (guiManager) {
		if (function == "clear")
			guiManager->ClearActionsOnResizeMainWindow();
		else {
			LuaScript* luaScript = LuaScript::GetLuaScriptState(L);
			guiManager->AddActionOnResizeMainWindow(boost::bind(&LuaScript::RunScript, luaScript->GetFileName(), function, args, false, (sameScript ? script : NULL)));
		}
	}

	return 1;
}

int GUIManager::LuaGUIAddActionOnSetPosition(lua_State* L) {
	int params = lua_gettop(L);
	int p = params;

	GUIManager* guiManager = (GUIManager*)((unsigned long)lua_tonumber(L, -p--));
	Window* wnd = (Window*)((unsigned long)lua_tonumber(L, -p--));
	std::string function = lua_tostring(L, -p--);
	LuaArguments args;
	for (p; p > 1; p--)
		args.push_back(lua_tostring(L, -p));
	bool sameScript = (bool)lua_tonumber(L, -p--);

	if (guiManager && wnd) {
		if (function == "clear")
			guiManager->ClearActionsOnSetPosition(wnd);
		else {
			LuaScript* luaScript = LuaScript::GetLuaScriptState(L);
			guiManager->AddActionOnSetPosition(wnd, boost::bind(&LuaScript::RunScript, luaScript->GetFileName(), function, args, false, (sameScript ? luaScript : NULL)));
		}
	}

	return 1;
}

int GUIManager::LuaGUIAddActionOnSetSize(lua_State* L) {
	int params = lua_gettop(L);
	int p = params;

	GUIManager* guiManager = (GUIManager*)((unsigned long)lua_tonumber(L, -p--));
	Window* wnd = (Window*)((unsigned long)lua_tonumber(L, -p--));
	std::string function = lua_tostring(L, -p--);
	LuaArguments args;
	for (p; p > 1; p--)
		args.push_back(lua_tostring(L, -p));
	bool sameScript = (bool)lua_tonumber(L, -p--);

	if (guiManager && wnd) {
		if (function == "clear")
			guiManager->ClearActionsOnSetSize(wnd);
		else {
			LuaScript* luaScript = LuaScript::GetLuaScriptState(L);
			guiManager->AddActionOnSetSize(wnd, boost::bind(&LuaScript::RunScript, luaScript->GetFileName(), function, args, false, (sameScript ? luaScript : NULL)));
		}
	}


	return 1;
}


int GUIManager::LuaGUIConnGetFirstWindow(lua_State* L) {
	GUIConnection* guiConnection = (GUIConnection*)((unsigned long)LuaScript::PopNumber(L));

	Window* wnd = NULL;
	if (guiConnection)
		wnd = guiConnection->GetFirstWindow();

	LuaScript::PushNumber(L, (unsigned long)wnd);
	return 1;
}

int GUIManager::LuaGUIConnGetSecondWindow(lua_State* L) {
	GUIConnection* guiConnection = (GUIConnection*)((unsigned long)LuaScript::PopNumber(L));

	Window* wnd = NULL;
	if (guiConnection)
		wnd = guiConnection->GetSecondWindow();

	LuaScript::PushNumber(L, (unsigned long)wnd);
	return 1;
}
int GUIManager::LuaGUIConnGetOtherWindow(lua_State* L) {
	Window* wnd = (Window*)((unsigned long)LuaScript::PopNumber(L));
	GUIConnection* guiConnection = (GUIConnection*)((unsigned long)LuaScript::PopNumber(L));

	Window* wndOther = NULL;
	if (guiConnection)
		wndOther = guiConnection->GetOtherWindow(wnd);

	LuaScript::PushNumber(L, (unsigned long)wndOther);
	return 1;
}

int GUIManager::LuaGUIConnContainWindow(lua_State* L) {
	Window* wnd = (Window*)((unsigned long)LuaScript::PopNumber(L));
	GUIConnection* guiConnection = (GUIConnection*)((unsigned long)LuaScript::PopNumber(L));

	bool state = false;
	if (guiConnection && wnd)
		state = guiConnection->ContainWindow(wnd);

	LuaScript::PushNumber(L, (int)state);
	return 1;
}

int GUIManager::LuaGUIConnSetAction(lua_State* L) {
	GUI_ACTIONS actionOther = (GUI_ACTIONS)LuaScript::PopNumber(L);
	GUI_ACTIONS actionMain = (GUI_ACTIONS)LuaScript::PopNumber(L);
	Window* wnd = (Window*)((unsigned long)LuaScript::PopNumber(L));
	GUIConnection* guiConnection = (GUIConnection*)((unsigned long)LuaScript::PopNumber(L));

	if (guiConnection && wnd)
		guiConnection->SetAction(wnd, actionMain, actionOther);

	return 1;
}


void GUIManager::LuaRegisterFunctions(lua_State* L) {
	lua_pushinteger(L, GUIACTION_NONE);
	lua_setglobal(L, "GUIACTION_NONE");
	lua_pushinteger(L, GUIACTION_MOVE_TOP);
	lua_setglobal(L, "GUIACTION_MOVE_TOP");
	lua_pushinteger(L, GUIACTION_MOVE_BOTTOM);
	lua_setglobal(L, "GUIACTION_MOVE_BOTTOM");
	lua_pushinteger(L, GUIACTION_MOVE_LEFT);
	lua_setglobal(L, "GUIACTION_MOVE_LEFT");
	lua_pushinteger(L, GUIACTION_MOVE_RIGHT);
	lua_setglobal(L, "GUIACTION_MOVE_RIGHT");
	lua_pushinteger(L, GUIACTION_RESIZE_TOP);
	lua_setglobal(L, "GUIACTION_RESIZE_TOP");
	lua_pushinteger(L, GUIACTION_RESIZE_BOTTOM);
	lua_setglobal(L, "GUIACTION_RESIZE_BOTTOM");
	lua_pushinteger(L, GUIACTION_RESIZE_LEFT);
	lua_setglobal(L, "GUIACTION_RESIZE_LEFT");
	lua_pushinteger(L, GUIACTION_RESIZE_RIGHT);
	lua_setglobal(L, "GUIACTION_RESIZE_RIGHT");
	lua_pushinteger(L, GUIACTION_MINIMIZE);
	lua_setglobal(L, "GUIACTION_MINIMIZE");

	//GUIAddWindow(guiManagerPtr, windowPtr)
	lua_register(L, "GUIAddWindow", GUIManager::LuaGUIAddWindow);

	//GUIRemoveWindow(guiManagerPtr, windowPtr)
	lua_register(L, "GUIRemoveWindow", GUIManager::LuaGUIRemoveWindow);

	//GUIGetWindows(guiManagerPtr) : windows{windowPtr, ...}
	lua_register(L, "GUIGetWindows", GUIManager::LuaGUIGetWindows);

	//GUIClearWindows(guiManagerPtr)
	lua_register(L, "GUIClearWindows", GUIManager::LuaGUIClearWindows);

	//GUIContainWindow(guiManagerPtr, windowPtr)
	lua_register(L, "GUIContainWindow", GUIManager::LuaGUIContainWindow);

	//GUIAddConnection(guiManagerPtr, windowPtr1, windowPtr2) : guiConnectionPtr
	lua_register(L, "GUIAddConnection", GUIManager::LuaGUIAddConnection);

	//GUIRemoveConnection(guiManagerPtr, guiConnectionPtr)
	lua_register(L, "GUIRemoveConnection", GUIManager::LuaGUIRemoveConnection);

	//GUIRemoveConnections(guiManagerPtr, windowPtr)
	lua_register(L, "GUIRemoveConnections", GUIManager::LuaGUIRemoveConnections);

	//GUIGetConnections(guiManagerPtr, windowPtr) : connections{guiConnectionPtr, ...}
	lua_register(L, "GUIGetConnections", GUIManager::LuaGUIGetConnections);

	//GUIAddActionOnResizeMainWindow(guiManagerPtr, functionName, arg1, arg2, ...)
	lua_register(L, "GUIAddActionOnResizeMainWindow", GUIManager::LuaGUIAddActionOnResizeMainWindow);

	//GUIAddActionOnSetPosition(guiManagerPtr, windowPtr, functionName, arg1, arg2, ...)
	lua_register(L, "GUIAddActionOnSetPosition", GUIManager::LuaGUIAddActionOnSetPosition);

	//GUIAddActionOnSetSize(guiManagerPtr, windowPtr, functionName, arg1, arg2, ...)
	lua_register(L, "GUIAddActionOnSetSize", GUIManager::LuaGUIAddActionOnSetSize);


	//GUIConnGetFirstWindow(guiConnectionPtr) : windowPtr
	lua_register(L, "GUIConnGetFirstWindow", GUIManager::LuaGUIConnGetFirstWindow);

	//GUIConnGetSecondWindow(guiConnectionPtr) : windowPtr
	lua_register(L, "GUIConnGetSecondWindow", GUIManager::LuaGUIConnGetSecondWindow);

	//GUIConnGetOtherWindow(guiConnectionPtr, windowPtr) : windowPtr
	lua_register(L, "GUIConnGetOtherWindow", GUIManager::LuaGUIConnGetOtherWindow);

	//GUIConnContainWindow(guiConnectionPtr, windowPtr) : state
	lua_register(L, "GUIConnContainWindow", GUIManager::LuaGUIConnContainWindow);

	//GUIConnSetAction(guiConnectionPtr, actionMain, actionOther)
	lua_register(L, "GUIConnSetAction", GUIManager::LuaGUIConnSetAction);
}
