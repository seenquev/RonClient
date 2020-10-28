/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#include "input.h"

#include "allocator.h"
#include "luascript.h"
#include "tools.h"


// ---- Mouse ---- //

Mouse::Mouse() {
	holder.type = 0x00;
	holder.holder = NULL;

	buttons = MOUSE_NONE;

	POINT point;
	GetCursorPos(&point);

	curX = point.x;
	curY = point.y;
	oldCurX = curX;
	oldCurY = curY;
	lockX = 0;
	lockY = 0;
	cursor = 0;

	comment = "";

	SetMouseCursor(IDC_ARROW);
}

Mouse::~Mouse() {
}

void Mouse::SetHolder(int posX, int posY, unsigned char _type, void* _window, void* _holder, void* _variable) {
	holder.type = _type;
	holder.holder = _holder;
	holder.variable = NULL;
	holder.posX = posX;
	holder.posY = posY;
	holder.window = _window;
	holder.variable = _variable;
}

HOLDER Mouse::GetHolder() {
	return holder;
}

void Mouse::SetLock(int posX, int posY) {
	lockX = posX;
	lockY = posY;
}

unsigned short Mouse::GetButtons() {
	return buttons;
}

void Mouse::UpdateMouse(MSG& msg) {
	oldCurX = curX;
	oldCurY = curY;
	buttons = (buttons & 0x03F) | ((buttons << 6) & 0xFC0);
	wheel = 0;

	if (msg.message == WM_MOUSEMOVE) {
		curX = LOWORD(msg.lParam);
		curY = HIWORD(msg.lParam);
	}
	else if (msg.message == WM_LBUTTONDOWN)
		buttons = buttons | MOUSE_LEFT;
	else if (msg.message == WM_MBUTTONDOWN)
		buttons = buttons | MOUSE_MIDDLE;
	else if (msg.message == WM_RBUTTONDOWN)
		buttons = buttons | MOUSE_RIGHT;
	else if (msg.message == WM_LBUTTONDBLCLK) {
		buttons = buttons | MOUSE_LEFT_DBL;
		buttons = buttons | MOUSE_LEFT;
	}
	else if (msg.message == WM_MBUTTONDBLCLK) {
		buttons = buttons | MOUSE_MIDDLE_DBL;
		buttons = buttons | MOUSE_MIDDLE;
	}
	else if (msg.message == WM_RBUTTONDBLCLK) {
		buttons = buttons | MOUSE_RIGHT_DBL;
		buttons = buttons | MOUSE_RIGHT;
	}
	else if (msg.message == WM_LBUTTONUP)
		buttons = buttons & ~(MOUSE_LEFT | MOUSE_LEFT_DBL);
	else if (msg.message == WM_MBUTTONUP)
		buttons = buttons & ~(MOUSE_MIDDLE | MOUSE_MIDDLE_DBL);
	else if (msg.message == WM_RBUTTONUP)
		buttons = buttons & ~(MOUSE_RIGHT | MOUSE_RIGHT_DBL);
	else if (msg.message == 0x020A)
		wheel = (int)msg.wParam;
}

void Mouse::SetMouseCursor(LPCTSTR cursor) {
	SetCursor(LoadCursor(NULL, cursor));
}


//Lua functions

int Mouse::LuaSetMouseHolder(lua_State* L) {
	int params = lua_gettop(L);
	void* windowPtr = (void*)((unsigned long)(params >= 7 ? LuaScript::PopNumber(L) : 0));
	void* variablePtr = (void*)((unsigned long)(params >= 6 ? LuaScript::PopNumber(L) : 0));
	void* holderPtr = (void*)((unsigned long)(params >= 5 ? LuaScript::PopNumber(L) : 0));
	unsigned char type = LuaScript::PopNumber(L);
	int posY = LuaScript::PopNumber(L);
	int posX = LuaScript::PopNumber(L);
	Mouse* mouse = (Mouse*)((unsigned long)LuaScript::PopNumber(L));

	if (mouse)
		mouse->SetHolder(posX, posY, type, holderPtr, variablePtr, windowPtr);

	return 1;
}

