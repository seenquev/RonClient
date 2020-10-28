/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#ifndef __WINDOW_H_
#define __WINDOW_H_

#include <list>
#include <set>
#include <string>
#include <dirent.h>

#include "allocator.h"

#include "ad2d.h"
#include "const.h"
#include "container.h"
#include "guimanager.h"
#include "iniloader.h"
#include "input.h"
#include "item.h"
#include "logger.h"
#include "luascript.h"
#include "minimap.h"
#include "mthread.h"
#include "realtime.h"


class WindowElement;
class WindowElementContainer;
class WindowElementText;
class WindowElementLink;
class WindowElementImage;
class WindowElementButton;
class WindowElementTextarea;
class WindowElementMemo;
class WindowElementTableMemo;
class WindowElementList;
class WindowElementTimer;
class WindowElementTab;
class WindowElementItemContainer;
class WindowElementCheckBox;
class WindowElementScrollBar;
class WindowElementHealthBar;
class WindowElementManaBar;
class WindowElementSkillBar;
class WindowElementPlayground;
class WindowElementMiniMap;
class WindowElementColorMap;
class WindowElementBattle;
class WindowElementVIP;
class WindowElementCooldown;
class WindowElementSelect;
class WindowTemplate;
class Windows;
class Window;
class ContainerWindow;

class VIPList;

class Channel;

class Game;
struct HotKey;
struct InputActions;


enum WINDOW_TYPE {
	WND_MESSAGE			= 0,
	WND_STARTUP			= 1,
	WND_LOGIN			= 2,
	WND_SERVERS			= 3,
	WND_CHARACTERSLIST	= 4,
	WND_QUEUE			= 5,
	WND_GAME			= 6,
	WND_CONSOLE			= 7,
	WND_INVENTORY		= 8,
	WND_CONTAINER		= 9,
	WND_COUNTER			= 10,
	WND_CHANNELSLIST	= 11,
	WND_MINIMAP			= 12,
	WND_STATISTICS		= 13,
	WND_BATTLE			= 14,
	WND_VIPLIST			= 15,
	WND_ADDVIP			= 16,
	WND_TEXT			= 17,
	WND_HOUSETEXT		= 18,
	WND_TRADE			= 19,
	WND_SHOP			= 20,
	WND_CHANGEOUTFIT	= 21,
	WND_QUESTS			= 22,
	WND_MISSIONS		= 23,
	WND_SPELLS			= 24,
	WND_SETHOTKEYTEXT	= 25,
	WND_SETHOTKEY		= 26,
	WND_OPTGAME			= 27,
	WND_OPTGRAPHIC		= 28,
	WND_OPTTEXT			= 29,
	WND_OPTWINDOWS		= 30,
	WND_OPTTEMPLATES	= 31,
	WND_OPTSOUND		= 32,
	WND_ABOUT			= 33,
	WND_ERROR			= 34,
	WND_UPDATE			= 35,
	WND_ADDFAVORITE		= 36,
	WND_CRAFTBOX		= 37,
	WND_EDITMARKER		= 38,
	WND_EDITWAYPOINT	= 39,
	WND_SAVEWAYPOINTS	= 40,
	WND_LOADWAYPOINTS	= 41,
	WND_BOT				= 42,
	WND_OLDHOTKEYS		= 43,
	WND_REPORTERROR		= 44,
	WND_STATUS			= 0x7C,
	WND_HOTKEYS			= 0x7D,
	WND_MENU			= 0x7E,
	WND_OPTIONS			= 0x7F,
	WND_CLOSE			= 0xFD,
	WND_CUSTOM			= 0xFE,
	WND_ALL				= 0xFF,
};

enum WINDOW_ELEMENT_ALIGN {
	ALIGN_H_LEFT	= 1,
	ALIGN_H_RIGHT	= 2,
	ALIGN_H_CENTER	= 3,
	ALIGN_V_TOP		= 4,
	ALIGN_V_BOTTOM	= 8,
	ALIGN_V_CENTER	= 12,
};

enum WINDOW_ELEMENTS {
	ELEMENT_HEADER			= 1,
	ELEMENT_WNDICONS		= 2,
	ELEMENT_BORDER			= 4,
	ELEMENT_CONTAINER		= 8,
	ELEMENT_SCROLL			= 16,
	ELEMENT_BUTTON			= 32,
	ELEMENT_TAB				= 64,
	ELEMENT_TEXTAREA		= 128,
	ELEMENT_MEMO			= 256,
	ELEMENT_LIST			= 512,
	ELEMENT_TIMER			= 1024,
	ELEMENT_ITEMCONTAINER	= 2048,
	ELEMENT_CHECKBOX		= 4096,
	ELEMENT_SCROLLBAR		= 8192,
	ELEMENT_HEALTHBAR		= 16384,
	ELEMENT_MANABAR			= 32768,
	ELEMENT_SKILLBAR		= 65536,
};

enum WINDOW_ACTIONS {
	ACT_CLOSE,
	ACT_MINIMIZE,
	ACT_EXECUTE,
};

enum WINDOWELEMENT_FLAGS {
	FLAG_NONE = 0x00000000,
	FLAG_TEXTTIME = 0x00000001,
	FLAG_ALL = 0xFFFFFFFF,
};

enum BUTTON_TYPS {
	BUTTON_NONE,
	BUTTON_NORMAL,
	BUTTON_SWITCH,
	BUTTON_CONTINUE,
	BUTTON_TAB,
	BUTTON_TAB_SIDE,
};


struct ElementINI {
	int posX;
	int posY;
	int width;
	int height;
	int fontSize;
	int textBorder;
	int colorB;
	int colorExB;
	COLOR color;
	COLOR colorEx;
	unsigned char colorC;
	unsigned char colorExC;
};


struct MenuData {
	unsigned char action;
	TextString text;
	void* data;

