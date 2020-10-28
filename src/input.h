/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#ifndef __INPUT_H_
#define __INPUT_H_

#include <string>
#include <windows.h>

#include "luascript.h"


enum MOUSE_BUTTONS {
	MOUSE_NONE				= 0x000,
	MOUSE_LEFT				= 0x001,
	MOUSE_MIDDLE			= 0x002,
	MOUSE_RIGHT				= 0x004,
	MOUSE_LEFT_DBL			= 0x008,
	MOUSE_MIDDLE_DBL		= 0x010,
	MOUSE_RIGHT_DBL			= 0x020,
	MOUSE_OLD_LEFT			= 0x040,
	MOUSE_OLD_MIDDLE		= 0x080,
	MOUSE_OLD_RIGHT			= 0x100,
	MOUSE_OLD_LEFT_DBL		= 0x200,
	MOUSE_OLD_MIDDLE_DBL	= 0x400,
	MOUSE_OLD_RIGHT_DBL		= 0x800,
};


struct HOLDER {
	int				posX;
	int 			posY;
	unsigned char	type;
	void*			holder;
	void*			variable;
	void*			window;
};


class Mouse {
private:
	HOLDER		holder;

public:
	int		curX;
	int		curY;
	int		oldCurX;
	int		oldCurY;
	int		lockX;
	int		lockY;
	int		wheel;
	int		cursor;

	std::string		comment;

	unsigned short	buttons;

public:
	Mouse();
	~Mouse();

	void SetHolder(int posX, int posY, unsigned char _type, void* _window = NULL, void* _holder = NULL, void* _variable = NULL);
	HOLDER GetHolder();
	void SetLock(int posX, int posY);
	unsigned short GetButtons();
	void UpdateMouse(MSG& msg);

	void SetMouseCursor(LPCTSTR cursor);

	//Lua functions
	static int LuaSetMouseHolder(lua_State* L);
	static int LuaGetMouseHolder(lua_State* L);
	static int LuaGetMouseButtons(lua_State* L);
	static int LuaGetMousePosition(lua_State* L);

	static void LuaRegisterFunctions(lua_State* L);
};


class Keyboard {
public:
	bool	key[256];
	bool	oldKey[256];

	bool	capsLock;
	bool	numLock;
	bool	scrollLock;

	int		keyChar;
	int		oldKeyChar;

public:
	Keyboard();
	~Keyboard();

	void UpdateKeyboard(MSG& msg);
	void UpdateKeyChar(MSG& msg);

	void ResetKeyboardStatus();

	//Lua functions
	static int LuaGetKeyState(lua_State* L);
	static int LuaGetCapsLockState(lua_State* L);
	static int LuaGetNumLockState(lua_State* L);
	static int LuaGetScrollLockState(lua_State* L);
	static int LuaGetKeyChar(lua_State* L);
	static int LuaGetOldKeyChar(lua_State* L);

	static void LuaRegisterFunctions(lua_State* L);
};

#endif //__INPUT_H_