int Mouse::LuaGetMouseHolder(lua_State* L) {
	Mouse* mouse = (Mouse*)((unsigned long)LuaScript::PopNumber(L));

	if (mouse) {
		HOLDER holder = mouse->GetHolder();

		lua_newtable(L);
		lua_pushstring(L, "posX");
		lua_pushnumber(L, holder.posX);
		lua_settable(L, -3);
		lua_pushstring(L, "posY");
		lua_pushnumber(L, holder.posY);
		lua_settable(L, -3);
		lua_pushstring(L, "type");
		lua_pushnumber(L, holder.type);
		lua_settable(L, -3);
		lua_pushstring(L, "holderPtr");
		lua_pushnumber(L, (unsigned long)holder.holder);
		lua_settable(L, -3);
		lua_pushstring(L, "variablePtr");
		lua_pushnumber(L, (unsigned long)holder.variable);
		lua_settable(L, -3);
		lua_pushstring(L, "windowPtr");
		lua_pushnumber(L, (unsigned long)holder.window);
		lua_settable(L, -3);
		return 1;
	}

	lua_pushnil(L);
	return 1;
}

int Mouse::LuaGetMouseButtons(lua_State* L) {
	Mouse* mouse = (Mouse*)((unsigned long)LuaScript::PopNumber(L));

	unsigned int buttons;
	if (mouse)
		buttons = mouse->GetButtons();

	LuaScript::PushNumber(L, buttons);
	return 1;
}

int Mouse::LuaGetMousePosition(lua_State* L) {
	Mouse* mouse = (Mouse*)((unsigned long)LuaScript::PopNumber(L));

	if (mouse) {
		lua_newtable(L);
		lua_pushstring(L, "x");
		lua_pushnumber(L, mouse->curX);
		lua_settable(L, -3);
		lua_pushstring(L, "y");
		lua_pushnumber(L, mouse->curY);
		lua_settable(L, -3);
		return 1;
	}

	lua_pushnil(L);
	return 1;
}


void Mouse::LuaRegisterFunctions(lua_State* L) {
	lua_pushinteger(L, MOUSE_LEFT);
	lua_setglobal(L, "MOUSE_LEFT");
	lua_pushinteger(L, MOUSE_MIDDLE);
	lua_setglobal(L, "MOUSE_MIDDLE");
	lua_pushinteger(L, MOUSE_RIGHT);
	lua_setglobal(L, "MOUSE_RIGHT");
	lua_pushinteger(L, MOUSE_LEFT_DBL);
	lua_setglobal(L, "MOUSE_LEFT_DBL");
	lua_pushinteger(L, MOUSE_MIDDLE_DBL);
	lua_setglobal(L, "MOUSE_MIDDLE_DBL");
	lua_pushinteger(L, MOUSE_RIGHT_DBL);
	lua_setglobal(L, "MOUSE_RIGHT_DBL");
	lua_pushinteger(L, MOUSE_OLD_LEFT);
	lua_setglobal(L, "MOUSE_OLD_LEFT");
	lua_pushinteger(L, MOUSE_OLD_MIDDLE);
	lua_setglobal(L, "MOUSE_OLD_MIDDLE");
	lua_pushinteger(L, MOUSE_OLD_RIGHT);
	lua_setglobal(L, "MOUSE_OLD_RIGHT");
	lua_pushinteger(L, MOUSE_OLD_LEFT_DBL);
	lua_setglobal(L, "MOUSE_OLD_LEFT_DBL");
	lua_pushinteger(L, MOUSE_OLD_MIDDLE_DBL);
	lua_setglobal(L, "MOUSE_OLD_MIDDLE_DBL");
	lua_pushinteger(L, MOUSE_OLD_RIGHT_DBL);
	lua_setglobal(L, "MOUSE_OLD_RIGHT_DBL");

	//setMouseHolder(mousePtr, posX, posY, type, holderPtr = 0, variablePtr = 0, windowPtr = 0)
	lua_register(L, "setMouseHolder", Mouse::LuaSetMouseHolder);

	//getMouseHolder(mousePtr) : holder{posX, posY, type, holderPtr, variablePtr, windowPtr}
	lua_register(L, "getMouseHolder", Mouse::LuaGetMouseHolder);

	//getMouseButtons(mousePtr) : buttons
	lua_register(L, "getMouseButtons", Mouse::LuaGetMouseButtons);

	//getMousePosition(mousePtr) : position{x, y}
	lua_register(L, "getMousePosition", Mouse::LuaGetMousePosition);
}