	MenuData() { data = NULL; }
	MenuData(unsigned char act, TextString tx, void* dt) : action(act), text(tx), data(dt) { }
	MenuData(unsigned char act, std::string tx, void* dt) : action(act), text(tx), data(dt) { }
	void ReleaseData() {
		if (!data)
			return;

		switch(action) {
			case ACTION_LOOK:
			case ACTION_USE:
			case ACTION_USEWITH:
			case ACTION_OPEN:
			case ACTION_OPENNEW:
			case ACTION_ROTATE:
			case ACTION_TRADEWITH:
			case ACTION_MOVETO:
			case ACTION_CLEARFREE: {
				ThingData* thing = (ThingData*)data;
				if (thing)
					delete_debug(thing, M_PLACE);
				break;
			}
			case ACTION_COPYNAME:
			case ACTION_SENDMESSAGE:
			case ACTION_IGNORE:
			case ACTION_UNIGNORE:
			case ACTION_ADDTOVIP:
			case ACTION_COPY:
			case ACTION_COPYMESSAGE: {
				std::string* text = (std::string*)data;
				if (text)
					delete_debug(text, M_PLACE);
				break;
			}
			case ACTION_ADDMARKER:
			case ACTION_EDITMARKER:
			case ACTION_REMOVEMARKER:
			case ACTION_ADDWAYPOINT:
			case ACTION_EDITWAYPOINT:
			case ACTION_REMOVEWAYPOINT: {
				Position* pos = (Position*)data;
				if (pos)
					delete_debug(pos, M_PLACE);
				break;
			}
			case ACTION_EXECUTE: {
				Signal* signal = (Signal*)data;
				if (signal)
					delete_debug(signal, M_PLACE);
				break;
			}
			default:
				break;
		}
	}

	~MenuData() { }
};

struct TEMPLATE_Icons {
	AD2D_Image slot[10];
};

struct TEMPLATE_Header {
	AD2D_Image l;
	AD2D_Image c;
	AD2D_Image r;

	int l_x, l_y, l_width;
	int c_y, c_height;
	int r_x, r_y, r_width;
	int t_offsetx, t_offsety;
};

struct TEMPLATE_WndIcons {
	AD2D_Image close;
	AD2D_Image minimize;
	AD2D_Image moveup;

	int x, y;
	int w, h;
};

struct TEMPLATE_Border {
	AD2D_Image t;
	AD2D_Image b;
	AD2D_Image l;
	AD2D_Image r;
	AD2D_Image tl;
	AD2D_Image tr;
	AD2D_Image bl;
	AD2D_Image br;
	AD2D_Image bg;

	bool repeatBG;
	int t_y, t_height;
	int b_y, b_height;
	int l_x, l_width;
	int r_x, r_width;
	int tl_x, tl_y, tl_width, tl_height;
	int tr_x, tr_y, tr_width, tr_height;
	int bl_x, bl_y, bl_width, bl_height;
	int br_x, br_y, br_width, br_height;
};

struct TEMPLATE_Container {
	AD2D_Image bg;
	AD2D_Image t;
	AD2D_Image b;
	AD2D_Image l;
	AD2D_Image r;
	AD2D_Image tl;
	AD2D_Image tr;
	AD2D_Image bl;
	AD2D_Image br;

	int l_width, r_width, t_height, b_height;
};

struct TEMPLATE_Scroll {
	AD2D_Image t;
	AD2D_Image b;
	AD2D_Image l;
	AD2D_Image r;
	AD2D_Image bgv;
	AD2D_Image bgh;
	AD2D_Image brv;
	AD2D_Image brh;

	int w, h;
};

struct TEMPLATE_Button {
	AD2D_Image tl;
	AD2D_Image tr;
	AD2D_Image bl;
	AD2D_Image br;
	AD2D_Image t;
	AD2D_Image b;
	AD2D_Image l;
	AD2D_Image r;
	AD2D_Image bg;

	int l_width, r_width, t_height, b_height;
};

struct TEMPLATE_TabButton {
	AD2D_Image tl;
	AD2D_Image tr;
	AD2D_Image bl;
	AD2D_Image br;
	AD2D_Image t;
	AD2D_Image b;
	AD2D_Image l;
	AD2D_Image r;
	AD2D_Image bg;
	AD2D_Image x;

	int l_width, r_width, t_height, b_height;
};

struct TEMPLATE_Textarea {
	AD2D_Image tl;
	AD2D_Image tr;
	AD2D_Image bl;
	AD2D_Image br;
	AD2D_Image t;
	AD2D_Image b;
	AD2D_Image l;
	AD2D_Image r;
	AD2D_Image bg;

	int l_width, r_width, t_height, b_height;
};

struct TEMPLATE_Memo {
	AD2D_Image tl;
	AD2D_Image tr;
	AD2D_Image bl;
	AD2D_Image br;
	AD2D_Image t;
	AD2D_Image b;
	AD2D_Image l;
	AD2D_Image r;
	AD2D_Image m;
	AD2D_Image bg;

	int l_width, r_width, t_height, b_height;
};

struct TEMPLATE_List {
	AD2D_Image tl;
	AD2D_Image tr;
	AD2D_Image bl;
	AD2D_Image br;
	AD2D_Image t;
	AD2D_Image b;
	AD2D_Image l;
	AD2D_Image r;
	AD2D_Image bg;
	AD2D_Image se;

	int l_width, r_width, t_height, b_height;
};

struct TEMPLATE_Timer {
	AD2D_Image tl;
	AD2D_Image tr;
	AD2D_Image bl;
	AD2D_Image br;
	AD2D_Image t;
	AD2D_Image b;
	AD2D_Image l;
	AD2D_Image r;
	AD2D_Image bg;

	int l_width, r_width, t_height, b_height;
};

struct TEMPLATE_Item {
	AD2D_Image tl;
	AD2D_Image tr;
	AD2D_Image bl;
	AD2D_Image br;
	AD2D_Image t;
	AD2D_Image b;
	AD2D_Image l;
	AD2D_Image r;
	AD2D_Image bg;

	int l_width, r_width, t_height, b_height;
};

struct TEMPLATE_CheckBox {
	AD2D_Image uch;
	AD2D_Image ch;

	int text_offset, width, height;
};

