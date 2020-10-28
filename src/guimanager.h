/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#ifndef __GUIMANAGER_H_
#define __GUIMANAGER_H_

#include <map>
#include <set>

#include "input.h"
#include "luascript.h"
#include "mthread.h"


class Window;

class GUIConnection;
class GUIManager;


enum GUI_ACTIONS {
	GUIACTION_NONE = 0,
	GUIACTION_MOVE_TOP = 1,
	GUIACTION_MOVE_BOTTOM = 2,
	GUIACTION_MOVE_LEFT = 3,
	GUIACTION_MOVE_RIGHT = 4,
	GUIACTION_RESIZE_TOP = 5,
	GUIACTION_RESIZE_BOTTOM = 6,
	GUIACTION_RESIZE_LEFT = 7,
	GUIACTION_RESIZE_RIGHT = 8,
	GUIACTION_MINIMIZE = 9,
};


typedef std::map<GUI_ACTIONS, GUI_ACTIONS> GUIActionMap;
typedef std::map<Window*, GUIActionMap> GUIWindowActionsMap;

class GUIConnection {
private:
	Window* wnd1;
	Window* wnd2;

	GUIWindowActionsMap	actions;

public:
	GUIConnection();
	GUIConnection(Window* w1, Window* w2);
	~GUIConnection();

	Window* GetFirstWindow();
	Window* GetSecondWindow();
	Window* GetOtherWindow(Window* wnd);
	bool ContainWindow(Window* wnd);

	void SetAction(Window* wnd, GUI_ACTIONS actionMain, GUI_ACTIONS actionOther);

	friend class GUIManager;
};


typedef std::set<Window*> WindowsSet;
typedef std::set<GUIConnection*> GUIConnectionsSet;
typedef std::map<Window*, GUIConnectionsSet> GUIConnectionsMap;
typedef std::map<Window*, Signal> GUISignals;


class GUIManager {
private:
	WindowsSet			windows;
	GUIConnectionsMap	connectionsMap;

	Signal	onResizeMainWindow;
	GUISignals	onSetPosition;
	GUISignals	onSetSize;

	LuaScript*	guiMaker;
	LuaScript*	guiKeyChecker;

public:
	MUTEX	lockGUIManager;

public:
	GUIManager();
	~GUIManager();

	void CleanUp();

	void AddWindow(Window* wnd);
	void RemoveWindow(Window* wnd);
	WindowsSet GetWindows();
	void ClearWindows();
	bool ContainWindow(Window* wnd);

	GUIConnection* AddConnection(Window* wnd1, Window* wnd2);
	void RemoveConnection(GUIConnection* connection);
	void RemoveConnections(Window* wnd);
	GUIConnectionsSet GetConnections(Window* wnd);

	void MakeGUI();
	bool CheckGUIKey(Keyboard* keyboard);

	void AddActionOnResizeMainWindow(boost::function<void()> func);
	void AddActionOnSetPosition(Window* wnd, boost::function<void()> func);
	void AddActionOnSetSize(Window* wnd, boost::function<void()> func);
	bool ExecuteOnResizeMainWindow();
	bool ExecuteOnSetPosition(Window* wnd);
	bool ExecuteOnSetSize(Window* wnd);
	void ClearActionsOnResizeMainWindow();
	void ClearActionsOnSetPosition(Window* wnd);
	void ClearActionsOnSetSize(Window* wnd);

	void OnWindowChangePosition(Window* wnd, int dx, int dy, GUIConnection* excludeConnection = NULL);
	void OnWindowChangeSize(Window* wnd, int dx, int dy, bool negative, GUIConnection* excludeConnection = NULL);

	//Lua functions
	static int LuaGUIAddWindow(lua_State* L);
	static int LuaGUIRemoveWindow(lua_State* L);
	static int LuaGUIGetWindows(lua_State* L);
	static int LuaGUIClearWindows(lua_State* L);
	static int LuaGUIContainWindow(lua_State* L);
	static int LuaGUIAddConnection(lua_State* L);
	static int LuaGUIRemoveConnection(lua_State* L);
	static int LuaGUIRemoveConnections(lua_State* L);
	static int LuaGUIGetConnections(lua_State* L);
	static int LuaGUIAddActionOnResizeMainWindow(lua_State* L);
	static int LuaGUIAddActionOnSetPosition(lua_State* L);
	static int LuaGUIAddActionOnSetSize(lua_State* L);

	static int LuaGUIConnGetFirstWindow(lua_State* L);
	static int LuaGUIConnGetSecondWindow(lua_State* L);
	static int LuaGUIConnGetOtherWindow(lua_State* L);
	static int LuaGUIConnContainWindow(lua_State* L);
	static int LuaGUIConnSetAction(lua_State* L);

	static void LuaRegisterFunctions(lua_State* L);
};

#endif //__GUIMANAGER_H_