// ---- Keyboard ---- //

Keyboard::Keyboard() {
	for (int x = 0; x < 256; x++) {
		key[x] = false;
		oldKey[x] = false;
	}

	capsLock = false;
	numLock = false;
	scrollLock = false;
}

Keyboard::~Keyboard() {
}

void Keyboard::UpdateKeyboard(MSG& msg) {
	for (int x = 0; x < 256; x++)
		oldKey[x] = key[x];

	if (msg.message == WM_SYSKEYDOWN) msg.message = WM_KEYDOWN;
	if (msg.message == WM_SYSKEYUP) msg.message = WM_KEYUP;

	if (msg.message == WM_KEYDOWN) {
		if (msg.wParam < 256) key[msg.wParam] = true;
		if (msg.wParam == VK_CAPITAL) capsLock = !capsLock;
		if (msg.wParam == VK_NUMLOCK) numLock = !numLock;
		if (msg.wParam == VK_SCROLL) scrollLock = !scrollLock;
	}
	else if (msg.message == WM_KEYUP) {
		if (msg.wParam < 256) key[msg.wParam] = false;
	}
}

void Keyboard::UpdateKeyChar(MSG& msg) {
	oldKeyChar = keyChar;
	keyChar = 0;

	if (msg.message == WM_CHAR) {
		keyChar = msg.wParam;
	}
	else if (msg.message == WM_KEYDOWN) {
		unsigned int scanCode = (msg.lParam >> 16) & 0xFFFF;
		if (msg.wParam == VK_UP && !(scanCode & 0x0100)) keyChar = 255 + VK_NUMPAD8;
		else if (msg.wParam == VK_UP) keyChar = 255 + msg.wParam;
		else if (msg.wParam == VK_DOWN && !(scanCode & 0x0100)) keyChar = 255 + VK_NUMPAD2;
		else if (msg.wParam == VK_DOWN) keyChar = 255 + msg.wParam;
		else if (msg.wParam == VK_LEFT && !(scanCode & 0x0100)) keyChar = 255 + VK_NUMPAD4;
		else if (msg.wParam == VK_LEFT) keyChar = 255 + msg.wParam;
		else if (msg.wParam == VK_RIGHT && !(scanCode & 0x0100)) keyChar = 255 + VK_NUMPAD6;
		else if (msg.wParam == VK_RIGHT) keyChar = 255 + msg.wParam;
		else if (msg.wParam == VK_HOME && !(scanCode & 0x0100)) keyChar = 255 + VK_NUMPAD7;
		else if (msg.wParam == VK_HOME) keyChar = 255 + msg.wParam;
		else if (msg.wParam == VK_END && !(scanCode & 0x0100)) keyChar = 255 + VK_NUMPAD1;
		else if (msg.wParam == VK_END) keyChar = 255 + msg.wParam;
		else if (msg.wParam == VK_INSERT) keyChar = 255 + msg.wParam;
		else if (msg.wParam == VK_DELETE) keyChar = 255 + msg.wParam;
		else if (msg.wParam == VK_PRIOR && !(scanCode & 0x0100)) keyChar = 255 + VK_NUMPAD9;
		else if (msg.wParam == VK_PRIOR) keyChar = 255 + msg.wParam;
		else if (msg.wParam == VK_NEXT && !(scanCode & 0x0100)) keyChar = 255 + VK_NUMPAD3;
		else if (msg.wParam == VK_NEXT) keyChar = 255 + msg.wParam;
		else if (msg.wParam == VK_SNAPSHOT) keyChar = 255 + msg.wParam;
		else if (msg.wParam >= 112 && msg.wParam <= 123) keyChar = 255 + msg.wParam;
	}
}