struct TEMPLATE_ScrollBar {
	AD2D_Image l;
	AD2D_Image r;
	AD2D_Image m;
	AD2D_Image br;

	int l_width, r_width, height;
};

struct TEMPLATE_Bar {
	AD2D_Image empty;
	AD2D_Image bar;

	int width, height;
};


class WindowElement {
private:
	unsigned char	align;

	int				posX;
	int				posY;
	unsigned short	width;
	unsigned short	height;

	int				border;
	int				fontSize;

	Window*						window;
	WindowTemplate*				wndTemplate;
	WindowElementContainer*		parent;

	Signal			func;
	Signal			funcOnActivate;
	Signal			funcOnDeactivate;

	std::string		comment;

	unsigned int	flags;

	bool			active;
	bool			alwaysActive;
	bool			lockWidth;
	bool			lockHeight;
	bool			visible;
	bool			enabled;

	unsigned int	shine;

public:
	WindowElement();
	virtual ~WindowElement();

	static ElementINI GetElementINI(INILoader& iniLoader, std::string iniTag);
	void ApplyElementINI(INILoader& iniLoader, std::string iniTag);

	virtual void SetWindow(Window* window);
	virtual void SetParent(WindowElementContainer* parent);
	Window* GetWindow();
	WindowElementContainer* GetParent();
	void SetWindowTemplate(WindowTemplate* wndTemplate);
	WindowTemplate* GetWindowTemplate();
	virtual void SetSize(unsigned short wdh, unsigned short hgt);
	POINT GetPosition();
	POINT GetSize();
	void SetFlags(unsigned int flags);
	bool CheckFlags(unsigned int flags);
	void ClearFlags(unsigned int flags);
	void SetPosition(int pX, int pY);
	void SetAlwaysActive(bool alwaysActive);
	void SetVisible(bool state);
	void SetEnabled(bool state);
	void SetShine(unsigned int shine);
	unsigned int GetShine();
	virtual void SetBorder(int border);
	virtual void SetFontSize(int fontSize);
	bool IsActive();
	void SetComment(std::string cmt);
	virtual std::string GetComment();
	POINT GetParentScrollOffset();
	POINT AdjustViewport(AD2D_Viewport& source, AD2D_Viewport& destination);
	POINT GetAbsolutePosition(bool withWindow = false);
	void SetLocks(bool wdh, bool hgt);
	void SetAction(boost::function<void()> f);
	void SetOnActivate(boost::function<void()> f);
	void SetOnDeactivate(boost::function<void()> f);
	virtual void OpenMenu(Mouse& mouse, Keyboard& keyboard, std::list<MenuData>& actionList);

	virtual void Print(POINT& d, AD2D_Window& gfx) = 0;
	void PrintShine(POINT& d, AD2D_Window& gfx);

	friend class WindowElementContainer;
	friend class WindowElementText;
	friend class WindowElementLink;
	friend class WindowElementImage;
	friend class WindowElementButton;
	friend class WindowElementTextarea;
	friend class WindowElementMemo;
	friend class WindowElementTableMemo;
	friend class WindowElementList;
	friend class WindowElementTimer;
	friend class WindowElementTab;
	friend class WindowElementItemContainer;
	friend class WindowElementCheckBox;
	friend class WindowElementScrollBar;
	friend class WindowElementHealthBar;
	friend class WindowElementManaBar;
	friend class WindowElementSkillBar;
	friend class WindowElementPlayground;
	friend class WindowElementCooldown;
	friend class WindowElementSelect;
	friend class WindowElementMiniMap;
	friend class WindowElementColorMap;
	friend class WindowElementBattle;
	friend class WindowElementVIP;
	friend class Windows;
	friend class Window;
};

class WindowElementContainer : public WindowElement {
private:
	std::list<WindowElement*>	elements;
	float						scrollH;
	float						scrollV;
	int							intWidth;
	int							intHeight;

	bool						cntbackground;
	bool						cntborder;

	bool						scroll;
	bool						hAlwaysVisible;
	bool						vAlwaysVisible;

public:
	WindowElementContainer();
	virtual ~WindowElementContainer();

	void Create(unsigned char al, int pX, int pY, unsigned short wdh, unsigned short hgt, bool bg, bool br, WindowTemplate* wTemplate);
	void SetScroll(bool scroll);
	void SetScrollAlwaysVisible(bool horizontal, bool vertical);

	virtual void SetWindow(Window* window);

	virtual void SetSize(unsigned short wdh, unsigned short hgt);
	void SetIntSize(unsigned short wdh, unsigned short hgt);
	void AdjustSize();
	POINT GetScrollOffset();
	POINT GetIntSize();

	void SetContainerBackground(bool state);
	void SetContainerBorder(bool state);

	void AddElement(WindowElement* wndElement);
	void RemoveElement(WindowElement* wndElement);
	void RemoveAllElements();
	void DeleteAllElements();
	unsigned int GetElementsSize();

	template<class T> T* Find() {
		std::list<WindowElement*>::iterator it = elements.begin();
		for (it; it != elements.end(); it++) {
			T* element = dynamic_cast<T*>(*it);
			if (element)
				return element;
		}
	}

	virtual void Print(POINT& d, AD2D_Window& gfx);

	friend class WindowElement;
	friend class WindowElementTab;
	friend class Windows;
	friend class Window;
};

class WindowElementText : public WindowElement {
private:
	TextString		text;
	int				precision;

	TypePointer		value;

	int				txtAlign;

	COLOR						color;
	boost::function<COLOR()>	colorFunc;

public:
	WindowElementText();
	virtual ~WindowElementText();

	void Create(unsigned char al, int pX, int pY, int wdh, WindowTemplate* wTemplate);
	void SetText(TextString txt);
	void SetText(std::string txt);
	virtual void SetBorder(int border);
	virtual void SetFontSize(int fontSize);
	void SetAlign(int align);
	void SetPrecision(int precision);
	TextString GetText();
	void SetColor(float r, float g, float b, float a = 1.0f);
	void SetColor(boost::function<COLOR()> func);

	void SetValuePtr(TypePointer pointer);
	TypePointer GetValuePtr();

	virtual void Print(POINT& d, AD2D_Window& gfx);
};

class WindowElementLink : public WindowElement {
private:
	TextString		text;
	COLOR			colorOut;
	COLOR			colorIn;
	MenuData		mdata;

	Mouse*			mouse;

	int				txtAlign;

public:
	WindowElementLink();
	virtual ~WindowElementLink();

	void Create(unsigned char al, int pX, int pY, int wdh, Mouse* ms, MenuData* md, WindowTemplate* wTemplate);
	void SetText(TextString txt);
	void SetText(std::string txt);
	virtual void SetBorder(int border);
	virtual void SetFontSize(int fontSize);
	void SetAlign(int align);
	TextString GetText();
	void SetColorOut(float r, float g, float b, float a = 1.0f);
	void SetColorIn(float r, float g, float b, float a = 1.0f);

	virtual void Print(POINT& d, AD2D_Window& gfx);
};

class WindowElementImage : public WindowElement {
private:
	AD2D_Image*		image;

public:
	WindowElementImage();
	virtual ~WindowElementImage();

	void Create(unsigned char al, int pX, int pY, int wdh, int hgt, AD2D_Image* img, WindowTemplate* wTemplate);
	void Create(unsigned char al, int pX, int pY, AD2D_Image* img, WindowTemplate* wTemplate);
	void SetImage(AD2D_Image* img);

	virtual void Print(POINT& d, AD2D_Window& gfx);

	friend class WindowElement;
	friend class Windows;
	friend class Window;
};

class WindowElementButton : public WindowElement {
private:
	unsigned char	type;
	bool			pressed;
	bool			inverse;
	bool			closeIcon;

	AD2D_Image*		image;
	std::string		text;

	COLOR						color;
	boost::function<COLOR()>	colorFunc;

public:
	WindowElementButton();
	virtual ~WindowElementButton();

	void Create(unsigned char al, unsigned char tp, int pX, int pY, unsigned short wdh, unsigned short hgt, WindowTemplate* wTemplate);
	void SetImage(AD2D_Image* img);
	void SetText(std::string txt);
	void SetColor(float r, float g, float b, float a = 1.0f);
	void SetColor(boost::function<COLOR()> func);
	void SetPressed(bool pressed);
	bool GetPressed();
	void SetInverse(bool inverse);
	bool GetInverse();
	void SetCloseIcon(bool state);
	bool GetCloseIcon();

	virtual void OpenMenu(Mouse& mouse, Keyboard& keyboard, std::list<MenuData>& actionList);

	virtual void Print(POINT& d, AD2D_Window& gfx);

	friend class WindowElement;
	friend class Windows;
	friend class Window;
};

class WindowElementTextarea : public WindowElement {
private:
	TextString		text;
	TextString		_text;

	std::vector<std::pair<unsigned int, TextString> > lines;

	std::vector<std::string>	textList;
	unsigned char				textListIt;

	unsigned int	maxLength;

	unsigned int	cur;
	POINT			curBegin;
	POINT			curEnd;

	float			scrollH;
	float			scrollV;

	bool			hidden;
	bool			editable;
	bool			multiline;

	unsigned char	color;

	int				offset;

private:
	void	UpdateText();

public:
	WindowElementTextarea();
	virtual ~WindowElementTextarea();

	void Create(unsigned char al, int pX, int pY, unsigned short wdh, unsigned short hgt, bool ed, bool ml, WindowTemplate* wTemplate);
	void SetIntSize(int wdh, int hgt);
	void SetOffset(int offset);
	virtual void SetFontSize(int fontSize);
	void SetText(std::string txt);
	void AddTextLine(TextString txt);
	unsigned int GetAllLinesNumber();
	void SetMaxLength(unsigned int length);
	bool IsEditable();
	bool IsSelection();
	std::string GetText();
	std::string* GetTextPtr();
	void SetHidden(bool state);
	void SetActive(bool state);
	void SetColor(unsigned char color);

	virtual void SetSize(unsigned short wdh, unsigned short hgt);

	void GetCursor(POINT& cursor, float& pix, std::string* txt = NULL);
	void SetCursor(POINT cursor, float pix = 0.0f, std::string* txt = NULL);
	unsigned int GetLinesNumber();
	unsigned int GetLineNumber();
	int GetScrollOffset();
	std::string SelectWordAtCursor(float pix = 0.0f);

	void CheckInput(Keyboard& keyboard);

	virtual void OpenMenu(Mouse& mouse, Keyboard& keyboard, std::list<MenuData>& actionList);

	virtual void Print(POINT& d, AD2D_Window& gfx);

	friend class WindowElement;
	friend class Windows;
	friend class Window;
};

class WindowElementMemo : public WindowElement {
private:
	std::list<std::pair<TextString, bool> >	elements;
	unsigned short			option;

	float			scrollV;

	COLOR			color;

	Signal			dblFunc;

public:
	WindowElementMemo();
	virtual ~WindowElementMemo();

	void Create(unsigned char al, int pX, int pY, unsigned short wdh, unsigned short hgt, WindowTemplate* wTemplate);
	void SetColor(float r, float g, float b, float a = 1.0f);
	void SetOption(unsigned short option, bool execute = true);
	void SetOption(std::string option, bool execute = true);
	unsigned short GetOption();
	unsigned short GetOptionsSize();
	void AddElement(TextString text, bool active = true, bool notNull = false);
	void AddElementPtr(std::string* text, std::string* color, bool active = true, bool notNull = false);
	TextString GetElement();
	std::list<TextString> GetElements();
	void RemoveElement();
	void MoveUpElement();
	void MoveDownElement();
	void Clear();

	void SetDblAction(boost::function<void()> func);

	virtual void SetSize(unsigned short wdh, unsigned short hgt);

	int GetScrollOffset();

	void CheckInput(Keyboard& keyboard);

	virtual void Print(POINT& d, AD2D_Window& gfx);

	friend class WindowElement;
	friend class Windows;
	friend class Window;
};