void Keyboard::ResetKeyboardStatus() {
	for (int i = 0; i < 256; i++) {
		oldKey[i] = false;
		key[i] = false;
	}

	oldKeyChar = 0;
	keyChar = 0;
}


//Lua functions

int Keyboard::LuaGetKeyState(lua_State* L) {
	int key = LuaScript::PopNumber(L);
	Keyboard* keyboard = (Keyboard*)((unsigned long)LuaScript::PopNumber(L));

	if (keyboard && keyboard->key[key])
		LuaScript::PushNumber(L, 1);
	else
		LuaScript::PushNumber(L, 0);

	return 1;
}

int Keyboard::LuaGetCapsLockState(lua_State* L) {
	Keyboard* keyboard = (Keyboard*)((unsigned long)LuaScript::PopNumber(L));

	if (keyboard)
		LuaScript::PushNumber(L, (int)keyboard->capsLock);

	LuaScript::PushNumber(L, 0);
	return 1;
}

int Keyboard::LuaGetNumLockState(lua_State* L) {
	Keyboard* keyboard = (Keyboard*)((unsigned long)LuaScript::PopNumber(L));

	if (keyboard)
		LuaScript::PushNumber(L, (int)keyboard->numLock);

	LuaScript::PushNumber(L, 0);
	return 1;
}

int Keyboard::LuaGetScrollLockState(lua_State* L) {
	Keyboard* keyboard = (Keyboard*)((unsigned long)LuaScript::PopNumber(L));

	if (keyboard)
		LuaScript::PushNumber(L, (int)keyboard->scrollLock);

	LuaScript::PushNumber(L, 0);
	return 1;
}

int Keyboard::LuaGetKeyChar(lua_State* L) {
	Keyboard* keyboard = (Keyboard*)((unsigned long)LuaScript::PopNumber(L));

	if (keyboard)
		LuaScript::PushNumber(L, keyboard->keyChar);

	LuaScript::PushNumber(L, 0);
	return 1;
}

int Keyboard::LuaGetOldKeyChar(lua_State* L) {
	Keyboard* keyboard = (Keyboard*)((unsigned long)LuaScript::PopNumber(L));

	if (keyboard)
		LuaScript::PushNumber(L, keyboard->oldKeyChar);

	LuaScript::PushNumber(L, 0);
	return 1;
}


void Keyboard::LuaRegisterFunctions(lua_State* L) {
	//getKeyState(keyboardPtr, key) : state
	lua_register(L, "getKeyState", Keyboard::LuaGetKeyState);

	//getCapsLockState(keyboardPtr) : state
	lua_register(L, "getCapsLockState", Keyboard::LuaGetCapsLockState);

	//getNumLockState(keyboardPtr) : state
	lua_register(L, "getNumLockState", Keyboard::LuaGetNumLockState);

	//getScrollLockState(keyboardPtr) : state
	lua_register(L, "getScrollLockState", Keyboard::LuaGetScrollLockState);

	//getKeyChar(keyboardPtr) : keyChar
	lua_register(L, "getKeyChar", Keyboard::LuaGetKeyChar);

	//getOldKeyChar(keyboardPtr) : keyChar
	lua_register(L, "getOldKeyChar", Keyboard::LuaGetOldKeyChar);
}