class WindowElementTableMemo : public WindowElement {
private:
	std::vector<std::vector<WindowElement*> >	rows;
	std::vector<int>							cols;
	std::vector<std::string>					colsNames;

	unsigned short			option;

	float			scrollV;

	int				border;
	int				rowHeight;

	Signal			dblFunc;

public:
	WindowElementTableMemo();
	virtual ~WindowElementTableMemo();

	void Create(unsigned char al, int pX, int pY, unsigned short wdh, unsigned short hgt, WindowTemplate* wTemplate);
	void SetOption(unsigned short option, bool execute = true);
	unsigned short GetOption();
	unsigned short GetOptionsSize();
	void AddRow();
	void AddColumn(WindowElement* element);
	void SetRowHeight(int height);
	void SetColumnWidthPX(int num, int width);
	void SetColumnWidthPercent(int num, int width);
	void SetColumnName(int num, std::string name);

	void RemoveRow();
	void MoveUpRow();
	void MoveDownRow();
	void Clear();

	void SetDblAction(boost::function<void()> func);

	virtual void SetSize(unsigned short wdh, unsigned short hgt);

	int GetScrollOffset();

	void CheckInput(Keyboard& keyboard);

	virtual void Print(POINT& d, AD2D_Window& gfx);

	friend class WindowElement;
	friend class Windows;
	friend class Window;
};

class WindowElementList : public WindowElement {
private:
	std::list<std::pair<TextString, bool> >	elements;
	unsigned short			option;

	COLOR			color;

public:
	WindowElementList();
	virtual ~WindowElementList();

	void Create(unsigned char al, int pX, int pY, unsigned short wdh, unsigned short hgt, WindowTemplate* wTemplate);
	void SetColor(float r, float g, float b, float a = 1.0f);
	void SetOption(unsigned short option);
	void SetOption(std::string option);
	unsigned short GetOption();
	unsigned short GetOptionsSize();
	void AddElement(TextString text, bool active = true);
	TextString GetElement();
	void RemoveElement();
	void Clear();

	void CheckInput(Keyboard& keyboard);

	virtual void OpenMenu(Mouse& mouse, Keyboard& keyboard, std::list<MenuData>& actionList);

	virtual void Print(POINT& d, AD2D_Window& gfx);

	friend class WindowElement;
	friend class Windows;
	friend class Window;
};

class WindowElementTimer : public WindowElement {
private:
	time_lt	startTime;
	time_lt	stopTime;

	static MUTEX				lockLoop;
	THREAD						loopThread;
	Signal						loopFunc;

public:
	WindowElementTimer();
	virtual ~WindowElementTimer();

	void Create(unsigned char al, int pX, int pY, unsigned short wdh, unsigned short hgt, WindowTemplate* wTemplate);
	void SetStopTime(time_lt time);
	void SetTimeInterval(time_lt time);
	void Run();
	void Loop();
	void Stop();

	virtual void Print(POINT& d, AD2D_Window& gfx);

	friend class WindowElement;
	friend class Windows;
	friend class Window;
};

typedef std::pair<WindowElementButton*, WindowElementContainer*> TabElement;
typedef std::list<TabElement> TabList;

class WindowElementTab : public WindowElementContainer {
private:
	unsigned short		tabsHeight;

	TabList							tabList;
	WindowElementContainer*			buttonsContainer;
	WindowElementContainer*			mainContainer;
	WindowElementButton*			buttonLeft;
	WindowElementButton*			buttonRight;

	TabElement	activeTab;

	COLOR	color;

	MUTEX	lockTab;

public:
	WindowElementTab();
	virtual ~WindowElementTab();

	void Create(unsigned char al, int pX, int pY, unsigned short wdh, unsigned short hgt, bool bg, bool br, WindowTemplate* wTemplate);

	void SetColor(float r, float g, float b, float a = 1.0f);
	void SetTabsHeight(int hgt);

	virtual void SetWindow(Window* window);
	virtual void SetSize(unsigned short wdh, unsigned short hgt);

	virtual void SetBorder(int border);

	int GetTabsSize();

	void UpdateButtonsPosition();
	TabElement AddTab(std::string name, bool closeable = false);
	TabElement GetTab(WindowElementButton* button);
	TabElement GetTab(int number);
	TabElement GetActiveTab();
	TabElement* GetActiveTabPtr();
	int GetActiveTabNumber();
	void RemoveTab(WindowElementButton* button);
	void SetActiveTab(WindowElementButton* button);
	void SetActiveTabByNumber(int number);
	bool MoveActiveTab(WindowElementButton* button);
	void MoveTab(WindowElementButton* button, char dir);
	void RenameTab(WindowElementButton* button, std::string name);
	void ScrollTab(char dir);
	void ClearTabs();

	friend class WindowElement;
	friend class Windows;
	friend class Window;
};

class WindowElementItemContainer : public WindowElement {
private:
	AD2D_Image*		image;

	Item*			item;
	Creature*		creature;
	HotKey*			hotkey;

	Container*		container;
	unsigned short	slot;

public:
	Signal			funcLook;
	Signal			funcUse;

public:
	WindowElementItemContainer();
	virtual ~WindowElementItemContainer();

	void Create(unsigned char al, int pX, int pY, unsigned short wdh, unsigned short hgt, Container* container, WindowTemplate* wTemplate);
	void SetImage(AD2D_Image* image);
	void SetItem(Item* item);
	void SetCreature(Creature* creature);
	void SetHotKey(HotKey* hotkey);
	Item* GetItem();
	Creature* GetCreature();
	HotKey* GetHotKey();
	void SetSlot(unsigned short slot);
	unsigned short GetSlot();
	Container* GetContainer();
	void SetVirtualCount(unsigned char* count);

	void SetLookAction(boost::function<void()> f);
	void SetUseAction(boost::function<void()> f);

	virtual void OpenMenu(Mouse& mouse, Keyboard& keyboard, std::list<MenuData>& actionList);

	virtual void Print(POINT& d, AD2D_Window& gfx);

	friend class WindowElement;
	friend class Windows;
	friend class Window;
};

class WindowElementCheckBox : public WindowElement {
private:
	std::string		text;

	bool			checked;

	COLOR			color;

public:
	WindowElementCheckBox();
	virtual ~WindowElementCheckBox();

	void Create(unsigned char al, int pX, int pY, unsigned short wdh, WindowTemplate* wTemplate);
	void SetColor(float r, float g, float b, float a = 1.0f);
	void SetState(bool checked);
	void SetText(std::string txt);
	bool GetState();
	std::string GetText();

	virtual void Print(POINT& d, AD2D_Window& gfx);

	friend class WindowElement;
	friend class Windows;
	friend class Window;
};

class WindowElementScrollBar : public WindowElement {
private:
	TypePointer value;
	float	minValue;
	float	maxValue;

	std::string	valueStr;

public:
	WindowElementScrollBar();
	virtual ~WindowElementScrollBar();

	void Create(unsigned char al, int pX, int pY, unsigned short wdh, WindowTemplate* wTemplate);

	void SetValuePtr(TypePointer pointer);
	TypePointer GetValuePtr();

	void SetValue(float value);
	void SetMinValue(float minValue);
	void SetMaxValue(float maxValue);
	float GetValue();
	float GetMinValue();
	float GetMaxValue();

	void CheckInput(Keyboard& keyboard);

	virtual void Print(POINT& d, AD2D_Window& gfx);

	friend class WindowElement;
	friend class Windows;
	friend class Window;
};

class WindowElementHealthBar : public WindowElement {
private:
	unsigned short* value;
	unsigned short* maxValue;

public:
	WindowElementHealthBar();
	virtual ~WindowElementHealthBar();

	void Create(unsigned char al, int pX, int pY, int fSize, unsigned short* valuePtr, unsigned short* maxValuePtr, WindowTemplate* wTemplate);

	virtual void Print(POINT& d, AD2D_Window& gfx);

	friend class WindowElement;
	friend class Windows;
	friend class Window;
};

class WindowElementManaBar : public WindowElement {
private:
	unsigned short* value;
	unsigned short* maxValue;

public:
	WindowElementManaBar();
	virtual ~WindowElementManaBar();

	void Create(unsigned char al, int pX, int pY, int fSize, unsigned short* valuePtr, unsigned short* maxValuePtr, WindowTemplate* wTemplate);

	virtual void Print(POINT& d, AD2D_Window& gfx);

	friend class WindowElement;
	friend class Windows;
	friend class Window;
};

class WindowElementSkillBar : public WindowElement {
private:
	TypePointer value;
	float minValue;
	float maxValue;

	COLOR						color;
	boost::function<COLOR()>	colorFunc;

public:
	WindowElementSkillBar();
	virtual ~WindowElementSkillBar();

	void Create(unsigned char al, int pX, int pY, int wdh, int hgt, TypePointer valuePtr, WindowTemplate* wTemplate);
	void SetColor(float r, float g, float b, float a = 1.0f);
	void SetColor(boost::function<COLOR()> func);

	void SetValuePtr(TypePointer pointer);
	TypePointer GetValuePtr();

	void SetMinValue(float value);
	void SetMaxValue(float value);

	virtual std::string GetComment();

	virtual void Print(POINT& d, AD2D_Window& gfx);

	friend class WindowElement;
	friend class Windows;
	friend class Window;
};


class WindowElementPlayground : public WindowElement {
private:
	Game*	game;

public:
	WindowElementPlayground();
	virtual ~WindowElementPlayground();

	void Create(unsigned char al, int pX, int pY, unsigned short wdh, unsigned short hgt, Game* gm, WindowTemplate* wTemplate);
	virtual void SetSize(unsigned short wdh, unsigned short hgt);
	virtual void OpenMenu(Mouse& mouse, Keyboard& keyboard, std::list<MenuData>& actionList);

	POINT GetTilePos(int curX, int curY);

	virtual void Print(POINT& d, AD2D_Window& gfx);

	friend class WindowElement;
	friend class Windows;
	friend class Window;
};

class WindowElementCooldown : public WindowElement {
private:
	AD2D_Image*		icon;
	AD2D_Image*		background;

	unsigned char	groupID;
	unsigned char	spellID;

	time_lt			castTime;
	unsigned int	period;

public:
	WindowElementCooldown();
	~WindowElementCooldown();

	void Create(unsigned char al, int pX, int pY, unsigned short wdh, unsigned short hgt, WindowTemplate* wTemplate);

	void SetIcon(AD2D_Image* icon);
	void SetBackground(AD2D_Image* background);
	void SetGroupID(unsigned char id);
	void SetSpellID(unsigned char id);
	void SetCast(time_lt castTime, unsigned int period);
	unsigned int GetTimeRemain();

	virtual void Print(POINT& d, AD2D_Window& gfx);

	friend class WindowElement;
	friend class Windows;
	friend class Window;
};

class WindowElementSelect : public WindowElement {
private:
	COLOR			colorOut;
	COLOR			colorIn;
	MenuData		mdata;

	Mouse*			mouse;

public:
	WindowElementSelect();
	virtual ~WindowElementSelect();

	void Create(unsigned char al, int pX, int pY, int wdh, int hgt, Mouse* ms, MenuData* md, WindowTemplate* wTemplate);
	void SetColorOut(float r, float g, float b, float a = 1.0f);
	void SetColorIn(float r, float g, float b, float a = 1.0f);

	virtual void Print(POINT& d, AD2D_Window& gfx);
};


class WindowElementMiniMap : public WindowElement {
private:
	MiniMap*	minimap;

public:
	WindowElementMiniMap();
	virtual ~WindowElementMiniMap();

	void Create(unsigned char al, int pX, int pY, unsigned short wdh, unsigned short hgt, MiniMap* mm, WindowTemplate* wTemplate);
	virtual void OpenMenu(Mouse& mouse, Keyboard& keyboard, std::list<MenuData>& actionList);

	void SetZoom(float zoom);
	float GetZoom();
	void GetOffsetPos(float& x, float& y);
	void SetOffsetPos(float x, float y);

	virtual void Print(POINT& d, AD2D_Window& gfx);

	friend class WindowElement;
	friend class Windows;
	friend class Window;
};

class WindowElementColorMap : public WindowElement {
private:
	unsigned char index;

public:
	WindowElementColorMap();
	virtual ~WindowElementColorMap();

	void SetIndex(unsigned char index);
	unsigned char GetIndex();

	void Create(unsigned char al, int pX, int pY, unsigned short wdh, unsigned short hgt, WindowTemplate* wTemplate);

	virtual void Print(POINT& d, AD2D_Window& gfx);

	friend class WindowElement;
	friend class Windows;
	friend class Window;
};

class WindowElementBattle : public WindowElementContainer {
private:
	unsigned int creatureID;

public:
	void SetCreatureID(unsigned int id);
	unsigned int GetCreatureID();

	virtual void OpenMenu(Mouse& mouse, Keyboard& keyboard, std::list<MenuData>& actionList);

	virtual void Print(POINT& d, AD2D_Window& gfx);

	friend class WindowElement;
	friend class Windows;
	friend class Window;
};

class WindowElementVIP : public WindowElementContainer {
private:
	VIPList* viplist;
	unsigned int creatureID;

public:
	void SetVIPList(VIPList* viplist);
	VIPList* GetVIPList();

	void SetCreatureID(unsigned int id);
	unsigned int GetCreatureID();

	virtual void OpenMenu(Mouse& mouse, Keyboard& keyboard, std::list<MenuData>& actionList);

	virtual void Print(POINT& d, AD2D_Window& gfx);

	friend class WindowElement;
	friend class Windows;
	friend class Window;
};


class WindowTemplate {
public:
	std::string		templateName;

	unsigned short	elements;

	TEMPLATE_Header		tempHeader;
	TEMPLATE_Border		tempBorder;
	TEMPLATE_WndIcons	tempWndIcons;
	TEMPLATE_Container	tempContainer;
	TEMPLATE_Scroll		tempScroll;
	TEMPLATE_Button		tempButton;
	TEMPLATE_TabButton	tempTab;
	TEMPLATE_Textarea	tempTextarea;
	TEMPLATE_Memo		tempMemo;
	TEMPLATE_List		tempList;
	TEMPLATE_Timer		tempTimer;
	TEMPLATE_Item		tempItem;
	TEMPLATE_CheckBox	tempCheckBox;
	TEMPLATE_ScrollBar	tempScrollBar;
	TEMPLATE_Bar		tempHealthBar;
	TEMPLATE_Bar		tempManaBar;
	TEMPLATE_Bar		tempSkillBar;

	AD2D_Font*		font;

public:
	WindowTemplate();
	~WindowTemplate();

	void LoadTemplate(std::string path, std::string name);
	void SetFont(AD2D_Font* fnt);
	AD2D_Font* GetFont();

	unsigned short GetElements();
};


typedef std::map<std::string, WindowTemplate*> Templates;
typedef std::map<std::string, Templates> TemplateGroups;
typedef std::map<std::string, AD2D_Image*> TemplateBackgrounds;

class Windows {
private:
	AD2D_Window*				gfx;
	Game*						game;
	TemplateGroups				templatesMap;
	TemplateBackgrounds			templateBackgrounds;
	std::list<Window*>			wndList;

public:
	GUIManager					guiManager;

	static Windows*				wnds;
	static HWND					hWnd;

	static MUTEX				lockWindows;

public:
	Windows(HWND hWnd, AD2D_Window* gfx, Game* game);
	~Windows();

	AD2D_Window* GetGFX();
	Game* GetGame();

	GUIManager* GetGUIManager();

	POINT GetWindowMargin();
	POINT GetWindowSize();
	int GetWindowWidth(WINDOW_TYPE type);
	bool IsFixedPosWindow(WINDOW_TYPE type);
	void FitFixedWindows();
	void FitFixedWindows(Window* wnd, int offset);
	void FitGameWindow();
	void MatchGameWindows();

	Window* GetCollidingWindow(int offsetX, int offsetY, Window* wnd);
	POINT GetFixedOpenPosition(int offset, Window* wnd);

	void UpdateWindows(POINT oldWndSize);
	void SaveGUIWindows(bool playing);
	void LoadGUIWindows();

	Window* FindWindow(WINDOW_TYPE type);
	Window* OpenWindow(WINDOW_TYPE type, ...);
	void AddWindow(Window* wnd);
	void KillWindow(Window* wnd);
	void ReplaceWindow(Window* wOld, Window* wNew);
	void CloseWindows(WINDOW_TYPE wndType, bool immediately = false);
	void MoveOnTop(Window* wnd, bool force = false);
	void MoveDown(Window* wnd, bool force = false);
	bool IsOnList(Window* wnd);

	void LoadTemplates(AD2D_Font* font);
	void ReleaseTemplates();
	void AddWindowTemplate(std::string group, WindowTemplate* wndTemplate);
	WindowTemplate* GetWindowTemplate(std::string group, std::string name);
	void AddWindowTemplateGroupBackground(std::string group, AD2D_Image* background);
	AD2D_Image* GetWindowTemplateGroupBackground(std::string group);
	std::list<std::string> GetTemplateGroups();

	POINT AlignWindowPosition(Window* wnd);
	POINT AlignWindowSize(Window* wnd, bool _left, bool _right, bool _top, bool _bottom, std::list<Window*>* connected);
	void MoveConnectedWindows(Window* wnd, bool bleft, bool bright, bool btop, bool bbottom, int dx, int dy, std::list<Window*>* connected);
	std::list<Window*> GetConnectedWindows(Window* wnd, std::list<Window*>* currentList = NULL);

	Window* GetTopWindow();
	Window* GetWindowUnderCursor(Mouse& mouse);

	void CheckHolder(Mouse& mouse, Keyboard& keyboard, RealTime& realTime, bool inMessageLoop = false);
	void CheckInput(Mouse& mouse, Keyboard& keyboard, InputActions& actions);
	void Check(Mouse& mouse, Keyboard& keyboard, RealTime& realTime);
	void CheckAnimations(Mouse& mouse, Keyboard& keyboard, RealTime& realTime);
	void Print();

	//Lua functions
	static int LuaGetWindowTemplate(lua_State* L);
	static int LuaGetWindowTemplateFont(lua_State* L);
	static int LuaCreateWindow(lua_State* L);
	static int LuaFindWindow(lua_State* L);
	static int LuaOpenWindow(lua_State* L);
	static int LuaGetWindowByTitle(lua_State* L);
	static int LuaAddWindow(lua_State* L);
	static int LuaKillWindow(lua_State* L);
	static int LuaCloseWindow(lua_State* L);
	static int LuaMoveWindowOnTop(lua_State* L);
	static int LuaIsWindowOnList(lua_State* L);
	static int LuaSetWindowParam(lua_State* L);
	static int LuaGetWindowParam(lua_State* L);
	static int LuaCreateWindowElement(lua_State* L);
	static int LuaGetWindowElement(lua_State* L);
	static int LuaAddWindowElement(lua_State* L);
	static int LuaRemoveWindowElement(lua_State* L);
	static int LuaDeleteWindowElement(lua_State* L);
	static int LuaClearWindowElements(lua_State* L);
	static int LuaSetWindowElementParam(lua_State* L);
	static int LuaGetWindowElementParam(lua_State* L);
	static int LuaAddMemoElement(lua_State* L);
	static int LuaAddTableMemoRow(lua_State* L);
	static int LuaAddTableMemoColumn(lua_State* L);
	static int LuaAddListElement(lua_State* L);
	static int LuaTimerRun(lua_State* L);
	static int LuaTimerStop(lua_State* L);
	static int LuaGetGUIManager(lua_State* L);

	static void LuaRegisterFunctions(lua_State* L);
};


typedef std::map<Window*, std::pair<bool, bool> > ConnectionMap;

class Window {
private:
	unsigned short		elements;
	unsigned char		wndType;

	WindowTemplate*			wndTemplate;
	WindowElementContainer	wndContainer;

	std::string		title;

	bool			alwaysActive;
	int				minimized;
	int				posX;
	int				posY;
	int				width;
	int				height;
	int				defWidth, minWidth, maxWidth;
	int				defHeight, minHeight, maxHeight;

	ConnectionMap	connectedWindows;

	bool			lockToBorderWidth;
	bool			lockToBorderHeight;
	bool			lockResizeWidth;
	bool			lockResizeHeight;

	bool			closeAbility;
	bool			minimizeAbility;
	bool			hideAbility;

	WindowElement*			activeElement;

	Signal		func;
	Signal		activateFunc;
	Signal		closeFunc;
	Signal		minimizeFunc;

	std::list<unsigned char>	actions;

public:
	Window(unsigned char type, int pX, int pY, WindowTemplate* wTemplate);
	virtual ~Window();

	void SetWindowType(unsigned char type);
	unsigned char GetWindowType();
	void SetWindowTemplate(WindowTemplate* wndTemplate);
	WindowTemplate* GetWindowTemplate();
	WindowElementContainer* GetWindowContainer();

	void SetBackground(bool state);
	void SetElements(unsigned short elements);
	unsigned short GetElements();

	void SetCloseAbility(bool state);
	bool GetCloseAbility();
	void SetMinimizeAbility(bool state);
	bool GetMinimizeAbility();
	void SetHideAbility(bool state);
	bool GetHideAbility();

	bool IsUnderCursor(Mouse& mouse);
	void SetAlwaysActive(bool alwaysActive);
	void SetTitle(std::string tl);
	void SetPosition(int pX, int pY, GUIConnection* excludeConnection = NULL, bool ignoreConnection = false);
	void SetSize(int wdh, int hgt, GUIConnection* excludeConnection = NULL, bool ignoreConnection = false);
	void SetMinSize(int wdh, int hgt);
	void SetMaxSize(int wdh, int hgt);
	void SetIntSize(unsigned short wdh, unsigned short hgt);
	void SetLockToBorder(bool lockWidth, bool lockHeight);
	void SetScrollAlwaysVisible(bool horizontal, bool vertical);
	POINT GetPosition();
	POINT GetSize(bool internal = false);
	void AdjustSize();
	void AddExecuteFunction(boost::function<void()> f);
	void AddActivateFunction(boost::function<void()> f);
	void AddCloseFunction(boost::function<void()> f);
	void AddMinimizeFunction(boost::function<void()> f);
	void SetAction(unsigned char action);
	void ConnectToWindow(Window* wnd, bool connectWidth, bool connectHeight);
	void DisconnectWindow(Window* wnd);
	bool IsConnectedToWindow(Window* wnd);

	void SetActiveElement(WindowElement* element);
	void SkipActiveElement();

	void AddElement(WindowElement* wndElement);
	WindowElement* GetWindowElementUnderCursor(RECT& rect, WindowElementContainer* container, int x, int y);
	WindowElement* GetWindowElementUnderCursor(Mouse& mouse);
	WindowElement* GetActiveElement();

	void ExecuteFunction();
	void ActivateFunction();
	void CloseFunction();
	void MinimizeFunction();

	void Print(AD2D_Window& gfx, bool active);

	friend class Windows;
	friend class ContainerWindow;
	friend class GUIManager;
};

class ContainerWindow : public Window {
private:
	Container*	container;

	static std::map<unsigned char, ContainerWindow*> containers;

	static MUTEX lockContainerWindow;

public:
	ContainerWindow(unsigned char type, int pX, int pY, Container* container, WindowTemplate* wTemplate);
	virtual ~ContainerWindow();

	Container* GetContainer();
	unsigned char GetIndex();

	static void AddContainerWindow(ContainerWindow* containerWindow);
	static ContainerWindow* GetContainerWindow(unsigned char index);
	static void RemoveContainerWindow(unsigned char index);
};

#endif //__WINDOW_H_
