/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#include <dirent.h>

#include "window.h"

#include "allocator.h"
#include "bot.h"
#include "cooldowns.h"
#include "const.h"
#include "craftbox.h"
#include "filemanager.h"
#include "game.h"
#include "iniloader.h"
#include "item.h"
#include "luascript.h"
#include "map.h"
#include "player.h"
#include "protocol.h"
#include "questlog.h"
#include "servers.h"
#include "shop.h"
#include "status.h"
#include "thing.h"
#include "viplist.h"
#include "text.h"
#include "tools.h"
#include "updater.h"


// ---- WindowElement ---- //

WindowElement::WindowElement() {
	align = 0;
	posX = 0;
	posY = 0;
	width = 0;
	height = 0;
	window = NULL;
	wndTemplate = NULL;
	parent = NULL;

	func.Clear();

	flags = 0;

	active = false;
	alwaysActive = false;
	lockWidth = false;
	lockHeight = false;
	visible = true;
	enabled = true;

	border = 0;
	fontSize = 14;

	shine = 0;
}

WindowElement::~WindowElement() { }


ElementINI WindowElement::GetElementINI(INILoader& iniLoader, std::string iniTag) {
	int p = 0;

	ElementINI e;
	e.posX = (iniLoader.GetValue(iniTag, p) != "" ? atoi(iniLoader.GetValue(iniTag, p).c_str()) : -1); p++;
	e.posY = (iniLoader.GetValue(iniTag, p) != "" ? atoi(iniLoader.GetValue(iniTag, p).c_str()) : -1); p++;
	e.width = (iniLoader.GetValue(iniTag, p) != "" ? atoi(iniLoader.GetValue(iniTag, p).c_str()) : -1); p++;
	e.height = (iniLoader.GetValue(iniTag, p) != "" ? atoi(iniLoader.GetValue(iniTag, p).c_str()) : -1); p++;
	e.fontSize = (iniLoader.GetValue(iniTag, p) != "" ? atoi(iniLoader.GetValue(iniTag, p).c_str()) : -1); p++;
	e.textBorder = (iniLoader.GetValue(iniTag, p) != "" ? atoi(iniLoader.GetValue(iniTag, p).c_str()) : -1); p++;
	std::string col = iniLoader.GetValue(iniTag, p); p++;
	std::string colEx = iniLoader.GetValue(iniTag, p); p++;
	e.colorB = -1;
	e.colorExB = -1;
	e.color = COLOR(1.0f, 1.0f, 1.0f, 1.0f);
	e.colorEx = COLOR(1.0f, 1.0f, 1.0f, 1.0f);
	e.colorC = 215;
	e.colorExC = 215;

	int c = 0;
	int i = 0;
	std::string temp;
	while(c < col.length()) {
		char ch = col[c++];
		if (ch == ' ') {
			if (i == 0) e.color.red = atof(temp.c_str());
			else if (i == 1) e.color.green = atof(temp.c_str());
			else if (i == 2) e.color.blue = atof(temp.c_str());
			else if (i == 3) e.color.alpha = atof(temp.c_str());
			temp.clear();
			i++;
		}
		else temp.push_back(ch);
		e.colorB = 0;
	}
	if (i == 0 && temp != "") e.colorC = atoi(temp.c_str());
	else if (i == 1 && temp != "") e.color.green = atof(temp.c_str());
	else if (i == 2 && temp != "") e.color.blue = atof(temp.c_str());
	else if (i == 3 && temp != "") e.color.alpha = atof(temp.c_str());

	c = 0;
	i = 0;
	temp.clear();
	while(c < colEx.length()) {
		char ch = colEx[c++];
		if (ch == ' ') {
			if (i == 0) e.colorEx.red = atof(temp.c_str());
			else if (i == 1) e.colorEx.green = atof(temp.c_str());
			else if (i == 2) e.colorEx.blue = atof(temp.c_str());
			else if (i == 3) e.colorEx.alpha = atof(temp.c_str());
			temp.clear();
			i++;
		}
		else temp.push_back(ch);
		e.colorExB = 0;
	}
	if (i == 0 && temp != "") e.colorExC = atoi(temp.c_str());
	else if (i == 1 && temp != "") e.colorEx.green = atof(temp.c_str());
	else if (i == 2 && temp != "") e.colorEx.blue = atof(temp.c_str());
	else if (i == 3 && temp != "") e.colorEx.alpha = atof(temp.c_str());

	return e;
}

void WindowElement::ApplyElementINI(INILoader& iniLoader, std::string iniTag) {
	ElementINI e = GetElementINI(iniLoader, iniTag);

	WindowElementText* text = dynamic_cast<WindowElementText*>(this);
	WindowElementLink* link = dynamic_cast<WindowElementLink*>(this);
	WindowElementButton* button = dynamic_cast<WindowElementButton*>(this);
	WindowElementMemo* memo = dynamic_cast<WindowElementMemo*>(this);
	WindowElementList* list = dynamic_cast<WindowElementList*>(this);
	WindowElementTextarea* textarea = dynamic_cast<WindowElementTextarea*>(this);
	WindowElementTab* tab = dynamic_cast<WindowElementTab*>(this);
	WindowElementCheckBox* checkbox = dynamic_cast<WindowElementCheckBox*>(this);
	WindowElementSelect* select = dynamic_cast<WindowElementSelect*>(this);

	if (e.posX >= 0) posX = e.posX;
	if (e.posY >= 0) posY = e.posY;

	if (!tab) {
		if (e.width >= 0) width = e.width;
		if (e.height >= 0) height = e.height;
	}
	else {
		int wdh = (e.width >= 0 ? e.width : width);
		int hgt = (e.height >= 0 ? e.height : height);
		tab->SetSize(wdh, hgt);
	}

	if (e.fontSize >= 0) SetFontSize(e.fontSize);
	if (e.textBorder >= 0) SetBorder(e.textBorder);
	if (e.colorB >= 0) {
		if (text) text->SetColor(e.color.red, e.color.green, e.color.blue, e.color.alpha);
		if (link) link->SetColorOut(e.color.red, e.color.green, e.color.blue, e.color.alpha);
		if (button) button->SetColor(e.color.red, e.color.green, e.color.blue, e.color.alpha);
		if (memo) memo->SetColor(e.color.red, e.color.green, e.color.blue, e.color.alpha);
		if (list) list->SetColor(e.color.red, e.color.green, e.color.blue, e.color.alpha);
		if (textarea) textarea->SetColor(e.colorC);
		if (tab) tab->SetColor(e.color.red, e.color.green, e.color.blue, e.color.alpha);
		if (checkbox) checkbox->SetColor(e.color.red, e.color.green, e.color.blue, e.color.alpha);
		if (select) select->SetColorOut(e.color.red, e.color.green, e.color.blue, e.color.alpha);
	}
	if (e.colorExB >= 0) {
		if (link) link->SetColorIn(e.colorEx.red, e.colorEx.green, e.colorEx.blue, e.colorEx.alpha);
		if (select) select->SetColorIn(e.colorEx.red, e.colorEx.green, e.colorEx.blue, e.colorEx.alpha);
	}
}


void WindowElement::SetBorder(int border) {
	this->border = border;
}

void WindowElement::SetFontSize(int fontSize) {
	this->fontSize = fontSize;
}

void WindowElement::SetWindow(Window* window) {
	this->window = window;
}

void WindowElement::SetParent(WindowElementContainer* parent) {
	this->parent = parent;
}

Window* WindowElement::GetWindow() {
	return window;
}

WindowElementContainer* WindowElement::GetParent() {
	return parent;
}

void WindowElement::SetWindowTemplate(WindowTemplate* wndTemplate) {
	this->wndTemplate = wndTemplate;
}

WindowTemplate* WindowElement::GetWindowTemplate() {
	return wndTemplate;
}

void WindowElement::SetSize(unsigned short wdh, unsigned short hgt) {
	width = wdh;
	height = hgt;
}

POINT WindowElement::GetPosition() {
	POINT pos = {posX, posY};
	return pos;
}

POINT WindowElement::GetSize() {
	POINT size = {width, height};
	return size;
}

void WindowElement::SetFlags(unsigned int flags) {
	this->flags |= flags;
}

bool WindowElement::CheckFlags(unsigned int flags) {
	if (this->flags & flags == flags)
		return true;

	return false;
}

void WindowElement::ClearFlags(unsigned int flags) {
	this->flags &= ~flags;
}

void WindowElement::SetPosition(int pX, int pY) {
	posX = pX;
	posY = pY;
}

void WindowElement::SetAlwaysActive(bool alwaysActive) {
	this->alwaysActive = alwaysActive;
}

void WindowElement::SetVisible(bool state) {
	this->visible = state;
}

void WindowElement::SetEnabled(bool state) {
	this->enabled = state;
}

void WindowElement::SetShine(unsigned int shine) {
	this->shine = shine;
}

unsigned int WindowElement::GetShine() {
	return shine;
}

bool WindowElement::IsActive() {
	return this->active;
}

void WindowElement::SetComment(std::string cmt) {
	this->comment = cmt;
}

std::string WindowElement::GetComment() {
	return comment;
}

POINT WindowElement::GetParentScrollOffset() {
	POINT ret = {0, 0};
	if (parent) {
		ret.x = (int)parent->scrollH;
		ret.y = (int)parent->scrollV;
	}

	return ret;
}

POINT WindowElement::AdjustViewport(AD2D_Viewport& current, AD2D_Viewport& next) {
	int cX1 = current.startX;
	int cY1 = current.startY;
	int cX2 = current.startX + current.width - 1;
	int cY2 = current.startY + current.height - 1;

	int nX1 = next.startX;
	int nY1 = next.startY;
	int nX2 = next.startX + next.width - 1;
	int nY2 = next.startY + next.height - 1;

	POINT diff = {0, 0};

	bool hChange = false;
	bool vChange = false;

	if (cX1 > nX1) { diff.x = cX1 - nX1;	nX1 = cX1;	hChange = true; }
	if (cY1 > nY1) { diff.y = cY1 - nY1;	nY1 = cY1;	vChange = true; }
	if (cX2 < nX2) { nX2 = cX2;	hChange = true; }
	if (cY2 < nY2) { nY2 = cY2;	vChange = true; }

	if (cX1 > nX2) { nX2 = cX1 - 1;	hChange = true; }
	if (cY1 > nY2) { nY2 = cY1 - 1;	vChange = true; }
	if (cX2 < nX1) { nX1 = cX2;	hChange = true; }
	if (cY2 < nY1) { nY1 = cY2;	vChange = true; }

	if (hChange && nX1 == nX2) nX2 = nX1 - 1;
	if (vChange && nY1 == nY2) nY2 = nY1 - 1;

	next.startX = nX1;
	next.startY = nY1;
	next.width = nX2 - nX1 + 1;
	next.height = nY2 - nY1 + 1;

	return diff;
}

POINT WindowElement::GetAbsolutePosition(bool withWindow) {
	POINT abs = {posX, posY};

	WindowElementContainer* parent = this->parent;
	while(parent) {
		int borderH = (parent->border ? wndTemplate->tempContainer.l_width : 0);
		int borderV = (parent->border ? wndTemplate->tempContainer.t_height : 0);
		abs.x += borderH + parent->posX - (int)parent->scrollH;
		abs.y += borderV + parent->posY - (int)parent->scrollV;
		parent = parent->parent;
	}

	if (withWindow) {
		POINT pos = {0, 0};
		if (window)
			pos = window->GetPosition();
		abs.x += pos.x;
		abs.y += pos.y;
	}

	return abs;
}

void WindowElement::SetLocks(bool wdh, bool hgt) {
	lockWidth = wdh;
	lockHeight = hgt;
}

void WindowElement::SetAction(boost::function<void()> f) {
	if (!f) {
		func.Clear();
		return;
	}

	func.PushFunction(f);
}

void WindowElement::SetOnActivate(boost::function<void()> f) {
	if (!f) {
		funcOnActivate.Clear();
		return;
	}

	funcOnActivate.PushFunction(f);
}

void WindowElement::SetOnDeactivate(boost::function<void()> f) {
	if (!f) {
		funcOnDeactivate.Clear();
		return;
	}

	funcOnDeactivate.PushFunction(f);
}

void WindowElement::OpenMenu(Mouse& mouse, Keyboard& keyboard, std::list<MenuData>& actionList) { }

void WindowElement::Print(POINT& d, AD2D_Window& gfx) {
	AD2D_Viewport currentVP = gfx.GetCurrentViewport();
	AD2D_Viewport nextVP;

	POINT scroll = GetParentScrollOffset();
	int offsetH = scroll.x;
	int offsetV = scroll.y;

	nextVP.startX = currentVP.startX + posX - offsetH - d.x;
	nextVP.startY = currentVP.startY + posY - offsetV - d.y;
	nextVP.width = width;
	nextVP.height = height;

	POINT diff = AdjustViewport(currentVP, nextVP);

	gfx.SetViewport(nextVP);
	gfx.SetViewport(currentVP);
}

void WindowElement::PrintShine(POINT& d, AD2D_Window& gfx) {
	unsigned char* ch_shine = (unsigned char*)&shine;
	COLOR color((float)(ch_shine[0]) / 255,
				(float)(ch_shine[1]) / 255,
				(float)(ch_shine[2]) / 255, 1.0f);
	int strength = ch_shine[3] % 4;
	int mode = ch_shine[3] / 4;

	if (strength == 0)
		return;

    AD2D_Image* shiner = Icons::GetShinerIcon();
	if (!shine)
		return;

	COLOR currentColor = AD2D_Window::GetColor();
	float rand_shine = 0.0f;
	if ((int)(color.alpha * 1000) != 0)
		rand_shine = (float)(rand() % (int)(color.alpha * 1000)) / 2000;

	if (mode == 0)
    	gfx.SetBlendFunc(BLEND_SRCALPHA, BLEND_ONE);
	else
		gfx.SetBlendFunc(BLEND_SRCALPHA, BLEND_INVSRCALPHA);

	AD2D_Window::SetColor(color.red, color.green, color.blue, currentColor.alpha * (0.25f + rand_shine));

	float XX = 0.05 * width * strength;
	float YY = 0.05 * height * strength;
	gfx.PutImage(posX - XX, posY - YY, posX + width + XX, posY + height + YY, *shiner);

	gfx.SetBlendFunc(BLEND_SRCALPHA, BLEND_INVSRCALPHA);
	AD2D_Window::SetColor(currentColor.red, currentColor.green, currentColor.blue, currentColor.alpha);
}


// ---- WindowElementContainer ---- //

WindowElementContainer::WindowElementContainer() {
	intWidth = 0;
	intHeight = 0;
	scrollH = 0.0f;
	scrollV = 0.0f;
}

WindowElementContainer::~WindowElementContainer() {
	std::list<WindowElement*>::iterator it = elements.begin();
	for (it; it != elements.end(); it++) {
		WindowElement* element = *it;
		if (element)
			delete_debug(element, M_PLACE);
	}

	elements.clear();
}

void WindowElementContainer::Create(unsigned char al, int pX, int pY, unsigned short wdh, unsigned short hgt, bool bg, bool br, WindowTemplate* wTemplate) {
	align = al;
	posX = pX;
	posY = pY;
	width = wdh;
	height = hgt;
	cntbackground = bg;
	cntborder = br;
	wndTemplate = wTemplate;

	scroll = true;
	hAlwaysVisible = false;
	vAlwaysVisible = false;
}

void WindowElementContainer::SetScroll(bool scroll) {
	this->scroll = scroll;
}

void WindowElementContainer::SetScrollAlwaysVisible(bool horizontal, bool vertical) {
	hAlwaysVisible = horizontal;
	vAlwaysVisible = vertical;
}

void WindowElementContainer::SetWindow(Window* window) {
	std::list<WindowElement*>::iterator it = elements.begin();
	for (it; it != elements.end(); it++) {
		WindowElement* element = *it;
		if (element)
			element->SetWindow(window);
	}

	this->window = window;
}

void WindowElementContainer::SetSize(unsigned short wdh, unsigned short hgt) {
	int oldWidth = width;
	int oldHeight = height;
	POINT oldOffset = GetScrollOffset();

	width = wdh;
	height = hgt;

	POINT offset = GetScrollOffset();
	if (scrollH > (float)offset.x) scrollH = (float)offset.x;
	if (scrollV > (float)offset.y) scrollV = (float)offset.y;

	int resizeIntWidth = 0;
	int resizeIntHeight = 0;

	std::list<WindowElement*>::iterator it;
	for (it = elements.begin(); it != elements.end(); it++) {
		int scrollWdh = (vAlwaysVisible ? 0 : (oldOffset.y && !offset.y ? wndTemplate->tempScroll.w : (!oldOffset.y && offset.y ? - wndTemplate->tempScroll.w : 0)));
		int scrollHgt = (hAlwaysVisible ? 0 : (oldOffset.x && !offset.x ? wndTemplate->tempScroll.h : (!oldOffset.x && offset.x ? - wndTemplate->tempScroll.h : 0)));

		if ((*it)->lockWidth || (*it)->lockHeight) {
			int newWidth = ((*it)->lockWidth ? (*it)->width + (wdh - oldWidth) + scrollWdh : (*it)->width);
			int newHeight = ((*it)->lockHeight ? (*it)->height + (hgt - oldHeight) + scrollHgt : (*it)->height);
			(*it)->SetSize(newWidth, newHeight);
		}
		if ((*it)->align & ALIGN_H_RIGHT) {
			(*it)->posX += (wdh - oldWidth) + scrollWdh;
		}
		if ((*it)->align & ALIGN_V_BOTTOM) {
			(*it)->posY += (hgt - oldHeight) + scrollHgt;
		}
	}
}

void WindowElementContainer::AdjustSize() {
	if (intWidth == 0 && intHeight == 0)
		return;

	int hScroll = 0;
	int vScroll = 0;
	if (this->scroll && wndTemplate->GetElements() & ELEMENT_SCROLL) {
		hScroll = (hAlwaysVisible ? wndTemplate->tempScroll.h : 0);
		vScroll = (vAlwaysVisible ? wndTemplate->tempScroll.w : 0);
	}

	int extWidth = intWidth + wndTemplate->tempContainer.l_width + wndTemplate->tempContainer.r_width + vScroll;
	int extHeight = intHeight + wndTemplate->tempContainer.t_height + wndTemplate->tempContainer.b_height + hScroll;

	SetSize(extWidth, extHeight);
}

void WindowElementContainer::SetIntSize(unsigned short wdh, unsigned short hgt) {
	intWidth = wdh;
	intHeight = hgt;
}

POINT WindowElementContainer::GetScrollOffset() {
	int borderH = 0;
	int borderV = 0;
	if (border && wndTemplate->GetElements() & ELEMENT_CONTAINER) {
		borderH = wndTemplate->tempContainer.l_width + wndTemplate->tempContainer.r_width;
		borderV = wndTemplate->tempContainer.t_height + wndTemplate->tempContainer.b_height;
	}

	int hScroll = 0;
	int vScroll = 0;
	if (this->scroll && wndTemplate->GetElements() & ELEMENT_SCROLL) {
		hScroll = (hAlwaysVisible ? wndTemplate->tempScroll.h : 0);
		vScroll = (vAlwaysVisible ? wndTemplate->tempScroll.w : 0);
	}

	int horizontal = 0;
	int vertical = 0;
	if (width - borderH - vScroll < intWidth) {
		horizontal = intWidth - (width - borderH - vScroll);
		if (this->scroll && wndTemplate->GetElements() & ELEMENT_SCROLL && height - borderV - wndTemplate->tempScroll.h < intHeight) {
			vertical = intHeight - (height - borderV - wndTemplate->tempScroll.h);
			horizontal += wndTemplate->tempScroll.w - vScroll;
		}
	}
	else if (height - borderV - hScroll < intHeight) {
		vertical = intHeight - (height - borderV - hScroll);
		if (this->scroll && wndTemplate->GetElements() & ELEMENT_SCROLL && width - borderH - wndTemplate->tempScroll.w < intWidth) {
			horizontal = intWidth - (width - borderH - wndTemplate->tempScroll.w);
			vertical += wndTemplate->tempScroll.h - hScroll;
		}
	}

	POINT ret = {horizontal, vertical};
	return ret;
}

POINT WindowElementContainer::GetIntSize() {
	int borderH = 0;
	int borderV = 0;
	if (border && wndTemplate->GetElements() & ELEMENT_CONTAINER) {
		borderH = wndTemplate->tempContainer.l_width + wndTemplate->tempContainer.r_width;
		borderV = wndTemplate->tempContainer.t_height + wndTemplate->tempContainer.b_height;
	}

	int hScroll = 0;
	int vScroll = 0;
	if (this->scroll && wndTemplate->GetElements() & ELEMENT_SCROLL) {
		hScroll = (hAlwaysVisible ? wndTemplate->tempScroll.h : 0);
		vScroll = (vAlwaysVisible ? wndTemplate->tempScroll.w : 0);
	}

	POINT ret;
	ret.x = width - borderH - vScroll;
	ret.y = height - borderV - hScroll;

	return ret;
}

void WindowElementContainer::Print(POINT& d, AD2D_Window& gfx) {
	AD2D_Viewport currentVP = gfx.GetCurrentViewport();

	POINT scroll = GetParentScrollOffset();
	int offsetH = scroll.x;
	int offsetV = scroll.y;

	AD2D_Viewport nextVP;
	nextVP.startX = currentVP.startX + posX - offsetH - d.x;
	nextVP.startY = currentVP.startY + posY - offsetV - d.y;
	nextVP.width = width;
	nextVP.height = height;
	POINT diff = AdjustViewport(currentVP, nextVP);

	gfx.SetViewport(nextVP);

	if ((cntborder || !parent) && cntbackground && wndTemplate->GetElements() & ELEMENT_CONTAINER) {
		POINT diff;
		AD2D_Viewport containerVP;

		containerVP.Create(	currentVP.startX + posX + wndTemplate->tempContainer.l_width - offsetH - d.x,
							currentVP.startY + posY + wndTemplate->tempContainer.t_height - offsetV - d.y,
							currentVP.startX + posX + width - wndTemplate->tempContainer.r_width - offsetH - d.x - 1,
							currentVP.startY + posY + height - wndTemplate->tempContainer.b_height - offsetV - d.y - 1);
		diff = AdjustViewport(nextVP, containerVP);
		gfx.SetViewport(containerVP);
		if (wndTemplate->tempContainer.bg.GetWidth() > 0) {
			for (int x = - (int)scrollH % wndTemplate->tempContainer.bg.GetWidth(); x < width + wndTemplate->tempContainer.bg.GetWidth(); x += wndTemplate->tempContainer.bg.GetWidth())
			for (int y = - (int)scrollV % wndTemplate->tempContainer.bg.GetHeight(); y < height + wndTemplate->tempContainer.bg.GetHeight(); y += wndTemplate->tempContainer.bg.GetHeight())
				gfx.PutImage(x - diff.x, y - diff.y, wndTemplate->tempContainer.bg);
		}

		gfx.SetViewport(nextVP);
	}

	AD2D_Viewport intNextVP;
	POINT intDiff = diff;

	if (cntborder && wndTemplate->GetElements() & ELEMENT_CONTAINER) {
		int x = 0;
		int y = 0;

		POINT diff;
		AD2D_Viewport containerVP;

		containerVP.Create(	currentVP.startX + posX + wndTemplate->tempContainer.l_width - offsetH - d.x,
							currentVP.startY + posY - offsetV - d.y,
							currentVP.startX + posX + width - wndTemplate->tempContainer.r_width - offsetH - d.x - 1,
							currentVP.startY + posY + wndTemplate->tempContainer.t.GetHeight() - offsetV - d.y - 1);
		diff = AdjustViewport(nextVP, containerVP);
		gfx.SetViewport(containerVP);
		if (wndTemplate->tempContainer.t.GetWidth() > 0) {
			for (x = 0; x < width - wndTemplate->tempContainer.l_width - wndTemplate->tempContainer.r_width; x += wndTemplate->tempContainer.t.GetWidth())
				gfx.PutImage(x - diff.x, - diff.y, wndTemplate->tempContainer.t);
		}

		containerVP.Create(	currentVP.startX + posX - offsetH - d.x,
							currentVP.startY + posY + wndTemplate->tempContainer.t_height - offsetV - d.y,
							currentVP.startX + posX + wndTemplate->tempContainer.l.GetWidth() - offsetH - d.x - 1,
							currentVP.startY + posY + height - wndTemplate->tempContainer.b_height - offsetV - d.y - 1);
		diff = AdjustViewport(nextVP, containerVP);
		gfx.SetViewport(containerVP);
		if (wndTemplate->tempContainer.l.GetWidth() > 0) {
			for (y = 0; y < height - wndTemplate->tempContainer.t_height - wndTemplate->tempContainer.b_height; y += wndTemplate->tempContainer.l.GetHeight())
				gfx.PutImage(- diff.x, y - diff.y, wndTemplate->tempContainer.l);
		}

		containerVP.Create(	currentVP.startX + posX + width - wndTemplate->tempContainer.r.GetWidth() - offsetH - d.x,
							currentVP.startY + posY + wndTemplate->tempContainer.t_height - offsetV - d.y,
							currentVP.startX + posX + width - offsetH - d.x - 1,
							currentVP.startY + posY + height - wndTemplate->tempContainer.b_height - offsetV - d.y - 1);
		diff = AdjustViewport(nextVP, containerVP);
		gfx.SetViewport(containerVP);
		if (wndTemplate->tempContainer.r.GetWidth() > 0) {
			for (y = 0; y < height - wndTemplate->tempContainer.t_height - wndTemplate->tempContainer.b_height; y += wndTemplate->tempContainer.r.GetHeight())
				gfx.PutImage(- diff.x, y - diff.y, wndTemplate->tempContainer.r);
		}

		containerVP.Create(	currentVP.startX + posX + wndTemplate->tempContainer.l_width - offsetH - d.x,
							currentVP.startY + posY + height - wndTemplate->tempContainer.b.GetHeight() - offsetV - d.y,
							currentVP.startX + posX + width - wndTemplate->tempContainer.r_width - offsetH - d.x - 1,
							currentVP.startY + posY + height - offsetV - d.y - 1);
		diff = AdjustViewport(nextVP, containerVP);
		gfx.SetViewport(containerVP);
		if (wndTemplate->tempContainer.b.GetWidth() > 0) {
			for (x = 0; x < width - wndTemplate->tempContainer.l_width - wndTemplate->tempContainer.r_width; x += wndTemplate->tempContainer.b.GetWidth())
				gfx.PutImage(x - diff.x, - diff.y, wndTemplate->tempContainer.b);
		}

		containerVP.Create(	currentVP.startX + posX - offsetH - d.x,
							currentVP.startY + posY - offsetV - d.y,
							currentVP.startX + posX + width - offsetH - d.x - 1,
							currentVP.startY + posY + height - offsetV - d.y - 1);
		diff = AdjustViewport(nextVP, containerVP);
		gfx.SetViewport(containerVP);
		gfx.PutImage(- diff.x, - diff.y, wndTemplate->tempContainer.tl);
		gfx.PutImage(width - wndTemplate->tempContainer.tr.GetWidth() - diff.x, - diff.y, wndTemplate->tempContainer.tr);
		gfx.PutImage(- diff.x, height - wndTemplate->tempContainer.bl.GetHeight() - diff.y, wndTemplate->tempContainer.bl);
		gfx.PutImage(width - wndTemplate->tempContainer.br.GetWidth() - diff.x, height - wndTemplate->tempContainer.br.GetHeight() - diff.y, wndTemplate->tempContainer.br);

		intNextVP.startX = currentVP.startX + posX + wndTemplate->tempContainer.l_width - offsetH - d.x;
		intNextVP.startY = currentVP.startY + posY + wndTemplate->tempContainer.t_height - offsetV - d.y;
		intNextVP.width = width - wndTemplate->tempContainer.l_width - wndTemplate->tempContainer.r_width;
		intNextVP.height = height - wndTemplate->tempContainer.t_height - wndTemplate->tempContainer.b_height;
		intDiff = AdjustViewport(nextVP, intNextVP);
		gfx.SetViewport(intNextVP);
	}
	else
		gfx.SetViewport(nextVP);

	std::list<WindowElement*>::iterator it = elements.begin();
	for (it; it != elements.end(); it++) {
		WindowElement* wndElement = *it;
		if (wndElement && wndElement->visible) {
			if (wndElement->GetShine())
				wndElement->PrintShine(intDiff, gfx);

			wndElement->Print(intDiff, gfx);
		}
	}

	gfx.SetViewport(nextVP);

	if (this->scroll && wndTemplate->GetElements() & ELEMENT_SCROLL) {
		int left = 0;
		int right = 0;
		int top = 0;
		int bottom = 0;
		if (border) {
			left = wndTemplate->tempContainer.l_width;
			right = wndTemplate->tempContainer.r_width;
			top = wndTemplate->tempContainer.t_height;
			bottom = wndTemplate->tempContainer.b_height;
		}
		POINT offset = GetScrollOffset();
		if (offset.x || hAlwaysVisible) {
			int x = 0;

			POINT diff;
			AD2D_Viewport scrollVP;
			scrollVP.Create(currentVP.startX + posX + left - offsetH - d.x,
							currentVP.startY + posY + height - bottom - wndTemplate->tempScroll.h - offsetV - d.y,
							currentVP.startX + posX + width - right - offsetH - d.x - 1,
							currentVP.startY + posY + height - bottom - offsetV - d.y - 1);
			diff = AdjustViewport(nextVP, scrollVP);
			gfx.SetViewport(scrollVP);

			for (x = 0; x < width - left - right; x += wndTemplate->tempScroll.bgh.GetWidth())
				gfx.PutImage(x - diff.x, - diff.y, wndTemplate->tempScroll.bgh);
			gfx.PutImage(- diff.x, - diff.y, wndTemplate->tempScroll.l);
			gfx.PutImage(width - left - right - wndTemplate->tempScroll.r.GetWidth() - diff.x, - diff.y, wndTemplate->tempScroll.r);

			if (offset.x) {
				float position = scrollH / offset.x;
				gfx.PutImage(wndTemplate->tempScroll.l.GetWidth() + (int)((width - left - right - wndTemplate->tempScroll.l.GetWidth() - wndTemplate->tempScroll.r.GetWidth() - wndTemplate->tempScroll.brh.GetWidth()) * position) - diff.x, - diff.y, wndTemplate->tempScroll.brh);
			}
		}
		if (offset.y || vAlwaysVisible) {
			int y = 0;
			int h = 0;
			if (offset.x || hAlwaysVisible) h += wndTemplate->tempScroll.h;

			POINT diff;
			AD2D_Viewport scrollVP;
			scrollVP.Create(currentVP.startX + posX + width - right - wndTemplate->tempScroll.w - offsetH - d.x,
							currentVP.startY + posY + top - offsetV - d.y,
							currentVP.startX + posX + width - right - offsetH - d.x - 1,
							currentVP.startY + posY + height - bottom - h - offsetV - d.y - 1);
			diff = AdjustViewport(nextVP, scrollVP);
			gfx.SetViewport(scrollVP);

			for (y = 0; y < height - top - bottom - h; y += wndTemplate->tempScroll.bgv.GetHeight())
				gfx.PutImage(- diff.x, y - diff.y, wndTemplate->tempScroll.bgv);
			gfx.PutImage(- diff.x, - diff.y, wndTemplate->tempScroll.t);
			gfx.PutImage(- diff.x, height - top - bottom - wndTemplate->tempScroll.b.GetHeight() - h - diff.y, wndTemplate->tempScroll.b);

			if (offset.y) {
				float position = scrollV / offset.y;
				gfx.PutImage(- diff.x, wndTemplate->tempScroll.t.GetHeight() + (int)((height - top - bottom - wndTemplate->tempScroll.t.GetHeight() - wndTemplate->tempScroll.b.GetHeight() - wndTemplate->tempScroll.brv.GetHeight() - h) * position) - diff.y, wndTemplate->tempScroll.brv);
			}
		}
	}

	gfx.SetViewport(currentVP);
}

void WindowElementContainer::SetContainerBackground(bool state) {
	this->cntbackground = state;
}

void WindowElementContainer::SetContainerBorder(bool state) {
	this->cntborder = state;
}

void WindowElementContainer::AddElement(WindowElement* wndElement) {
	if (wndElement) {
		if (!wndElement->parent) {
			wndElement->SetWindow(this->window);
			wndElement->SetParent(this);
		}
		else
			return;
	}
	else
		return;

	elements.push_back(wndElement);
}

void WindowElementContainer::RemoveElement(WindowElement* wndElement) {
	if (wndElement) {
		wndElement->SetWindow(NULL);
		wndElement->SetParent(NULL);
	}
	else
		return;

	std::list<WindowElement*>::iterator it = std::find(elements.begin(), elements.end(), wndElement);
	if (it != elements.end())
		elements.erase(it);
}

void WindowElementContainer::RemoveAllElements() {
	std::list<WindowElement*>::iterator it = elements.begin();
	for (it; it != elements.end(); it++) {
		WindowElement* element = *it;
		if (element) {
			element->SetWindow(NULL);
			element->SetParent(NULL);
		}
	}

	elements.clear();
}

void WindowElementContainer::DeleteAllElements() {
	std::list<WindowElement*>::iterator it = elements.begin();
	for (it; it != elements.end(); it++) {
		WindowElement* element = *it;
		if (element)
			delete_debug(element, M_PLACE);
	}

	elements.clear();
}

unsigned int WindowElementContainer::GetElementsSize() {
	return elements.size();
}


// ---- WindowElementText ---- //

WindowElementText::WindowElementText() { }

WindowElementText::~WindowElementText() { }

void WindowElementText::Create(unsigned char al, int pX, int pY, int wgh, WindowTemplate* wTemplate) {
	align = al;
	posX = pX;
	posY = pY;
	width = wgh;
	height = 14;
	wndTemplate = wTemplate;

	border = 0;

	txtAlign = 0;
	fontSize = 14;
	precision = 2;

	color = COLOR(1.0f, 1.0f, 1.0f, 1.0f);
}

void WindowElementText::SetText(TextString txt) {
	text = txt;
	height = fontSize + border * 2;
}

void WindowElementText::SetText(std::string txt) {
	text = TextString(txt);
	height = fontSize + border * 2;
}

void WindowElementText::SetBorder(int border) {
	this->border = border;
	height = fontSize + border * 2;
}

void WindowElementText::SetAlign(int align) {
	this->txtAlign = align;
}

void WindowElementText::SetFontSize(int fontSize) {
	this->fontSize = fontSize;
	height = fontSize + border * 2;
}

void WindowElementText::SetPrecision(int precision) {
	this->precision = precision;
}

TextString WindowElementText::GetText() {
	return text;
}

void WindowElementText::SetColor(float r, float g, float b, float a) {
	color.red = r;
	color.green = g;
	color.blue = b;
	color.alpha = a;
}

void WindowElementText::SetColor(boost::function<COLOR()> func) {
	colorFunc = func;
}

void WindowElementText::SetValuePtr(TypePointer pointer) {
	value = pointer;
}

TypePointer WindowElementText::GetValuePtr() {
	return value;
}


void WindowElementText::Print(POINT& d, AD2D_Window& gfx) {
	AD2D_Viewport currentVP = gfx.GetCurrentViewport();
	AD2D_Viewport nextVP;

	POINT scroll = GetParentScrollOffset();
	int offsetH = scroll.x;
	int offsetV = scroll.y;

	nextVP.startX = currentVP.startX + posX - offsetH - d.x - border;
	nextVP.startY = currentVP.startY + posY - offsetV - d.y - border;
	nextVP.width = width;
	nextVP.height = height;

	POINT diff = AdjustViewport(currentVP, nextVP);

	gfx.SetViewport(nextVP);

	COLOR currentColor = AD2D_Window::GetColor();

	std::string textValue;
	int textWidth = 0;

	if (value.ptr) {
		if (CheckFlags(FLAG_TEXTTIME)) {
			unsigned int UINTvalue = atoi(value.GetValue(0).c_str());
			int hours = UINTvalue / 60;
			int minutes = UINTvalue % 60;
			textValue = (hours < 10 ? std::string("0" + value2str(hours)) : value2str(hours)) + ":" + (minutes < 10 ? std::string("0" + value2str(minutes)) : value2str(minutes));
		}
		else
			textValue = value.GetValue(precision);

		textWidth = wndTemplate->font->GetTextWidth(textValue, fontSize) + border + 1;
	}
	else if (txtAlign)
		textWidth = wndTemplate->font->GetTextWidth(text.text, fontSize) + border + 1;

	int txtPos = 0;
	if (txtAlign == 1 && !value.ptr)
		txtPos = (width - textWidth) / 2;
	else if (txtAlign == 2 && !value.ptr)
		txtPos = (width - textWidth);

	if (colorFunc)
		color = colorFunc();

	AD2D_Window::SetColor(color.red * currentColor.red, color.green * currentColor.green, color.blue * currentColor.blue, color.alpha * currentColor.alpha);
	gfx.CPrint(- diff.x + txtPos, - diff.y, fontSize, *wndTemplate->font, text.text.c_str(), text.color.c_str(), border);
	if (value.ptr)
		gfx.Print(- diff.x + width - textWidth, - diff.y, fontSize, *wndTemplate->font, textValue.c_str(), border);
	AD2D_Window::SetColor(currentColor.red, currentColor.green, currentColor.blue, currentColor.alpha);

	gfx.SetViewport(currentVP);
}


// ---- WindowElementLink ---- //

WindowElementLink::WindowElementLink() { }

WindowElementLink::~WindowElementLink() {
	mdata.ReleaseData();
}

void WindowElementLink::Create(unsigned char al, int pX, int pY, int wdh, Mouse* ms, MenuData* md, WindowTemplate* wTemplate) {
	align = al;
	posX = pX;
	posY = pY;
	width = wdh;
	height = 0;
	mouse = ms;

	if (md)
		mdata = *md;

	wndTemplate = wTemplate;

	border = 0;

	txtAlign = 0;
	fontSize = 14;

	colorOut = COLOR(1.0f, 1.0f, 1.0f, 1.0f);
	colorIn = COLOR(0.6f, 0.6f, 0.6f, 1.0f);
}

void WindowElementLink::SetText(TextString txt) {
	text = txt;
	height = fontSize + border * 2;
}

void WindowElementLink::SetText(std::string txt) {
	text = TextString(txt);
	height = fontSize + border * 2;
}

void WindowElementLink::SetBorder(int border) {
	this->border = border;
	height = fontSize + border * 2;
}

void WindowElementLink::SetAlign(int align) {
	this->txtAlign = align;
}

void WindowElementLink::SetFontSize(int fontSize) {
	this->fontSize = fontSize;
	height = fontSize + border * 2;
}

TextString WindowElementLink::GetText() {
	return text;
}

void WindowElementLink::SetColorOut(float r, float g, float b, float a) {
	colorOut.red = r;
	colorOut.green = g;
	colorOut.blue = b;
	colorOut.alpha = a;
}

void WindowElementLink::SetColorIn(float r, float g, float b, float a) {
	colorIn.red = r;
	colorIn.green = g;
	colorIn.blue = b;
	colorIn.alpha = a;
}

void WindowElementLink::Print(POINT& d, AD2D_Window& gfx) {
	AD2D_Viewport currentVP = gfx.GetCurrentViewport();
	AD2D_Viewport nextVP;

	POINT scroll = GetParentScrollOffset();
	int offsetH = scroll.x;
	int offsetV = scroll.y;

	nextVP.startX = currentVP.startX + posX - offsetH - d.x - border;
	nextVP.startY = currentVP.startY + posY - offsetV - d.y - border;
	nextVP.width = width;
	nextVP.height = height;

	POINT diff = AdjustViewport(currentVP, nextVP);

	gfx.SetViewport(nextVP);

	COLOR currentColor = AD2D_Window::GetColor();

	int textWidth = 0;

	if (txtAlign)
		textWidth = wndTemplate->font->GetTextWidth(text.text, fontSize) + border + 1;

	int txtPos = 0;
	if (txtAlign == 1)
		txtPos = (width - textWidth) / 2;
	else if (txtAlign == 2)
		txtPos = (width - textWidth);

	Window* window = GetWindow();
	POINT wndPos = window->GetPosition();
	POINT wndSize = window->GetSize();

	POINT pos = GetAbsolutePosition(true);
	if (window &&
		mouse->curX >= wndPos.x && mouse->curX < wndPos.x + wndSize.x && mouse->curY >= wndPos.y && mouse->curY < wndPos.y + wndSize.y &&
		mouse->curX >= pos.x && mouse->curX < pos.x + width && mouse->curY >= pos.y && mouse->curY < pos.y + height)
			AD2D_Window::SetColor(colorIn.red * currentColor.red, colorIn.green * currentColor.green, colorIn.blue * currentColor.blue, colorIn.alpha * currentColor.alpha);
	else
		AD2D_Window::SetColor(colorOut.red * currentColor.red, colorOut.green * currentColor.green, colorOut.blue * currentColor.blue, colorOut.alpha * currentColor.alpha);
	gfx.CPrint(- diff.x + txtPos, - diff.y, fontSize, *wndTemplate->font, text.text.c_str(), text.color.c_str(), border);
	AD2D_Window::SetColor(currentColor.red, currentColor.green, currentColor.blue, currentColor.alpha);

	gfx.SetViewport(currentVP);
}


// ---- WindowElementImage ---- //

WindowElementImage::WindowElementImage() {
	image = NULL;
}

WindowElementImage::~WindowElementImage() { }

void WindowElementImage::Create(unsigned char al, int pX, int pY, int wdh, int hgt, AD2D_Image* img, WindowTemplate* wTemplate) {
	if (!img)
		return;

	align = al;
	posX = pX;
	posY = pY;
	width = wdh;
	height = hgt;

	if (width == 0 || height == 0) {
		width = img->GetWidth();
		height = img->GetHeight();
	}

	wndTemplate = wTemplate;
	image = img;
}

void WindowElementImage::Create(unsigned char al, int pX, int pY, AD2D_Image* img, WindowTemplate* wTemplate) {
	if (!img)
		return;

	Create(al, pX, pY, img->GetWidth(), img->GetHeight(), img, wTemplate);
}

void WindowElementImage::SetImage(AD2D_Image* img) {
	image = img;
}

void WindowElementImage::Print(POINT& d, AD2D_Window& gfx) {
	AD2D_Viewport currentVP = gfx.GetCurrentViewport();
	AD2D_Viewport nextVP;

	POINT scroll = GetParentScrollOffset();
	int offsetH = scroll.x;
	int offsetV = scroll.y;

	nextVP.startX = currentVP.startX + posX - offsetH - d.x;
	nextVP.startY = currentVP.startY + posY - offsetV - d.y;
	nextVP.width = width;
	nextVP.height = height;

	POINT diff = AdjustViewport(currentVP, nextVP);

	gfx.SetViewport(nextVP);

	if (image)
		gfx.PutImage(0 - diff.x, 0 - diff.y, width - diff.x, height - diff.y, *image);

	gfx.SetViewport(currentVP);
}


// ---- WindowElementButton ---- //

WindowElementButton::WindowElementButton() { }

WindowElementButton::~WindowElementButton() { }

void WindowElementButton::Create(unsigned char al, unsigned char tp, int pX, int pY, unsigned short wdh, unsigned short hgt, WindowTemplate* wTemplate) {
	align = al;
	posX = pX;
	posY = pY;
	width = wdh;
	height = hgt;

	wndTemplate = wTemplate;

	image = NULL;
	text = "";
	pressed = false;
	inverse = false;
	closeIcon = false;
	type = tp;

	fontSize = 14;

	SetColor(1.0f, 1.0f, 1.0f);
}

void WindowElementButton::SetImage(AD2D_Image* img) {
	image = img;
}

void WindowElementButton::SetText(std::string txt) {
	text = txt;
}

void WindowElementButton::SetColor(float r, float g, float b, float a) {
	color.red = r;
	color.green = g;
	color.blue = b;
	color.alpha = a;
}

void WindowElementButton::SetColor(boost::function<COLOR()> func) {
	colorFunc = func;
}

void WindowElementButton::SetPressed(bool pressed) {
	this->pressed = pressed;
}

bool WindowElementButton::GetPressed() {
	return this->pressed;
}

void WindowElementButton::SetInverse(bool inverse) {
	this->inverse = inverse;
}

bool WindowElementButton::GetInverse() {
	return this->inverse;
}

void WindowElementButton::SetCloseIcon(bool state) {
	this->closeIcon = state;
}

bool WindowElementButton::GetCloseIcon() {
	return this->closeIcon;
}

void WindowElementButton::OpenMenu(Mouse& mouse, Keyboard& keyboard, std::list<MenuData>& actionList) {
	WindowElementContainer* parent = this->parent;
	if (parent && parent->parent)
		parent = parent->parent;
	WindowElementTab* tab = dynamic_cast<WindowElementTab*>(parent);

	if (tab) {
		TabElement elem = tab->GetTab(this);
		if (elem.first) {
			actionList.push_back(MenuData(ACTION_CLOSECHANNEL, Text::GetText("ACTION_CLOSECHANNEL", Game::options.language), (void*)elem.first));
			actionList.push_back(MenuData(ACTION_SAVECHANNEL, Text::GetText("ACTION_SAVECHANNEL", Game::options.language), (void*)elem.first));
		}
	}
}

void WindowElementButton::Print(POINT& d, AD2D_Window& gfx) {
	AD2D_Viewport currentVP = gfx.GetCurrentViewport();
	AD2D_Viewport nextVP;

	POINT scroll = GetParentScrollOffset();
	int offsetH = scroll.x;
	int offsetV = scroll.y;

	nextVP.startX = currentVP.startX + posX - offsetH - d.x;
	nextVP.startY = currentVP.startY + posY - offsetV - d.y;
	nextVP.width = width;
	nextVP.height = height;

	POINT diff = AdjustViewport(currentVP, nextVP);

	gfx.SetViewport(nextVP);

	COLOR currentColor = AD2D_Window::GetColor();

	WINDOW_ELEMENTS elem = ((type >= BUTTON_TAB && (wndTemplate->GetElements() & ELEMENT_TAB)) ? ELEMENT_TAB : ELEMENT_BUTTON);
	if (wndTemplate->GetElements() & elem) {
		int x = 0;
		int y = 0;

		POINT diff;
		AD2D_Viewport buttonVP;

		if (!enabled || (pressed && !inverse) || (!pressed && inverse))
			AD2D_Window::SetColor(0.5 * currentColor.red, 0.5 * currentColor.green, 0.5 * currentColor.blue, currentColor.alpha);
		else
			AD2D_Window::SetColor(currentColor.red, currentColor.green, currentColor.blue, currentColor.alpha);

		if (elem == ELEMENT_BUTTON) {
			buttonVP.Create(	currentVP.startX + posX + wndTemplate->tempButton.l_width - offsetH - d.x,
								currentVP.startY + posY + wndTemplate->tempButton.t_height - offsetV - d.y,
								currentVP.startX + posX + width - wndTemplate->tempButton.r_width - offsetH - d.x - 1,
								currentVP.startY + posY + height - wndTemplate->tempButton.b_height - offsetV - d.y - 1);
			diff = AdjustViewport(nextVP, buttonVP);
			gfx.SetViewport(buttonVP);
			if (wndTemplate->tempButton.bg.GetWidth() > 0) {
				for (x = 0; x < width - wndTemplate->tempButton.l_width - wndTemplate->tempButton.r_width; x += wndTemplate->tempButton.bg.GetWidth())
				for (y = 0; y < height - wndTemplate->tempButton.t_height - wndTemplate->tempButton.b_height; y += wndTemplate->tempButton.bg.GetHeight())
					gfx.PutImage(x - diff.x, y - diff.y, wndTemplate->tempButton.bg);
			}

			buttonVP.Create(	currentVP.startX + posX + wndTemplate->tempButton.l_width - offsetH - d.x,
								currentVP.startY + posY - offsetV - d.y,
								currentVP.startX + posX + width - wndTemplate->tempButton.r_width - offsetH - d.x - 1,
								currentVP.startY + posY + wndTemplate->tempButton.t.GetHeight() - offsetV - d.y - 1);
			diff = AdjustViewport(nextVP, buttonVP);
			gfx.SetViewport(buttonVP);
			if (wndTemplate->tempButton.t.GetWidth() > 0) {
				for (x = 0; x < width - wndTemplate->tempButton.l_width - wndTemplate->tempButton.r_width; x += wndTemplate->tempButton.t.GetWidth())
					gfx.PutImage(x - diff.x, - diff.y, wndTemplate->tempButton.t);
			}

			buttonVP.Create(	currentVP.startX + posX - offsetH - d.x,
								currentVP.startY + posY + wndTemplate->tempButton.t_height - offsetV - d.y,
								currentVP.startX + posX + wndTemplate->tempButton.l.GetWidth() - offsetH - d.x - 1,
								currentVP.startY + posY + height - wndTemplate->tempButton.b_height - offsetV - d.y - 1);
			diff = AdjustViewport(nextVP, buttonVP);
			gfx.SetViewport(buttonVP);
			if (wndTemplate->tempButton.l.GetWidth() > 0) {
				for (y = 0; y < height - wndTemplate->tempButton.t_height - wndTemplate->tempButton.b_height; y += wndTemplate->tempButton.l.GetHeight())
					gfx.PutImage(- diff.x, y - diff.y, wndTemplate->tempButton.l);
			}

			buttonVP.Create(	currentVP.startX + posX + width - wndTemplate->tempButton.r.GetWidth() - offsetH - d.x,
								currentVP.startY + posY + wndTemplate->tempButton.t_height - offsetV - d.y,
								currentVP.startX + posX + width - offsetH - d.x - 1,
								currentVP.startY + posY + height - wndTemplate->tempButton.b_height - offsetV - d.y - 1);
			diff = AdjustViewport(nextVP, buttonVP);
			gfx.SetViewport(buttonVP);
			if (wndTemplate->tempButton.r.GetWidth() > 0) {
				for (y = 0; y < height - wndTemplate->tempButton.t_height - wndTemplate->tempButton.b_height; y += wndTemplate->tempButton.r.GetHeight())
					gfx.PutImage(- diff.x, y - diff.y, wndTemplate->tempButton.r);
			}

			buttonVP.Create(	currentVP.startX + posX + wndTemplate->tempButton.l_width - offsetH - d.x,
								currentVP.startY + posY + height - wndTemplate->tempButton.b.GetHeight() - offsetV - d.y,
								currentVP.startX + posX + width - wndTemplate->tempButton.r_width - offsetH - d.x - 1,
								currentVP.startY + posY + height - offsetV - d.y - 1);
			diff = AdjustViewport(nextVP, buttonVP);
			gfx.SetViewport(buttonVP);
			if (wndTemplate->tempButton.b.GetWidth() > 0) {
				for (x = 0; x < width - wndTemplate->tempButton.l_width - wndTemplate->tempButton.r_width; x += wndTemplate->tempButton.b.GetWidth())
					gfx.PutImage(x - diff.x, - diff.y, wndTemplate->tempButton.b);
			}

			buttonVP.Create(	currentVP.startX + posX - offsetH - d.x,
								currentVP.startY + posY - offsetV - d.y,
								currentVP.startX + posX + width - offsetH - d.x - 1,
								currentVP.startY + posY + height - offsetV - d.y - 1);
			diff = AdjustViewport(nextVP, buttonVP);
			gfx.SetViewport(buttonVP);
			gfx.PutImage(- diff.x, - diff.y, wndTemplate->tempButton.tl);
			gfx.PutImage(width - wndTemplate->tempButton.tr.GetWidth() - diff.x, - diff.y, wndTemplate->tempButton.tr);
			gfx.PutImage(- diff.x, height - wndTemplate->tempButton.bl.GetHeight() - diff.y, wndTemplate->tempButton.bl);
			gfx.PutImage(width - wndTemplate->tempButton.br.GetWidth() - diff.x, height - wndTemplate->tempButton.br.GetHeight() - diff.y, wndTemplate->tempButton.br);
		}
		else if (elem == ELEMENT_TAB) {
			buttonVP.Create(	currentVP.startX + posX + wndTemplate->tempTab.l_width - offsetH - d.x,
								currentVP.startY + posY + wndTemplate->tempTab.t_height - offsetV - d.y,
								currentVP.startX + posX + width - wndTemplate->tempTab.r_width - offsetH - d.x - 1,
								currentVP.startY + posY + height - wndTemplate->tempTab.b_height - offsetV - d.y - 1);
			diff = AdjustViewport(nextVP, buttonVP);
			gfx.SetViewport(buttonVP);
			if (wndTemplate->tempTab.bg.GetWidth() > 0) {
				for (x = 0; x < width - wndTemplate->tempTab.l_width - wndTemplate->tempTab.r_width; x += wndTemplate->tempTab.bg.GetWidth())
				for (y = 0; y < height - wndTemplate->tempTab.t_height - wndTemplate->tempTab.b_height; y += wndTemplate->tempTab.bg.GetHeight())
					gfx.PutImage(x - diff.x, y - diff.y, wndTemplate->tempTab.bg);
			}

			buttonVP.Create(	currentVP.startX + posX + wndTemplate->tempTab.l_width - offsetH - d.x,
								currentVP.startY + posY - offsetV - d.y,
								currentVP.startX + posX + width - wndTemplate->tempTab.r_width - offsetH - d.x - 1,
								currentVP.startY + posY + wndTemplate->tempTab.t.GetHeight() - offsetV - d.y - 1);
			diff = AdjustViewport(nextVP, buttonVP);
			gfx.SetViewport(buttonVP);
			if (wndTemplate->tempTab.t.GetWidth() > 0) {
				for (x = 0; x < width - wndTemplate->tempTab.l_width - wndTemplate->tempTab.r_width; x += wndTemplate->tempTab.t.GetWidth())
					gfx.PutImage(x - diff.x, - diff.y, wndTemplate->tempTab.t);
			}

			buttonVP.Create(	currentVP.startX + posX - offsetH - d.x,
								currentVP.startY + posY + wndTemplate->tempTab.t_height - offsetV - d.y,
								currentVP.startX + posX + wndTemplate->tempTab.l.GetWidth() - offsetH - d.x - 1,
								currentVP.startY + posY + height - wndTemplate->tempTab.b_height - offsetV - d.y - 1);
			diff = AdjustViewport(nextVP, buttonVP);
			gfx.SetViewport(buttonVP);
			if (wndTemplate->tempTab.l.GetWidth() > 0) {
				for (y = 0; y < height - wndTemplate->tempTab.t_height - wndTemplate->tempTab.b_height; y += wndTemplate->tempTab.l.GetHeight())
					gfx.PutImage(- diff.x, y - diff.y, wndTemplate->tempTab.l);
			}

			buttonVP.Create(	currentVP.startX + posX + width - wndTemplate->tempTab.r.GetWidth() - offsetH - d.x,
								currentVP.startY + posY + wndTemplate->tempTab.t_height - offsetV - d.y,
								currentVP.startX + posX + width - offsetH - d.x - 1,
								currentVP.startY + posY + height - wndTemplate->tempTab.b_height - offsetV - d.y - 1);
			diff = AdjustViewport(nextVP, buttonVP);
			gfx.SetViewport(buttonVP);
			if (wndTemplate->tempTab.r.GetWidth() > 0) {
				for (y = 0; y < height - wndTemplate->tempTab.t_height - wndTemplate->tempTab.b_height; y += wndTemplate->tempTab.r.GetHeight())
					gfx.PutImage(- diff.x, y - diff.y, wndTemplate->tempTab.r);
			}

			buttonVP.Create(	currentVP.startX + posX + wndTemplate->tempTab.l_width - offsetH - d.x,
								currentVP.startY + posY + height - wndTemplate->tempTab.b.GetHeight() - offsetV - d.y,
								currentVP.startX + posX + width - wndTemplate->tempTab.r_width - offsetH - d.x - 1,
								currentVP.startY + posY + height - offsetV - d.y - 1);
			diff = AdjustViewport(nextVP, buttonVP);
			gfx.SetViewport(buttonVP);
			if (wndTemplate->tempTab.b.GetWidth() > 0) {
				for (x = 0; x < width - wndTemplate->tempTab.l_width - wndTemplate->tempTab.r_width; x += wndTemplate->tempTab.b.GetWidth())
					gfx.PutImage(x - diff.x, - diff.y, wndTemplate->tempTab.b);
			}

			buttonVP.Create(	currentVP.startX + posX - offsetH - d.x,
								currentVP.startY + posY - offsetV - d.y,
								currentVP.startX + posX + width - offsetH - d.x - 1,
								currentVP.startY + posY + height - offsetV - d.y - 1);
			diff = AdjustViewport(nextVP, buttonVP);
			gfx.SetViewport(buttonVP);
			gfx.PutImage(- diff.x, - diff.y, wndTemplate->tempTab.tl);
			gfx.PutImage(width - wndTemplate->tempTab.tr.GetWidth() - diff.x, - diff.y, wndTemplate->tempTab.tr);
			gfx.PutImage(- diff.x, height - wndTemplate->tempTab.bl.GetHeight() - diff.y, wndTemplate->tempTab.bl);
			gfx.PutImage(width - wndTemplate->tempTab.br.GetWidth() - diff.x, height - wndTemplate->tempTab.br.GetHeight() - diff.y, wndTemplate->tempTab.br);
			if (closeIcon)
				gfx.PutImage(width - wndTemplate->tempTab.x.GetWidth() - diff.x, - diff.y, wndTemplate->tempTab.x);
		}
		gfx.SetViewport(nextVP);

		if (colorFunc)
			color = colorFunc();

		if (!enabled || (pressed && !inverse) || (!pressed && inverse))
			AD2D_Window::SetColor(0.5 * color.red * currentColor.red, 0.5 * color.green * currentColor.green, 0.5 * color.blue * currentColor.blue, color.alpha * currentColor.alpha);
		else
			AD2D_Window::SetColor(color.red * currentColor.red, color.green * currentColor.green, color.blue * currentColor.blue, color.alpha * currentColor.alpha);

		if (image) {
			int iwidth = (image->GetWidth() > width ? width : image->GetWidth());
			int iheight = (image->GetHeight() > height ? height : image->GetHeight());
			int posx = (width - iwidth) / 2;
			int posy = (height - iheight) / 2;
			gfx.PutImage(posx - diff.x, posy - diff.y, posx + iwidth - diff.x, posy + iheight - diff.y, *image);
		}

		if (text != "") {
			int posx = (width - wndTemplate->font->GetTextWidth(text, fontSize)) / 2;
			int posy = (height - fontSize) / 2;
			gfx.Print(posx - diff.x, posy - diff.y, fontSize, *wndTemplate->font, text.c_str(), border);
		}

		AD2D_Window::SetColor(currentColor.red, currentColor.green, currentColor.blue, currentColor.alpha);
	}

	gfx.SetViewport(currentVP);
}


// ---- WindowElementTextarea ---- //

WindowElementTextarea::WindowElementTextarea() { }

WindowElementTextarea::~WindowElementTextarea() { }

void WindowElementTextarea::Create(unsigned char al, int pX, int pY, unsigned short wdh, unsigned short hgt, bool ed, bool ml, WindowTemplate* wTemplate) {
	align = al;
	posX = pX;
	posY = pY;
	width = wdh;
	height = hgt;

	wndTemplate = wTemplate;

	maxLength = 0;

	textListIt = 0;

	cur = 0;
	curBegin.x = 0;
	curBegin.y = 0;
	curEnd.x = 0;
	curEnd.y = 0;

	scrollH = 0.0f;
	scrollV = 0.0f;

	offset = 0;

	fontSize = 14;
	border = 0;

	color = 215;

	hidden = false;
	editable = ed;
	multiline = ml;
}

void WindowElementTextarea::SetIntSize(int wdh, int hgt) {
	int borderH = wndTemplate->tempTextarea.l_width + wndTemplate->tempTextarea.r_width;
	int borderV = wndTemplate->tempTextarea.t_height + wndTemplate->tempTextarea.b_height;

	SetSize(wdh + borderH, hgt + borderV);
}

void WindowElementTextarea::SetOffset(int offset) {
	this->offset = offset;
}

void WindowElementTextarea::SetFontSize(int fontSize) {
	this->fontSize = fontSize;

	UpdateText();
	int offset = GetScrollOffset();

	if (scrollV < 0.0f)
		scrollV = 0.0f;
	else if (scrollV > (float)offset)
		scrollV = (float)offset;
}

void WindowElementTextarea::UpdateText() {
	int borderH = wndTemplate->tempTextarea.l_width + wndTemplate->tempTextarea.r_width + (multiline ? wndTemplate->tempScroll.w : 0);
	int length = (width - borderH);

	_text = (multiline ? ScrollText(this->text, wndTemplate->font, fontSize, length, offset) : this->text);

	lines = DivideText(_text);
}

void WindowElementTextarea::SetText(std::string txt) {
	text = TextString(txt, color);
	cur = text.text.length();

	UpdateText();

	unsigned int offset = GetScrollOffset();
	if (scrollV > offset) scrollV = offset;
}

void WindowElementTextarea::AddTextLine(TextString txt) {
	unsigned int offset = GetScrollOffset();

	if (!text.text.empty()) {
		text.text.push_back('\n');
		text.color.push_back(0);
	}

	text.text += txt.text;
	text.color += txt.color;

	int borderH = wndTemplate->tempTextarea.l_width + wndTemplate->tempTextarea.r_width + (multiline ? wndTemplate->tempScroll.w : 0);
	int length = (width - borderH);

	TextString tempText = ScrollText(txt, wndTemplate->font, fontSize, length, this->offset);
	std::vector<std::pair<unsigned int, TextString> > tempLines = DivideText(tempText);

	if (!_text.text.empty()) {
		_text.text.push_back('\n');
		_text.color.push_back(0);
	}
	else
		lines.clear();

	_text.text += tempText.text;
	_text.color += tempText.color;

	unsigned int line = (!lines.empty() ? lines[lines.size() - 1].first + 1 : 0);
	for (int i = 0; i < tempLines.size(); i++)
		lines.push_back(std::pair<unsigned int, TextString>(tempLines[i].first + line, tempLines[i].second));
	//UpdateText();

	unsigned int newOffset = GetScrollOffset();
	if (scrollV == offset) scrollV = newOffset;
}

unsigned int WindowElementTextarea::GetAllLinesNumber() {
	return lines.size();
}

void WindowElementTextarea::SetMaxLength(unsigned int length) {
	this->maxLength = length;
}

bool WindowElementTextarea::IsEditable() {
	return editable;
}

bool WindowElementTextarea::IsSelection() {
	return (curBegin < curEnd);
}

std::string WindowElementTextarea::GetText() {
	return text.text;
}

std::string* WindowElementTextarea::GetTextPtr() {
	return &text.text;
}

void WindowElementTextarea::SetHidden(bool state) {
	hidden = state;
}

void WindowElementTextarea::SetActive(bool state) {
	active = state;
}

void WindowElementTextarea::SetColor(unsigned char color) {
	this->color = color;
}

void WindowElementTextarea::SetSize(unsigned short wdh, unsigned short hgt) {
	bool endOfArea = false;
	unsigned int offset = GetScrollOffset();
	if (scrollV == (float)offset) endOfArea = true;

	width = wdh;
	height = hgt;

	UpdateText();

	offset = GetScrollOffset();
	if (scrollV > (float)offset)
		scrollV = (float)offset;
	else if (endOfArea)
		scrollV = offset;
}

void WindowElementTextarea::GetCursor(POINT& cursor, float& pix, std::string* txt) {
	std::string text = (txt ? *txt : this->_text.text);

	cursor.x = 0;
	cursor.y = 0;
	int ncur = 0;
	int tcur = 0;
	int lines = 0;
	pix = 0.0f;
	while(tcur < text.length() && ncur < this->cur) {
		unsigned char c = text[tcur];
		if (hidden) c = '*';

		if (c == '\n' || c == 12 || c == 13) {
			if (c == '\n')
				lines = 0;
			else
				lines++;

			cursor.y++;
			cursor.x = 0;
			pix = (lines ? offset : 0);
			if (c != 13) ncur++;
		}
		else {
			cursor.x++;
			pix += wndTemplate->font->GetCharWidth(c, fontSize);
			ncur++;
		}

		tcur++;
	}
}

void WindowElementTextarea::SetCursor(POINT cursor, float toPix, std::string* txt) {
	std::string text = (txt ? *txt : this->_text.text);

	POINT tcursor = {0, 0};
	int ncur = 0;
	int tcur = 0;
	int lines = 0;
	float pix = 0.0f;
	while(tcur < text.length()) {
		unsigned char c = text[tcur];
		if (hidden) c = '*';

		if (c == '\n' || c == 12 || c == 13) {
			if (cursor.y == tcursor.y) {
				this->cur = ncur;
				return;
			}

			if (c == '\n')
				lines = 0;
			else
				lines++;

			tcursor.y++;
			tcursor.x = 0;
			pix = (lines ? offset : 0);
			if (c != 13) ncur++;

			if (cursor.y == tcursor.y && (
				(toPix == 0.0f && cursor.x < tcursor.x) ||
				(toPix != 0.0f && pix + wndTemplate->font->GetCharWidth(' ', fontSize) >= toPix)))
			{
				this->cur = ncur;
				return;
			}
		}
		else {
			if (cursor.y == tcursor.y && (
				(toPix == 0.0f && cursor.x == tcursor.x) ||
				(toPix != 0.0f && pix + wndTemplate->font->GetCharWidth(c, fontSize) >= toPix && pix < toPix)))
			{
				this->cur = ncur;
				return;
			}
			tcursor.x++;
			pix += wndTemplate->font->GetCharWidth(c, fontSize);
			ncur++;
		}

		tcur++;
	}
	this->cur = this->text.text.length();
}

unsigned int WindowElementTextarea::GetLinesNumber() {
	if (hidden)
		return 1;

	size_t lastPos = 0;
	size_t pos = text.text.find('\n', lastPos);

	unsigned int lines = 1;
	while(lastPos < text.text.length() && pos != std::string::npos) {
		lastPos = pos + 1;
		pos = text.text.find('\n', lastPos);
		lines++;
	}

	return lines;
}

unsigned int WindowElementTextarea::GetLineNumber() {
	if (hidden)
		return 1;

	size_t lastPos = 0;
	size_t pos = 0;

	unsigned int lines = 1;
	while((pos = text.text.find('\n', lastPos)) != std::string::npos && pos < cur) {
		lastPos = pos + 1;
		lines++;
	}

	return lines;
}

int WindowElementTextarea::GetScrollOffset() {
	int borderV = wndTemplate->tempTextarea.t_height + wndTemplate->tempTextarea.b_height;

	std::string text = this->_text.text;

	int lines = 0;
	int tcur = 0;
	while(tcur < text.length()) {
		char c = text[tcur];
		if (hidden) c = '*';

		if (c == '\n' || c == 12 || c == 13)
			lines++;
		tcur++;
	}

	int offset = (lines + 1) * fontSize - (height - borderV);
	if (offset < 0) offset = 0;

	return offset;
}

std::string WindowElementTextarea::SelectWordAtCursor(float pix) {
	int cBeg = cur;
	int cEnd = cur;
	POINT pos;

	if (text.text[cur] == ' ')
		return "";

	char ch;
	while(cBeg > 0 && (cBeg < 1 || !IsSpecialChar(text.text[cBeg - 1])))
		cBeg--;
	cur = cBeg;
	GetCursor(pos, pix);
	curBegin = pos;

	while(cEnd < text.text.length() && !IsSpecialChar(text.text[cEnd]))
		cEnd++;
	cur = cEnd;
	GetCursor(pos, pix);
	curEnd = pos;

	return text.text.substr(cBeg, cEnd - cBeg);
}

void WindowElementTextarea::CheckInput(Keyboard& keyboard) {
	if (this->editable && this->enabled) {
		bool clipboardOperation = false;
		if (keyboard.keyChar == 8) {
			if (this->curBegin != this->curEnd) {
				this->SetCursor(this->curBegin);
				unsigned int begin = this->cur;
				this->SetCursor(this->curEnd);
				unsigned int end = this->cur;
				this->text.text.erase(begin, end - begin);
				this->text.color.erase(begin, end - begin);

				this->SetCursor(this->curBegin);
			}
			else if (this->cur > 0) {
				this->text.text.erase(this->cur - 1, 1);
				this->text.color.erase(this->cur - 1, 1);
				this->cur--;
			}
		}
		else if (keyboard.keyChar == 13 && !this->multiline) {
			this->textList.insert(this->textList.begin(), this->text.text);
			if (this->textList.size() > 50)
				this->textList.erase(this->textList.begin() + this->textList.size() - 1);
			this->textListIt = 0;

			if (this->func.IsExecutable())
				this->func.Execute();

			keyboard.keyChar = 0;
		}
		else if (keyboard.keyChar == 27) {
			this->text.text.clear();
			this->text.color.clear();
			this->cur = 0;
		}
		else if (keyboard.keyChar == 255 + VK_DELETE) {
			if (this->curBegin != this->curEnd) {
				this->SetCursor(this->curBegin);
				unsigned int begin = this->cur;
				this->SetCursor(this->curEnd);
				unsigned int end = this->cur;
				this->text.text.erase(begin, end - begin);
				this->text.color.erase(begin, end - begin);

				this->SetCursor(this->curBegin);
			}
			else if (this->cur < this->text.text.length()) {
				this->text.text.erase(this->cur, 1);
				this->text.color.erase(this->cur, 1);
			}
		}
		else if (keyboard.keyChar == 255 + VK_UP && keyboard.key[VK_SHIFT] && this->multiline) {
			POINT cursor;
			float pix;
			this->GetCursor(cursor, pix);
			if (cursor.y > 0) cursor.y--;
			else cursor.x = 0;
			this->SetCursor(cursor, pix);
		}
		else if (keyboard.keyChar == 255 + VK_UP && keyboard.key[VK_SHIFT] && !this->multiline) {
			std::string text = "";
			if (this->textListIt < textList.size() + 1) {
				if (this->textListIt < textList.size())
					this->textListIt++;
				if (this->textListIt > 0)
					text = this->textList[this->textListIt - 1];
			}
			this->text = TextString(text, 215);
			this->cur = this->text.text.length();
		}
		else if (keyboard.keyChar == 255 + VK_DOWN && keyboard.key[VK_SHIFT] && this->multiline) {
			POINT cursor;
			float pix;
			this->GetCursor(cursor, pix);
			cursor.y++;
			this->SetCursor(cursor, pix);
			//this->GetCursor(cursor);
		}
		else if (keyboard.keyChar == 255 + VK_DOWN && keyboard.key[VK_SHIFT] && !this->multiline) {
			std::string text = "";
			if (this->textListIt > 0) {
				this->textListIt--;
				if (this->textListIt > 0)
					text = this->textList[this->textListIt - 1];
			}
			this->text = TextString(text, 215);
			this->cur = this->text.text.length();
		}
		else if (keyboard.keyChar == 255 + VK_LEFT) {
			if (this->cur > 0) this->cur--;
		}
		else if (keyboard.keyChar == 255 + VK_RIGHT) {
			this->cur++;
			if (this->cur > this->text.text.length()) this->cur = this->text.text.length();
		}
		else if (keyboard.keyChar == 255 + VK_HOME) {
			this->cur = 0;
		}
		else if (keyboard.keyChar == 255 + VK_END) {
			this->cur = this->text.text.length();
		}
		else if (keyboard.keyChar == 255 + VK_PRIOR) {
		}
		else if (keyboard.keyChar == 255 + VK_NEXT) {
		}
		else if (keyboard.keyChar == 1) { // CTRL + A
			POINT cursor;
			float pix;
			this->curBegin = doPOINT(0, 0);
			this->cur = this->text.text.length();
			this->GetCursor(cursor, pix);
			this->curEnd = cursor;

			clipboardOperation = true;
		}
		else if (keyboard.keyChar == 24) { // CTRL + X
			if (this->curBegin != this->curEnd) {
				this->SetCursor(this->curBegin);
				unsigned int begin = this->cur;
				this->SetCursor(this->curEnd);
				unsigned int end = this->cur;

				std::string clipboard = this->text.text.substr(begin, end - begin);
				Game::SetClipboard(clipboard);

				this->text.text.erase(begin, end - begin);
				this->text.color.erase(begin, end - begin);

				this->SetCursor(this->curBegin);
			}
		}
		else if (keyboard.keyChar == 3) { //CTRL + C
			if (this->curBegin != this->curEnd) {
				unsigned int cur = this->cur;

				this->SetCursor(this->curBegin);
				unsigned int begin = this->cur;
				this->SetCursor(this->curEnd);
				unsigned int end = this->cur;

				this->cur = cur;

				std::string clipboard = this->text.text.substr(begin, end - begin);
				Game::SetClipboard(clipboard);
			}

			clipboardOperation = true;
		}
		else if (keyboard.keyChar == 22) { // CTRL + V
			if (this->curBegin != this->curEnd) {
				this->SetCursor(this->curBegin);
				unsigned int begin = this->cur;
				this->SetCursor(this->curEnd);
				unsigned int end = this->cur;
				this->text.text.erase(begin, end - begin);
				this->text.color.erase(begin, end - begin);

				this->SetCursor(this->curBegin);
			}

			std::string clipboard = Game::GetClipboard();
			this->text.text.insert(this->cur, clipboard);
			this->text.color.insert(this->cur, clipboard.length(), this->color);
			this->cur += clipboard.length();
		}
		else if (keyboard.keyChar <= 255 && (this->maxLength == 0 || this->text.text.length() < this->maxLength)) {
			if (this->curBegin != this->curEnd) {
				this->SetCursor(this->curBegin);
				unsigned int begin = this->cur;
				this->SetCursor(this->curEnd);
				unsigned int end = this->cur;
				this->text.text.erase(begin, end - begin);
				this->text.color.erase(begin, end - begin);

				this->SetCursor(this->curBegin);
				this->curEnd = this->curBegin;
			}

			if (keyboard.keyChar == 13) keyboard.keyChar = '\n';
			this->text.text.insert(this->cur, 1, keyboard.keyChar);
			this->text.color.insert(this->cur, 1, this->color);
			this->cur++;
		}
		this->UpdateText();

		POINT cursor;
		float pix;
		this->GetCursor(cursor, pix);
		if (!keyboard.key[VK_SHIFT] && !clipboardOperation) {
			this->curBegin = cursor;
			this->curEnd = cursor;
		}
		if (this->multiline) {
			if ((cursor.y + 1) * this->fontSize - (int)this->scrollV > this->height - this->wndTemplate->tempTextarea.t_height - this->wndTemplate->tempTextarea.b_height)
				this->scrollV = (cursor.y + 1) * this->fontSize - (this->height - this->wndTemplate->tempTextarea.t_height - this->wndTemplate->tempTextarea.b_height);
			else if (cursor.y * this->fontSize - (int)this->scrollV < 0)
				this->scrollV = cursor.y * this->fontSize;
		}
		else {
			if ((pix + 4) - (int)this->scrollH > this->width - this->wndTemplate->tempTextarea.l_width - this->wndTemplate->tempTextarea.r_width)
				this->scrollH = (pix + 4) - (this->width - this->wndTemplate->tempTextarea.l_width - this->wndTemplate->tempTextarea.r_width);
			else if (pix - (int)this->scrollH < 0)
				this->scrollH = pix;
		}
	}
	else {
		if (keyboard.keyChar == 1) { // CTRL + A
			POINT cursor;
			float pix;
			this->curBegin = doPOINT(0, 0);
			this->cur = this->text.text.length();
			this->GetCursor(cursor, pix);
			this->curEnd = cursor;

			keyboard.keyChar = 0;
		}
		else if (keyboard.keyChar == 3) { //CTRL + C
			if (this->curBegin != this->curEnd) {
				this->SetCursor(this->curBegin);
				unsigned int begin = this->cur;
				this->SetCursor(this->curEnd);
				unsigned int end = this->cur;

				std::string clipboard = this->text.text.substr(begin, end - begin);
				Game::SetClipboard(clipboard);
			}

			keyboard.keyChar = 0;
		}
	}
}

void WindowElementTextarea::OpenMenu(Mouse& mouse, Keyboard& keyboard, std::list<MenuData>& actionList) {
	POINT absPos = GetAbsolutePosition(true);

	SetCursor(this->curBegin);
	unsigned int begin = cur;
	SetCursor(this->curEnd);
	unsigned int end = cur;

	std::string selected = text.text.substr(begin, end - begin);

	POINT pos;
	pos.x = (mouse.curX + (int)scrollH - (absPos.x + wndTemplate->tempTextarea.l_width)) / fontSize;
	pos.y = (mouse.curY + (int)scrollV - (absPos.y + wndTemplate->tempTextarea.t_height)) / fontSize;
	float pix = mouse.curX + (int)scrollH - (absPos.x + wndTemplate->tempTextarea.l_width);

	if (pos.x >= 0 && pos.y >= 0) {
		SetCursor(pos, pix);
		GetCursor(pos, pix);
		if (pos < curBegin || pos > curEnd) {
			curBegin = pos;
			curEnd = pos;
		}
	}

	if (editable)
		actionList.push_back(MenuData(ACTION_PASTE, Text::GetText("ACTION_PASTE", Game::options.language), (void*)this));

	if (curBegin != curEnd) {
		actionList.push_back(MenuData(ACTION_COPY, Text::GetText("ACTION_COPY", Game::options.language), (void*)new(M_PLACE) std::string(selected) ));
		if (editable)
			actionList.push_back(MenuData(ACTION_CUT, Text::GetText("ACTION_CUT", Game::options.language), (void*)this));
	}

	Channel* ch = Channel::GetChannel((void*)this);
	if (ch) {
		unsigned int line = GetLineNumber();
		ChatMessageType mType = ch->GetMessage(line);

		std::string message = mType.second.second.text;
		std::string creatureName = mType.second.first;

		if (actionList.size() > 0)
			actionList.push_back(MenuData(0x00, "-", NULL));

		actionList.push_back(MenuData(ACTION_COPYMESSAGE, Text::GetText("ACTION_COPYMESSAGE", Game::options.language), (void*)new(M_PLACE) std::string(message) ));
		if (creatureName != "") {
			actionList.push_back(MenuData(0x00, "-", NULL));
			actionList.push_back(MenuData(ACTION_SENDMESSAGE, Text::GetText("ACTION_SENDMESSAGE", Game::options.language), (void*)new(M_PLACE) std::string(creatureName) ));
			if (!Creature::IsIgnored(creatureName))
				actionList.push_back(MenuData(ACTION_IGNORE, Text::GetText("ACTION_IGNORE", Game::options.language), (void*)new(M_PLACE) std::string(creatureName) ));
			else
				actionList.push_back(MenuData(ACTION_UNIGNORE, Text::GetText("ACTION_UNIGNORE", Game::options.language), (void*)new(M_PLACE) std::string(creatureName) ));
			actionList.push_back(MenuData(ACTION_ADDTOVIP, Text::GetText("ACTION_ADDTOVIP", Game::options.language), (void*)new(M_PLACE) std::string(creatureName) ));
			actionList.push_back(MenuData(0x00, "-", NULL));
			actionList.push_back(MenuData(ACTION_COPYNAME, Text::GetText("ACTION_COPYNAME", Game::options.language), (void*)new(M_PLACE) std::string(creatureName) ));
		}
		actionList.push_back(MenuData(0x00, "-", NULL));
		actionList.push_back(MenuData(ACTION_CLEARCHANNEL, Text::GetText("ACTION_CLEARCHANNEL", Game::options.language), (void*)ch));
	}
}

void WindowElementTextarea::Print(POINT& d, AD2D_Window& gfx) {
	AD2D_Viewport currentVP = gfx.GetCurrentViewport();
	AD2D_Viewport nextVP;

	POINT scroll = GetParentScrollOffset();
	int offsetH = scroll.x;
	int offsetV = scroll.y;

	nextVP.startX = currentVP.startX + posX - offsetH - d.x;
	nextVP.startY = currentVP.startY + posY - offsetV - d.y;
	nextVP.width = width;
	nextVP.height = height;

	POINT diff = AdjustViewport(currentVP, nextVP);

	gfx.SetViewport(nextVP);

	COLOR currentColor = AD2D_Window::GetColor();

	AD2D_Viewport intNextVP;
	POINT intDiff = diff;
	if (wndTemplate->GetElements() & ELEMENT_TEXTAREA) {
		int x = 0;
		int y = 0;

		if (!enabled)
			AD2D_Window::SetColor(0.6f * currentColor.red, 0.6f * currentColor.green, 0.6f * currentColor.blue, currentColor.alpha);

		POINT diff;
		AD2D_Viewport textareaVP;

		textareaVP.Create(	currentVP.startX + posX + wndTemplate->tempTextarea.l_width - offsetH - d.x,
							currentVP.startY + posY + wndTemplate->tempTextarea.t_height - offsetV - d.y,
							currentVP.startX + posX + width - wndTemplate->tempTextarea.r_width - offsetH - d.x - 1,
							currentVP.startY + posY + height - wndTemplate->tempTextarea.b_height - offsetV - d.y - 1);
		diff = AdjustViewport(nextVP, textareaVP);
		gfx.SetViewport(textareaVP);
		if (wndTemplate->tempTextarea.bg.GetWidth() > 0) {
			for (x = 0; x < width - wndTemplate->tempTextarea.l_width - wndTemplate->tempTextarea.r_width; x += wndTemplate->tempTextarea.bg.GetWidth())
			for (y = 0; y < height - wndTemplate->tempTextarea.t_height - wndTemplate->tempTextarea.b_height; y += wndTemplate->tempTextarea.bg.GetHeight())
				gfx.PutImage(x - diff.x, y - diff.y, wndTemplate->tempTextarea.bg);
		}

		textareaVP.Create(	currentVP.startX + posX + wndTemplate->tempTextarea.l_width - offsetH - d.x,
							currentVP.startY + posY - offsetV - d.y,
							currentVP.startX + posX + width - wndTemplate->tempTextarea.r_width - offsetH - d.x - 1,
							currentVP.startY + posY + wndTemplate->tempTextarea.t.GetHeight() - offsetV - d.y - 1);
		diff = AdjustViewport(nextVP, textareaVP);
		gfx.SetViewport(textareaVP);
		if (wndTemplate->tempTextarea.t.GetWidth() > 0) {
			for (x = 0; x < width - wndTemplate->tempTextarea.l_width - wndTemplate->tempTextarea.r_width; x += wndTemplate->tempTextarea.t.GetWidth())
				gfx.PutImage(x - diff.x, - diff.y, wndTemplate->tempTextarea.t);
		}

		textareaVP.Create(	currentVP.startX + posX - offsetH - d.x,
							currentVP.startY + posY + wndTemplate->tempTextarea.t_height - offsetV - d.y,
							currentVP.startX + posX + wndTemplate->tempTextarea.l.GetWidth() - offsetH - d.x - 1,
							currentVP.startY + posY + height - wndTemplate->tempTextarea.b_height - offsetV - d.y - 1);
		diff = AdjustViewport(nextVP, textareaVP);
		gfx.SetViewport(textareaVP);
		if (wndTemplate->tempTextarea.l.GetWidth() > 0) {
			for (y = 0; y < height - wndTemplate->tempTextarea.t_height - wndTemplate->tempTextarea.b_height; y += wndTemplate->tempTextarea.l.GetHeight())
				gfx.PutImage(- diff.x, y - diff.y, wndTemplate->tempTextarea.l);
		}

		textareaVP.Create(	currentVP.startX + posX + width - wndTemplate->tempTextarea.r.GetWidth() - offsetH - d.x,
							currentVP.startY + posY + wndTemplate->tempTextarea.t_height - offsetV - d.y,
							currentVP.startX + posX + width - offsetH - d.x - 1,
							currentVP.startY + posY + height - wndTemplate->tempTextarea.b_height - offsetV - d.y - 1);
		diff = AdjustViewport(nextVP, textareaVP);
		gfx.SetViewport(textareaVP);
		if (wndTemplate->tempTextarea.r.GetWidth() > 0) {
			for (y = 0; y < height - wndTemplate->tempTextarea.t_height - wndTemplate->tempTextarea.b_height; y += wndTemplate->tempTextarea.r.GetHeight())
				gfx.PutImage(- diff.x, y - diff.y, wndTemplate->tempTextarea.r);
		}

		textareaVP.Create(	currentVP.startX + posX + wndTemplate->tempTextarea.l_width - offsetH - d.x,
							currentVP.startY + posY + height - wndTemplate->tempTextarea.b.GetHeight() - offsetV - d.y,
							currentVP.startX + posX + width - wndTemplate->tempTextarea.r_width - offsetH - d.x - 1,
							currentVP.startY + posY + height - offsetV - d.y - 1);
		diff = AdjustViewport(nextVP, textareaVP);
		gfx.SetViewport(textareaVP);
		if (wndTemplate->tempTextarea.b.GetWidth() > 0) {
			for (x = 0; x < width - wndTemplate->tempTextarea.l_width - wndTemplate->tempTextarea.r_width; x += wndTemplate->tempTextarea.b.GetWidth())
				gfx.PutImage(x - diff.x, - diff.y, wndTemplate->tempTextarea.b);
		}

		textareaVP.Create(	currentVP.startX + posX - offsetH - d.x,
							currentVP.startY + posY - offsetV - d.y,
							currentVP.startX + posX + width - offsetH - d.x - 1,
							currentVP.startY + posY + height - offsetV - d.y - 1);
		diff = AdjustViewport(nextVP, textareaVP);
		gfx.SetViewport(textareaVP);
		gfx.PutImage(- diff.x, - diff.y, wndTemplate->tempTextarea.tl);
		gfx.PutImage(width - wndTemplate->tempTextarea.tr.GetWidth() - diff.x, - diff.y, wndTemplate->tempTextarea.tr);
		gfx.PutImage(- diff.x, height - wndTemplate->tempTextarea.bl.GetHeight() - diff.y, wndTemplate->tempTextarea.bl);
		gfx.PutImage(width - wndTemplate->tempTextarea.br.GetWidth() - diff.x, height - wndTemplate->tempTextarea.br.GetHeight() - diff.y, wndTemplate->tempTextarea.br);

		intNextVP.startX = currentVP.startX + posX + wndTemplate->tempTextarea.l_width - offsetH - d.x;
		intNextVP.startY = currentVP.startY + posY + wndTemplate->tempTextarea.t_height - offsetV - d.y;
		intNextVP.width = width - wndTemplate->tempTextarea.l_width - wndTemplate->tempTextarea.r_width;
		intNextVP.height = height - wndTemplate->tempTextarea.t_height - wndTemplate->tempTextarea.b_height;
		intDiff = AdjustViewport(nextVP, intNextVP);
		gfx.SetViewport(intNextVP);
	}
	else
		gfx.SetViewport(nextVP);

	std::string text = this->_text.text;

	if (curBegin < curEnd) {
		POINT tcursor = {0, 0};
		int tcur = 0;
		int lines = 0;
		float pix = 0.0f;
		while(tcur < text.length()) {
			unsigned char c = text[tcur];
			if (hidden) c = '*';

			if (tcursor >= curBegin && tcursor < curEnd) {
				float charWidth = wndTemplate->font->GetCharWidth(c, fontSize);
				AD2D_Window::SetColor(0.5f * currentColor.red, 0.5f * currentColor.green, 0.5f * currentColor.blue, currentColor.alpha);
				gfx.PutRect(- intDiff.x + pix - this->scrollH, - intDiff.y + (tcursor.y + 0) * fontSize - this->scrollV,
							- intDiff.x + pix + charWidth - this->scrollH, - intDiff.y + (tcursor.y + 1) * fontSize - this->scrollV);
				AD2D_Window::SetColor(currentColor.red, currentColor.green, currentColor.blue, currentColor.alpha);
			}

			if (c == '\n' || c == 12 || c == 13) {
				if (c == '\n')
					lines = 0;
				else
					lines++;
				tcursor.y++;
				tcursor.x = 0;
				pix = (lines ? (float)offset : 0.0f);
			}
			else {
				tcursor.x++;
				pix += wndTemplate->font->GetCharWidth(c, fontSize);
			}

			tcur++;
		}
	}

	int lineBegin;
	int lineEnd;
	lineBegin = (int)this->scrollV / fontSize;
	lineEnd = ((int)this->scrollV + height - wndTemplate->tempTextarea.t_height - wndTemplate->tempTextarea.b_height) / fontSize + 1;

	for (int i = lineBegin; i < lineEnd && i < lines.size(); i++) {
		int _offset = (i <= 0 || lines[i-1].first != lines[i].first ? 0 : offset);
		unsigned int line = lines[i].first + 1;

		std::string printText = lines[i].second.text;
		std::string printColor = lines[i].second.color;
		if (hidden) printText.replace(0, printText.size(), printText.size(), '*');

		if (enabled)
			AD2D_Window::SetColor(currentColor.red, currentColor.green, currentColor.blue, currentColor.alpha);
		else
			AD2D_Window::SetColor(0.6f * currentColor.red, 0.6f * currentColor.green, 0.6f * currentColor.blue, currentColor.alpha);
		gfx.CPrint(- intDiff.x + _offset - (int)this->scrollH, - intDiff.y + i * fontSize - (int)this->scrollV, fontSize, *wndTemplate->font, printText.c_str(), printColor.c_str(), border);
		AD2D_Window::SetColor(currentColor.red, currentColor.green, currentColor.blue, currentColor.alpha);
	}

	if (active && editable && enabled) {
		POINT cursor;
		float pix;
		GetCursor(cursor, pix, &text);

		if ((RealTime::getTime() / 200) % 2 == 0)
			gfx.Print(- intDiff.x + (int)pix - (int)this->scrollH, - intDiff.y + cursor.y * fontSize - (int)this->scrollV, fontSize, *wndTemplate->font, "|", border);
	}

	gfx.SetViewport(nextVP);

	if (multiline && wndTemplate->GetElements() & ELEMENT_SCROLL) {
		int offset = GetScrollOffset();

		int y = 0;

		POINT diff;
		AD2D_Viewport scrollVP;
		scrollVP.Create(currentVP.startX + posX + width - wndTemplate->tempTextarea.r_width - wndTemplate->tempScroll.w - offsetH - d.x,
						currentVP.startY + posY + wndTemplate->tempTextarea.t_height - offsetV - d.y,
						currentVP.startX + posX + width - wndTemplate->tempTextarea.r_width - offsetH - d.x - 1,
						currentVP.startY + posY + height - wndTemplate->tempTextarea.b_height - offsetV - d.y - 1);
		diff = AdjustViewport(nextVP, scrollVP);
		gfx.SetViewport(scrollVP);

		for (y = 0; y < height - wndTemplate->tempTextarea.t_height - wndTemplate->tempTextarea.b_height; y += wndTemplate->tempScroll.bgv.GetHeight())
			gfx.PutImage(- diff.x, y - diff.y, wndTemplate->tempScroll.bgv);
		gfx.PutImage(- diff.x, - diff.y, wndTemplate->tempScroll.t);
		gfx.PutImage(- diff.x, height - wndTemplate->tempTextarea.t_height - wndTemplate->tempTextarea.b_height - wndTemplate->tempScroll.b.GetHeight() - diff.y, wndTemplate->tempScroll.b);

		if (offset) {
			float position = this->scrollV / offset;
			gfx.PutImage(- diff.x, wndTemplate->tempScroll.t.GetHeight() + (int)((height - wndTemplate->tempTextarea.t_height - wndTemplate->tempTextarea.b_height - wndTemplate->tempScroll.t.GetHeight() - wndTemplate->tempScroll.b.GetHeight() - wndTemplate->tempScroll.brv.GetHeight()) * position) - diff.y, wndTemplate->tempScroll.brv);
		}
	}

	gfx.SetViewport(currentVP);
}


// ---- WindowElementMemo ---- //

WindowElementMemo::WindowElementMemo() { }

WindowElementMemo::~WindowElementMemo() { }

void WindowElementMemo::Create(unsigned char al, int pX, int pY, unsigned short wdh, unsigned short hgt, WindowTemplate* wTemplate) {
	align = al;
	posX = pX;
	posY = pY;
	width = wdh;
	height = hgt;

	wndTemplate = wTemplate;

	scrollV = 0.0f;

	option = 0;

	fontSize = 14;

	color = COLOR(1.0f, 1.0f, 1.0f, 1.0f);

	dblFunc.Clear();
}

void WindowElementMemo::SetColor(float r, float g, float b, float a) {
	color.red = r;
	color.green = g;
	color.blue = b;
	color.alpha = a;
}

void WindowElementMemo::SetOption(unsigned short option, bool execute) {
	if (option >= elements.size())
		return;
	this->option = option;

	int borderV = wndTemplate->tempMemo.t_height + wndTemplate->tempMemo.b_height;

	if (scrollV > option * fontSize)
		scrollV = option * fontSize;
	else if (scrollV < (option + 1) * fontSize - (height - borderV))
		scrollV = (option + 1) * fontSize - (height - borderV);

	if (execute && func.IsExecutable())
		func.Execute();
}

void WindowElementMemo::SetOption(std::string option, bool execute) {
	std::list<std::pair<TextString, bool> >::iterator it = elements.begin();
	int opt = 0;
	for (it; it != elements.end(); it++) {
		if (it->first.text == option) {
			this->option = opt;
			break;
		}
		opt++;
	}

	int borderV = wndTemplate->tempMemo.t_height + wndTemplate->tempMemo.b_height;

	if (scrollV > this->option * fontSize)
		scrollV = this->option * fontSize;
	else if (scrollV < (this->option + 1) * fontSize - (height - borderV))
		scrollV = (this->option + 1) * fontSize - (height - borderV);

	if (execute && func.IsExecutable())
		func.Execute();
}

unsigned short WindowElementMemo::GetOption() {
	return this->option;
}

unsigned short WindowElementMemo::GetOptionsSize() {
	return elements.size();
}

void WindowElementMemo::AddElement(TextString text, bool active, bool notNull) {
	if (!notNull || text.text != "")
		elements.push_back(std::pair<TextString, bool>(text, active));
}

void WindowElementMemo::AddElementPtr(std::string* text, std::string* color, bool active, bool notNull) {
	TextString textStr;
	if (text && color)
		textStr = TextString(*text, *color);
	else if (text)
		textStr = TextString(*text);

	AddElement(textStr, active, notNull);
}

TextString WindowElementMemo::GetElement() {
	std::list<std::pair<TextString, bool> >::iterator it = elements.begin();
	unsigned short opt = 0;
	for (it, opt; it != elements.end() && opt < option; it++, opt++);

	return (it != elements.end() ? it->first : TextString(""));
}

std::list<TextString> WindowElementMemo::GetElements() {
	std::list<TextString> ret;

	std::list<std::pair<TextString, bool> >::iterator it = elements.begin();
	for (it; it != elements.end(); it++)
		if (it->second)
			ret.push_back(it->first);

	return ret;
}

void WindowElementMemo::RemoveElement() {
	std::list<std::pair<TextString, bool> >::iterator it = elements.begin();
	unsigned short opt = 0;
	for (it, opt; it != elements.end() && opt < option; it++, opt++);

	if (it != elements.end())
		elements.erase(it);
}

void WindowElementMemo::MoveUpElement() {
	std::list<std::pair<TextString, bool> >::iterator it = elements.begin();
	std::list<std::pair<TextString, bool> >::iterator last_it = it;
	unsigned short opt = 0;
	for (it, opt; it != elements.end() && opt < option; it++, opt++)
		last_it = it;

	if (it != elements.end() && it != elements.begin()) {
		std::pair<TextString, bool> temp = *it;
		*it = *last_it;
		*last_it = temp;
		SetOption(option - 1, false);
	}
}

void WindowElementMemo::MoveDownElement() {
	std::list<std::pair<TextString, bool> >::iterator it = elements.begin();
	std::list<std::pair<TextString, bool> >::iterator last_it = it;
	unsigned short opt = 0;
	for (it, opt; it != elements.end() && opt < option + 1; it++, opt++)
		last_it = it;

	if (it != elements.end() && it != elements.begin()) {
		std::pair<TextString, bool> temp = *it;
		*it = *last_it;
		*last_it = temp;
		SetOption(option + 1, false);
	}
}

void WindowElementMemo::Clear() {
	option = 0;
	elements.clear();
}


void WindowElementMemo::SetDblAction(boost::function<void()> func) {
	if (!func) {
		dblFunc.Clear();
		return;
	}

    dblFunc.PushFunction(func);
}


void WindowElementMemo::SetSize(unsigned short wdh, unsigned short hgt) {
	width = wdh;
	height = hgt;

	unsigned int offset = GetScrollOffset();
	if (scrollV > (float)offset) scrollV = (float)offset;
}


int WindowElementMemo::GetScrollOffset() {
	int borderV = wndTemplate->tempMemo.t_height + wndTemplate->tempMemo.b_height;

	int intHeight = elements.size() * fontSize;

	int offset = intHeight - (height - borderV);
	if (offset < 0) offset = 0;

	return offset;
}

void WindowElementMemo::CheckInput(Keyboard& keyboard) {
	if (keyboard.keyChar == 255 + VK_UP) {
		unsigned short opt = this->GetOption();
		if (opt > 0) opt--;
		this->SetOption(opt);
		keyboard.keyChar = 0;
	}
	else if (keyboard.keyChar == 255 + VK_DOWN) {
		unsigned short opt = this->GetOption();
		opt++;
		this->SetOption(opt);
		keyboard.keyChar = 0;
	}
	else if (keyboard.keyChar == 13) {
		if (this->func.IsExecutable())
			this->func.Execute();
		keyboard.keyChar = 0;
	}
}

void WindowElementMemo::Print(POINT& d, AD2D_Window& gfx) {
	AD2D_Viewport currentVP = gfx.GetCurrentViewport();
	AD2D_Viewport nextVP;

	POINT scroll = GetParentScrollOffset();
	int offsetH = scroll.x;
	int offsetV = scroll.y;

	nextVP.startX = currentVP.startX + posX - offsetH - d.x;
	nextVP.startY = currentVP.startY + posY - offsetV - d.y;
	nextVP.width = width;
	nextVP.height = height;

	POINT diff = AdjustViewport(currentVP, nextVP);

	gfx.SetViewport(nextVP);

	COLOR currentColor = AD2D_Window::GetColor();

	AD2D_Viewport intNextVP;
	POINT intDiff = diff;
	if (wndTemplate->GetElements() & ELEMENT_MEMO) {
		int x = 0;
		int y = 0;

		POINT diff;
		AD2D_Viewport memoVP;

		memoVP.Create(	currentVP.startX + posX + wndTemplate->tempMemo.l_width - offsetH - d.x,
							currentVP.startY + posY + wndTemplate->tempMemo.t_height - offsetV - d.y,
							currentVP.startX + posX + width - wndTemplate->tempMemo.r_width - offsetH - d.x - 1,
							currentVP.startY + posY + height - wndTemplate->tempMemo.b_height - offsetV - d.y - 1);
		diff = AdjustViewport(nextVP, memoVP);
		gfx.SetViewport(memoVP);
		if (wndTemplate->tempMemo.bg.GetWidth() > 0) {
			for (x = 0; x < width - wndTemplate->tempMemo.l_width - wndTemplate->tempMemo.r_width; x += wndTemplate->tempMemo.bg.GetWidth())
			for (y = 0; y < height - wndTemplate->tempMemo.t_height - wndTemplate->tempMemo.b_height; y += wndTemplate->tempMemo.bg.GetHeight())
				gfx.PutImage(x - diff.x, y - diff.y, wndTemplate->tempMemo.bg);
		}

		memoVP.Create(	currentVP.startX + posX + wndTemplate->tempMemo.l_width - offsetH - d.x,
							currentVP.startY + posY - offsetV - d.y,
							currentVP.startX + posX + width - wndTemplate->tempMemo.r_width - offsetH - d.x - 1,
							currentVP.startY + posY + wndTemplate->tempMemo.t.GetHeight() - offsetV - d.y - 1);
		diff = AdjustViewport(nextVP, memoVP);
		gfx.SetViewport(memoVP);
		if (wndTemplate->tempMemo.t.GetWidth() > 0) {
			for (x = 0; x < width - wndTemplate->tempMemo.l_width - wndTemplate->tempMemo.r_width; x += wndTemplate->tempMemo.t.GetWidth())
				gfx.PutImage(x - diff.x, - diff.y, wndTemplate->tempMemo.t);
		}

		memoVP.Create(	currentVP.startX + posX - offsetH - d.x,
							currentVP.startY + posY + wndTemplate->tempMemo.t_height - offsetV - d.y,
							currentVP.startX + posX + wndTemplate->tempMemo.l.GetWidth() - offsetH - d.x - 1,
							currentVP.startY + posY + height - wndTemplate->tempMemo.b_height - offsetV - d.y - 1);
		diff = AdjustViewport(nextVP, memoVP);
		gfx.SetViewport(memoVP);
		if (wndTemplate->tempMemo.l.GetWidth() > 0) {
			for (y = 0; y < height - wndTemplate->tempMemo.t_height - wndTemplate->tempMemo.b_height; y += wndTemplate->tempMemo.l.GetHeight())
				gfx.PutImage(- diff.x, y - diff.y, wndTemplate->tempMemo.l);
		}

		memoVP.Create(	currentVP.startX + posX + width - wndTemplate->tempMemo.r.GetWidth() - offsetH - d.x,
							currentVP.startY + posY + wndTemplate->tempMemo.t_height - offsetV - d.y,
							currentVP.startX + posX + width - offsetH - d.x - 1,
							currentVP.startY + posY + height - wndTemplate->tempMemo.b_height - offsetV - d.y - 1);
		diff = AdjustViewport(nextVP, memoVP);
		gfx.SetViewport(memoVP);
		if (wndTemplate->tempMemo.r.GetWidth() > 0) {
			for (y = 0; y < height - wndTemplate->tempMemo.t_height - wndTemplate->tempMemo.b_height; y += wndTemplate->tempMemo.r.GetHeight())
				gfx.PutImage(- diff.x, y - diff.y, wndTemplate->tempMemo.r);
		}

		memoVP.Create(	currentVP.startX + posX + wndTemplate->tempMemo.l_width - offsetH - d.x,
							currentVP.startY + posY + height - wndTemplate->tempMemo.b.GetHeight() - offsetV - d.y,
							currentVP.startX + posX + width - wndTemplate->tempMemo.r_width - offsetH - d.x - 1,
							currentVP.startY + posY + height - offsetV - d.y - 1);
		diff = AdjustViewport(nextVP, memoVP);
		gfx.SetViewport(memoVP);
		if (wndTemplate->tempMemo.b.GetWidth() > 0) {
			for (x = 0; x < width - wndTemplate->tempMemo.l_width - wndTemplate->tempMemo.r_width; x += wndTemplate->tempMemo.b.GetWidth())
				gfx.PutImage(x - diff.x, - diff.y, wndTemplate->tempMemo.b);
		}

		memoVP.Create(	currentVP.startX + posX - offsetH - d.x,
							currentVP.startY + posY - offsetV - d.y,
							currentVP.startX + posX + width - offsetH - d.x - 1,
							currentVP.startY + posY + height - offsetV - d.y - 1);
		diff = AdjustViewport(nextVP, memoVP);
		gfx.SetViewport(memoVP);
		gfx.PutImage(- diff.x, - diff.y, wndTemplate->tempMemo.tl);
		gfx.PutImage(width - wndTemplate->tempMemo.tr.GetWidth() - diff.x, - diff.y, wndTemplate->tempMemo.tr);
		gfx.PutImage(- diff.x, height - wndTemplate->tempMemo.bl.GetHeight() - diff.y, wndTemplate->tempMemo.bl);
		gfx.PutImage(width - wndTemplate->tempMemo.br.GetWidth() - diff.x, height - wndTemplate->tempMemo.br.GetHeight() - diff.y, wndTemplate->tempMemo.br);

		intNextVP.startX = currentVP.startX + posX + wndTemplate->tempMemo.l_width - offsetH - d.x;
		intNextVP.startY = currentVP.startY + posY + wndTemplate->tempMemo.t_height - offsetV - d.y;
		intNextVP.width = width - wndTemplate->tempMemo.l_width - wndTemplate->tempMemo.r_width;
		intNextVP.height = height - wndTemplate->tempMemo.t_height - wndTemplate->tempMemo.b_height;
		intDiff = AdjustViewport(nextVP, intNextVP);
		gfx.SetViewport(intNextVP);
	}
	else
		gfx.SetViewport(nextVP);

	std::list<std::pair<TextString, bool> >::iterator it = elements.begin();
	unsigned short opt = 0;
	for (it, opt; it != elements.end(); it++, opt++) {
		if (opt % 2 == 1) {
			AD2D_Window::SetColor(currentColor.red, currentColor.green, currentColor.blue, 0.1f * currentColor.alpha);
			gfx.PutRect(- intDiff.x, opt * fontSize - intDiff.y - (int)this->scrollV, width - intDiff.x, (opt + 1) * fontSize - intDiff.y - (int)this->scrollV);
		}

		if (opt == option) {
			AD2D_Window::SetColor(currentColor.red, currentColor.green, currentColor.blue, 0.3f * currentColor.alpha);
			gfx.PutRect(- intDiff.x, opt * fontSize - intDiff.y - (int)this->scrollV, width - intDiff.x, (opt + 1) * fontSize - intDiff.y - (int)this->scrollV);
		}

		float factor = 1.0f;
		if (!it->second)
			factor = 0.7f;

		AD2D_Window::SetColor(currentColor.red * color.red * factor, currentColor.green * color.green * factor, currentColor.blue * color.blue * factor, currentColor.alpha * color.alpha * factor);
		gfx.CPrint(- intDiff.x, opt * fontSize - intDiff.y - (int)this->scrollV, fontSize, *wndTemplate->font, it->first.text.c_str(), it->first.color.c_str(), border);
	}
	AD2D_Window::SetColor(currentColor.red, currentColor.green, currentColor.blue, currentColor.alpha);

	gfx.SetViewport(nextVP);

	if (wndTemplate->GetElements() & ELEMENT_SCROLL) {
		int offset = GetScrollOffset();

		int y = 0;

		POINT diff;
		AD2D_Viewport scrollVP;
		scrollVP.Create(currentVP.startX + posX + width - wndTemplate->tempMemo.r_width - wndTemplate->tempScroll.w - offsetH - d.x,
						currentVP.startY + posY + wndTemplate->tempMemo.t_height - offsetV - d.y,
						currentVP.startX + posX + width - wndTemplate->tempMemo.r_width - offsetH - d.x - 1,
						currentVP.startY + posY + height - wndTemplate->tempMemo.b_height - offsetV - d.y - 1);
		diff = AdjustViewport(nextVP, scrollVP);
		gfx.SetViewport(scrollVP);

		for (y = 0; y < height - wndTemplate->tempMemo.t_height - wndTemplate->tempMemo.b_height; y += wndTemplate->tempScroll.bgv.GetHeight())
			gfx.PutImage(- diff.x, y - diff.y, wndTemplate->tempScroll.bgv);
		gfx.PutImage(- diff.x, - diff.y, wndTemplate->tempScroll.t);
		gfx.PutImage(- diff.x, height - wndTemplate->tempMemo.t_height - wndTemplate->tempMemo.b_height - wndTemplate->tempScroll.b.GetHeight() - diff.y, wndTemplate->tempScroll.b);

		if (offset) {
			float position = this->scrollV / offset;
			gfx.PutImage(- diff.x, wndTemplate->tempScroll.t.GetHeight() + (int)((height - wndTemplate->tempMemo.t_height - wndTemplate->tempMemo.b_height - wndTemplate->tempScroll.t.GetHeight() - wndTemplate->tempScroll.b.GetHeight() - wndTemplate->tempScroll.brv.GetHeight()) * position) - diff.y, wndTemplate->tempScroll.brv);
		}
	}

	gfx.SetViewport(currentVP);
}


// ---- WindowElementTable ---- //

WindowElementTableMemo::WindowElementTableMemo() { }

WindowElementTableMemo::~WindowElementTableMemo() {
	Clear();
}

void WindowElementTableMemo::Create(unsigned char al, int pX, int pY, unsigned short wdh, unsigned short hgt, WindowTemplate* wTemplate) {
	align = al;
	posX = pX;
	posY = pY;
	width = wdh;
	height = hgt;

	wndTemplate = wTemplate;

	scrollV = 0.0f;

	option = 0;

	rowHeight = 14;

	dblFunc.Clear();
}

void WindowElementTableMemo::SetOption(unsigned short option, bool execute) {
	if (option >= rows.size())
		return;
	this->option = option;

	int borderV = wndTemplate->tempMemo.t_height + wndTemplate->tempMemo.b_height;

	if (scrollV > option * rowHeight)
		scrollV = option * rowHeight;
	else if (scrollV < (option + 1) * rowHeight - (height - borderV))
		scrollV = (option + 1) * rowHeight - (height - borderV);

	if (execute && func.IsExecutable())
		func.Execute();
}


unsigned short WindowElementTableMemo::GetOption() {
	return this->option;
}

unsigned short WindowElementTableMemo::GetOptionsSize() {
	return rows.size();
}


void WindowElementTableMemo::AddRow() {
	std::vector<WindowElement*> row;
	rows.push_back(row);

	option = rows.size() - 1;
}

void WindowElementTableMemo::AddColumn(WindowElement* element) {
	if (option >= 0 & option < rows.size()) {
		rows[option].push_back(element);
		if (rows[option].size() > cols.size()) {
			cols.push_back(0);
			colsNames.push_back("");
		}
	}
}

void WindowElementTableMemo::SetRowHeight(int height) {
	rowHeight = height;
}

void WindowElementTableMemo::SetColumnWidthPX(int num, int width) {
	if (num >= 0 && num < cols.size())
		cols[num] = width;
}

void WindowElementTableMemo::SetColumnWidthPercent(int num, int width) {
	if (num >= 0 && num < cols.size())
		cols[num] = -width;
}

void WindowElementTableMemo::SetColumnName(int num, std::string name) {
	if (num >= 0 && num < cols.size())
		colsNames[num] = name;
}


void WindowElementTableMemo::RemoveRow() {
	if (option >= 0 && option < rows.size())
		rows.erase(rows.begin() + option);
}

void WindowElementTableMemo::MoveUpRow() {
	if (option > 0 && option < rows.size()) {
		std::vector<WindowElement*> temp = rows[option - 1];
		rows[option - 1] = rows[option];
		rows[option] = temp;
		option--;
	}
}

void WindowElementTableMemo::MoveDownRow() {
	if (option >= 0 && option < rows.size() - 1) {
		std::vector<WindowElement*> temp = rows[option + 1];
		rows[option + 1] = rows[option];
		rows[option] = temp;
		option++;
	}
}

void WindowElementTableMemo::Clear() {
	option = 0;
	for (int i = 0; i < rows.size(); i++)
	for (int j = 0; j < rows[i].size(); j++) {
		WindowElement* element = rows[i][j];
		if (element)
			delete_debug(element, M_PLACE);
	}
	rows.clear();
}


void WindowElementTableMemo::SetDblAction(boost::function<void()> func) {
	if (!func) {
		dblFunc.Clear();
		return;
	}

    dblFunc.PushFunction(func);
}


void WindowElementTableMemo::SetSize(unsigned short wdh, unsigned short hgt) {
	width = wdh;
	height = hgt;

	unsigned int offset = GetScrollOffset();
	if (scrollV > (float)offset) scrollV = (float)offset;
}


int WindowElementTableMemo::GetScrollOffset() {
	int borderV = wndTemplate->tempMemo.t_height + wndTemplate->tempMemo.b_height;

	int intHeight = rows.size() * rowHeight + rowHeight;

	int offset = intHeight - (height - borderV);
	if (offset < 0) offset = 0;

	return offset;
}

void WindowElementTableMemo::CheckInput(Keyboard& keyboard) {
	if (keyboard.keyChar == 255 + VK_UP) {
		unsigned short opt = this->GetOption();
		if (opt > 0) opt--;
		this->SetOption(opt);
		keyboard.keyChar = 0;
	}
	else if (keyboard.keyChar == 255 + VK_DOWN) {
		unsigned short opt = this->GetOption();
		opt++;
		this->SetOption(opt);
		keyboard.keyChar = 0;
	}
	else if (keyboard.keyChar == 13) {
		if (this->func.IsExecutable())
			this->func.Execute();
		keyboard.keyChar = 0;
	}
}

void WindowElementTableMemo::Print(POINT& d, AD2D_Window& gfx) {
	AD2D_Viewport currentVP = gfx.GetCurrentViewport();
	AD2D_Viewport nextVP;

	POINT scroll = GetParentScrollOffset();
	int offsetH = scroll.x;
	int offsetV = scroll.y;

	nextVP.startX = currentVP.startX + posX - offsetH - d.x;
	nextVP.startY = currentVP.startY + posY - offsetV - d.y;
	nextVP.width = width;
	nextVP.height = height;

	POINT diff = AdjustViewport(currentVP, nextVP);

	gfx.SetViewport(nextVP);

	COLOR currentColor = AD2D_Window::GetColor();

	AD2D_Viewport intNextVP;
	POINT intDiff = diff;
	if (wndTemplate->GetElements() & ELEMENT_MEMO) {
		int x = 0;
		int y = 0;

		POINT diff;
		AD2D_Viewport memoVP;

		memoVP.Create(	currentVP.startX + posX + wndTemplate->tempMemo.l_width - offsetH - d.x,
							currentVP.startY + posY + wndTemplate->tempMemo.t_height - offsetV - d.y,
							currentVP.startX + posX + width - wndTemplate->tempMemo.r_width - offsetH - d.x - 1,
							currentVP.startY + posY + height - wndTemplate->tempMemo.b_height - offsetV - d.y - 1);
		diff = AdjustViewport(nextVP, memoVP);
		gfx.SetViewport(memoVP);
		if (wndTemplate->tempMemo.bg.GetWidth() > 0) {
			for (x = 0; x < width - wndTemplate->tempMemo.l_width - wndTemplate->tempMemo.r_width; x += wndTemplate->tempMemo.bg.GetWidth())
			for (y = 0; y < height - wndTemplate->tempMemo.t_height - wndTemplate->tempMemo.b_height; y += wndTemplate->tempMemo.bg.GetHeight())
				gfx.PutImage(x - diff.x, y - diff.y, wndTemplate->tempMemo.bg);
		}

		memoVP.Create(	currentVP.startX + posX + wndTemplate->tempMemo.l_width - offsetH - d.x,
							currentVP.startY + posY - offsetV - d.y,
							currentVP.startX + posX + width - wndTemplate->tempMemo.r_width - offsetH - d.x - 1,
							currentVP.startY + posY + wndTemplate->tempMemo.t.GetHeight() - offsetV - d.y - 1);
		diff = AdjustViewport(nextVP, memoVP);
		gfx.SetViewport(memoVP);
		if (wndTemplate->tempMemo.t.GetWidth() > 0) {
			for (x = 0; x < width - wndTemplate->tempMemo.l_width - wndTemplate->tempMemo.r_width; x += wndTemplate->tempMemo.t.GetWidth())
				gfx.PutImage(x - diff.x, - diff.y, wndTemplate->tempMemo.t);
		}

		memoVP.Create(	currentVP.startX + posX - offsetH - d.x,
							currentVP.startY + posY + wndTemplate->tempMemo.t_height - offsetV - d.y,
							currentVP.startX + posX + wndTemplate->tempMemo.l.GetWidth() - offsetH - d.x - 1,
							currentVP.startY + posY + height - wndTemplate->tempMemo.b_height - offsetV - d.y - 1);
		diff = AdjustViewport(nextVP, memoVP);
		gfx.SetViewport(memoVP);
		if (wndTemplate->tempMemo.l.GetWidth() > 0) {
			for (y = 0; y < height - wndTemplate->tempMemo.t_height - wndTemplate->tempMemo.b_height; y += wndTemplate->tempMemo.l.GetHeight())
				gfx.PutImage(- diff.x, y - diff.y, wndTemplate->tempMemo.l);
		}

		memoVP.Create(	currentVP.startX + posX + width - wndTemplate->tempMemo.r.GetWidth() - offsetH - d.x,
							currentVP.startY + posY + wndTemplate->tempMemo.t_height - offsetV - d.y,
							currentVP.startX + posX + width - offsetH - d.x - 1,
							currentVP.startY + posY + height - wndTemplate->tempMemo.b_height - offsetV - d.y - 1);
		diff = AdjustViewport(nextVP, memoVP);
		gfx.SetViewport(memoVP);
		if (wndTemplate->tempMemo.r.GetWidth() > 0) {
			for (y = 0; y < height - wndTemplate->tempMemo.t_height - wndTemplate->tempMemo.b_height; y += wndTemplate->tempMemo.r.GetHeight())
				gfx.PutImage(- diff.x, y - diff.y, wndTemplate->tempMemo.r);
		}

		memoVP.Create(	currentVP.startX + posX + wndTemplate->tempMemo.l_width - offsetH - d.x,
							currentVP.startY + posY + height - wndTemplate->tempMemo.b.GetHeight() - offsetV - d.y,
							currentVP.startX + posX + width - wndTemplate->tempMemo.r_width - offsetH - d.x - 1,
							currentVP.startY + posY + height - offsetV - d.y - 1);
		diff = AdjustViewport(nextVP, memoVP);
		gfx.SetViewport(memoVP);
		if (wndTemplate->tempMemo.b.GetWidth() > 0) {
			for (x = 0; x < width - wndTemplate->tempMemo.l_width - wndTemplate->tempMemo.r_width; x += wndTemplate->tempMemo.b.GetWidth())
				gfx.PutImage(x - diff.x, - diff.y, wndTemplate->tempMemo.b);
		}

		memoVP.Create(	currentVP.startX + posX - offsetH - d.x,
							currentVP.startY + posY - offsetV - d.y,
							currentVP.startX + posX + width - offsetH - d.x - 1,
							currentVP.startY + posY + height - offsetV - d.y - 1);
		diff = AdjustViewport(nextVP, memoVP);
		gfx.SetViewport(memoVP);
		gfx.PutImage(- diff.x, - diff.y, wndTemplate->tempMemo.tl);
		gfx.PutImage(width - wndTemplate->tempMemo.tr.GetWidth() - diff.x, - diff.y, wndTemplate->tempMemo.tr);
		gfx.PutImage(- diff.x, height - wndTemplate->tempMemo.bl.GetHeight() - diff.y, wndTemplate->tempMemo.bl);
		gfx.PutImage(width - wndTemplate->tempMemo.br.GetWidth() - diff.x, height - wndTemplate->tempMemo.br.GetHeight() - diff.y, wndTemplate->tempMemo.br);

		intNextVP.startX = currentVP.startX + posX + wndTemplate->tempMemo.l_width - offsetH - d.x;
		intNextVP.startY = currentVP.startY + posY + wndTemplate->tempMemo.t_height - offsetV - d.y;
		intNextVP.width = width - wndTemplate->tempMemo.l_width - wndTemplate->tempMemo.r_width;
		intNextVP.height = height - wndTemplate->tempMemo.t_height - wndTemplate->tempMemo.b_height;
		intDiff = AdjustViewport(nextVP, intNextVP);
		gfx.SetViewport(intNextVP);
	}
	else
		gfx.SetViewport(nextVP);

	int intWidth = width - wndTemplate->tempMemo.l_width - wndTemplate->tempMemo.r_width;

	std::vector<float> widthVec;
	for (int i = 0; i < cols.size(); i++) {
		if (cols[i] > 0) widthVec.push_back(cols[i]);
		else if (cols[i] < 0) widthVec.push_back(-(float)(intWidth * cols[i]) / 100);
		else widthVec.push_back((float)intWidth / cols.size());
	}

	for (int i = 0; i < rows.size() + 1; i++) {
		if (i == 0) {
			AD2D_Window::SetColor(0.0f, 0.0f, 0.0f, 0.3f * currentColor.alpha);
			gfx.PutRect(- intDiff.x, i * rowHeight - intDiff.y - (int)this->scrollV, width - intDiff.x, (i + 1) * rowHeight - intDiff.y - (int)this->scrollV);

			AD2D_Window::SetColor(currentColor.red, currentColor.green, currentColor.blue, currentColor.alpha);
			float posX = 0.0f;
			for (int j = 0; j < cols.size(); j++) {
				std::string name = (j < colsNames.size() ? colsNames[j] : std::string(""));
				int middle = (widthVec[j] - wndTemplate->font->GetTextWidth(name, rowHeight)) / 2;
				gfx.Print(posX + middle - intDiff.x, i * rowHeight - intDiff.y - (int)this->scrollV, rowHeight, *wndTemplate->font, name.c_str(), border);
				posX += widthVec[j];
			}
		}
		else {
			if (i % 2 == 0) {
				AD2D_Window::SetColor(currentColor.red, currentColor.green, currentColor.blue, 0.05f * currentColor.alpha);
				gfx.PutRect(- intDiff.x, i * rowHeight - intDiff.y - (int)this->scrollV, width - intDiff.x, (i + 1) * rowHeight - intDiff.y - (int)this->scrollV);
			}

			if (i - 1 == option) {
				AD2D_Window::SetColor(currentColor.red, currentColor.green, currentColor.blue, 0.3f * currentColor.alpha);
				gfx.PutRect(- intDiff.x, i * rowHeight - intDiff.y - (int)this->scrollV, width - intDiff.x, (i + 1) * rowHeight - intDiff.y - (int)this->scrollV);
			}

			AD2D_Window::SetColor(currentColor.red, currentColor.green, currentColor.blue, currentColor.alpha);
			float posX = 0.0f;
			for (int j = 0; j < rows[i - 1].size(); j++) {
				if (j < cols.size()) {
					WindowElement* element = rows[i - 1][j];
					if (element) {
						POINT d = intDiff;
						d.x -= (int)posX;
						d.y -= i * rowHeight - (int)this->scrollV;
						element->Print(d, gfx);
					}
					posX += widthVec[j];
				}
			}
		}
	}

	if (wndTemplate->tempMemo.m.GetHeight()) {
		AD2D_Window::SetColor(currentColor.red, currentColor.green, currentColor.blue, currentColor.alpha);
		float posX = 0;
		for (int i = 0; i < widthVec.size(); i++) {
			posX += widthVec[i];
			for (int j = - intDiff.y - (int)this->scrollV; j < height; j += wndTemplate->tempMemo.m.GetHeight())
				gfx.PutImage(posX - intDiff.x - wndTemplate->tempMemo.m.GetWidth() / 2, j, wndTemplate->tempMemo.m);
		}
	}

	gfx.SetViewport(nextVP);

	if (wndTemplate->GetElements() & ELEMENT_SCROLL) {
		int offset = GetScrollOffset();

		int y = 0;

		POINT diff;
		AD2D_Viewport scrollVP;
		scrollVP.Create(currentVP.startX + posX + width - wndTemplate->tempMemo.r_width - wndTemplate->tempScroll.w - offsetH - d.x,
						currentVP.startY + posY + wndTemplate->tempMemo.t_height - offsetV - d.y,
						currentVP.startX + posX + width - wndTemplate->tempMemo.r_width - offsetH - d.x - 1,
						currentVP.startY + posY + height - wndTemplate->tempMemo.b_height - offsetV - d.y - 1);
		diff = AdjustViewport(nextVP, scrollVP);
		gfx.SetViewport(scrollVP);

		for (y = 0; y < height - wndTemplate->tempMemo.t_height - wndTemplate->tempMemo.b_height; y += wndTemplate->tempScroll.bgv.GetHeight())
			gfx.PutImage(- diff.x, y - diff.y, wndTemplate->tempScroll.bgv);
		gfx.PutImage(- diff.x, - diff.y, wndTemplate->tempScroll.t);
		gfx.PutImage(- diff.x, height - wndTemplate->tempMemo.t_height - wndTemplate->tempMemo.b_height - wndTemplate->tempScroll.b.GetHeight() - diff.y, wndTemplate->tempScroll.b);

		if (offset) {
			float position = this->scrollV / offset;
			gfx.PutImage(- diff.x, wndTemplate->tempScroll.t.GetHeight() + (int)((height - wndTemplate->tempMemo.t_height - wndTemplate->tempMemo.b_height - wndTemplate->tempScroll.t.GetHeight() - wndTemplate->tempScroll.b.GetHeight() - wndTemplate->tempScroll.brv.GetHeight()) * position) - diff.y, wndTemplate->tempScroll.brv);
		}
	}

	gfx.SetViewport(currentVP);
}


// ---- WindowElementList ---- //

WindowElementList::WindowElementList() { }

WindowElementList::~WindowElementList() { }

void WindowElementList::Create(unsigned char al, int pX, int pY, unsigned short wdh, unsigned short hgt, WindowTemplate* wTemplate) {
	align = al;
	posX = pX;
	posY = pY;
	width = wdh;
	height = hgt;

	wndTemplate = wTemplate;

	option = 0;

	fontSize = 14;

	color = COLOR(1.0f, 1.0f, 1.0f, 1.0f);
}

void WindowElementList::SetColor(float r, float g, float b, float a) {
	color.red = r;
	color.green = g;
	color.blue = b;
	color.alpha = a;
}

void WindowElementList::SetOption(unsigned short option) {
	if (option >= elements.size())
		option = elements.size() - 1;
	this->option = option;

	if (func.IsExecutable())
		func.Execute();
}

void WindowElementList::SetOption(std::string option) {
	std::list<std::pair<TextString, bool> >::iterator it = elements.begin();
	int opt = 0;
	for (it; it != elements.end(); it++) {
		if (it->first.text == option) {
			this->option = opt;
			break;
		}
		opt++;
	}

	if (func.IsExecutable())
		func.Execute();
}

unsigned short WindowElementList::GetOption() {
	return this->option;
}

unsigned short WindowElementList::GetOptionsSize() {
	return elements.size();
}

void WindowElementList::AddElement(TextString text, bool active) {
	elements.push_back(std::pair<TextString, bool>(text, active));
}

TextString WindowElementList::GetElement() {
	std::list<std::pair<TextString, bool> >::iterator it = elements.begin();
	unsigned short opt = 0;
	for (it, opt; it != elements.end() && opt < option; it++, opt++);

	return (it != elements.end() ? it->first : TextString(""));
}

void WindowElementList::RemoveElement() {
	std::list<std::pair<TextString, bool> >::iterator it = elements.begin();
	unsigned short opt = 0;
	for (it, opt; it != elements.end() && opt < option; it++, opt++);

	if (it != elements.end())
		elements.erase(it);
}

void WindowElementList::Clear() {
	option = 0;
	elements.clear();
}


void WindowElementList::CheckInput(Keyboard& keyboard) {
	if (keyboard.keyChar == 255 + VK_UP) {
		unsigned short opt = this->GetOption();
		if (opt > 0) opt--;
		this->SetOption(opt);
		keyboard.keyChar = 0;
	}
	else if (keyboard.keyChar == 255 + VK_DOWN) {
		unsigned short opt = this->GetOption();
		opt++;
		this->SetOption(opt);
		keyboard.keyChar = 0;
	}
	else if (keyboard.keyChar == 13) {
		if (this->func.IsExecutable())
			this->func.Execute();
		keyboard.keyChar = 0;
	}
}

void WindowElementList::OpenMenu(Mouse& mouse, Keyboard& keyboard, std::list<MenuData>& actionList) {
	std::list<std::pair<TextString, bool> >::iterator it = elements.begin();
	for (it; it != elements.end(); it++) {
		TextString text = it->first;
		if (it->second) {
			Signal* signal = new(M_PLACE) Signal;
			signal->PushFunction(boost::bind(static_cast<void (WindowElementList::*)(std::string)>(&WindowElementList::SetOption), this, text.text));
			actionList.push_back(MenuData(ACTION_EXECUTE, text, (void*)signal));
		}
	}
}

void WindowElementList::Print(POINT& d, AD2D_Window& gfx) {
	AD2D_Viewport currentVP = gfx.GetCurrentViewport();
	AD2D_Viewport nextVP;

	POINT scroll = GetParentScrollOffset();
	int offsetH = scroll.x;
	int offsetV = scroll.y;

	nextVP.startX = currentVP.startX + posX - offsetH - d.x;
	nextVP.startY = currentVP.startY + posY - offsetV - d.y;
	nextVP.width = width;
	nextVP.height = height;

	POINT diff = AdjustViewport(currentVP, nextVP);

	gfx.SetViewport(nextVP);

	COLOR currentColor = AD2D_Window::GetColor();

	AD2D_Viewport intNextVP;
	POINT intDiff = diff;
	if (wndTemplate->GetElements() & ELEMENT_LIST) {
		int x = 0;
		int y = 0;

		POINT diff;
		AD2D_Viewport listVP;

		listVP.Create(	currentVP.startX + posX + wndTemplate->tempList.l_width - offsetH - d.x,
							currentVP.startY + posY + wndTemplate->tempList.t_height - offsetV - d.y,
							currentVP.startX + posX + width - wndTemplate->tempList.r_width - offsetH - d.x - 1,
							currentVP.startY + posY + height - wndTemplate->tempList.b_height - offsetV - d.y - 1);
		diff = AdjustViewport(nextVP, listVP);
		gfx.SetViewport(listVP);
		if (wndTemplate->tempList.bg.GetWidth() > 0) {
			for (x = 0; x < width - wndTemplate->tempList.l_width - wndTemplate->tempList.r_width; x += wndTemplate->tempList.bg.GetWidth())
			for (y = 0; y < height - wndTemplate->tempList.t_height - wndTemplate->tempList.b_height; y += wndTemplate->tempList.bg.GetHeight())
				gfx.PutImage(x - diff.x, y - diff.y, wndTemplate->tempList.bg);
		}

		listVP.Create(	currentVP.startX + posX + wndTemplate->tempList.l_width - offsetH - d.x,
							currentVP.startY + posY - offsetV - d.y,
							currentVP.startX + posX + width - wndTemplate->tempList.r_width - offsetH - d.x - 1,
							currentVP.startY + posY + wndTemplate->tempList.t.GetHeight() - offsetV - d.y - 1);
		diff = AdjustViewport(nextVP, listVP);
		gfx.SetViewport(listVP);
		if (wndTemplate->tempList.t.GetWidth() > 0) {
			for (x = 0; x < width - wndTemplate->tempList.l_width - wndTemplate->tempList.r_width; x += wndTemplate->tempList.t.GetWidth())
				gfx.PutImage(x - diff.x, - diff.y, wndTemplate->tempList.t);
		}

		listVP.Create(	currentVP.startX + posX - offsetH - d.x,
							currentVP.startY + posY + wndTemplate->tempList.t_height - offsetV - d.y,
							currentVP.startX + posX + wndTemplate->tempList.l.GetWidth() - offsetH - d.x - 1,
							currentVP.startY + posY + height - wndTemplate->tempList.b_height - offsetV - d.y - 1);
		diff = AdjustViewport(nextVP, listVP);
		gfx.SetViewport(listVP);
		if (wndTemplate->tempList.l.GetWidth() > 0) {
			for (y = 0; y < height - wndTemplate->tempList.t_height - wndTemplate->tempList.b_height; y += wndTemplate->tempList.l.GetHeight())
				gfx.PutImage(- diff.x, y - diff.y, wndTemplate->tempList.l);
		}

		listVP.Create(	currentVP.startX + posX + width - wndTemplate->tempList.r.GetWidth() - offsetH - d.x,
							currentVP.startY + posY + wndTemplate->tempList.t_height - offsetV - d.y,
							currentVP.startX + posX + width - offsetH - d.x - 1,
							currentVP.startY + posY + height - wndTemplate->tempList.b_height - offsetV - d.y - 1);
		diff = AdjustViewport(nextVP, listVP);
		gfx.SetViewport(listVP);
		if (wndTemplate->tempList.r.GetWidth() > 0) {
			for (y = 0; y < height - wndTemplate->tempList.t_height - wndTemplate->tempList.b_height; y += wndTemplate->tempList.r.GetHeight())
				gfx.PutImage(- diff.x, y - diff.y, wndTemplate->tempList.r);
		}

		listVP.Create(	currentVP.startX + posX + wndTemplate->tempList.l_width - offsetH - d.x,
							currentVP.startY + posY + height - wndTemplate->tempList.b.GetHeight() - offsetV - d.y,
							currentVP.startX + posX + width - wndTemplate->tempList.r_width - offsetH - d.x - 1,
							currentVP.startY + posY + height - offsetV - d.y - 1);
		diff = AdjustViewport(nextVP, listVP);
		gfx.SetViewport(listVP);
		if (wndTemplate->tempList.b.GetWidth() > 0) {
			for (x = 0; x < width - wndTemplate->tempList.l_width - wndTemplate->tempList.r_width; x += wndTemplate->tempList.b.GetWidth())
				gfx.PutImage(x - diff.x, - diff.y, wndTemplate->tempList.b);
		}

		listVP.Create(	currentVP.startX + posX - offsetH - d.x,
							currentVP.startY + posY - offsetV - d.y,
							currentVP.startX + posX + width - offsetH - d.x - 1,
							currentVP.startY + posY + height - offsetV - d.y - 1);
		diff = AdjustViewport(nextVP, listVP);
		gfx.SetViewport(listVP);
		gfx.PutImage(- diff.x, - diff.y, wndTemplate->tempList.tl);
		gfx.PutImage(width - wndTemplate->tempList.tr.GetWidth() - diff.x, - diff.y, wndTemplate->tempList.tr);
		gfx.PutImage(- diff.x, height - wndTemplate->tempList.bl.GetHeight() - diff.y, wndTemplate->tempList.bl);
		gfx.PutImage(width - wndTemplate->tempList.br.GetWidth() - diff.x, height - wndTemplate->tempList.br.GetHeight() - diff.y, wndTemplate->tempList.br);

		intNextVP.startX = currentVP.startX + posX + wndTemplate->tempList.l_width - offsetH - d.x;
		intNextVP.startY = currentVP.startY + posY + wndTemplate->tempList.t_height - offsetV - d.y;
		intNextVP.width = width - wndTemplate->tempList.l_width - wndTemplate->tempList.r_width;
		intNextVP.height = height - wndTemplate->tempList.t_height - wndTemplate->tempList.b_height;
		intDiff = AdjustViewport(nextVP, intNextVP);
		gfx.SetViewport(intNextVP);
	}
	else
		gfx.SetViewport(nextVP);

	TextString text = GetElement();
	AD2D_Window::SetColor(currentColor.red * color.red, currentColor.green * color.green, currentColor.blue * color.blue, currentColor.alpha * color.alpha);
	gfx.CPrint(0, 0, fontSize, *wndTemplate->font, text.text.c_str(), text.color.c_str(), border);
	AD2D_Window::SetColor(currentColor.red, currentColor.green, currentColor.blue, currentColor.alpha);

	if (wndTemplate->GetElements() & ELEMENT_LIST) {
		int selectorPosX = (width - wndTemplate->tempList.l_width - wndTemplate->tempList.r_width - wndTemplate->tempList.se.GetWidth());
		int selectorPosY = (height - wndTemplate->tempList.t_height - wndTemplate->tempList.b_height - wndTemplate->tempList.se.GetHeight()) / 2;
		gfx.PutImage(selectorPosX, selectorPosY, wndTemplate->tempList.se);
	}

	gfx.SetViewport(currentVP);
}


// ---- WindowElementTimer ---- //

MUTEX	WindowElementTimer::lockLoop;

WindowElementTimer::WindowElementTimer() { }

WindowElementTimer::~WindowElementTimer() {
	Stop();
	if (loopThread.joinable())
        loopThread.join();
}

void WindowElementTimer::Create(unsigned char al, int pX, int pY, unsigned short wdh, unsigned short hgt, WindowTemplate* wTemplate) {
	align = al;
	posX = pX;
	posY = pY;
	width = wdh;
	height = hgt;

	wndTemplate = wTemplate;

	startTime = 0;
	stopTime = 0;
}

void WindowElementTimer::SetStopTime(time_lt time) {
	stopTime = time;
}

void WindowElementTimer::SetTimeInterval(time_lt time) {
	stopTime = RealTime::getTime() + time;
}

void WindowElementTimer::Run() {
	LOCKCLASS lockClass(lockLoop);

	startTime = RealTime::getTime();

	loopFunc.Clear();
	loopFunc.Assign(func);
	THREAD loop(&WindowElementTimer::Loop, this);
	loopThread.swap(loop);
}

void WindowElementTimer::Loop() {
	while(RealTime::getTime() < stopTime && startTime)
		Sleep(50);

	LOCKCLASS lockClass(lockLoop);

	startTime = 0;

	if (loopFunc.IsExecutable()) {
		loopFunc.Execute();
		PostMessage(Windows::hWnd, WM_CHAR, 0, 0);
	}

	loopFunc.Clear();
}

void WindowElementTimer::Stop() {
	LOCKCLASS lockClass(lockLoop);

	loopFunc.Clear();

	startTime = 0;
}

void WindowElementTimer::Print(POINT& d, AD2D_Window& gfx) {
	AD2D_Viewport currentVP = gfx.GetCurrentViewport();
	AD2D_Viewport nextVP;

	POINT scroll = GetParentScrollOffset();
	int offsetH = scroll.x;
	int offsetV = scroll.y;

	nextVP.startX = currentVP.startX + posX - offsetH - d.x;
	nextVP.startY = currentVP.startY + posY - offsetV - d.y;
	nextVP.width = width;
	nextVP.height = height;

	POINT diff = AdjustViewport(currentVP, nextVP);

	gfx.SetViewport(nextVP);

	COLOR currentColor = AD2D_Window::GetColor();

	if (wndTemplate->GetElements() & ELEMENT_TIMER) {
		int x = 0;
		int y = 0;

		POINT diff;
		AD2D_Viewport timerVP;

		timerVP.Create(	currentVP.startX + posX + wndTemplate->tempTimer.l_width - offsetH - d.x,
						currentVP.startY + posY + wndTemplate->tempTimer.t_height - offsetV - d.y,
						currentVP.startX + posX + width - wndTemplate->tempTimer.r_width - offsetH - d.x - 1,
						currentVP.startY + posY + height - wndTemplate->tempTimer.b_height - offsetV - d.y - 1);
		diff = AdjustViewport(nextVP, timerVP);
		gfx.SetViewport(timerVP);
		if (wndTemplate->tempTimer.bg.GetWidth() > 0) {
			for (x = 0; x < width - wndTemplate->tempTimer.l_width - wndTemplate->tempTimer.r_width; x += wndTemplate->tempTimer.bg.GetWidth())
			for (y = 0; y < height - wndTemplate->tempTimer.t_height - wndTemplate->tempTimer.b_height; y += wndTemplate->tempTimer.bg.GetHeight())
				gfx.PutImage(x - diff.x, y - diff.y, wndTemplate->tempTimer.bg);
		}

		timerVP.Create(	currentVP.startX + posX + wndTemplate->tempTimer.l_width - offsetH - d.x,
						currentVP.startY + posY - offsetV - d.y,
						currentVP.startX + posX + width - wndTemplate->tempTimer.r_width - offsetH - d.x - 1,
						currentVP.startY + posY + wndTemplate->tempTimer.t.GetHeight() - offsetV - d.y - 1);
		diff = AdjustViewport(nextVP, timerVP);
		gfx.SetViewport(timerVP);
		if (wndTemplate->tempTimer.t.GetWidth() > 0) {
			for (x = 0; x < width - wndTemplate->tempTimer.l_width - wndTemplate->tempTimer.r_width; x += wndTemplate->tempTimer.t.GetWidth())
				gfx.PutImage(x - diff.x, - diff.y, wndTemplate->tempTimer.t);
		}

		timerVP.Create(	currentVP.startX + posX - offsetH - d.x,
						currentVP.startY + posY + wndTemplate->tempTimer.t_height - offsetV - d.y,
						currentVP.startX + posX + wndTemplate->tempTimer.l.GetWidth() - offsetH - d.x - 1,
						currentVP.startY + posY + height - wndTemplate->tempTimer.b_height - offsetV - d.y - 1);
		diff = AdjustViewport(nextVP, timerVP);
		gfx.SetViewport(timerVP);
		if (wndTemplate->tempTimer.l.GetWidth() > 0) {
			for (y = 0; y < height - wndTemplate->tempTimer.t_height - wndTemplate->tempTimer.b_height; y += wndTemplate->tempTimer.l.GetHeight())
				gfx.PutImage(- diff.x, y - diff.y, wndTemplate->tempTimer.l);
		}

		timerVP.Create(	currentVP.startX + posX + width - wndTemplate->tempTimer.r.GetWidth() - offsetH - d.x,
						currentVP.startY + posY + wndTemplate->tempTimer.t_height - offsetV - d.y,
						currentVP.startX + posX + width - offsetH - d.x - 1,
						currentVP.startY + posY + height - wndTemplate->tempTimer.b_height - offsetV - d.y - 1);
		diff = AdjustViewport(nextVP, timerVP);
		gfx.SetViewport(timerVP);
		if (wndTemplate->tempTimer.r.GetWidth() > 0) {
			for (y = 0; y < height - wndTemplate->tempTimer.t_height - wndTemplate->tempTimer.b_height; y += wndTemplate->tempTimer.r.GetHeight())
				gfx.PutImage(- diff.x, y - diff.y, wndTemplate->tempTimer.r);
		}

		timerVP.Create(	currentVP.startX + posX + wndTemplate->tempTimer.l_width - offsetH - d.x,
						currentVP.startY + posY + height - wndTemplate->tempTimer.b.GetHeight() - offsetV - d.y,
						currentVP.startX + posX + width - wndTemplate->tempTimer.r_width - offsetH - d.x - 1,
						currentVP.startY + posY + height - offsetV - d.y - 1);
		diff = AdjustViewport(nextVP, timerVP);
		gfx.SetViewport(timerVP);
		if (wndTemplate->tempTimer.b.GetWidth() > 0) {
			for (x = 0; x < width - wndTemplate->tempTimer.l_width - wndTemplate->tempTimer.r_width; x += wndTemplate->tempTimer.b.GetWidth())
				gfx.PutImage(x - diff.x, - diff.y, wndTemplate->tempTimer.b);
		}

		timerVP.Create(	currentVP.startX + posX - offsetH - d.x,
						currentVP.startY + posY - offsetV - d.y,
						currentVP.startX + posX + width - offsetH - d.x - 1,
						currentVP.startY + posY + height - offsetV - d.y - 1);
		diff = AdjustViewport(nextVP, timerVP);
		gfx.SetViewport(timerVP);
		gfx.PutImage(- diff.x, - diff.y, wndTemplate->tempTimer.tl);
		gfx.PutImage(width - wndTemplate->tempTimer.tr.GetWidth() - diff.x, - diff.y, wndTemplate->tempTimer.tr);
		gfx.PutImage(- diff.x, height - wndTemplate->tempTimer.bl.GetHeight() - diff.y, wndTemplate->tempTimer.bl);
		gfx.PutImage(width - wndTemplate->tempTimer.br.GetWidth() - diff.x, height - wndTemplate->tempTimer.br.GetHeight() - diff.y, wndTemplate->tempTimer.br);
	}

	gfx.SetViewport(nextVP);

	long offset = (startTime != 0 ? stopTime - RealTime::getTime() : 0);
	if (offset <= 0) offset = 0;

	float factor = 1.0f - (float)offset / (stopTime - startTime);

	int borderH = wndTemplate->tempTimer.l_width + wndTemplate->tempTimer.r_width;
	int borderV = wndTemplate->tempTimer.t_height + wndTemplate->tempTimer.b_height;

	AD2D_Window::SetColor(0.0f * currentColor.red, 1.0f * currentColor.green, 0.0f * currentColor.blue, 0.3f * currentColor.alpha);
	gfx.PutRect(wndTemplate->tempTimer.l_width - diff.x, wndTemplate->tempTimer.t_height - diff.y, wndTemplate->tempTimer.l_width - diff.x + (int)((float)(width - borderH) * factor), wndTemplate->tempTimer.t_height - diff.y + (int)(height - borderV));
	AD2D_Window::SetColor(currentColor.red, currentColor.green, currentColor.blue, currentColor.alpha);

	gfx.SetViewport(currentVP);
}


// ---- WindowElementTab ---- //

WindowElementTab::WindowElementTab() {
	buttonsContainer = NULL;
	mainContainer = NULL;
	buttonLeft = NULL;
	buttonRight = NULL;
}

WindowElementTab::~WindowElementTab() {
	buttonsContainer->RemoveAllElements();
	mainContainer->RemoveAllElements();

	TabList::iterator tit = tabList.begin();
	for (tit; tit != tabList.end(); tit++) {
		WindowElementButton* button = tit->first;
		WindowElementContainer* container = tit->second;
		if (button)
			delete_debug(button, M_PLACE);
		if (container)
			delete_debug(container, M_PLACE);
	}

	std::list<WindowElement*>::iterator it = elements.begin();
	for (it; it != elements.end(); it++) {
		WindowElement* element = *it;
		if (element)
			delete_debug(element, M_PLACE);
	}

	elements.clear();
}

void WindowElementTab::Create(unsigned char al, int pX, int pY, unsigned short wdh, unsigned short hgt, bool bg, bool br, WindowTemplate* wTemplate) {
	align = al;
	posX = pX;
	posY = pY;
	width = wdh;
	height = hgt;
	tabsHeight = 16;
	cntbackground = false;
	cntborder = false;
	wndTemplate = wTemplate;

	color = COLOR(1.0f, 1.0f, 1.0f, 1.0f);

	SetScrollAlwaysVisible(false, false);

	buttonsContainer = new(M_PLACE) WindowElementContainer;
	buttonsContainer->Create(al, 16, 0, wdh - 32, tabsHeight, false, false, wTemplate);
	buttonsContainer->SetScroll(false);
	buttonsContainer->SetLocks(true, false);

	mainContainer = new(M_PLACE) WindowElementContainer;
	mainContainer->Create(al, 0, 16, wdh, hgt - tabsHeight, bg, br, wTemplate);
	mainContainer->SetLocks(true, true);

	buttonLeft = new(M_PLACE) WindowElementButton;
	buttonLeft->Create(ALIGN_H_LEFT, BUTTON_TAB_SIDE, 0, 0, 16, tabsHeight, wTemplate);
	buttonLeft->SetText("<");
	buttonLeft->SetBorder(border);
	buttonLeft->SetFontSize(fontSize);
	buttonLeft->SetColor(color.red, color.green, color.blue, color.alpha);

	boost::function<void()> funcLeft = boost::bind(&WindowElementTab::ScrollTab, this, -50);
	buttonLeft->SetAction(funcLeft);

	buttonRight = new(M_PLACE) WindowElementButton;
	buttonRight->Create(ALIGN_H_RIGHT, BUTTON_TAB_SIDE, wdh - 16, 0, 16, tabsHeight, wTemplate);
	buttonRight->SetText(">");
	buttonRight->SetBorder(border);
	buttonRight->SetFontSize(fontSize);
	buttonRight->SetColor(color.red, color.green, color.blue, color.alpha);

	boost::function<void()> funcRight = boost::bind(&WindowElementTab::ScrollTab, this, 50);
	buttonRight->SetAction(funcRight);

	this->AddElement(buttonsContainer);
	this->AddElement(mainContainer);

	this->AddElement(buttonLeft);
	this->AddElement(buttonRight);
}

void WindowElementTab::SetColor(float r, float g, float b, float a) {
	this->color = COLOR(r, g, b, a);

	if (buttonsContainer) {
		TabList::iterator it = tabList.begin();
		for (it; it != tabList.end(); it++) {
			WindowElementButton* button = it->first;
			button->SetColor(r, g, b, a);
		}
	}
	if (buttonLeft)
		buttonLeft->SetColor(r, g, b, a);
	if (buttonRight)
		buttonRight->SetColor(r, g, b, a);
}

void WindowElementTab::SetTabsHeight(int hgt) {
	tabsHeight = hgt;

	if (buttonsContainer) {
		buttonsContainer->SetSize(width - 32, tabsHeight);

		TabList::iterator it = tabList.begin();
		for (it; it != tabList.end(); it++) {
			WindowElementButton* button = it->first;
			POINT size = button->GetSize();
			button->SetSize(size.x, tabsHeight);
		}
	}
	if (mainContainer) {
		mainContainer->SetSize(width, height - tabsHeight);
		mainContainer->SetPosition(0, tabsHeight);
	}
	if (buttonLeft)
		buttonLeft->SetSize(16, tabsHeight);
	if (buttonRight)
		buttonRight->SetSize(16, tabsHeight);
}

void WindowElementTab::SetWindow(Window* window) {
	if (buttonsContainer)
		buttonsContainer->SetWindow(window);
	if (mainContainer)
		mainContainer->SetWindow(window);

	this->window = window;
}

void WindowElementTab::SetSize(unsigned short wdh, unsigned short hgt) {
	int oldWidth = width;
	int oldHeight = height;
	POINT oldOffset = GetScrollOffset();

	width = wdh;
	height = hgt;

	POINT offset = GetScrollOffset();
	if (scrollH > (float)offset.x) scrollH = (float)offset.x;
	if (scrollV > (float)offset.y) scrollV = (float)offset.y;

	int resizeIntWidth = 0;
	int resizeIntHeight = 0;

	std::list<WindowElement*> additElements = elements;
	TabList::iterator tit = tabList.begin();
	for (tit; tit != tabList.end(); tit++) {
		WindowElementButton* button = tit->first;
		WindowElementContainer* container = tit->second;
		if (container && !container->parent)
			additElements.push_back(container);
	}

	std::list<WindowElement*>::iterator it;
	for (it = additElements.begin(); it != additElements.end(); it++) {
		int scrollWdh = (vAlwaysVisible ? 0 : (oldOffset.y && !offset.y ? wndTemplate->tempScroll.w : (!oldOffset.y && offset.y ? - wndTemplate->tempScroll.w : 0)));
		int scrollHgt = (hAlwaysVisible ? 0 : (oldOffset.x && !offset.x ? wndTemplate->tempScroll.h : (!oldOffset.x && offset.x ? - wndTemplate->tempScroll.h : 0)));

		if ((*it)->lockWidth || (*it)->lockHeight) {
			int newWidth = ((*it)->lockWidth ? (*it)->width + (wdh - oldWidth) + scrollWdh : (*it)->width);
			int newHeight = ((*it)->lockHeight ? (*it)->height + (hgt - oldHeight) + scrollHgt : (*it)->height);
			(*it)->SetSize(newWidth, newHeight);
		}
		if ((*it)->align & ALIGN_H_RIGHT) {
			(*it)->posX += (wdh - oldWidth) + scrollWdh;
		}
		if ((*it)->align & ALIGN_V_BOTTOM) {
			(*it)->posY += (hgt - oldHeight) + scrollHgt;
		}
	}
}

void WindowElementTab::SetBorder(int border) {
	this->border = border;

	if (buttonsContainer) {
		TabList::iterator it = tabList.begin();
		for (it; it != tabList.end(); it++) {
			WindowElementButton* button = it->first;
			button->SetBorder(border);
		}
	}
	if (buttonLeft)
		buttonLeft->SetBorder(border);
	if (buttonRight)
		buttonRight->SetBorder(border);
}

int WindowElementTab::GetTabsSize() {
	LOCKCLASS lockClass(lockTab);

	return tabList.size();
}

void WindowElementTab::UpdateButtonsPosition() {
	LOCKCLASS lockClass(lockTab);

	TabList::iterator it = tabList.begin();

	int position = 0;
	for (it; it != tabList.end(); it++) {
		WindowElementButton* button = it->first;
		button->SetPosition(position, 0);
		position += button->width;
	}

	buttonsContainer->SetIntSize(position, tabsHeight);
}

TabElement WindowElementTab::AddTab(std::string name, bool closeable) {
	LOCKCLASS lockClass(lockTab);

	int buttonWidth = 100;
	if (wndTemplate->font->GetTextWidth(name, 14) + 20 > 100) {
		int characters = wndTemplate->font->GetTextWidthCharNumber(name, 14, 80);
		std::string _name = name.substr(0, characters) + "...";
		name = _name;
	}

	WindowElementButton* button = new(M_PLACE) WindowElementButton;
	button->Create(0, BUTTON_TAB, 0, 0, buttonWidth, tabsHeight, wndTemplate);
	button->SetText(name);
	button->SetInverse(true);
	button->SetCloseIcon(closeable);
	button->SetBorder(border);
	button->SetFontSize(fontSize);
	button->SetColor(color.red, color.green, color.blue, color.alpha);

	int cwidth = mainContainer->width - (mainContainer->border ? wndTemplate->tempContainer.l_width + wndTemplate->tempContainer.r_width : 0);
	int cheight = mainContainer->height - (mainContainer->border ? wndTemplate->tempContainer.t_height + wndTemplate->tempContainer.b_height : 0);

	WindowElementContainer* container = new(M_PLACE) WindowElementContainer;
	container->Create(0, 0, 0, cwidth, cheight, false, false, wndTemplate);
	container->SetLocks(true, true);

	boost::function<void()> func = boost::bind(&WindowElementTab::SetActiveTab, this, button);
	button->SetAction(func);

	tabList.push_back(TabElement(button, container));

	buttonsContainer->AddElement(button);
	mainContainer->AddElement(container);

	SetActiveTab(button);

	UpdateButtonsPosition();

	return TabElement(button, container);
}

TabElement WindowElementTab::GetTab(WindowElementButton* button) {
	LOCKCLASS lockClass(lockTab);

	TabElement elem;
	elem.first = NULL;
	elem.second = NULL;

	TabList::iterator it = tabList.begin();
	for (it; it != tabList.end(); it++) {
		WindowElementButton* ibutton = it->first;
		WindowElementContainer* icontainer = it->second;
		if (ibutton == button) {
			elem = *it;
			return elem;
		}
	}

	return elem;
}

TabElement WindowElementTab::GetTab(int number) {
	LOCKCLASS lockClass(lockTab);

	TabElement elem;
	elem.first = NULL;
	elem.second = NULL;

	TabList::iterator it = tabList.begin();
	for (it; it != tabList.end() && number > 0; it++, number--);

	if (it != tabList.end())
		elem = *it;

	return elem;
}

TabElement WindowElementTab::GetActiveTab() {
	LOCKCLASS lockClass(lockTab);

	return activeTab;
}

TabElement* WindowElementTab::GetActiveTabPtr() {
	LOCKCLASS lockClass(lockTab);

	return &activeTab;
}

int WindowElementTab::GetActiveTabNumber() {
	LOCKCLASS lockClass(lockTab);

	int number = 1;

	TabList::iterator it = tabList.begin();
	for (it, number; it != tabList.end(); it++, number++) {
		WindowElementButton* button = it->first;
		if (activeTab.first == button)
			return number;
	}

	return 0;
}

void WindowElementTab::RemoveTab(WindowElementButton* button) {
	LOCKCLASS lockClass(lockTab);

	TabList::iterator it = tabList.begin();
	for (it; it != tabList.end(); it++) {
		WindowElementButton* ibutton = it->first;
		WindowElementContainer* icontainer = it->second;
		if (ibutton == button) {
			if (activeTab == *it && !MoveActiveTab(ibutton))
				break;

			buttonsContainer->RemoveElement(ibutton);
			delete_debug(ibutton, M_PLACE);
			delete_debug(icontainer, M_PLACE);
			tabList.erase(it);

			break;
		}
	}

	UpdateButtonsPosition();
}

void WindowElementTab::SetActiveTab(WindowElementButton* button) {
	LOCKCLASS lockClass(lockTab);

	TabList::iterator it = tabList.begin();
	for (it; it != tabList.end(); it++) {
		WindowElementButton* ibutton = it->first;
		WindowElementContainer* icontainer = it->second;
		if (ibutton == button) {
			activeTab = *it;
			ibutton->SetColor(NULL);
			ibutton->SetColor(color.red, color.green, color.blue, color.alpha);
			ibutton->SetPressed(true);
			mainContainer->RemoveAllElements();
			mainContainer->AddElement(icontainer);
		}
		else
			ibutton->SetPressed(false);
	}
}

void WindowElementTab::SetActiveTabByNumber(int number) {
	LOCKCLASS lockClass(lockTab);

	WindowElementButton* button = NULL;
	TabList::iterator it = tabList.begin();
	for (it, number; it != tabList.end() && number != 0; it++, number--)
		button = it->first;

	SetActiveTab(button);
}

bool WindowElementTab::MoveActiveTab(WindowElementButton* button) {
	LOCKCLASS lockClass(lockTab);

	TabList::iterator it = tabList.begin();
	TabList::iterator last_it = tabList.begin();
	for (it; it != tabList.end(); it++) {
		WindowElementButton* ibutton = it->first;
		if (ibutton == button) {
			if (it != tabList.begin())
				activeTab = *last_it;
			else {
				it++;
				if (it != tabList.end())
					activeTab = *it;
				else
					return false;
			}

			SetActiveTab(activeTab.first);
			break;
		}

		last_it = it;
	}

	return true;
}

void WindowElementTab::MoveTab(WindowElementButton* button, char dir) {
	LOCKCLASS lockClass(lockTab);

	TabList::iterator it = tabList.begin();
	TabList::iterator prev_it = it;
	TabList::iterator next_it = it;
	for (it; it != tabList.end(); it++) {
		next_it = it;
		next_it++;

		WindowElementButton* ibutton = it->first;
		if (ibutton == button) {
			if (dir < 0 && prev_it != it) {
				tabList.splice(prev_it, tabList, it);
			}
			else if (dir > 0 && next_it != tabList.end()) {
				tabList.splice(it, tabList, next_it);
			}

			break;
		}

		prev_it = it;
	}

	UpdateButtonsPosition();
}

void WindowElementTab::RenameTab(WindowElementButton* button, std::string name) {
	TabList::iterator it = tabList.begin();
	for (it; it != tabList.end(); it++) {
		WindowElementButton* ibutton = it->first;
		if (ibutton == button)
			break;
	}

	if (it == tabList.end())
		return;

	int buttonWidth = 100;
	if (wndTemplate->font->GetTextWidth(name, 14) + 20 > 100) {
		int characters = wndTemplate->font->GetTextWidthCharNumber(name, 14, 80);
		std::string _name = name.substr(0, characters) + "...";
		name = _name;
	}

	button->SetText(name);
}

void WindowElementTab::ScrollTab(char dir) {
	LOCKCLASS lockClass(lockTab);

	TabList::iterator it = tabList.begin();

	int position = 0;
	for (it; it != tabList.end(); it++) {
		WindowElementButton* button = it->first;

		position += button->width;
	}

	buttonsContainer->scrollH += dir;

	if (buttonsContainer->scrollH < 0 || position < buttonsContainer->width)
		buttonsContainer->scrollH = 0;
	else if (position > buttonsContainer->width && buttonsContainer->scrollH > position - buttonsContainer->width)
		buttonsContainer->scrollH = position - buttonsContainer->width;
}

void WindowElementTab::ClearTabs() {
	LOCKCLASS lockClass(lockTab);

	TabList::iterator it = tabList.begin();
	for (it; it != tabList.end(); it++) {
		WindowElementButton* ibutton = it->first;
		WindowElementContainer* icontainer = it->second;

		buttonsContainer->RemoveElement(ibutton);
		delete_debug(ibutton, M_PLACE);
		delete_debug(icontainer, M_PLACE);
	}
	tabList.clear();
}


// ---- WindowElementItemContainer ---- //

WindowElementItemContainer::WindowElementItemContainer() {
	image = NULL;
	item = NULL;
	creature = NULL;
	container = NULL;
	hotkey = NULL;
	slot = 0;
}

WindowElementItemContainer::~WindowElementItemContainer() {
	if (item)
		delete_debug(item, M_PLACE);
	if (creature)
		delete_debug(creature, M_PLACE);
	if (hotkey && hotkey->itemContainer)
		hotkey->itemContainer = NULL;
	else if (hotkey && !hotkey->itemContainer)
		delete_debug(hotkey, M_PLACE);
}

void WindowElementItemContainer::Create(unsigned char al, int pX, int pY, unsigned short wdh, unsigned short hgt, Container* container, WindowTemplate* wTemplate) {
	align = al;
	posX = pX;
	posY = pY;
	width = wdh;
	height = hgt;
	wndTemplate = wTemplate;

	border = 1;

	this->container = container;
}

void WindowElementItemContainer::SetImage(AD2D_Image* image) {
	this->image = image;
}

void WindowElementItemContainer::SetItem(Item* item) {
	if (this->item)
		delete_debug(this->item, M_PLACE);

	this->item = item;
}

void WindowElementItemContainer::SetCreature(Creature* creature) {
	this->creature = creature;
}

void WindowElementItemContainer::SetHotKey(HotKey* hotkey) {
	this->hotkey = hotkey;
	if (hotkey) {
		hotkey->itemContainer = this;
		if (hotkey->itemID != 0) {
			Item* item = new(M_PLACE) Item;
			item->SetID(hotkey->itemID);
			if (hotkey->fluid && (*item)() && (*item)()->fluid)
				item->SetCount(hotkey->fluid);
			else
				item->SetCount(0);
			this->SetItem(item);
		}
	}
}

Item* WindowElementItemContainer::GetItem() {
	return this->item;
}

Creature* WindowElementItemContainer::GetCreature() {
	return this->creature;
}

HotKey* WindowElementItemContainer::GetHotKey() {
	return this->hotkey;
}

void WindowElementItemContainer::SetSlot(unsigned short slot) {
	this->slot = slot;
}

unsigned short WindowElementItemContainer::GetSlot() {
	return this->slot;
}

Container* WindowElementItemContainer::GetContainer() {
	return this->container;
}

void WindowElementItemContainer::SetLookAction(boost::function<void()> f) {
	if (!f) {
		funcLook.Clear();
		return;
	}

	funcLook.PushFunction(f);
}

void WindowElementItemContainer::SetUseAction(boost::function<void()> f) {
	if (!f) {
		funcUse.Clear();
		return;
	}

	funcUse.PushFunction(f);
}

void WindowElementItemContainer::OpenMenu(Mouse& mouse, Keyboard& keyboard, std::list<MenuData>& actionList) {
	Item* _item = NULL;
	if (item)
		_item = item;
	else if (container)
		_item = container->GetItem(slot);

	Window* window = GetWindow();
	if (window && window->GetWindowType() == WND_HOTKEYS) {
		HotKey* hotkey = (HotKey*)this->hotkey;
		unsigned char slot = GetSlot();

		if (hotkey->text != "" || image || (_item && (*_item)()))
			actionList.push_back(MenuData(ACTION_SETHOTKEY, Text::GetText("ACTION_SETHOTKEY", Game::options.language), (void*)(unsigned int)slot));

		if (actionList.size() > 0)
			actionList.push_back(MenuData(0x00, "-", NULL));

		actionList.push_back(MenuData(ACTION_SETHOTKEYTEXT, Text::GetText("ACTION_SETHOTKEYTEXT", Game::options.language), (void*)(unsigned int)slot));

		if ((_item && (*_item)())) {
			actionList.push_back(MenuData(0x00, "-", NULL));
			actionList.push_back(MenuData(ACTION_SETHOTKEYMODE1, Text::GetText("ACTION_SETHOTKEYMODE1", Game::options.language), (void*)(unsigned int)slot));
			actionList.push_back(MenuData(ACTION_SETHOTKEYMODE2, Text::GetText("ACTION_SETHOTKEYMODE2", Game::options.language), (void*)(unsigned int)slot));
			actionList.push_back(MenuData(ACTION_SETHOTKEYMODE3, Text::GetText("ACTION_SETHOTKEYMODE3", Game::options.language), (void*)(unsigned int)slot));
		}
		else if (hotkey->text != "") {
			actionList.push_back(MenuData(0x00, "-", NULL));
			actionList.push_back(MenuData(ACTION_SETHOTKEYMODE4, Text::GetText("ACTION_SETHOTKEYMODE4", Game::options.language), (void*)(unsigned int)slot));
			actionList.push_back(MenuData(ACTION_SETHOTKEYMODE5, Text::GetText("ACTION_SETHOTKEYMODE5", Game::options.language), (void*)(unsigned int)slot));
		}

		if (hotkey->text != "" || (_item && (*_item)())) {
			actionList.push_back(MenuData(0x00, "-", NULL));
			actionList.push_back(MenuData(ACTION_CLEARHOTKEY, Text::GetText("ACTION_CLEARHOTKEY", Game::options.language), (void*)(unsigned int)slot));
		}
	}
	else if (window && window->GetWindowType() == WND_SHOP) {
		if (_item && (*_item)()) {
			ThingData thing;
			thing.fromPos = Position(0, 0, 0);
			thing.toPos = Position(0, 0, 0);
			thing.fromStackPos = 0;
			thing.itemId = _item->GetID();
			thing.count = _item->GetCount();

			actionList.push_back(MenuData(ACTION_LOOK, Text::GetText("ACTION_LOOK", Game::options.language), (void*)new(M_PLACE) ThingData(&thing)));
		}
	}
	else if (window && window->GetWindowType() == WND_BOT) {
		if (_item && (*_item)()) {
			ThingData thing;
			thing.scroll = (unsigned char*)this;

			actionList.push_back(MenuData(ACTION_CLEARFREE, Text::GetText("ACTION_CLEARFREE", Game::options.language), (void*)new(M_PLACE) ThingData(&thing)));
		}
	}
	else if (_item && (*_item)()) {
		ThingData thing;
		if (container->GetIndex() != CONTAINER_TRADE && container->GetIndex() != CONTAINER_INVENTORY) {
			thing.fromPos = Position(0xFFFF, 64 + container->GetIndex(), slot);
			thing.toPos = Position(0, 0, 0);
			thing.scroll = (unsigned char*)(unsigned int)container->GetIndex();
		}
		else if (container->GetIndex() == CONTAINER_INVENTORY) {
			thing.fromPos = Position(0xFFFF, slot + 1, 0);
			thing.toPos = Position(0, 0, 0);
			thing.scroll = (unsigned char*)(unsigned int)Container::GetFreeIndex();
		}
		else if (container->GetIndex() == CONTAINER_TRADE) {
			thing.fromPos = Position(0, 0, 0);
			thing.toPos = Position(0xFFFF, container->GetID(), slot);
			thing.scroll = NULL;
		}
		thing.fromStackPos = 0;
		thing.itemId = _item->GetID();
		thing.count = _item->GetCount();

		if (container->GetIndex() != CONTAINER_TRADE) {
			actionList.push_back(MenuData(ACTION_LOOK, Text::GetText("ACTION_LOOK", Game::options.language), (void*)new(M_PLACE) ThingData(&thing)));
			if (!(*_item)()->container) {
				if (!(*_item)()->useable)
					actionList.push_back(MenuData(ACTION_USE, Text::GetText("ACTION_USE", Game::options.language), (void*)new(M_PLACE) ThingData(&thing)));
				else
					actionList.push_back(MenuData(ACTION_USEWITH, Text::GetText("ACTION_USEWITH", Game::options.language), (void*)new(M_PLACE) ThingData(&thing)));
			}
			else {
				actionList.push_back(MenuData(ACTION_OPEN, Text::GetText("ACTION_OPEN", Game::options.language), (void*)new(M_PLACE) ThingData(&thing)));
				actionList.push_back(MenuData(ACTION_OPENNEW, Text::GetText("ACTION_OPENNEW", Game::options.language), (void*)new(M_PLACE) ThingData(&thing)));
			}
			if ((*_item)()->rotateable)
				actionList.push_back(MenuData(ACTION_ROTATE, Text::GetText("ACTION_ROTATE", Game::options.language), (void*)new(M_PLACE) ThingData(&thing)));
			if ((*_item)()->pickupable)
				actionList.push_back(MenuData(ACTION_TRADEWITH, Text::GetText("ACTION_TRADEWITH", Game::options.language), (void*)new(M_PLACE) ThingData(&thing)));
		}
		else
			actionList.push_back(MenuData(ACTION_LOOK, Text::GetText("ACTION_LOOK", Game::options.language), (void*)new(M_PLACE) ThingData(&thing)));
	}
}

void WindowElementItemContainer::Print(POINT& d, AD2D_Window& gfx) {
	AD2D_Viewport currentVP = gfx.GetCurrentViewport();
	AD2D_Viewport nextVP;

	POINT scroll = GetParentScrollOffset();
	int offsetH = scroll.x;
	int offsetV = scroll.y;

	nextVP.startX = currentVP.startX + posX - offsetH - d.x;
	nextVP.startY = currentVP.startY + posY - offsetV - d.y;
	nextVP.width = width;
	nextVP.height = height;

	POINT diff = AdjustViewport(currentVP, nextVP);

	gfx.SetViewport(nextVP);

	COLOR currentColor = AD2D_Window::GetColor();

	if (wndTemplate->GetElements() & ELEMENT_ITEMCONTAINER) {
		int x = 0;
		int y = 0;

		POINT diff;
		AD2D_Viewport textareaVP;

		textareaVP.Create(	currentVP.startX + posX + wndTemplate->tempItem.l_width - offsetH - d.x,
							currentVP.startY + posY + wndTemplate->tempItem.t_height - offsetV - d.y,
							currentVP.startX + posX + width - wndTemplate->tempItem.r_width - offsetH - d.x - 1,
							currentVP.startY + posY + height - wndTemplate->tempItem.b_height - offsetV - d.y - 1);
		diff = AdjustViewport(nextVP, textareaVP);
		gfx.SetViewport(textareaVP);
		if (wndTemplate->tempItem.bg.GetWidth() > 0) {
			for (x = 0; x < width - wndTemplate->tempItem.l_width - wndTemplate->tempItem.r_width; x += wndTemplate->tempItem.bg.GetWidth())
			for (y = 0; y < height - wndTemplate->tempItem.t_height - wndTemplate->tempItem.b_height; y += wndTemplate->tempItem.bg.GetHeight())
				gfx.PutImage(x - diff.x, y - diff.y, wndTemplate->tempItem.bg);
		}

		textareaVP.Create(	currentVP.startX + posX + wndTemplate->tempItem.l_width - offsetH - d.x,
							currentVP.startY + posY - offsetV - d.y,
							currentVP.startX + posX + width - wndTemplate->tempItem.r_width - offsetH - d.x - 1,
							currentVP.startY + posY + wndTemplate->tempItem.t.GetHeight() - offsetV - d.y - 1);
		diff = AdjustViewport(nextVP, textareaVP);
		gfx.SetViewport(textareaVP);
		if (wndTemplate->tempItem.t.GetWidth() > 0) {
			for (x = 0; x < width - wndTemplate->tempItem.l_width - wndTemplate->tempItem.r_width; x += wndTemplate->tempItem.t.GetWidth())
				gfx.PutImage(x - diff.x, - diff.y, wndTemplate->tempItem.t);
		}

		textareaVP.Create(	currentVP.startX + posX - offsetH - d.x,
							currentVP.startY + posY + wndTemplate->tempItem.t_height - offsetV - d.y,
							currentVP.startX + posX + wndTemplate->tempItem.l.GetWidth() - offsetH - d.x - 1,
							currentVP.startY + posY + height - wndTemplate->tempItem.b_height - offsetV - d.y - 1);
		diff = AdjustViewport(nextVP, textareaVP);
		gfx.SetViewport(textareaVP);
		if (wndTemplate->tempItem.l.GetWidth() > 0) {
			for (y = 0; y < height - wndTemplate->tempItem.t_height - wndTemplate->tempItem.b_height; y += wndTemplate->tempItem.l.GetHeight())
				gfx.PutImage(- diff.x, y - diff.y, wndTemplate->tempItem.l);
		}

		textareaVP.Create(	currentVP.startX + posX + width - wndTemplate->tempItem.r.GetWidth() - offsetH - d.x,
							currentVP.startY + posY + wndTemplate->tempItem.t_height - offsetV - d.y,
							currentVP.startX + posX + width - offsetH - d.x - 1,
							currentVP.startY + posY + height - wndTemplate->tempItem.b_height - offsetV - d.y - 1);
		diff = AdjustViewport(nextVP, textareaVP);
		gfx.SetViewport(textareaVP);
		if (wndTemplate->tempItem.r.GetWidth() > 0) {
			for (y = 0; y < height - wndTemplate->tempItem.t_height - wndTemplate->tempItem.b_height; y += wndTemplate->tempItem.r.GetHeight())
				gfx.PutImage(- diff.x, y - diff.y, wndTemplate->tempItem.r);
		}

		textareaVP.Create(	currentVP.startX + posX + wndTemplate->tempItem.l_width - offsetH - d.x,
							currentVP.startY + posY + height - wndTemplate->tempItem.b.GetHeight() - offsetV - d.y,
							currentVP.startX + posX + width - wndTemplate->tempItem.r_width - offsetH - d.x - 1,
							currentVP.startY + posY + height - offsetV - d.y - 1);
		diff = AdjustViewport(nextVP, textareaVP);
		gfx.SetViewport(textareaVP);
		if (wndTemplate->tempItem.b.GetWidth() > 0) {
			for (x = 0; x < width - wndTemplate->tempItem.l_width - wndTemplate->tempItem.r_width; x += wndTemplate->tempItem.b.GetWidth())
				gfx.PutImage(x - diff.x, - diff.y, wndTemplate->tempItem.b);
		}

		textareaVP.Create(	currentVP.startX + posX - offsetH - d.x,
							currentVP.startY + posY - offsetV - d.y,
							currentVP.startX + posX + width - offsetH - d.x - 1,
							currentVP.startY + posY + height - offsetV - d.y - 1);
		diff = AdjustViewport(nextVP, textareaVP);
		gfx.SetViewport(textareaVP);
		gfx.PutImage(- diff.x, - diff.y, wndTemplate->tempItem.tl);
		gfx.PutImage(width - wndTemplate->tempItem.tr.GetWidth() - diff.x, - diff.y, wndTemplate->tempItem.tr);
		gfx.PutImage(- diff.x, height - wndTemplate->tempItem.bl.GetHeight() - diff.y, wndTemplate->tempItem.bl);
		gfx.PutImage(width - wndTemplate->tempItem.br.GetWidth() - diff.x, height - wndTemplate->tempItem.br.GetHeight() - diff.y, wndTemplate->tempItem.br);
	}

	AD2D_Window::SetColor(currentColor.red, currentColor.green, currentColor.blue, currentColor.alpha);

	if (image) {
		int px = (width - image->GetWidth()) / 2;
		int py = (height - image->GetHeight()) / 2;
		gfx.PutImage(px - diff.x, py - diff.y, *image);
	}

	Item* _item = NULL;
	if (item)
		_item = item;
	else if (container)
		_item = container->GetItem(slot);

	if (_item) {
		if (_item->GetShine())
			_item->PrintItemShine(&gfx, Position(0, 0, 0), - (float)diff.x, - (float)diff.y, (float)width, (float)height, 0, _item->GetShine());

		_item->PrintItem(&gfx, Position(0, 0, 0), - (float)diff.x, - (float)diff.y, (float)width, (float)height, 0);
		if ((*_item)() && (*_item)()->stackable && _item->GetCount() > 0) {
			std::string count;
			count = value2str(_item->GetCount());
			gfx.PPrint(width - wndTemplate->font->GetTextWidth(count, 12) - diff.x, height - 12 - diff.y, 12, *wndTemplate->font, count.c_str(), border);
		}
	}

	if (creature) {
		Outfit outfit = creature->GetOutfit();
		int lookType = (outfit.lookType != 0 ? Item::GetItemsCount() + outfit.lookType : outfit.lookTypeEx);

		float xOffset = 0.0f;
		float yOffset = 0.0f;

		bool attacked = (Player::GetAttackID() != 0 && creature->GetID() == Player::GetAttackID() ? true : false);
		bool followed = (Player::GetFollowID() != 0 && creature->GetID() == Player::GetFollowID() ? true : false);
		bool selected = (Player::GetSelectID() != 0 && creature->GetID() == Player::GetSelectID() ? true : false);

		ItemType* iType = (*creature)();
		if (iType) {
			int div = (iType->m_height > iType->m_width ? iType->m_height : iType->m_width);
			float factor = (iType->m_skip > 0 ? 32.0f / (float)iType->m_skip : 1.0f);
			float iwidth = (float)width * factor;
			float iheight = (float)height * factor;

			if (iType->hasOffset) {
				xOffset = (float)(iType->xOffset) * (iwidth / 32);
				yOffset = (float)(iType->yOffset) * (iheight / 32);
			}

			creature->PrintCreature(&gfx, Position(0, 0, 0), xOffset + ((float)width - iwidth) - (float)diff.x, yOffset + ((float)height - iheight) - (float)diff.y, iwidth, iheight);
		}
		creature->PrintDetails(&gfx, wndTemplate->font, Position(0, 0, 0), 0, 0, width, height, -1, -1, -1, attacked, followed, selected);
	}

	if (hotkey) {
		if (hotkey->itemID) {
			std::string text;
			if (hotkey->mode == 1) text = "Y";
			else if (hotkey->mode == 2) text = "T";
			else if (hotkey->mode == 3) text = "C";
			gfx.PPrint( - diff.x, height - 12 - diff.y, 12, *wndTemplate->font, text.c_str(), border);
		}

		if (hotkey->spellID) {
			AD2D_Image* image = Icons::GetSpellIcon(hotkey->spellID);
			if (image) {
				gfx.PutImage( - diff.x, - diff.y, - diff.x + width, - diff.y + height, *image);
			}

			std::string text = "ID: " + value2str(hotkey->spellID);
			gfx.PPrint( - diff.x, - diff.y, 12, *wndTemplate->font, text.c_str(), border);
		}

		if (!hotkey->itemID && hotkey->text != "") {
			std::string text;
			if (hotkey->mode == 0) text = "abc";
			else if (hotkey->mode == 1) text = "C abc";
			else if (hotkey->mode == 2) text = "I abc";
			gfx.PPrint( - diff.x, height - 12 - diff.y, 12, *wndTemplate->font, text.c_str(), border);
		}
	}

	AD2D_Window::SetColor(currentColor.red, currentColor.green, currentColor.blue, currentColor.alpha);

	gfx.SetViewport(currentVP);
}


// ---- WindowElementCheckBox ---- //

WindowElementCheckBox::WindowElementCheckBox() { }

WindowElementCheckBox::~WindowElementCheckBox() { }

void WindowElementCheckBox::Create(unsigned char al, int pX, int pY, unsigned short wdh, WindowTemplate* wTemplate) {
	align = al;
	posX = pX;
	posY = pY;
	wndTemplate = wTemplate;

	fontSize = 14;

	width = std::max(wndTemplate->tempCheckBox.width, (int)wdh);
	height = wndTemplate->tempCheckBox.height;

	SetColor(1.0f, 1.0f, 1.0f, 1.0f);

	checked = false;
}

void WindowElementCheckBox::SetColor(float r, float g, float b, float a) {
	color.red = r;
	color.green = g;
	color.blue = b;
	color.alpha = a;
}

void WindowElementCheckBox::SetState(bool checked) {
	this->checked = checked;
}

void WindowElementCheckBox::SetText(std::string txt) {
	this->text = txt;
}

bool WindowElementCheckBox::GetState() {
	return this->checked;
}

std::string WindowElementCheckBox::GetText() {
	return this->text;
}

void WindowElementCheckBox::Print(POINT& d, AD2D_Window& gfx) {
	AD2D_Viewport currentVP = gfx.GetCurrentViewport();
	AD2D_Viewport nextVP;

	POINT scroll = GetParentScrollOffset();
	int offsetH = scroll.x;
	int offsetV = scroll.y;

	nextVP.startX = currentVP.startX + posX - offsetH - d.x;
	nextVP.startY = currentVP.startY + posY - offsetV - d.y;
	nextVP.width = width;
	nextVP.height = height;

	POINT diff = AdjustViewport(currentVP, nextVP);

	gfx.SetViewport(nextVP);

	COLOR currentColor = AD2D_Window::GetColor();

	float factor = 1.0f;
	if (!enabled)
		factor = 0.5f;

	if (enabled && checked)
		gfx.PutImage( - diff.x, - diff.y, wndTemplate->tempCheckBox.ch);
	else
		gfx.PutImage( - diff.x, - diff.y, wndTemplate->tempCheckBox.uch);

	int offset = (wndTemplate->tempCheckBox.height - fontSize) / 2;
	AD2D_Window::SetColor(currentColor.red * color.red * factor, currentColor.green * color.green * factor, currentColor.blue * color.blue * factor, currentColor.alpha * color.alpha);
	gfx.Print(wndTemplate->tempCheckBox.text_offset - diff.x, offset - diff.y, fontSize, *wndTemplate->font, text.c_str(), border);
	AD2D_Window::SetColor(currentColor.red, currentColor.green, currentColor.blue, currentColor.alpha);

	gfx.SetViewport(currentVP);
}


// ---- WindowElementScrollBar ---- //

WindowElementScrollBar::WindowElementScrollBar() { }

WindowElementScrollBar::~WindowElementScrollBar() { }

void WindowElementScrollBar::Create(unsigned char al, int pX, int pY, unsigned short wdh, WindowTemplate* wTemplate) {
	align = al;
	posX = pX;
	posY = pY;
	wndTemplate = wTemplate;

	minValue = 0.0f;
	maxValue = 1.0f;

	valueStr = "";

	width = wdh;
	height = wndTemplate->tempScrollBar.height;
}


void WindowElementScrollBar::SetValuePtr(TypePointer pointer) {
	value = pointer;
}

TypePointer WindowElementScrollBar::GetValuePtr() {
	return value;
}

void WindowElementScrollBar::SetValue(float value) {
	if (this->value.type == "" || !this->value.ptr)
		return;

	if (minValue > maxValue)
		return;

	if (value < minValue) value = minValue;
	else if (value > maxValue) value = maxValue;

	this->value.SetValue(float2str(value, 6));

	valueStr = "";
}

void WindowElementScrollBar::SetMinValue(float minValue) {
	this->minValue = minValue;
}

void WindowElementScrollBar::SetMaxValue(float maxValue) {
	this->maxValue = maxValue;
}

float WindowElementScrollBar::GetValue() {
	float ret = atof(value.GetValue().c_str());
	if (ret < minValue)
		ret = minValue;
	else if (ret > maxValue)
		ret = maxValue;
	return ret;
}

float WindowElementScrollBar::GetMinValue() {
	return minValue;
}

float WindowElementScrollBar::GetMaxValue() {
	return maxValue;
}

void WindowElementScrollBar::CheckInput(Keyboard& keyboard) {
	std::string textValue = value.GetValue(6);

	if (keyboard.keyChar == 8) {
		if (valueStr.length() > 0)
			valueStr.erase(valueStr.length() - 1);
		value.SetValue(valueStr);
		keyboard.keyChar = 0;
	}
	else if (keyboard.keyChar >= '0' && keyboard.keyChar <= '9') {
		valueStr.push_back(keyboard.keyChar);
		if (atof(valueStr.c_str()) > maxValue)
			valueStr = float2str(maxValue, 0);
		value.SetValue(valueStr);
		keyboard.keyChar = 0;
	}
	else if (keyboard.keyChar == 255 + VK_UP) {
		SetValue(GetMaxValue());
		keyboard.keyChar = 0;
	}
	else if (keyboard.keyChar == 255 + VK_DOWN) {
		SetValue(GetMinValue());
		keyboard.keyChar = 0;
	}
	else if (keyboard.keyChar == 255 + VK_LEFT) {
		SetValue(GetValue() - 1.0f);
		keyboard.keyChar = 0;
	}
	else if (keyboard.keyChar == 255 + VK_RIGHT) {
		SetValue(GetValue() + 1.0f);
		keyboard.keyChar = 0;
	}
}

void WindowElementScrollBar::Print(POINT& d, AD2D_Window& gfx) {
	AD2D_Viewport currentVP = gfx.GetCurrentViewport();
	AD2D_Viewport nextVP;

	POINT scroll = GetParentScrollOffset();
	int offsetH = scroll.x;
	int offsetV = scroll.y;

	nextVP.startX = currentVP.startX + posX - offsetH - d.x;
	nextVP.startY = currentVP.startY + posY - offsetV - d.y;
	nextVP.width = width;
	nextVP.height = height;

	POINT diff = AdjustViewport(currentVP, nextVP);

	gfx.SetViewport(nextVP);

	if (wndTemplate->GetElements() & ELEMENT_SCROLLBAR) {
		POINT diff;
		AD2D_Viewport scrollVP;

		scrollVP.Create(	currentVP.startX + posX - offsetH - d.x,
							currentVP.startY + posY - offsetV - d.y,
							currentVP.startX + posX + wndTemplate->tempScrollBar.l.GetWidth() - offsetH - d.x - 1,
							currentVP.startY + posY + height - offsetV - d.y - 1);
		diff = AdjustViewport(nextVP, scrollVP);
		gfx.SetViewport(scrollVP);
		gfx.PutImage( - diff.x, - diff.y, wndTemplate->tempScrollBar.l);

		scrollVP.Create(	currentVP.startX + posX + wndTemplate->tempScrollBar.l.GetWidth() - offsetH - d.x,
							currentVP.startY + posY - offsetV - d.y,
							currentVP.startX + posX + width - wndTemplate->tempScrollBar.r.GetWidth() - offsetH - d.x - 1,
							currentVP.startY + posY + height - offsetV - d.y - 1);
		diff = AdjustViewport(nextVP, scrollVP);
		gfx.SetViewport(scrollVP);
		int offset = (wndTemplate->tempScrollBar.height - wndTemplate->tempScrollBar.m.GetHeight()) / 2;
		if (wndTemplate->tempScrollBar.m.GetWidth() > 0) {
			for (int x = 0; x < width - wndTemplate->tempScrollBar.l.GetWidth() - wndTemplate->tempScrollBar.r.GetWidth(); x += wndTemplate->tempScrollBar.m.GetWidth())
				gfx.PutImage(x - diff.x, offset - diff.y, wndTemplate->tempScrollBar.m);
		}

		scrollVP.Create(	currentVP.startX + posX + width - wndTemplate->tempScrollBar.r.GetWidth() - offsetH - d.x,
							currentVP.startY + posY - offsetV - d.y,
							currentVP.startX + posX + width - offsetH - d.x - 1,
							currentVP.startY + posY + height - offsetV - d.y - 1);
		diff = AdjustViewport(nextVP, scrollVP);
		gfx.SetViewport(scrollVP);
		gfx.PutImage( - diff.x, - diff.y, wndTemplate->tempScrollBar.r);
	}

	gfx.SetViewport(nextVP);

	if (value.ptr && minValue < maxValue) {
		int offsetX = (int)((GetValue() - GetMinValue()) / (GetMaxValue() - GetMinValue()) * (width - wndTemplate->tempScrollBar.l_width - wndTemplate->tempScrollBar.r_width - wndTemplate->tempScrollBar.br.GetWidth()));
		int offsetY = (wndTemplate->tempScrollBar.height - wndTemplate->tempScrollBar.br.GetHeight()) / 2;
		gfx.PutImage(wndTemplate->tempScrollBar.l_width + offsetX - diff.x, offsetY - diff.y, wndTemplate->tempScrollBar.br);
	}

	gfx.SetViewport(currentVP);
}


// ---- WindowElementHealthBar ---- //

WindowElementHealthBar::WindowElementHealthBar() {
	value = NULL;
	maxValue = NULL;
}

WindowElementHealthBar::~WindowElementHealthBar() { }

void WindowElementHealthBar::Create(unsigned char al, int pX, int pY, int fSize, unsigned short* valuePtr, unsigned short* maxValuePtr, WindowTemplate* wTemplate) {
	align = al;
	posX = pX;
	posY = pY;
	wndTemplate = wTemplate;

	fontSize = fSize;

	value = valuePtr;
	maxValue = maxValuePtr;

	width = wndTemplate->tempHealthBar.width;
	height = wndTemplate->tempHealthBar.height;
}

void WindowElementHealthBar::Print(POINT& d, AD2D_Window& gfx) {
	AD2D_Viewport currentVP = gfx.GetCurrentViewport();
	AD2D_Viewport nextVP;
	AD2D_Viewport barVP;

	POINT scroll = GetParentScrollOffset();
	int offsetH = scroll.x;
	int offsetV = scroll.y;

	nextVP.startX = currentVP.startX + posX - offsetH - d.x;
	nextVP.startY = currentVP.startY + posY - offsetV - d.y;
	nextVP.width = width;
	nextVP.height = height;

	POINT diff = AdjustViewport(currentVP, nextVP);

	gfx.SetViewport(nextVP);

	COLOR currentColor = AD2D_Window::GetColor();

	gfx.PutImage(- diff.x, - diff.y, wndTemplate->tempHealthBar.empty);

	int new_width = (int)((float)width * (float)(*value)/(*maxValue));

	barVP.startX = currentVP.startX + posX - offsetH - d.x;
	barVP.startY = currentVP.startY + posY - offsetV - d.y;
	barVP.width = new_width;
	barVP.height = height;

	diff = AdjustViewport(currentVP, barVP);

	gfx.SetViewport(barVP);
	gfx.PutImage(- diff.x, - diff.y, wndTemplate->tempHealthBar.bar);

	std::stringstream ss;
	ss << (int)(*value) << "/" << (int)(*maxValue);
	float txt_width = wndTemplate->font->GetTextWidth(ss.str(), fontSize);
	float tx = width / 2 - txt_width / 2;
	float ty = height / 2 - fontSize / 2;

	gfx.SetViewport(nextVP);
	AD2D_Window::SetColor(currentColor.red * 1.0f, currentColor.green * 0.7f, currentColor.blue * 0.7f, currentColor.alpha);
	gfx.Print(- diff.x + tx, - diff.y + ty, fontSize, *wndTemplate->font, ss.str().c_str(), border);

	AD2D_Window::SetColor(currentColor.red, currentColor.green, currentColor.blue, currentColor.alpha);

	gfx.SetViewport(currentVP);
}


// ---- WindowElementManaBar ---- //

WindowElementManaBar::WindowElementManaBar() {
	value = NULL;
	maxValue = NULL;
}

WindowElementManaBar::~WindowElementManaBar() { }

void WindowElementManaBar::Create(unsigned char al, int pX, int pY, int fSize, unsigned short* valuePtr, unsigned short* maxValuePtr, WindowTemplate* wTemplate) {
	align = al;
	posX = pX;
	posY = pY;
	wndTemplate = wTemplate;

	fontSize = fSize;

	value = valuePtr;
	maxValue = maxValuePtr;

	width = wndTemplate->tempManaBar.width;
	height = wndTemplate->tempManaBar.height;
}

void WindowElementManaBar::Print(POINT& d, AD2D_Window& gfx) {
	AD2D_Viewport currentVP = gfx.GetCurrentViewport();
	AD2D_Viewport nextVP;
	AD2D_Viewport barVP;

	POINT scroll = GetParentScrollOffset();
	int offsetH = scroll.x;
	int offsetV = scroll.y;

	nextVP.startX = currentVP.startX + posX - offsetH - d.x;
	nextVP.startY = currentVP.startY + posY - offsetV - d.y;
	nextVP.width = width;
	nextVP.height = height;

	POINT diff = AdjustViewport(currentVP, nextVP);

	gfx.SetViewport(nextVP);

	COLOR currentColor = AD2D_Window::GetColor();

	gfx.PutImage(- diff.x, - diff.y, wndTemplate->tempManaBar.empty);

	int new_width = (int)((float)width * (float)(*value)/(*maxValue));

	barVP.startX = currentVP.startX + posX - offsetH - d.x;
	barVP.startY = currentVP.startY + posY - offsetV - d.y;
	barVP.width = new_width;
	barVP.height = height;

	diff = AdjustViewport(currentVP, nextVP);

	gfx.SetViewport(barVP);
	gfx.PutImage(- diff.x, - diff.y, wndTemplate->tempManaBar.bar);

	std::stringstream ss;
	ss << (int)(*value) << "/" << (int)(*maxValue);
	float txt_width = wndTemplate->font->GetTextWidth(ss.str(), fontSize);
	float tx = width / 2 - txt_width / 2;
	float ty = height / 2 - fontSize / 2;

	gfx.SetViewport(nextVP);
	AD2D_Window::SetColor(currentColor.red * 0.7f, currentColor.green * 0.7f, currentColor.blue * 1.0f, currentColor.alpha);
	gfx.Print(- diff.x + tx, - diff.y + ty, fontSize, *wndTemplate->font, ss.str().c_str(), border);

	AD2D_Window::SetColor(currentColor.red, currentColor.green, currentColor.blue, currentColor.alpha);

	gfx.SetViewport(currentVP);
}


// ---- WindowElementSkillBar ---- //

WindowElementSkillBar::WindowElementSkillBar() { }

WindowElementSkillBar::~WindowElementSkillBar() { }

void WindowElementSkillBar::Create(unsigned char al, int pX, int pY, int wdh, int hgt, TypePointer valuePtr, WindowTemplate* wTemplate) {
	align = al;
	posX = pX;
	posY = pY;
	wndTemplate = wTemplate;

	value = valuePtr;

	width = wdh;
	height = hgt;

	minValue = 0;
	maxValue = 100;

	color = COLOR(1.0f, 1.0f, 1.0f, 1.0f);
}

void WindowElementSkillBar::SetColor(float r, float g, float b, float a) {
	color.red = r;
	color.green = g;
	color.blue = b;
	color.alpha = a;
}

void WindowElementSkillBar::SetColor(boost::function<COLOR()> func) {
	colorFunc = func;
}

void WindowElementSkillBar::SetValuePtr(TypePointer pointer) {
	value = pointer;
}

TypePointer WindowElementSkillBar::GetValuePtr() {
	return value;
}

void WindowElementSkillBar::SetMinValue(float value) {
	minValue = value;
}

void WindowElementSkillBar::SetMaxValue(float value) {
	maxValue = value;
}

std::string WindowElementSkillBar::GetComment() {
	float a = atof(value.GetValue().c_str()) - minValue;
	float b = maxValue - minValue;

	int percent = (int)(100 - 100 * (b > 0 ? (float)(a/b) : 0.0f));

	std::string cmt = comment;
	cmt += (comment != "" ? "\n" : "") + value2str(percent) + "% " + Text::GetText("COMMENT_9", Game::options.language);
	return cmt;
}

void WindowElementSkillBar::Print(POINT& d, AD2D_Window& gfx) {
	AD2D_Viewport currentVP = gfx.GetCurrentViewport();
	AD2D_Viewport nextVP;

	POINT scroll = GetParentScrollOffset();
	int offsetH = scroll.x;
	int offsetV = scroll.y;

	nextVP.startX = currentVP.startX + posX - offsetH - d.x;
	nextVP.startY = currentVP.startY + posY - offsetV - d.y;
	nextVP.width = width;
	nextVP.height = height;

	POINT diff = AdjustViewport(currentVP, nextVP);

	gfx.SetViewport(nextVP);

	COLOR currentColor = AD2D_Window::GetColor();

	gfx.PutImage(- diff.x, - diff.y, width - diff.x, height - diff.y, wndTemplate->tempSkillBar.empty);

	int new_width = (minValue >= maxValue ? width : 0);
	if (value.ptr && minValue < maxValue)
		new_width = (int)((float)width * (atof(value.GetValue().c_str()) - minValue) / (maxValue - minValue));

	nextVP.startX = currentVP.startX + posX - offsetH - d.x;
	nextVP.startY = currentVP.startY + posY - offsetV - d.y;
	nextVP.width = new_width;
	nextVP.height = height;

	diff = AdjustViewport(currentVP, nextVP);
	gfx.SetViewport(nextVP);

	if (colorFunc)
		color = colorFunc();

	AD2D_Window::SetColor(color.red * currentColor.red, color.green * currentColor.green, color.blue * currentColor.blue, color.alpha * currentColor.alpha);
	gfx.PutImage(- diff.x, - diff.y, width - diff.x, height - diff.y, wndTemplate->tempSkillBar.bar);
	AD2D_Window::SetColor(currentColor.red, currentColor.green, currentColor.blue, currentColor.alpha);

	gfx.SetViewport(currentVP);
}


// ---- WindowElementPlayground ---- //

WindowElementPlayground::WindowElementPlayground() {
	game = NULL;
}

WindowElementPlayground::~WindowElementPlayground() { }

void WindowElementPlayground::Create(unsigned char al, int pX, int pY, unsigned short wdh, unsigned short hgt, Game* gm, WindowTemplate* wTemplate) {
	align = al;
	posX = pX;
	posY = pY;
	width = wdh;
	height = hgt;
	game = gm;
	wndTemplate = wTemplate;

	SetSize(wdh, hgt);
}

void WindowElementPlayground::SetSize(unsigned short wdh, unsigned short hgt) {
	if (!window)
		return;

	POINT wndSize = window->GetSize(true);
	wdh = wndSize.x;
	hgt = wndSize.y;

	if (Game::options.dontStrech) {
		width = 15 * 32;
		height = 11 * 32;
	}
	else if ((float)wdh / hgt < 15.0f / 11.0f) {
		width = wdh;
		height = (int)((float)((float)wdh / 15) * 11);
	}
	else if ((float)wdh / hgt > 15.0f / 11.0f) {
		height = hgt;
		width = (int)((float)((float)hgt / 11) * 15);
	}
	else {
		width = wdh;
		height = hgt;
	}

	int newPosX = ((int)wndSize.x - width) / 2;
	int newPosY = ((int)wndSize.y - height) / 2;
	SetPosition(newPosX, newPosY);
}

void WindowElementPlayground::OpenMenu(Mouse& mouse, Keyboard& keyboard, std::list<MenuData>& actionList) {
	POINT pos = GetAbsolutePosition(true);
	int x = (int)((float)(mouse.curX - pos.x) / ((float)width / 15));
	int y = (int)((float)(mouse.curY - pos.y) / ((float)height / 11));

	Player* player = game->GetPlayer();
	Map* map = game->GetMap();
	if (map) {
		POINT zPos = map->GetZ();
		Tile* tile = map->GetTopTile(x + 1, y + 1, zPos.x, zPos.y);
		if (tile) {
			Position pos = tile->pos;

			Item* useable = dynamic_cast<Item*>(tile->GetUseableThing());
			Item* item = dynamic_cast<Item*>(tile->GetTopDownThing());
			Creature* creature = dynamic_cast<Creature*>(tile->GetTopCreatureThing(true));

			if (item && (*item)()) {
				ThingData thing;
				thing.fromPos = pos;
				thing.toPos = Position(0, 0, 0);
				thing.fromStackPos = tile->GetThingStackPos(item);
				thing.itemId = item->GetID();
				thing.count = item->GetCount();
				thing.scroll = (unsigned char*)(unsigned int)Container::GetFreeIndex();

				actionList.push_back(MenuData(ACTION_LOOK, Text::GetText("ACTION_LOOK", Game::options.language), (void*)new(M_PLACE) ThingData(&thing)));
				if (!(*item)()->container) {
					if (!(*item)()->useable)
						actionList.push_back(MenuData(ACTION_USE, Text::GetText("ACTION_USE", Game::options.language), (void*)new(M_PLACE) ThingData(&thing)));
					else
						actionList.push_back(MenuData(ACTION_USEWITH, Text::GetText("ACTION_USEWITH", Game::options.language), (void*)new(M_PLACE) ThingData(&thing)));
				}
				else {
					actionList.push_back(MenuData(ACTION_OPEN, Text::GetText("ACTION_OPEN", Game::options.language), (void*)new(M_PLACE) ThingData(&thing)));
					actionList.push_back(MenuData(ACTION_OPENNEW, Text::GetText("ACTION_OPENNEW", Game::options.language), (void*)new(M_PLACE) ThingData(&thing)));
				}
				if ((*item)()->rotateable)
					actionList.push_back(MenuData(ACTION_ROTATE, Text::GetText("ACTION_ROTATE", Game::options.language), (void*)new(M_PLACE) ThingData(&thing)));
				if ((*item)()->pickupable)
					actionList.push_back(MenuData(ACTION_TRADEWITH, Text::GetText("ACTION_TRADEWITH", Game::options.language), (void*)new(M_PLACE) ThingData(&thing)));
			}
			if (useable && (*useable)() && item != useable) {
				ThingData thing;
				thing.fromPos = pos;
				thing.toPos = Position(0, 0, 0);
				thing.fromStackPos = tile->GetThingStackPos(useable);
				thing.itemId = useable->GetID();
				thing.count = useable->GetCount();
				thing.scroll = (unsigned char*)(unsigned int)Container::GetFreeIndex();

				if (actionList.empty())
					actionList.push_back(MenuData(ACTION_LOOK, Text::GetText("ACTION_LOOK", Game::options.language), (void*)new(M_PLACE) ThingData(&thing)));

				if (!(*useable)()->container) {
					if (!(*useable)()->useable)
						actionList.push_back(MenuData(ACTION_USE, Text::GetText("ACTION_USE", Game::options.language), (void*)new(M_PLACE) ThingData(&thing)));
					else
						actionList.push_back(MenuData(ACTION_USEWITH, Text::GetText("ACTION_USEWITH", Game::options.language), (void*)new(M_PLACE) ThingData(&thing)));
				}
			}
			if (creature) {
				Creature* playerCreature = player->GetCreature();
				if (creature != playerCreature) {
					actionList.push_back(MenuData(0x00, "-", NULL));
					actionList.push_back(MenuData(ACTION_ATTACK, Text::GetText("ACTION_ATTACK", Game::options.language), (void*)creature));
					actionList.push_back(MenuData(ACTION_FOLLOW, Text::GetText("ACTION_FOLLOW", Game::options.language), (void*)creature));
				}

				if (creature->IsPlayer() && creature != playerCreature) {
					actionList.push_back(MenuData(0x00, "-", NULL));
					actionList.push_back(MenuData(ACTION_SENDMESSAGE, Text::GetText("ACTION_SENDMESSAGE", Game::options.language), (void*)new(M_PLACE) std::string(creature->GetName()) ));
					if (!Creature::IsIgnored(creature->GetName()))
						actionList.push_back(MenuData(ACTION_IGNORE, Text::GetText("ACTION_IGNORE", Game::options.language), (void*)new(M_PLACE) std::string(creature->GetName()) ));
					else
						actionList.push_back(MenuData(ACTION_UNIGNORE, Text::GetText("ACTION_UNIGNORE", Game::options.language), (void*)new(M_PLACE) std::string(creature->GetName()) ));
					actionList.push_back(MenuData(ACTION_ADDTOVIP, Text::GetText("ACTION_ADDTOVIP", Game::options.language), (void*)new(M_PLACE) std::string(creature->GetName()) ));

					unsigned char playerShield = player->GetCreature()->GetShield();
					unsigned char shield = creature->GetShield();
					bool isPlayerLeader = (playerShield == SHIELD_YELLOW || playerShield == SHIELD_YELLOW_SHAREDEXP || playerShield == SHIELD_YELLOW_NOSHAREDEXP_BLINK || playerShield == SHIELD_YELLOW_NOSHAREDEXP);
					bool isPlayerMember = (playerShield == SHIELD_BLUE || playerShield == SHIELD_BLUE_SHAREDEXP || playerShield == SHIELD_BLUE_NOSHAREDEXP_BLINK || playerShield == SHIELD_BLUE_NOSHAREDEXP);
					bool isMember = (shield == SHIELD_BLUE || shield == SHIELD_BLUE_SHAREDEXP || shield == SHIELD_BLUE_NOSHAREDEXP_BLINK || shield == SHIELD_BLUE_NOSHAREDEXP);

					if ((isPlayerLeader || playerShield == SHIELD_NONE) && shield == SHIELD_NONE)
						actionList.push_back(MenuData(ACTION_PARTYINVITE, Text::GetText("ACTION_PARTYINVITE", Game::options.language), (void*)creature));
					else if (playerShield == SHIELD_NONE && shield == SHIELD_WHITEYELLOW)
						actionList.push_back(MenuData(ACTION_PARTYJOIN, Text::GetText("ACTION_PARTYJOIN", Game::options.language), (void*)creature));
					else if (isPlayerLeader && shield == SHIELD_WHITEBLUE)
						actionList.push_back(MenuData(ACTION_PARTYREVOKE, Text::GetText("ACTION_PARTYREVOKE", Game::options.language), (void*)creature));
					else if (isPlayerLeader && isMember)
						actionList.push_back(MenuData(ACTION_PARTYPASSLEADER, Text::GetText("ACTION_PARTYPASSLEADER", Game::options.language), (void*)creature));
				}
				else if (creature == playerCreature) {
					actionList.push_back(MenuData(0x00, "-", NULL));
					actionList.push_back(MenuData(ACTION_SETOUTFIT, Text::GetText("ACTION_SETOUTFIT", Game::options.language), NULL));
					if (!creature->GetOutfit().lookMount)
						actionList.push_back(MenuData(ACTION_MOUNT, Text::GetText("ACTION_MOUNT", Game::options.language), NULL));
					else
						actionList.push_back(MenuData(ACTION_DISMOUNT, Text::GetText("ACTION_DISMOUNT", Game::options.language), NULL));

					unsigned char shield = creature->GetShield();
					bool isLeader = (shield == SHIELD_YELLOW || shield == SHIELD_YELLOW_SHAREDEXP || shield == SHIELD_YELLOW_NOSHAREDEXP_BLINK || shield == SHIELD_YELLOW_NOSHAREDEXP);

					if (isLeader && shield == SHIELD_YELLOW)
						actionList.push_back(MenuData(ACTION_PARTYENABLESHARED, Text::GetText("ACTION_PARTYENABLESHARED", Game::options.language), NULL));
					else if (isLeader && shield != SHIELD_YELLOW)
						actionList.push_back(MenuData(ACTION_PARTYDISABLESHARED, Text::GetText("ACTION_PARTYDISABLESHARED", Game::options.language), NULL));
					if (shield != SHIELD_NONE)
						actionList.push_back(MenuData(ACTION_PARTYLEAVE, Text::GetText("ACTION_PARTYLEAVE", Game::options.language), NULL));
				}

				actionList.push_back(MenuData(0x00, "-", NULL));
				actionList.push_back(MenuData(ACTION_COPYNAME, Text::GetText("ACTION_COPYNAME", Game::options.language), (void*)new(M_PLACE) std::string(creature->GetName()) ));
			}
		}
	}
}

POINT WindowElementPlayground::GetTilePos(int curX, int curY) {
	float offsetX = 0.0f;
	float offsetY = 0.0f;
	float ratio = (float)width / 15;

	Player* player = game->GetPlayer();
	if (player)
		player->GetStepOffset(offsetX, offsetY);

	float x = curX / ratio + offsetX;
	float y = curY / ratio + offsetY;

	return doPOINT((int)x, (int)y);
}

void WindowElementPlayground::Print(POINT& d, AD2D_Window& gfx) {
	AD2D_Viewport currentVP = gfx.GetCurrentViewport();
	AD2D_Viewport nextVP;

	POINT scroll = GetParentScrollOffset();
	int offsetH = scroll.x;
	int offsetV = scroll.y;

	nextVP.startX = currentVP.startX + posX - offsetH - d.x;
	nextVP.startY = currentVP.startY + posY - offsetV - d.y;
	nextVP.width = width;
	nextVP.height = height;

	POINT diff = AdjustViewport(currentVP, nextVP);

	gfx.SetViewport(nextVP);

	COLOR currentColor = AD2D_Window::GetColor();

	AD2D_Window::SetColor(0.0f, 0.0f, 0.0f, 0.6f);
	gfx.PutRect(0, 0, width, height);
	AD2D_Window::SetColor(1.0f, 1.0f, 1.0f, 1.0f);

	Map* map = (game ? game->GetMap() : NULL);
	Player* player = (game ? game->GetPlayer() : NULL);

	if (map)
		map->PrintMap(&gfx, wndTemplate->font, player, -diff.x, -diff.y, width, height);

	AD2D_Window::SetColor(currentColor.red, currentColor.green, currentColor.blue, currentColor.alpha);

	gfx.SetViewport(currentVP);
}

// ---- WindowElementCooldown ---- //

WindowElementCooldown::WindowElementCooldown() {
	icon = NULL;
	background = NULL;
	groupID = 0;
	spellID = 0;

	castTime = 0;
	period = 0;
}

WindowElementCooldown::~WindowElementCooldown() { }

void WindowElementCooldown::Create(unsigned char al, int pX, int pY, unsigned short wdh, unsigned short hgt, WindowTemplate* wTemplate) {
	align = al;
	posX = pX;
	posY = pY;
	width = wdh;
	height = hgt;
	wndTemplate = wTemplate;
}

void WindowElementCooldown::SetIcon(AD2D_Image* icon) {
	this->icon = icon;
}

void WindowElementCooldown::SetBackground(AD2D_Image* background) {
	this->background = background;
}

void WindowElementCooldown::SetGroupID(unsigned char id) {
	groupID = id;
	spellID = 0;
}

void WindowElementCooldown::SetSpellID(unsigned char id) {
	spellID = id;
	groupID = 0;
}

void WindowElementCooldown::SetCast(time_lt castTime, unsigned int period) {
	this->castTime = castTime;
	this->period = period;
}

unsigned int WindowElementCooldown::GetTimeRemain() {
	time_lt time = RealTime::getTime();
	if (castTime + period > time)
		return (unsigned int)(castTime + period - time);
	else
		return 0;
}

void WindowElementCooldown::Print(POINT& d, AD2D_Window& gfx) {
	AD2D_Viewport currentVP = gfx.GetCurrentViewport();
	AD2D_Viewport nextVP;

	POINT scroll = GetParentScrollOffset();
	int offsetH = scroll.x;
	int offsetV = scroll.y;

	nextVP.startX = currentVP.startX + posX - offsetH - d.x;
	nextVP.startY = currentVP.startY + posY - offsetV - d.y;
	nextVP.width = width;
	nextVP.height = height;

	POINT diff = AdjustViewport(currentVP, nextVP);

	gfx.SetViewport(nextVP);

	COLOR currentColor = AD2D_Window::GetColor();

	if (!background)
		AD2D_Window::SetColor(currentColor.red * 0.5f, currentColor.green * 0.5f, currentColor.blue * 0.5f, currentColor.alpha);

	AD2D_Image* bg = (background ? background : (icon ? icon : NULL));
	if (bg)
		gfx.PutImage(- diff.x, - diff.y, width - diff.x, height - diff.y, *bg);
	else
		gfx.PutRect(- diff.x, - diff.y, width - diff.x, height - diff.y);

	if (RealTime::getTime() < castTime + period && period > 0) {
		float ratio = (float)((castTime + period) - RealTime::getTime()) / period;
		if (ratio < 0.0f)
			ratio = 0.0f;
		else if (ratio > 1.0f)
			ratio = 1.0f;

		AD2D_Viewport iconVP;
		iconVP.startX = currentVP.startX + posX - offsetH - d.x;
		iconVP.startY = currentVP.startY + posY - offsetV - d.y + ceil((float)height * (1.0 - ratio));
		iconVP.width = width;
		iconVP.height = floor((float)height * ratio);

		diff = AdjustViewport(nextVP, iconVP);

		gfx.SetViewport(iconVP);

		AD2D_Window::SetColor(1.0f, 1.0f, 1.0f, 1.0f);
		if (icon)
			gfx.PutImage(- diff.x, - diff.y - ceil((float)height * (1.0 - ratio)), width - diff.x, height - ceil((float)height * (1.0 - ratio)) - diff.y, *icon);
		else
			gfx.PutRect(- diff.x, - diff.y - ceil((float)height * (1.0 - ratio)), width - diff.x, height - ceil((float)height * (1.0 - ratio)) - diff.y);
	}

	AD2D_Window::SetColor(currentColor.red, currentColor.green, currentColor.blue, currentColor.alpha);

	gfx.SetViewport(currentVP);
}


// ---- WindowElementSelect ---- //

WindowElementSelect::WindowElementSelect() { }

WindowElementSelect::~WindowElementSelect() {
	mdata.ReleaseData();
}

void WindowElementSelect::Create(unsigned char al, int pX, int pY, int wdh, int hgt, Mouse* ms, MenuData* md, WindowTemplate* wTemplate) {
	align = al;
	posX = pX;
	posY = pY;
	width = wdh;
	height = hgt;
	mouse = ms;

	if (md)
		mdata = *md;

	wndTemplate = wTemplate;

	colorOut.red = 1.0f;
	colorOut.green = 1.0f;
	colorOut.blue = 1.0f;
	colorOut.alpha = 0.0f;

	colorIn.red = 1.0f;
	colorIn.green = 1.0f;
	colorIn.blue = 1.0f;
	colorIn.alpha = 0.4f;
}

void WindowElementSelect::SetColorOut(float r, float g, float b, float a) {
	colorOut.red = r;
	colorOut.green = g;
	colorOut.blue = b;
	colorOut.alpha = a;
}

void WindowElementSelect::SetColorIn(float r, float g, float b, float a) {
	colorIn.red = r;
	colorIn.green = g;
	colorIn.blue = b;
	colorIn.alpha = a;
}

void WindowElementSelect::Print(POINT& d, AD2D_Window& gfx) {
	AD2D_Viewport currentVP = gfx.GetCurrentViewport();
	AD2D_Viewport nextVP;

	POINT scroll = GetParentScrollOffset();
	int offsetH = scroll.x;
	int offsetV = scroll.y;

	nextVP.startX = currentVP.startX + posX - offsetH - d.x;
	nextVP.startY = currentVP.startY + posY - offsetV - d.y;
	nextVP.width = width;
	nextVP.height = height;

	POINT diff = AdjustViewport(currentVP, nextVP);

	gfx.SetViewport(nextVP);

	COLOR currentColor = AD2D_Window::GetColor();

	POINT pos = GetAbsolutePosition(true);
	if (window && mouse->curX >= pos.x && mouse->curX < pos.x + width && mouse->curY >= pos.y && mouse->curY < pos.y + height)
		AD2D_Window::SetColor(colorIn.red * currentColor.red, colorIn.green * currentColor.green, colorIn.blue * currentColor.blue, colorIn.alpha * currentColor.alpha);
	else
		AD2D_Window::SetColor(colorOut.red * currentColor.red, colorOut.green * currentColor.green, colorOut.blue * currentColor.blue, colorOut.alpha * currentColor.alpha);
	gfx.PutRect(- diff.x, - diff.y, width - diff.x, height - diff.y);
	AD2D_Window::SetColor(currentColor.red, currentColor.green, currentColor.blue, currentColor.alpha);

	gfx.SetViewport(currentVP);
}


// ---- WindowElementMiniMap ---- //

WindowElementMiniMap::WindowElementMiniMap() {
	minimap = NULL;
}

WindowElementMiniMap::~WindowElementMiniMap() { }

void WindowElementMiniMap::Create(unsigned char al, int pX, int pY, unsigned short wdh, unsigned short hgt, MiniMap* mm, WindowTemplate* wTemplate) {
	align = al;
	posX = pX;
	posY = pY;
	width = wdh;
	height = hgt;
	minimap = mm;
	wndTemplate = wTemplate;
}

void WindowElementMiniMap::OpenMenu(Mouse& mouse, Keyboard& keyboard, std::list<MenuData>& actionList) {
	POINT absPos = GetAbsolutePosition(true);

	if (!minimap)
		return;

	float zoom = minimap->GetZoom();
	float cx;
	float cy;
	minimap->GetOffsetPos(cx, cy);

	int xs = (int)ceil(((float)(mouse.curX - 5 - absPos.x) - width / 2 - cx) / (width * zoom / 256));
	int ys = (int)ceil(((float)(mouse.curY - 5 - absPos.y) - height / 2 - cy) / (height * zoom / 256));
	int xm = (int)ceil(((float)(mouse.curX - absPos.x) - width / 2 - cx) / (width * zoom / 256));
	int ym = (int)ceil(((float)(mouse.curY - absPos.y) - height / 2 - cy) / (height * zoom / 256));
	int xk = (int)ceil(((float)(mouse.curX + 5 - absPos.x) - width / 2 - cx) / (width * zoom / 256));
	int yk = (int)ceil(((float)(mouse.curY + 5 - absPos.y) - height / 2 - cy) / (height * zoom / 256));

	Game* game = Game::game;
	Map* map = (game ? game->GetMap() : NULL);
	Position corner = (map ? map->GetCorner() : Position(0, 0, 0));

	Position pos(corner.x + 8 + xm, corner.y + 6 + ym, corner.z + minimap->GetOffsetLevel());
	Position posMarker = pos;
	Position posWaypoint = pos;
	Marker marker(0, "");
	Waypoint waypoint(0, "");
	for (int y = yk; y >= ys; y--)
	for (int x = xk; x >= xs; x--) {
		Position _pos = Position(corner.x + 8 + x, corner.y + 6 + y, corner.z + minimap->GetOffsetLevel());
		Marker _marker = minimap->GetMarker(_pos);
		Waypoint _waypoint = minimap->GetWaypoint(_pos);

		if (_marker.first != 0) {
			posMarker = _pos;
			marker = _marker;
			break;
		}
		if (_waypoint.first != 0) {
			posWaypoint = _pos;
			waypoint = _waypoint;
		}
	}

	if (marker.first == 0) {
		actionList.push_back(MenuData(ACTION_ADDMARKER, Text::GetText("ACTION_ADDMARKER", Game::options.language), new(M_PLACE) Position(posMarker)));
	}
	else {
		actionList.push_back(MenuData(ACTION_EDITMARKER, Text::GetText("ACTION_EDITMARKER", Game::options.language), new(M_PLACE) Position(posMarker)));
		actionList.push_back(MenuData(ACTION_REMOVEMARKER, Text::GetText("ACTION_REMOVEMARKER", Game::options.language), new(M_PLACE) Position(posMarker)));
	}

	actionList.push_back(MenuData(0x00, "-", NULL));
	if (waypoint.first == 0)
		actionList.push_back(MenuData(ACTION_ADDWAYPOINT, Text::GetText("ACTION_ADDWAYPOINT", Game::options.language), new(M_PLACE) Position(posWaypoint)));
	else {
		actionList.push_back(MenuData(ACTION_EDITWAYPOINT, Text::GetText("ACTION_EDITWAYPOINT", Game::options.language), new(M_PLACE) Position(posWaypoint)));
		actionList.push_back(MenuData(ACTION_REMOVEWAYPOINT, Text::GetText("ACTION_REMOVEWAYPOINT", Game::options.language), new(M_PLACE) Position(posWaypoint)));
	}
	actionList.push_back(MenuData(0x00, "-", NULL));
	actionList.push_back(MenuData(ACTION_CLEARWAYPOINTS, Text::GetText("ACTION_CLEARWAYPOINTS", Game::options.language), NULL));
	actionList.push_back(MenuData(0x00, "-", NULL));
	actionList.push_back(MenuData(ACTION_SAVEWAYPOINTS, Text::GetText("ACTION_SAVEWAYPOINTS", Game::options.language), NULL));
	actionList.push_back(MenuData(ACTION_LOADWAYPOINTS, Text::GetText("ACTION_LOADWAYPOINTS", Game::options.language), NULL));
}

void WindowElementMiniMap::SetZoom(float zoom) {
	if (minimap)
		minimap->SetZoom(zoom);
}

float WindowElementMiniMap::GetZoom() {
	if (minimap)
		return minimap->GetZoom();

	return 0;
}

void WindowElementMiniMap::GetOffsetPos(float& x, float& y) {
	if (minimap)
		minimap->GetOffsetPos(x, y);
	else {
		x = 0;
		y = 0;
	}
}

void WindowElementMiniMap::SetOffsetPos(float x, float y) {
	if (minimap)
		minimap->SetOffsetPos(x, y);
}

void WindowElementMiniMap::Print(POINT& d, AD2D_Window& gfx) {
	AD2D_Viewport currentVP = gfx.GetCurrentViewport();
	AD2D_Viewport nextVP;

	POINT scroll = GetParentScrollOffset();
	int offsetH = scroll.x;
	int offsetV = scroll.y;

	nextVP.startX = currentVP.startX + posX - offsetH - d.x;
	nextVP.startY = currentVP.startY + posY - offsetV - d.y;
	nextVP.width = width;
	nextVP.height = height;

	POINT diff = AdjustViewport(currentVP, nextVP);

	gfx.SetViewport(nextVP);

	COLOR currentColor = AD2D_Window::GetColor();

	AD2D_Window::SetColor(1.0f, 1.0f, 1.0f, 1.0f);
	if (minimap) {
		minimap->PrintMiniMap(&gfx, width, height);

		float cx;
		float cy;
		minimap->GetOffsetPos(cx, cy);

		AD2D_Window::SetColor(0.0f, 0.0f, 0.0f, currentColor.alpha);
		gfx.PutPoint(cx + width / 2, cy + height / 2, 5);
		AD2D_Window::SetColor(1.0f, 1.0f, 1.0f, currentColor.alpha);
		gfx.PutPoint(cx + width / 2, cy + height / 2, 3);
		AD2D_Window::SetColor(currentColor.red, currentColor.green, currentColor.blue, currentColor.alpha);
	}

	gfx.SetViewport(currentVP);
}


// ---- WindowElementColorMap ---- //

WindowElementColorMap::WindowElementColorMap() { }

WindowElementColorMap::~WindowElementColorMap() { }

void WindowElementColorMap::Create(unsigned char al, int pX, int pY, unsigned short wdh, unsigned short hgt, WindowTemplate* wTemplate) {
	align = al;
	posX = pX;
	posY = pY;
	width = wdh;
	height = hgt;
	wndTemplate = wTemplate;

	index = 0;
}

void WindowElementColorMap::SetIndex(unsigned char index) {
	this->index = index;
}

unsigned char WindowElementColorMap::GetIndex() {
	return this->index;
}

void WindowElementColorMap::Print(POINT& d, AD2D_Window& gfx) {
	AD2D_Viewport currentVP = gfx.GetCurrentViewport();
	AD2D_Viewport nextVP;

	POINT scroll = GetParentScrollOffset();
	int offsetH = scroll.x;
	int offsetV = scroll.y;

	nextVP.startX = currentVP.startX + posX - offsetH - d.x;
	nextVP.startY = currentVP.startY + posY - offsetV - d.y;
	nextVP.width = width;
	nextVP.height = height;

	POINT diff = AdjustViewport(currentVP, nextVP);

	gfx.SetViewport(nextVP);

	COLOR currentColor = AD2D_Window::GetColor();

	for (int i = 0; i < 19 * 7; i++) {
		unsigned char a = i % 19;
		unsigned char b = i / 19;
		float posX = ((float)width / 19) * a;
		float posY = ((float)height / 7) * b;

		Outfit::SetTemplateColor(currentColor, i);
		gfx.PutRect(posX, posY, posX + (float)width / 19, posY + (float)height / 7);
	}

	unsigned char a = index % 19;
	unsigned char b = index / 19;
	float posX = ((float)width / 19) * a;
	float posY = ((float)height / 7) * b;

	glDisable(GL_TEXTURE_2D);
	glEnable(GL_LINE_SMOOTH);
	glLineWidth(2);
	glBegin(GL_LINES);
		AD2D_Window::SetColor(1.0f, 1.0f, 1.0f, 1.0f);
		glVertex2d(posX + 1, posY);							glVertex2d(posX + 1 + (float)width / 19, posY + (float)height / 7);
		glVertex2d(posX + 1, posY + (float)height / 7);		glVertex2d(posX + 1 + (float)width / 19, posY);
		AD2D_Window::SetColor(0.0f, 0.0f, 0.0f, 1.0f);
		glVertex2d(posX - 1, posY);							glVertex2d(posX - 1 + (float)width / 19, posY + (float)height / 7);
		glVertex2d(posX - 1, posY + (float)height / 7);		glVertex2d(posX - 1 + (float)width / 19, posY);
	glEnd();
	glDisable(GL_LINE_SMOOTH);
	glEnable(GL_TEXTURE_2D);

	AD2D_Window::SetColor(currentColor.red, currentColor.green, currentColor.blue, currentColor.alpha);

	gfx.SetViewport(currentVP);
}


// ---- WindowElementBattle ---- //

void WindowElementBattle::SetCreatureID(unsigned int id) {
	creatureID = id;
}

unsigned int WindowElementBattle::GetCreatureID() {
	return creatureID;
}

void WindowElementBattle::OpenMenu(Mouse& mouse, Keyboard& keyboard, std::list<MenuData>& actionList) {
	Creature* creature = Creature::GetFromKnown(creatureID);
	Creature* playerCreature = Creature::GetFromKnown(Player::GetCreatureID());
	if (creature) {
		ThingData thing;
		thing.fromPos = creature->pos;
		thing.toPos = Position(0, 0, 0);
		thing.fromStackPos = 0;
		thing.itemId = 0;
		thing.count = 0;
		thing.scroll = NULL;

		actionList.push_back(MenuData(ACTION_LOOK, Text::GetText("ACTION_LOOK", Game::options.language), (void*)new(M_PLACE) ThingData(&thing)));
		actionList.push_back(MenuData(0x00, "-", NULL));
		actionList.push_back(MenuData(ACTION_ATTACK, Text::GetText("ACTION_ATTACK", Game::options.language), (void*)creature));
		actionList.push_back(MenuData(ACTION_FOLLOW, Text::GetText("ACTION_FOLLOW", Game::options.language), (void*)creature));

		if (creature->IsPlayer()) {
			actionList.push_back(MenuData(0x00, "-", NULL));
			actionList.push_back(MenuData(ACTION_SENDMESSAGE, Text::GetText("ACTION_SENDMESSAGE", Game::options.language), (void*)new(M_PLACE) std::string(creature->GetName()) ));
			if (!Creature::IsIgnored(creature->GetName()))
				actionList.push_back(MenuData(ACTION_IGNORE, Text::GetText("ACTION_IGNORE", Game::options.language), (void*)new(M_PLACE) std::string(creature->GetName()) ));
			else
				actionList.push_back(MenuData(ACTION_UNIGNORE, Text::GetText("ACTION_UNIGNORE", Game::options.language), (void*)new(M_PLACE) std::string(creature->GetName()) ));
			actionList.push_back(MenuData(ACTION_ADDTOVIP, Text::GetText("ACTION_ADDTOVIP", Game::options.language), (void*)new(M_PLACE) std::string(creature->GetName()) ));

			unsigned char playerShield = playerCreature->GetShield();
			unsigned char shield = creature->GetShield();
			bool isPlayerLeader = (playerShield == SHIELD_YELLOW || playerShield == SHIELD_YELLOW_SHAREDEXP || playerShield == SHIELD_YELLOW_NOSHAREDEXP_BLINK || playerShield == SHIELD_YELLOW_NOSHAREDEXP);
			bool isPlayerMember = (playerShield == SHIELD_BLUE || playerShield == SHIELD_BLUE_SHAREDEXP || playerShield == SHIELD_BLUE_NOSHAREDEXP_BLINK || playerShield == SHIELD_BLUE_NOSHAREDEXP);
			bool isMember = (shield == SHIELD_BLUE || shield == SHIELD_BLUE_SHAREDEXP || shield == SHIELD_BLUE_NOSHAREDEXP_BLINK || shield == SHIELD_BLUE_NOSHAREDEXP);

			if ((isPlayerLeader || playerShield == SHIELD_NONE) && shield == SHIELD_NONE)
				actionList.push_back(MenuData(ACTION_PARTYINVITE, Text::GetText("ACTION_PARTYINVITE", Game::options.language), (void*)creature));
			else if (playerShield == SHIELD_NONE && shield == SHIELD_WHITEYELLOW)
				actionList.push_back(MenuData(ACTION_PARTYJOIN, Text::GetText("ACTION_PARTYJOIN", Game::options.language), (void*)creature));
			else if (isPlayerLeader && shield == SHIELD_WHITEBLUE)
				actionList.push_back(MenuData(ACTION_PARTYREVOKE, Text::GetText("ACTION_PARTYREVOKE", Game::options.language), (void*)creature));
			else if (isPlayerLeader && isMember)
				actionList.push_back(MenuData(ACTION_PARTYPASSLEADER, Text::GetText("ACTION_PARTYPASSLEADER", Game::options.language), (void*)creature));
		}

		actionList.push_back(MenuData(0x00, "-", NULL));
		actionList.push_back(MenuData(ACTION_COPYNAME, Text::GetText("ACTION_COPYNAME", Game::options.language), (void*)new(M_PLACE) std::string(creature->GetName()) ));
	}
}

void WindowElementBattle::Print(POINT& d, AD2D_Window& gfx) { }


// ---- WindowElementVIP ---- //

void WindowElementVIP::SetVIPList(VIPList* viplist) {
	this->viplist = viplist;
}

VIPList* WindowElementVIP::GetVIPList() {
	return viplist;
}

void WindowElementVIP::SetCreatureID(unsigned int id) {
	creatureID = id;
}

unsigned int WindowElementVIP::GetCreatureID() {
	return creatureID;
}

void WindowElementVIP::OpenMenu(Mouse& mouse, Keyboard& keyboard, std::list<MenuData>& actionList) {
	actionList.push_back(MenuData(ACTION_ADDTOVIP, Text::GetText("ACTION_ADDTOVIP", Game::options.language), (void*)new(M_PLACE) std::string("!@#$%") ));
	if (creatureID) {
		std::string creatureName = viplist->GetCreatureName(creatureID);

		actionList.push_back(MenuData(ACTION_REMOVEFROMVIP, Text::GetText("ACTION_REMOVEFROMVIP", Game::options.language), (void*)creatureID));
		actionList.push_back(MenuData(0x00, "-", NULL));
		actionList.push_back(MenuData(ACTION_SENDMESSAGE, Text::GetText("ACTION_SENDMESSAGE", Game::options.language), (void*)new(M_PLACE) std::string(creatureName) ));
	}
}

void WindowElementVIP::Print(POINT& d, AD2D_Window& gfx) { }


// ---- WindowTemplate ---- //

WindowTemplate::WindowTemplate() {
	elements = 0;

	memset(&tempHeader, 0, sizeof(tempHeader));
	memset(&tempBorder, 0, sizeof(tempBorder));
	memset(&tempWndIcons, 0, sizeof(tempWndIcons));
	memset(&tempContainer, 0, sizeof(tempContainer));
	memset(&tempScroll, 0, sizeof(tempScroll));
	memset(&tempButton, 0, sizeof(tempButton));
	memset(&tempTab, 0, sizeof(tempTab));
	memset(&tempTextarea, 0, sizeof(tempTextarea));
	memset(&tempMemo, 0, sizeof(tempMemo));
	memset(&tempList, 0, sizeof(tempList));
	memset(&tempTimer, 0, sizeof(tempTimer));
	memset(&tempItem, 0, sizeof(tempItem));
	memset(&tempCheckBox, 0, sizeof(tempCheckBox));
	memset(&tempScrollBar, 0, sizeof(tempScrollBar));
	memset(&tempHealthBar, 0, sizeof(tempHealthBar));
	memset(&tempManaBar, 0, sizeof(tempManaBar));
	memset(&tempSkillBar, 0, sizeof(tempSkillBar));

	font = NULL;
}

WindowTemplate::~WindowTemplate() {
	if (font)
		font = NULL;
}


void WindowTemplate::LoadTemplate(std::string path, std::string name) {
	INILoader iniLoader;
	if (!iniLoader.OpenFile(path + "/" + name + "/template.ini"))
		return;

	FileManager* files = FileManager::fileManager;
	if (!files)
		return;

	std::string value;
	std::string filename;

	std::string fontPath = path + "/" + name + "/font.png";
	std::string fontPathW = path + "/" + name + "/font.dat";
	unsigned char* fontData = files->GetFileData(fontPath);
	unsigned char* fontDataW = files->GetFileData(fontPathW);
	if (fontData && fontDataW) {
		font = new(M_PLACE) AD2D_Font;
		font->CreatePNG_(fontData, files->GetFileSize(fontPath));
		font->ReadWidthMap_(fontDataW, files->GetFileSize(fontPathW));
	}
	else {
		if (fontData)
			delete_debug(fontData, M_PLACE);
		if (fontDataW)
			delete_debug(fontDataW, M_PLACE);
	}

	if ((value = iniLoader.GetValue("ELEMENT_HEADER")) != "") {
		filename = path + "/" + name + "/" + value + "/left.png";
		tempHeader.l.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/center.png";
		tempHeader.c.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/right.png";
		tempHeader.r.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));

		INILoader posLoader;
		if (posLoader.OpenFile(path + "/" + name + "/" + value + "/position.ini")) {
			tempHeader.l_x = atoi(posLoader.GetValue("LEFT_X").c_str());
			tempHeader.l_y = atoi(posLoader.GetValue("LEFT_Y").c_str());
			tempHeader.l_width = atoi(posLoader.GetValue("LEFT_WIDTH").c_str());

			tempHeader.c_y = atoi(posLoader.GetValue("CENTER_Y").c_str());
			tempHeader.c_height = atoi(posLoader.GetValue("CENTER_HEIGHT").c_str());

			tempHeader.r_x = atoi(posLoader.GetValue("RIGHT_X").c_str());
			tempHeader.r_y = atoi(posLoader.GetValue("RIGHT_Y").c_str());
			tempHeader.r_width = atoi(posLoader.GetValue("RIGHT_WIDTH").c_str());

			tempHeader.t_offsetx = atoi(posLoader.GetValue("TEXT_OFFSET", 0).c_str());
			tempHeader.t_offsety = atoi(posLoader.GetValue("TEXT_OFFSET", 1).c_str());
		}

		elements |= ELEMENT_HEADER;
	}

	if ((value = iniLoader.GetValue("ELEMENT_WNDICONS")) != "") {
		filename = path + "/" + name + "/" + value + "/close.png";
		tempWndIcons.close.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/minimize.png";
		tempWndIcons.minimize.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/moveup.png";
		tempWndIcons.moveup.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));

		INILoader posLoader;
		if (posLoader.OpenFile(path + "/" + name + "/" + value + "/position.ini")) {
			tempWndIcons.x = atoi(posLoader.GetValue("POSITION_X").c_str());
			tempWndIcons.y = atoi(posLoader.GetValue("POSITION_Y").c_str());
			tempWndIcons.w = atoi(posLoader.GetValue("WIDTH").c_str());
			tempWndIcons.h = atoi(posLoader.GetValue("HEIGHT").c_str());
		}

		elements |= ELEMENT_WNDICONS;
	}

	if ((value = iniLoader.GetValue("ELEMENT_BORDER")) != "") {
		filename = path + "/" + name + "/" + value + "/top.png";
		tempBorder.t.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/bottom.png";
		tempBorder.b.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/left.png";
		tempBorder.l.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/right.png";
		tempBorder.r.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));

		filename = path + "/" + name + "/" + value + "/top_left.png";
		tempBorder.tl.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/top_right.png";
		tempBorder.tr.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/bottom_left.png";
		tempBorder.bl.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/bottom_right.png";
		tempBorder.br.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));

		filename = path + "/" + name + "/" + value + "/background.png";
		tempBorder.bg.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));

		INILoader posLoader;
		if (posLoader.OpenFile(path + "/" + name + "/" + value + "/position.ini")) {
			tempBorder.repeatBG = true;
			if (posLoader.GetValue("BACKGROUND") == "match")
				tempBorder.repeatBG = false;

			tempBorder.l_x = atoi(posLoader.GetValue("LEFT_X").c_str());
			tempBorder.l_width = atoi(posLoader.GetValue("LEFT_WIDTH").c_str());

			tempBorder.r_x = atoi(posLoader.GetValue("RIGHT_X").c_str());
			tempBorder.r_width = atoi(posLoader.GetValue("RIGHT_WIDTH").c_str());

			tempBorder.t_y = atoi(posLoader.GetValue("TOP_Y").c_str());
			tempBorder.t_height = atoi(posLoader.GetValue("TOP_HEIGHT").c_str());

			tempBorder.b_y = atoi(posLoader.GetValue("BOTTOM_Y").c_str());
			tempBorder.b_height = atoi(posLoader.GetValue("BOTTOM_HEIGHT").c_str());

			tempBorder.tl_x = atoi(posLoader.GetValue("TOP_LEFT_X").c_str());
			tempBorder.tl_y = atoi(posLoader.GetValue("TOP_LEFT_Y").c_str());
			tempBorder.tl_width = atoi(posLoader.GetValue("TOP_LEFT_WIDTH").c_str());
			tempBorder.tl_height = atoi(posLoader.GetValue("TOP_LEFT_HEIGHT").c_str());

			tempBorder.tr_x = atoi(posLoader.GetValue("TOP_RIGHT_X").c_str());
			tempBorder.tr_y = atoi(posLoader.GetValue("TOP_RIGHT_Y").c_str());
			tempBorder.tr_width = atoi(posLoader.GetValue("TOP_RIGHT_WIDTH").c_str());
			tempBorder.tr_height = atoi(posLoader.GetValue("TOP_RIGHT_HEIGHT").c_str());

			tempBorder.bl_x = atoi(posLoader.GetValue("BOTTOM_LEFT_X").c_str());
			tempBorder.bl_y = atoi(posLoader.GetValue("BOTTOM_LEFT_Y").c_str());
			tempBorder.bl_width = atoi(posLoader.GetValue("BOTTOM_LEFT_WIDTH").c_str());
			tempBorder.bl_height = atoi(posLoader.GetValue("BOTTOM_LEFT_HEIGHT").c_str());

			tempBorder.br_x = atoi(posLoader.GetValue("BOTTOM_RIGHT_X").c_str());
			tempBorder.br_y = atoi(posLoader.GetValue("BOTTOM_RIGHT_Y").c_str());
			tempBorder.br_width = atoi(posLoader.GetValue("BOTTOM_RIGHT_WIDTH").c_str());
			tempBorder.br_height = atoi(posLoader.GetValue("BOTTOM_RIGHT_HEIGHT").c_str());
		}

		elements |= ELEMENT_BORDER;
	}

	if ((value = iniLoader.GetValue("ELEMENT_CONTAINER")) != "") {
		filename = path + "/" + name + "/" + value + "/background.png";
		tempContainer.bg.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));

		filename = path + "/" + name + "/" + value + "/top.png";
		tempContainer.t.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/bottom.png";
		tempContainer.b.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/left.png";
		tempContainer.l.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/right.png";
		tempContainer.r.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));

		filename = path + "/" + name + "/" + value + "/top_left.png";
		tempContainer.tl.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/top_right.png";
		tempContainer.tr.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/bottom_left.png";
		tempContainer.bl.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/bottom_right.png";
		tempContainer.br.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));

		INILoader posLoader;
		if (posLoader.OpenFile(path + "/" + name + "/" + value + "/position.ini")) {
			tempContainer.l_width = atoi(posLoader.GetValue("LEFT_WIDTH").c_str());
			tempContainer.r_width = atoi(posLoader.GetValue("RIGHT_WIDTH").c_str());
			tempContainer.t_height = atoi(posLoader.GetValue("TOP_HEIGHT").c_str());
			tempContainer.b_height = atoi(posLoader.GetValue("BOTTOM_HEIGHT").c_str());
		}

		elements |= ELEMENT_CONTAINER;
	}

	if ((value = iniLoader.GetValue("ELEMENT_SCROLL")) != "") {
		filename = path + "/" + name + "/" + value + "/top.png";
		tempScroll.t.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/bottom.png";
		tempScroll.b.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/left.png";
		tempScroll.l.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/right.png";
		tempScroll.r.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/bar_horizontal.png";
		tempScroll.brh.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/bar_vertical.png";
		tempScroll.brv.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/horizontal.png";
		tempScroll.bgh.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/vertical.png";
		tempScroll.bgv.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));

		tempScroll.w = tempScroll.bgv.GetWidth();
		tempScroll.h = tempScroll.bgh.GetHeight();
		elements |= ELEMENT_SCROLL;
	}

	if ((value = iniLoader.GetValue("ELEMENT_BUTTON")) != "") {
		filename = path + "/" + name + "/" + value + "/top_left.png";
		tempButton.tl.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/top_right.png";
		tempButton.tr.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/bottom_left.png";
		tempButton.bl.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/bottom_right.png";
		tempButton.br.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/top.png";
		tempButton.t.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/bottom.png";
		tempButton.b.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/left.png";
		tempButton.l.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/right.png";
		tempButton.r.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/background.png";
		tempButton.bg.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));

		INILoader posLoader;
		if (posLoader.OpenFile(path + "/" + name + "/" + value + "/position.ini")) {
			tempButton.l_width = atoi(posLoader.GetValue("LEFT_WIDTH").c_str());
			tempButton.r_width = atoi(posLoader.GetValue("RIGHT_WIDTH").c_str());
			tempButton.t_height = atoi(posLoader.GetValue("TOP_HEIGHT").c_str());
			tempButton.b_height = atoi(posLoader.GetValue("BOTTOM_HEIGHT").c_str());
		}

		elements |= ELEMENT_BUTTON;
	}

	if ((value = iniLoader.GetValue("ELEMENT_TAB")) != "") {
		filename = path + "/" + name + "/" + value + "/top_left.png";
		tempTab.tl.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/top_right.png";
		tempTab.tr.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/bottom_left.png";
		tempTab.bl.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/bottom_right.png";
		tempTab.br.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/top.png";
		tempTab.t.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/bottom.png";
		tempTab.b.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/left.png";
		tempTab.l.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/right.png";
		tempTab.r.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/background.png";
		tempTab.bg.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/cross.png";
		tempTab.x.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));

		INILoader posLoader;
		if (posLoader.OpenFile(path + "/" + name + "/" + value + "/position.ini")) {
			tempTab.l_width = atoi(posLoader.GetValue("LEFT_WIDTH").c_str());
			tempTab.r_width = atoi(posLoader.GetValue("RIGHT_WIDTH").c_str());
			tempTab.t_height = atoi(posLoader.GetValue("TOP_HEIGHT").c_str());
			tempTab.b_height = atoi(posLoader.GetValue("BOTTOM_HEIGHT").c_str());
		}

		elements |= ELEMENT_TAB;
	}

	if ((value = iniLoader.GetValue("ELEMENT_TEXTAREA")) != "") {
		filename = path + "/" + name + "/" + value + "/top_left.png";
		tempTextarea.tl.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/top_right.png";
		tempTextarea.tr.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/bottom_left.png";
		tempTextarea.bl.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/bottom_right.png";
		tempTextarea.br.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/top.png";
		tempTextarea.t.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/bottom.png";
		tempTextarea.b.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/left.png";
		tempTextarea.l.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/right.png";
		tempTextarea.r.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/background.png";
		tempTextarea.bg.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));

		INILoader posLoader;
		if (posLoader.OpenFile(path + "/" + name + "/" + value + "/position.ini")) {
			tempTextarea.l_width = atoi(posLoader.GetValue("LEFT_WIDTH").c_str());
			tempTextarea.r_width = atoi(posLoader.GetValue("RIGHT_WIDTH").c_str());
			tempTextarea.t_height = atoi(posLoader.GetValue("TOP_HEIGHT").c_str());
			tempTextarea.b_height = atoi(posLoader.GetValue("BOTTOM_HEIGHT").c_str());
		}

		elements |= ELEMENT_TEXTAREA;
	}

	if ((value = iniLoader.GetValue("ELEMENT_MEMO")) != "") {
		filename = path + "/" + name + "/" + value + "/top_left.png";
		tempMemo.tl.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/top_right.png";
		tempMemo.tr.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/bottom_left.png";
		tempMemo.bl.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/bottom_right.png";
		tempMemo.br.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/top.png";
		tempMemo.t.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/bottom.png";
		tempMemo.b.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/left.png";
		tempMemo.l.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/right.png";
		tempMemo.r.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/middle.png";
		tempMemo.m.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/background.png";
		tempMemo.bg.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));

		INILoader posLoader;
		if (posLoader.OpenFile(path + "/" + name + "/" + value + "/position.ini")) {
			tempMemo.l_width = atoi(posLoader.GetValue("LEFT_WIDTH").c_str());
			tempMemo.r_width = atoi(posLoader.GetValue("RIGHT_WIDTH").c_str());
			tempMemo.t_height = atoi(posLoader.GetValue("TOP_HEIGHT").c_str());
			tempMemo.b_height = atoi(posLoader.GetValue("BOTTOM_HEIGHT").c_str());
		}

		elements |= ELEMENT_MEMO;
	}

	if ((value = iniLoader.GetValue("ELEMENT_LIST")) != "") {
		filename = path + "/" + name + "/" + value + "/top_left.png";
		tempList.tl.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/top_right.png";
		tempList.tr.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/bottom_left.png";
		tempList.bl.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/bottom_right.png";
		tempList.br.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/top.png";
		tempList.t.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/bottom.png";
		tempList.b.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/left.png";
		tempList.l.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/right.png";
		tempList.r.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/background.png";
		tempList.bg.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/selector.png";
		tempList.se.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));

		INILoader posLoader;
		if (posLoader.OpenFile(path + "/" + name + "/" + value + "/position.ini")) {
			tempList.l_width = atoi(posLoader.GetValue("LEFT_WIDTH").c_str());
			tempList.r_width = atoi(posLoader.GetValue("RIGHT_WIDTH").c_str());
			tempList.t_height = atoi(posLoader.GetValue("TOP_HEIGHT").c_str());
			tempList.b_height = atoi(posLoader.GetValue("BOTTOM_HEIGHT").c_str());
		}

		elements |= ELEMENT_LIST;
	}

	if ((value = iniLoader.GetValue("ELEMENT_TIMER")) != "") {
		filename = path + "/" + name + "/" + value + "/top_left.png";
		tempTimer.tl.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/top_right.png";
		tempTimer.tr.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/bottom_left.png";
		tempTimer.bl.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/bottom_right.png";
		tempTimer.br.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/top.png";
		tempTimer.t.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/bottom.png";
		tempTimer.b.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/left.png";
		tempTimer.l.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/right.png";
		tempTimer.r.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/background.png";
		tempTimer.bg.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));

		INILoader posLoader;
		if (posLoader.OpenFile(path + "/" + name + "/" + value + "/position.ini")) {
			tempTimer.l_width = atoi(posLoader.GetValue("LEFT_WIDTH").c_str());
			tempTimer.r_width = atoi(posLoader.GetValue("RIGHT_WIDTH").c_str());
			tempTimer.t_height = atoi(posLoader.GetValue("TOP_HEIGHT").c_str());
			tempTimer.b_height = atoi(posLoader.GetValue("BOTTOM_HEIGHT").c_str());
		}

		elements |= ELEMENT_TIMER;
	}

	if ((value = iniLoader.GetValue("ELEMENT_ITEMCONTAINER")) != "") {
		filename = path + "/" + name + "/" + value + "/top_left.png";
		tempItem.tl.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/top_right.png";
		tempItem.tr.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/bottom_left.png";
		tempItem.bl.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/bottom_right.png";
		tempItem.br.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/top.png";
		tempItem.t.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/bottom.png";
		tempItem.b.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/left.png";
		tempItem.l.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/right.png";
		tempItem.r.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/background.png";
		tempItem.bg.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));

		INILoader posLoader;
		if (posLoader.OpenFile(path + "/" + name + "/" + value + "/position.ini")) {
			tempItem.l_width = atoi(posLoader.GetValue("LEFT_WIDTH").c_str());
			tempItem.r_width = atoi(posLoader.GetValue("RIGHT_WIDTH").c_str());
			tempItem.t_height = atoi(posLoader.GetValue("TOP_HEIGHT").c_str());
			tempItem.b_height = atoi(posLoader.GetValue("BOTTOM_HEIGHT").c_str());
		}

		elements |= ELEMENT_ITEMCONTAINER;
	}

	if ((value = iniLoader.GetValue("ELEMENT_CHECKBOX")) != "") {
		filename = path + "/" + name + "/" + value + "/unchecked.png";
		tempCheckBox.uch.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/checked.png";
		tempCheckBox.ch.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));

		tempCheckBox.width = tempCheckBox.uch.GetWidth();
		tempCheckBox.height = tempCheckBox.uch.GetHeight();

		INILoader posLoader;
		if (posLoader.OpenFile(path + "/" + name + "/" + value + "/position.ini")) {
			tempCheckBox.text_offset = atoi(posLoader.GetValue("TEXT_OFFSET").c_str());
		}

		elements |= ELEMENT_CHECKBOX;
	}

	if ((value = iniLoader.GetValue("ELEMENT_SCROLLBAR")) != "") {
		filename = path + "/" + name + "/" + value + "/left.png";
		tempScrollBar.l.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/middle.png";
		tempScrollBar.m.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/right.png";
		tempScrollBar.r.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/bar.png";
		tempScrollBar.br.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));

		INILoader posLoader;
		if (posLoader.OpenFile(path + "/" + name + "/" + value + "/position.ini")) {
			tempScrollBar.l_width = atoi(posLoader.GetValue("LEFT_WIDTH").c_str());
			tempScrollBar.r_width = atoi(posLoader.GetValue("RIGHT_WIDTH").c_str());
			tempScrollBar.height = atoi(posLoader.GetValue("HEIGHT").c_str());
		}

		elements |= ELEMENT_SCROLLBAR;
	}

	if ((value = iniLoader.GetValue("ELEMENT_HEALTHBAR")) != "") {
		filename = path + "/" + name + "/" + value + "/empty.png";
		tempHealthBar.empty.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/bar.png";
		tempHealthBar.bar.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));

		tempHealthBar.width = tempHealthBar.empty.GetWidth();
		tempHealthBar.height = tempHealthBar.empty.GetHeight();

		elements |= ELEMENT_HEALTHBAR;
	}

	if ((value = iniLoader.GetValue("ELEMENT_MANABAR")) != "") {
		filename = path + "/" + name + "/" + value + "/empty.png";
		tempManaBar.empty.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/bar.png";
		tempManaBar.bar.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));

		tempManaBar.width = tempManaBar.empty.GetWidth();
		tempManaBar.height = tempManaBar.empty.GetHeight();

		elements |= ELEMENT_MANABAR;
	}

	if ((value = iniLoader.GetValue("ELEMENT_SKILLBAR")) != "") {
		filename = path + "/" + name + "/" + value + "/empty.png";
		tempSkillBar.empty.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));
		filename = path + "/" + name + "/" + value + "/bar.png";
		tempSkillBar.bar.CreatePNG_(files->GetFileData(filename), files->GetFileSize(filename));

		tempSkillBar.width = tempSkillBar.empty.GetWidth();
		tempSkillBar.height = tempSkillBar.empty.GetHeight();

		elements |= ELEMENT_SKILLBAR;
	}

	templateName = name;
}

void WindowTemplate::SetFont(AD2D_Font* fnt) {
	font = fnt;
}

AD2D_Font* WindowTemplate::GetFont() {
	return font;
}

unsigned short WindowTemplate::GetElements() {
	return elements;
}


// ---- Windows ---- //

Windows*	Windows::wnds = NULL;
HWND		Windows::hWnd = NULL;

MUTEX		Windows::lockWindows;


Windows::Windows(HWND hWnd, AD2D_Window* gfx, Game* game) {
	this->game = game;
	this->gfx = gfx;
	Windows::wnds = this;
	Windows::hWnd = hWnd;
}

Windows::~Windows() {
	LOCKCLASS lockClass(lockWindows);

	for (std::list<Window*>::iterator it = wndList.begin(); it != wndList.end(); it++)
		if (*it)
			delete_debug(*it, M_PLACE);

	wndList.clear();

	Windows::wnds = NULL;
}

AD2D_Window* Windows::GetGFX() {
	return gfx;
}

Game* Windows::GetGame() {
	return this->game;
}

GUIManager* Windows::GetGUIManager() {
	return &guiManager;
}

void Windows::UpdateWindows(POINT oldWndSize) {
	LOCKCLASS lockClass(lockWindows);

	if (oldWndSize.x == 0 || oldWndSize.y == 0)
		return;

	POINT margin = GetWindowMargin();
	POINT wndSize = GetWindowSize();

	std::list<Window*>::iterator it = wndList.begin();
	for (it; it != wndList.end(); it++) {
		Window* wnd = *it;
		if (!wnd)
			continue;

		if (wnd->wndType == WND_STARTUP) {
			wnd->SetPosition(100, (wndSize.y - wnd->height - 100));
			continue;
		}

		int posX = wnd->posX;
		int posY = wnd->posY;
		int oldPosX = posX;
		int oldPosY = posY;
		int width = wnd->width;
		int height = wnd->height;
		int oldWidth = width;
		int oldHeight = height;

		std::list<Window*> connected = GetConnectedWindows(wnd);

		bool ignorePos = false;
		if (wnd->GetHideAbility() || wnd->wndType == WND_OPTIONS)
			ignorePos = true;

		if (posX + width > wndSize.x) width = wndSize.x - posX;
		if (posY + height > wndSize.y) height = wndSize.y - posY;
		wnd->SetSize(width, height);
		width = wnd->width;
		height = wnd->height;

		if (!ignorePos) {
			if (posX + width > wndSize.x) posX = wndSize.x - width;
			if (posY + height > wndSize.y) posY = wndSize.y - height;
			wnd->SetPosition(posX, posY);
			posX = wnd->posX;
			posY = wnd->posY;
		}

		if (wnd->defWidth == 0 && posX + width >= oldWndSize.x && posX + width != wndSize.x) width = wndSize.x - posX;
		if (wnd->defHeight == 0 && posY + height >= oldWndSize.y && posY + height != wndSize.y) height = wndSize.y - posY;
		wnd->SetSize(width, height);
		width = wnd->width;
		height = wnd->height;

		if (!ignorePos) {
			if (posX + width >= oldWndSize.x && posX + width != wndSize.x) posX = wndSize.x - width;
			if (posY + height >= oldWndSize.y && posY + height != wndSize.y) posY = wndSize.y - height;
			wnd->SetPosition(posX, posY);
			posX = wnd->posX;
			posY = wnd->posY;
		}

		if (Game::options.fixedPositions && IsFixedPosWindow((WINDOW_TYPE)wnd->wndType)) {
			if (posX != margin.x) {
				posX = wndSize.x - margin.y - wnd->width;
				wnd->SetPosition(posX, posY);
				posX = wnd->posX;
				posY = wnd->posY;
			}
		}

		MoveConnectedWindows(wnd, true, true, true, true, (wnd->width - oldWidth) + (wnd->posX - oldPosX), (wnd->height - oldHeight) + (wnd->posY - oldPosY), &connected);
	}

	if (!guiManager.ExecuteOnResizeMainWindow()) {
		Game* game = Game::game;
		if (game) {
			Window* wndGame = game->GetWindowGame();
			Window* wndConsole = game->GetWindowConsole();
			if (wndGame && wndConsole) {
				wndGame->SetMaxSize(0, wndSize.y - wndConsole->minHeight);
				wndConsole->SetMaxSize(0, wndSize.y - wndGame->minHeight);
			}
		}

		if (Game::options.fixedPositions)
			FitGameWindow();
	}
}

void Windows::SaveGUIWindows(bool playing) {
	LOCKCLASS lockClass(lockWindows);

	INILoader iniWindows;
	if (!playing)
		iniWindows.OpenFile("templates/gui.ini");

	POINT wndPos = gfx->GetWindowPos();
    POINT wndSize = gfx->GetWindowSize();
    iniWindows.SetValue("POS_X", value2str(wndPos.x));
    iniWindows.SetValue("POS_Y", value2str(wndPos.y));
    if (!playing) {
    	POINT oldSize;
    	oldSize.x = atoi(iniWindows.GetValue("RES_X").c_str());
    	oldSize.y = atoi(iniWindows.GetValue("RES_Y").c_str());
    	if (oldSize.x != wndSize.x) {
			iniWindows.SetValue("RES_X", value2str(oldSize.x), 0);
    		iniWindows.SetValue("RES_X", value2str(wndSize.x), 1);
    	}
    	else iniWindows.SetValue("RES_X", value2str(wndSize.x));

    	if (oldSize.y != wndSize.y) {
    		iniWindows.SetValue("RES_Y", value2str(oldSize.y), 0);
    		iniWindows.SetValue("RES_Y", value2str(wndSize.y), 1);
    	}
    	else iniWindows.SetValue("RES_Y", value2str(wndSize.y));
    }
    else {
    	iniWindows.SetValue("RES_X", value2str(wndSize.x));
		iniWindows.SetValue("RES_Y", value2str(wndSize.y));
    }

    if (gfx->GetMaximized())
    	iniWindows.SetValue("MAXIMIZED", "1");

	if (playing) {
		WindowsSet guiWindows = guiManager.GetWindows();
		WindowsSet::iterator it = guiWindows.begin();
		int i = 0;
		for (it; it != guiWindows.end(); it++) {
			Window* wnd = *it;

			if (wnd && wnd->wndType != WND_CUSTOM) {
				std::string wndTag = "WINDOW_" + value2str(i++);

				POINT pos = { wnd->posX, wnd->posY };
				POINT size = { wnd->width, wnd->height };

				iniWindows.SetValue(wndTag, value2str(pos.x), 0);
				iniWindows.SetValue(wndTag, value2str(pos.y), 1);
				iniWindows.SetValue(wndTag, value2str(size.x), 2);
				iniWindows.SetValue(wndTag, value2str(size.y), 3);
				iniWindows.SetValue(wndTag, value2str((int)wnd->wndType), 4);
			}
		}
	}

    iniWindows.SaveFile("templates/gui.ini");
}

void Windows::LoadGUIWindows() {
	INILoader iniWindows;

	iniWindows.OpenFile("templates/gui.ini");

	POINT oldSize;
	oldSize.x = atoi(iniWindows.GetValue("RES_X").c_str());
	oldSize.y = atoi(iniWindows.GetValue("RES_Y").c_str());

	std::map<int, Window*> wndMap;

	int i = 0;
	while(iniWindows.GetValue(std::string("WINDOW_" + value2str(i))) != "") {
		std::string wndTag = "WINDOW_" + value2str(i);

		WINDOW_TYPE wndType = (WINDOW_TYPE)atoi(iniWindows.GetValue(wndTag, 4).c_str());
		if (wndType != WND_CUSTOM) {
			Window* wnd = OpenWindow(wndType, game);
			wndMap[i] = wnd;
		}
		i++;
	}

	i = 0;
	while(iniWindows.GetValue(std::string("WINDOW_" + value2str(i))) != "") {
		std::string wndTag = "WINDOW_" + value2str(i);

		POINT pos;
		POINT size;
		pos.x = atoi(iniWindows.GetValue(wndTag, 0).c_str());
		pos.y = atoi(iniWindows.GetValue(wndTag, 1).c_str());
		size.x = atoi(iniWindows.GetValue(wndTag, 2).c_str());
		size.y = atoi(iniWindows.GetValue(wndTag, 3).c_str());
		WINDOW_TYPE wndType = (WINDOW_TYPE)atoi(iniWindows.GetValue(wndTag, 4).c_str());

		if (wndType != WND_CUSTOM) {
			Window* wnd = wndMap[i];
			if (wnd) {
				wnd->posX = pos.x;
				wnd->posY = pos.y;
				wnd->width = size.x;
				wnd->height = size.y;
			}
		}
		i++;
	}
	guiManager.MakeGUI();

	UpdateWindows(oldSize);
}


Window* Windows::FindWindow(WINDOW_TYPE type) {
	LOCKCLASS lockClass(lockWindows);

	std::list<Window*>::reverse_iterator it = wndList.rbegin();
    for (it; it != wndList.rend(); it++) {
		Window* wnd = *it;
		if (wnd && wnd->wndType == type)
			return wnd;
	}

	return NULL;
}


Window* Windows::OpenWindow(WINDOW_TYPE type, ...) {
	LOCKCLASS lockClass(lockWindows);

	va_list vl;
	va_start(vl, type);

	POINT margin = GetWindowMargin();
	POINT wndSize = GetWindowSize();

	Window* wnd = NULL;
	if (type == WND_MESSAGE) {
		std::string message = va_arg(vl, char*);
		va_end(vl);

		INILoader iniWindow;
		if (!iniWindow.OpenFile(std::string("templates/") + Game::options.templatesGroup + std::string("/windows/message.ini")))
			return NULL;

		std::string templateName = iniWindow.GetValue("TEMPLATE");
		int wndWidth = atoi(iniWindow.GetValue("SIZE",0).c_str());
		int wndHeight = atoi(iniWindow.GetValue("SIZE",1).c_str());

		WindowTemplate* wndTemplate = GetWindowTemplate(Game::options.templatesGroup, templateName);

		wnd = new(M_PLACE) Window(type, 0, 0, wndTemplate);
		wnd->SetTitle("Message");

		ElementINI e = WindowElement::GetElementINI(iniWindow, "TEXT");
		int size = (e.fontSize >= 0 ? e.fontSize : 14);

		TextString text = ScrollText(TextString(message, 0), wndTemplate->font, size, wndWidth);
		DividedText lines = DivideText(text);
		for (int i = 0; i < lines.size(); i++) {
			std::string line = lines[i].second.text;
			int width = wndTemplate->font->GetTextWidth(line, size);
			WindowElementText* tx_line = new(M_PLACE) WindowElementText;
			tx_line->Create(0, (wndWidth - width) / 2, 5 + i * size, width, wndTemplate);
			tx_line->ApplyElementINI(iniWindow, "TEXT");
			tx_line->SetText(line);
			wnd->AddElement(tx_line);
		}

		wndHeight = lines.size() * size + 10;

		wnd->SetIntSize(wndWidth, wndHeight);
		wnd->AdjustSize();
		wnd->SetIntSize(0, 0);
		wnd->SetPosition(wndSize.x / 2 - wnd->width / 2, wndSize.y / 2 - wnd->height / 2);
		wnd->SetMinSize(wnd->width, wnd->height);
		wnd->SetMaxSize(wnd->width, wnd->height);

		AddWindow(wnd);
	}
	else if (type == WND_STARTUP) {
		Game* game = va_arg(vl, Game*);
		va_end(vl);

		INILoader iniWindow;
		if (!iniWindow.OpenFile(std::string("templates/") + Game::options.templatesGroup + std::string("/windows/startup.ini")))
			return NULL;

		std::string templateName = iniWindow.GetValue("TEMPLATE");
		int wndWidth = atoi(iniWindow.GetValue("SIZE",0).c_str());
		int wndHeight = atoi(iniWindow.GetValue("SIZE",1).c_str());

		WindowTemplate* wndTemplate = GetWindowTemplate(Game::options.templatesGroup, templateName);

		wnd = new(M_PLACE) Window(type, 0, 0, wndTemplate);
		wnd->SetElements(wnd->GetElements() & ~ELEMENT_HEADER);
		wnd->SetTitle("STARTUP");
		wnd->SetCloseAbility(false);
		wnd->SetIntSize(wndWidth, wndHeight);
		wnd->AdjustSize();
		wnd->SetMinSize(wnd->width, wnd->height);
		wnd->SetMaxSize(wnd->width, wnd->height);
		wnd->SetPosition(100, (wndSize.y - wnd->height - 100));

		WindowElementButton* bt_start = new(M_PLACE) WindowElementButton;
		bt_start->Create(0, BUTTON_NORMAL, 0, 0, 0, 0, wndTemplate);
		bt_start->ApplyElementINI(iniWindow, "BUTTON_START");
		bt_start->SetText(Text::GetText("STARTUP_0", Game::options.language));
		bt_start->SetBorder(0);

		WindowElementButton* bt_options = new(M_PLACE) WindowElementButton;
		bt_options->Create(0, BUTTON_NORMAL, 0, 0, 0, 0, wndTemplate);
		bt_options->ApplyElementINI(iniWindow, "BUTTON_OPTIONS");
		bt_options->SetText(Text::GetText("STARTUP_1", Game::options.language));
		bt_options->SetBorder(0);

		WindowElementButton* bt_homepage = new(M_PLACE) WindowElementButton;
		bt_homepage->Create(0, BUTTON_NORMAL, 0, 0, 0, 0, wndTemplate);
		bt_homepage->ApplyElementINI(iniWindow, "BUTTON_HOMEPAGE");
		bt_homepage->SetText(Text::GetText("STARTUP_2", Game::options.language));
		bt_homepage->SetBorder(0);

		WindowElementButton* bt_exit = new(M_PLACE) WindowElementButton;
		bt_exit->Create(0, BUTTON_NORMAL, 0, 0, 0, 0, wndTemplate);
		bt_exit->ApplyElementINI(iniWindow, "BUTTON_EXIT");
		bt_exit->SetText(Text::GetText("STARTUP_3", Game::options.language));
		bt_exit->SetBorder(0);

		bt_start->SetAction(boost::bind(&Game::onMakeAction, game, this, wnd, ACTION_OPENWINDOW, (void*)WND_LOGIN));
		bt_options->SetAction(boost::bind(&Game::onOptions, game, this, wnd, 0xFF));
		bt_homepage->SetAction(boost::bind(&Game::onHomePage, game));
		bt_exit->SetAction(boost::bind(&Game::PostQuit));

		wnd->AddElement(bt_start);
		wnd->AddElement(bt_options);
		wnd->AddElement(bt_homepage);
		wnd->AddElement(bt_exit);

		AddWindow(wnd);
	}
	else if (type == WND_LOGIN) {
		Game* game = va_arg(vl, Game*);
		va_end(vl);

		Host host = game->GetHost();

		INILoader iniWindow;
		if (!iniWindow.OpenFile(std::string("templates/") + Game::options.templatesGroup + std::string("/windows/login.ini")))
			return NULL;

		std::string templateName = iniWindow.GetValue("TEMPLATE");
		int wndWidth = atoi(iniWindow.GetValue("SIZE",0).c_str());
		int wndHeight = atoi(iniWindow.GetValue("SIZE",1).c_str());

		WindowTemplate* wndTemplate = GetWindowTemplate(Game::options.templatesGroup, templateName);

		wnd = new(M_PLACE) Window(type, 0, 0, wndTemplate);
		wnd->SetTitle(Text::GetText("LOGIN_T", Game::options.language));
		//wnd->SetCloseAbility(false);
		wnd->SetIntSize(wndWidth, wndHeight);
		wnd->AdjustSize();
		wnd->SetMinSize(wnd->width, wnd->height);
		wnd->SetMaxSize(wnd->width, wnd->height);
		wnd->SetPosition(wndSize.x / 2 - wnd->width / 2, wndSize.y / 2 - wnd->height / 2);

		WindowElementTextarea* ta_account = new(M_PLACE) WindowElementTextarea;
		ta_account->Create(0, 0, 0, 0, 0, true, false, wndTemplate);
		ta_account->ApplyElementINI(iniWindow, "TEXTAREA_ACCOUNT");
		ta_account->SetText(host.account);

		WindowElementTextarea* ta_password = new(M_PLACE) WindowElementTextarea;
		ta_password->Create(0, 0, 0, 0, 0, true, false, wndTemplate);
		ta_password->ApplyElementINI(iniWindow, "TEXTAREA_PASSWORD");
		ta_password->SetText("");
		ta_password->SetHidden(true);

		WindowElementButton* bt_login = new(M_PLACE) WindowElementButton;
		bt_login->Create(0, BUTTON_NORMAL, 0, 0, 0, 0, wndTemplate);
		bt_login->ApplyElementINI(iniWindow, "BUTTON_LOGIN");
		bt_login->SetText(Text::GetText("LOGIN_3", Game::options.language));

		WindowElementText* tx_account = new(M_PLACE) WindowElementText;
		tx_account->Create(0, 0, 0, 0xFFFF, wndTemplate);
		tx_account->ApplyElementINI(iniWindow, "TEXT_ACCOUNT");
		tx_account->SetText(Text::GetText("LOGIN_0", Game::options.language));
		tx_account->SetEnabled(false);

		WindowElementText* tx_password = new(M_PLACE) WindowElementText;
		tx_password->Create(0, 0, 0, 0xFFFF, wndTemplate);
		tx_password->ApplyElementINI(iniWindow, "TEXT_PASSWORD");
		tx_password->SetText(Text::GetText("LOGIN_1", Game::options.language));
		tx_password->SetEnabled(false);

		WindowElementList* ls_servers = NULL;
		WindowElementText* tx_servers = NULL;

		WindowElementTextarea* ta_server = NULL;
		WindowElementButton* bt_servers = NULL;

		std::vector<void*> pointers;

		if (Game::adminOptions.simpleLogin) {
			ls_servers = new(M_PLACE) WindowElementList;
			ls_servers->Create(0, 0, 0, 0, 0, wndTemplate);
			ls_servers->ApplyElementINI(iniWindow, "LIST_SERVERS");

			Servers* servers = game->GetServers();
			int num = 0;
			while(servers->GetFavoriteServer(num).host != "") {
				Server server = servers->GetFavoriteServer(num);
				if (!server.promoted)
					break;
				ls_servers->AddElement(server.name);
				num++;
			}

			num = servers->GetFavoriteNum(host.name);
			if (num != -1)
				ls_servers->SetOption(num);
			else
				ls_servers->SetOption(0);

			tx_servers = new(M_PLACE) WindowElementText;
			tx_servers->Create(0, 0, 0, 0xFFFF, wndTemplate);
			tx_servers->ApplyElementINI(iniWindow, "TEXT_SERVERS");
			tx_servers->SetText(Text::GetText("LOGIN_2", Game::options.language));
			tx_servers->SetEnabled(false);
		}
		else {
			ta_server = new(M_PLACE) WindowElementTextarea;
			ta_server->Create(0, 0, 0, 0, 0, true, false, wndTemplate);
			ta_server->ApplyElementINI(iniWindow, "TEXTAREA_SERVER");
			ta_server->SetText(host.name);
			ta_server->SetEnabled(false);

			bt_servers = new(M_PLACE) WindowElementButton;
			bt_servers->Create(0, BUTTON_NORMAL, 0, 0, 0, 0, wndTemplate);
			bt_servers->ApplyElementINI(iniWindow, "BUTTON_SERVERS");
			bt_servers->SetText(Text::GetText("LOGIN_2", Game::options.language));
		}

		pointers.push_back((void*)ta_account);
		pointers.push_back((void*)ta_password);
		pointers.push_back((void*)ls_servers);
		pointers.push_back((void*)ta_server);

		if (ls_servers)
			ls_servers->SetAction(boost::bind(&Game::onSetServer, game, (WindowElementTableMemo*)NULL, pointers));
		if (bt_servers)
			bt_servers->SetAction(boost::bind(&Game::onServers, game, pointers));

		boost::function<void()> func = boost::bind(&Game::onLogin, game, this, wnd, pointers);

		ta_account->SetAction(func);
		ta_password->SetAction(func);
		bt_login->SetAction(func);
		wnd->AddExecuteFunction(func);
		wnd->AddCloseFunction(boost::bind(&Windows::CloseWindows, this, WND_SERVERS, false));

		wnd->AddElement(ta_account);
		wnd->AddElement(ta_password);
		if (ls_servers && tx_servers) {
			wnd->AddElement(ls_servers);
			wnd->AddElement(tx_servers);
		}
		if (ta_server && bt_servers) {
			wnd->AddElement(ta_server);
			wnd->AddElement(bt_servers);
		}
		wnd->AddElement(bt_login);
		wnd->AddElement(tx_account);
		wnd->AddElement(tx_password);

		AddWindow(wnd);
	}
	else if (type == WND_SERVERS) {
		Game* game = va_arg(vl, Game*);
		std::vector<void*>* pointers = va_arg(vl, std::vector<void*>*);
		va_end(vl);

		Host host = game->GetHost();

		INILoader iniWindow;
		if (!iniWindow.OpenFile(std::string("templates/") + Game::options.templatesGroup + std::string("/windows/servers.ini")))
			return NULL;

		std::string templateName = iniWindow.GetValue("TEMPLATE");
		int wndWidth = atoi(iniWindow.GetValue("SIZE",0).c_str());
		int wndHeight = atoi(iniWindow.GetValue("SIZE",1).c_str());

		WindowTemplate* wndTemplate = GetWindowTemplate(Game::options.templatesGroup, templateName);

		wnd = new(M_PLACE) Window(type, 0, 0, wndTemplate);
		wnd->SetTitle(Text::GetText("SERVERS_T", Game::options.language));
		wnd->SetIntSize(wndWidth, wndHeight);
		wnd->AdjustSize();
		wnd->SetPosition(wndSize.x / 2 - wnd->width / 2, wndSize.y / 2 - wnd->height / 2);
		wnd->SetIntSize(0, 0);
		wnd->SetMinSize(wnd->width, wnd->height);
		wnd->SetMaxSize(0xFFFF, 0xFFFF);

		WindowElementTab* tb_servers = new(M_PLACE) WindowElementTab;
		tb_servers->Create(0, 0, 0, 0, 0, false, false, wndTemplate);
		tb_servers->ApplyElementINI(iniWindow, "TAB_SERVERS");
		TabElement tab1 = tb_servers->AddTab(Text::GetText("SERVERS_0", Game::options.language));
		TabElement tab2 = tb_servers->AddTab(Text::GetText("SERVERS_1", Game::options.language));
		TabElement tab3 = tb_servers->AddTab(Text::GetText("SERVERS_2", Game::options.language));
		tb_servers->SetActiveTab(tab2.first);
		tb_servers->SetLocks(true, true);

		wnd->AddElement(tb_servers);

		Servers* servers = game->GetServers();
		if (servers) {
			servers->SetContainers(tb_servers, tab1.second, tab2.second, tab3.second);
			servers->UpdateContainers(*pointers);
		}

		AddWindow(wnd);
	}
	else if (type == WND_CHARACTERSLIST) {
		Game* game = va_arg(vl, Game*);
		va_end(vl);

		Host host = game->GetHost();

		INILoader iniWindow;
		if (!iniWindow.OpenFile(std::string("templates/") + Game::options.templatesGroup + std::string("/windows/characterslist.ini")))
			return NULL;

		std::string templateName = iniWindow.GetValue("TEMPLATE");
		int wndWidth = atoi(iniWindow.GetValue("SIZE",0).c_str());
		int wndHeight = atoi(iniWindow.GetValue("SIZE",1).c_str());

		WindowTemplate* wndTemplate = GetWindowTemplate(Game::options.templatesGroup, templateName);

		wnd = new(M_PLACE) Window(type, 0, 0, wndTemplate);
		wnd->SetTitle(Text::GetText("CHARACTERSLIST_T", Game::options.language));
		wnd->SetIntSize(wndWidth, wndHeight);
		wnd->AdjustSize();
		wnd->SetPosition(wndSize.x / 2 - wnd->width / 2, wndSize.y / 2 - wnd->height / 2);
		wnd->SetIntSize(0, 0);
		wnd->SetMinSize(wnd->width, wnd->height);
		wnd->SetMaxSize(0xFFFF, 0xFFFF);

		WindowElementMemo* me_list = new(M_PLACE) WindowElementMemo;
		me_list->Create(0, 0, 0, 0, 0, wndTemplate);
		me_list->ApplyElementINI(iniWindow, "MEMO_LIST");
		me_list->SetLocks(true, true);

		wnd->SetActiveElement(me_list);
		//wnd->activeElement = me_list;
		//me_list->active = true;

		Character lastChar = game->GetCharacter();
		CharactersList list = game->GetCharacters();
		CharactersList::iterator it = list.begin();

		std::string lastOption = lastChar.name +  + " (" + lastChar.serv + ")";
		for (it; it != list.end(); it++)
			me_list->AddElement(std::string(it->name + " (" + it->serv + ")"));

		if (lastChar.name != "")
			me_list->SetOption(lastOption);

		WindowElementText* tx_pacc = new(M_PLACE) WindowElementText;
		tx_pacc->Create(ALIGN_V_BOTTOM, 0, 0, 0xFFFF, wndTemplate);
		tx_pacc->ApplyElementINI(iniWindow, "TEXT_ACCOUNT");
		if (host.PACC > 0) {
			tx_pacc->SetText(Text::GetText("CHARACTERSLIST_0", Game::options.language) + " " + value2str(host.PACC));
			tx_pacc->SetColor(0.3, 1.0f, 0.3f);
		}
		else if (host.PACC == 0xFFFF) {
			tx_pacc->SetText(Text::GetText("CHARACTERSLIST_1", Game::options.language));
			tx_pacc->SetColor(0.3, 1.0f, 0.3f);
		}
		else
			tx_pacc->SetText(Text::GetText("CHARACTERSLIST_2", Game::options.language));

		WindowElementButton* bt_login = new(M_PLACE) WindowElementButton;
		bt_login->Create(ALIGN_H_RIGHT | ALIGN_V_BOTTOM, BUTTON_NORMAL, 0, 0, 0, 0, wndTemplate);
		bt_login->ApplyElementINI(iniWindow, "BUTTON_LOGIN");
		bt_login->SetText(Text::GetText("CHARACTERSLIST_3", Game::options.language));

		wnd->AddActivateFunction(boost::bind(&Window::SetActiveElement, wnd, me_list));

		boost::function<void()> func = boost::bind(&Game::onEnterGame, game, this, wnd,
			&me_list->option);

		wnd->AddExecuteFunction(func);
		//me_list->SetAction(func);
		bt_login->SetAction(func);

		wnd->AddElement(me_list);
		wnd->AddElement(tx_pacc);
		wnd->AddElement(bt_login);

		AddWindow(wnd);
	}
	else if (type == WND_QUEUE) {
		Game* game = va_arg(vl, Game*);
		std::string message = va_arg(vl, char*);
		unsigned char seconds = va_arg(vl, int);
		va_end(vl);

		INILoader iniWindow;
		if (!iniWindow.OpenFile(std::string("templates/") + Game::options.templatesGroup + std::string("/windows/queue.ini")))
			return NULL;

		std::string templateName = iniWindow.GetValue("TEMPLATE");
		int wndWidth = atoi(iniWindow.GetValue("SIZE",0).c_str());
		int wndHeight = atoi(iniWindow.GetValue("SIZE",1).c_str());

		WindowTemplate* wndTemplate = GetWindowTemplate(Game::options.templatesGroup, templateName);

		wnd = new(M_PLACE) Window(type, 0, 0, wndTemplate);
		wnd->SetTitle(Text::GetText("QUEUE_T", Game::options.language));

		ElementINI e = WindowElement::GetElementINI(iniWindow, "TEXT");
		int size = (e.fontSize >= 0 ? size : 14);

		TextString text = ScrollText(TextString(message, 0), wndTemplate->font, size, wndWidth);
		DividedText lines = DivideText(text);
		for (int i = 0; i < lines.size(); i++) {
			std::string line = lines[i].second.text;
			int width = wndTemplate->font->GetTextWidth(line, size);
			WindowElementText* tx_line = new(M_PLACE) WindowElementText;
			tx_line->Create(0, (wndWidth - width) / 2, 5 + i * size, width, wndTemplate);
			tx_line->ApplyElementINI(iniWindow, "TEXT");
			tx_line->SetText(line);
			wnd->AddElement(tx_line);
		}

		wndHeight = lines.size() * size + 10;

		WindowElementTimer* tm_timer = new(M_PLACE) WindowElementTimer;
		tm_timer->Create(ALIGN_V_BOTTOM, 0, 0, 0, 0, wndTemplate);
		tm_timer->ApplyElementINI(iniWindow, "TIMER");
		tm_timer->SetPosition(tm_timer->posX, wndHeight + tm_timer->posY);
		tm_timer->SetLocks(true, false);
		tm_timer->SetTimeInterval(seconds * 1000);

		wndHeight += tm_timer->posY + tm_timer->height + 10;

		wnd->SetIntSize(wndWidth, wndHeight);
		wnd->AdjustSize();
		wnd->SetMinSize(wnd->width, wnd->height);
		wnd->SetMaxSize(wnd->width, wnd->height);
		wnd->SetIntSize(0, 0);
		wnd->SetPosition(wndSize.x / 2 - wnd->width / 2, wndSize.y / 2 - wnd->height / 2);

		boost::function<void()> func = boost::bind(&Game::onQueue, game, this, wnd);

		tm_timer->SetAction(func);
		tm_timer->Run();

		wnd->AddElement(tm_timer);

		AddWindow(wnd);
	}
	else if (type == WND_GAME) {
		Game* game = va_arg(vl, Game*);
		va_end(vl);

		INILoader iniWindow;
		if (!iniWindow.OpenFile(std::string("templates/") + Game::options.templatesGroup + std::string("/windows/game.ini")))
			return NULL;

		std::string templateName = iniWindow.GetValue("TEMPLATE");
		int wndWidth = atoi(iniWindow.GetValue("SIZE",0).c_str());
		int wndHeight = atoi(iniWindow.GetValue("SIZE",1).c_str());

		WindowTemplate* wndTemplate = GetWindowTemplate(Game::options.templatesGroup, templateName);

		wnd = new(M_PLACE) Window(type, 0, 0, wndTemplate);
		wnd->SetCloseAbility(false);
		wnd->SetAlwaysActive(true);
		wnd->SetElements(wnd->GetElements() & ~(ELEMENT_HEADER));
		wnd->SetTitle("Game");
		wnd->SetIntSize(wndWidth, wndHeight);
		wnd->AdjustSize();
		wnd->SetMinSize(80, 60);
		if (Game::options.dontStrech)
			wnd->SetMaxSize(0xFFFF, wnd->height);
		else
			wnd->SetMaxSize(0xFFFF, 0xFFFF);
		wnd->SetIntSize(0, 0);

		int offsetX = 0;
		int offsetY = 0;
		Window* wndStatus = game->GetWindowStatus();
		Window* wndConsole = game->GetWindowConsole();
		Window* wndHotkeys = game->GetWindowHotkeys();
		if (wndStatus)
			offsetX = wndStatus->width;
		if (wndConsole)
			offsetY = wndConsole->height;

		wnd->SetPosition(offsetX, offsetY);

		WindowElementPlayground* playground = new(M_PLACE) WindowElementPlayground;
		playground->Create(0, 0, 0, 480, 352, game, wndTemplate);
		playground->SetLocks(true, true);

		wnd->AddElement(playground);
		playground->SetSize(wndWidth, wndHeight);

		wnd->AddCloseFunction(boost::bind(&GUIManager::RemoveWindow, &guiManager, wnd));
		AddWindow(wnd);
		guiManager.AddWindow(wnd);

		if (Game::options.fixedPositions) {
			int size = wndSize.x - (wndStatus ? wndStatus->width : 0) - (wndHotkeys ? wndHotkeys->width : 0) - GetWindowWidth(WND_INVENTORY);
			wnd->SetSize(size, wnd->height);
		}

		game->SetWindowGame(wnd);
	}
	else if (type == WND_CONSOLE) {
		Game* game = va_arg(vl, Game*);
		va_end(vl);

		INILoader iniWindow;
		if (!iniWindow.OpenFile(std::string("templates/") + Game::options.templatesGroup + std::string("/windows/console.ini")))
			return NULL;

		std::string templateName = iniWindow.GetValue("TEMPLATE");
		int wndWidth = atoi(iniWindow.GetValue("SIZE",0).c_str());
		int wndHeight = atoi(iniWindow.GetValue("SIZE",1).c_str());

		WindowTemplate* wndTemplate = GetWindowTemplate(Game::options.templatesGroup, templateName);

		wnd = new(M_PLACE) Window(type, 0, 0, wndTemplate);
		wnd->SetCloseAbility(false);
		wnd->SetAlwaysActive(true);
		wnd->SetElements(wnd->GetElements() & ~(ELEMENT_HEADER));
		wnd->SetTitle("Console");
		wnd->SetIntSize(wndWidth, wndHeight);
		wnd->AdjustSize();
		wnd->SetMinSize(80, 100);
		wnd->SetMaxSize(0xFFFF, 0);
		wnd->SetIntSize(0, 0);

		WindowElementTab* tab_temp = new(M_PLACE) WindowElementTab;
		tab_temp->Create(0, 0, 0, 0, 0, false, false, wndTemplate);
		tab_temp->ApplyElementINI(iniWindow, "CONSOLE_TAB");
		tab_temp->SetLocks(true, true);

		game->SetConsoleTab(tab_temp);

		WindowElementTextarea* command = new(M_PLACE) WindowElementTextarea;
		command->Create(ALIGN_V_BOTTOM, 0, 0, 0, 0, true, false, wndTemplate);
		command->ApplyElementINI(iniWindow, "CONSOLE_COMMAND");
		command->SetLocks(true, false);

		WindowElementButton* bt_channels = new(M_PLACE) WindowElementButton;
		bt_channels->Create(ALIGN_H_RIGHT | ALIGN_V_BOTTOM, BUTTON_NORMAL, 0, 0, 0, 0, wndTemplate);
		bt_channels->ApplyElementINI(iniWindow, "BUTTON_CHANNELS");
		bt_channels->SetImage(Icons::GetButtonIcon(1));
		bt_channels->SetAction(boost::bind(&Game::onMakeAction, game, this, wnd, ACTION_CHANNELS, (void*)NULL));

		boost::function<void()> func = boost::bind(&Game::onConsoleEnter, game, this, command, tab_temp->GetActiveTabPtr());
		command->SetAction(func);

		wnd->AddElement(tab_temp);
		wnd->AddElement(command);
		wnd->AddElement(bt_channels);

		wnd->AddCloseFunction(boost::bind(&GUIManager::RemoveWindow, &guiManager, wnd));
		AddWindow(wnd);
		guiManager.AddWindow(wnd);

		Window* wndStatus = game->GetWindowStatus();
		Window* wndGame = game->GetWindowGame();

		int sizeX = (wndGame ? wndGame->width : wnd->width);
		int sizeY = (wndGame ? wndSize.y - wndGame->height : wndSize.y);
		wnd->SetSize(sizeX, sizeY);

		int offsetX = (wndStatus ? wndStatus->width : 0);
		int offsetY = (wndGame ? wndGame->height : 0);
		wnd->SetPosition(offsetX, offsetY);

		game->SetCmdLine(command);
		game->SetWindowConsole(wnd);

		ChatUsers empty;
		game->OpenChannel(CHANNEL_DEFAULT, Text::GetText("CHANNEL_DEFAULT", Game::options.language), empty, empty, false);
		game->OpenChannel(CHANNEL_SERVER_LOG, Text::GetText("CHANNEL_SERVER_LOG", Game::options.language), empty, empty, false);
	}
	else if (type == WND_INVENTORY) {
		Game* game = va_arg(vl, Game*);
		va_end(vl);

		INILoader iniWindow;
		if (!iniWindow.OpenFile(std::string("templates/") + Game::options.templatesGroup + std::string("/windows/inventory.ini")))
			return NULL;

		std::string templateName = iniWindow.GetValue("TEMPLATE");
		int wndWidth = atoi(iniWindow.GetValue("SIZE",0).c_str());
		int wndHeight = atoi(iniWindow.GetValue("SIZE",1).c_str());

		Player* player = game->GetPlayer();
		if (!player)
			return NULL;

		Statistics* statistics = player->GetStatistics();
		Container* container = player->GetInventory();

		std::string title = container->GetName();
		int places = container->GetPlaces();

		WindowTemplate* wndTemplate = GetWindowTemplate(Game::options.templatesGroup, templateName);

		wnd = new(M_PLACE) Window(type, 0, 0, wndTemplate);
		wnd->SetCloseAbility(false);
		wnd->SetTitle(Text::GetText("INVENTORY_T", Game::options.language));
		wnd->SetIntSize(wndWidth, wndHeight);
		wnd->AdjustSize();
		wnd->SetMinSize(wnd->width, wnd->height);
		wnd->SetMaxSize(wnd->width, wnd->height);

		POINT pos = GetFixedOpenPosition(wndSize.x - margin.y - wnd->width, wnd);
		wnd->SetPosition(pos.x, pos.y);

		WindowElementHealthBar* healthBar = new(M_PLACE) WindowElementHealthBar;
		healthBar->Create(0, 0, 0, 0, &statistics->health, &statistics->maxHealth, wndTemplate);
		healthBar->ApplyElementINI(iniWindow, "HEALTH_BAR");

		WindowElementManaBar* manaBar = new(M_PLACE) WindowElementManaBar;
		manaBar->Create(0, 0, 0, 0, &statistics->mana, &statistics->maxMana, wndTemplate);
		manaBar->ApplyElementINI(iniWindow, "MANA_BAR");

		WindowElementText* tx_cap = new(M_PLACE) WindowElementText;
		tx_cap->Create(0, 0, 0, 0, wndTemplate);
		tx_cap->ApplyElementINI(iniWindow, "TEXT_CAPACITY");
		tx_cap->SetText(Text::GetText("INVENTORY_8", Game::options.language));
		tx_cap->SetValuePtr(TypePointer("float", (void*)&statistics->capacity));

		WindowElementText* tx_soul = new(M_PLACE) WindowElementText;
		tx_soul->Create(0, 0, 0, 0, wndTemplate);
		tx_soul->ApplyElementINI(iniWindow, "TEXT_SOUL");
		tx_soul->SetText(Text::GetText("INVENTORY_9", Game::options.language));
		tx_soul->SetValuePtr(TypePointer("uint8", (void*)&statistics->soul));

		WindowElementContainer* cont = new(M_PLACE) WindowElementContainer;
		cont->Create(0, 0, 0, 0, 0, false, true, wndTemplate);
		cont->ApplyElementINI(iniWindow, "FIGHT_MODES");
		cont->SetIntSize(cont->width, cont->height);
		cont->AdjustSize();

		WindowElementButton* bt_fight1 = new(M_PLACE) WindowElementButton;
		bt_fight1->Create(0, BUTTON_SWITCH, 0, 0, 0, 0, wndTemplate);
		bt_fight1->ApplyElementINI(iniWindow, "FIGHT_MODE_1");
		bt_fight1->SetImage(Icons::GetFightIcon(1));
		bt_fight1->SetPressed(player->GetFightMode(0));
		bt_fight1->SetInverse(true);

		WindowElementButton* bt_fight2 = new(M_PLACE) WindowElementButton;
		bt_fight2->Create(0, BUTTON_SWITCH, 0, 0, 0, 0, wndTemplate);
		bt_fight2->ApplyElementINI(iniWindow, "FIGHT_MODE_2");
		bt_fight2->SetImage(Icons::GetFightIcon(2));
		bt_fight2->SetPressed(player->GetFightMode(1));
		bt_fight2->SetInverse(true);

		WindowElementButton* bt_fight3 = new(M_PLACE) WindowElementButton;
		bt_fight3->Create(0, BUTTON_SWITCH, 0, 0, 0, 0, wndTemplate);
		bt_fight3->ApplyElementINI(iniWindow, "FIGHT_MODE_3");
		bt_fight3->SetImage(Icons::GetFightIcon(3));
		bt_fight3->SetPressed(player->GetFightMode(2));
		bt_fight3->SetInverse(true);

		WindowElementButton* bt_fight4 = new(M_PLACE) WindowElementButton;
		bt_fight4->Create(0, BUTTON_SWITCH, 0, 0, 0, 0, wndTemplate);
		bt_fight4->ApplyElementINI(iniWindow, "FIGHT_MODE_4");
		bt_fight4->SetImage(Icons::GetFightIcon(4));
		bt_fight4->SetPressed(player->GetFightMode(3));
		bt_fight4->SetInverse(true);

		WindowElementButton* bt_fight5 = new(M_PLACE) WindowElementButton;
		bt_fight5->Create(0, BUTTON_SWITCH, 0, 0, 0, 0, wndTemplate);
		bt_fight5->ApplyElementINI(iniWindow, "FIGHT_MODE_5");
		bt_fight5->SetImage(Icons::GetFightIcon(5));
		bt_fight5->SetPressed(player->GetFightMode(4));
		bt_fight5->SetInverse(true);

		WindowElementButton* bt_fight6 = new(M_PLACE) WindowElementButton;
		bt_fight6->Create(0, BUTTON_SWITCH, 0, 0, 0, 0, wndTemplate);
		bt_fight6->ApplyElementINI(iniWindow, "FIGHT_MODE_6");
		bt_fight6->SetImage(Icons::GetFightIcon(6));
		bt_fight6->SetPressed(player->GetFightMode(5));

		WindowElementButton** buttons = new(M_PLACE) WindowElementButton*[6];
		buttons[0] = bt_fight1;
		buttons[1] = bt_fight2;
		buttons[2] = bt_fight3;
		buttons[3] = bt_fight4;
		buttons[4] = bt_fight5;
		buttons[5] = bt_fight6;

		boost::function<void()> func1 = boost::bind(&Game::onMakeAction, game, this, wnd, ACTION_CHANGEFIGHTMODE, (void*)buttons);
		boost::function<void()> func2 = boost::bind(&Game::onMakeAction, game, this, (Window*)NULL, ACTION_CHANGEFIGHTMODE, (void*)buttons);

		bt_fight1->SetAction(func1);
		bt_fight2->SetAction(func1);
		bt_fight3->SetAction(func1);
		bt_fight4->SetAction(func1);
		bt_fight5->SetAction(func1);
		bt_fight6->SetAction(func1);
		wnd->AddCloseFunction(func2);

		cont->AddElement(bt_fight1);
		cont->AddElement(bt_fight2);
		cont->AddElement(bt_fight3);
		cont->AddElement(bt_fight4);
		cont->AddElement(bt_fight5);
		cont->AddElement(bt_fight6);

		WindowElementButton* bt_stop = new(M_PLACE) WindowElementButton;
		bt_stop->Create(0, BUTTON_NORMAL, 0, 0, 0, 0, wndTemplate);
		bt_stop->ApplyElementINI(iniWindow, "BUTTON_STOP");
		bt_stop->SetText(Text::GetText("INVENTORY_0", Game::options.language));
		//bt_stop->SetFontSize(12);
		bt_stop->SetAction(boost::bind(&Game::PlayerStopAutoWalk, game));

		WindowElementButton* bt_options = new(M_PLACE) WindowElementButton;
		bt_options->Create(0, BUTTON_NORMAL, 0, 0, 0, 0, wndTemplate);
		bt_options->ApplyElementINI(iniWindow, "BUTTON_OPTIONS");
		bt_options->SetText(Text::GetText("INVENTORY_1", Game::options.language));
		//bt_options->SetFontSize(12);
		bt_options->SetAction(boost::bind(&Game::onOptions, game, this, wnd, 0xFF));

		WindowElementButton* bt_quests = new(M_PLACE) WindowElementButton;
		bt_quests->Create(0, BUTTON_NORMAL, 0, 0, 0, 0, wndTemplate);
		bt_quests->ApplyElementINI(iniWindow, "BUTTON_QUESTS");
		bt_quests->SetText(Text::GetText("INVENTORY_2", Game::options.language));
		//bt_quests->SetFontSize(12);
		bt_quests->SetAction(boost::bind(&Game::onMakeAction, game, this, wnd, ACTION_QUESTLOG, (void*)NULL));

		WindowElementButton* bt_logout = new(M_PLACE) WindowElementButton;
		bt_logout->Create(0, BUTTON_NORMAL, 0, 0, 0, 0, wndTemplate);
		bt_logout->ApplyElementINI(iniWindow, "BUTTON_LOGOUT");
		bt_logout->SetText(Text::GetText("INVENTORY_3", Game::options.language));
		//bt_logout->SetFontSize(12);
		bt_logout->SetAction(boost::bind(&Game::onMakeAction, game, this, wnd, ACTION_LOGOUT, (void*)NULL));

		WindowElementButton* bt_stats = new(M_PLACE) WindowElementButton;
		bt_stats->Create(0, BUTTON_NORMAL, 0, 0, 0, 0, wndTemplate);
		bt_stats->ApplyElementINI(iniWindow, "BUTTON_STATS");
		bt_stats->SetText(Text::GetText("INVENTORY_4", Game::options.language));
		//bt_stats->SetFontSize(12);
		bt_stats->SetAction(boost::bind(&Game::onMakeAction, game, this, wnd, ACTION_OPENWINDOW, (void*)WND_STATISTICS));

		WindowElementButton* bt_battle = new(M_PLACE) WindowElementButton;
		bt_battle->Create(0, BUTTON_NORMAL, 0, 0, 0, 0, wndTemplate);
		bt_battle->ApplyElementINI(iniWindow, "BUTTON_BATTLE");
		bt_battle->SetText(Text::GetText("INVENTORY_5", Game::options.language));
		//bt_battle->SetFontSize(12);
		bt_battle->SetAction(boost::bind(&Game::onMakeAction, game, this, wnd, ACTION_OPENWINDOW, (void*)WND_BATTLE));

		WindowElementButton* bt_vip = new(M_PLACE) WindowElementButton;
		bt_vip->Create(0, BUTTON_NORMAL, 0, 0, 0, 0, wndTemplate);
		bt_vip->ApplyElementINI(iniWindow, "BUTTON_VIP");
		bt_vip->SetText(Text::GetText("INVENTORY_6", Game::options.language));
		//bt_vip->SetFontSize(12);
		bt_vip->SetAction(boost::bind(&Game::onMakeAction, game, this, wnd, ACTION_OPENWINDOW, (void*)WND_VIPLIST));

		WindowElementButton* bt_error = new(M_PLACE) WindowElementButton;
		bt_error->Create(0, BUTTON_NORMAL, 0, 0, 0, 0, wndTemplate);
		bt_error->ApplyElementINI(iniWindow, "BUTTON_REPORTERROR");
		//bt_error->SetColor(1.0f, 0.7f, 0.0f);
		//bt_error->SetFontSize(12);
		bt_error->SetText(Text::GetText("INVENTORY_10", Game::options.language));
		bt_error->SetAction(boost::bind(&Game::onMakeAction, game, this, wnd, ACTION_OPENWINDOW, (void*)WND_REPORTERROR));

		wnd->AddElement(manaBar);
		wnd->AddElement(healthBar);
		wnd->AddElement(tx_cap);
		wnd->AddElement(tx_soul);
		wnd->AddElement(cont);
		wnd->AddElement(bt_stop);
		wnd->AddElement(bt_options);
		wnd->AddElement(bt_quests);
		wnd->AddElement(bt_logout);
		wnd->AddElement(bt_stats);
		wnd->AddElement(bt_battle);
		wnd->AddElement(bt_vip);
		wnd->AddElement(bt_error);

		if (!Game::adminOptions.disableBot) {
			WindowElementButton* bt_bot = new(M_PLACE) WindowElementButton;
			bt_bot->Create(0, BUTTON_NORMAL, 0, 0, 0, 0, wndTemplate);
			bt_bot->ApplyElementINI(iniWindow, "BUTTON_BOT");
			bt_bot->SetText(Text::GetText("INVENTORY_7", Game::options.language));
			//bt_bot->SetFontSize(12);
			bt_bot->SetAction(boost::bind(&Game::onMakeAction, game, this, wnd, ACTION_OPENWINDOW, (void*)WND_BOT));

			wnd->AddElement(bt_bot);
		}

		WindowElementContainer* equipment = new(M_PLACE) WindowElementContainer;
		equipment->Create(0, 0, 0, 0, 0, false, false, wndTemplate);
		equipment->ApplyElementINI(iniWindow, "EQUIPMENT");
		for (int i = 0; i < places; i++) {
			std::string equip = std::string("EQUIPMENT_") + value2str(i + 1);

			AD2D_Image* slotImage = Icons::GetSlotIcon(i + 1);

			WindowElementItemContainer* itemContainer = new(M_PLACE) WindowElementItemContainer;
			itemContainer->Create(0, 0, 0, 0, 0, container, wndTemplate);
			itemContainer->ApplyElementINI(iniWindow, equip);
			itemContainer->SetImage(slotImage);
			itemContainer->SetSlot(i);
			equipment->AddElement(itemContainer);
		}
		wnd->AddElement(equipment);

		wnd->AddCloseFunction(boost::bind(&GUIManager::RemoveWindow, &guiManager, wnd));
		AddWindow(wnd);
		guiManager.AddWindow(wnd);

		game->SetWindowInventory(wnd);
	}
	else if (type == WND_CONTAINER) {
		Game* game = va_arg(vl, Game*);
		Container* container = va_arg(vl, Container*);
		va_end(vl);

		INILoader iniWindow;
		if (!iniWindow.OpenFile(std::string("templates/") + Game::options.templatesGroup + std::string("/windows/container.ini")))
			return NULL;

		std::string templateName = iniWindow.GetValue("TEMPLATE");
		int wndWidth = atoi(iniWindow.GetValue("SIZE",0).c_str());
		int wndHeight = atoi(iniWindow.GetValue("SIZE",1).c_str());

		ContainerWindow* cntWnd = ContainerWindow::GetContainerWindow(container->GetIndex());

		std::string title = container->GetName();
		int places = container->GetPlaces();
		int size = std::max(4, (int)container->GetSize());

		WindowTemplate* wndTemplate = GetWindowTemplate(Game::options.templatesGroup, templateName);

		int posX = atoi(iniWindow.GetValue("ITEMS_SLOTS",0).c_str());
		int posY = atoi(iniWindow.GetValue("ITEMS_SLOTS",1).c_str());
		int width = atoi(iniWindow.GetValue("ITEMS_SLOTS",2).c_str());
		int height = atoi(iniWindow.GetValue("ITEMS_SLOTS",3).c_str());
		int spacingH = atoi(iniWindow.GetValue("ITEMS_SLOTS",4).c_str());
		int spacingV = atoi(iniWindow.GetValue("ITEMS_SLOTS",5).c_str());

		wndHeight = posY + (height + spacingV) * (places / 4 + (places % 4 ? 1 : 0)) + 10;
		int wndCurHeight = posY + height * (size / 4 + (size % 4 ? 1 : 0)) + 10;

		wnd = new(M_PLACE) ContainerWindow(type, 0, 0, container, wndTemplate);
		wnd->SetScrollAlwaysVisible(false, true);
		wnd->SetTitle(title);
		wnd->SetIntSize(wndWidth, wndHeight);
		wnd->AdjustSize();
		wnd->SetMinSize(wnd->width, wnd->minHeight);
		wnd->SetMaxSize(wnd->width, wnd->height);

		wnd->SetIntSize(wndWidth, wndCurHeight);
		wnd->AdjustSize();
		wnd->SetIntSize(wndWidth, wndHeight);

		WindowElementItemContainer* itemContainer = NULL;
		for (int i = 0; i < places; i++) {
			int pX = i % 4;
			int pY = i / 4;

			itemContainer = new(M_PLACE) WindowElementItemContainer;
			itemContainer->Create(0, posX + pX * (width + spacingH), posY + pY * (height + spacingV), width, height, container, wndTemplate);
			itemContainer->SetSlot(i);
			wnd->AddElement(itemContainer);
		}

		boost::function<void()> func = boost::bind(&Game::PlayerContainerClose, game, container->GetIndex());
		wnd->AddCloseFunction(func);

		if (cntWnd) {
			int wdh = wnd->width;
			int hgt = wnd->height;
			wnd->posX = cntWnd->posX;
			wnd->posY = cntWnd->posY;
			wnd->width = cntWnd->width;
			wnd->height = cntWnd->height;

			if (cntWnd->minimized)
				wnd->minimized = hgt;
			else {
				wnd->minimized = 0;
				wnd->SetSize(cntWnd->width, cntWnd->height);
			}

			ReplaceWindow(cntWnd, wnd);
		}
		else {
			POINT pos = GetFixedOpenPosition(wndSize.x - margin.y - wnd->width, wnd);
			wnd->SetPosition(pos.x, pos.y);

			AddWindow(wnd);
		}
	}
	else if (type == WND_CHANNELSLIST) {
		Game* game = va_arg(vl, Game*);
		va_end(vl);

		Host host = game->GetHost();

		INILoader iniWindow;
		if (!iniWindow.OpenFile(std::string("templates/") + Game::options.templatesGroup + std::string("/windows/channelslist.ini")))
			return NULL;

		std::string templateName = iniWindow.GetValue("TEMPLATE");
		int wndWidth = atoi(iniWindow.GetValue("SIZE",0).c_str());
		int wndHeight = atoi(iniWindow.GetValue("SIZE",1).c_str());

		WindowTemplate* wndTemplate = GetWindowTemplate(Game::options.templatesGroup, templateName);

		wnd = new(M_PLACE) Window(type, 0, 0, wndTemplate);
		wnd->SetTitle(Text::GetText("CHANNELSLIST_T", Game::options.language));
		wnd->SetIntSize(wndWidth, wndHeight);
		wnd->AdjustSize();
		wnd->SetMinSize(wnd->width, wnd->height);
		wnd->SetMaxSize(wnd->width, wnd->height);
		wnd->SetPosition(wndSize.x / 2 - wnd->width / 2, wndSize.y / 2 - wnd->height / 2);
		wnd->SetIntSize(0, 0);

		WindowElementMemo* me_list = new(M_PLACE) WindowElementMemo;
		me_list->Create(0, 0, 0, 0, 0, wndTemplate);
		me_list->ApplyElementINI(iniWindow, "MEMO_LIST");
		me_list->SetLocks(true, true);

		ChannelsList list = game->GetChannels();
		ChannelsList::iterator it = list.begin();
		for (it; it != list.end(); it++)
			me_list->AddElement(std::string(it->GetName()));

		WindowElementText* tx_private = new(M_PLACE) WindowElementText;
		tx_private->Create(ALIGN_V_BOTTOM, 0, 0, 0xFFFF, wndTemplate);
		tx_private->ApplyElementINI(iniWindow, "TEXT_PRIVATE");
		tx_private->SetText(Text::GetText("CHANNELSLIST_0", Game::options.language));

		WindowElementTextarea* ta_private = new(M_PLACE) WindowElementTextarea;
		ta_private->Create(ALIGN_V_BOTTOM, 0, 0, 0, 0, true, false, wndTemplate);
		ta_private->ApplyElementINI(iniWindow, "TEXTAREA_PRIVATE");
		ta_private->SetLocks(true, false);
		std::string* ptr = ta_private->GetTextPtr();

		WindowElementButton* bt_login = new(M_PLACE) WindowElementButton;
		bt_login->Create(ALIGN_H_RIGHT | ALIGN_V_BOTTOM, BUTTON_NORMAL, 0, 0, 0, 0, wndTemplate);
		bt_login->ApplyElementINI(iniWindow, "BUTTON_LOGIN");
		bt_login->SetText(Text::GetText("CHANNELSLIST_1", Game::options.language));

		boost::function<void()> func = boost::bind(&Game::onOpenChannel, game, this, wnd, &me_list->option, ptr);

		wnd->AddExecuteFunction(func);
		me_list->SetDblAction(func);
		ta_private->SetAction(func);
		bt_login->SetAction(func);

		wnd->AddElement(me_list);
		wnd->AddElement(tx_private);
		wnd->AddElement(ta_private);
		wnd->AddElement(bt_login);

		wnd->SetActiveElement(me_list);

		AddWindow(wnd);
	}
	else if (type == WND_COUNTER) {
		Game* game = va_arg(vl, Game*);
		ThingData* moveData = (ThingData*)va_arg(vl, void*);
		va_end(vl);

		INILoader iniWindow;
		if (!iniWindow.OpenFile(std::string("templates/") + Game::options.templatesGroup + std::string("/windows/counter.ini")))
			return NULL;

		std::string templateName = iniWindow.GetValue("TEMPLATE");
		int wndWidth = atoi(iniWindow.GetValue("SIZE",0).c_str());
		int wndHeight = atoi(iniWindow.GetValue("SIZE",1).c_str());

		WindowTemplate* wndTemplate = GetWindowTemplate(Game::options.templatesGroup, templateName);

		wnd = new(M_PLACE) Window(type, 0, 0, wndTemplate);
		wnd->SetTitle(Text::GetText("COUNTER_T", Game::options.language));
		wnd->SetIntSize(wndWidth, wndHeight);
		wnd->AdjustSize();
		wnd->SetMinSize(wnd->width, wnd->height);
		wnd->SetMaxSize(wnd->width, wnd->height);

		wnd->SetIntSize(0, 0);
		wnd->SetPosition(wndSize.x / 2 - wnd->width / 2, wndSize.y / 2 - wnd->height / 2);

		Item* item = new(M_PLACE) Item;
		item->SetID(moveData->itemId);
		item->SetCount(moveData->count);

		WindowElementItemContainer* itemContainer = new(M_PLACE) WindowElementItemContainer;
		itemContainer = new(M_PLACE) WindowElementItemContainer;
		itemContainer->Create(0, 0, 0, 0, 0, NULL, wndTemplate);
		itemContainer->ApplyElementINI(iniWindow, "ITEM_PREVIEW");
		itemContainer->SetItem(item);

		WindowElementScrollBar* scroll = new(M_PLACE) WindowElementScrollBar;
		scroll->Create(0, 0, 0, 0, wndTemplate);
		scroll->ApplyElementINI(iniWindow, "SCROLL");
		scroll->SetValuePtr(TypePointer("uint8", (void*)&item->count));
		scroll->SetMinValue(1.0f);
		scroll->SetMaxValue((float)moveData->count);
		scroll->SetAlwaysActive(true);
		moveData->scroll = &item->count;

		WindowElementButton* bt_acc = new(M_PLACE) WindowElementButton;
		bt_acc->Create(0, BUTTON_NORMAL, 0, 0, 0, 0, wndTemplate);
		bt_acc->ApplyElementINI(iniWindow, "BUTTON_ACCEPT");
		bt_acc->SetText(Text::GetText("COUNTER_0", Game::options.language));

		WindowElementButton* bt_rej = new(M_PLACE) WindowElementButton;
		bt_rej->Create(0, BUTTON_NORMAL, 0, 0, 0, 0, wndTemplate);
		bt_rej->ApplyElementINI(iniWindow, "BUTTON_REJECT");
		bt_rej->SetText(Text::GetText("COUNTER_1", Game::options.language));

		wnd->AddElement(itemContainer);
		wnd->AddElement(scroll);
		wnd->AddElement(bt_acc);
		wnd->AddElement(bt_rej);
		wnd->SetActiveElement(scroll);

		boost::function<void()> func1 = boost::bind(&Game::onMakeAction, game, this, wnd, ACTION_MOVETO, (void*)moveData);
		wnd->AddExecuteFunction(func1);
		bt_acc->SetAction(func1);

		boost::function<void()> func2 = boost::bind(&Window::SetAction, wnd, ACT_CLOSE);
		bt_rej->SetAction(func2);

		boost::function<void()> func3 = boost::bind(&Game::onMakeAction, game, this, (Window*)NULL, ACTION_MOVETO, (void*)moveData);
		wnd->AddCloseFunction(func3);

		AddWindow(wnd);
	}
	else if (type == WND_MINIMAP) {
		Game* game = va_arg(vl, Game*);
		va_end(vl);

		INILoader iniWindow;
		if (!iniWindow.OpenFile(std::string("templates/") + Game::options.templatesGroup + std::string("/windows/minimap.ini")))
			return NULL;

		std::string templateName = iniWindow.GetValue("TEMPLATE");
		int wndWidth = atoi(iniWindow.GetValue("SIZE",0).c_str());
		int wndHeight = atoi(iniWindow.GetValue("SIZE",1).c_str());

		WindowTemplate* wndTemplate = GetWindowTemplate(Game::options.templatesGroup, templateName);

		wnd = new(M_PLACE) Window(type, 0, 0, wndTemplate);
		wnd->SetTitle(Text::GetText("MINIMAP_T", Game::options.language));
		wnd->SetIntSize(wndWidth, wndHeight);
		wnd->AdjustSize();
		wnd->SetMinSize(wnd->width, wnd->height);
		wnd->SetMaxSize(wnd->width, wnd->height);

		POINT pos = GetFixedOpenPosition(wndSize.x - margin.y - wnd->width, wnd);
		wnd->SetPosition(pos.x, pos.y);

		Map* map = game->GetMap();
		MiniMap* minimap = map->GetMiniMap();

		WindowElementMiniMap* mmap = new(M_PLACE) WindowElementMiniMap;
		mmap->Create(0, 0, 0, wndWidth - 20, wndHeight - 20, minimap, wndTemplate);
		mmap->SetLocks(true, true);
		wnd->AddElement(mmap);

		WindowElementButton* bt_levelup = new(M_PLACE) WindowElementButton;
		bt_levelup->Create(0, BUTTON_NORMAL, wndWidth - 20, 0, 20, (wndHeight - 20) / 3, wndTemplate);
		bt_levelup->ApplyElementINI(iniWindow, "BUTTON_LEVELUP");
		bt_levelup->SetText("/\\");
		bt_levelup->SetAction(boost::bind(&Map::MoveMiniMapLevel, map, -1));
		wnd->AddElement(bt_levelup);

		WindowElementButton* bt_levelzero = new(M_PLACE) WindowElementButton;
		bt_levelzero->Create(0, BUTTON_NORMAL, wndWidth - 20, (wndHeight - 20) / 3, 20, (wndHeight - 20) / 3, wndTemplate);
		bt_levelzero->ApplyElementINI(iniWindow, "BUTTON_RESETLEVEL");
		bt_levelzero->SetText("-");
		bt_levelzero->SetAction(boost::bind(&Map::ResetMiniMapLevel, map));
		wnd->AddElement(bt_levelzero);

		WindowElementButton* bt_leveldown = new(M_PLACE) WindowElementButton;
		bt_leveldown->Create(0, BUTTON_NORMAL, wndWidth - 20, 2 * (wndHeight - 20) / 3, 20, (wndHeight - 20) / 3, wndTemplate);
		bt_levelzero->ApplyElementINI(iniWindow, "BUTTON_LEVELDOWN");
		bt_leveldown->SetText("\\/");
		bt_leveldown->SetAction(boost::bind(&Map::MoveMiniMapLevel, map, 1));
		wnd->AddElement(bt_leveldown);

		WindowElementButton* bt_zoomin = new(M_PLACE) WindowElementButton;
		bt_zoomin->Create(0, BUTTON_NORMAL, 0, wndHeight - 20, (wndWidth - 20) / 2, 20, wndTemplate);
		bt_zoomin->ApplyElementINI(iniWindow, "BUTTON_ZOOMIN");
		bt_zoomin->SetText("Zoom in");
		bt_zoomin->SetAction(boost::bind(&Map::ChangeMiniMapZoom, map, 1.25f));
		wnd->AddElement(bt_zoomin);

		WindowElementButton* bt_zoomout = new(M_PLACE) WindowElementButton;
		bt_zoomout->Create(0, BUTTON_NORMAL, (wndWidth - 20) / 2, wndHeight - 20, (wndWidth - 20) / 2, 20, wndTemplate);
		bt_zoomout->ApplyElementINI(iniWindow, "BUTTON_ZOOMOUT");
		bt_zoomout->SetText("Zoom out");
		bt_zoomout->SetAction(boost::bind(&Map::ChangeMiniMapZoom, map, 0.8f));
		wnd->AddElement(bt_zoomout);

		WindowElementButton* bt_reset = new(M_PLACE) WindowElementButton;
		bt_reset->Create(0, BUTTON_NORMAL, wndWidth - 20, wndHeight - 20, 20, 20, wndTemplate);
		bt_reset->ApplyElementINI(iniWindow, "BUTTON_RESET");
		bt_reset->SetText("X");
		bt_reset->SetAction(boost::bind(&Map::ResetMiniMapZoom, map));
		wnd->AddElement(bt_reset);

		wnd->AddCloseFunction(boost::bind(&GUIManager::RemoveWindow, &guiManager, wnd));
		AddWindow(wnd);
		guiManager.AddWindow(wnd);

		game->SetWindowMiniMap(wnd);
	}
	else if (type == WND_STATISTICS) {
		Game* game = va_arg(vl, Game*);
		va_end(vl);

		INILoader iniWindow;
		if (!iniWindow.OpenFile(std::string("templates/") + Game::options.templatesGroup + std::string("/windows/statistics.ini")))
			return NULL;

		std::string templateName = iniWindow.GetValue("TEMPLATE");
		int wndWidth = atoi(iniWindow.GetValue("SIZE",0).c_str());
		int wndHeight = atoi(iniWindow.GetValue("SIZE",1).c_str());

		WindowTemplate* wndTemplate = GetWindowTemplate(Game::options.templatesGroup, templateName);

		wnd = new(M_PLACE) Window(type, 0, 0, wndTemplate);
		wnd->SetScrollAlwaysVisible(false, true);
		wnd->SetTitle(Text::GetText("STATISTICS_T", Game::options.language));

		Player* player = game->GetPlayer();
		Statistics* stats = player->GetStatistics();

		int posY = 5;

		WindowElementText* tx_exp = new(M_PLACE) WindowElementText;
		tx_exp->Create(0, 5, posY, wndWidth - 10, wndTemplate);
		tx_exp->ApplyElementINI(iniWindow, "TEXT_EXPERIENCE");
		tx_exp->SetValuePtr(TypePointer("uint64", (void*)&stats->experience));
		tx_exp->SetText(Text::GetText("STATISTICS_0", Game::options.language));
		posY += 12;

		WindowElementText* tx_lvl = new(M_PLACE) WindowElementText;
		tx_lvl->Create(0, 5, posY, wndWidth - 10, wndTemplate);
		tx_lvl->ApplyElementINI(iniWindow, "TEXT_LEVEL");
		tx_lvl->SetValuePtr(TypePointer("uint16", (void*)&stats->level));
		tx_lvl->SetText(Text::GetText("STATISTICS_1", Game::options.language));
		posY += 14;

		WindowElementSkillBar* lvlBar = new(M_PLACE) WindowElementSkillBar;
		lvlBar->Create(0, 10, posY, wndWidth - 20, 10, TypePointer("uint8", (void*)&stats->level_p), wndTemplate);
		lvlBar->SetColor(1.0f, 0.0f, 0.0f);
		lvlBar->ApplyElementINI(iniWindow, "BAR_LEVEL");
		posY += 15;

		WindowElementText* tx_mag = new(M_PLACE) WindowElementText;
		tx_mag->Create(0, 5, posY, wndWidth - 10, wndTemplate);
		tx_mag->ApplyElementINI(iniWindow, "TEXT_MAGICLEVEL");
		tx_mag->SetValuePtr(TypePointer("uint16", (void*)&stats->magicLevel));
		tx_mag->SetText(Text::GetText("STATISTICS_2", Game::options.language));
		posY += 14;

		WindowElementSkillBar* magBar = new(M_PLACE) WindowElementSkillBar;
		magBar->Create(0, 10, posY, wndWidth - 20, 10, TypePointer("uint8", (void*)&stats->magicLevel_p), wndTemplate);
		magBar->SetColor(0.0f, 0.5f, 1.0f);
		magBar->ApplyElementINI(iniWindow, "BAR_MAGICLEVEL");
		posY += 15;

		WindowElementText* tx_hp = new(M_PLACE) WindowElementText;
		tx_hp->Create(0, 5, posY, wndWidth - 10, wndTemplate);
		tx_hp->ApplyElementINI(iniWindow, "TEXT_HEALTH");
		tx_hp->SetValuePtr(TypePointer("uint16", (void*)&stats->health));
		tx_hp->SetText(Text::GetText("STATISTICS_3", Game::options.language));
		posY += 12;

		WindowElementText* tx_mn = new(M_PLACE) WindowElementText;
		tx_mn->Create(0, 5, posY, wndWidth - 10, wndTemplate);
		tx_mn->ApplyElementINI(iniWindow, "TEXT_MANA");
		tx_mn->SetValuePtr(TypePointer("uint16", (void*)&stats->mana));
		tx_mn->SetText(Text::GetText("STATISTICS_4", Game::options.language));
		posY += 17;

		WindowElementText* tx_cap = new(M_PLACE) WindowElementText;
		tx_cap->Create(0, 5, posY, wndWidth - 10, wndTemplate);
		tx_cap->ApplyElementINI(iniWindow, "TEXT_CAPACITY");
		tx_cap->SetValuePtr(TypePointer("float", (void*)&stats->capacity));
		tx_cap->SetText(Text::GetText("STATISTICS_5", Game::options.language));
		posY += 12;

		WindowElementText* tx_sou = new(M_PLACE) WindowElementText;
		tx_sou->Create(0, 5, posY, wndWidth - 10, wndTemplate);
		tx_sou->ApplyElementINI(iniWindow, "TEXT_SOUL");
		tx_sou->SetValuePtr(TypePointer("uint8", (void*)&stats->soul));
		tx_sou->SetText(Text::GetText("STATISTICS_6", Game::options.language));
		posY += 12;

		WindowElementText* tx_sta = new(M_PLACE) WindowElementText;
		tx_sta->Create(0, 5, posY, wndWidth - 10, wndTemplate);
		tx_sta->ApplyElementINI(iniWindow, "TEXT_STAMINA");
		tx_sta->SetFlags(FLAG_TEXTTIME);
		tx_sta->SetValuePtr(TypePointer("uint16", (void*)&stats->stamina));
		tx_sta->SetText(Text::GetText("STATISTICS_7", Game::options.language));
		posY += 14;

		WindowElementSkillBar* staminaBar = new(M_PLACE) WindowElementSkillBar;
		staminaBar->Create(0, 10, posY, wndWidth - 20, 10, TypePointer("uint16", (void*)&stats->stamina), wndTemplate);
		staminaBar->SetColor(1.0f, 0.7f, 0.0f);
		staminaBar->ApplyElementINI(iniWindow, "BAR_STAMINA");
		staminaBar->SetMaxValue(3360);
		posY += 20;

		wnd->AddElement(tx_exp);
		wnd->AddElement(tx_lvl);
		wnd->AddElement(lvlBar);
		wnd->AddElement(tx_mag);
		wnd->AddElement(magBar);
		wnd->AddElement(tx_hp);
		wnd->AddElement(tx_mn);
		wnd->AddElement(tx_cap);
		wnd->AddElement(tx_sou);
		wnd->AddElement(tx_sta);
		wnd->AddElement(staminaBar);

		for (int i = 0; i < 7; i++) {
			WindowElementText* tx_skill = new(M_PLACE) WindowElementText;
			tx_skill->Create(0, 5, posY, wndWidth - 10, wndTemplate);
			tx_skill->ApplyElementINI(iniWindow, "TEXT_SKILL_" + value2str(i + 1));
			tx_skill->SetValuePtr(TypePointer("uint8", (void*)&stats->skill[i]));
			if (i == 0) tx_skill->SetText(Text::GetText("STATISTICS_SKILL1", Game::options.language));
			else if (i == 1) tx_skill->SetText(Text::GetText("STATISTICS_SKILL2", Game::options.language));
			else if (i == 2) tx_skill->SetText(Text::GetText("STATISTICS_SKILL3", Game::options.language));
			else if (i == 3) tx_skill->SetText(Text::GetText("STATISTICS_SKILL4", Game::options.language));
			else if (i == 4) tx_skill->SetText(Text::GetText("STATISTICS_SKILL5", Game::options.language));
			else if (i == 5) tx_skill->SetText(Text::GetText("STATISTICS_SKILL6", Game::options.language));
			else if (i == 6) tx_skill->SetText(Text::GetText("STATISTICS_SKILL7", Game::options.language));
			else tx_skill->SetText("Skill:");
			posY += 14;

			WindowElementSkillBar* skillBar = new(M_PLACE) WindowElementSkillBar;
			skillBar->Create(0, 10, posY, wndWidth - 20, 10, TypePointer("uint8", (void*)&stats->skill_p[i]), wndTemplate);
			skillBar->SetColor(0.0f, 1.0f, 0.0f);
			skillBar->ApplyElementINI(iniWindow, "BAR_SKILL_" + value2str(i + 1));
			posY += 15;

			wnd->AddElement(tx_skill);
			wnd->AddElement(skillBar);
		}

		wnd->SetIntSize(wndWidth, wndHeight);
		wnd->AdjustSize();
		wnd->SetMinSize(wnd->width, wnd->height);
		wnd->SetIntSize(wndWidth, posY);
		wnd->AdjustSize();
		wnd->SetMaxSize(wnd->width, wnd->height);

		wnd->SetIntSize(wndWidth, wndHeight);
		wnd->AdjustSize();
		wnd->SetIntSize(wndWidth, posY);

		POINT pos = GetFixedOpenPosition(wndSize.x - margin.y - wnd->width, wnd);
		wnd->SetPosition(pos.x, pos.y);

		wnd->AddCloseFunction(boost::bind(&GUIManager::RemoveWindow, &guiManager, wnd));
		AddWindow(wnd);
		guiManager.AddWindow(wnd);

		game->SetWindowStatistics(wnd);
	}
	else if (type == WND_BATTLE) {
		Game* game = va_arg(vl, Game*);
		va_end(vl);

		INILoader iniWindow;
		if (!iniWindow.OpenFile(std::string("templates/") + Game::options.templatesGroup + std::string("/windows/battle.ini")))
			return NULL;

		std::string templateName = iniWindow.GetValue("TEMPLATE");
		int wndWidth = atoi(iniWindow.GetValue("SIZE",0).c_str());
		int wndHeight = atoi(iniWindow.GetValue("SIZE",1).c_str());

		WindowTemplate* wndTemplate = GetWindowTemplate(Game::options.templatesGroup, templateName);

		wnd = new(M_PLACE) Window(type, 0, 0, wndTemplate);
		wnd->SetScrollAlwaysVisible(false, true);
		wnd->SetTitle(Text::GetText("BATTLE_T", Game::options.language));
		wnd->SetIntSize(wndWidth, wndHeight);
		wnd->AdjustSize();
		wnd->SetMinSize(wnd->width, 64);
		wnd->SetMaxSize(wnd->width, 0xFFFF);
		wnd->SetIntSize(0, 0);

		Map* map = game->GetMap();
		Battle* battle = (map ? map->GetBattle() : NULL);
		if (battle) {
			battle->SetContainer(wnd->GetWindowContainer());
			wnd->AddCloseFunction(boost::bind(&Battle::SetContainer, battle, (WindowElementContainer*)NULL));
			battle->UpdateContainer();
		}

		POINT pos = GetFixedOpenPosition(wndSize.x - margin.y - wnd->width, wnd);
		wnd->SetPosition(pos.x, pos.y);

		wnd->AddCloseFunction(boost::bind(&GUIManager::RemoveWindow, &guiManager, wnd));
		AddWindow(wnd);
		guiManager.AddWindow(wnd);

		game->SetWindowBattle(wnd);
	}
	else if (type == WND_VIPLIST) {
		Game* game = va_arg(vl, Game*);
		va_end(vl);

		INILoader iniWindow;
		if (!iniWindow.OpenFile(std::string("templates/") + Game::options.templatesGroup + std::string("/windows/viplist.ini")))
			return NULL;

		std::string templateName = iniWindow.GetValue("TEMPLATE");
		int wndWidth = atoi(iniWindow.GetValue("SIZE",0).c_str());
		int wndHeight = atoi(iniWindow.GetValue("SIZE",1).c_str());

		WindowTemplate* wndTemplate = GetWindowTemplate(Game::options.templatesGroup, templateName);

		wnd = new(M_PLACE) Window(type, 0, 0, wndTemplate);
		wnd->SetScrollAlwaysVisible(false, true);
		wnd->SetTitle(Text::GetText("VIPLIST_T", Game::options.language));
		wnd->SetIntSize(wndWidth, wndHeight);
		wnd->AdjustSize();
		wnd->SetMinSize(wnd->width, 64);
		wnd->SetMaxSize(wnd->width, 0xFFFF);
		wnd->SetIntSize(0, 0);

		WindowElementVIP* vip = new(M_PLACE) WindowElementVIP;
		vip->Create(0, 5, 0, 0xFFFF, 0xFFFF, false, false, wndTemplate);
		vip->SetCreatureID(0x00);

		wnd->AddElement(vip);

		VIPList* viplist = game->GetVIPList();
		if (viplist) {
			viplist->SetContainer(wnd->GetWindowContainer());
			wnd->AddCloseFunction(boost::bind(&VIPList::SetContainer, viplist, (WindowElementContainer*)NULL));
			viplist->UpdateContainer();
		}

		POINT pos = GetFixedOpenPosition(wndSize.x - margin.y - wnd->width, wnd);
		wnd->SetPosition(pos.x, pos.y);

		wnd->AddCloseFunction(boost::bind(&GUIManager::RemoveWindow, &guiManager, wnd));
		AddWindow(wnd);
		guiManager.AddWindow(wnd);

		game->SetWindowVIPList(wnd);
	}
	else if (type == WND_ADDVIP) {
		Game* game = va_arg(vl, Game*);
		va_end(vl);

		INILoader iniWindow;
		if (!iniWindow.OpenFile(std::string("templates/") + Game::options.templatesGroup + std::string("/windows/addvip.ini")))
			return NULL;

		std::string templateName = iniWindow.GetValue("TEMPLATE");
		int wndWidth = atoi(iniWindow.GetValue("SIZE",0).c_str());
		int wndHeight = atoi(iniWindow.GetValue("SIZE",1).c_str());

		WindowTemplate* wndTemplate = GetWindowTemplate(Game::options.templatesGroup, templateName);

		wnd = new(M_PLACE) Window(type, 0, 0, wndTemplate);
		wnd->SetTitle(Text::GetText("ADDVIP_T", Game::options.language));
		wnd->SetIntSize(wndWidth, wndHeight);
		wnd->AdjustSize();
		wnd->SetIntSize(0, 0);
		wnd->SetMinSize(wnd->width, wnd->height);
		wnd->SetMaxSize(wnd->width, wnd->height);

		wnd->SetPosition(wndSize.x / 2 - wnd->width / 2, wndSize.y / 2 - wnd->height / 2);

		WindowElementTextarea* line = new(M_PLACE) WindowElementTextarea;
		line->Create(0, 0, 0, 0, 0, true, false, wndTemplate);
		line->ApplyElementINI(iniWindow, "TEXTAREA");
		line->SetLocks(true, false);
		std::string* ptr = line->GetTextPtr();

		WindowElementButton* button = new(M_PLACE) WindowElementButton;
		button->Create(ALIGN_H_RIGHT, BUTTON_NORMAL, 0, 0, 0, 0, wndTemplate);
		button->ApplyElementINI(iniWindow, "BUTTON_ADD");
		button->SetText(Text::GetText("ADDVIP_0", Game::options.language));

		boost::function<void()> func = boost::bind(&Game::onAddVIP, game, this, wnd, ptr);
		line->SetAction(func);
		button->SetAction(func);

		wnd->AddElement(line);
		wnd->AddElement(button);

		line->active = true;
		wnd->SetActiveElement(line);

		AddWindow(wnd);
	}
	else if (type == WND_TEXT) {
		Game* game = va_arg(vl, Game*);
		unsigned int textID = va_arg(vl, unsigned int);
		unsigned short itemID = va_arg(vl, int);
		std::string* text = va_arg(vl, std::string*);
		std::string* writer = va_arg(vl, std::string*);
		std::string* date = va_arg(vl, std::string*);
		unsigned int maxLength = va_arg(vl, unsigned int);
		va_end(vl);

		INILoader iniWindow;
		if (!iniWindow.OpenFile(std::string("templates/") + Game::options.templatesGroup + std::string("/windows/text.ini")))
			return NULL;

		std::string templateName = iniWindow.GetValue("TEMPLATE");
		int wndWidth = atoi(iniWindow.GetValue("SIZE",0).c_str());
		int wndHeight = atoi(iniWindow.GetValue("SIZE",1).c_str());

		WindowTemplate* wndTemplate = GetWindowTemplate(Game::options.templatesGroup, templateName);

		wnd = new(M_PLACE) Window(type, 0, 0, wndTemplate);
		wnd->SetTitle(Text::GetText("TEXT_T", Game::options.language));
		wnd->SetIntSize(wndWidth, wndHeight);
		wnd->AdjustSize();
		wnd->SetIntSize(0, 0);
		wnd->SetMinSize(wnd->width, wnd->height);
		wnd->SetMaxSize(0xFFFF, 0xFFFF);

		wnd->SetPosition(wndSize.x / 2 - wnd->width / 2, wndSize.y / 2 - wnd->height / 2);

		WindowElementTextarea* ta_text = new(M_PLACE) WindowElementTextarea;
		ta_text->Create(0, 0, 0, 0, 0, true, true, wndTemplate);
		ta_text->ApplyElementINI(iniWindow, "TEXTAREA");
		ta_text->SetLocks(true, true);
		ta_text->SetText(*text);
		ta_text->SetMaxLength(maxLength);
		std::string* ptr = ta_text->GetTextPtr();

		WindowElementButton* button = new(M_PLACE) WindowElementButton;
		button->Create(ALIGN_H_RIGHT, BUTTON_NORMAL, 0, 0, 0, 0, wndTemplate);
		button->ApplyElementINI(iniWindow, "BUTTON_OK");
		button->SetText(Text::GetText("TEXT_0", Game::options.language));

		boost::function<void()> func = boost::bind(&Game::onTextWindow, game, this, wnd, textID, ptr);
		button->SetAction(func);

		wnd->AddElement(ta_text);
		wnd->AddElement(button);

		ta_text->active = true;
		wnd->SetActiveElement(ta_text);

		AddWindow(wnd);
	}
	else if (type == WND_HOUSETEXT) {
		Game* game = va_arg(vl, Game*);
		unsigned char listID = va_arg(vl, int);
		unsigned int textID = va_arg(vl, unsigned int);
		unsigned short itemID = va_arg(vl, int);
		std::string* text = va_arg(vl, std::string*);
		unsigned int maxLength = va_arg(vl, unsigned int);
		va_end(vl);

		INILoader iniWindow;
		if (!iniWindow.OpenFile(std::string("templates/") + Game::options.templatesGroup + std::string("/windows/housetext.ini")))
			return NULL;

		std::string templateName = iniWindow.GetValue("TEMPLATE");
		int wndWidth = atoi(iniWindow.GetValue("SIZE",0).c_str());
		int wndHeight = atoi(iniWindow.GetValue("SIZE",1).c_str());

		WindowTemplate* wndTemplate = GetWindowTemplate(Game::options.templatesGroup, templateName);

		wnd = new(M_PLACE) Window(type, 0, 0, wndTemplate);
		wnd->SetTitle(Text::GetText("HOUSETEXT_T", Game::options.language));
		wnd->SetIntSize(192, 240);
		wnd->AdjustSize();
		wnd->SetIntSize(0, 0);
		wnd->SetMinSize(wnd->width, wnd->height);
		wnd->SetMaxSize(0xFFFF, 0xFFFF);

		wnd->SetPosition(wndSize.x / 2 - wnd->width / 2, wndSize.y / 2 - wnd->height / 2);

		WindowElementTextarea* ta_text = new(M_PLACE) WindowElementTextarea;
		ta_text->Create(0, 0, 0, 0, 0, true, true, wndTemplate);
		ta_text->ApplyElementINI(iniWindow, "TEXTAREA");
		ta_text->SetLocks(true, true);
		ta_text->SetText(*text);
		ta_text->SetMaxLength(maxLength);
		std::string* ptr = ta_text->GetTextPtr();

		WindowElementButton* button = new(M_PLACE) WindowElementButton;
		button->Create(ALIGN_H_RIGHT, BUTTON_NORMAL, 0, 0, 0, 0, wndTemplate);
		button->ApplyElementINI(iniWindow, "BUTTON_OK");
		button->SetText(Text::GetText("HOUSETEXT_0", Game::options.language));

		boost::function<void()> func = boost::bind(&Game::onHouseWindow, game, this, wnd, listID, textID, ptr);
		button->SetAction(func);

		wnd->AddElement(ta_text);
		wnd->AddElement(button);

		ta_text->active = true;
		wnd->SetActiveElement(ta_text);

		AddWindow(wnd);
	}
	else if (type == WND_TRADE) {
		Game* game = va_arg(vl, Game*);
		va_end(vl);

		INILoader iniWindow;
		if (!iniWindow.OpenFile(std::string("templates/") + Game::options.templatesGroup + std::string("/windows/trade.ini")))
			return NULL;

		std::string templateName = iniWindow.GetValue("TEMPLATE");
		int wndWidth = atoi(iniWindow.GetValue("SIZE",0).c_str());
		int wndHeight = atoi(iniWindow.GetValue("SIZE",1).c_str());

		WindowTemplate* wndTemplate = GetWindowTemplate(Game::options.templatesGroup, templateName);

		wnd = new(M_PLACE) Window(type, 0, 0, wndTemplate);
		wnd->SetTitle(Text::GetText("TRADE_T", Game::options.language));
		wnd->SetIntSize(wndWidth, wndHeight);
		wnd->AdjustSize();
		wnd->SetIntSize(0, 0);
		wnd->SetMinSize(wnd->width, wnd->height);
		wnd->SetMaxSize(wnd->width, 0xFFFF);

		wnd->SetPosition(wndSize.x / 2 - wnd->width / 2, wndSize.y / 2 - wnd->height / 2);

		WindowElementContainer* container = new(M_PLACE) WindowElementContainer;
		container->Create(0, 0, 0, 0, 0, false, false, wndTemplate);
		container->ApplyElementINI(iniWindow, "TRADE_CONTAINER");
		container->SetLocks(false, true);

		WindowElementButton* accept = new(M_PLACE) WindowElementButton;
		accept->Create(ALIGN_V_BOTTOM, BUTTON_SWITCH, 0, 0, 0, 0, wndTemplate);
		accept->ApplyElementINI(iniWindow, "BUTTON_ACCEPT");
		accept->SetVisible(false);
		accept->SetText(Text::GetText("TRADE_0", Game::options.language));
		accept->SetAction(boost::bind(&Game::onTrade, game, this, (Window*)NULL, accept, false));

		WindowElementButton* reject = new(M_PLACE) WindowElementButton;
		reject->Create(ALIGN_V_BOTTOM, BUTTON_NORMAL, 0, 0, 0, 0, wndTemplate);
		reject->ApplyElementINI(iniWindow, "BUTTON_REJECT");
		reject->SetText(Text::GetText("TRADE_1", Game::options.language));
		reject->SetAction(boost::bind(&Game::onTrade, game, this, wnd, accept, true));

		wnd->AddElement(container);
		wnd->AddElement(accept);
		wnd->AddElement(reject);

		Trade* trade = game->GetTrade();
		if (trade) {
			trade->SetButton(accept);
			trade->SetContainer(container);
			wnd->AddCloseFunction(boost::bind(&Game::onTrade, game, this, (Window*)NULL, accept, true));
		}

		AddWindow(wnd);
	}
	else if (type == WND_SHOP) {
		Game* game = va_arg(vl, Game*);
		va_end(vl);

		INILoader iniWindow;
		if (!iniWindow.OpenFile(std::string("templates/") + Game::options.templatesGroup + std::string("/windows/shop.ini")))
			return NULL;

		std::string templateName = iniWindow.GetValue("TEMPLATE");
		int wndWidth = atoi(iniWindow.GetValue("SIZE",0).c_str());
		int wndHeight = atoi(iniWindow.GetValue("SIZE",1).c_str());

		WindowTemplate* wndTemplate = GetWindowTemplate(Game::options.templatesGroup, templateName);

		Shop* shop = game->GetShop();
		std::string name = shop->GetName();
		std::string title = Text::GetText("SHOP_T", Game::options.language) + (name.length() > 0 ? std::string(": ") + name : std::string(""));

		wnd = new(M_PLACE) Window(type, 0, 0, wndTemplate);
		wnd->SetTitle(title);
		wnd->SetIntSize(wndWidth, wndHeight);
		wnd->AdjustSize();
		wnd->SetIntSize(0, 0);
		wnd->SetMinSize(wnd->width, wnd->height);
		wnd->SetMaxSize(wnd->width, 0xFFFF);

		wnd->SetPosition(wndSize.x / 2 - wnd->width / 2, wndSize.y / 2 - wnd->height / 2);

		WindowElementButton* bt_buy = new(M_PLACE) WindowElementButton;
		bt_buy->Create(0, BUTTON_SWITCH, 0, 0, 0, 0, wndTemplate);
		bt_buy->ApplyElementINI(iniWindow, "BUTTON_BUY");
		bt_buy->SetText(Text::GetText("SHOP_0", Game::options.language));
		bt_buy->SetPressed(true);

		WindowElementButton* bt_sell = new(M_PLACE) WindowElementButton;
		bt_sell->Create(0, BUTTON_SWITCH, 0, 0, 0, 0, wndTemplate);
		bt_sell->ApplyElementINI(iniWindow, "BUTTON_SELL");
		bt_sell->SetText(Text::GetText("SHOP_1", Game::options.language));

		WindowElementMemo* memo = new(M_PLACE) WindowElementMemo;
		memo->Create(0, 0, 0, 0, 0, wndTemplate);
		memo->ApplyElementINI(iniWindow, "MEMO");
		memo->SetLocks(false, true);

		unsigned char* count = new(M_PLACE) unsigned char;
		unsigned int* price = new(M_PLACE) unsigned int;
		unsigned int* money = shop->GetMoneyPtr();
		bool ignoreCap = shop->GetIgnoreCap();
		bool inBackpack = shop->GetInBackpack();
		*count = 0;
		*price = 0;

		WindowElementScrollBar* scrollBar = new(M_PLACE) WindowElementScrollBar;
		scrollBar->Create(ALIGN_V_BOTTOM, 0, 0, 0, wndTemplate);
		scrollBar->ApplyElementINI(iniWindow, "SCROLL");
		scrollBar->SetValuePtr(TypePointer("uint8", (void*)count));
		scrollBar->SetMinValue(1);
		scrollBar->SetMaxValue(0);

		WindowElementText* tx_amount = new(M_PLACE) WindowElementText;
		tx_amount->Create(ALIGN_V_BOTTOM, 0, 0, 0, wndTemplate);
		tx_amount->ApplyElementINI(iniWindow, "TEXT_AMMOUNT");
		tx_amount->SetText(Text::GetText("SHOP_2", Game::options.language));
		tx_amount->SetValuePtr(TypePointer("uint8", (void*)count));

		WindowElementText* tx_price = new(M_PLACE) WindowElementText;
		tx_price->Create(ALIGN_V_BOTTOM, 0, 0, 0, wndTemplate);
		tx_price->ApplyElementINI(iniWindow, "TEXT_PRICE");
		tx_price->SetText(Text::GetText("SHOP_3", Game::options.language));
		tx_price->SetValuePtr(TypePointer("uint16", (void*)price));

		WindowElementText* tx_money = new(M_PLACE) WindowElementText;
		tx_money->Create(ALIGN_V_BOTTOM, 0, 0, 0, wndTemplate);
		tx_money->ApplyElementINI(iniWindow, "TEXT_MONEY");
		tx_money->SetText(Text::GetText("SHOP_4", Game::options.language));
		tx_money->SetValuePtr(TypePointer("uint32", (void*)money));

		WindowElementItemContainer* itemCont = new(M_PLACE) WindowElementItemContainer;
		itemCont->Create(ALIGN_V_BOTTOM, 0, 0, 0, 0, NULL, wndTemplate);
		itemCont->ApplyElementINI(iniWindow, "ITEM_PREVIEW");

		WindowElementCheckBox* cb_ignorecap = new(M_PLACE) WindowElementCheckBox;
		cb_ignorecap->Create(ALIGN_V_BOTTOM, 0, 0, 0, wndTemplate);
		cb_ignorecap->ApplyElementINI(iniWindow, "CHECKBOX_IGNORECAP");
		cb_ignorecap->SetText(Text::GetText("SHOP_5", Game::options.language));
		cb_ignorecap->SetState(ignoreCap);

		WindowElementCheckBox* cb_inbackpack = new(M_PLACE) WindowElementCheckBox;
		cb_inbackpack->Create(ALIGN_V_BOTTOM, 0, 0, 0, wndTemplate);
		cb_inbackpack->ApplyElementINI(iniWindow, "CHECKBOX_INBACKPACK");
		cb_inbackpack->SetText(Text::GetText("SHOP_6", Game::options.language));
		cb_inbackpack->SetState(inBackpack);

		WindowElementButton* bt_ok = new(M_PLACE) WindowElementButton;
		bt_ok->Create(ALIGN_V_BOTTOM, BUTTON_NORMAL, 0, 0, 0, 0, wndTemplate);
		bt_ok->ApplyElementINI(iniWindow, "BUTTON_OK");
		bt_ok->SetText(Text::GetText("SHOP_7", Game::options.language));

		std::vector<void*> pointers;
		pointers.push_back((void*)memo);
		pointers.push_back((void*)scrollBar);
		pointers.push_back((void*)itemCont);
		pointers.push_back((void*)bt_buy);
		pointers.push_back((void*)bt_sell);
		pointers.push_back((void*)cb_ignorecap);
		pointers.push_back((void*)cb_inbackpack);
		pointers.push_back((void*)count);
		pointers.push_back((void*)price);
		pointers.push_back((void*)money);

		scrollBar->SetAction(boost::bind(&Game::onShop, game, this, (Window*)NULL, pointers, 0, true, false, false));
		memo->SetAction(boost::bind(&Game::onShop, game, this, (Window*)NULL, pointers, 0, false, false, false));
		bt_buy->SetAction(boost::bind(&Game::onShop, game, this, (Window*)NULL, pointers, 1, false, false, false));
		bt_sell->SetAction(boost::bind(&Game::onShop, game, this, (Window*)NULL, pointers, 2, false, false, false));
		bt_ok->SetAction(boost::bind(&Game::onShop, game, this, (Window*)NULL, pointers, 0, false, true, false));
		cb_ignorecap->SetAction(boost::bind(&Game::onShop, game, this, (Window*)NULL, pointers, 0, false, false, true));
		cb_inbackpack->SetAction(boost::bind(&Game::onShop, game, this, (Window*)NULL, pointers, 0, false, false, true));

		ShopItemsList purchaseItems = shop->GetPurchaseItems();
		ShopItemsList::iterator it = purchaseItems.begin();
		for (it; it != purchaseItems.end(); it++) {
			ShopItem purchaseItem = *it;

			ShopDetailItem item = shop->GetShopDetailItemByID(purchaseItem.itemID, purchaseItem.count);
			std::string text = item.name + " / " + value2str(item.buyPrice) + " gp / " + float2str((float)item.weight / 100) + "oz.";

			memo->AddElement(text);
		}
		memo->SetOption(0);

		shop->SetMemo(memo);
		shop->SetPurchase(true);

		wnd->AddElement(bt_buy);
		wnd->AddElement(bt_sell);
		wnd->AddElement(memo);
		wnd->AddElement(scrollBar);
		wnd->AddElement(tx_amount);
		wnd->AddElement(tx_price);
		wnd->AddElement(tx_money);
		wnd->AddElement(itemCont);
		wnd->AddElement(cb_ignorecap);
		wnd->AddElement(cb_inbackpack);
		wnd->AddElement(bt_ok);

		wnd->AddExecuteFunction(boost::bind(&Game::onShop, game, this, (Window*)NULL, pointers, 0, false, true, false));
		wnd->AddCloseFunction(boost::bind(&Game::onShop, game, this, wnd, pointers, 0, false, false, false));

		AddWindow(wnd);
	}
	else if (type == WND_CHANGEOUTFIT) {
		Game* game = va_arg(vl, Game*);
		Outfit* outfit = va_arg(vl, Outfit*);
		std::vector<OutfitType>* outfits = va_arg(vl, std::vector<OutfitType>*);
		std::vector<OutfitType>* mounts = va_arg(vl, std::vector<OutfitType>*);
		va_end(vl);

		unsigned char o_it = 0;
		for (o_it = 0; o_it < outfits->size(); o_it++)
			if (outfit->lookType == (*outfits)[o_it].lookType)
				break;
		if (o_it == outfits->size()) o_it = 0;

		unsigned char m_it = 0;
		for (m_it = 0; m_it < mounts->size(); m_it++)
			if (outfit->lookMount == (*mounts)[m_it].lookType)
				break;
		if (m_it == mounts->size()) m_it = 0;

		INILoader iniWindow;
		if (!iniWindow.OpenFile(std::string("templates/") + Game::options.templatesGroup + std::string("/windows/changeoutfit.ini")))
			return NULL;

		std::string templateName = iniWindow.GetValue("TEMPLATE");
		int wndWidth = atoi(iniWindow.GetValue("SIZE",0).c_str());
		int wndHeight = atoi(iniWindow.GetValue("SIZE",1).c_str());

		WindowTemplate* wndTemplate = GetWindowTemplate(Game::options.templatesGroup, templateName);

		wnd = new(M_PLACE) Window(type, 0, 0, wndTemplate);
		wnd->SetTitle(Text::GetText("CHANGEOUTFIT_T", Game::options.language));
		wnd->SetIntSize(wndWidth, wndHeight);
		wnd->AdjustSize();
		wnd->SetMinSize(wnd->width, wnd->height);
		wnd->SetMaxSize(wnd->width, wnd->height);
		wnd->SetIntSize(0, 0);

		Outfit outfit_;

		outfit_ = *outfit;
		outfit_.lookMount = 0;
		Creature* outfitCreature = new(M_PLACE) Creature;
		outfitCreature->SetDirection(SOUTH);
		outfitCreature->SetOutfit(outfit_);

		outfit_.lookType = (mounts->size() > 0 ? (*mounts)[m_it].lookType : 0);
		outfit_.lookMount = 0;
		outfit_.lookAddons = 0;
		Creature* mountCreature = new(M_PLACE) Creature;
		mountCreature->SetDirection(SOUTH);
		mountCreature->SetOutfit(outfit_);

		WindowElementItemContainer* outfitCont = new(M_PLACE) WindowElementItemContainer;
		outfitCont->Create(0, 0, 0, 0, 0, NULL, wndTemplate);
		outfitCont->ApplyElementINI(iniWindow, "OUTFIT_PREVIEW");
		outfitCont->SetCreature(outfitCreature);

		WindowElementItemContainer* mountCont = new(M_PLACE) WindowElementItemContainer;
		mountCont->Create(0, 0, 0, 0, 0, NULL, wndTemplate);
		mountCont->ApplyElementINI(iniWindow, "MOUNT_PREVIEW");
		mountCont->SetCreature(mountCreature);

		WindowElementScrollBar* outfitScroll = new(M_PLACE) WindowElementScrollBar;
		outfitScroll->Create(0, 0, 0, 0, wndTemplate);
		outfitScroll->ApplyElementINI(iniWindow, "SCROLL_OUTFITS");
		if (outfits->size() > 0) {
			unsigned char* outfitIt = new(M_PLACE) unsigned char;
			*outfitIt = o_it;

			outfitScroll->SetValuePtr(TypePointer("uint8", (void*)outfitIt));
			outfitScroll->SetMinValue(0);
			outfitScroll->SetMaxValue(outfits->size() - 1);
		}

		WindowElementScrollBar* mountScroll = new(M_PLACE) WindowElementScrollBar;
		mountScroll->Create(0, 0, 0, 0, wndTemplate);
		mountScroll->ApplyElementINI(iniWindow, "SCROLL_MOUNTS");
		if (mounts->size() > 0) {
			unsigned char* mountIt = new(M_PLACE) unsigned char;
			*mountIt = m_it;

			mountScroll->SetValuePtr(TypePointer("uint8", (void*)mountIt));
			mountScroll->SetMinValue(0);
			mountScroll->SetMaxValue(mounts->size() - 1);
		}

		WindowElementText* outfitText = new(M_PLACE) WindowElementText;
		outfitText->Create(0, 0, 0, 0xFFFF, wndTemplate);
		outfitText->ApplyElementINI(iniWindow, "TEXT_OUTFIT");
		outfitText->SetText((outfits->size() > 0 ? (*outfits)[o_it].name : Text::GetText("CHANGEOUTFIT_0", Game::options.language)));

		WindowElementText* mountText = new(M_PLACE) WindowElementText;
		mountText->Create(0, 0, 0, 0xFFFF, wndTemplate);
		mountText->ApplyElementINI(iniWindow, "TEXT_MOUNT");
		mountText->SetText((mounts->size() > 0 ? (*mounts)[m_it].name : Text::GetText("CHANGEOUTFIT_1", Game::options.language)));

		WindowElementButton* bt_head = new(M_PLACE) WindowElementButton;
		bt_head->Create(0, BUTTON_SWITCH, 0, 0, 0, 0, wndTemplate);
		bt_head->ApplyElementINI(iniWindow, "BUTTON_HEAD");
		bt_head->SetText(Text::GetText("CHANGEOUTFIT_2", Game::options.language));
		bt_head->SetPressed(true);
		bt_head->SetInverse(true);

		WindowElementButton* bt_body = new(M_PLACE) WindowElementButton;
		bt_body->Create(0, BUTTON_SWITCH, 0, 0, 0, 0, wndTemplate);
		bt_body->ApplyElementINI(iniWindow, "BUTTON_BODY");
		bt_body->SetText(Text::GetText("CHANGEOUTFIT_3", Game::options.language));
		bt_body->SetInverse(true);

		WindowElementButton* bt_legs = new(M_PLACE) WindowElementButton;
		bt_legs->Create(0, BUTTON_SWITCH, 0, 0, 0, 0, wndTemplate);
		bt_legs->ApplyElementINI(iniWindow, "BUTTON_LEGS");
		bt_legs->SetText(Text::GetText("CHANGEOUTFIT_4", Game::options.language));
		bt_legs->SetInverse(true);

		WindowElementButton* bt_feet = new(M_PLACE) WindowElementButton;
		bt_feet->Create(0, BUTTON_SWITCH, 0, 0, 0, 0, wndTemplate);
		bt_feet->ApplyElementINI(iniWindow, "BUTTON_FEET");
		bt_feet->SetText(Text::GetText("CHANGEOUTFIT_5", Game::options.language));
		bt_feet->SetInverse(true);

		WindowElementContainer* container = new(M_PLACE) WindowElementContainer;
		container->Create(0, 0, 0, 0, 0, false, true, wndTemplate);
		container->ApplyElementINI(iniWindow, "COLOR_MAP");
		container->SetIntSize(0, 0);
		POINT c_size = container->GetIntSize();

		WindowElementColorMap* colormap = new(M_PLACE) WindowElementColorMap;
		colormap->Create(0, 0, 0, c_size.x, c_size.y, wndTemplate);
		colormap->SetIndex(outfit->lookHead);

		container->AddElement(colormap);

		WindowElementCheckBox* addon1 = new(M_PLACE) WindowElementCheckBox;
		addon1->Create(0, 0, 0, 0xFFFF, wndTemplate);
		addon1->ApplyElementINI(iniWindow, "ADDON_1");
		addon1->SetText(Text::GetText("CHANGEOUTFIT_6", Game::options.language));
		addon1->SetEnabled(outfits->size() > 0 && ((*outfits)[o_it].lookAddons & 0x01) == 0x01);
		addon1->SetState((outfit->lookAddons & 0x01) == 0x01);

		WindowElementCheckBox* addon2 = new(M_PLACE) WindowElementCheckBox;
		addon2->Create(0, 0, 0, 0xFFFF, wndTemplate);
		addon2->ApplyElementINI(iniWindow, "ADDON_2");
		addon2->SetText(Text::GetText("CHANGEOUTFIT_7", Game::options.language));
		addon2->SetEnabled(outfits->size() > 0 && ((*outfits)[o_it].lookAddons & 0x02) == 0x02);
		addon2->SetState((outfit->lookAddons & 0x02) == 0x02);

		WindowElementCheckBox* addon3 = new(M_PLACE) WindowElementCheckBox;
		addon3->Create(0, 0, 0, 0xFFFF, wndTemplate);
		addon3->ApplyElementINI(iniWindow, "ADDON_3");
		addon3->SetText(Text::GetText("CHANGEOUTFIT_8", Game::options.language));
		addon3->SetEnabled(outfits->size() > 0 && ((*outfits)[o_it].lookAddons & 0x04) == 0x04);
		addon3->SetState((outfit->lookAddons & 0x04) == 0x04);

		WindowElementButton* bt_ok = new(M_PLACE) WindowElementButton;
		bt_ok->Create(0, BUTTON_NORMAL, 0, 0, 0, 0, wndTemplate);
		bt_ok->ApplyElementINI(iniWindow, "BUTTON_OK");
		bt_ok->SetText(Text::GetText("CHANGEOUTFIT_9", Game::options.language));

		std::vector<void*> pointers;
		pointers.clear();
		pointers.push_back((void*)outfitCreature);
		pointers.push_back((void*)mountCreature);
		bt_ok->SetAction(boost::bind(&Game::onChangeOutfit, game, this, wnd, 0xFE, pointers));

		pointers.clear();
		pointers.push_back((void*)outfitScroll);
		pointers.push_back((void*)outfitText);
		pointers.push_back((void*)outfitCreature);
		pointers.push_back((void*)outfits);
		pointers.push_back((void*)addon1);
		pointers.push_back((void*)addon2);
		pointers.push_back((void*)addon3);
		outfitScroll->SetAction(boost::bind(&Game::onChangeOutfit, game, this, (Window*)NULL, 0x00, pointers));

		pointers.clear();
		pointers.push_back((void*)mountScroll);
		pointers.push_back((void*)mountText);
		pointers.push_back((void*)mountCreature);
		pointers.push_back((void*)mounts);
		mountScroll->SetAction(boost::bind(&Game::onChangeOutfit, game, this, (Window*)NULL, 0x01, pointers));

		pointers.clear();
		pointers.push_back((void*)addon1);
		pointers.push_back((void*)addon2);
		pointers.push_back((void*)addon3);
		pointers.push_back((void*)outfitCreature);
		addon1->SetAction(boost::bind(&Game::onChangeOutfit, game, this, (Window*)NULL, 0x02, pointers));
		addon2->SetAction(boost::bind(&Game::onChangeOutfit, game, this, (Window*)NULL, 0x02, pointers));
		addon3->SetAction(boost::bind(&Game::onChangeOutfit, game, this, (Window*)NULL, 0x02, pointers));

		pointers.clear();
		pointers.push_back((void*)colormap);
		pointers.push_back((void*)bt_head);
		pointers.push_back((void*)bt_body);
		pointers.push_back((void*)bt_legs);
		pointers.push_back((void*)bt_feet);
		pointers.push_back((void*)outfitCreature);
		colormap->SetAction(boost::bind(&Game::onChangeOutfit, game, this, (Window*)NULL, 0x10, pointers));
		bt_head->SetAction(boost::bind(&Game::onChangeOutfit, game, this, (Window*)NULL, 0x11, pointers));
		bt_body->SetAction(boost::bind(&Game::onChangeOutfit, game, this, (Window*)NULL, 0x12, pointers));
		bt_legs->SetAction(boost::bind(&Game::onChangeOutfit, game, this, (Window*)NULL, 0x13, pointers));
		bt_feet->SetAction(boost::bind(&Game::onChangeOutfit, game, this, (Window*)NULL, 0x14, pointers));

		pointers.clear();
		pointers.push_back((void*)outfitScroll);
		pointers.push_back((void*)mountScroll);
		pointers.push_back((void*)outfits);
		pointers.push_back((void*)mounts);
		wnd->AddCloseFunction(boost::bind(&Game::onChangeOutfit, game, this, (Window*)NULL, 0xFF, pointers));

		wnd->SetPosition(wndSize.x / 2 - wnd->width / 2, wndSize.y / 2 - wnd->height / 2);

		wnd->AddElement(outfitCont);
		wnd->AddElement(outfitScroll);
		wnd->AddElement(outfitText);
		wnd->AddElement(bt_head);
		wnd->AddElement(bt_body);
		wnd->AddElement(bt_legs);
		wnd->AddElement(bt_feet);
		wnd->AddElement(container);
		wnd->AddElement(mountCont);
		wnd->AddElement(mountScroll);
		wnd->AddElement(mountText);
		wnd->AddElement(addon1);
		wnd->AddElement(addon2);
		wnd->AddElement(addon3);
		wnd->AddElement(bt_ok);

		AddWindow(wnd);
	}
	else if (type == WND_QUESTS) {
		Game* game = va_arg(vl, Game*);
		va_end(vl);

		INILoader iniWindow;
		if (!iniWindow.OpenFile(std::string("templates/") + Game::options.templatesGroup + std::string("/windows/quests.ini")))
			return NULL;

		std::string templateName = iniWindow.GetValue("TEMPLATE");
		int wndWidth = atoi(iniWindow.GetValue("SIZE",0).c_str());
		int wndHeight = atoi(iniWindow.GetValue("SIZE",1).c_str());

		WindowTemplate* wndTemplate = GetWindowTemplate(Game::options.templatesGroup, templateName);

		wnd = new(M_PLACE) Window(type, 0, 0, wndTemplate);
		wnd->SetTitle(Text::GetText("QUESTS_T", Game::options.language));
		wnd->SetIntSize(wndWidth, wndHeight);
		wnd->AdjustSize();
		wnd->SetMinSize(wnd->width, wnd->height);
		wnd->SetMaxSize(wnd->width, 0);

		wnd->SetPosition((wndSize.x - wnd->width) / 2, (wndSize.y - wnd->height) / 2);

		QuestsMap quests;
		QuestLog* questlog = game->GetQuestLog();
		if (questlog)
			quests = questlog->GetQuests();

		WindowElementMemo* memo = new(M_PLACE) WindowElementMemo;
		memo->Create(0, 0, 0, 0, 0, wndTemplate);
		memo->ApplyElementINI(iniWindow, "MEMO");
		memo->SetLocks(true, true);

		QuestsMap::iterator it = quests.begin();
		for (it; it != quests.end(); it++) {
			Quest quest = it->second;
			memo->AddElement(quest.name + " " + (quest.completed ? Text::GetText("QUESTS_0", Game::options.language) : Text::GetText("QUESTS_1", Game::options.language)));
		}

		WindowElementButton* bt_show = new(M_PLACE) WindowElementButton;
		bt_show->Create(ALIGN_V_BOTTOM | ALIGN_H_RIGHT, BUTTON_NORMAL, 0, 0, 0, 0, wndTemplate);
		bt_show->ApplyElementINI(iniWindow, "BUTTON_SHOW");
		bt_show->SetText(Text::GetText("QUESTS_2", Game::options.language));

		WindowElementButton* bt_ok = new(M_PLACE) WindowElementButton;
		bt_ok->Create(ALIGN_V_BOTTOM | ALIGN_H_RIGHT, BUTTON_NORMAL, 0, 0, 0, 0, wndTemplate);
		bt_ok->ApplyElementINI(iniWindow, "BUTTON_OK");
		bt_ok->SetText(Text::GetText("QUESTS_3", Game::options.language));

		bt_show->SetAction(boost::bind(&Game::onQuestLog, game, this, (Window*)NULL, memo, (WindowElementMemo*)NULL, (WindowElementTextarea*)NULL));
		bt_ok->SetAction(boost::bind(&Game::onQuestLog, game, this, wnd, memo, (WindowElementMemo*)NULL, (WindowElementTextarea*)NULL));

		wnd->AddExecuteFunction(boost::bind(&Game::onQuestLog, game, this, (Window*)NULL, memo, (WindowElementMemo*)NULL, (WindowElementTextarea*)NULL));

		wnd->AddElement(memo);
		wnd->AddElement(bt_show);
		wnd->AddElement(bt_ok);

		AddWindow(wnd);
	}
	else if (type == WND_MISSIONS) {
		Game* game = va_arg(vl, Game*);
		va_end(vl);

		INILoader iniWindow;
		if (!iniWindow.OpenFile(std::string("templates/") + Game::options.templatesGroup + std::string("/windows/missions.ini")))
			return NULL;

		std::string templateName = iniWindow.GetValue("TEMPLATE");
		int wndWidth = atoi(iniWindow.GetValue("SIZE",0).c_str());
		int wndHeight = atoi(iniWindow.GetValue("SIZE",1).c_str());

		WindowTemplate* wndTemplate = GetWindowTemplate(Game::options.templatesGroup, templateName);

		wnd = new(M_PLACE) Window(type, 0, 0, wndTemplate);
		wnd->SetTitle(Text::GetText("MISSIONS_T", Game::options.language));
		wnd->SetIntSize(wndWidth, wndHeight);
		wnd->AdjustSize();
		wnd->SetMinSize(wnd->width, wnd->height);
		wnd->SetMaxSize(wnd->width, 0);

		wnd->SetPosition((wndSize.x - wnd->width) / 2, (wndSize.y - wnd->height) / 2);

		MissionsList missions;
		QuestLog* questlog = game->GetQuestLog();
		if (questlog)
			missions = questlog->GetMissions();

		WindowElementMemo* memo = new(M_PLACE) WindowElementMemo;
		memo->Create(0, 0, 0, 0, 0, wndTemplate);
		memo->ApplyElementINI(iniWindow, "MEMO");
		memo->SetLocks(true, true);

		std::string description;
		MissionsList::iterator it = missions.begin();
		for (it; it != missions.end(); it++) {
			Mission mission = *it;
			memo->AddElement(mission.name);
			if (it == missions.begin())
				description = mission.description;
		}

		WindowElementTextarea* textarea = new(M_PLACE) WindowElementTextarea;
		textarea->Create(0, 0, 0, 0, 0, false, true, wndTemplate);
		textarea->ApplyElementINI(iniWindow, "TEXTAREA");
		textarea->SetText(description);

		WindowElementButton* bt_ok = new(M_PLACE) WindowElementButton;
		bt_ok->Create(ALIGN_V_BOTTOM | ALIGN_H_RIGHT, BUTTON_NORMAL, 0, 0, 0, 0, wndTemplate);
		bt_ok->ApplyElementINI(iniWindow, "BUTTON_OK");
		bt_ok->SetText(Text::GetText("MISSIONS_0", Game::options.language));

		memo->SetAction(boost::bind(&Game::onQuestLog, game, this, (Window*)NULL, (WindowElementMemo*)NULL, memo, textarea));
		bt_ok->SetAction(boost::bind(&Game::onQuestLog, game, this, wnd, (WindowElementMemo*)NULL, memo, textarea));

		wnd->AddElement(memo);
		wnd->AddElement(textarea);
		wnd->AddElement(bt_ok);

		AddWindow(wnd);
	}
	else if (type == WND_SPELLS) {
		Game* game = va_arg(vl, Game*);
		SpellsList* spells = va_arg(vl, SpellsList*);
		va_end(vl);

		INILoader iniWindow;
		if (!iniWindow.OpenFile(std::string("templates/") + Game::options.templatesGroup + std::string("/windows/spells.ini")))
			return NULL;

		std::string templateName = iniWindow.GetValue("TEMPLATE");
		int wndWidth = atoi(iniWindow.GetValue("SIZE",0).c_str());
		int wndHeight = atoi(iniWindow.GetValue("SIZE",1).c_str());

		WindowTemplate* wndTemplate = GetWindowTemplate(Game::options.templatesGroup, templateName);

		wnd = new(M_PLACE) Window(type, 0, 0, wndTemplate);
		wnd->SetTitle(Text::GetText("SPELLS_T", Game::options.language));
		wnd->SetIntSize(wndWidth, wndHeight);
		wnd->AdjustSize();
		wnd->SetMinSize(wnd->width, wnd->height);
		wnd->SetMaxSize(wnd->width, 0);

		wnd->SetPosition((wndSize.x - wnd->width) / 2, (wndSize.y - wnd->height) / 2);

		ElementINI e = WindowElement::GetElementINI(iniWindow, "TEXT");

		int num = 0;
		SpellsList::iterator it = spells->begin();
		for (it, num; it != spells->end(); it++, num++) {
			HotKey* hk = new(M_PLACE) HotKey;
			hk->keyChar = 0;
			hk->text = it->second.first;
			hk->itemID = 0;
			hk->fluid = 0;
			hk->spellID = it->first;
			hk->mode = 1;
			hk->itemContainer = NULL;

			WindowElementItemContainer* hotkey = new(M_PLACE) WindowElementItemContainer;
			hotkey->Create(0, 10, 10 + num * 32, 32, 32, NULL, wndTemplate);
			hotkey->SetHotKey(hk);

			WindowElementText* text = new(M_PLACE) WindowElementText;
			text->Create(0, 52, 19 + num * 32, 0xFFFF, wndTemplate);
			if (e.fontSize >= 0) text->SetFontSize(e.fontSize);
			if (e.textBorder >= 0) text->SetBorder(e.textBorder);
			if (e.colorB >= 0) text->SetColor(e.color.red, e.color.green, e.color.blue, e.color.alpha);
			text->SetText(it->second.second);

			wnd->AddElement(hotkey);
			wnd->AddElement(text);
		}

		wnd->SetIntSize(0, 20 + num * 32);

		AddWindow(wnd);
	}
	else if (type == WND_SETHOTKEYTEXT) {
		Game* game = va_arg(vl, Game*);
		unsigned char key = va_arg(vl, int);
		va_end(vl);

		INILoader iniWindow;
		if (!iniWindow.OpenFile(std::string("templates/") + Game::options.templatesGroup + std::string("/windows/sethotkeytext.ini")))
			return NULL;

		std::string templateName = iniWindow.GetValue("TEMPLATE");
		int wndWidth = atoi(iniWindow.GetValue("SIZE",0).c_str());
		int wndHeight = atoi(iniWindow.GetValue("SIZE",1).c_str());

		WindowTemplate* wndTemplate = GetWindowTemplate(Game::options.templatesGroup, templateName);

		wnd = new(M_PLACE) Window(type, 0, 0, wndTemplate);
		wnd->SetTitle(Text::GetText("SETHOTKEYTEXT_T", Game::options.language));
		wnd->SetIntSize(wndWidth, wndHeight);
		wnd->AdjustSize();
		wnd->SetIntSize(0, 0);
		wnd->SetMinSize(wnd->width, wnd->height);
		wnd->SetMaxSize(wnd->width, wnd->height);

		std::string text = "";
		HotKey* hk = game->GetHotKey(key);
		if (hk)
			text = hk->text;

		wnd->SetPosition(wndSize.x / 2 - wnd->width / 2, wndSize.y / 2 - wnd->height / 2);

		WindowElementTextarea* line = new(M_PLACE) WindowElementTextarea;
		line->Create(0, 0, 0, 0, 0, true, false, wndTemplate);
		line->ApplyElementINI(iniWindow, "TEXTAREA");
		line->SetLocks(true, false);
		line->SetText(text);
		std::string* ptr = line->GetTextPtr();

		WindowElementButton* button = new(M_PLACE) WindowElementButton;
		button->Create(ALIGN_H_RIGHT, BUTTON_NORMAL, 0, 0, 0, 0, wndTemplate);
		button->ApplyElementINI(iniWindow, "BUTTON_SET");
		button->SetText(Text::GetText("SETHOTKEYTEXT_0", Game::options.language));

		boost::function<void()> func = boost::bind(&Game::onSetHotKey, game, this, wnd, (Keyboard*)NULL, key, ptr);
		line->SetAction(func);
		button->SetAction(func);

		wnd->AddElement(line);
		wnd->AddElement(button);

		line->active = true;
		wnd->SetActiveElement(line);

		AddWindow(wnd);
	}
	else if (type == WND_SETHOTKEY) {
		Game* game = va_arg(vl, Game*);
		Keyboard* keyboard = va_arg(vl, Keyboard*);
		unsigned char key = va_arg(vl, int);
		va_end(vl);

		INILoader iniWindow;
		if (!iniWindow.OpenFile(std::string("templates/") + Game::options.templatesGroup + std::string("/windows/sethotkey.ini")))
			return NULL;

		std::string templateName = iniWindow.GetValue("TEMPLATE");
		int wndWidth = atoi(iniWindow.GetValue("SIZE",0).c_str());
		int wndHeight = atoi(iniWindow.GetValue("SIZE",1).c_str());

		WindowTemplate* wndTemplate = GetWindowTemplate(Game::options.templatesGroup, templateName);

		wnd = new(M_PLACE) Window(type, 0, 0, wndTemplate);
		wnd->SetTitle(Text::GetText("SETHOTKEY_T", Game::options.language));

		std::string message;
		if (keyboard && keyboard->capsLock)
			message =  Text::GetText("SETHOTKEY_0", Game::options.language);
		else
			message = Text::GetText("SETHOTKEY_1", Game::options.language);

		ElementINI e = WindowElement::GetElementINI(iniWindow, "TEXT");
		int size = (e.fontSize >= 0 ? e.fontSize : 14);

		TextString text = ScrollText(TextString(message, 0), wndTemplate->font, size, wndWidth);
		DividedText lines = DivideText(text);
		for (int i = 0; i < lines.size(); i++) {
			std::string line = lines[i].second.text;
			int width = wndTemplate->font->GetTextWidth(line, size);
			WindowElementText* tx_line = new(M_PLACE) WindowElementText;
			tx_line->Create(0, (wndWidth - width) / 2, 5 + i * size, width, wndTemplate);
			tx_line->ApplyElementINI(iniWindow, "TEXT");
			tx_line->SetText(line);
			wnd->AddElement(tx_line);
		}

		wndHeight = lines.size() * size + 10;

		wnd->SetIntSize(wndWidth, wndHeight);
		wnd->AdjustSize();
		wnd->SetMinSize(wnd->width, wnd->height);
		wnd->SetMaxSize(wnd->width, wnd->height);
		wnd->SetPosition(wndSize.x / 2 - wnd->width / 2, wndSize.y / 2 - wnd->height / 2);

		wnd->AddExecuteFunction(boost::bind(&Game::onSetHotKey, game, this, wnd, keyboard, key, (std::string*)NULL));

		AddWindow(wnd);
	}
	else if (type == WND_OPTGAME) {
		Game* game = va_arg(vl, Game*);
		va_end(vl);

		INILoader iniWindow;
		if (!iniWindow.OpenFile(std::string("templates/") + Game::options.templatesGroup + std::string("/windows/optgame.ini")))
			return NULL;

		std::string templateName = iniWindow.GetValue("TEMPLATE");
		int wndWidth = atoi(iniWindow.GetValue("SIZE",0).c_str());
		int wndHeight = atoi(iniWindow.GetValue("SIZE",1).c_str());

		WindowTemplate* wndTemplate = GetWindowTemplate(Game::options.templatesGroup, templateName);

		wnd = new(M_PLACE) Window(type, 0, 0, wndTemplate);
		wnd->SetTitle(Text::GetText("OPTGAME_T", Game::options.language));
		wnd->SetIntSize(wndWidth, wndHeight);
		wnd->AdjustSize();
		wnd->SetMinSize(wnd->width, wnd->height);
		wnd->SetMaxSize(wnd->width, wnd->height);
		wnd->SetPosition((wndSize.x - wnd->width) / 2, (wndSize.y - wnd->height) / 2);

		std::list<std::string> languages = Languages::GetLanguages();

		WindowElementCheckBox* cb_autoupdate = new(M_PLACE) WindowElementCheckBox;
		cb_autoupdate->Create(0, 0, 0, 0, wndTemplate);
		cb_autoupdate->ApplyElementINI(iniWindow, "CHECKBOX_AUTOUPDATE");
		cb_autoupdate->SetText(Text::GetText("OPTGAME_0", Game::options.language));
		cb_autoupdate->SetState(Game::options.autoUpdate);
		cb_autoupdate->SetAction(boost::bind(&Game::onGame, game, this, wnd, (WindowElementMemo*)NULL, cb_autoupdate, (WindowElementCheckBox*)NULL));

		WindowElementCheckBox* cb_controls = new(M_PLACE) WindowElementCheckBox;
		cb_controls->Create(0, 0, 0, 0, wndTemplate);
		cb_controls->ApplyElementINI(iniWindow, "CHECKBOX_CONTROLS");
		cb_controls->SetText(Text::GetText("OPTGAME_1", Game::options.language));
		cb_controls->SetState(Game::options.classicControls);
		cb_controls->SetAction(boost::bind(&Game::onGame, game, this, wnd, (WindowElementMemo*)NULL, (WindowElementCheckBox*)NULL, cb_controls));

		WindowElementText* tx_lang = new(M_PLACE) WindowElementText;
		tx_lang->Create(0, 0, 0, 0, wndTemplate);
		tx_lang->ApplyElementINI(iniWindow, "TEXT_LANGUAGE");
		tx_lang->SetText(Text::GetText("OPTGAME_2", Game::options.language));

		WindowElementMemo* me_lang = new(M_PLACE) WindowElementMemo;
		me_lang->Create(0, 0, 0, 0, 0, wndTemplate);
		me_lang->ApplyElementINI(iniWindow, "MEMO_LANGUAGE");

		std::list<std::string>::iterator it = languages.begin();
		for (it; it != languages.end(); it++)
			me_lang->AddElement(*it);
		me_lang->SetOption(Game::options.language);

		WindowElementButton* bt_set = new(M_PLACE) WindowElementButton;
		bt_set->Create(0, BUTTON_NORMAL, 0, 0, 0, 0, wndTemplate);
		bt_set->ApplyElementINI(iniWindow, "BUTTON_SET");
		bt_set->SetText(Text::GetText("OPTGAME_3", Game::options.language));
		bt_set->SetAction(boost::bind(&Window::SetAction, wnd, ACT_CLOSE));
		bt_set->SetAction(boost::bind(&Game::onGame, game, this, wnd, me_lang, (WindowElementCheckBox*)NULL, (WindowElementCheckBox*)NULL));
		bt_set->SetAction(boost::bind(&Game::SaveOptions, game));

		wnd->AddExecuteFunction(boost::bind(&Game::onGame, game, this, wnd, me_lang, (WindowElementCheckBox*)NULL, (WindowElementCheckBox*)NULL));

		wnd->AddElement(cb_autoupdate);
		wnd->AddElement(cb_controls);
		wnd->AddElement(tx_lang);
		wnd->AddElement(me_lang);
		wnd->AddElement(bt_set);

		AddWindow(wnd);
	}
	else if (type == WND_OPTGRAPHIC) {
		Game* game = va_arg(vl, Game*);
		va_end(vl);

		INILoader iniWindow;
		if (!iniWindow.OpenFile(std::string("templates/") + Game::options.templatesGroup + std::string("/windows/optgraphic.ini")))
			return NULL;

		std::string templateName = iniWindow.GetValue("TEMPLATE");
		int wndWidth = atoi(iniWindow.GetValue("SIZE",0).c_str());
		int wndHeight = atoi(iniWindow.GetValue("SIZE",1).c_str());

		WindowTemplate* wndTemplate = GetWindowTemplate(Game::options.templatesGroup, templateName);

		wnd = new(M_PLACE) Window(type, 0, 0, wndTemplate);
		wnd->SetTitle(Text::GetText("OPTGRAPHIC_T", Game::options.language));
		wnd->SetIntSize(wndWidth, wndHeight);
		wnd->AdjustSize();
		wnd->SetMinSize(wnd->width, wnd->height);
		wnd->SetMaxSize(wnd->width, wnd->height);
		wnd->SetPosition((wndSize.x - wnd->width) / 2, (wndSize.y - wnd->height) / 2);

		std::vector<void*> pointers;

		Options* options = game->GetOptions();

		WindowElementText* tx_api = new(M_PLACE) WindowElementText;
		tx_api->Create(0, 0, 0, 0, wndTemplate);
		tx_api->ApplyElementINI(iniWindow, "TEXT_GRAPHICSAPI");
		tx_api->SetText(Text::GetText("OPTGRAPHIC_0", Game::options.language));

		WindowElementList* ls_api = new(M_PLACE) WindowElementList;
		ls_api->Create(0, 0, 0, 0, 0, wndTemplate);
		ls_api->ApplyElementINI(iniWindow, "LIST_GRAPHICSAPI");
		ls_api->AddElement(Text::GetText("GRAPHICSAPI_0", Game::options.language));
		ls_api->AddElement(Text::GetText("GRAPHICSAPI_1", Game::options.language));
		ls_api->SetOption(Game::options.graphicsAPI);

		WindowElementText* tx_fps = new(M_PLACE) WindowElementText;
		tx_fps->Create(0, 0, 0, 0, wndTemplate);
		tx_fps->ApplyElementINI(iniWindow, "TEXT_FPS");
		tx_fps->SetText(Text::GetText("OPTGRAPHIC_1", Game::options.language));
		tx_fps->SetValuePtr(TypePointer("uint8", (void*)&options->limitFPS));

		WindowElementScrollBar* sb_fps = new(M_PLACE) WindowElementScrollBar;
		sb_fps->Create(0, 0, 0, 0, wndTemplate);
		sb_fps->ApplyElementINI(iniWindow, "SCROLL_FPS");
		sb_fps->SetValuePtr(TypePointer("uint8", (void*)&options->limitFPS));
		sb_fps->SetMinValue(10);
		sb_fps->SetMaxValue(200);

		WindowElementText* tx_ambient = new(M_PLACE) WindowElementText;
		tx_ambient->Create(0, 0, 0, 0, wndTemplate);
		tx_ambient->ApplyElementINI(iniWindow, "TEXT_AMBIENT");
		tx_ambient->SetText(Text::GetText("OPTGRAPHIC_2", Game::options.language));
		tx_ambient->SetValuePtr(TypePointer("uint8", (void*)&options->ambientLight));

		WindowElementScrollBar* sb_ambient = new(M_PLACE) WindowElementScrollBar;
		sb_ambient->Create(0, 0, 0, 0, wndTemplate);
		sb_ambient->ApplyElementINI(iniWindow, "SCROLL_AMBIENT");
		sb_ambient->SetValuePtr(TypePointer("uint8", (void*)&options->ambientLight));
		sb_ambient->SetMinValue(0);
		sb_ambient->SetMaxValue(50);

		WindowElementCheckBox* cb_names = new(M_PLACE) WindowElementCheckBox;
		cb_names->Create(0, 0, 0, 0, wndTemplate);
		cb_names->ApplyElementINI(iniWindow, "CHECKBOX_NAMES");
		cb_names->SetText(Text::GetText("OPTGRAPHIC_3", Game::options.language));
		cb_names->SetState(options->showNames);

		WindowElementCheckBox* cb_bars = new(M_PLACE) WindowElementCheckBox;
		cb_bars->Create(0, 0, 0, 0, wndTemplate);
		cb_bars->ApplyElementINI(iniWindow, "CHECKBOX_BARS");
		cb_bars->SetText(Text::GetText("OPTGRAPHIC_4", Game::options.language));
		cb_bars->SetState(options->showBars);

		WindowElementCheckBox* cb_strech = new(M_PLACE) WindowElementCheckBox;
		cb_strech->Create(0, 0, 0, 0, wndTemplate);
		cb_strech->ApplyElementINI(iniWindow, "CHECKBOX_STRECH");
		cb_strech->SetText(Text::GetText("OPTGRAPHIC_5", Game::options.language));
		cb_strech->SetState(options->dontStrech);

		WindowElementCheckBox* cb_particles = new(M_PLACE) WindowElementCheckBox;
		cb_particles->Create(0, 0, 0, 0, wndTemplate);
		cb_particles->ApplyElementINI(iniWindow, "CHECKBOX_PARTICLES");
		cb_particles->SetText(Text::GetText("OPTGRAPHIC_10", Game::options.language));
		cb_particles->SetState(options->printParticles);

		WindowElementCheckBox* cb_level = new(M_PLACE) WindowElementCheckBox;
		cb_level->Create(0, 0, 0, 0, wndTemplate);
		cb_level->ApplyElementINI(iniWindow, "CHECKBOX_LEVEL");
		cb_level->SetText(Text::GetText("OPTGRAPHIC_6", Game::options.language));
		cb_level->SetState(options->printOneLevel);

		WindowElementCheckBox* cb_render = new(M_PLACE) WindowElementCheckBox;
		cb_render->Create(0, 0, 0, 0, wndTemplate);
		cb_render->ApplyElementINI(iniWindow, "CHECKBOX_RENDER");
		cb_render->SetText(Text::GetText("OPTGRAPHIC_11", Game::options.language));
		cb_render->SetState(options->renderToTexture);

		WindowElementButton* bt_opttext = new(M_PLACE) WindowElementButton;
		bt_opttext->Create(0, BUTTON_NORMAL, 0, 0, 0, 0, wndTemplate);
		bt_opttext->ApplyElementINI(iniWindow, "BUTTON_TEXT");
		bt_opttext->SetText(Text::GetText("OPTGRAPHIC_8", Game::options.language));
		bt_opttext->SetAction(boost::bind(&Game::onMakeAction, game, this, wnd, ACTION_OPENWINDOW, (void*)WND_OPTTEXT));

		WindowElementButton* bt_optwindows = new(M_PLACE) WindowElementButton;
		bt_optwindows->Create(0, BUTTON_NORMAL, 0, 0, 0, 0, wndTemplate);
		bt_optwindows->ApplyElementINI(iniWindow, "BUTTON_WINDOWS");
		bt_optwindows->SetText(Text::GetText("OPTGRAPHIC_9", Game::options.language));
		bt_optwindows->SetAction(boost::bind(&Game::onMakeAction, game, this, wnd, ACTION_OPENWINDOW, (void*)WND_OPTWINDOWS));

		WindowElementButton* bt_close = new(M_PLACE) WindowElementButton;
		bt_close->Create(0, BUTTON_NORMAL, 0, 0, 0, 0, wndTemplate);
		bt_close->ApplyElementINI(iniWindow, "BUTTON_CLOSE");
		bt_close->SetText(Text::GetText("OPTGRAPHIC_7", Game::options.language));
		bt_close->SetAction(boost::bind(&Window::SetAction, wnd, ACT_CLOSE));

		pointers.push_back((void*)cb_names);
		pointers.push_back((void*)cb_bars);
		pointers.push_back((void*)cb_strech);
		pointers.push_back((void*)cb_particles);
		pointers.push_back((void*)cb_level);
		pointers.push_back((void*)cb_render);
		pointers.push_back((void*)ls_api);
		sb_fps->SetAction(boost::bind(&Game::onGraphics, game, this, wnd, pointers));
		cb_names->SetAction(boost::bind(&Game::onGraphics, game, this, wnd, pointers));
		cb_bars->SetAction(boost::bind(&Game::onGraphics, game, this, wnd, pointers));
		cb_strech->SetAction(boost::bind(&Game::onGraphics, game, this, wnd, pointers));
		cb_particles->SetAction(boost::bind(&Game::onGraphics, game, this, wnd, pointers));
		cb_level->SetAction(boost::bind(&Game::onGraphics, game, this, wnd, pointers));
		cb_render->SetAction(boost::bind(&Game::onGraphics, game, this, wnd, pointers));
		ls_api->SetAction(boost::bind(&Game::onGraphics, game, this, wnd, pointers));

		wnd->AddCloseFunction(boost::bind(&Game::SaveOptions, game));

		wnd->AddElement(tx_api);
		wnd->AddElement(ls_api);
		wnd->AddElement(tx_fps);
		wnd->AddElement(sb_fps);
		wnd->AddElement(tx_ambient);
		wnd->AddElement(sb_ambient);
		wnd->AddElement(cb_names);
		wnd->AddElement(cb_bars);
		wnd->AddElement(cb_strech);
		wnd->AddElement(cb_particles);
		wnd->AddElement(cb_level);
		wnd->AddElement(cb_render);
		wnd->AddElement(bt_opttext);
		wnd->AddElement(bt_optwindows);
		wnd->AddElement(bt_close);

		AddWindow(wnd);
	}
	else if (type == WND_OPTTEXT) {
		Game* game = va_arg(vl, Game*);
		va_end(vl);

		INILoader iniWindow;
		if (!iniWindow.OpenFile(std::string("templates/") + Game::options.templatesGroup + std::string("/windows/opttext.ini")))
			return NULL;

		std::string templateName = iniWindow.GetValue("TEMPLATE");
		int wndWidth = atoi(iniWindow.GetValue("SIZE",0).c_str());
		int wndHeight = atoi(iniWindow.GetValue("SIZE",1).c_str());

		WindowTemplate* wndTemplate = GetWindowTemplate(Game::options.templatesGroup, templateName);

		wnd = new(M_PLACE) Window(type, 0, 0, wndTemplate);
		wnd->SetTitle(Text::GetText("OPTTEXT_T", Game::options.language));
		wnd->SetIntSize(wndWidth, wndHeight);
		wnd->AdjustSize();
		wnd->SetPosition((wndSize.x - wnd->width) / 2, (wndSize.y - wnd->height) / 2);

		std::vector<void*> pointers;

		Options* options = game->GetOptions();

		WindowElementText* tx_size = new(M_PLACE) WindowElementText;
		tx_size->Create(0, 0, 0, 0, wndTemplate);
		tx_size->ApplyElementINI(iniWindow, "TEXT_SIZE");
		tx_size->SetText(Text::GetText("OPTTEXT_0", Game::options.language));
		tx_size->SetValuePtr(TypePointer("uint8", (void*)&options->textSize));

		WindowElementScrollBar* sb_size = new(M_PLACE) WindowElementScrollBar;
		sb_size->Create(0, 0, 0, 0, wndTemplate);
		sb_size->ApplyElementINI(iniWindow, "SCROLL_SIZE");
		sb_size->SetValuePtr(TypePointer("uint8", (void*)&options->textSize));
		sb_size->SetMinValue(10);
		sb_size->SetMaxValue(28);

		WindowElementText* tx_flying = new(M_PLACE) WindowElementText;
		tx_flying->Create(0, 0, 0, 0, wndTemplate);
		tx_flying->ApplyElementINI(iniWindow, "TEXT_FLYING");
		tx_flying->SetText(Text::GetText("OPTTEXT_1", Game::options.language));
		tx_flying->SetValuePtr(TypePointer("uint8", (void*)&options->flyingSpeed));

		WindowElementScrollBar* sb_flying = new(M_PLACE) WindowElementScrollBar;
		sb_flying->Create(0, 0, 0, 0, wndTemplate);
		sb_flying->ApplyElementINI(iniWindow, "SCROLL_FLYING");
		sb_flying->SetValuePtr(TypePointer("uint8", (void*)&options->flyingSpeed));
		sb_flying->SetMinValue(0);
		sb_flying->SetMaxValue(100);

		WindowElementText* tx_disappear = new(M_PLACE) WindowElementText;
		tx_disappear->Create(0, 0, 0, 0, wndTemplate);
		tx_disappear->ApplyElementINI(iniWindow, "TEXT_DISAPPEAR");
		tx_disappear->SetText(Text::GetText("OPTTEXT_2", Game::options.language));
		tx_disappear->SetValuePtr(TypePointer("uint8", (void*)&options->disappearingSpeed));

		WindowElementScrollBar* sb_disappear = new(M_PLACE) WindowElementScrollBar;
		sb_disappear->Create(0, 0, 0, 0, wndTemplate);
		sb_disappear->ApplyElementINI(iniWindow, "SCROLL_DISAPPEAR");
		sb_disappear->SetValuePtr(TypePointer("uint8", (void*)&options->disappearingSpeed));
		sb_disappear->SetMinValue(0);
		sb_disappear->SetMaxValue(100);

		WindowElementButton* bt_close = new(M_PLACE) WindowElementButton;
		bt_close->Create(0, BUTTON_NORMAL, 0, 0, 0, 0, wndTemplate);
		bt_close->ApplyElementINI(iniWindow, "BUTTON_CLOSE");
		bt_close->SetText(Text::GetText("OPTTEXT_3", Game::options.language));
		bt_close->SetAction(boost::bind(&Window::SetAction, wnd, ACT_CLOSE));

		pointers.push_back((void*)sb_size);
		sb_size->SetAction(boost::bind(&Game::onText, game, this, wnd, pointers));

		wnd->AddCloseFunction(boost::bind(&Game::SaveOptions, game));

		wnd->AddElement(tx_size);
		wnd->AddElement(sb_size);
		wnd->AddElement(tx_flying);
		wnd->AddElement(sb_flying);
		wnd->AddElement(tx_disappear);
		wnd->AddElement(sb_disappear);
		wnd->AddElement(bt_close);

		AddWindow(wnd);
	}
	else if (type == WND_OPTWINDOWS) {
		Game* game = va_arg(vl, Game*);
		va_end(vl);

		INILoader iniWindow;
		if (!iniWindow.OpenFile(std::string("templates/") + Game::options.templatesGroup + std::string("/windows/optwindows.ini")))
			return NULL;

		std::string templateName = iniWindow.GetValue("TEMPLATE");
		int wndWidth = atoi(iniWindow.GetValue("SIZE",0).c_str());
		int wndHeight = atoi(iniWindow.GetValue("SIZE",1).c_str());

		WindowTemplate* wndTemplate = GetWindowTemplate(Game::options.templatesGroup, templateName);

		wnd = new(M_PLACE) Window(type, 0, 0, wndTemplate);
		wnd->SetTitle(Text::GetText("OPTWINDOWS_T", Game::options.language));
		wnd->SetIntSize(wndWidth, wndHeight);
		wnd->AdjustSize();
		wnd->SetPosition((wndSize.x - wnd->width) / 2, (wndSize.y - wnd->height) / 2);

		std::vector<void*> pointers;

		Options* options = game->GetOptions();

		WindowElementCheckBox* cb_fixedpos = new(M_PLACE) WindowElementCheckBox;
		cb_fixedpos->Create(0, 0, 0, 0, wndTemplate);
		cb_fixedpos->ApplyElementINI(iniWindow, "CHECKBOX_FIXEDPOS");
		cb_fixedpos->SetText(Text::GetText("OPTWINDOWS_0", Game::options.language));
		cb_fixedpos->SetState(options->fixedPositions);

		WindowElementCheckBox* cb_fadeout = new(M_PLACE) WindowElementCheckBox;
		cb_fadeout->Create(0, 0, 0, 0, wndTemplate);
		cb_fadeout->ApplyElementINI(iniWindow, "CHECKBOX_FADEOUT");
		cb_fadeout->SetText(Text::GetText("OPTWINDOWS_1", Game::options.language));
		cb_fadeout->SetState(options->fadeOutInactive);

		WindowElementButton* bt_templates = new(M_PLACE) WindowElementButton;
		bt_templates->Create(0, BUTTON_NORMAL, 0, 0, 0, 0, wndTemplate);
		bt_templates->ApplyElementINI(iniWindow, "BUTTON_TEMPLATES");
		bt_templates->SetText(Text::GetText("OPTWINDOWS_2", Game::options.language));
		bt_templates->SetAction(boost::bind(&Game::onMakeAction, game, this, wnd, ACTION_OPENWINDOW, (void*)WND_OPTTEMPLATES));

		WindowElementButton* bt_close = new(M_PLACE) WindowElementButton;
		bt_close->Create(0, BUTTON_NORMAL, 0, 0, 0, 0, wndTemplate);
		bt_close->ApplyElementINI(iniWindow, "BUTTON_CLOSE");
		bt_close->SetText(Text::GetText("OPTWINDOWS_3", Game::options.language));
		bt_close->SetAction(boost::bind(&Window::SetAction, wnd, ACT_CLOSE));

		pointers.push_back((void*)cb_fixedpos);
		pointers.push_back((void*)cb_fadeout);
		cb_fixedpos->SetAction(boost::bind(&Game::onWindows, game, this, wnd, pointers));
		cb_fadeout->SetAction(boost::bind(&Game::onWindows, game, this, wnd, pointers));

		wnd->AddCloseFunction(boost::bind(&Game::SaveOptions, game));

		wnd->AddElement(cb_fixedpos);
		wnd->AddElement(cb_fadeout);
		wnd->AddElement(bt_templates);
		wnd->AddElement(bt_close);

		AddWindow(wnd);
	}
	else if (type == WND_OPTTEMPLATES) {
		Game* game = va_arg(vl, Game*);
		va_end(vl);

		INILoader iniWindow;
		if (!iniWindow.OpenFile(std::string("templates/") + Game::options.templatesGroup + std::string("/windows/opttemplates.ini")))
			return NULL;

		std::string templateName = iniWindow.GetValue("TEMPLATE");
		int wndWidth = atoi(iniWindow.GetValue("SIZE",0).c_str());
		int wndHeight = atoi(iniWindow.GetValue("SIZE",1).c_str());

		WindowTemplate* wndTemplate = GetWindowTemplate(Game::options.templatesGroup, templateName);

		wnd = new(M_PLACE) Window(type, 0, 0, wndTemplate);
		wnd->SetTitle(Text::GetText("OPTTEMPLATES_T", Game::options.language));
		wnd->SetIntSize(wndWidth, wndHeight);
		wnd->AdjustSize();
		wnd->SetPosition((wndSize.x - wnd->width) / 2, (wndSize.y - wnd->height) / 2);

		std::list<std::string> templates = GetTemplateGroups();

		WindowElementMemo* me_temp = new(M_PLACE) WindowElementMemo;
		me_temp->Create(0, 0, 0, 0, 0, wndTemplate);
		me_temp->ApplyElementINI(iniWindow, "MEMO");

		std::list<std::string>::iterator it = templates.begin();
		for (it; it != templates.end(); it++)
			me_temp->AddElement(*it);
		me_temp->SetOption(Game::options.templatesGroup);

		WindowElementButton* bt_set = new(M_PLACE) WindowElementButton;
		bt_set->Create(0, BUTTON_NORMAL, 0, 0, 0, 0, wndTemplate);
		bt_set->ApplyElementINI(iniWindow, "BUTTON_SET");
		bt_set->SetText(Text::GetText("OPTTEMPLATES_0", Game::options.language));
		bt_set->SetAction(boost::bind(&Window::SetAction, wnd, ACT_CLOSE));
		bt_set->SetAction(boost::bind(&Game::onTemplate, game, this, wnd, me_temp));

		wnd->AddExecuteFunction(boost::bind(&Game::onTemplate, game, this, wnd, me_temp));
		wnd->AddCloseFunction(boost::bind(&Game::SaveOptions, game));

		wnd->AddElement(me_temp);
		wnd->AddElement(bt_set);

		AddWindow(wnd);
	}
	else if (type == WND_OPTSOUND) {
		Game* game = va_arg(vl, Game*);
		va_end(vl);

		INILoader iniWindow;
		if (!iniWindow.OpenFile(std::string("templates/") + Game::options.templatesGroup + std::string("/windows/optsound.ini")))
			return NULL;

		std::string templateName = iniWindow.GetValue("TEMPLATE");
		int wndWidth = atoi(iniWindow.GetValue("SIZE",0).c_str());
		int wndHeight = atoi(iniWindow.GetValue("SIZE",1).c_str());

		WindowTemplate* wndTemplate = GetWindowTemplate(Game::options.templatesGroup, templateName);

		wnd = new(M_PLACE) Window(type, 0, 0, wndTemplate);
		wnd->SetTitle(Text::GetText("OPTSOUND_T", Game::options.language));
		wnd->SetIntSize(wndWidth, wndHeight);
		wnd->AdjustSize();
		wnd->SetPosition((wndSize.x - wnd->width) / 2, (wndSize.y - wnd->height) / 2);
		wnd->SetMinSize(wnd->width, wnd->height);
		wnd->SetMaxSize(wnd->width, wnd->height);

		Options* options = game->GetOptions();

		WindowElementText* tx_music = new(M_PLACE) WindowElementText;
		tx_music->Create(0, 0, 0, 0, wndTemplate);
		tx_music->ApplyElementINI(iniWindow, "TEXT_MUSIC");
		tx_music->SetText(Text::GetText("OPTSOUND_0", Game::options.language));
		tx_music->SetValuePtr(TypePointer("uint8", (void*)&options->musicGain));

		WindowElementScrollBar* sb_music = new(M_PLACE) WindowElementScrollBar;
		sb_music->Create(0, 0, 0, 0, wndTemplate);
		sb_music->ApplyElementINI(iniWindow, "SCROLL_MUSIC");
		sb_music->SetValuePtr(TypePointer("uint8", (void*)&options->musicGain));
		sb_music->SetMinValue(0);
		sb_music->SetMaxValue(100);
		sb_music->SetAction(boost::bind(&Game::onSounds, game, this, wnd));

		WindowElementText* tx_sound = new(M_PLACE) WindowElementText;
		tx_sound->Create(0, 0, 0, 0, wndTemplate);
		tx_sound->ApplyElementINI(iniWindow, "TEXT_SOUND");
		tx_sound->SetText(Text::GetText("OPTSOUND_1", Game::options.language));
		tx_sound->SetValuePtr(TypePointer("uint8", (void*)&options->soundGain));

		WindowElementScrollBar* sb_sound = new(M_PLACE) WindowElementScrollBar;
		sb_sound->Create(0, 0, 0, 0, wndTemplate);
		sb_sound->ApplyElementINI(iniWindow, "SCROLL_SOUND");
		sb_sound->SetValuePtr(TypePointer("uint8", (void*)&options->soundGain));
		sb_sound->SetMinValue(0);
		sb_sound->SetMaxValue(100);
		sb_sound->SetAction(boost::bind(&Game::onSounds, game, this, wnd));

		WindowElementButton* bt_close = new(M_PLACE) WindowElementButton;
		bt_close->Create(0, BUTTON_NORMAL, 0, 0, 0, 0, wndTemplate);
		bt_close->ApplyElementINI(iniWindow, "BUTTON_CLOSE");
		bt_close->SetText(Text::GetText("OPTSOUND_2", Game::options.language));
		bt_close->SetAction(boost::bind(&Window::SetAction, wnd, ACT_CLOSE));

		wnd->AddCloseFunction(boost::bind(&Game::SaveOptions, game));

		wnd->AddElement(tx_music);
		wnd->AddElement(sb_music);
		wnd->AddElement(tx_sound);
		wnd->AddElement(sb_sound);
		wnd->AddElement(bt_close);

		AddWindow(wnd);
	}
	else if (type == WND_ABOUT) {
		Game* game = va_arg(vl, Game*);
		va_end(vl);

		INILoader iniWindow;
		if (!iniWindow.OpenFile(std::string("templates/") + Game::options.templatesGroup + std::string("/windows/about.ini")))
			return NULL;

		std::string templateName = iniWindow.GetValue("TEMPLATE");
		WindowTemplate* wndTemplate = GetWindowTemplate(Game::options.templatesGroup, templateName);

		wnd = new(M_PLACE) Window(type, 0, 0, wndTemplate);
		wnd->SetTitle(Text::GetText("ABOUT_T", Game::options.language));
		wnd->SetIntSize(300, 120);
		wnd->AdjustSize();
		wnd->SetPosition((wndSize.x - wnd->width) / 2, (wndSize.y - wnd->height) / 2);
		wnd->SetMinSize(wnd->width, wnd->height);
		wnd->SetMaxSize(wnd->width, wnd->height);

		WindowElementImage* im_logo = new(M_PLACE) WindowElementImage;
		im_logo->Create(0, 0, 0, Icons::GetLogoIcon(), wndTemplate);

		WindowElementText* tx_name = new(M_PLACE) WindowElementText;
		tx_name->Create(0, 90, 0, 0xFFFF, wndTemplate);
		tx_name->ApplyElementINI(iniWindow, "TEXT_NAME");
		tx_name->SetText(std::string(APP_NAME) + " " + std::string(APP_VERSION));

		WindowElementText* tx_company = new(M_PLACE) WindowElementText;
		tx_company->Create(0, 90, 26, 0xFFFF, wndTemplate);
		tx_company->ApplyElementINI(iniWindow, "TEXT_COMPANY");
		tx_company->SetText(APP_COMPANY);

		int ln_width = wndTemplate->font->GetTextWidth(APP_WEBSITE, 14);
		Mouse* mouse = game->GetMouse();

		WindowElementLink* ln_link = new(M_PLACE) WindowElementLink;
		ln_link->Create(0, 90, 40, ln_width, mouse, NULL, wndTemplate);
		ln_link->SetText(APP_WEBSITE);
		ln_link->SetColorOut(0.5f, 0.5f, 1.0f);
		ln_link->SetColorIn(0.2f, 0.2f, 0.7f);
		ln_link->ApplyElementINI(iniWindow, "LINK_LINK");
		ln_link->SetAction(boost::bind(&Game::PostOpenBrowser, APP_WEBSITE));

		WindowElementText* tx_rights = new(M_PLACE) WindowElementText;
		tx_rights->Create(0, 90, 66, 0xFFFF, wndTemplate);
		tx_rights->ApplyElementINI(iniWindow, "TEXT_RIGHTS");
		tx_rights->SetText(APP_RIGHTS);

		WindowElementButton* bt_close = new(M_PLACE) WindowElementButton;
		bt_close->Create(0, BUTTON_NORMAL, 120, 90, 60, 20, wndTemplate);
		bt_close->ApplyElementINI(iniWindow, "BUTTON_CLOSE");
		bt_close->SetText(Text::GetText("ABOUT_0", Game::options.language));
		bt_close->SetAction(boost::bind(&Window::SetAction, wnd, ACT_CLOSE));

		wnd->AddElement(im_logo);
		wnd->AddElement(tx_name);
		wnd->AddElement(tx_company);
		wnd->AddElement(ln_link);
		wnd->AddElement(tx_rights);
		wnd->AddElement(bt_close);

		AddWindow(wnd);
	}
	else if (type == WND_ERROR) {
		Game* game = va_arg(vl, Game*);
		Host* host = va_arg(vl, Host*);
		Character* character = va_arg(vl, Character*);
		std::string message = va_arg(vl, char*);
		va_end(vl);

		INILoader iniWindow;
		if (!iniWindow.OpenFile(std::string("templates/") + Game::options.templatesGroup + std::string("/windows/error.ini")))
			return NULL;

		std::string templateName = iniWindow.GetValue("TEMPLATE");
		int wndWidth = atoi(iniWindow.GetValue("SIZE",0).c_str());
		int wndHeight = atoi(iniWindow.GetValue("SIZE",1).c_str());

		WindowTemplate* wndTemplate = GetWindowTemplate(Game::options.templatesGroup, templateName);

		wnd = new(M_PLACE) Window(type, 0, 0, wndTemplate);
		wnd->SetTitle(Text::GetText("ERROR_T", Game::options.language));
		wnd->SetIntSize(wndWidth, wndHeight);
		wnd->AdjustSize();
		wnd->SetIntSize(0, 0);
		wnd->SetPosition(wndSize.x / 2 - wnd->width / 2, wndSize.y / 2 - wnd->height / 2);
		wnd->SetMinSize(200, 100);
		wnd->SetMaxSize(0, 0);

		WindowElementTextarea* ta_message = new(M_PLACE) WindowElementTextarea;
		ta_message->Create(0, 0, 0, 0, 0, false, true, wndTemplate);
		ta_message->ApplyElementINI(iniWindow, "TEXTAREA_MESSAGE");
		ta_message->SetOffset(16);
		ta_message->SetText(message);
		ta_message->SetLocks(true, true);

		WindowElementText* tx_comment = new(M_PLACE) WindowElementText;
		tx_comment->Create(ALIGN_V_BOTTOM, 0, 0, 0xFFFF, wndTemplate);
		tx_comment->ApplyElementINI(iniWindow, "TEXT_COMMENT");
		tx_comment->SetText(Text::GetText("ERROR_0", Game::options.language));

		WindowElementTextarea* ta_comment = new(M_PLACE) WindowElementTextarea;
		ta_comment->Create(ALIGN_V_BOTTOM, 0, 0, 0, 0, true, false, wndTemplate);
		ta_comment->ApplyElementINI(iniWindow, "TEXTAREA_COMMENT");
		ta_comment->SetLocks(true, false);

		WindowElementButton* bt_send = new(M_PLACE) WindowElementButton;
		bt_send->Create(ALIGN_H_RIGHT | ALIGN_V_BOTTOM, BUTTON_NORMAL, 0, 0, 0, 0, wndTemplate);
		bt_send->ApplyElementINI(iniWindow, "BUTTON_SEND");
		bt_send->SetText(Text::GetText("ERROR_1", Game::options.language));
		bt_send->SetAction(boost::bind(&Game::onError, game, this, wnd, *host, *character, message, ta_comment->GetTextPtr()));

		WindowElementButton* bt_close = new(M_PLACE) WindowElementButton;
		bt_close->Create(ALIGN_H_RIGHT | ALIGN_V_BOTTOM, BUTTON_NORMAL, 0, 0, 0, 0, wndTemplate);
		bt_close->ApplyElementINI(iniWindow, "BUTTON_CLOSE");
		bt_close->SetText(Text::GetText("ERROR_2", Game::options.language));
		bt_close->SetAction(boost::bind(&Window::SetAction, wnd, ACT_CLOSE));

		wnd->AddCloseFunction(boost::bind(&Game::onError, game, this, wnd, *host, *character, std::string(""), ta_comment->GetTextPtr()));

		wnd->AddElement(ta_message);
		wnd->AddElement(tx_comment);
		wnd->AddElement(ta_comment);
		wnd->AddElement(bt_send);
		wnd->AddElement(bt_close);

		AddWindow(wnd);
	}
	else if (type == WND_UPDATE) {
		Game* game = va_arg(vl, Game*);
		va_end(vl);

		INILoader iniWindow;
		if (!iniWindow.OpenFile(std::string("templates/") + Game::options.templatesGroup + std::string("/windows/update.ini")))
			return NULL;

		std::string templateName = iniWindow.GetValue("TEMPLATE");
		int wndWidth = atoi(iniWindow.GetValue("SIZE",0).c_str());
		int wndHeight = atoi(iniWindow.GetValue("SIZE",1).c_str());

		WindowTemplate* wndTemplate = GetWindowTemplate(Game::options.templatesGroup, templateName);

		wnd = new(M_PLACE) Window(type, 0, 0, wndTemplate);
		wnd->SetTitle(Text::GetText("UPDATE_T", Game::options.language));
		wnd->SetIntSize(wndWidth, wndHeight);
		wnd->AdjustSize();
		wnd->SetPosition(wndSize.x / 2 - wnd->width / 2, wndSize.y / 2 - wnd->height / 2);
		wnd->SetMinSize(wnd->width, wnd->height);
		wnd->SetMaxSize(wnd->width, wnd->height);

		WindowElementContainer* container = wnd->GetWindowContainer();
		Updater* updater = game->GetUpdater();
		updater->SetContainer(container);

		std::string text = Text::GetText("UPDATE_0", Game::options.language);
		int textWdh = wndTemplate->font->GetTextWidth(text, 14);

		WindowElementText* tx_info = new(M_PLACE) WindowElementText;
		tx_info->Create(0, (wndWidth - textWdh) / 2, (wndHeight - 14) / 2, 0xFFFF, wndTemplate);
		tx_info->ApplyElementINI(iniWindow, "TEXT");
		tx_info->SetText(text);

		wnd->AddCloseFunction(boost::bind(&Game::onUpdate, game, this, wnd));

		wnd->AddElement(tx_info);

		AddWindow(wnd);
	}
	else if (type == WND_ADDFAVORITE) {
		Game* game = va_arg(vl, Game*);
		std::vector<void*>* pointers1 = va_arg(vl, std::vector<void*>*);
		va_end(vl);

		Servers* servers = game->GetServers();

		INILoader iniWindow;
		if (!iniWindow.OpenFile(std::string("templates/") + Game::options.templatesGroup + std::string("/windows/addfavorite.ini")))
			return NULL;

		std::string templateName = iniWindow.GetValue("TEMPLATE");
		int wndWidth = atoi(iniWindow.GetValue("SIZE",0).c_str());
		int wndHeight = atoi(iniWindow.GetValue("SIZE",1).c_str());

		WindowTemplate* wndTemplate = GetWindowTemplate(Game::options.templatesGroup, templateName);

		wnd = new(M_PLACE) Window(type, 0, 0, wndTemplate);
		wnd->SetTitle(Text::GetText("ADDFAVORITE_T", Game::options.language));
		wnd->SetIntSize(wndWidth, wndHeight);
		wnd->AdjustSize();
		wnd->SetPosition(wndSize.x / 2 - wnd->width / 2, wndSize.y / 2 - wnd->height / 2);
		wnd->SetMinSize(wnd->width, wnd->height);
		wnd->SetMaxSize(wnd->width, wnd->height);

		Host host = game->GetHost();

		WindowElementTextarea* ta_name = new(M_PLACE) WindowElementTextarea;
		ta_name->Create(0, 0, 0, 0, 0, true, false, wndTemplate);
		ta_name->ApplyElementINI(iniWindow, "TEXTAREA_NAME");
		ta_name->SetText(host.name);

		WindowElementTextarea* ta_host = new(M_PLACE) WindowElementTextarea;
		ta_host->Create(0, 0, 0, 0, 0, true, false, wndTemplate);
		ta_host->ApplyElementINI(iniWindow, "TEXTAREA_HOST");
		ta_host->SetText(host.host);

		WindowElementTextarea* ta_port = new(M_PLACE) WindowElementTextarea;
		ta_port->Create(0, 0, 0, 0, 0, true, false, wndTemplate);
		ta_port->ApplyElementINI(iniWindow, "TEXTAREA_PORT");
		ta_port->SetText(host.port);

		WindowElementList* ls_protocol = new(M_PLACE) WindowElementList;
		ls_protocol->Create(0, 0, 0, 0, 0, wndTemplate);
		ls_protocol->ApplyElementINI(iniWindow, "LIST_PROTOCOL");
		for (int i = 0; i < atoi(protsStr[0]); i++) {
			std::string protocolVersion = protsStr[i + 1];
			ls_protocol->AddElement(protocolVersion);
		}
		ls_protocol->SetOption(host.protocol);

		WindowElementTextarea* ta_updateURL = new(M_PLACE) WindowElementTextarea;
		ta_updateURL->Create(0, 0, 0, 0, 0, true, false, wndTemplate);
		ta_updateURL->ApplyElementINI(iniWindow, "TEXTAREA_UPDATEURL");
		ta_updateURL->SetText(host.update);

		WindowElementTextarea* ta_websiteURL = new(M_PLACE) WindowElementTextarea;
		ta_websiteURL->Create(0, 0, 0, 0, 0, true, false, wndTemplate);
		ta_websiteURL->ApplyElementINI(iniWindow, "TEXTAREA_WEBSITEURL");
		ta_websiteURL->SetText(host.website);

		WindowElementTextarea* ta_createAccountURL = new(M_PLACE) WindowElementTextarea;
		ta_createAccountURL->Create(0, 0, 0, 0, 0, true, false, wndTemplate);
		ta_createAccountURL->ApplyElementINI(iniWindow, "TEXTAREA_CREATEACCOUNTURL");
		ta_createAccountURL->SetText(host.createacc);

		WindowElementText* tx_name = new(M_PLACE) WindowElementText;
		tx_name->Create(0, 0, 0, 0xFFFF, wndTemplate);
		tx_name->ApplyElementINI(iniWindow, "TEXT_NAME");
		tx_name->SetText(Text::GetText("ADDFAVORITE_0", Game::options.language));

		WindowElementText* tx_host = new(M_PLACE) WindowElementText;
		tx_host->Create(0, 0, 0, 0xFFFF, wndTemplate);
		tx_host->ApplyElementINI(iniWindow, "TEXT_HOST");
		tx_host->SetText(Text::GetText("ADDFAVORITE_1", Game::options.language));

		WindowElementText* tx_port = new(M_PLACE) WindowElementText;
		tx_port->Create(0, 0, 0, 0xFFFF, wndTemplate);
		tx_port->ApplyElementINI(iniWindow, "TEXT_PORT");
		tx_port->SetText(Text::GetText("ADDFAVORITE_2", Game::options.language));

		WindowElementText* tx_protocol = new(M_PLACE) WindowElementText;
		tx_protocol->Create(0, 0, 0, 0xFFFF, wndTemplate);
		tx_protocol->ApplyElementINI(iniWindow, "TEXT_PROTOCOL");
		tx_protocol->SetText(Text::GetText("ADDFAVORITE_3", Game::options.language));

		WindowElementText* tx_updateURL = new(M_PLACE) WindowElementText;
		tx_updateURL->Create(0, 0, 0, 0xFFFF, wndTemplate);
		tx_updateURL->ApplyElementINI(iniWindow, "TEXT_UPDATEURL");
		tx_updateURL->SetText(Text::GetText("ADDFAVORITE_4", Game::options.language));

		WindowElementText* tx_websiteURL = new(M_PLACE) WindowElementText;
		tx_websiteURL->Create(0, 0, 0, 0xFFFF, wndTemplate);
		tx_websiteURL->ApplyElementINI(iniWindow, "TEXT_WEBSITEURL");
		tx_websiteURL->SetText(Text::GetText("ADDFAVORITE_5", Game::options.language));

		WindowElementText* tx_createAccountURL = new(M_PLACE) WindowElementText;
		tx_createAccountURL->Create(0, 0, 0, 0xFFFF, wndTemplate);
		tx_createAccountURL->ApplyElementINI(iniWindow, "TEXT_CREATEACCOUNTURL");
		tx_createAccountURL->SetText(Text::GetText("ADDFAVORITE_6", Game::options.language));

		WindowElementButton* bt_add = new(M_PLACE) WindowElementButton;
		bt_add->Create(0, BUTTON_NORMAL, 0, 0, 0, 0, wndTemplate);
		bt_add->ApplyElementINI(iniWindow, "BUTTON_ADD");
		bt_add->SetText(Text::GetText("ADDFAVORITE_7", Game::options.language));

		std::vector<void*> pointers2;
		pointers2.push_back((void*)ta_name);
		pointers2.push_back((void*)ta_host);
		pointers2.push_back((void*)ta_port);
		pointers2.push_back((void*)ls_protocol);
		pointers2.push_back((void*)ta_updateURL);
		pointers2.push_back((void*)ta_websiteURL);
		pointers2.push_back((void*)ta_createAccountURL);

		boost::function<void()> func = boost::bind(&Game::onAddFavoriteServer, game, false, *pointers1, pointers2);

		ta_name->SetAction(func);
		ta_host->SetAction(func);
		ta_port->SetAction(func);
		ta_updateURL->SetAction(func);
		ta_websiteURL->SetAction(func);
		ta_createAccountURL->SetAction(func);
		bt_add->SetAction(func);

		wnd->AddCloseFunction(boost::bind(&Servers::SaveFavorites, servers));
		wnd->AddExecuteFunction(func);

		wnd->AddElement(ta_name);
		wnd->AddElement(ta_host);
		wnd->AddElement(ta_port);
		wnd->AddElement(ls_protocol);
		wnd->AddElement(ta_updateURL);
		wnd->AddElement(ta_websiteURL);
		wnd->AddElement(ta_createAccountURL);
		wnd->AddElement(tx_name);
		wnd->AddElement(tx_host);
		wnd->AddElement(tx_port);
		wnd->AddElement(tx_protocol);
		wnd->AddElement(tx_updateURL);
		wnd->AddElement(tx_websiteURL);
		wnd->AddElement(tx_createAccountURL);
		wnd->AddElement(bt_add);

		AddWindow(wnd);
	}
	else if (type == WND_CRAFTBOX) {
		Game* game = va_arg(vl, Game*);
		va_end(vl);

		INILoader iniWindow;
		if (!iniWindow.OpenFile(std::string("templates/") + Game::options.templatesGroup + std::string("/windows/craftbox.ini")))
			return NULL;

		std::string templateName = iniWindow.GetValue("TEMPLATE");
		int wndWidth = atoi(iniWindow.GetValue("SIZE",0).c_str());
		int wndHeight = atoi(iniWindow.GetValue("SIZE",1).c_str());

		WindowTemplate* wndTemplate = GetWindowTemplate(Game::options.templatesGroup, templateName);

		wnd = new(M_PLACE) Window(type, 0, 0, wndTemplate);
		wnd->SetTitle(Text::GetText("CRAFTBOX_T", Game::options.language));
		wnd->SetIntSize(wndWidth, wndHeight);
		wnd->AdjustSize();
		wnd->SetPosition((wndSize.x - wnd->width) / 2, (wndSize.y - wnd->height) / 2);
		wnd->SetMinSize(wnd->width, wnd->height);
		wnd->SetMaxSize(wnd->width, wnd->height);

		Options* options = game->GetOptions();

		WindowElementContainer* ct_elements = new(M_PLACE) WindowElementContainer;
		ct_elements->Create(0, 0, 0, 0, 0, true, true, wndTemplate);
		ct_elements->ApplyElementINI(iniWindow, "TAB_ELEMENTS");
		ct_elements->SetLocks(true, true);

		WindowElementContainer* ct_requirements = new(M_PLACE) WindowElementContainer;
		ct_requirements->Create(ALIGN_V_BOTTOM, 0, 0, 0, 0, true, true, wndTemplate);
		ct_requirements->ApplyElementINI(iniWindow, "CONTAINER_REQUIREMENTS");
		ct_requirements->SetLocks(true, false);

		wnd->AddElement(ct_elements);
		wnd->AddElement(ct_requirements);

		CraftBox* craftbox = game->GetCraftBox();
		CraftMap crafts = craftbox->GetCraftElements();
		CraftMap::iterator it = crafts.begin();
		int count = 0;
		for (it; it != crafts.end(); it++) {
			CraftElement craftElement = it->first;
			Item* item = new(M_PLACE) Item;
			item->SetID(craftElement.first);
			item->SetCount(craftElement.second);

			int posX = count % 4;
			int posY = count / 4;

			WindowElementItemContainer* itemContainer = new(M_PLACE) WindowElementItemContainer;
			itemContainer->Create(0, posX * 32, posY * 32, 32, 32, NULL, wndTemplate);
			itemContainer->SetItem(item);

			ct_elements->AddElement(itemContainer);

			count++;
		}

		craftbox->SetContainer(ct_requirements);
		craftbox->UpdateContainer(NULL);

		wnd->AddCloseFunction(boost::bind(&CraftBox::SetContainer, craftbox, (WindowElementContainer*)NULL));

		AddWindow(wnd);
	}
	else if (type == WND_EDITMARKER) {
		Game* game = va_arg(vl, Game*);
		Position* pos = va_arg(vl, Position*);
		Marker* marker = va_arg(vl, Marker*);
		va_end(vl);

		INILoader iniWindow;
		if (!iniWindow.OpenFile(std::string("templates/") + Game::options.templatesGroup + std::string("/windows/editmarker.ini")))
			return NULL;

		std::string templateName = iniWindow.GetValue("TEMPLATE");
		int wndWidth = atoi(iniWindow.GetValue("SIZE",0).c_str());
		int wndHeight = atoi(iniWindow.GetValue("SIZE",1).c_str());

		WindowTemplate* wndTemplate = GetWindowTemplate(Game::options.templatesGroup, templateName);

		wnd = new(M_PLACE) Window(type, 0, 0, wndTemplate);
		wnd->SetTitle(Text::GetText("EDITMARKER_T", Game::options.language));
		wnd->SetIntSize(wndWidth, wndHeight);
		wnd->AdjustSize();
		wnd->SetPosition((wndSize.x - wnd->width) / 2, (wndSize.y - wnd->height) / 2);
		wnd->SetMinSize(wnd->width, wnd->height);
		wnd->SetMaxSize(wnd->width, wnd->height);

		WindowElementContainer* ct_markers = new(M_PLACE) WindowElementContainer;
		ct_markers->Create(0, 0, 0, 0, 0, true, true, wndTemplate);
		ct_markers->ApplyElementINI(iniWindow, "CONTAINER_MARKERS");
		ct_markers->SetScrollAlwaysVisible(false, true);
		ct_markers->SetLocks(true, true);

		int borderH = wndTemplate->tempContainer.l_width + wndTemplate->tempContainer.r_width + wndTemplate->tempScroll.h;
		int count = (ct_markers->width - borderH) / 16;
		int margin = ((ct_markers->width - borderH) % 16) / 2;

		std::list<void*>* pointers = new(M_PLACE) std::list<void*>;

		int iconNum = 1;
		std::string comment = "";
		if (marker) {
			iconNum = marker->first;
			comment = marker->second;
		}

		int i = 0;
		AD2D_Image* icon = NULL;
		while((icon = Icons::GetMinimapIcon(i + 1))) {
			int posX = margin + (i % count) * 16;
			int posY = margin + (i / count) * 16;
			WindowElementButton* bt_marker = new(M_PLACE) WindowElementButton;
			bt_marker->Create(0, BUTTON_SWITCH, posX, posY, 16, 16, wndTemplate);
			bt_marker->SetImage(icon);

			if (i + 1 == iconNum)
				bt_marker->SetPressed(true);

			bt_marker->SetAction(boost::bind(&Game::onMarker, game, (Window*)NULL, pointers, (WindowElementTextarea*)NULL, *pos, i + 1, false));

			pointers->push_back((void*)bt_marker);

			ct_markers->AddElement(bt_marker);
			ct_markers->SetIntSize(0, posY + 16 + margin * 2);

			i++;
		}

		WindowElementText* tx_comment = new(M_PLACE) WindowElementText;
		tx_comment->Create(0, 0, 0, 0xFFFF, wndTemplate);
		tx_comment->ApplyElementINI(iniWindow, "TEXT_COMMENT");
		tx_comment->SetText(Text::GetText("EDITMARKER_0", Game::options.language));

		WindowElementTextarea* ta_comment = new(M_PLACE) WindowElementTextarea;
		ta_comment->Create(0, 0, 0, 0, 0, true, false, wndTemplate);
		ta_comment->ApplyElementINI(iniWindow, "TEXTAREA_COMMENT");
		ta_comment->SetText(comment);

		WindowElementButton* bt_ok = new(M_PLACE) WindowElementButton;
		bt_ok->Create(0, BUTTON_NORMAL, 0, 0, 0, 0, wndTemplate);
		bt_ok->ApplyElementINI(iniWindow, "BUTTON_OK");
		bt_ok->SetText(Text::GetText("EDITMARKER_1", Game::options.language));

		ta_comment->SetAction(boost::bind(&Game::onMarker, game, wnd, pointers, ta_comment, *pos, 0, false));
		bt_ok->SetAction(boost::bind(&Game::onMarker, game, wnd, pointers, ta_comment, *pos, 0, false));

		wnd->AddCloseFunction(boost::bind(&Game::onMarker, game, (Window*)NULL, pointers, ta_comment, *pos, -1, true));

		wnd->AddElement(ct_markers);
		wnd->AddElement(tx_comment);
		wnd->AddElement(ta_comment);
		wnd->AddElement(bt_ok);

		wnd->SetActiveElement(ta_comment);

		AddWindow(wnd);
	}
	else if (type == WND_EDITWAYPOINT) {
		Game* game = va_arg(vl, Game*);
		Position* pos = va_arg(vl, Position*);
		Waypoint* waypoint = va_arg(vl, Waypoint*);
		va_end(vl);

		INILoader iniWindow;
		if (!iniWindow.OpenFile(std::string("templates/") + Game::options.templatesGroup + std::string("/windows/editwaypoint.ini")))
			return NULL;

		std::string templateName = iniWindow.GetValue("TEMPLATE");
		int wndWidth = atoi(iniWindow.GetValue("SIZE",0).c_str());
		int wndHeight = atoi(iniWindow.GetValue("SIZE",1).c_str());


		WindowTemplate* wndTemplate = GetWindowTemplate(Game::options.templatesGroup, templateName);

		wnd = new(M_PLACE) Window(type, 0, 0, wndTemplate);
		wnd->SetTitle(Text::GetText("EDITWAYPOINT_T", Game::options.language));
		wnd->SetIntSize(wndWidth, wndHeight);
		wnd->AdjustSize();
		wnd->SetPosition((wndSize.x - wnd->width) / 2, (wndSize.y - wnd->height) / 2);
		wnd->SetMinSize(wnd->width, wnd->height);
		wnd->SetMaxSize(wnd->width, wnd->height);

		int iconNum = 1;
		std::string comment = "";
		if (waypoint) {
			iconNum = waypoint->first;
			comment = waypoint->second;
		}

		WindowElementText* tx_comment = new(M_PLACE) WindowElementText;
		tx_comment->Create(0, 0, 0, 0xFFFF, wndTemplate);
		tx_comment->ApplyElementINI(iniWindow, "TEXT_COMMENT");
		tx_comment->SetText(Text::GetText("EDITWAYPOINT_0", Game::options.language));

		WindowElementTextarea* ta_comment = new(M_PLACE) WindowElementTextarea;
		ta_comment->Create(0, 0, 0, 0, 0, true, false, wndTemplate);
		ta_comment->ApplyElementINI(iniWindow, "TEXTAREA_COMMENT");
		ta_comment->SetText(comment);

		WindowElementContainer* ct_waypoints = new(M_PLACE) WindowElementContainer;
		ct_waypoints->Create(0, 0, 0, 0, 0, true, true, wndTemplate);
		ct_waypoints->ApplyElementINI(iniWindow, "CONTAINER_WAYPOINTS");
		ct_waypoints->SetScrollAlwaysVisible(false, true);
		ct_waypoints->SetLocks(true, true);

		int borderH = wndTemplate->tempContainer.l_width + wndTemplate->tempContainer.r_width + wndTemplate->tempScroll.h;
		int count = (ct_waypoints->width - borderH) / 16;
		int margin = ((ct_waypoints->width - borderH) % 16) / 2;

		std::list<void*>* pointers = new(M_PLACE) std::list<void*>;

		int i = 0;
		AD2D_Image* icon = NULL;
		while((icon = Icons::GetWaypointIcon(i + 1))) {
			int posX = margin + (i % count) * 16;
			int posY = margin + (i / count) * 16;
			WindowElementButton* bt_waypoint = new(M_PLACE) WindowElementButton;
			bt_waypoint->Create(0, BUTTON_SWITCH, posX, posY, 16, 16, wndTemplate);
			bt_waypoint->SetImage(icon);

			bt_waypoint->SetAction(boost::bind(&Game::onWaypoint, game, (Window*)NULL, pointers, (WindowElementTextarea*)NULL, *pos, i + 1, false));
			if (i == 1)
				bt_waypoint->SetAction(boost::bind(&WindowElement::SetComment, ta_comment, Text::GetText("COMMENT_7", Game::options.language)));
			else if (i == 2)
				bt_waypoint->SetAction(boost::bind(&WindowElement::SetComment, ta_comment, Text::GetText("COMMENT_8", Game::options.language)));
			else if (i == 0)
				bt_waypoint->SetAction(boost::bind(&WindowElement::SetComment, ta_comment, ""));

			if (i + 1 == iconNum) {
				bt_waypoint->SetPressed(true);
				bt_waypoint->func.Execute();
			}

			pointers->push_back((void*)bt_waypoint);

			ct_waypoints->AddElement(bt_waypoint);
			ct_waypoints->SetIntSize(0, posY + 16 + margin * 2);

			i++;
		}

		WindowElementButton* bt_ok = new(M_PLACE) WindowElementButton;
		bt_ok->Create(0, BUTTON_NORMAL, 0, 0, 0, 0, wndTemplate);
		bt_ok->ApplyElementINI(iniWindow, "BUTTON_OK");
		bt_ok->SetText(Text::GetText("EDITWAYPOINT_1", Game::options.language));

		ta_comment->SetAction(boost::bind(&Game::onWaypoint, game, wnd, pointers, ta_comment, *pos, 0, false));
		bt_ok->SetAction(boost::bind(&Game::onWaypoint, game, wnd, pointers, ta_comment, *pos, 0, false));

		wnd->AddCloseFunction(boost::bind(&Game::onWaypoint, game, (Window*)NULL, pointers, ta_comment, *pos, -1, true));

		wnd->AddElement(ct_waypoints);
		wnd->AddElement(tx_comment);
		wnd->AddElement(ta_comment);
		wnd->AddElement(bt_ok);

		wnd->SetActiveElement(ta_comment);

		AddWindow(wnd);
	}
	else if (type == WND_SAVEWAYPOINTS) {
		Game* game = va_arg(vl, Game*);
		va_end(vl);

		INILoader iniWindow;
		if (!iniWindow.OpenFile(std::string("templates/") + Game::options.templatesGroup + std::string("/windows/savewaypoints.ini")))
			return NULL;

		std::string templateName = iniWindow.GetValue("TEMPLATE");
		int wndWidth = atoi(iniWindow.GetValue("SIZE",0).c_str());
		int wndHeight = atoi(iniWindow.GetValue("SIZE",1).c_str());

		WindowTemplate* wndTemplate = GetWindowTemplate(Game::options.templatesGroup, templateName);

		wnd = new(M_PLACE) Window(type, 0, 0, wndTemplate);
		wnd->SetTitle(Text::GetText("SAVEWAYPOINTS_T", Game::options.language));
		wnd->SetIntSize(wndWidth, wndHeight);
		wnd->AdjustSize();
		wnd->SetIntSize(0, 0);
		wnd->SetMinSize(wnd->width, wnd->height);
		wnd->SetMaxSize(wnd->width, wnd->height);

		wnd->SetPosition(wndSize.x / 2 - wnd->width / 2, wndSize.y / 2 - wnd->height / 2);

		WindowElementTextarea* line = new(M_PLACE) WindowElementTextarea;
		line->Create(0, 0, 0, 0, 0, true, false, wndTemplate);
		line->ApplyElementINI(iniWindow, "TEXTAREA");
		line->SetBorder(1);
		line->SetLocks(true, false);
		std::string* ptr = line->GetTextPtr();

		WindowElementButton* button = new(M_PLACE) WindowElementButton;
		button->Create(ALIGN_H_RIGHT, BUTTON_NORMAL, 0, 0, 0, 0, wndTemplate);
		button->ApplyElementINI(iniWindow, "BUTTON_SAVE");
		button->SetText(Text::GetText("SAVEWAYPOINTS_0", Game::options.language));

		boost::function<void()> func = boost::bind(&Game::onSaveWaypoints, game, this, wnd, ptr);
		line->SetAction(func);
		button->SetAction(func);

		wnd->AddElement(line);
		wnd->AddElement(button);

		line->active = true;
		wnd->SetActiveElement(line);

		AddWindow(wnd);
	}
	else if (type == WND_LOADWAYPOINTS) {
		Game* game = va_arg(vl, Game*);
		va_end(vl);

		INILoader iniWindow;
		if (!iniWindow.OpenFile(std::string("templates/") + Game::options.templatesGroup + std::string("/windows/loadwaypoints.ini")))
			return NULL;

		std::string templateName = iniWindow.GetValue("TEMPLATE");
		int wndWidth = atoi(iniWindow.GetValue("SIZE",0).c_str());
		int wndHeight = atoi(iniWindow.GetValue("SIZE",1).c_str());

		WindowTemplate* wndTemplate = GetWindowTemplate(Game::options.templatesGroup, templateName);

		wnd = new(M_PLACE) Window(type, 0, 0, wndTemplate);
		wnd->SetTitle(Text::GetText("LOADWAYPOINTS_T", Game::options.language));
		wnd->SetIntSize(wndWidth, wndHeight);
		wnd->AdjustSize();
		wnd->SetIntSize(0, 0);
		wnd->SetMinSize(wnd->width, wnd->height);
		wnd->SetMaxSize(wnd->width, wnd->height);

		wnd->SetPosition(wndSize.x / 2 - wnd->width / 2, wndSize.y / 2 - wnd->height / 2);

		WindowElementMemo* files = new(M_PLACE) WindowElementMemo;
		files->Create(0, 0, 0, 0, 0, wndTemplate);
		files->ApplyElementINI(iniWindow, "MEMO");

		DIR *dp, *child_dp;
    	struct dirent *dirp;
		std::string directory = game->filesLocation + std::string("/minimap/waypoints");
    	if ((dp = opendir(directory.c_str())) != NULL) {
    		std::map<std::string, int> waypoints;
    		int i = 0;
    		while((dirp = readdir(dp)) != NULL) {
    			std::string fullpath = std::string(directory + "/" + dirp->d_name);
    			child_dp = opendir(fullpath.c_str());
    			if (child_dp)
    				closedir(child_dp);
				else
					waypoints[dirp->d_name] = i++;
    		}

    		std::map<std::string, int>::iterator it = waypoints.begin();
    		for (it; it != waypoints.end(); it++)
    			files->AddElement(it->first);
    	}
    	closedir(dp);

		WindowElementButton* button = new(M_PLACE) WindowElementButton;
		button->Create(ALIGN_H_RIGHT, BUTTON_NORMAL, 0, 0, 0, 0, wndTemplate);
		button->ApplyElementINI(iniWindow, "BUTTON_LOAD");
		button->SetText(Text::GetText("LOADWAYPOINTS_0", Game::options.language));

		boost::function<void()> func = boost::bind(&Game::onLoadWaypoints, game, this, wnd, files);
		files->SetDblAction(func);
		button->SetAction(func);

		wnd->AddElement(files);
		wnd->AddElement(button);

		AddWindow(wnd);
	}
	else if (type == WND_BOT) {
		Game* game = va_arg(vl, Game*);
		va_end(vl);

		Bot* bot = game->GetBot();
		if (!bot || Game::adminOptions.disableBot)
			return NULL;

		Mouse* mouse = game->GetMouse();

		HealingStruct healingData = bot->GetHealingData();
		ManaRefillStruct manaRefillData = bot->GetManaRefillData();
		AimBotStruct aimBotData = bot->GetAimBotData();
		CaveBotStruct caveBotData = bot->GetCaveBotData();
		ScriptsBotStruct scriptsBotData = bot->GetScriptsBotData();

		INILoader iniWindow;
		if (!iniWindow.OpenFile(std::string("templates/") + Game::options.templatesGroup + std::string("/windows/bot.ini")))
			return NULL;

		std::string templateName = iniWindow.GetValue("TEMPLATE");
		if (templateName == "")
			return NULL;

		int wndWidth = atoi(iniWindow.GetValue("SIZE",0).c_str());
		int wndHeight = atoi(iniWindow.GetValue("SIZE",1).c_str());

		WindowTemplate* wndTemplate = GetWindowTemplate(Game::options.templatesGroup, templateName);

		wnd = new(M_PLACE) Window(type, 0, 0, wndTemplate);
		wnd->SetTitle(Text::GetText("BOT_T", Game::options.language));
		wnd->SetIntSize(wndWidth, wndHeight);
		wnd->AdjustSize();
		wnd->SetPosition((wndSize.x - wnd->width) / 2, (wndSize.y - wnd->height) / 2);
		wnd->SetMinSize(wnd->width, wnd->height);
		wnd->SetMaxSize(wnd->width, wnd->height);

		WindowElementTab* tb_options = new(M_PLACE) WindowElementTab;
		tb_options->Create(0, 0, 0, 0, 0, false, false, wndTemplate);
		tb_options->ApplyElementINI(iniWindow, "TAB_OPTIONS");
		tb_options->SetLocks(true, true);

		TabElement tabHealing = tb_options->AddTab(Text::GetText("BOT_TAB0", Game::options.language));

		WindowElementCheckBox* cb_heal1 = new(M_PLACE) WindowElementCheckBox;
		cb_heal1->Create(0, 0, 0, 0, wndTemplate);
		cb_heal1->ApplyElementINI(iniWindow, "CHECKBOX_HEAL1");
		cb_heal1->SetText(Text::GetText("BOT_TAB0_0", Game::options.language));
		cb_heal1->SetState(healingData.selfHealing);

		WindowElementCheckBox* cb_heal2 = new(M_PLACE) WindowElementCheckBox;
		cb_heal2->Create(0, 0, 0, 0, wndTemplate);
		cb_heal2->ApplyElementINI(iniWindow, "CHECKBOX_HEAL2");
		cb_heal2->SetText(Text::GetText("BOT_TAB0_1", Game::options.language));
		cb_heal2->SetState(healingData.friendHealing);

		WindowElementText* tx_minhealth1 = new(M_PLACE) WindowElementText;
		tx_minhealth1->Create(0, 0, 0, 0xFFFF, wndTemplate);
		tx_minhealth1->ApplyElementINI(iniWindow, "TEXT_MINHEALTH1");
		tx_minhealth1->SetText(Text::GetText("BOT_TAB0_2", Game::options.language));

		WindowElementText* tx_minhealth2 = new(M_PLACE) WindowElementText;
		tx_minhealth2->Create(0, 0, 0, 0xFFFF, wndTemplate);
		tx_minhealth2->ApplyElementINI(iniWindow, "TEXT_MINHEALTH2");
		tx_minhealth2->SetText(Text::GetText("BOT_TAB0_3", Game::options.language));

		WindowElementTextarea* ta_minhealth1 = new(M_PLACE) WindowElementTextarea;
		ta_minhealth1->Create(0, 0, 0, 0, 0, true, false, wndTemplate);
		ta_minhealth1->ApplyElementINI(iniWindow, "TEXTAREA_MINHEALTH1");
		ta_minhealth1->SetText(value2str(healingData.minSelfHealth));

		WindowElementTextarea* ta_minhealth2 = new(M_PLACE) WindowElementTextarea;
		ta_minhealth2->Create(0, 0, 0, 0, 0, true, false, wndTemplate);
		ta_minhealth2->ApplyElementINI(iniWindow, "TEXTAREA_MINHEALTH2");
		ta_minhealth2->SetText(value2str(healingData.minFriendHealth));

		WindowElementText* tx_healitem = new(M_PLACE) WindowElementText;
		tx_healitem->Create(0, 0, 0, 0xFFFF, wndTemplate);
		tx_healitem->ApplyElementINI(iniWindow, "TEXT_HEALITEM");
		tx_healitem->SetText(Text::GetText("BOT_TAB0_4", Game::options.language));

		WindowElementItemContainer* ic_healitem = new(M_PLACE) WindowElementItemContainer;
		Container* container = new(M_PLACE) Container(CONTAINER_FREE, 0, "HealItem", 1, false);
		Item* healItem = NULL;
		if (healingData.itemID != 0) {
			healItem = new(M_PLACE) Item;
			healItem->SetID(healingData.itemID);
			if ((*healItem)() && (*healItem)()->stackable)
				healItem->SetCount(0);
			else
				healItem->SetCount(healingData.itemSubType);
		}
		container->AddItem(healItem);
		ic_healitem->Create(0, 0, 0, 0, 0, container, wndTemplate);
		ic_healitem->ApplyElementINI(iniWindow, "ITEMCONTAINER_HEALITEM");
		ic_healitem->SetSlot(0);

		WindowElementText* tx_spell1 = new(M_PLACE) WindowElementText;
		tx_spell1->Create(0, 0, 0, 0xFFFF, wndTemplate);
		tx_spell1->ApplyElementINI(iniWindow, "TEXT_SPELL1");
		tx_spell1->SetText(Text::GetText("BOT_TAB0_5", Game::options.language));

		WindowElementText* tx_spell2 = new(M_PLACE) WindowElementText;
		tx_spell2->Create(0, 0, 0, 0xFFFF, wndTemplate);
		tx_spell2->ApplyElementINI(iniWindow, "TEXT_SPELL2");
		tx_spell2->SetText(Text::GetText("BOT_TAB0_6", Game::options.language));

		WindowElementTextarea* ta_spell1 = new(M_PLACE) WindowElementTextarea;
		ta_spell1->Create(0, 0, 0, 0, 0, true, false, wndTemplate);
		ta_spell1->ApplyElementINI(iniWindow, "TEXTAREA_SPELL1");
		ta_spell1->SetText(healingData.selfHealWords);

		WindowElementTextarea* ta_spell2 = new(M_PLACE) WindowElementTextarea;
		ta_spell2->Create(0, 0, 0, 0, 0, true, false, wndTemplate);
		ta_spell2->ApplyElementINI(iniWindow, "TEXTAREA_SPELL2");
		ta_spell2->SetText(healingData.friendHealWords);

		tabHealing.second->AddElement(cb_heal1);
		tabHealing.second->AddElement(cb_heal2);
		tabHealing.second->AddElement(tx_minhealth1);
		tabHealing.second->AddElement(tx_minhealth2);
		tabHealing.second->AddElement(ta_minhealth1);
		tabHealing.second->AddElement(ta_minhealth2);
		tabHealing.second->AddElement(tx_healitem);
		tabHealing.second->AddElement(ic_healitem);
		tabHealing.second->AddElement(tx_spell1);
		tabHealing.second->AddElement(tx_spell2);
		tabHealing.second->AddElement(ta_spell1);
		tabHealing.second->AddElement(ta_spell2);

		TabElement tabManaRefill = tb_options->AddTab(Text::GetText("BOT_TAB1", Game::options.language));

		WindowElementCheckBox* cb_manarefill = new(M_PLACE) WindowElementCheckBox;
		cb_manarefill->Create(0, 0, 0, 0, wndTemplate);
		cb_manarefill->ApplyElementINI(iniWindow, "CHECKBOX_MANAREFILL");
		cb_manarefill->SetText(Text::GetText("BOT_TAB1_0", Game::options.language));
		cb_manarefill->SetState(manaRefillData.manaRefill);

		WindowElementText* tx_manastart = new(M_PLACE) WindowElementText;
		tx_manastart->Create(0, 0, 0, 0xFFFF, wndTemplate);
		tx_manastart->ApplyElementINI(iniWindow, "TEXT_MANASTART");
		tx_manastart->SetText(Text::GetText("BOT_TAB1_1", Game::options.language));

		WindowElementText* tx_manafinish = new(M_PLACE) WindowElementText;
		tx_manafinish->Create(0, 0, 0, 0xFFFF, wndTemplate);
		tx_manafinish->ApplyElementINI(iniWindow, "TEXT_MANAFINISH");
		tx_manafinish->SetText(Text::GetText("BOT_TAB1_2", Game::options.language));

		WindowElementTextarea* ta_manastart = new(M_PLACE) WindowElementTextarea;
		ta_manastart->Create(0, 0, 0, 0, 0, true, false, wndTemplate);
		ta_manastart->ApplyElementINI(iniWindow, "TEXTAREA_MANASTART");
		ta_manastart->SetText(value2str(manaRefillData.manaStart));

		WindowElementTextarea* ta_manafinish = new(M_PLACE) WindowElementTextarea;
		ta_manafinish->Create(0, 0, 0, 0, 0, true, false, wndTemplate);
		ta_manafinish->ApplyElementINI(iniWindow, "TEXTAREA_MANAFINISH");
		ta_manafinish->SetText(value2str(manaRefillData.manaFinish));

		WindowElementText* tx_manarefillitem = new(M_PLACE) WindowElementText;
		tx_manarefillitem->Create(0, 0, 0, 0xFFFF, wndTemplate);
		tx_manarefillitem->ApplyElementINI(iniWindow, "TEXT_MANAREFILLITEM");
		tx_manarefillitem->SetText(Text::GetText("BOT_TAB1_3", Game::options.language));

		WindowElementItemContainer* ic_manarefillitem = new(M_PLACE) WindowElementItemContainer;
		Container* containerMR = new(M_PLACE) Container(CONTAINER_FREE, 0, "ManaRefillItem", 1, false);
		Item* manaRefillItem = NULL;
		if (manaRefillData.itemID != 0) {
			manaRefillItem = new(M_PLACE) Item;
			manaRefillItem->SetID(manaRefillData.itemID);
			if ((*manaRefillItem)() && (*manaRefillItem)()->stackable)
				manaRefillItem->SetCount(0);
			else
				manaRefillItem->SetCount(manaRefillData.itemSubType);
		}
		containerMR->AddItem(manaRefillItem);
		ic_manarefillitem->Create(0, 0, 0, 0, 0, containerMR, wndTemplate);
		ic_manarefillitem->ApplyElementINI(iniWindow, "ITEMCONTAINER_MANAREFILLITEM");
		ic_manarefillitem->SetSlot(0);

		tabManaRefill.second->AddElement(cb_manarefill);
		tabManaRefill.second->AddElement(tx_manastart);
		tabManaRefill.second->AddElement(tx_manafinish);
		tabManaRefill.second->AddElement(ta_manastart);
		tabManaRefill.second->AddElement(ta_manafinish);
		tabManaRefill.second->AddElement(tx_manarefillitem);
		tabManaRefill.second->AddElement(ic_manarefillitem);

		TabElement tabAimBot = tb_options->AddTab(Text::GetText("BOT_TAB2", Game::options.language));

		WindowElementCheckBox* cb_aimbot = new(M_PLACE) WindowElementCheckBox;
		cb_aimbot->Create(0, 0, 0, 0, wndTemplate);
		cb_aimbot->ApplyElementINI(iniWindow, "CHECKBOX_AIMBOT");
		cb_aimbot->SetText(Text::GetText("BOT_TAB2_0", Game::options.language));
		cb_aimbot->SetState(aimBotData.aimBot);

		WindowElementCheckBox* cb_autotargeting = new(M_PLACE) WindowElementCheckBox;
		cb_autotargeting->Create(0, 0, 0, 0, wndTemplate);
		cb_autotargeting->ApplyElementINI(iniWindow, "CHECKBOX_AUTOTARGETING");
		cb_autotargeting->SetText(Text::GetText("BOT_TAB2_1", Game::options.language));
		cb_autotargeting->SetState(aimBotData.autoTargeting);

		WindowElementText* tx_aimbotitem = new(M_PLACE) WindowElementText;
		tx_aimbotitem->Create(0, 0, 0, 0xFFFF, wndTemplate);
		tx_aimbotitem->ApplyElementINI(iniWindow, "TEXT_AIMBOTITEM");
		tx_aimbotitem->SetText(Text::GetText("BOT_TAB2_2", Game::options.language));

		WindowElementItemContainer* ic_aimbotitem = new(M_PLACE) WindowElementItemContainer;
		Container* containerAB = new(M_PLACE) Container(CONTAINER_FREE, 0, "AimBotItem", 1, false);
		Item* AimBotItem = NULL;
		if (aimBotData.itemID != 0) {
			AimBotItem = new(M_PLACE) Item;
			AimBotItem->SetID(aimBotData.itemID);
			if ((*AimBotItem)() && (*AimBotItem)()->stackable)
				AimBotItem->SetCount(0);
			else
				AimBotItem->SetCount(aimBotData.itemSubType);
		}
		containerAB->AddItem(AimBotItem);
		ic_aimbotitem->Create(0, 0, 0, 0, 0, containerAB, wndTemplate);
		ic_aimbotitem->ApplyElementINI(iniWindow, "ITEMCONTAINER_AIMBOTITEM");
		ic_aimbotitem->SetSlot(0);

		WindowElementText* tx_offensivewords = new(M_PLACE) WindowElementText;
		tx_offensivewords->Create(0, 0, 0, 0xFFFF, wndTemplate);
		tx_offensivewords->ApplyElementINI(iniWindow, "TEXT_OFFENSIVEWORDS");
		tx_offensivewords->SetText(Text::GetText("BOT_TAB2_3", Game::options.language));

		WindowElementTextarea* ta_offensivewords = new(M_PLACE) WindowElementTextarea;
		ta_offensivewords->Create(0, 0, 0, 0, 0, true, false, wndTemplate);
		ta_offensivewords->ApplyElementINI(iniWindow, "TEXTAREA_OFFENSIVEWORDS");
		ta_offensivewords->SetText(aimBotData.offensiveWords);

		tabAimBot.second->AddElement(cb_aimbot);
		tabAimBot.second->AddElement(cb_autotargeting);
		tabAimBot.second->AddElement(tx_aimbotitem);
		tabAimBot.second->AddElement(ic_aimbotitem);
		tabAimBot.second->AddElement(tx_offensivewords);
		tabAimBot.second->AddElement(ta_offensivewords);

		TabElement tabCaveBot = tb_options->AddTab(Text::GetText("BOT_TAB3", Game::options.language));

		WindowElementCheckBox* cb_cavebot = new(M_PLACE) WindowElementCheckBox;
		cb_cavebot->Create(0, 0, 0, 0, wndTemplate);
		cb_cavebot->ApplyElementINI(iniWindow, "CHECKBOX_CAVEBOT");
		cb_cavebot->SetText(Text::GetText("BOT_TAB3_0", Game::options.language));
		cb_cavebot->SetState(caveBotData.caveBot);

		WindowElementCheckBox* cb_checkbody = new(M_PLACE) WindowElementCheckBox;
		cb_checkbody->Create(0, 0, 0, 0, wndTemplate);
		cb_checkbody->ApplyElementINI(iniWindow, "CHECKBOX_CHECKBODY");
		cb_checkbody->SetText(Text::GetText("BOT_TAB3_1", Game::options.language));
		cb_checkbody->SetState(caveBotData.checkBody);

		WindowElementCheckBox* cb_movetobody = new(M_PLACE) WindowElementCheckBox;
		cb_movetobody->Create(0, 0, 0, 0, wndTemplate);
		cb_movetobody->ApplyElementINI(iniWindow, "CHECKBOX_MOVETOBODY");
		cb_movetobody->SetText(Text::GetText("BOT_TAB3_2", Game::options.language));
		cb_movetobody->SetState(caveBotData.moveToBody);

		WindowElementCheckBox* cb_eatfood = new(M_PLACE) WindowElementCheckBox;
		cb_eatfood->Create(0, 0, 0, 0, wndTemplate);
		cb_eatfood->ApplyElementINI(iniWindow, "CHECKBOX_EATFOOD");
		cb_eatfood->SetText(Text::GetText("BOT_TAB3_3", Game::options.language));
		cb_eatfood->SetState(caveBotData.eatFood);

		WindowElementCheckBox* cb_takeloot = new(M_PLACE) WindowElementCheckBox;
		cb_takeloot->Create(0, 0, 0, 0, wndTemplate);
		cb_takeloot->ApplyElementINI(iniWindow, "CHECKBOX_TAKELOOT");
		cb_takeloot->SetText(Text::GetText("BOT_TAB3_4", Game::options.language));
		cb_takeloot->SetState(caveBotData.takeLoot);

		WindowElementText* tx_foodlist = new(M_PLACE) WindowElementText;
		tx_foodlist->Create(0, 0, 0, 0xFFFF, wndTemplate);
		tx_foodlist->ApplyElementINI(iniWindow, "TEXT_FOODLIST");
		tx_foodlist->SetText(Text::GetText("BOT_TAB3_5", Game::options.language));

		WindowElementText* tx_lootlist = new(M_PLACE) WindowElementText;
		tx_lootlist->Create(0, 0, 0, 0xFFFF, wndTemplate);
		tx_lootlist->ApplyElementINI(iniWindow, "TEXT_LOOTLIST");
		tx_lootlist->SetText(Text::GetText("BOT_TAB3_6", Game::options.language));

		WindowElementMemo* me_foodlist = new(M_PLACE) WindowElementMemo;
		me_foodlist->Create(0, 0, 0, 0, 0, wndTemplate);
		me_foodlist->ApplyElementINI(iniWindow, "MEMO_FOODLIST");
		for (std::list<TextString>::iterator it = caveBotData.foodList.begin(); it != caveBotData.foodList.end(); it++) {
			std::string value = it->text;
			me_foodlist->AddElement(value);
		}

		WindowElementMemo* me_lootlist = new(M_PLACE) WindowElementMemo;
		me_lootlist->Create(0, 0, 0, 0, 0, wndTemplate);
		me_lootlist->ApplyElementINI(iniWindow, "MEMO_LOOTLIST");
		for (std::list<TextString>::iterator it = caveBotData.lootList.begin(); it != caveBotData.lootList.end(); it++) {
			std::string value = it->text;
			me_lootlist->AddElement(value);
		}

		WindowElementTextarea* ta_additem = new(M_PLACE) WindowElementTextarea;
		ta_additem->Create(0, 0, 0, 0, 0, true, false, wndTemplate);
		ta_additem->ApplyElementINI(iniWindow, "TEXTAREA_ADDITEM");
		ta_additem->SetComment(Text::GetText("COMMENT_6", Game::options.language));

		WindowElementButton* bt_addfood = new(M_PLACE) WindowElementButton;
		bt_addfood->Create(0, BUTTON_NORMAL, 0, 0, 0, 0, wndTemplate);
		bt_addfood->ApplyElementINI(iniWindow, "BUTTON_ADDFOOD");
		bt_addfood->SetText(Text::GetText("BOT_TAB3_9", Game::options.language));

		WindowElementButton* bt_removefood = new(M_PLACE) WindowElementButton;
		bt_removefood->Create(0, BUTTON_NORMAL, 0, 0, 0, 0, wndTemplate);
		bt_removefood->ApplyElementINI(iniWindow, "BUTTON_REMOVEFOOD");
		bt_removefood->SetText(Text::GetText("BOT_TAB3_10", Game::options.language));

		WindowElementButton* bt_addloot = new(M_PLACE) WindowElementButton;
		bt_addloot->Create(0, BUTTON_NORMAL, 0, 0, 0, 0, wndTemplate);
		bt_addloot->ApplyElementINI(iniWindow, "BUTTON_ADDLOOT");
		bt_addloot->SetText(Text::GetText("BOT_TAB3_9", Game::options.language));

		WindowElementButton* bt_removeloot = new(M_PLACE) WindowElementButton;
		bt_removeloot->Create(0, BUTTON_NORMAL,0, 0, 0, 0, wndTemplate);
		bt_removeloot->ApplyElementINI(iniWindow, "BUTTON_REMOVELOOT");
		bt_removeloot->SetText(Text::GetText("BOT_TAB3_10", Game::options.language));

		WindowElementText* tx_lootbagindex = new(M_PLACE) WindowElementText;
		tx_lootbagindex->Create(0, 0, 0, 0xFFFF, wndTemplate);
		tx_lootbagindex->ApplyElementINI(iniWindow, "TEXT_LOOTBAGINDEX");
		tx_lootbagindex->SetText(Text::GetText("BOT_TAB3_7", Game::options.language));

		WindowElementTextarea* ta_lootbagindex = new(M_PLACE) WindowElementTextarea;
		ta_lootbagindex->Create(0, 0, 0, 0, 0, true, false, wndTemplate);
		ta_lootbagindex->ApplyElementINI(iniWindow, "TEXTAREA_LOOTBAGINDEX");
		ta_lootbagindex->SetText(value2str(caveBotData.lootBag));
		ta_lootbagindex->SetEnabled(false);

		WindowElementButton* bt_lootbagindex = new(M_PLACE) WindowElementButton;
		bt_lootbagindex->Create(0, BUTTON_NORMAL, 0, 0, 0, 0, wndTemplate);
		bt_lootbagindex->ApplyElementINI(iniWindow, "BUTTON_LOOTBAGINDEX");
		bt_lootbagindex->SetText(Text::GetText("BOT_TAB3_8", Game::options.language));

		bt_addfood->SetAction(boost::bind(&WindowElementMemo::AddElementPtr, me_foodlist, ta_additem->GetTextPtr(), (std::string*)NULL, true, true));
		bt_addfood->SetAction(boost::bind(&Game::onBotLists, game, (WindowElementMemo*)NULL, (WindowElementMemo*)NULL, me_foodlist, (WindowElementMemo*)NULL));
		bt_removefood->SetAction(boost::bind(&WindowElementMemo::RemoveElement, me_foodlist));
		bt_removefood->SetAction(boost::bind(&Game::onBotLists, game, (WindowElementMemo*)NULL, (WindowElementMemo*)NULL, me_foodlist, (WindowElementMemo*)NULL));
		bt_addloot->SetAction(boost::bind(&WindowElementMemo::AddElementPtr, me_lootlist, ta_additem->GetTextPtr(), (std::string*)NULL, true, true));
		bt_addloot->SetAction(boost::bind(&Game::onBotLists, game, (WindowElementMemo*)NULL, (WindowElementMemo*)NULL, (WindowElementMemo*)NULL, me_lootlist));
		bt_removeloot->SetAction(boost::bind(&WindowElementMemo::RemoveElement, me_lootlist));
		bt_removeloot->SetAction(boost::bind(&Game::onBotLists, game, (WindowElementMemo*)NULL, (WindowElementMemo*)NULL, (WindowElementMemo*)NULL, me_lootlist));
		bt_lootbagindex->SetAction(boost::bind(&Mouse::SetHolder, mouse, 0, 0, 0x80, (void*)wnd, (void*)ta_lootbagindex, (void*)NULL));

		tabCaveBot.second->AddElement(cb_cavebot);
		tabCaveBot.second->AddElement(cb_checkbody);
		tabCaveBot.second->AddElement(cb_movetobody);
		tabCaveBot.second->AddElement(cb_eatfood);
		tabCaveBot.second->AddElement(cb_takeloot);
		tabCaveBot.second->AddElement(tx_foodlist);
		tabCaveBot.second->AddElement(tx_lootlist);
		tabCaveBot.second->AddElement(me_foodlist);
		tabCaveBot.second->AddElement(me_lootlist);
		tabCaveBot.second->AddElement(ta_additem);
		tabCaveBot.second->AddElement(bt_addfood);
		tabCaveBot.second->AddElement(bt_removefood);
		tabCaveBot.second->AddElement(bt_addloot);
		tabCaveBot.second->AddElement(bt_removeloot);
		tabCaveBot.second->AddElement(tx_lootbagindex);
		tabCaveBot.second->AddElement(ta_lootbagindex);
		tabCaveBot.second->AddElement(bt_lootbagindex);

		TabElement tabScripts = tb_options->AddTab(Text::GetText("BOT_TAB4", Game::options.language));

		WindowElementCheckBox* cb_script1 = new(M_PLACE) WindowElementCheckBox;
		cb_script1->Create(0, 0, 0, 0, wndTemplate);
		cb_script1->ApplyElementINI(iniWindow, "CHECKBOX_SCRIPT1");
		cb_script1->SetState(scriptsBotData.scriptsState[0]);

		WindowElementTextarea* ta_script1 = new(M_PLACE) WindowElementTextarea;
		ta_script1->Create(0, 0, 0, 0, 0, true, false, wndTemplate);
		ta_script1->ApplyElementINI(iniWindow, "TEXTAREA_SCRIPT1");
		ta_script1->SetText(scriptsBotData.filenames[0]);

		WindowElementCheckBox* cb_script2 = new(M_PLACE) WindowElementCheckBox;
		cb_script2->Create(0, 0, 0, 0, wndTemplate);
		cb_script2->ApplyElementINI(iniWindow, "CHECKBOX_SCRIPT2");
		cb_script2->SetState(scriptsBotData.scriptsState[1]);

		WindowElementTextarea* ta_script2 = new(M_PLACE) WindowElementTextarea;
		ta_script2->Create(0, 0, 0, 0, 0, true, false, wndTemplate);
		ta_script2->ApplyElementINI(iniWindow, "TEXTAREA_SCRIPT2");
		ta_script2->SetText(scriptsBotData.filenames[1]);

		WindowElementCheckBox* cb_script3 = new(M_PLACE) WindowElementCheckBox;
		cb_script3->Create(0, 0, 0, 0, wndTemplate);
		cb_script3->ApplyElementINI(iniWindow, "CHECKBOX_SCRIPT3");
		cb_script3->SetState(scriptsBotData.scriptsState[2]);

		WindowElementTextarea* ta_script3 = new(M_PLACE) WindowElementTextarea;
		ta_script3->Create(0, 0, 0, 0, 0, true, false, wndTemplate);
		ta_script3->ApplyElementINI(iniWindow, "TEXTAREA_SCRIPT3");
		ta_script3->SetText(scriptsBotData.filenames[2]);

		WindowElementCheckBox* cb_script4 = new(M_PLACE) WindowElementCheckBox;
		cb_script4->Create(0, 0, 0, 0, wndTemplate);
		cb_script4->ApplyElementINI(iniWindow, "CHECKBOX_SCRIPT4");
		cb_script4->SetState(scriptsBotData.scriptsState[3]);

		WindowElementTextarea* ta_script4 = new(M_PLACE) WindowElementTextarea;
		ta_script4->Create(0, 0, 0, 0, 0, true, false, wndTemplate);
		ta_script4->ApplyElementINI(iniWindow, "TEXTAREA_SCRIPT4");
		ta_script4->SetText(scriptsBotData.filenames[3]);

		WindowElementCheckBox* cb_script5 = new(M_PLACE) WindowElementCheckBox;
		cb_script5->Create(0, 0, 0, 0, wndTemplate);
		cb_script5->ApplyElementINI(iniWindow, "CHECKBOX_SCRIPT5");
		cb_script5->SetState(scriptsBotData.scriptsState[4]);

		WindowElementTextarea* ta_script5= new(M_PLACE) WindowElementTextarea;
		ta_script5->Create(0, 0, 0, 0, 0, true, false, wndTemplate);
		ta_script5->ApplyElementINI(iniWindow, "TEXTAREA_SCRIPT5");
		ta_script5->SetText(scriptsBotData.filenames[4]);

		WindowElementCheckBox* cb_script6 = new(M_PLACE) WindowElementCheckBox;
		cb_script6->Create(0, 0, 0, 0, wndTemplate);
		cb_script6->ApplyElementINI(iniWindow, "CHECKBOX_SCRIPT6");
		cb_script6->SetState(scriptsBotData.scriptsState[5]);

		WindowElementTextarea* ta_script6 = new(M_PLACE) WindowElementTextarea;
		ta_script6->Create(0, 0, 0, 0, 0, true, false, wndTemplate);
		ta_script6->ApplyElementINI(iniWindow, "TEXTAREA_SCRIPT6");
		ta_script6->SetText(scriptsBotData.filenames[5]);

		WindowElementCheckBox* cb_script7 = new(M_PLACE) WindowElementCheckBox;
		cb_script7->Create(0, 0, 0, 0, wndTemplate);
		cb_script7->ApplyElementINI(iniWindow, "CHECKBOX_SCRIPT7");
		cb_script7->SetState(scriptsBotData.scriptsState[6]);

		WindowElementTextarea* ta_script7 = new(M_PLACE) WindowElementTextarea;
		ta_script7->Create(0, 0, 0, 0, 0, true, false, wndTemplate);
		ta_script7->ApplyElementINI(iniWindow, "TEXTAREA_SCRIPT7");
		ta_script7->SetText(scriptsBotData.filenames[6]);

		WindowElementCheckBox* cb_script8 = new(M_PLACE) WindowElementCheckBox;
		cb_script8->Create(0, 0, 0, 0, wndTemplate);
		cb_script8->ApplyElementINI(iniWindow, "CHECKBOX_SCRIPT8");
		cb_script8->SetState(scriptsBotData.scriptsState[7]);

		WindowElementTextarea* ta_script8 = new(M_PLACE) WindowElementTextarea;
		ta_script8->Create(0, 0, 0, 0, 0, true, false, wndTemplate);
		ta_script8->ApplyElementINI(iniWindow, "TEXTAREA_SCRIPT8");
		ta_script8->SetText(scriptsBotData.filenames[7]);

		tabScripts.second->AddElement(cb_script1);
		tabScripts.second->AddElement(ta_script1);
		tabScripts.second->AddElement(cb_script2);
		tabScripts.second->AddElement(ta_script2);
		tabScripts.second->AddElement(cb_script3);
		tabScripts.second->AddElement(ta_script3);
		tabScripts.second->AddElement(cb_script4);
		tabScripts.second->AddElement(ta_script4);
		tabScripts.second->AddElement(cb_script5);
		tabScripts.second->AddElement(ta_script5);
		tabScripts.second->AddElement(cb_script6);
		tabScripts.second->AddElement(ta_script6);
		tabScripts.second->AddElement(cb_script7);
		tabScripts.second->AddElement(ta_script7);
		tabScripts.second->AddElement(cb_script8);
		tabScripts.second->AddElement(ta_script8);


		TabElement tabPlayers = tb_options->AddTab(Text::GetText("BOT_TAB5", Game::options.language));

		WindowElementText* tx_friends = new(M_PLACE) WindowElementText;
		tx_friends->Create(0, 0, 0, 0, wndTemplate);
		tx_friends->ApplyElementINI(iniWindow, "TEXT_FRIENDS");
		tx_friends->SetText(Text::GetText("BOT_TAB5_0", Game::options.language));

		WindowElementText* tx_enemies = new(M_PLACE) WindowElementText;
		tx_enemies->Create(0, 0, 0, 0, wndTemplate);
		tx_enemies->ApplyElementINI(iniWindow, "TEXT_ENEMIES");
		tx_enemies->SetText(Text::GetText("BOT_TAB5_1", Game::options.language));

		WindowElementMemo* me_friends = new(M_PLACE) WindowElementMemo;
		me_friends->Create(0, 0, 0, 0, 0, wndTemplate);
		me_friends->ApplyElementINI(iniWindow, "MEMO_FRIENDS");
		PlayersList friends = bot->GetFriends();
		for (PlayersList::iterator it = friends.begin(); it != friends.end(); it++)
			me_friends->AddElement(*it);

		WindowElementMemo* me_enemies = new(M_PLACE) WindowElementMemo;
		me_enemies->Create(0, 0, 0, 0, 0, wndTemplate);
		me_enemies->ApplyElementINI(iniWindow, "MEMO_ENEMIES");
		PlayersList enemies = bot->GetEnemies();
		for (PlayersList::iterator it = enemies.begin(); it != enemies.end(); it++)
			me_enemies->AddElement(*it);

		WindowElementButton* bt_moveup1 = new(M_PLACE) WindowElementButton;
		bt_moveup1->Create(0, BUTTON_NORMAL, 0, 0, 0, 0, wndTemplate);
		bt_moveup1->ApplyElementINI(iniWindow, "BUTTON_MOVEUP1");
		bt_moveup1->SetText("/\\");

		WindowElementButton* bt_movedown1 = new(M_PLACE) WindowElementButton;
		bt_movedown1->Create(0, BUTTON_NORMAL, 0, 0, 0, 0, wndTemplate);
		bt_movedown1->ApplyElementINI(iniWindow, "BUTTON_MOVEDOWN1");
		bt_movedown1->SetText("\\/");

		WindowElementButton* bt_remove1 = new(M_PLACE) WindowElementButton;
		bt_remove1->Create(0, BUTTON_NORMAL, 0, 0, 0, 0, wndTemplate);
		bt_remove1->ApplyElementINI(iniWindow, "BUTTON_REMOVE1");
		bt_remove1->SetText(Text::GetText("BOT_TAB5_2", Game::options.language));

		WindowElementButton* bt_add1 = new(M_PLACE) WindowElementButton;
		bt_add1->Create(0, BUTTON_NORMAL, 0, 0, 0, 0, wndTemplate);
		bt_add1->ApplyElementINI(iniWindow, "BUTTON_ADD1");
		bt_add1->SetText(Text::GetText("BOT_TAB5_3", Game::options.language));

		WindowElementButton* bt_moveup2 = new(M_PLACE) WindowElementButton;
		bt_moveup2->Create(0, BUTTON_NORMAL, 0, 0, 0, 0, wndTemplate);
		bt_moveup2->ApplyElementINI(iniWindow, "BUTTON_MOVEUP2");
		bt_moveup2->SetText("/\\");

		WindowElementButton* bt_movedown2 = new(M_PLACE) WindowElementButton;
		bt_movedown2->Create(0, BUTTON_NORMAL, 0, 0, 0, 0, wndTemplate);
		bt_movedown2->ApplyElementINI(iniWindow, "BUTTON_MOVEDOWN2");
		bt_movedown2->SetText("\\/");

		WindowElementButton* bt_remove2 = new(M_PLACE) WindowElementButton;
		bt_remove2->Create(0, BUTTON_NORMAL, 0, 0, 0, 0, wndTemplate);
		bt_remove2->ApplyElementINI(iniWindow, "BUTTON_REMOVE2");
		bt_remove2->SetText(Text::GetText("BOT_TAB5_2", Game::options.language));

		WindowElementButton* bt_add2 = new(M_PLACE) WindowElementButton;
		bt_add2->Create(0, BUTTON_NORMAL, 0, 0, 0, 0, wndTemplate);
		bt_add2->ApplyElementINI(iniWindow, "BUTTON_ADD2");
		bt_add2->SetText(Text::GetText("BOT_TAB5_4", Game::options.language));

		WindowElementText* tx_addplayer = new(M_PLACE) WindowElementText;
		tx_addplayer->Create(0, 0, 0, 0xFFFF, wndTemplate);
		tx_addplayer->ApplyElementINI(iniWindow, "TEXT_ADDPLAYER");
		tx_addplayer->SetText(Text::GetText("BOT_TAB5_5", Game::options.language));

		WindowElementTextarea* ta_addplayer = new(M_PLACE) WindowElementTextarea;
		ta_addplayer->Create(0, 0, 0, 0, 0, true, false, wndTemplate);
		ta_addplayer->ApplyElementINI(iniWindow, "TEXTAREA_ADDPLAYER");

		bt_moveup1->SetAction(boost::bind(&WindowElementMemo::MoveUpElement, me_friends));
		bt_moveup1->SetAction(boost::bind(&Game::onBotLists, game, me_friends, (WindowElementMemo*)NULL, (WindowElementMemo*)NULL, (WindowElementMemo*)NULL));
		bt_movedown1->SetAction(boost::bind(&WindowElementMemo::MoveDownElement, me_friends));
		bt_movedown1->SetAction(boost::bind(&Game::onBotLists, game, me_friends, (WindowElementMemo*)NULL, (WindowElementMemo*)NULL, (WindowElementMemo*)NULL));
		bt_remove1->SetAction(boost::bind(&WindowElementMemo::RemoveElement, me_friends));
		bt_remove1->SetAction(boost::bind(&Game::onBotLists, game, me_friends, (WindowElementMemo*)NULL, (WindowElementMemo*)NULL, (WindowElementMemo*)NULL));
		bt_moveup2->SetAction(boost::bind(&WindowElementMemo::MoveUpElement, me_enemies));
		bt_moveup2->SetAction(boost::bind(&Game::onBotLists, game, (WindowElementMemo*)NULL, me_enemies, (WindowElementMemo*)NULL, (WindowElementMemo*)NULL));
		bt_movedown2->SetAction(boost::bind(&WindowElementMemo::MoveDownElement, me_enemies));
		bt_movedown2->SetAction(boost::bind(&Game::onBotLists, game, (WindowElementMemo*)NULL, me_enemies, (WindowElementMemo*)NULL, (WindowElementMemo*)NULL));
		bt_remove2->SetAction(boost::bind(&WindowElementMemo::RemoveElement, me_enemies));
		bt_remove2->SetAction(boost::bind(&Game::onBotLists, game, (WindowElementMemo*)NULL, me_enemies, (WindowElementMemo*)NULL, (WindowElementMemo*)NULL));
		bt_add1->SetAction(boost::bind(&WindowElementMemo::AddElementPtr, me_friends, ta_addplayer->GetTextPtr(), (std::string*)NULL, true, true));
		bt_add1->SetAction(boost::bind(&WindowElementTextarea::SetText, ta_addplayer, std::string("")));
		bt_add1->SetAction(boost::bind(&Game::onBotLists, game, me_friends, (WindowElementMemo*)NULL, (WindowElementMemo*)NULL, (WindowElementMemo*)NULL));
		bt_add2->SetAction(boost::bind(&WindowElementMemo::AddElementPtr, me_enemies, ta_addplayer->GetTextPtr(), (std::string*)NULL, true, true));
		bt_add2->SetAction(boost::bind(&WindowElementTextarea::SetText, ta_addplayer, std::string("")));
		bt_add2->SetAction(boost::bind(&Game::onBotLists, game, (WindowElementMemo*)NULL, me_enemies, (WindowElementMemo*)NULL, (WindowElementMemo*)NULL));

		tabPlayers.second->AddElement(tx_friends);
		tabPlayers.second->AddElement(tx_enemies);
		tabPlayers.second->AddElement(me_friends);
		tabPlayers.second->AddElement(me_enemies);
		tabPlayers.second->AddElement(bt_moveup1);
		tabPlayers.second->AddElement(bt_movedown1);
		tabPlayers.second->AddElement(bt_remove1);
		tabPlayers.second->AddElement(bt_add1);
		tabPlayers.second->AddElement(bt_moveup2);
		tabPlayers.second->AddElement(bt_movedown2);
		tabPlayers.second->AddElement(bt_remove2);
		tabPlayers.second->AddElement(bt_add2);
		tabPlayers.second->AddElement(tx_addplayer);
		tabPlayers.second->AddElement(ta_addplayer);

		std::vector<void*> pointers;
		pointers.push_back((void*)cb_heal1);
		pointers.push_back((void*)cb_heal2);
		pointers.push_back((void*)ta_minhealth1);
		pointers.push_back((void*)ta_minhealth2);
		pointers.push_back((void*)ic_healitem);
		pointers.push_back((void*)ta_spell1);
		pointers.push_back((void*)ta_spell2);
		pointers.push_back((void*)cb_manarefill);
		pointers.push_back((void*)ta_manastart);
		pointers.push_back((void*)ta_manafinish);
		pointers.push_back((void*)ic_manarefillitem);
		pointers.push_back((void*)cb_aimbot);
		pointers.push_back((void*)cb_autotargeting);
		pointers.push_back((void*)ic_aimbotitem);
		pointers.push_back((void*)ta_offensivewords);
		pointers.push_back((void*)cb_cavebot);
		pointers.push_back((void*)cb_checkbody);
		pointers.push_back((void*)cb_movetobody);
		pointers.push_back((void*)cb_eatfood);
		pointers.push_back((void*)cb_takeloot);
		pointers.push_back((void*)me_foodlist);
		pointers.push_back((void*)me_lootlist);
		pointers.push_back((void*)ta_lootbagindex);
		pointers.push_back((void*)cb_script1);
		pointers.push_back((void*)ta_script1);
		pointers.push_back((void*)cb_script2);
		pointers.push_back((void*)ta_script2);
		pointers.push_back((void*)cb_script3);
		pointers.push_back((void*)ta_script3);
		pointers.push_back((void*)cb_script4);
		pointers.push_back((void*)ta_script4);
		pointers.push_back((void*)cb_script5);
		pointers.push_back((void*)ta_script5);
		pointers.push_back((void*)cb_script6);
		pointers.push_back((void*)ta_script6);
		pointers.push_back((void*)cb_script7);
		pointers.push_back((void*)ta_script7);
		pointers.push_back((void*)cb_script8);
		pointers.push_back((void*)ta_script8);

		WindowElementButton* bt_apply = new(M_PLACE) WindowElementButton;
		bt_apply->Create(ALIGN_H_RIGHT | ALIGN_V_BOTTOM, BUTTON_NORMAL, 0, 0, 0, 0, wndTemplate);
		bt_apply->ApplyElementINI(iniWindow, "BUTTON_APPLY");
		bt_apply->SetText(Text::GetText("BOT_0", Game::options.language));
		bt_apply->SetAction(boost::bind(&Game::onBotApply, game, pointers));

		WindowElementButton* bt_close = new(M_PLACE) WindowElementButton;
		bt_close->Create(ALIGN_H_RIGHT | ALIGN_V_BOTTOM, BUTTON_NORMAL, 0, 0, 0, 0, wndTemplate);
		bt_close->ApplyElementINI(iniWindow, "BUTTON_CLOSE");
		bt_close->SetText(Text::GetText("BOT_1", Game::options.language));
		bt_close->SetAction(boost::bind(&Window::SetAction, wnd, ACT_CLOSE));

		wnd->AddElement(tb_options);
		wnd->AddElement(bt_apply);
		wnd->AddElement(bt_close);

		wnd->AddCloseFunction(boost::bind(&delete_debug<Container>, container, M_PLACE));
		wnd->AddCloseFunction(boost::bind(&delete_debug<Container>, containerMR, M_PLACE));
		wnd->AddCloseFunction(boost::bind(&delete_debug<Container>, containerAB, M_PLACE));

		AddWindow(wnd);
	}
	else if (type == WND_OLDHOTKEYS) {
		Game* game = va_arg(vl, Game*);
		va_end(vl);

		HotKey* hotkeys = new(M_PLACE) HotKey[48];
		for (int i = 0; i < 48; i++) {
			hotkeys[i].keyChar = 0;
			hotkeys[i].text = "";
			hotkeys[i].itemID = 0;
			hotkeys[i].fluid = 0;
			hotkeys[i].spellID = 0;
			hotkeys[i].mode = 0;
			hotkeys[i].itemContainer = NULL;
		}

		INILoader iniWindow;
		if (!iniWindow.OpenFile(std::string("templates/") + Game::options.templatesGroup + std::string("/windows/oldhotkeys.ini")))
			return NULL;

		std::string templateName = iniWindow.GetValue("TEMPLATE");
		int wndWidth = atoi(iniWindow.GetValue("SIZE",0).c_str());
		int wndHeight = atoi(iniWindow.GetValue("SIZE",1).c_str());

		WindowTemplate* wndTemplate = GetWindowTemplate(Game::options.templatesGroup, templateName);

		wnd = new(M_PLACE) Window(type, 0, 0, wndTemplate);
		wnd->SetTitle(Text::GetText("OLDHOTKEYS_T", Game::options.language));
		wnd->SetIntSize(wndWidth, wndHeight);
		wnd->AdjustSize();
		wnd->SetPosition((wndSize.x - wnd->width) / 2, (wndSize.y - wnd->height) / 2);
		wnd->SetMinSize(wnd->width, wnd->height);
		wnd->SetMaxSize(wnd->width, wnd->height);

		WindowElementText* tx_keylist = new(M_PLACE) WindowElementText;
		tx_keylist->Create(0, 0, 0, 0, wndTemplate);
		tx_keylist->ApplyElementINI(iniWindow, "TEXT_KEYLIST");
		tx_keylist->SetText(Text::GetText("OLDHOTKEYS_0", Game::options.language));

		WindowElementMemo* me_keylist = new(M_PLACE) WindowElementMemo;
		me_keylist->Create(0, 0, 0, 0, 0, wndTemplate);
		me_keylist->ApplyElementINI(iniWindow, "MEMO_KEYLIST");
		for (int i = 0; i < 48; i++) {
			std::string text;
			HotKey* hk = game->GetHotKey(i);
			if (hk->keyChar != 0) {
				text += "[";
				if (hk->keyChar & 0x4000)
					text += "CTRL + ";
				if (hk->keyChar & 0x8000)
					text += "SHIFT + ";
				if (hk->keyChar & 0x0FFF < 255)
					text.push_back((char)(hk->keyChar & 0x0FFF));
				else
					text += "F" + value2str((hk->keyChar & 0x0FFF) - 366);
				text += "]: " + hk->text;
			}
			else
				text = "[]: " + hk->text;

			me_keylist->AddElement(text);
		}

		WindowElementText* tx_keytext = new(M_PLACE) WindowElementText;
		tx_keytext->Create(0, 0, 0, 0, wndTemplate);
		tx_keytext->ApplyElementINI(iniWindow, "TEXT_KEYTEXT");
		tx_keytext->SetText(Text::GetText("OLDHOTKEYS_1", Game::options.language));

		WindowElementTextarea* ta_keytext = new(M_PLACE) WindowElementTextarea;
		ta_keytext->Create(0, 0, 0, 0, 0, true, false, wndTemplate);
		ta_keytext->ApplyElementINI(iniWindow, "TEXTAREA_KEYTEXT");

		WindowElementButton* bt_bind = new(M_PLACE) WindowElementButton;
		bt_bind->Create(0, BUTTON_NORMAL, 0, 0, 0, 0, wndTemplate);
		bt_bind->ApplyElementINI(iniWindow, "BUTTON_BIND");
		bt_bind->SetText(Text::GetText("OLDHOTKEYS_2", Game::options.language));

		WindowElementCheckBox* cb_immediately = new(M_PLACE) WindowElementCheckBox;
		cb_immediately->Create(0, 0, 0, 0, wndTemplate);
		cb_immediately->ApplyElementINI(iniWindow, "CHECKBOX_IMMEDIATELY");
		cb_immediately->SetText(Text::GetText("OLDHOTKEYS_3", Game::options.language));

		WindowElementText* tx_keyitem = new(M_PLACE) WindowElementText;
		tx_keyitem->Create(0, 0, 0, 0, wndTemplate);
		tx_keyitem->ApplyElementINI(iniWindow, "TEXT_KEYITEM");
		tx_keyitem->SetText(Text::GetText("OLDHOTKEYS_4", Game::options.language));

		WindowElementItemContainer* ic_keyitem = new(M_PLACE) WindowElementItemContainer;
		ic_keyitem->Create(0, 0, 0, 0, 0, NULL, wndTemplate);
		ic_keyitem->ApplyElementINI(iniWindow, "ITEMCONTAINER_KEYITEM");

		WindowElementButton* bt_selectitem = new(M_PLACE) WindowElementButton;
		bt_selectitem->Create(0, BUTTON_NORMAL, 0, 0, 0, 0, wndTemplate);
		bt_selectitem->ApplyElementINI(iniWindow, "BUTTON_SELECTITEM");
		bt_selectitem->SetText(Text::GetText("OLDHOTKEYS_5", Game::options.language));

		WindowElementButton* bt_clearitem = new(M_PLACE) WindowElementButton;
		bt_clearitem->Create(0, BUTTON_NORMAL, 0, 0, 0, 0, wndTemplate);
		bt_clearitem->ApplyElementINI(iniWindow, "BUTTON_CLEARITEM");
		bt_clearitem->SetText(Text::GetText("OLDHOTKEYS_6", Game::options.language));

		WindowElementCheckBox* cb_use0 = new(M_PLACE) WindowElementCheckBox;
		cb_use0->Create(0, 0, 0, 0, wndTemplate);
		cb_use0->ApplyElementINI(iniWindow, "CHECKBOX_USEONYOURSELF");
		cb_use0->SetText(Text::GetText("OLDHOTKEYS_7", Game::options.language));

		WindowElementCheckBox* cb_use1 = new(M_PLACE) WindowElementCheckBox;
		cb_use1->Create(0, 0, 0, 0, wndTemplate);
		cb_use1->ApplyElementINI(iniWindow, "CHECKBOX_USEONTARGET");
		cb_use1->SetText(Text::GetText("OLDHOTKEYS_8", Game::options.language));

		WindowElementCheckBox* cb_use2 = new(M_PLACE) WindowElementCheckBox;
		cb_use2->Create(0, 0, 0, 0, wndTemplate);
		cb_use2->ApplyElementINI(iniWindow, "CHECKBOX_USEWITHCROSSHAIR");
		cb_use2->SetText(Text::GetText("OLDHOTKEYS_9", Game::options.language));

		WindowElementButton* bt_ok = new(M_PLACE) WindowElementButton;
		bt_ok->Create(0, BUTTON_NORMAL, 0, 0, 0, 0, wndTemplate);
		bt_ok->ApplyElementINI(iniWindow, "BUTTON_OK");
		bt_ok->SetText(Text::GetText("OLDHOTKEYS_10", Game::options.language));

		WindowElementButton* bt_cancel = new(M_PLACE) WindowElementButton;
		bt_cancel->Create(0, BUTTON_NORMAL, 0, 0, 0, 0, wndTemplate);
		bt_cancel->ApplyElementINI(iniWindow, "BUTTON_CANCEL");
		bt_cancel->SetText(Text::GetText("OLDHOTKEYS_11", Game::options.language));

		std::vector<void*> pointers;
		pointers.push_back((void*)hotkeys);
		pointers.push_back((void*)me_keylist);
		pointers.push_back((void*)ta_keytext);
		pointers.push_back((void*)cb_immediately);
		pointers.push_back((void*)ic_keyitem);
		pointers.push_back((void*)cb_use0);
		pointers.push_back((void*)cb_use1);
		pointers.push_back((void*)cb_use2);

		wnd->AddElement(tx_keylist);
		wnd->AddElement(me_keylist);
		wnd->AddElement(tx_keytext);
		wnd->AddElement(ta_keytext);
		wnd->AddElement(bt_bind);
		wnd->AddElement(cb_immediately);
		wnd->AddElement(tx_keyitem);
		wnd->AddElement(ic_keyitem);
		wnd->AddElement(bt_selectitem);
		wnd->AddElement(bt_clearitem);
		wnd->AddElement(cb_use0);
		wnd->AddElement(cb_use1);
		wnd->AddElement(cb_use2);
		wnd->AddElement(bt_ok);
		wnd->AddElement(bt_cancel);

		wnd->AddCloseFunction(boost::bind(&Game::onOldHotkey, game, wnds, wnd, pointers, 0xFF));

		AddWindow(wnd);
	}
	else if (type == WND_REPORTERROR) {
		Game* game = va_arg(vl, Game*);
		va_end(vl);

		Updater* updater = game->GetUpdater();
		Character character = game->GetCharacter();

		INILoader iniWindow;
		if (!iniWindow.OpenFile(std::string("templates/") + Game::options.templatesGroup + std::string("/windows/reporterror.ini")))
			return NULL;

		std::string templateName = iniWindow.GetValue("TEMPLATE");
		int wndWidth = atoi(iniWindow.GetValue("SIZE",0).c_str());
		int wndHeight = atoi(iniWindow.GetValue("SIZE",1).c_str());

		WindowTemplate* wndTemplate = GetWindowTemplate(Game::options.templatesGroup, templateName);

		wnd = new(M_PLACE) Window(type, 0, 0, wndTemplate);
		wnd->SetTitle(Text::GetText("REPORTERROR_T", Game::options.language));
		wnd->SetIntSize(wndWidth, wndHeight);
		wnd->AdjustSize();
		wnd->SetPosition((wndSize.x - wnd->width) / 2, (wndSize.y - wnd->height) / 2);

		WindowElementText* tx_title = new(M_PLACE) WindowElementText;
		tx_title->Create(0, 0, 0, 0, wndTemplate);
		tx_title->ApplyElementINI(iniWindow, "TEXT_TITLE");
		tx_title->SetText(Text::GetText("REPORTERROR_0", Game::options.language));
		tx_title->SetEnabled(false);

		WindowElementTextarea* ta_title = new(M_PLACE) WindowElementTextarea;
		ta_title->Create(0, 0, 0, 0, 0, true, false, wndTemplate);
		ta_title->ApplyElementINI(iniWindow, "TEXTAREA_TITLE");
		ta_title->SetLocks(true, false);

		WindowElementText* tx_desc = new(M_PLACE) WindowElementText;
		tx_desc->Create(0, 0, 0, 0, wndTemplate);
		tx_desc->ApplyElementINI(iniWindow, "TEXT_DESCRIPTION");
		tx_desc->SetText(Text::GetText("REPORTERROR_1", Game::options.language));
		tx_desc->SetEnabled(false);

		WindowElementTextarea* ta_desc = new(M_PLACE) WindowElementTextarea;
		ta_desc->Create(0, 0, 0, 0, 0, true, true, wndTemplate);
		ta_desc->ApplyElementINI(iniWindow, "TEXTAREA_DESCRIPTION");
		ta_desc->SetLocks(true, true);

		WindowElementButton* bt_send = new(M_PLACE) WindowElementButton;
		bt_send->Create(ALIGN_H_RIGHT | ALIGN_V_BOTTOM, BUTTON_NORMAL, 0, 0, 0, 0, wndTemplate);
		bt_send->ApplyElementINI(iniWindow, "BUTTON_SEND");
		bt_send->SetText(Text::GetText("REPORTERROR_2", Game::options.language));
		bt_send->SetAction(boost::bind(&Updater::SendErrorReport, updater, APP_INTERACT_ADDRESS, character.name + "/" + character.serv, ta_title->GetTextPtr(), ta_desc->GetTextPtr()));
		bt_send->SetAction(boost::bind(&Window::SetAction, wnd, ACT_CLOSE));

		wnd->SetActiveElement(ta_title);

		wnd->AddElement(tx_title);
		wnd->AddElement(ta_title);
		wnd->AddElement(tx_desc);
		wnd->AddElement(ta_desc);
		wnd->AddElement(bt_send);

		AddWindow(wnd);
	}
	else if (type == WND_STATUS) {
		Game* game = va_arg(vl, Game*);
		va_end(vl);

		INILoader iniWindow;
		if (!iniWindow.OpenFile(std::string("templates/") + Game::options.templatesGroup + std::string("/windows/status.ini")))
			return NULL;

		std::string templateName = iniWindow.GetValue("TEMPLATE");
		WindowTemplate* wndTemplate = GetWindowTemplate(Game::options.templatesGroup, templateName);

		wnd = new(M_PLACE) Window(type, 0, 0, wndTemplate);
		wnd->SetCloseAbility(false);
		wnd->SetAlwaysActive(true);
		wnd->SetElements(wnd->GetElements() & ~(ELEMENT_HEADER));
		wnd->SetTitle("Status");

		wnd->SetIntSize(32, 250);
		wnd->AdjustSize();

		wnd->SetPosition(0, 0);
		wnd->SetLockToBorder(false, true);

		WindowElementContainer* cnt_coolGroup = new(M_PLACE) WindowElementContainer;
		cnt_coolGroup->Create(0, 0, 0, 32, 128, false, false, wndTemplate);

		WindowElementImage* line = new(M_PLACE) WindowElementImage;
		line->Create(0, 0, 128, &wndTemplate->tempScrollBar.m, wndTemplate);

		WindowElementContainer* cnt_coolSpell = new(M_PLACE) WindowElementContainer;
		cnt_coolSpell->Create(0, 0, 139, 32, 111, false, false, wndTemplate);
		cnt_coolSpell->SetLocks(false, true);

		WindowElementContainer* cnt_stat = new(M_PLACE) WindowElementContainer;
		cnt_stat->Create(ALIGN_V_BOTTOM, 0, 139, 32, 111, false, false, wndTemplate);

		wnd->AddElement(cnt_coolGroup);
		wnd->AddElement(cnt_coolSpell);
		wnd->AddElement(line);
		wnd->AddElement(cnt_stat);

		Cooldowns* cooldowns = game->GetCooldowns();
		if (cooldowns) {
			cooldowns->SetContainerGroup(cnt_coolGroup);
			cooldowns->SetContainerSpell(cnt_coolSpell);
			wnd->AddCloseFunction(boost::bind(&Cooldowns::SetContainerGroup, cooldowns, (WindowElementContainer*)NULL));
			wnd->AddCloseFunction(boost::bind(&Cooldowns::SetContainerSpell, cooldowns, (WindowElementContainer*)NULL));

			cooldowns->UpdateContainerGroup();
			cooldowns->UpdateContainerSpell();
		}

		Status* status = game->GetStatus();
		if (status) {
			status->SetContainer(cnt_stat);
			wnd->AddCloseFunction(boost::bind(&Status::SetContainer, status, (WindowElementContainer*)NULL));

			status->UpdateContainer();
		}

		wnd->SetSize(wnd->width, wndSize.y);
		wnd->SetMinSize(wnd->width, 32);
		wnd->SetMaxSize(wnd->width, 0);

		wnd->AddCloseFunction(boost::bind(&GUIManager::RemoveWindow, &guiManager, wnd));
		AddWindow(wnd);
		guiManager.AddWindow(wnd);

		game->SetWindowStatus(wnd);
	}
	else if (type == WND_HOTKEYS) {
		Game* game = va_arg(vl, Game*);
		va_end(vl);

		INILoader iniWindow;
		if (!iniWindow.OpenFile(std::string("templates/") + Game::options.templatesGroup + std::string("/windows/hotkeys.ini")))
			return NULL;

		std::string templateName = iniWindow.GetValue("TEMPLATE");
		WindowTemplate* wndTemplate = GetWindowTemplate(Game::options.templatesGroup, templateName);

		wnd = new(M_PLACE) Window(type, 0, 0, wndTemplate);
		wnd->SetCloseAbility(false);
		wnd->SetAlwaysActive(true);
		wnd->SetScrollAlwaysVisible(false, true);
		wnd->SetElements(wnd->GetElements() & ~(ELEMENT_HEADER));
		wnd->SetTitle("Hot Keys");

		wnd->SetIntSize(32, 48 * 32);
		wnd->AdjustSize();
		wnd->SetSize(wnd->width, wndSize.y);
		wnd->SetMinSize(wnd->width, 32);
		wnd->SetMaxSize(wnd->width, 0);

		for (int i = 0; i < 48; i++) {
			HotKey* hk = game->GetHotKey(i);
			WindowElementItemContainer* hotkey = new(M_PLACE) WindowElementItemContainer;
			hotkey->Create(0, 0, i * 32, 32, 32, NULL, wndTemplate);
			hotkey->SetHotKey(hk);
			hotkey->SetSlot(i);

			wnd->AddElement(hotkey);
		}

		wnd->SetPosition(wndSize.x - wnd->width, 0);
		wnd->SetLockToBorder(true, true);

		wnd->AddCloseFunction(boost::bind(&GUIManager::RemoveWindow, &guiManager, wnd));
		AddWindow(wnd);
		guiManager.AddWindow(wnd);

		game->SetWindowHotkeys(wnd);
	}
	else if (type == WND_MENU) {
		Game* game = va_arg(vl, Game*);
		Mouse* mouse = (Mouse*)va_arg(vl, void*);
		Keyboard* keyboard = (Keyboard*)va_arg(vl, void*);
		std::list<MenuData>* actionList = (std::list<MenuData>*)va_arg(vl, void*);
		va_end(vl);

		INILoader iniWindow;
		if (!iniWindow.OpenFile(std::string("templates/") + Game::options.templatesGroup + std::string("/windows/menu.ini")))
			return NULL;

		std::string templateName = iniWindow.GetValue("TEMPLATE");
		WindowTemplate* wndTemplate = GetWindowTemplate(Game::options.templatesGroup, templateName);

		wnd = new(M_PLACE) Window(type, 0, 0, wndTemplate);
		wnd->SetAlwaysActive(true);
		wnd->SetElements(wnd->GetElements() & ~(ELEMENT_HEADER));
		wnd->SetTitle("Menu");

		ElementINI e = WindowElement::GetElementINI(iniWindow, "TEXT");
		int size = (e.fontSize >= 0 ? e.fontSize : 14);

		int width = 0;
		int height = 0;
		std::list<MenuData>::iterator it;
		for (it = actionList->begin(); it != actionList->end(); it++) {
			TextString text = it->text;
			int textWidth = wndTemplate->font->GetTextWidth(text.text, size) + 8;
			if (textWidth > width)
				width = textWidth;

			if (text.text != "-") {
				WindowElementText* txt = new(M_PLACE) WindowElementText;
				txt->Create(0, 3, height, textWidth, wndTemplate);
				txt->ApplyElementINI(iniWindow, "TEXT");
				txt->SetText(text);
				wnd->AddElement(txt);
				height += size;
			}
			else
				height += 8;
		}

		int pos = 0;
		for (it = actionList->begin(); it != actionList->end(); it++) {
			TextString text = it->text;

			if (text.text != "-") {
				WindowElementSelect* select = new(M_PLACE) WindowElementSelect;
				select->Create(0, 0, pos, width, size, mouse, &(*it), wndTemplate);
				select->ApplyElementINI(iniWindow, "SELECT");
				wnd->AddElement(select);
				pos += size;

				boost::function<void()> func = boost::bind(&Game::onMakeAction, game, this, wnd, it->action, it->data);
				select->SetAction(func);
			}
			else
				pos += 8;
		}

		wnd->SetIntSize(width, height);
		wnd->AdjustSize();
		wnd->SetIntSize(0, 0);
		wnd->SetMinSize(wnd->width, wnd->height);
		wnd->SetMaxSize(wnd->width, wnd->height);

		wnd->SetPosition(mouse->curX - wnd->width / 2, mouse->curY - 15);

		AddWindow(wnd);
	}
	else if (type == WND_OPTIONS) {
		Game* game = va_arg(vl, Game*);
		va_end(vl);

		INILoader iniWindow;
		if (!iniWindow.OpenFile(std::string("templates/") + Game::options.templatesGroup + std::string("/windows/options.ini")))
			return NULL;

		std::string templateName = iniWindow.GetValue("TEMPLATE");
		WindowTemplate* wndTemplate = GetWindowTemplate(Game::options.templatesGroup, templateName);

		wnd = new(M_PLACE) Window(type, 0, 0, wndTemplate);
		wnd->SetCloseAbility(false);
		wnd->SetAlwaysActive(true);
		wnd->SetElements(wnd->GetElements() & ~(ELEMENT_HEADER));
		wnd->SetTitle("Options");

		Mouse* mouse = game->GetMouse();
		Keyboard* keyboard = game->GetKeyboard();

		WindowElementLink* ln_game = new(M_PLACE) WindowElementLink;
		ln_game->Create(0, 0, 0, 80, mouse, NULL, wndTemplate);
		ln_game->SetText(Text::GetText("OPTION_GAME", Game::options.language));
		ln_game->SetBorder(1);
		ln_game->ApplyElementINI(iniWindow, "LINK_GAME");
		ln_game->SetAction(boost::bind(&Game::onOptions, game, this, wnd, 0));

		WindowElementLink* ln_graph = new(M_PLACE) WindowElementLink;
		ln_graph->Create(0, 80, 0, 80, mouse, NULL, wndTemplate);
		ln_graph->SetText(Text::GetText("OPTION_GRAPHICS", Game::options.language));
		ln_graph->SetBorder(1);
		ln_graph->ApplyElementINI(iniWindow, "LINK_GRAPHICS");
		ln_graph->SetAction(boost::bind(&Game::onOptions, game, this, wnd, 1));

		WindowElementLink* ln_sound = new(M_PLACE) WindowElementLink;
		ln_sound->Create(0, 160, 0, 80, mouse, NULL, wndTemplate);
		ln_sound->SetText(Text::GetText("OPTION_SOUND", Game::options.language));
		ln_sound->SetBorder(1);
		ln_sound->ApplyElementINI(iniWindow, "LINK_SOUNDS");
		ln_sound->SetAction(boost::bind(&Game::onOptions, game, this, wnd, 2));

		WindowElementLink* ln_wnds = new(M_PLACE) WindowElementLink;
		ln_wnds->Create(0, 240, 0, 80, mouse, NULL, wndTemplate);
		ln_wnds->SetText(Text::GetText("OPTION_WINDOWS", Game::options.language));
		ln_wnds->SetBorder(1);
		ln_wnds->ApplyElementINI(iniWindow, "LINK_WINDOWS");
		ln_wnds->SetAction(boost::bind(&Game::onOptions, game, this, wnd, 3));

		WindowElementLink* ln_about = new(M_PLACE) WindowElementLink;
		ln_about->Create(0, 320, 0, 80, mouse, NULL, wndTemplate);
		ln_about->SetText(Text::GetText("OPTION_ABOUT", Game::options.language));
		ln_about->SetBorder(1);
		ln_about->ApplyElementINI(iniWindow, "LINK_ABOUT");
		ln_about->SetAction(boost::bind(&Game::onOptions, game, this, wnd, 4));

		wnd->SetIntSize(0, 14);
		wnd->AdjustSize();
		wnd->SetSize(wndSize.x, wnd->height);
		wnd->SetMinSize(0, wnd->height);
		wnd->SetMaxSize(0, wnd->height);
		wnd->SetIntSize(0, 0);
		wnd->posY = -wnd->height;

		wnd->AddElement(ln_game);
		wnd->AddElement(ln_graph);
		wnd->AddElement(ln_sound);
		wnd->AddElement(ln_wnds);
		wnd->AddElement(ln_about);

		AddWindow(wnd);
	}
	else if (type == WND_CLOSE) {
		Game* game = va_arg(vl, Game*);
		va_end(vl);

		INILoader iniWindow;
		if (!iniWindow.OpenFile(std::string("templates/") + Game::options.templatesGroup + std::string("/windows/close.ini")))
			return NULL;

		std::string templateName = iniWindow.GetValue("TEMPLATE");
		int wndWidth = atoi(iniWindow.GetValue("SIZE",0).c_str());
		int wndHeight = atoi(iniWindow.GetValue("SIZE",1).c_str());

		WindowTemplate* wndTemplate = GetWindowTemplate(Game::options.templatesGroup, templateName);

		wnd = new(M_PLACE) Window(type, 0, 0, wndTemplate);
		wnd->SetTitle(Text::GetText("CLOSE_T", Game::options.language));
		wnd->SetIntSize(wndWidth, wndHeight);
		wnd->AdjustSize();
		wnd->SetIntSize(0, 0);
		wnd->SetPosition((wndSize.x - wnd->width) / 2, (wndSize.y - wnd->height) / 2);
		wnd->SetMinSize(wnd->width, wnd->height);
		wnd->SetMaxSize(wnd->width, wnd->height);

		std::string txt = Text::GetText("CLOSE_0", Game::options.language);
		int twidth = wndTemplate->font->GetTextWidth(txt, 14);

		Protocol* protocol = game->GetProtocol();

		WindowElementText* text = new(M_PLACE) WindowElementText;
		text->Create(0, (200 - twidth) / 2, 5, 0xFFFF, wndTemplate);
		text->ApplyElementINI(iniWindow, "TEXT");
		text->SetText(txt);

		WindowElementButton* bt_exit = new(M_PLACE) WindowElementButton;
		bt_exit->Create(0, BUTTON_NORMAL, 10, 24, 60, 20, wndTemplate);
		bt_exit->ApplyElementINI(iniWindow, "BUTTON_EXIT");
		bt_exit->SetText(Text::GetText("CLOSE_1", Game::options.language));
		bt_exit->SetAction(boost::bind(&Game::PostQuit));

		WindowElementButton* bt_logout = new(M_PLACE) WindowElementButton;
		bt_logout->Create(0, BUTTON_NORMAL, 70, 24, 60, 20, wndTemplate);
		bt_logout->ApplyElementINI(iniWindow, "BUTTON_LOGOUT");
		bt_logout->SetText(Text::GetText("CLOSE_2", Game::options.language));
		bt_logout->SetAction(boost::bind(&Protocol::SendLogout, protocol));

		WindowElementButton* bt_cancel = new(M_PLACE) WindowElementButton;
		bt_cancel->Create(0, BUTTON_NORMAL, 130, 24, 60, 20, wndTemplate);
		bt_cancel->ApplyElementINI(iniWindow, "BUTTON_CANCEL");
		bt_cancel->SetText(Text::GetText("CLOSE_3", Game::options.language));
		bt_cancel->SetAction(boost::bind(&Window::SetAction, wnd, ACT_CLOSE));

		wnd->AddExecuteFunction(boost::bind(&Game::PostQuit));

		wnd->AddElement(text);
		wnd->AddElement(bt_exit);
		wnd->AddElement(bt_logout);
		wnd->AddElement(bt_cancel);

		AddWindow(wnd);
	}

	return wnd;
}

void Windows::AddWindow(Window* wnd) {
	LOCKCLASS lockClass1(lockWindows);

	if (!wnd) {
		Logger::AddLog("Windows::AddWindow()", "Trying to add NULL pointer to list!", LOG_WARNING);
		return;
	}

	wndList.push_back(wnd);

	ContainerWindow* cntWindow = dynamic_cast<ContainerWindow*>(wnd);
	if (cntWindow)
		ContainerWindow::AddContainerWindow(cntWindow);

	if (Game::options.fixedPositions)
		FitGameWindow();
}

void Windows::KillWindow(Window* wnd) {
	LOCKCLASS lockClass(lockWindows);

	std::list<Window*>::iterator it = std::find(wndList.begin(), wndList.end(), wnd);
	if (it != wndList.end()) {
		if (game) {
			if (wnd->wndType == WND_GAME) game->SetWindowGame(NULL);
			else if (wnd->wndType == WND_CONSOLE) game->SetWindowConsole(NULL);
			else if (wnd->wndType == WND_STATUS) game->SetWindowStatus(NULL);
			else if (wnd->wndType == WND_HOTKEYS) game->SetWindowHotkeys(NULL);
			else if (wnd->wndType == WND_MINIMAP) game->SetWindowMiniMap(NULL);
			else if (wnd->wndType == WND_INVENTORY) game->SetWindowInventory(NULL);
			else if (wnd->wndType == WND_STATISTICS) game->SetWindowStatistics(NULL);
			else if (wnd->wndType == WND_BATTLE) game->SetWindowBattle(NULL);
			else if (wnd->wndType == WND_VIPLIST) game->SetWindowVIPList(NULL);
		}

		wnd->CloseFunction();

		if (IsFixedPosWindow((WINDOW_TYPE)wnd->wndType))
			FitFixedWindows(wnd, - wnd->height);

		delete_debug(wnd, M_PLACE);
		wndList.erase(it);

		if (Game::options.fixedPositions)
			FitGameWindow();
	}
	else
		Logger::AddLog("Windows::KillWindow()", "Window not in list!", LOG_WARNING);
}

void Windows::ReplaceWindow(Window* wOld, Window* wNew) {
	LOCKCLASS lockClass1(lockWindows);

	if (!wOld || !wNew) {
		Logger::AddLog("Windows::ReplaceWindow()", "Pointer to window is NULL!", LOG_WARNING);
		return;
	}

	std::list<Window*>::iterator it = std::find(wndList.begin(), wndList.end(), wOld);
	if (it != wndList.end()) {
		Window* oldWindow = *it;
		*it = wNew;
		delete_debug(oldWindow, M_PLACE);

		ContainerWindow* cntWindow = dynamic_cast<ContainerWindow*>(wNew);
		if (cntWindow)
			ContainerWindow::AddContainerWindow(cntWindow);
	}
	else
		Logger::AddLog("Windows::ReplaceWindow()", "Window not in list!", LOG_WARNING);
}

void Windows::CloseWindows(WINDOW_TYPE wndType, bool immediately) {
	LOCKCLASS lockClass(lockWindows);

	if (immediately) {
		std::list<Window*> toDelete;
		std::list<Window*>::iterator it = wndList.begin();
		for (it; it != wndList.end(); it++) {
			Window* wnd = *it;
			if (wnd && wnd->wndType == wndType || wndType == WND_ALL)
				toDelete.push_back(wnd);
		}

		for (it = toDelete.begin(); it != toDelete.end(); it++) {
			Window* wnd = *it;
			KillWindow(wnd);
		}
	}
	else {
		std::list<Window*>::iterator it = wndList.begin();
		for (it; it != wndList.end(); it++) {
			Window* wnd = *it;
			if (wnd && wnd->wndType == wndType || wndType == WND_ALL)
				wnd->SetAction(ACT_CLOSE);
		}
	}
}

void Windows::MoveOnTop(Window* wnd, bool force) {
	LOCKCLASS lockClass(lockWindows);

	if (!wnd || (wnd && wnd->alwaysActive && !force))
		return;

	std::list<Window*>::iterator it = std::find(wndList.begin(), wndList.end(), wnd);
	if (it != wndList.end()) {
		wndList.erase(it);
		wndList.push_back(wnd);
	}
	else
		Logger::AddLog("Windows::MoveOnTop()", "Window not in list!", LOG_WARNING);
}

void Windows::MoveDown(Window* wnd, bool force) {
	LOCKCLASS lockClass(lockWindows);

	if (wnd && (!force && wnd->alwaysActive))
		return;

	std::list<Window*>::iterator it = wndList.begin();
	std::list<Window*>::iterator last_it = it;
	for (it; it != wndList.end(); it++) {
		Window* twnd = *it;
		if (twnd == wnd) {
			if (it != wndList.begin()) {
				wndList.erase(it);
				wndList.insert(last_it, wnd);
			}
			break;
		}
		last_it = it;
	}
}

bool Windows::IsOnList(Window* wnd) {
	LOCKCLASS lockClass(lockWindows);

	std::list<Window*>::iterator it = std::find(wndList.begin(), wndList.end(), wnd);
	if (it != wndList.end())
		return true;

	return false;
}


void Windows::LoadTemplates(AD2D_Font* font) {
	LOCKCLASS lockClass(lockWindows);

	FileManager* files = FileManager::fileManager;
	if (!files)
		return;

	std::set<std::string> templates;

	DIR *dp, *child_dp;
    struct dirent *dirp;

    if((dp = opendir("templates")) == NULL)
    	return;

	while ((dirp = readdir(dp)) != NULL) {
		if (std::string(dirp->d_name) == "." || std::string(dirp->d_name) == ".." || std::string(dirp->d_name) == "windows")
			continue;

		std::string templatePath = std::string("templates/") + dirp->d_name;
		child_dp = opendir(templatePath.c_str());
		if (child_dp) {
			templates.insert(dirp->d_name);
			closedir(child_dp);
		}
	}
	closedir(dp);

	INILoader iniLoader;
	if (iniLoader.OpenFile("templates/templates.ini")) {
		int pos = 0;
		while(iniLoader.GetValue("TEMPLATES", pos) != "")
			templates.insert(iniLoader.GetValue("TEMPLATES", pos++));
	}

	std::string templateGroup;
	for (std::set<std::string>::iterator it = templates.begin(); it != templates.end(); it++) {
		std::string templateGroup = *it;
		std::string templatePath = std::string("templates/") + templateGroup;

		std::set<std::string> subTemplates;

		INILoader iniSubLoader;
		if (iniSubLoader.OpenFile(templatePath + "/subtemplates.ini")) {
			int pos = 0;
			while(iniSubLoader.GetValue("SUBTEMPLATES", pos) != "")
				subTemplates.insert(iniSubLoader.GetValue("SUBTEMPLATES", pos++));
		}

		std::string subTemplate;
		for (std::set<std::string>::iterator sit = subTemplates.begin(); sit != subTemplates.end(); sit++) {
			subTemplate = *sit;

			WindowTemplate *wndTemp = new(M_PLACE) WindowTemplate;
			wndTemp->LoadTemplate(templatePath, subTemplate);
			if (!wndTemp->GetFont()) {
				//AD2D_Font* _font = new(M_PLACE) AD2D_Font;
				//_font->CreatePNG_(files->GetFileData("font.png"), files->GetFileSize("font.png"));
				//_font->ReadWidthMap_(files->GetFileData("font.dat"), files->GetFileSize("font.dat"));
				wndTemp->SetFont(font);
			}

			AddWindowTemplate(templateGroup, wndTemp);
		}

		AD2D_Image* background = new(M_PLACE) AD2D_Image;
		background->CreatePNG_(files->GetFileData(templatePath + "/background.png"), files->GetFileSize(templatePath + "/background.png"));
		AddWindowTemplateGroupBackground(templateGroup, background);
	}

	/*if (!GetWindowTemplate(Game::options.templatesGroup, "main") ||
		!GetWindowTemplate(Game::options.templatesGroup, "game") ||
		!GetWindowTemplate(Game::options.templatesGroup, "menu"))
			Game::options.templatesGroup = templateGroup;*/
}

void Windows::ReleaseTemplates() {
	LOCKCLASS lockClass(lockWindows);

	TemplateGroups::iterator git = templatesMap.begin();
	for (git; git != templatesMap.end(); git++) {
		Templates::iterator it = git->second.begin();
		for (it; it != git->second.end(); it++) {
			WindowTemplate* temp = it->second;

			if (temp)
				delete_debug(temp, M_PLACE);
		}
	}

	TemplateBackgrounds::iterator bit = templateBackgrounds.begin();
	for (bit; bit != templateBackgrounds.end(); bit++) {
		AD2D_Image* bgImage = bit->second;
		if (bgImage)
			delete_debug(bgImage, M_PLACE);
	}
}

void Windows::AddWindowTemplate(std::string group, WindowTemplate* wndTemplate) {
	LOCKCLASS lockClass(lockWindows);

	if (!wndTemplate) {
		Logger::AddLog("Windows::AddWindowTemplate()", "Trying to add NULL pointer to map!", LOG_WARNING);
		return;
	}

	templatesMap[group][wndTemplate->templateName] = wndTemplate;
}

WindowTemplate* Windows::GetWindowTemplate(std::string group, std::string name) {
	LOCKCLASS lockClass(lockWindows);

	TemplateGroups::iterator git = templatesMap.find(group);
	if (git != templatesMap.end()) {
		Templates::iterator it = git->second.find(name);
		if (it != git->second.end())
			return it->second;

		Logger::AddLog("Windows::GetWindowTemplate()", "Template \"" + name + "\" not found!", LOG_WARNING);
		return NULL;
	}

	Logger::AddLog("Windows::GetWindowTemplate()", "Template group not found!", LOG_WARNING);
	return NULL;
}

void Windows::AddWindowTemplateGroupBackground(std::string group, AD2D_Image* background) {
	LOCKCLASS lockClass(lockWindows);

	templateBackgrounds[group] = background;
}

AD2D_Image* Windows::GetWindowTemplateGroupBackground(std::string group) {
	LOCKCLASS lockClass(lockWindows);

	TemplateBackgrounds::iterator it = templateBackgrounds.find(group);
	if (it != templateBackgrounds.end())
		return it->second;

	Logger::AddLog("Windows::GetWindowTemplateGroupBackground()", "TemplateBackground not found!", LOG_WARNING);

	return NULL;
}

std::list<std::string> Windows::GetTemplateGroups() {
	LOCKCLASS lockClass(lockWindows);

	std::list<std::string> list;

	TemplateGroups::iterator it = templatesMap.begin();
	for (it; it != templatesMap.end(); it++)
		list.push_back(it->first);

	return list;
}

POINT Windows::AlignWindowPosition(Window* wnd) {
	LOCKCLASS lockClass(lockWindows);

	POINT ret = { wnd->posX, wnd->posY };

	if (!wnd) {
		Logger::AddLog("Windows::AlignWindowPosition()", "NULL window pointer!", LOG_WARNING);
		return ret;
	}

	int left = wnd->posX;
	int right = wnd->posX + wnd->width;
	int top = wnd->posY;
	int bottom = wnd->posY + (wnd->minimized ? wnd->wndTemplate->tempHeader.c_height : wnd->height);

	std::list<Window*>::iterator it = wndList.begin();
	for (it; it != wndList.end(); it++) {
		Window* twnd = *it;

		if (twnd == wnd)
			continue;

		int tleft = twnd->posX;
		int tright = twnd->posX + twnd->width;
		int ttop = twnd->posY;
		int tbottom = twnd->posY + (twnd->minimized ? twnd->wndTemplate->tempHeader.c_height : twnd->height);

		if (right + 10 >= tleft && left - 10 <= tright &&
			bottom + 10 >= ttop && top - 10 <= tbottom) {

			if (left > tright - 10 && left < tright + 10) ret.x = tright;
			else if (left > tleft - 10 && left < tleft + 10) ret.x = tleft;
			if (right > tright - 10 && right < tright + 10) ret.x = tright - wnd->width;
			else if (right > tleft - 10 && right < tleft + 10) ret.x = tleft - wnd->width;
			if (top > tbottom - 10 && top < tbottom + 10) ret.y = tbottom;
			else if (top > ttop - 10 && top < ttop + 10) ret.y = ttop;
			if (bottom > tbottom - 10 && bottom < tbottom + 10) ret.y = tbottom - (bottom - top);
			else if (bottom > ttop - 10 && bottom < ttop + 10) ret.y = ttop - (bottom - top);
		}
	}

	if (gfx) {
		POINT size = gfx->GetWindowSize();

		int tleft = 0;
		int tright = size.x;
		int ttop = 0;
		int tbottom = size.y;

		if (right + 10 >= tleft && left - 10 <= tright &&
			bottom + 10 >= ttop && top - 10 <= tbottom) {

			if (left > tright - 10 && left < tright + 10) ret.x = tright;
			else if (left > tleft - 10 && left < tleft + 10) ret.x = tleft;
			if (right > tright - 10 && right < tright + 10) ret.x = tright - wnd->width;
			else if (right > tleft - 10 && right < tleft + 10) ret.x = tleft - wnd->width;
			if (top > tbottom - 10 && top < tbottom + 10) ret.y = tbottom;
			else if (top > ttop - 10 && top < ttop + 10) ret.y = ttop;
			if (bottom > tbottom - 10 && bottom < tbottom + 10) ret.y = tbottom - (bottom - top);
			else if (bottom > ttop - 10 && bottom < ttop + 10) ret.y = ttop - (bottom - top);
		}
	}

	return ret;
}

POINT Windows::AlignWindowSize(Window* wnd, bool _left, bool _right, bool _top, bool _bottom, std::list<Window*>* connected) {
	LOCKCLASS lockClass(lockWindows);

	POINT ret = { wnd->width, wnd->height };

	if (!wnd) {
		Logger::AddLog("Windows::AlignWindowSize()", "NULL window pointer!", LOG_WARNING);
		return ret;
	}

	int left = wnd->posX;
	int right = wnd->posX + wnd->width;
	int top = wnd->posY;
	int bottom = wnd->posY + (wnd->minimized ? wnd->wndTemplate->tempHeader.c_height : wnd->height);

	std::list<Window*>::iterator it = wndList.begin();
	for (it; it != wndList.end(); it++) {
		Window* twnd = *it;

		if (twnd == wnd || wnd->IsConnectedToWindow(twnd) || connected->end() != std::find(connected->begin(), connected->end(), twnd))
			continue;

		int tleft = twnd->posX;
		int tright = twnd->posX + twnd->width;
		int ttop = twnd->posY;
		int tbottom = twnd->posY + (twnd->minimized ? twnd->wndTemplate->tempHeader.c_height : twnd->height);

		if (right + 10 >= tleft && left - 10 <= tright &&
			bottom + 10 >= ttop && top - 10 <= tbottom)
		{
			if (_right && right > tright - 10 && right < tright + 10) ret.x = tright - left;
			else if (_right && right > tleft - 10 && right < tleft + 10) ret.x = tleft - left;
			else if (_left && left > tright - 10 && left < tright + 10) ret.x = right - tright;
			else if (_left && left > tleft - 10 && left < tleft + 10) ret.x = right - tleft;
			if (_bottom && bottom > tbottom - 10 && bottom < tbottom + 10) ret.y = tbottom - top;
			else if (_bottom && bottom > ttop - 10 && bottom < ttop + 10) ret.y = ttop - top;
			else if (_top && top > tbottom - 10 && top < tbottom + 10) ret.y = tbottom - bottom;
			else if (_top && top > ttop - 10 && top < ttop + 10) ret.y = ttop - bottom;
		}
	}

	if (gfx) {
		POINT size = gfx->GetWindowSize();

		int tleft = 0;
		int tright = size.x;
		int ttop = 0;
		int tbottom = size.y;

		if (_right && right > tright - 10 && right < tright + 10) ret.x = tright - left;
		else if (_right && right > tleft - 10 && right < tleft + 10) ret.x = tleft - left;
		else if (_left && left > tright - 10 && left < tright + 10) ret.x = right - tright;
		else if (_left && left > tleft - 10 && left < tleft + 10) ret.x = right - tleft;
		if (_bottom && bottom > tbottom - 10 && bottom < tbottom + 10) ret.y = tbottom - top;
		else if (_bottom && bottom > ttop - 10 && bottom < ttop + 10) ret.y = ttop - top;
		else if (_top && top > tbottom - 10 && top < tbottom + 10) ret.y = tbottom - bottom;
		else if (_top && top > ttop - 10 && top < ttop + 10) ret.y = ttop - bottom;
	}

	return ret;
}

void Windows::MoveConnectedWindows(Window* wnd, bool bleft, bool bright, bool btop, bool bbottom, int dx, int dy, std::list<Window*>* connected) {
	LOCKCLASS lockClass(lockWindows);

	if (!wnd) {
		Logger::AddLog("Windows::MoveConnectedWindows()", "NULL window pointer!", LOG_WARNING);
		return;
	}

	if (!bleft && !bright && !btop && !bbottom)
		return;

	int left = wnd->posX;
	int right = wnd->posX + wnd->width;
	int top = wnd->posY;
	int bottom = wnd->posY + (wnd->minimized ? wnd->wndTemplate->tempHeader.c_height : wnd->height);

	std::list<Window*> iconnected;
	if (connected)
		iconnected.assign(connected->begin(), connected->end());

	std::list<Window*>::iterator iit = std::find(iconnected.begin(), iconnected.end(), wnd);
	if (iit != iconnected.end())
		iconnected.erase(iit);

	std::list<Window*>::iterator it;
	std::list<Window*>::iterator itEnd;
	if (!connected) {
		it = wndList.begin();
		itEnd = wndList.end();
	}
	else {
		it = connected->begin();
		for (it; it != connected->end(); it++) {
			Window* twnd = *it;
			std::list<Window*>::iterator Tit = std::find(wndList.begin(), wndList.end(), twnd);
			if (Tit == wndList.end())
				*it = NULL;
		}
		it = connected->begin();
		itEnd = connected->end();
	}

	for (it; it != itEnd; it++) {
		Window* twnd = *it;

		if (twnd == NULL || twnd == wnd || twnd->alwaysActive || wnd->IsConnectedToWindow(twnd))
			continue;

		std::list<Window*>::iterator iit = std::find(iconnected.begin(), iconnected.end(), twnd);
		if (iit != iconnected.end())
			iconnected.erase(iit);

		int tleft = twnd->posX;
		int tright = twnd->posX + twnd->width;
		int ttop = twnd->posY;
		int tbottom = twnd->posY + (twnd->minimized ? twnd->wndTemplate->tempHeader.c_height : twnd->height);

		if ((((bright && right - dx >= tleft) || (bleft && left - dx <= tright)) ||
			((bbottom && bottom - dy >= ttop) || (btop && top - dy <= tbottom))) && !(
			((right - dx == tleft) || (left - dx == tright)) &&
			((bottom - dy == ttop) || (top - dy == tbottom)) ))
		{
			int newPosX = twnd->posX;
			int newPosY = twnd->posY;
			if ((bright && right - dx == tleft) || (bleft && left - dx == tright))
				newPosX = twnd->posX + dx;
			if ((bbottom && bottom - dy == ttop) || (btop && top - dy == tbottom))
				newPosY = twnd->posY + dy;

			int ndx = newPosX - twnd->posX;
			int ndy = newPosY - twnd->posY;
			if (ndx != 0 || ndy != 0) {
				twnd->SetPosition(newPosX, newPosY);
				MoveConnectedWindows(twnd, ndx < 0, ndx > 0, ndy < 0, ndy > 0, ndx, ndy, (connected ? &iconnected : NULL));
			}
		}
	}
}

std::list<Window*> Windows::GetConnectedWindows(Window* wnd, std::list<Window*>* currentList) {
	LOCKCLASS lockClass(lockWindows);

	std::list<Window*> connected;

	if (!wnd) {
		Logger::AddLog("Windows::GetConnectedWindows()", "NULL window pointer!", LOG_WARNING);
		return connected;
	}

	if (!currentList)
		connected.push_back(wnd);
	else if (!currentList->empty())
		connected.insert(connected.end(), currentList->begin(), currentList->end());

	int left = wnd->posX;
	int right = wnd->posX + wnd->width;
	int top = wnd->posY;
	int bottom = wnd->posY + (wnd->minimized ? wnd->wndTemplate->tempHeader.c_height : wnd->height);

	std::list<Window*>::iterator it = wndList.begin();
	for (it; it != wndList.end(); it++) {
		Window* twnd = *it;

		if (twnd == wnd)
			continue;

		if (currentList) {
			std::list<Window*>::iterator it = std::find(currentList->begin(), currentList->end(), twnd);
			if (it != currentList->end())
				continue;
		}

		int tleft = twnd->posX;
		int tright = twnd->posX + twnd->width;
		int ttop = twnd->posY;
		int tbottom = twnd->posY + (twnd->minimized ? twnd->wndTemplate->tempHeader.c_height : twnd->height);

		if (((right == tleft || left == tright) && top <= tbottom && bottom >= ttop) ||
			((bottom == ttop || top == tbottom) && left <= tright && right >= tleft))
		{
			connected.push_back(twnd);

			//std::list<Window*> tconnected = GetConnectedWindows(twnd, &connected);
			//if (!tconnected.empty())
			//	connected.assign(tconnected.begin(), tconnected.end());
		}
	}

	return connected;
}

Window* Windows::GetTopWindow() {
	LOCKCLASS lockClass(lockWindows);

	Window* wnd = (wndList.size() ? wndList.back() : NULL);
	return wnd;
}

Window* Windows::GetWindowUnderCursor(Mouse& mouse) {
	LOCKCLASS lockClass(lockWindows);

	Window* underCursor = NULL;
	for (std::list<Window*>::iterator it = wndList.begin(); it != wndList.end(); it++) {
		Window* wnd = *it;
		if (wnd && wnd->IsUnderCursor(mouse))
			underCursor = wnd;
	}

	return underCursor;
}

void Windows::CheckHolder(Mouse& mouse, Keyboard& keyboard, RealTime& realTime, bool inMessageLoop) {
	LOCKCLASS lockClass(lockWindows);

	Window* active = GetTopWindow();
	Window* underCursor = GetWindowUnderCursor(mouse);
	HOLDER holder = mouse.GetHolder();

	if (holder.type != 0x00) {
		if (holder.type == 0x01) {
			Window* wnd = (Window*)holder.window;
			std::list<Window*>* connectedList = (std::list<Window*>*)holder.variable;
			if (wnd && (mouse.curX != mouse.oldCurX || mouse.curY != mouse.oldCurY)) {
				POINT margin = GetWindowMargin();
				POINT wndSize = GetWindowSize();
				int newPosX = mouse.curX + holder.posX;
				int newPosY = mouse.curY + holder.posY;
				if (newPosX < 0) newPosX = 0;
				if (newPosY < 0) newPosY = 0;

				bool fixedPos = IsFixedPosWindow((WINDOW_TYPE)wnd->wndType) && Game::options.fixedPositions;
				if (fixedPos) {
					if (newPosX < (wndSize.x - wnd->width) / 2)
						newPosX = margin.x;
					else
						newPosX = wndSize.x - margin.y - wnd->width;

					if (wnd->posX != newPosX) {
						FitFixedWindows(wnd, - wnd->height);

						Window* twnd = GetCollidingWindow(newPosX, newPosY, wnd);
						if (twnd) {
							newPosY = twnd->posY;
							FitFixedWindows(twnd, wnd->height);
							twnd->SetPosition(twnd->posX, twnd->posY + wnd->height);
						}

						wnd->SetPosition(newPosX, newPosY);
					}
					else {
						Window* twnd = GetCollidingWindow(newPosX, newPosY, wnd);
						if (twnd) {
							int offset = 0;
							if (newPosY > wnd->posY && newPosY < wnd->posY + twnd->height / 2) {
								newPosY = twnd->posY - wnd->height;
							}
							else if (newPosY > wnd->posY && newPosY >= wnd->posY + twnd->height / 2) {
								offset =
								newPosY = twnd->posY + twnd->height - wnd->height;
								offset = - wnd->height;
								//twnd->SetPosition(twnd->posX, newPosY - twnd->height);
							}
							else if (newPosY < wnd->posY && newPosY >= twnd->posY + twnd->height / 2) {
								newPosY = twnd->posY + twnd->height;
							}
							else if (newPosY < wnd->posY && newPosY < twnd->posY + twnd->height / 2) {
								newPosY = twnd->posY;
								offset = wnd->height;
								//twnd->SetPosition(twnd->posX, newPosY + wnd->height);
							}

							if (offset > 0) {
								wnd->SetPosition(newPosX, newPosY);
								FitFixedWindows(twnd, offset);
								twnd->SetPosition(twnd->posX, twnd->posY + offset);
							}
							else if (offset < 0) {
								twnd->SetPosition(twnd->posX, twnd->posY + offset);
								FitFixedWindows(wnd, offset);
								wnd->SetPosition(newPosX, newPosY);
							}
							else
								wnd->SetPosition(newPosX, newPosY);
						}
						else {
							int offset = newPosY - wnd->posY;
							FitFixedWindows(wnd, offset);
							wnd->SetPosition(newPosX, newPosY);
						}
					}
				}
				else
					wnd->SetPosition(newPosX, newPosY);

				if (!keyboard.key[VK_SHIFT]) {
					POINT np = AlignWindowPosition(wnd);
					if (!fixedPos)
						wnd->SetPosition(np.x, np.y);
					else {
						int offset = np.y - wnd->posY;
						FitFixedWindows(wnd, offset);
						wnd->SetPosition(wnd->posX, np.y);
					}
				}

				if (fixedPos)
					FitGameWindow();
			}
		}
		else if (holder.type == 0x02) {
			Window* wnd = (Window*)holder.window;
			std::list<Window*>* connectedList = (std::list<Window*>*)holder.variable;
			if (wnd && (mouse.curX != mouse.oldCurX || mouse.curY != mouse.oldCurY)) {
				int width = (holder.posX != 0 ? mouse.curX - wnd->posX + holder.posX : wnd->width);
				int height = (holder.posY != 0 ? mouse.curY - wnd->posY + holder.posY : wnd->height);

				int oldWidth = wnd->width;
				int oldHeight = wnd->height;

				if (Game::options.fixedPositions && (wnd->wndType == WND_GAME || wnd->wndType == WND_CONSOLE))
					width = oldWidth;

				wnd->SetSize(width, height);
				if (!keyboard.key[VK_SHIFT]) {
					POINT ns = AlignWindowSize(wnd, false, true, false, true, connectedList);

					wnd->SetSize(ns.x, ns.y);
				}

				int dx = wnd->width - oldWidth;
				int dy = wnd->height - oldHeight;

				//MoveConnectedWindows(wnd, false, true, false, true, dx, dy, connectedList);

				/*ConnectionMap::iterator it = wnd->connectedWindows.begin();
				for (it; it != wnd->connectedWindows.end(); it++) {
					Window* connected = it->first;
					//connected->SetSize(	(wnd->posY == connected->posY && wnd->height == connected->height ? connected->width - dx : wnd->width),
					//					(wnd->posX == connected->posX && wnd->width == connected->width ? connected->height - dy : wnd->height));
					MoveConnectedWindows(connected, false, true, false, true, dx, dy, connectedList);
				}*/
			}
		}
		else if (holder.type == 0x03) {
			Window* wnd = (Window*)holder.window;
			std::list<Window*>* connectedList = (std::list<Window*>*)holder.variable;
			if (wnd && (mouse.curX != mouse.oldCurX || mouse.curY != mouse.oldCurY)) {
				int width = (holder.posX != 0 ? holder.posX - mouse.curX : wnd->width);
				int height = (holder.posY != 0 ? holder.posY + mouse.curY : wnd->height);

				int oldPosX = wnd->posX;
				int oldWidth = wnd->width;
				int oldHeight = wnd->height;

				if (Game::options.fixedPositions && (wnd->wndType == WND_GAME || wnd->wndType == WND_CONSOLE))
					width = oldWidth;

				wnd->SetSize(-width, height, NULL, true);
				wnd->SetPosition(oldPosX - (wnd->width - oldWidth), wnd->posY, NULL, true);
				wnd->SetSize(width, height);

				if (!keyboard.key[VK_SHIFT]) {
					POINT ns = AlignWindowSize(wnd, true, false, false, true, connectedList);
					wnd->SetSize(-ns.x, ns.y, NULL, true);
					wnd->SetPosition(oldPosX - (wnd->width - oldWidth), wnd->posY, NULL, true);
					wnd->SetSize(ns.x, ns.y);
				}

				int dx = wnd->width - oldWidth;
				int dy = wnd->height - oldHeight;

				//MoveConnectedWindows(wnd, true, false, false, false, -dx, dy, connectedList);
				//MoveConnectedWindows(wnd, false, false, false, true, -dx, dy, connectedList);

				/*ConnectionMap::iterator it = wnd->connectedWindows.begin();
				for (it; it != wnd->connectedWindows.end(); it++) {
					Window* connected = it->first;
					//connected->SetSize(	(wnd->posY == connected->posY && wnd->height == connected->height ? connected->width - dx : wnd->width),
					//					(wnd->posX == connected->posX && wnd->width == connected->width ? connected->height - dy : wnd->height));
					MoveConnectedWindows(connected, true, false, false, false, -dx, dy, connectedList);
					MoveConnectedWindows(connected, false, false, false, true, -dx, dy, connectedList);
				}*/
			}
		}
		else if (holder.type == 0x04) {
			Window* wnd = (Window*)holder.window;
			std::list<Window*>* connectedList = (std::list<Window*>*)holder.variable;
			if (wnd && (mouse.curX != mouse.oldCurX || mouse.curY != mouse.oldCurY)) {
				int width = (holder.posX != 0 ? holder.posX - mouse.curX : wnd->width);
				int height = (holder.posY != 0 ? holder.posY - mouse.curY : wnd->height);

				int oldPosX = wnd->posX;
				int oldPosY = wnd->posY;
				int oldWidth = wnd->width;
				int oldHeight = wnd->height;

				if (Game::options.fixedPositions && (wnd->wndType == WND_GAME || wnd->wndType == WND_CONSOLE))
					width = oldWidth;

				if (wnd->wndType == WND_GAME || wnd->wndType == WND_CONSOLE)
					height = oldHeight;

				wnd->SetSize(-width, -height, NULL, true);
				wnd->SetPosition(oldPosX - (wnd->width - oldWidth), oldPosY - (wnd->height - oldHeight), NULL, true);
				wnd->SetSize(width, height);

				if (!keyboard.key[VK_SHIFT]) {
					POINT ns = AlignWindowSize(wnd, true, false, true, false, connectedList);
					wnd->SetSize(-ns.x, -ns.y, NULL, true);
					wnd->SetPosition(oldPosX - (wnd->width - oldWidth), oldPosY - (wnd->height - oldHeight), NULL, true);
					wnd->SetSize(ns.x, ns.y);
				}

				int dx = wnd->width - oldWidth;
				int dy = wnd->height - oldHeight;

				//MoveConnectedWindows(wnd, false, true, false, false, -dx, -dy, connectedList);
				//MoveConnectedWindows(wnd, false, false, true, false, -dx, -dy, connectedList);

				/*ConnectionMap::iterator it = wnd->connectedWindows.begin();
				for (it; it != wnd->connectedWindows.end(); it++) {
					Window* connected = it->first;
					//connected->SetSize(	(wnd->posY == connected->posY && wnd->height == connected->height ? connected->width - dx : wnd->width),
					//					(wnd->posX == connected->posX && wnd->width == connected->width ? connected->height - dy : wnd->height));
					MoveConnectedWindows(connected, false, true, false, false, -dx, -dy, connectedList);
					MoveConnectedWindows(connected, false, false, true, false, -dx, -dy, connectedList);
				}*/
			}
		}
		else if (holder.type == 0x11) {
			WindowElementContainer* container = (WindowElementContainer*)holder.holder;
			if (container && (mouse.curX != mouse.oldCurX || mouse.curY != mouse.oldCurY)) {
				int borderH = 0;
				if (container->border && container->wndTemplate->GetElements() & ELEMENT_CONTAINER)
					borderH = container->wndTemplate->tempContainer.l_width + container->wndTemplate->tempContainer.r_width;

				int horizontal = holder.posY;
				container->scrollH = (float)(horizontal * (mouse.curX - holder.posX)) / (container->width - borderH - container->wndTemplate->tempScroll.l.GetWidth() - container->wndTemplate->tempScroll.r.GetWidth() - container->wndTemplate->tempScroll.brh.GetWidth());
				if (container->scrollH < 0.0f) container->scrollH = 0.0f;
				if (container->scrollH > (float)horizontal) container->scrollH = (float)horizontal;
			}
		}
		else if (holder.type == 0x12) {
			WindowElementContainer* container = (WindowElementContainer*)holder.holder;
			if (container && (mouse.curX != mouse.oldCurX || mouse.curY != mouse.oldCurY)) {
				int borderV = 0;
				if (container->border && container->wndTemplate->GetElements() & ELEMENT_CONTAINER)
					borderV = container->wndTemplate->tempContainer.t_height + container->wndTemplate->tempContainer.b_height;

				POINT offset = container->GetScrollOffset();
				int h = (offset.x || container->hAlwaysVisible ? container->wndTemplate->tempScroll.h : 0);

				int vertical = holder.posY;
				container->scrollV = (float)(vertical * (mouse.curY - holder.posX)) / (container->height - borderV - container->wndTemplate->tempScroll.t.GetHeight() - container->wndTemplate->tempScroll.b.GetHeight() - container->wndTemplate->tempScroll.brv.GetHeight() - h);
				if (container->scrollV < 0.0f) container->scrollV = 0.0f;
				if (container->scrollV > (float)vertical) container->scrollV = (float)vertical;
			}
		}
		else if (holder.type == 0x13 && !inMessageLoop) {
			WindowElementContainer* container = (WindowElementContainer*)holder.holder;
			if (container) {
				POINT offset = container->GetScrollOffset();

				container->scrollH += holder.posX * realTime.getFactor() * 100;
				container->scrollV += holder.posY * realTime.getFactor() * 100;

				if (container->scrollH < 0.0f) container->scrollH = 0.0f;
				else if (container->scrollH > (float)offset.x) container->scrollH = (float)offset.x;

				if (container->scrollV < 0.0f) container->scrollV = 0.0f;
				else if (container->scrollV > (float)offset.y) container->scrollV = (float)offset.y;
			}
		}
		else if (holder.type == 0x14) {
			WindowElementTextarea* textarea = (WindowElementTextarea*)holder.holder;
			if (textarea && (mouse.curX != mouse.oldCurX || mouse.curY != mouse.oldCurY)) {
				int vertical = holder.posY;
				textarea->scrollV = (float)(vertical * (mouse.curY - holder.posX)) / (textarea->height - textarea->wndTemplate->tempTextarea.t_height - textarea->wndTemplate->tempTextarea.b_height - textarea->wndTemplate->tempScroll.t.GetHeight() - textarea->wndTemplate->tempScroll.b.GetHeight() - textarea->wndTemplate->tempScroll.brv.GetHeight());
				if (textarea->scrollV < 0.0f) textarea->scrollV = 0.0f;
				else if (textarea->scrollV > (float)vertical) textarea->scrollV = (float)vertical;
			}
		}
		else if (holder.type == 0x15 && !inMessageLoop) {
			WindowElementTextarea* textarea = (WindowElementTextarea*)holder.holder;
			if (textarea) {
				int offset = textarea->GetScrollOffset();

				textarea->scrollV += holder.posY * realTime.getFactor() * 100;

				if (textarea->scrollV < 0.0f) textarea->scrollV = 0.0f;
				else if (textarea->scrollV > (float)offset) textarea->scrollV = (float)offset;
			}
		}
		else if (holder.type == 0x16) {
			WindowElement* memo = (WindowElement*)holder.holder;
			WindowElementMemo* memo1 = dynamic_cast<WindowElementMemo*>(memo);
			WindowElementTableMemo* memo2 = dynamic_cast<WindowElementTableMemo*>(memo);
			if (memo1 && (mouse.curX != mouse.oldCurX || mouse.curY != mouse.oldCurY)) {
				int vertical = holder.posY;
				memo1->scrollV = (float)(vertical * (mouse.curY - holder.posX)) / (memo1->height - memo1->wndTemplate->tempMemo.t_height - memo1->wndTemplate->tempMemo.b_height - memo1->wndTemplate->tempScroll.t.GetHeight() - memo1->wndTemplate->tempScroll.b.GetHeight() - memo1->wndTemplate->tempScroll.brv.GetHeight());
				if (memo1->scrollV < 0.0f) memo1->scrollV = 0.0f;
				else if (memo1->scrollV > (float)vertical) memo1->scrollV = (float)vertical;
			}
			else if (memo2 && (mouse.curX != mouse.oldCurX || mouse.curY != mouse.oldCurY)) {
				int vertical = holder.posY;
				memo2->scrollV = (float)(vertical * (mouse.curY - holder.posX)) / (memo2->height - memo2->wndTemplate->tempMemo.t_height - memo2->wndTemplate->tempMemo.b_height - memo2->wndTemplate->tempScroll.t.GetHeight() - memo2->wndTemplate->tempScroll.b.GetHeight() - memo2->wndTemplate->tempScroll.brv.GetHeight());
				if (memo2->scrollV < 0.0f) memo2->scrollV = 0.0f;
				else if (memo2->scrollV > (float)vertical) memo2->scrollV = (float)vertical;
			}
		}
		else if (holder.type == 0x17 && !inMessageLoop) {
			WindowElement* memo = (WindowElement*)holder.holder;
			WindowElementMemo* memo1 = dynamic_cast<WindowElementMemo*>(memo);
			WindowElementTableMemo* memo2 = dynamic_cast<WindowElementTableMemo*>(memo);
			if (memo1) {
				int offset = memo1->GetScrollOffset();

				memo1->scrollV += holder.posY * realTime.getFactor() * 100;

				if (memo1->scrollV < 0.0f) memo1->scrollV = 0.0f;
				else if (memo1->scrollV > (float)offset) memo1->scrollV = (float)offset;
			}
			else if (memo2) {
				int offset = memo2->GetScrollOffset();

				memo2->scrollV += holder.posY * realTime.getFactor() * 100;

				if (memo2->scrollV < 0.0f) memo2->scrollV = 0.0f;
				else if (memo2->scrollV > (float)offset) memo2->scrollV = (float)offset;
			}
		}
		else if (holder.type == 0x18) {
			WindowElementScrollBar* scrollBar = (WindowElementScrollBar*)holder.holder;
			if (scrollBar && (mouse.curX != mouse.oldCurX || mouse.curY != mouse.oldCurY) && (scrollBar->GetMinValue() != scrollBar->GetMaxValue())) {
				float oldValue = scrollBar->GetValue();
				float newValue = scrollBar->GetMinValue() + ((scrollBar->GetMaxValue() - scrollBar->GetMinValue()) * (mouse.curX - holder.posX)) / (float)(holder.posY);

				if (newValue < scrollBar->GetMinValue()) newValue = scrollBar->GetMinValue();
				else if (newValue > scrollBar->GetMaxValue()) newValue = scrollBar->GetMaxValue();

				scrollBar->SetValue(newValue);

				if (scrollBar->func.IsExecutable() && oldValue != scrollBar->GetValue())
					scrollBar->func.Execute();
			}
		}
		else if (holder.type == 0x19 && !inMessageLoop) {
			WindowElementScrollBar* scrollBar = (WindowElementScrollBar*)holder.holder;
			if (scrollBar && (scrollBar->GetMinValue() != scrollBar->GetMaxValue())) {
				float oldValue = scrollBar->GetValue();
				float newValue = scrollBar->GetValue() + holder.posY * realTime.getFactor() * (scrollBar->GetMaxValue() - scrollBar->GetMinValue()) / 2;

				if (newValue < scrollBar->GetMinValue()) newValue = scrollBar->GetMinValue();
				else if (newValue > scrollBar->GetMaxValue()) newValue = scrollBar->GetMaxValue();

				scrollBar->SetValue(newValue);

				if (scrollBar->func.IsExecutable() && oldValue != scrollBar->GetValue())
					scrollBar->func.Execute();
			}
		}
		else if (holder.type == 0x20) {
			Window* wnd = (Window*)holder.window;
			if (wnd) {
				WindowElementTextarea* textarea = (WindowElementTextarea*)holder.holder;

				POINT absPos = textarea->GetAbsolutePosition(true);

				POINT pos;
				pos.x = (mouse.curX + (int)textarea->scrollH - (absPos.x + textarea->wndTemplate->tempTextarea.l_width)) / textarea->fontSize;
				pos.y = (mouse.curY + (int)textarea->scrollV - (absPos.y + textarea->wndTemplate->tempTextarea.t_height)) / textarea->fontSize;
				float pix = mouse.curX + (int)textarea->scrollH - (absPos.x + textarea->wndTemplate->tempTextarea.l_width);

				if (pos.x >= 0 && pos.y >= 0) {
					textarea->SetCursor(pos, pix);
					textarea->GetCursor(pos, pix);
					POINT hpos = {holder.posX, holder.posY};
					if (pos < hpos) {
						textarea->curBegin = pos;
						textarea->curEnd = hpos;
					}
					else if (pos == hpos) {
						textarea->curBegin = hpos;
						textarea->curEnd = hpos;
					}
					else if (pos > hpos) {
						textarea->curBegin = hpos;
						textarea->curEnd = pos;
					}
				}
			}
		}
		else if (holder.type == 0x30) {
			Window* wnd = (Window*)holder.window;
			if (wnd) {
				WindowElementButton* button = (WindowElementButton*)holder.holder;

				POINT absPos = button->GetAbsolutePosition(true);

				if (mouse.curX >= absPos.x && mouse.curX < absPos.x + button->width &&
					mouse.curY >= absPos.y && mouse.curY < absPos.y + button->height) button->pressed = true;
				else
					button->pressed = false;
			}
		}
		else if (holder.type == 0x40) {
			WindowElementButton* hbutton = (WindowElementButton*)holder.holder;

			POINT absPos = hbutton->GetAbsolutePosition(true);
			if (mouse.curX < absPos.x && mouse.curX < mouse.oldCurX) {
				WindowElementTab* tab = dynamic_cast<WindowElementTab*>(hbutton->parent->parent);
				if (tab)
					tab->MoveTab(hbutton, -1);
			}
			else if (mouse.curX > absPos.x + hbutton->width && mouse.curX > mouse.oldCurX) {
				WindowElementTab* tab = dynamic_cast<WindowElementTab*>(hbutton->parent->parent);
				if (tab)
					tab->MoveTab(hbutton, 1);
			}
		}
		else if (holder.type == 0x60) {
			Window* wnd = (Window*)holder.window;
			if (wnd) {
				WindowElementMiniMap* minimap = (WindowElementMiniMap*)holder.holder;

				minimap->SetOffsetPos(mouse.curX - holder.posX, mouse.curY - holder.posY);
			}
		}
	}
}

void Windows::CheckInput(Mouse& mouse, Keyboard& keyboard, InputActions& actions) {
	actions.actWalk = false;
	actions.actLook = false;
	actions.actUse = false;
	actions.actAttack = false;
	actions.actOpenMenu = false;

	int keyShift = VK_SHIFT;
	int keyControl = VK_CONTROL;
	int keyAlt = VK_MENU;

	if (Game::options.classicControls) {
		if (!(mouse.GetButtons() & MOUSE_LEFT) && (mouse.GetButtons() & MOUSE_OLD_LEFT) && !(mouse.GetButtons() & MOUSE_RIGHT)) {
			if (keyboard.key[keyShift])
				actions.actLook = true;
			else if (keyboard.key[keyControl])
				actions.actOpenMenu = true;
			else if (keyboard.key[keyAlt])
				actions.actUse = true;
			else
				actions.actWalk = true;
		}
		if (!(mouse.GetButtons() & MOUSE_RIGHT) && (mouse.GetButtons() & MOUSE_OLD_RIGHT) && !(mouse.GetButtons() & MOUSE_LEFT)) {
			if (keyboard.key[keyShift])
				actions.actLook = true;
			else if (keyboard.key[keyControl])
				actions.actOpenMenu = true;
			else if (keyboard.key[keyAlt])
				actions.actUse = true;
			else {
				actions.actAttack = true;
				actions.actUse = true;
			}
		}
	}
	else {
		if (!(mouse.GetButtons() & MOUSE_LEFT) && (mouse.GetButtons() & MOUSE_OLD_LEFT) && !(mouse.GetButtons() & MOUSE_RIGHT)) {
			if (keyboard.key[keyShift])
				actions.actLook = true;
			else if (keyboard.key[keyControl])
				actions.actUse = true;
			else if (keyboard.key[keyAlt])
				actions.actAttack = true;
			else
				actions.actWalk = true;
		}
		if (!(mouse.GetButtons() & MOUSE_RIGHT) && (mouse.GetButtons() & MOUSE_OLD_RIGHT) && !(mouse.GetButtons() & MOUSE_LEFT)) {
			if (keyboard.key[keyShift])
				actions.actLook = true;
			else if (keyboard.key[keyControl])
				actions.actUse = true;
			else if (keyboard.key[keyAlt])
				actions.actAttack = true;
			else
				actions.actOpenMenu = true;
		}
	}
}

void Windows::Check(Mouse& mouse, Keyboard& keyboard, RealTime& realTime) {
	LOCKCLASS lockClass1(game->lockGame);
	LOCKCLASS lockClass2(lockWindows);

	Window* underCursor = GetWindowUnderCursor(mouse);
	Window* active = GetTopWindow();

	bool actioned = false;

	if (active && keyboard.keyChar != 0) {
		if (active->wndType == WND_COUNTER) {
			WindowElementScrollBar* scrollBar = dynamic_cast<WindowElementScrollBar*>(active->activeElement);
			if (keyboard.keyChar == 13) {
				active->SetAction(ACT_EXECUTE);
				keyboard.keyChar = 0;
			}
			else if (keyboard.keyChar == 27) {
				active->SetAction(ACT_CLOSE);
				keyboard.keyChar = 0;
			}
			else if (scrollBar)
				scrollBar->CheckInput(keyboard);
		}
		else if (active->wndType == WND_SETHOTKEY && keyboard.keyChar == 13)
			keyboard.keyChar = 0;
		else if (active->wndType == WND_SETHOTKEY && keyboard.keyChar == 27) {
			keyboard.keyChar = 0;
			active->ExecuteFunction();
		}
		else if (keyboard.keyChar == 13 && active->func.IsExecutable())
			active->SetAction(ACT_EXECUTE);
        else if (keyboard.keyChar == 27 && active->GetCloseAbility() && active->wndType != WND_LOGIN && active->wndType != WND_ADDFAVORITE)
			active->SetAction(ACT_CLOSE);
		else if (keyboard.keyChar == 9) {
			active->SkipActiveElement();
		}
		else if (active->activeElement) {
			WindowElementTextarea* textarea = dynamic_cast<WindowElementTextarea*>(active->activeElement);
			WindowElementMemo* memo = dynamic_cast<WindowElementMemo*>(active->activeElement);
			WindowElementTableMemo* tmemo = dynamic_cast<WindowElementTableMemo*>(active->activeElement);
			if (textarea)
				textarea->CheckInput(keyboard);
			else if (memo)
				memo->CheckInput(keyboard);
			else if (tmemo)
				tmemo->CheckInput(keyboard);
		}
	}

	HOLDER holder = mouse.GetHolder();

	if (holder.type == 0x00 &&
		underCursor && underCursor->wndTemplate->GetElements() & ELEMENT_BORDER && !underCursor->minimized &&
		underCursor->wndType != WND_STATUS && underCursor->wndType != WND_HOTKEYS && underCursor->wndType != WND_CONSOLE)
	{
		if ((underCursor->minWidth != underCursor->maxWidth || underCursor->minHeight != underCursor->maxHeight) &&
			mouse.curX >= underCursor->posX + underCursor->width - underCursor->wndTemplate->tempBorder.br_width && mouse.curX < underCursor->posX + underCursor->width + underCursor->wndTemplate->tempBorder.br_x &&
			mouse.curY >= underCursor->posY + underCursor->height - underCursor->wndTemplate->tempBorder.br_height && mouse.curY < underCursor->posY + underCursor->height + underCursor->wndTemplate->tempBorder.br_y)
		{
			mouse.cursor = 1;
		}
		else if (underCursor->minHeight != underCursor->maxHeight &&
			mouse.curX >= underCursor->posX + underCursor->wndTemplate->tempBorder.bl_width && mouse.curX < underCursor->posX + underCursor->width - underCursor->wndTemplate->tempBorder.br_width &&
			mouse.curY >= underCursor->posY + underCursor->height + underCursor->wndTemplate->tempBorder.b_y - underCursor->wndTemplate->tempBorder.b_height && mouse.curY < underCursor->posY + underCursor->height + underCursor->wndTemplate->tempBorder.b_y)
		{
			mouse.cursor = 2;
		}
		else if (underCursor->minWidth != underCursor->maxWidth &&
			mouse.curX >= underCursor->posX + underCursor->width + underCursor->wndTemplate->tempBorder.r_x - underCursor->wndTemplate->tempBorder.r_width && mouse.curX < underCursor->posX + underCursor->width + underCursor->wndTemplate->tempBorder.r_x &&
			mouse.curY >= underCursor->posY + underCursor->wndTemplate->tempHeader.c_height + underCursor->wndTemplate->tempBorder.tr_height && mouse.curY < underCursor->posY + underCursor->height - underCursor->wndTemplate->tempBorder.br_height)
		{
			mouse.cursor = 3;
		}
		else if ((underCursor->minWidth != underCursor->maxWidth || underCursor->minHeight != underCursor->maxHeight) &&
			mouse.curX >= underCursor->posX + underCursor->wndTemplate->tempBorder.bl_x && mouse.curX < underCursor->posX + underCursor->wndTemplate->tempBorder.bl_width &&
			mouse.curY >= underCursor->posY + underCursor->height - underCursor->wndTemplate->tempBorder.bl_height && mouse.curY < underCursor->posY + underCursor->height + underCursor->wndTemplate->tempBorder.bl_y)
		{
			mouse.cursor = 4;
		}
		else if (underCursor->minWidth != underCursor->maxWidth &&
			mouse.curX >= underCursor->posX + underCursor->wndTemplate->tempBorder.l_x && mouse.curX < underCursor->posX + underCursor->wndTemplate->tempBorder.l_width &&
			mouse.curY >= underCursor->posY + underCursor->wndTemplate->tempHeader.c_height + underCursor->wndTemplate->tempBorder.tr_height && mouse.curY < underCursor->posY + underCursor->height - underCursor->wndTemplate->tempBorder.br_height)
		{
			mouse.cursor = 3;
		}
		else if (underCursor->minHeight != underCursor->maxHeight &&
			mouse.curX >= underCursor->posX + underCursor->wndTemplate->tempBorder.bl_width && mouse.curX < underCursor->posX + underCursor->width - underCursor->wndTemplate->tempBorder.br_width &&
			mouse.curY >= underCursor->posY && mouse.curY < underCursor->posY + underCursor->wndTemplate->tempBorder.t_height + 1)
		{
			mouse.cursor = 2;
		}
		else
			mouse.cursor = 0;
	}
	else
		mouse.cursor = 0;

	if (holder.type == 0x00 && (mouse.GetButtons() & MOUSE_LEFT) && (mouse.GetButtons() & MOUSE_RIGHT))
		mouse.SetHolder(0, 0, 0xFF);

	if ((mouse.GetButtons() & MOUSE_LEFT) && !(mouse.GetButtons() & MOUSE_OLD_LEFT) && !(mouse.GetButtons() & MOUSE_RIGHT)) {
		if (active != underCursor) {
			if (active->activeElement && !active->activeElement->alwaysActive) {
				active->activeElement->active = false;
				active->activeElement = NULL;
			}
			MoveOnTop(underCursor);
			active = underCursor;
		}

		if (underCursor)
			underCursor->ActivateFunction();

		if (!underCursor || underCursor->wndType != WND_MENU)
			CloseWindows(WND_MENU);
		if (!underCursor || underCursor->wndType != WND_COUNTER)
			CloseWindows(WND_COUNTER);

		if (holder.type == 0x00)
			mouse.SetLock(mouse.curX, mouse.curY);

		WindowElement* element = NULL;
		WindowElementContainer* container = NULL;
		WindowElementLink* link = NULL;
		WindowElementButton* button = NULL;
		WindowElementTextarea* textarea = NULL;
		WindowElementMemo* memo = NULL;
		WindowElementTableMemo* tmemo = NULL;
		WindowElementItemContainer* itemContainer = NULL;
		WindowElementPlayground* playground = NULL;
		WindowElementCheckBox* checkBox = NULL;
		WindowElementScrollBar* scrollBar = NULL;
		if (underCursor) {
			RECT rect = {underCursor->posX, underCursor->posY, underCursor->posX + underCursor->width - 1, underCursor->posY + underCursor->height - 1};
			element = underCursor->GetWindowElementUnderCursor(rect, &underCursor->wndContainer, mouse.curX, mouse.curY);
			container = dynamic_cast<WindowElementContainer*>(element);
			link = dynamic_cast<WindowElementLink*>(element);
			button = dynamic_cast<WindowElementButton*>(element);
			textarea = dynamic_cast<WindowElementTextarea*>(element);
			memo = dynamic_cast<WindowElementMemo*>(element);
			tmemo = dynamic_cast<WindowElementTableMemo*>(element);
			itemContainer = dynamic_cast<WindowElementItemContainer*>(element);
			checkBox = dynamic_cast<WindowElementCheckBox*>(element);
			scrollBar = dynamic_cast<WindowElementScrollBar*>(element);
			playground = dynamic_cast<WindowElementPlayground*>(element);

			if (!underCursor->activeElement || !underCursor->activeElement->alwaysActive) {
				if (element) {
					if (underCursor->activeElement)
						underCursor->activeElement->active = false;
					underCursor->activeElement = element;
					element->active = true;
				}
			}
		}

		ContainerWindow* cntWindow = dynamic_cast<ContainerWindow*>(underCursor);
		Container* cntContainer = (cntWindow ? cntWindow->GetContainer() : NULL);

		int size = (underCursor ? underCursor->wndTemplate->tempWndIcons.w * 2 + (cntContainer && cntContainer->IsChild() ? underCursor->wndTemplate->tempWndIcons.w : 0) : 0);

		if (holder.type == 0x00 &&
			underCursor && underCursor->GetElements() & ELEMENT_HEADER && underCursor->wndTemplate->GetElements() & ELEMENT_HEADER &&
			mouse.curX >= underCursor->posX && mouse.curX < underCursor->posX + underCursor->width &&
			mouse.curY >= underCursor->posY && mouse.curY < underCursor->posY + underCursor->wndTemplate->tempHeader.c_height &&
			!(mouse.curX >= underCursor->posX + underCursor->width - underCursor->wndTemplate->tempWndIcons.x - size && mouse.curX < underCursor->posX + underCursor->width - underCursor->wndTemplate->tempWndIcons.x &&
			mouse.curY >= underCursor->posY + underCursor->wndTemplate->tempWndIcons.y && mouse.curY < underCursor->posY + underCursor->wndTemplate->tempWndIcons.y + underCursor->wndTemplate->tempWndIcons.h))
		{
			std::list<Window*>* connected = new(M_PLACE) std::list<Window*>;
			*connected = GetConnectedWindows(underCursor);

			mouse.SetHolder(underCursor->posX - mouse.curX,
							underCursor->posY - mouse.curY,
							0x01, (void*)underCursor, (void*)underCursor, (void*)connected);
		}

		if (holder.type == 0x00 &&
			underCursor && underCursor->wndTemplate->GetElements() & ELEMENT_BORDER &&
			underCursor->wndType != WND_STATUS && underCursor->wndType != WND_HOTKEYS && underCursor->wndType != WND_CONSOLE)
		{
			if ((underCursor->minWidth != underCursor->maxWidth || underCursor->minHeight != underCursor->maxHeight) &&
				mouse.curX >= underCursor->posX + underCursor->width - underCursor->wndTemplate->tempBorder.br_width && mouse.curX < underCursor->posX + underCursor->width + underCursor->wndTemplate->tempBorder.br_x &&
				mouse.curY >= underCursor->posY + underCursor->height - underCursor->wndTemplate->tempBorder.br_height && mouse.curY < underCursor->posY + underCursor->height + underCursor->wndTemplate->tempBorder.br_y)
			{
				std::list<Window*>* connected = new(M_PLACE) std::list<Window*>;
				*connected = GetConnectedWindows(underCursor);

				mouse.SetHolder(underCursor->posX + underCursor->width - mouse.curX,
								underCursor->posY + underCursor->height - mouse.curY,
								0x02, (void*)underCursor,(void*)underCursor,(void*)connected);
			}
			else if (underCursor->minHeight != underCursor->maxHeight &&
				mouse.curX >= underCursor->posX + underCursor->wndTemplate->tempBorder.bl_width && mouse.curX < underCursor->posX + underCursor->width - underCursor->wndTemplate->tempBorder.br_width &&
				mouse.curY >= underCursor->posY + underCursor->height + underCursor->wndTemplate->tempBorder.b_y - underCursor->wndTemplate->tempBorder.b_height && mouse.curY < underCursor->posY + underCursor->height + underCursor->wndTemplate->tempBorder.b_y)
			{
				std::list<Window*>* connected = new(M_PLACE) std::list<Window*>;
				*connected = GetConnectedWindows(underCursor);

				mouse.SetHolder(0,
								underCursor->posY + underCursor->height - mouse.curY,
								0x02, (void*)underCursor,(void*)underCursor,(void*)connected);
			}
			else if (underCursor->minWidth != underCursor->maxWidth &&
				mouse.curX >= underCursor->posX + underCursor->width + underCursor->wndTemplate->tempBorder.r_x - underCursor->wndTemplate->tempBorder.r_width && mouse.curX < underCursor->posX + underCursor->width + underCursor->wndTemplate->tempBorder.r_x &&
				mouse.curY >= underCursor->posY + underCursor->wndTemplate->tempHeader.c_height + underCursor->wndTemplate->tempBorder.tr_height && mouse.curY < underCursor->posY + underCursor->height - underCursor->wndTemplate->tempBorder.br_height)
			{
				std::list<Window*>* connected = new(M_PLACE) std::list<Window*>;
				*connected = GetConnectedWindows(underCursor);

				mouse.SetHolder(underCursor->posX + underCursor->width - mouse.curX,
								0,
								0x02, (void*)underCursor,(void*)underCursor,(void*)connected);
			}
			else if ((underCursor->minWidth != underCursor->maxWidth || underCursor->minHeight != underCursor->maxHeight) &&
				mouse.curX >= underCursor->posX + underCursor->wndTemplate->tempBorder.bl_x && mouse.curX < underCursor->posX + underCursor->wndTemplate->tempBorder.bl_width &&
				mouse.curY >= underCursor->posY + underCursor->height - underCursor->wndTemplate->tempBorder.bl_height && mouse.curY < underCursor->posY + underCursor->height + underCursor->wndTemplate->tempBorder.bl_y)
			{
				std::list<Window*>* connected = new(M_PLACE) std::list<Window*>;
				*connected = GetConnectedWindows(underCursor);

				mouse.SetHolder(underCursor->width + mouse.curX,
								underCursor->height - mouse.curY,
								0x03, (void*)underCursor,(void*)underCursor,(void*)connected);
			}
			else if (underCursor->minWidth != underCursor->maxWidth &&
				mouse.curX >= underCursor->posX + underCursor->wndTemplate->tempBorder.l_x && mouse.curX < underCursor->posX + underCursor->wndTemplate->tempBorder.l_width &&
				mouse.curY >= underCursor->posY + underCursor->wndTemplate->tempHeader.c_height + underCursor->wndTemplate->tempBorder.tr_height && mouse.curY < underCursor->posY + underCursor->height - underCursor->wndTemplate->tempBorder.br_height)
			{
				std::list<Window*>* connected = new(M_PLACE) std::list<Window*>;
				*connected = GetConnectedWindows(underCursor);

				mouse.SetHolder(underCursor->width + mouse.curX,
								0,
								0x03, (void*)underCursor,(void*)underCursor,(void*)connected);
			}
			else if (underCursor->minHeight != underCursor->maxHeight &&
				mouse.curX >= underCursor->posX + underCursor->wndTemplate->tempBorder.bl_width && mouse.curX < underCursor->posX + underCursor->width - underCursor->wndTemplate->tempBorder.br_width &&
				mouse.curY >= underCursor->posY && mouse.curY < underCursor->posY + underCursor->wndTemplate->tempBorder.t_height + 1)
			{
				std::list<Window*>* connected = new(M_PLACE) std::list<Window*>;
				*connected = GetConnectedWindows(underCursor);

				mouse.SetHolder(0,
								underCursor->height + mouse.curY,
								0x04, (void*)underCursor,(void*)underCursor,(void*)connected);
			}
		}

		if (holder.type == 0x00 && element) {
			POINT absPos = element->GetAbsolutePosition(true);

			if (container && container->scroll && underCursor->wndTemplate->GetElements() & ELEMENT_SCROLL) {
				int left = 0;
				int right = 0;
				int top = 0;
				int bottom = 0;
				if (container->border) {
					left = container->wndTemplate->tempContainer.l_width;
					right = container->wndTemplate->tempContainer.r_width;
					top = container->wndTemplate->tempContainer.t_height;
					bottom = container->wndTemplate->tempContainer.b_height;
				}

				POINT offset = container->GetScrollOffset();
				if (offset.x) {
					int barpos = (int)((container->width - left - right - container->wndTemplate->tempScroll.l.GetWidth() - container->wndTemplate->tempScroll.r.GetWidth() - container->wndTemplate->tempScroll.brh.GetWidth()) * (container->scrollH / offset.x));
					if (mouse.curX >= absPos.x + left + container->wndTemplate->tempScroll.l.GetWidth() + barpos && mouse.curX < absPos.x + left + container->wndTemplate->tempScroll.l.GetWidth() + barpos + container->wndTemplate->tempScroll.brh.GetWidth() &&
						mouse.curY >= absPos.y + container->height - bottom - container->wndTemplate->tempScroll.brh.GetHeight() && mouse.curY < absPos.y + container->height - bottom)
					{
						mouse.SetHolder(mouse.curX - barpos, offset.x,
							0x11, (void*)underCursor, (void*)container);
					}
					else
					if (mouse.curX >= absPos.x + left && mouse.curX < absPos.x + left + container->wndTemplate->tempScroll.l.GetWidth() &&
						mouse.curY >= absPos.y + container->height - bottom - container->wndTemplate->tempScroll.l.GetHeight() && mouse.curY < absPos.y + container->height - bottom)
					{
						mouse.SetHolder(-1, 0,
							0x13, (void*)underCursor, (void*)container);
					}
					else
					if (mouse.curX >= absPos.x + container->width - right - container->wndTemplate->tempScroll.r.GetWidth() && mouse.curX < absPos.x + container->width - right &&
						mouse.curY >= absPos.y + container->height - bottom - container->wndTemplate->tempScroll.r.GetHeight() && mouse.curY < absPos.y + container->height - bottom)
					{
						mouse.SetHolder(1, 0,
							0x13, (void*)underCursor, (void*)container);
					}
				}
				if (offset.y) {
					int h = 0;
					if (offset.x) h = container->wndTemplate->tempScroll.h;

					int barpos = (int)((container->height - top - bottom - container->wndTemplate->tempScroll.t.GetHeight() - container->wndTemplate->tempScroll.b.GetHeight() - container->wndTemplate->tempScroll.brv.GetHeight() - h) * (container->scrollV / offset.y));
					if (mouse.curX >= absPos.x + container->width - right - container->wndTemplate->tempScroll.bgv.GetWidth() && mouse.curX < absPos.x + container->width - right &&
						mouse.curY >= absPos.y + top + container->wndTemplate->tempScroll.t.GetHeight() + barpos && mouse.curY < absPos.y + top + container->wndTemplate->tempScroll.t.GetHeight() + barpos + container->wndTemplate->tempScroll.brv.GetHeight())
					{
						mouse.SetHolder(mouse.curY - barpos, offset.y,
							0x12, (void*)underCursor, (void*)container);
					}
					else
					if (mouse.curX >= absPos.x + container->width - right - container->wndTemplate->tempScroll.t.GetWidth() && mouse.curX < absPos.x + container->width - right &&
						mouse.curY >= absPos.y + top && mouse.curY < absPos.y + top + container->wndTemplate->tempScroll.t.GetHeight())
					{
						mouse.SetHolder(0, -1,
							0x13, (void*)underCursor, (void*)container);
					}
					else
					if (mouse.curX >= absPos.x + container->width - right - container->wndTemplate->tempScroll.b.GetWidth() && mouse.curX < absPos.x + container->width - right &&
						mouse.curY >= absPos.y + container->height - bottom - container->wndTemplate->tempScroll.b.GetHeight() - h && mouse.curY < absPos.y + container->height - bottom - h)
					{
						mouse.SetHolder(0, 1,
							0x13, (void*)underCursor, (void*)container);
					}
				}
			}

			if (button && button->enabled && button->visible) {
				if (button->type == BUTTON_NORMAL || button->type == BUTTON_TAB_SIDE) {
					mouse.SetHolder(0, 0, 0x30, (void*)underCursor, (void*)button);
				}
				else if (button->type == BUTTON_SWITCH) {
					if (button->pressed) button->pressed = false;
					else button->pressed = true;
					if (button->func.IsExecutable())
						button->func.Execute();
				}
				else if (button->type == BUTTON_TAB) {
					if (button->GetCloseIcon() &&
						mouse.curX >= absPos.x + button->width - button->wndTemplate->tempTab.x.GetWidth() && mouse.curX < absPos.x + button->width &&
						mouse.curY >= absPos.y && mouse.curY < absPos.y + button->wndTemplate->tempTab.x.GetHeight())
					{
						WindowElementContainer* parent = button->parent;
						if (parent) {
							WindowElementTab* tab = dynamic_cast<WindowElementTab*>(parent->parent);
						}
					}
					else {
						mouse.SetHolder(0, 0, 0x40, (void*)underCursor, (void*)button);
						if (button->func.IsExecutable())
							button->func.Execute();
					}
				}
				else {
					if (button->func.IsExecutable())
						button->func.Execute();
				}
			}

			if (textarea) {
				int scrollW = (textarea->multiline ? textarea->wndTemplate->tempScroll.w : 0);
				if (mouse.curX >= absPos.x + textarea->wndTemplate->tempTextarea.l_width && mouse.curX < absPos.x + textarea->width - textarea->wndTemplate->tempTextarea.r_width - scrollW &&
					mouse.curY >= absPos.y + textarea->wndTemplate->tempTextarea.t_height && mouse.curY < absPos.y + textarea->height - textarea->wndTemplate->tempTextarea.b_height)
				{
					POINT pos;
					pos.x = (mouse.curX + (int)textarea->scrollH - (absPos.x + textarea->wndTemplate->tempTextarea.l_width)) / textarea->fontSize;
					pos.y = (mouse.curY + (int)textarea->scrollV - (absPos.y + textarea->wndTemplate->tempTextarea.t_height)) / textarea->fontSize;
					float pix = mouse.curX + (int)textarea->scrollH - (absPos.x + textarea->wndTemplate->tempTextarea.l_width);

					if (pos.x >= 0 && pos.y >= 0) {
						textarea->SetCursor(pos, pix);
						textarea->GetCursor(pos, pix);
						textarea->curBegin = pos;
						textarea->curEnd = pos;
						mouse.SetHolder(pos.x, pos.y,
							0x20, (void*)underCursor, (void*)textarea);
					}
				}

				if (textarea->multiline && underCursor->wndTemplate->GetElements() & ELEMENT_SCROLL) {
					int offset = textarea->GetScrollOffset();
					if (offset) {
						int barpos = (int)((textarea->height - textarea->wndTemplate->tempTextarea.t_height - textarea->wndTemplate->tempTextarea.b_height - textarea->wndTemplate->tempScroll.t.GetHeight() - textarea->wndTemplate->tempScroll.b.GetHeight() - textarea->wndTemplate->tempScroll.brv.GetHeight()) * (textarea->scrollV / offset));
						if (mouse.curX >= absPos.x + textarea->width - textarea->wndTemplate->tempTextarea.r_width - textarea->wndTemplate->tempScroll.t.GetWidth() && mouse.curX < absPos.x + textarea->width - textarea->wndTemplate->tempTextarea.r_width &&
							mouse.curY >= absPos.y + textarea->wndTemplate->tempTextarea.t_height + textarea->wndTemplate->tempScroll.t.GetHeight() + barpos && mouse.curY < absPos.y + textarea->wndTemplate->tempTextarea.t_height + textarea->wndTemplate->tempScroll.t.GetHeight() + barpos + textarea->wndTemplate->tempScroll.brv.GetHeight())
						{
							mouse.SetHolder(mouse.curY - barpos, offset,
								0x14, (void*)underCursor, (void*)textarea);
						}
						else
						if (mouse.curX >= absPos.x + textarea->width - textarea->wndTemplate->tempTextarea.r_width - textarea->wndTemplate->tempScroll.t.GetWidth() && mouse.curX < absPos.x + textarea->width - textarea->wndTemplate->tempTextarea.r_width &&
							mouse.curY >= absPos.y + textarea->wndTemplate->tempTextarea.t_height && mouse.curY < absPos.y + textarea->wndTemplate->tempTextarea.t_height + textarea->wndTemplate->tempScroll.t.GetHeight())
						{
							mouse.SetHolder(0, -1,
								0x15, (void*)underCursor, (void*)textarea);
						}
						else
						if (mouse.curX >= absPos.x + textarea->width - textarea->wndTemplate->tempTextarea.r_width - textarea->wndTemplate->tempScroll.t.GetWidth() && mouse.curX < absPos.x + textarea->width - textarea->wndTemplate->tempTextarea.r_width &&
							mouse.curY >= absPos.y + textarea->height - textarea->wndTemplate->tempTextarea.b_height - textarea->wndTemplate->tempScroll.b.GetHeight() && mouse.curY < absPos.y + textarea->height - textarea->wndTemplate->tempTextarea.b_height)
						{
							mouse.SetHolder(0, 1,
								0x15, (void*)underCursor, (void*)textarea);
						}
					}
				}
			}

			if (memo) {
				if (mouse.curX >= absPos.x + memo->wndTemplate->tempMemo.l_width && mouse.curX < absPos.x + memo->width - memo->wndTemplate->tempMemo.r_width - memo->wndTemplate->tempScroll.w &&
					mouse.curY >= absPos.y + memo->wndTemplate->tempMemo.t_height && mouse.curY < absPos.y + memo->height - memo->wndTemplate->tempMemo.b_height)
				{
					unsigned short option = (mouse.curY + (int)memo->scrollV - (absPos.y + memo->wndTemplate->tempMemo.t_height)) / memo->fontSize;
					memo->SetOption(option);
				}

				if (underCursor->wndTemplate->GetElements() & ELEMENT_SCROLL) {
					int offset = memo->GetScrollOffset();
					if (offset) {
						int barpos = (int)((memo->height - memo->wndTemplate->tempMemo.t_height - memo->wndTemplate->tempMemo.b_height - memo->wndTemplate->tempScroll.t.GetHeight() - memo->wndTemplate->tempScroll.b.GetHeight() - memo->wndTemplate->tempScroll.brv.GetHeight()) * (memo->scrollV / offset));
						if (mouse.curX >= absPos.x + memo->width - memo->wndTemplate->tempMemo.r_width - memo->wndTemplate->tempScroll.brv.GetWidth() && mouse.curX < absPos.x + memo->width - memo->wndTemplate->tempMemo.r_width &&
							mouse.curY >= absPos.y + memo->wndTemplate->tempMemo.t_height + memo->wndTemplate->tempScroll.t.GetHeight() + barpos && mouse.curY < absPos.y + memo->wndTemplate->tempMemo.t_height + memo->wndTemplate->tempScroll.t.GetHeight() + barpos + memo->wndTemplate->tempScroll.brv.GetHeight())
						{
							mouse.SetHolder(mouse.curY - barpos, offset,
								0x16, (void*)underCursor, (void*)(WindowElement*)memo);
						}
						else
						if (mouse.curX >= absPos.x + memo->width - memo->wndTemplate->tempMemo.r_width - memo->wndTemplate->tempScroll.t.GetWidth() && mouse.curX < absPos.x + memo->width - memo->wndTemplate->tempMemo.r_width &&
							mouse.curY >= absPos.y + memo->wndTemplate->tempMemo.t_height && mouse.curY < absPos.y + memo->wndTemplate->tempMemo.t_height + memo->wndTemplate->tempScroll.t.GetHeight())
						{
							mouse.SetHolder(0, -1,
								0x17, (void*)underCursor, (void*)(WindowElement*)memo);
						}
						else
						if (mouse.curX >= absPos.x + memo->width - memo->wndTemplate->tempMemo.r_width - memo->wndTemplate->tempScroll.b.GetWidth() && mouse.curX < absPos.x + memo->width - memo->wndTemplate->tempMemo.r_width &&
							mouse.curY >= absPos.y + memo->height - memo->wndTemplate->tempMemo.b_height - memo->wndTemplate->tempScroll.b.GetHeight() && mouse.curY < absPos.y + memo->height - memo->wndTemplate->tempMemo.b_height)
						{
							mouse.SetHolder(0, 1,
								0x17, (void*)underCursor, (void*)(WindowElement*)memo);
						}
					}
				}
			}

			if (tmemo) {
				if (mouse.curX >= absPos.x + tmemo->wndTemplate->tempMemo.l_width && mouse.curX < absPos.x + tmemo->width - tmemo->wndTemplate->tempMemo.r_width - tmemo->wndTemplate->tempScroll.w &&
					mouse.curY >= absPos.y + tmemo->wndTemplate->tempMemo.t_height && mouse.curY < absPos.y + tmemo->height - tmemo->wndTemplate->tempMemo.b_height)
				{
					unsigned short option = (mouse.curY + (int)tmemo->scrollV - (absPos.y + tmemo->wndTemplate->tempMemo.t_height)) / tmemo->rowHeight;
					if (option > 0)
						tmemo->SetOption(option - 1);
					else if (tmemo->GetWindow() && tmemo->GetWindow()->GetWindowType() == WND_SERVERS) {
						int posX = mouse.curX - (absPos.x + tmemo->wndTemplate->tempMemo.l_width);
						int intWidth = tmemo->width - tmemo->wndTemplate->tempMemo.l_width - tmemo->wndTemplate->tempMemo.r_width;

						float wdh = 0.0f;
						for (int i = 0; i < tmemo->cols.size(); i++) {
							float oldWdh = wdh;
							if (tmemo->cols[i] > 0) wdh += tmemo->cols[i];
							else if (tmemo->cols[i] < 0) wdh -= (float)(intWidth * tmemo->cols[i]) / 100;
							else wdh += (float)intWidth / tmemo->cols.size();

							if (oldWdh <= posX && wdh > posX) {
								Servers* servers = (Game::game ? Game::game->GetServers() : NULL);
								if (servers) {
									servers->SortFavorites(i);
									servers->UpdateFavoritesMemo(tmemo);
									servers->SaveFavorites();
								}
								break;
							}
						}
					}
				}

				if (underCursor->wndTemplate->GetElements() & ELEMENT_SCROLL) {
					int offset = tmemo->GetScrollOffset();
					if (offset) {
						int barpos = (int)((tmemo->height - tmemo->wndTemplate->tempMemo.t_height - tmemo->wndTemplate->tempMemo.b_height - tmemo->wndTemplate->tempScroll.t.GetHeight() - tmemo->wndTemplate->tempScroll.b.GetHeight() - tmemo->wndTemplate->tempScroll.brv.GetHeight()) * (tmemo->scrollV / offset));
						if (mouse.curX >= absPos.x + tmemo->width - tmemo->wndTemplate->tempMemo.r_width - tmemo->wndTemplate->tempScroll.brv.GetWidth() && mouse.curX < absPos.x + tmemo->width - tmemo->wndTemplate->tempMemo.r_width &&
							mouse.curY >= absPos.y + tmemo->wndTemplate->tempMemo.t_height + tmemo->wndTemplate->tempScroll.t.GetHeight() + barpos && mouse.curY < absPos.y + tmemo->wndTemplate->tempMemo.t_height + tmemo->wndTemplate->tempScroll.t.GetHeight() + barpos + tmemo->wndTemplate->tempScroll.brv.GetHeight())
						{
							mouse.SetHolder(mouse.curY - barpos, offset,
								0x16, (void*)underCursor, (void*)(WindowElement*)tmemo);
						}
						else
						if (mouse.curX >= absPos.x + tmemo->width - tmemo->wndTemplate->tempMemo.r_width - tmemo->wndTemplate->tempScroll.t.GetWidth() && mouse.curX < absPos.x + tmemo->width - tmemo->wndTemplate->tempMemo.r_width &&
							mouse.curY >= absPos.y + tmemo->wndTemplate->tempMemo.t_height && mouse.curY < absPos.y + tmemo->wndTemplate->tempMemo.t_height + tmemo->wndTemplate->tempScroll.t.GetHeight())
						{
							mouse.SetHolder(0, -1,
								0x17, (void*)underCursor, (void*)(WindowElement*)tmemo);
						}
						else
						if (mouse.curX >= absPos.x + tmemo->width - tmemo->wndTemplate->tempMemo.r_width - tmemo->wndTemplate->tempScroll.b.GetWidth() && mouse.curX < absPos.x + tmemo->width - tmemo->wndTemplate->tempMemo.r_width &&
							mouse.curY >= absPos.y + tmemo->height - tmemo->wndTemplate->tempMemo.b_height - tmemo->wndTemplate->tempScroll.b.GetHeight() && mouse.curY < absPos.y + tmemo->height - tmemo->wndTemplate->tempMemo.b_height)
						{
							mouse.SetHolder(0, 1,
								0x17, (void*)underCursor, (void*)(WindowElement*)tmemo);
						}
					}
				}
			}

			if (scrollBar) {
				int offset = scrollBar->width - scrollBar->wndTemplate->tempScrollBar.l_width - scrollBar->wndTemplate->tempScrollBar.r_width - scrollBar->wndTemplate->tempScrollBar.br.GetWidth();
				int barpos = (int)((scrollBar->GetValue() - scrollBar->GetMinValue()) / (scrollBar->GetMaxValue() - scrollBar->GetMinValue()) * offset);
				if (mouse.curX >= absPos.x + barpos + scrollBar->wndTemplate->tempScrollBar.l_width && mouse.curX < absPos.x + barpos + scrollBar->wndTemplate->tempScrollBar.l_width + scrollBar->wndTemplate->tempScrollBar.br.GetWidth() &&
					mouse.curY >= absPos.y && mouse.curY < absPos.y + scrollBar->wndTemplate->tempScrollBar.height)
				{
					mouse.SetHolder(mouse.curX - barpos, offset,
						0x18, (void*)underCursor, (void*)scrollBar);
				}

				if (scrollBar->GetValuePtr().type == "float" &&
					mouse.curX >= absPos.x && mouse.curX < absPos.x + scrollBar->wndTemplate->tempScrollBar.l_width &&
					mouse.curY >= absPos.y && mouse.curY < absPos.y + scrollBar->wndTemplate->tempScrollBar.height)
				{
					mouse.SetHolder(0, -1,
						0x19, (void*)underCursor, (void*)scrollBar);
				}

				if (scrollBar->GetValuePtr().type == "float" &&
					mouse.curX >= absPos.x + scrollBar->width - scrollBar->wndTemplate->tempScrollBar.r_width && mouse.curX < absPos.x + scrollBar->width &&
					mouse.curY >= absPos.y && mouse.curY < absPos.y + scrollBar->wndTemplate->tempScrollBar.height)
				{
					mouse.SetHolder(0, 1,
						0x19, (void*)underCursor, (void*)scrollBar);
				}
			}
		}
	}

	if ((mouse.GetButtons() & MOUSE_RIGHT) && !(mouse.GetButtons() & MOUSE_OLD_RIGHT) && !(mouse.GetButtons() & MOUSE_LEFT)) {
		if (active != underCursor) {
			if (active->activeElement && !active->activeElement->alwaysActive) {
				active->activeElement->active = false;
				active->activeElement = NULL;
			}
			MoveOnTop(underCursor);
			active = underCursor;
		}

		if (underCursor)
			underCursor->ActivateFunction();

		CloseWindows(WND_MENU);
		if (!underCursor || underCursor->wndType != WND_COUNTER)
			CloseWindows(WND_COUNTER);

		if (holder.type == 0x00)
			mouse.SetLock(mouse.curX, mouse.curY);

		WindowElement* element = NULL;
		WindowElementMiniMap* minimap = NULL;
		if (underCursor) {
			RECT rect = {underCursor->posX, underCursor->posY, underCursor->posX + underCursor->width - 1, underCursor->posY + underCursor->height - 1};
			element = underCursor->GetWindowElementUnderCursor(rect, &underCursor->wndContainer, mouse.curX, mouse.curY);
			minimap = dynamic_cast<WindowElementMiniMap*>(element);
		}

		if (holder.type == 0x00 && element) {
			POINT absPos = element->GetAbsolutePosition(true);

			if (minimap) {
				float x = 0.0f;
				float y = 0.0f;
				minimap->GetOffsetPos(x, y);
				mouse.SetHolder(mouse.curX - (int)x, mouse.curY - (int)y, 0x60, (void*)underCursor, (void*)minimap);
			}
		}
	}

	if ((mouse.GetButtons() & MOUSE_LEFT) && (mouse.GetButtons() & MOUSE_OLD_LEFT) && !(mouse.GetButtons() & MOUSE_RIGHT) &&
		(mouse.lockX != 0 && mouse.lockY != 0) &&
		(abs(mouse.curX - mouse.lockX) > 3 || abs(mouse.curY - mouse.lockY) > 3))
	{
		Mouse xMouse;
		xMouse.curX = mouse.lockX;
		xMouse.curY = mouse.lockY;
		Window* underCursor = GetWindowUnderCursor(xMouse);

		WindowElement* element = NULL;
		WindowElementItemContainer* itemContainer = NULL;
		WindowElementPlayground* playground = NULL;
		WindowElementMiniMap* minimap = NULL;
		if (underCursor) {
			RECT rect = {underCursor->posX, underCursor->posY, underCursor->posX + underCursor->width - 1, underCursor->posY + underCursor->height - 1};
			element = underCursor->GetWindowElementUnderCursor(rect, &underCursor->wndContainer, mouse.lockX, mouse.lockY);
			itemContainer = dynamic_cast<WindowElementItemContainer*>(element);
			playground = dynamic_cast<WindowElementPlayground*>(element);
			minimap = dynamic_cast<WindowElementMiniMap*>(element);
		}

		if (holder.type == 0x00 && element) {
			POINT absPos = element->GetAbsolutePosition(true);

			if (itemContainer) {
				Container* container = itemContainer->GetContainer();
				if (container && container->GetIndex() != CONTAINER_TRADE) {
					Position* fromPos = new(M_PLACE) Position(0, 0, 0);

					fromPos->x = 0xFFFF;
					if (container->GetIndex() == CONTAINER_INVENTORY)
						fromPos->y = itemContainer->GetSlot() + 1;
					else {
						fromPos->y = (container->GetIndex() & 0x3F) | 0x40;
						fromPos->z = itemContainer->GetSlot();
					}

					Thing* moveThing = NULL;
					unsigned short moveItemId = 0;
					unsigned char moveStackPos = 0;

					Item* item = container->GetItem(itemContainer->GetSlot());
					if (item) {
						Item* newItem = new(M_PLACE) Item(item);
						moveThing = newItem;
						moveItemId = newItem->GetID();
					}

					mouse.SetHolder(moveItemId, moveStackPos,
						0x50, NULL, (void*)fromPos, (void*)moveThing);
				}
				else if (itemContainer->GetHotKey()) {
					HotKey* hk = itemContainer->GetHotKey();
					Position* fromPos = new(M_PLACE) Position(0xFFFF, 0, 0);

					HotKey* hotKey = new(M_PLACE) HotKey;
					hotKey->keyChar = hk->keyChar;
					hotKey->text = hk->text;
					hotKey->itemID = hk->itemID;
					hotKey->fluid = hk->fluid;
					hotKey->spellID = hk->spellID;
					hotKey->mode = hk->mode;
					hotKey->itemContainer = hk->itemContainer;

					unsigned short moveItemId = 0;
					unsigned char moveStackPos = 0;

					mouse.SetHolder(moveItemId, moveStackPos,
						0x53, NULL, (void*)fromPos, (void*)hotKey);
				}
			}

			if (playground) {
				POINT p = playground->GetTilePos(mouse.lockX - absPos.x, mouse.lockY - absPos.y);
				int x = p.x;
				int y = p.y;

				Position pos(0, 0, 0);

				LOCKCLASS lockClass1(Map::lockMap);
				LOCKCLASS lockClass2(Tile::lockTile);

				Map* map = game->GetMap();
				if (map) {
					pos = map->GetCorner();
					pos.x += x + 1;
					pos.y += y + 1;

					Tile* tile = map->GetTile(pos);
					if (tile) {
						Thing* thing = tile->GetMoveableThing();
						if (thing) {
							Item* item = dynamic_cast<Item*>(thing);
							Creature* creature = dynamic_cast<Creature*>(thing);

							unsigned char stackPos = tile->GetThingStackPos(thing);
							unsigned short itemId = 0;
							Thing* moveThing = NULL;
							if (item) {
								Item* newItem = new(M_PLACE) Item(item);
								moveThing = newItem;
								itemId = item->GetID();
							}
							else if (creature) {
								Creature* newCreature = new(M_PLACE) Creature(creature);
								moveThing = newCreature;
								itemId = 0x62;
							}

							Position* fromPos = new(M_PLACE) Position(pos.x, pos.y, pos.z);

							mouse.SetHolder(itemId, stackPos,
								0x50, NULL, (void*)fromPos, (void*)moveThing);
						}
					}
				}
			}

			if (minimap) {
				float zoom = minimap->GetZoom();
				float cx;
				float cy;
				minimap->GetOffsetPos(cx, cy);

				Map* map = game->GetMap();
				MiniMap* mmap = (map ? map->GetMiniMap() : NULL);
				if (mmap) {
					Position corner = map->GetCorner();
					int xs = (int)ceil(((float)(mouse.lockX - 5 - absPos.x) - minimap->width / 2 - cx) / (minimap->width * zoom / 256));
					int ys = (int)ceil(((float)(mouse.lockY - 5 - absPos.y) - minimap->height / 2 - cy) / (minimap->height *zoom / 256));
					int xk = (int)ceil(((float)(mouse.lockX + 5 - absPos.x) - minimap->width / 2 - cx) / (minimap->width * zoom / 256));
					int yk = (int)ceil(((float)(mouse.lockY + 5 - absPos.y) - minimap->height / 2 - cy) / (minimap->height * zoom / 256));

					for (int y = yk; y >= ys; y--)
					for (int x = xk; x >= xs; x--) {
						Position pos = Position(corner.x + 8 + x, corner.y + 6 + y, corner.z + mmap->GetOffsetLevel());
						Marker marker = mmap->GetMarker(pos);
						Waypoint waypoint = mmap->GetWaypoint(pos);

						if (marker.first != 0 && marker.second != "") {
							mouse.SetHolder(marker.first, 0,
								0x70, NULL, (void*)new(M_PLACE) Position(pos));
							break;
						}
						else if (waypoint.first != 0) {
							mouse.SetHolder(waypoint.first, 0,
								0x71, NULL, (void*)new(M_PLACE) Position(pos));
							break;
						}
					}
				}
			}
		}

		mouse.SetLock(0, 0);
	}

	if ((mouse.GetButtons() & MOUSE_RIGHT) && (mouse.GetButtons() & MOUSE_OLD_RIGHT) && !(mouse.GetButtons() & MOUSE_LEFT) &&
		(mouse.lockX != 0 && mouse.lockY != 0) &&
		(abs(mouse.curX - mouse.lockX) > 3 || abs(mouse.curY - mouse.lockY) > 3))
	{
		Mouse xMouse;
		xMouse.curX = mouse.lockX;
		xMouse.curY = mouse.lockY;
		Window* underCursor = GetWindowUnderCursor(xMouse);

		WindowElement* element = NULL;
		if (underCursor) {
			RECT rect = {underCursor->posX, underCursor->posY, underCursor->posX + underCursor->width - 1, underCursor->posY + underCursor->height - 1};
			element = underCursor->GetWindowElementUnderCursor(rect, &underCursor->wndContainer, mouse.lockX, mouse.lockY);
		}

		if (holder.type == 0x00 && element) {
		}

		mouse.SetLock(0, 0);
	}

	CheckHolder(mouse, keyboard, realTime, true);
	holder = mouse.GetHolder();

	if (!(mouse.GetButtons() & MOUSE_LEFT) && (mouse.GetButtons() & MOUSE_OLD_LEFT) && !(mouse.GetButtons() & MOUSE_RIGHT)) {
		CloseWindows(WND_MENU);

		WindowElement* element = NULL;
		WindowElementLink* link = NULL;
		WindowElementList* list = NULL;
		WindowElementTextarea* textarea = NULL;
		WindowElementButton* button = NULL;
		WindowElementItemContainer* itemContainer = NULL;
		WindowElementCheckBox* checkBox = NULL;
		WindowElementScrollBar* scrollBar = NULL;
		WindowElementPlayground* playground = NULL;
		WindowElementMiniMap* minimap = NULL;
		WindowElementBattle* battle = NULL;
		WindowElementColorMap* colormap = NULL;
		WindowElementSelect* select = NULL;
		if (underCursor) {
			RECT rect = {underCursor->posX, underCursor->posY, underCursor->posX + underCursor->width - 1, underCursor->posY + underCursor->height - 1};
			element = underCursor->GetWindowElementUnderCursor(rect, &underCursor->wndContainer, mouse.curX, mouse.curY);
			link = dynamic_cast<WindowElementLink*>(element);
			list = dynamic_cast<WindowElementList*>(element);
			textarea = dynamic_cast<WindowElementTextarea*>(element);
			button = dynamic_cast<WindowElementButton*>(element);
			itemContainer = dynamic_cast<WindowElementItemContainer*>(element);
			checkBox = dynamic_cast<WindowElementCheckBox*>(element);
			scrollBar = dynamic_cast<WindowElementScrollBar*>(element);
			playground = dynamic_cast<WindowElementPlayground*>(element);
			minimap = dynamic_cast<WindowElementMiniMap*>(element);
			battle = dynamic_cast<WindowElementBattle*>(element);
			colormap = dynamic_cast<WindowElementColorMap*>(element);
			select = dynamic_cast<WindowElementSelect*>(element);
		}

		if (holder.type != 0x00) {
			Signal signal;
			if (holder.type == 0x02) {
				std::list<Window*>* connected = (std::list<Window*>*)holder.variable;
				delete_debug(connected, M_PLACE);
			}
			else if (holder.type == 0x30) {
				WindowElementButton* hbutton = (WindowElementButton*)holder.holder;
				hbutton->pressed = false;

				if (button == hbutton && hbutton->func.IsExecutable())
					signal.Assign(button->func);
			}
			else if (holder.type == 0x50) {
				Position* fromPos = (Position*)holder.holder;
				Thing* moveThing = (Thing*)holder.variable;
				unsigned short moveItemId = holder.posX;
				unsigned char moveStackPos = holder.posY;
				Position toPos = Position(0, 0, 0);

				if (itemContainer) {
					Window* window = itemContainer->GetWindow();
					Container* container = itemContainer->GetContainer();
					if (container && container->GetIndex() == CONTAINER_FREE) {
						Item* item = dynamic_cast<Item*>(moveThing);

						if (item) {
							Item* newItem = new(M_PLACE) Item(item);
							if ((*newItem)() && (*newItem)()->stackable)
								newItem->SetCount(0);
							container->TransformItem(itemContainer->GetSlot(), newItem);
						}
					}
					else if (container && container->GetIndex() != CONTAINER_TRADE) {
						toPos.x = 0xFFFF;
						if (container->GetIndex() == CONTAINER_INVENTORY)
							toPos.y = itemContainer->GetSlot() + 1;
						else {
							toPos.y = (container->GetIndex() & 0x3F) | 0x40;
							toPos.z = itemContainer->GetSlot();
						}
					}
					else if (itemContainer->GetHotKey()) {
						toPos.x = 0xFFFF;
						toPos.y = 0;
						toPos.z = 0;

						HotKey* hkTo = itemContainer->GetHotKey();
						if (hkTo && !hkTo->disabledItemChange) {
							Item* item = dynamic_cast<Item*>(moveThing);
							if (item) {
								if (hkTo->itemID == 0) {
									hkTo->keyChar = 0;
									hkTo->text = "";
									hkTo->mode = 3;
								}
								hkTo->itemID = item->GetID();
								hkTo->fluid = ((*item)() && (*item)()->fluid ? item->GetCount() : 0);
								hkTo->spellID = 0;
								if (hkTo->itemContainer) {
									Item* newItem = new(M_PLACE) Item(item);
									if (hkTo->fluid)
										newItem->SetCount(hkTo->fluid);
									else
										newItem->SetCount(0);
									hkTo->itemContainer->SetItem(newItem);
								}
							}
						}
					}
				}

				if (playground) {
					POINT absPos = playground->GetAbsolutePosition(true);
					POINT p = playground->GetTilePos(mouse.curX - absPos.x, mouse.curY - absPos.y);
					int x = p.x;
					int y = p.y;

					Map* map = game->GetMap();
					if (map) {
						toPos = map->GetCorner();
						toPos.x += x + 1;
						toPos.y += y + 1;

						POINT zPos = map->GetZ();
						Tile* tile = map->GetTopTile(x + 1, y + 1, zPos.x, zPos.y);
						if (tile)
							toPos = tile->pos;
					}
				}

				if (textarea && textarea->enabled && textarea->editable) {
					Item* item = dynamic_cast<Item*>(moveThing);

					if (item)
						textarea->SetText(value2str(item->GetID()));
				}

				if (toPos != Position(0, 0, 0) && toPos != Position(0xFFFF, 0x0000, 0x00) && *fromPos != toPos) {
					Item* item = dynamic_cast<Item*>(moveThing);
					if (item) {
						unsigned char count = 0;
						if ((*item)() && (*item)()->stackable) {
							if (item->GetCount() <= 1)
								count = 1;
							else if (keyboard.key[VK_SHIFT])
								count = 1;
							else if (keyboard.key[VK_CONTROL])
								count = item->GetCount();
						}
						else if ((*item)() && (*item)()->fluid)
							count = 1;
						else
							count = 1;

						if (item && count == 0) {
							ThingData* moveData = new(M_PLACE) ThingData;
							moveData->fromPos = *fromPos;
							moveData->toPos = toPos;
							moveData->fromStackPos = moveStackPos;
							moveData->itemId = moveItemId;
							moveData->count = item->GetCount();

							OpenWindow(WND_COUNTER, game, (void*)moveData);
						}
						else
							game->PlayerMoveThing(*fromPos, moveItemId, moveStackPos, toPos, count);
					}
					else
						game->PlayerMoveThing(*fromPos, moveItemId, moveStackPos, toPos, 1);
				}

				mouse.SetHolder(0, 0, 0x00);
				actioned = true;

				delete_debug(moveThing, M_PLACE);
				delete_debug(fromPos, M_PLACE);
			}
			else if (holder.type == 0x51) {
				Position* fromPos = (Position*)holder.holder;
				Thing* useThing = (Thing*)holder.variable;
				unsigned short useItemId = holder.posX;
				unsigned char useStackPos = holder.posY;
				Position toPos = Position(0, 0, 0);

				Thing* useWithThing = NULL;
				unsigned short useWithItemId = 0;
				unsigned char useWithStackPos = 0;

				unsigned int creatureID = 0;

				if (itemContainer) {
					Container* container = itemContainer->GetContainer();
					if (container && container->GetIndex() != CONTAINER_TRADE) {
						toPos.x = 0xFFFF;
						if (container->GetIndex() == CONTAINER_INVENTORY)
							toPos.y = itemContainer->GetSlot() + 1;
						else {
							toPos.y = (container->GetIndex() & 0x3F) | 0x40;
							toPos.z = itemContainer->GetSlot();
						}

						Item* item = container->GetItem(itemContainer->GetSlot());
						if (item) {
							useWithThing = item;
							useWithItemId = item->GetID();
						}
					}
				}

				if (playground) {
					POINT pos = playground->GetAbsolutePosition(true);
					POINT p = playground->GetTilePos(mouse.curX - pos.x, mouse.curY - pos.y);
					int x = p.x;
					int y = p.y;

					Player* player = game->GetPlayer();
					Map* map = game->GetMap();
					if (map) {
						toPos = map->GetCorner();
						toPos.x += x + 1;
						toPos.y += y + 1;

						Tile* tile = map->GetTile(toPos);
						if (tile) {
							Thing* thing = NULL;
							Thing* thingItem = tile->GetUseableThing(true);
							Thing* thingCreature = tile->GetTopCreatureThing(true);

							Item* item = dynamic_cast<Item*>(thingItem);
							if (item && (*item)() && (*item)()->action)
								thing = thingItem;
							else if (thingCreature) {
								thing = thingCreature;
								Creature* creature = dynamic_cast<Creature*>(thing);
								creatureID = creature->GetID();
							}
							else if (thingItem)
								thing = thingItem;

							if (useThing) {
								useWithStackPos = tile->GetThingStackPos(thing);

								Item* item = dynamic_cast<Item*>(thing);
								if (item)
									useWithItemId = item->GetID();
							}
						}
					}
				}

				if (battle)
					creatureID = battle->GetCreatureID();

				if (creatureID)
					game->PlayerUseBattleThing(*fromPos, useItemId, useStackPos, creatureID);
				else
					game->PlayerUseWithThing(*fromPos, useItemId, useStackPos, toPos, useWithItemId, useWithStackPos);

				mouse.SetHolder(0, 0, 0x00);
				actioned = true;

				delete_debug(useThing, M_PLACE);
				delete_debug(fromPos, M_PLACE);
			}
			else if (holder.type == 0x52) {
				Position* fromPos = (Position*)holder.holder;
				Thing* tradeThing = (Thing*)holder.variable;
				unsigned short tradeItemId = holder.posX;
				unsigned char tradeStackPos = holder.posY;
				unsigned int creatureID = 0;

				if (playground) {
					POINT pos = playground->GetAbsolutePosition(true);
					POINT p = playground->GetTilePos(mouse.curX - pos.x, mouse.curY - pos.y);
					int x = p.x;
					int y = p.y;

					Map* map = game->GetMap();
					if (map) {
						Position toPos = map->GetCorner();
						toPos.x += x + 1;
						toPos.y += y + 1;

						Tile* tile = map->GetTile(toPos);
						if (tile) {
							Creature* creature = dynamic_cast<Creature*>(tile->GetTopCreatureThing(true));
							if (creature)
								creatureID = creature->GetID();
						}
					}
				}

				if (battle)
					creatureID = battle->GetCreatureID();

				game->PlayerRequestTrade(*fromPos, tradeItemId, tradeStackPos, creatureID);

				mouse.SetHolder(0, 0, 0x00);
				actioned = true;

				delete_debug(tradeThing, M_PLACE);
				delete_debug(fromPos, M_PLACE);
			}
			else if (holder.type == 0x53) {
				Position* fromPos = (Position*)holder.holder;
				HotKey* hotKey = (HotKey*)holder.variable;
				Position toPos = Position(0, 0, 0);

				if (itemContainer && itemContainer->GetHotKey()) {
					toPos.x = 0xFFFF;
					toPos.y = 0;
					toPos.z = 0;

					HotKey* hkFrom = (hotKey->itemContainer ? hotKey->itemContainer->GetHotKey() : hotKey);
					HotKey* hkTo = itemContainer->GetHotKey();

					HotKey hkTemp;
					hkTemp.keyChar = hkTo->keyChar;
					hkTemp.text = hkTo->text;
					hkTemp.itemID = hkTo->itemID;
					hkTemp.fluid = hkTo->fluid;
					hkTemp.spellID = hkTo->spellID;
					hkTemp.mode = hkTo->mode;

					if (hkTo->itemContainer) {
						hkTo->keyChar = hkFrom->keyChar;
						hkTo->text = hkFrom->text;
						hkTo->itemID = hkFrom->itemID;
						hkTo->fluid = hkFrom->fluid;
						hkTo->spellID = hkFrom->spellID;
						hkTo->mode = hkFrom->mode;

						if (hkTo->itemID != 0) {
							Item* newItem = new(M_PLACE) Item;
							newItem->SetID(hkTo->itemID);
							if (hkTo->fluid && (*newItem)() && (*newItem)()->fluid)
								newItem->SetCount(hkTo->fluid);
							else
								newItem->SetCount(0);
							hkTo->itemContainer->SetItem(newItem);
						}
						else
							hkTo->itemContainer->SetItem(NULL);
					}

					if (hkFrom->itemContainer) {
						hkFrom->keyChar = hkTemp.keyChar;
						hkFrom->text = hkTemp.text;
						hkFrom->itemID = hkTemp.itemID;
						hkFrom->fluid = hkTemp.fluid;
						hkFrom->spellID = hkTemp.spellID;
						hkFrom->mode = hkTemp.mode;

						if (hkFrom->itemID != 0) {
							Item* newItem = new(M_PLACE) Item;
							newItem->SetID(hkFrom->itemID);
							if (hkFrom->fluid && (*newItem)() && (*newItem)()->fluid)
								newItem->SetCount(hkFrom->fluid);
							else
								newItem->SetCount(0);
							hkFrom->itemContainer->SetItem(newItem);
						}
						else
							hkFrom->itemContainer->SetItem(NULL);
					}
				}

				mouse.SetHolder(0, 0, 0x00);
				actioned = true;

				delete_debug(hotKey, M_PLACE);
				delete_debug(fromPos, M_PLACE);
			}
			else if (holder.type == 0x70) {
				Position* pos = (Position*)holder.holder;

				if (minimap) {
					POINT absPos = element->GetAbsolutePosition(true);

					float zoom = minimap->GetZoom();
					float cx;
					float cy;
					minimap->GetOffsetPos(cx, cy);

					Map* map = game->GetMap();
					MiniMap* mmap = (map ? map->GetMiniMap() : NULL);
					if (mmap) {
						Position corner = map->GetCorner();
						int x = (int)ceil(((float)(mouse.curX - absPos.x) - minimap->width / 2 - cx) / (minimap->width * zoom / 256));
						int y = (int)ceil(((float)(mouse.curY - absPos.y) - minimap->height / 2 - cy) / (minimap->height *zoom / 256));

						Position newPos(corner.x + 8 + x, corner.y + 6 + y, corner.z + mmap->GetOffsetLevel());

						Marker marker = mmap->GetMarker(*pos);
						mmap->AddMarker(newPos, marker);
						mmap->RemoveMarker(*pos);
					}
				}

				delete_debug(pos, M_PLACE);
			}
			else if (holder.type == 0x71) {
				Position* pos = (Position*)holder.holder;

				if (minimap) {
					POINT absPos = element->GetAbsolutePosition(true);

					float zoom = minimap->GetZoom();
					float cx;
					float cy;
					minimap->GetOffsetPos(cx, cy);

					Map* map = game->GetMap();
					MiniMap* mmap = (map ? map->GetMiniMap() : NULL);
					if (mmap) {
						Position corner = map->GetCorner();
						int x = (int)ceil(((float)(mouse.curX - absPos.x) - minimap->width / 2 - cx) / (minimap->width * zoom / 256));
						int y = (int)ceil(((float)(mouse.curY - absPos.y) - minimap->height / 2 - cy) / (minimap->height *zoom / 256));

						Position newPos(corner.x + 8 + x, corner.y + 6 + y, corner.z + mmap->GetOffsetLevel());

						Waypoint waypoint = mmap->GetWaypoint(*pos);
						unsigned short waypointNum = mmap->GetWaypointNumber(*pos);
						mmap->AddWaypoint(newPos, waypoint, waypointNum);
						mmap->RemoveWaypoint(*pos);
					}
				}

				delete_debug(pos, M_PLACE);
			}
			else if (holder.type == 0x80) {
				Window* wndBot = (Window*)holder.window;
				WindowElementTextarea* ta_lootbagindex = (WindowElementTextarea*)holder.holder;

				if (IsOnList(wndBot) && ta_lootbagindex) {
					ContainerWindow* cntWindow = dynamic_cast<ContainerWindow*>(underCursor);
					if (cntWindow) {
						unsigned char index = cntWindow->GetIndex();
						ta_lootbagindex->SetText(value2str(index));
					}
					else
						ta_lootbagindex->SetText("");
				}
			}

			mouse.SetHolder(0, 0, 0x00);

			if (signal.IsExecutable())
				signal.Execute();
		}
		else if (holder.type == 0x00 && element) {
			POINT absPos = element->GetAbsolutePosition(true);

			if (link && link->func.IsExecutable())
				link->func.Execute();

			if (select && select->func.IsExecutable())
				select->func.Execute();

			if (itemContainer && itemContainer->func.IsExecutable())
				itemContainer->func.Execute();

			if (button && button->enabled && button->visible) {
				if (button->type == BUTTON_TAB) {
					if (button->GetCloseIcon() &&
						mouse.curX >= absPos.x + button->width - button->wndTemplate->tempTab.x.GetWidth() && mouse.curX < absPos.x + button->width &&
						mouse.curY >= absPos.y && mouse.curY < absPos.y + button->wndTemplate->tempTab.x.GetHeight())
					{
						WindowElementContainer* parent = button->parent;
						if (parent) {
							WindowElementTab* tab = dynamic_cast<WindowElementTab*>(parent->parent);
							if (tab) {
								game->onMakeAction(this, underCursor, ACTION_CLOSECHANNEL, (void*)button);
							}
						}
					}
				}
			}

			if (list) {
				std::list<MenuData> actionList;
				element->OpenMenu(mouse, keyboard, actionList);
				Window* wndMenu = NULL;
				if (actionList.size())
					wndMenu = OpenWindow(WND_MENU, game, &mouse, &keyboard, (void*)&actionList);

				if (wndMenu) {
					wndMenu->SetPosition(absPos.x, absPos.y + list->height);
					wndMenu->SetSize(list->width, wndMenu->height);
				}
			}

			if (checkBox) {
				bool checked = checkBox->GetState();
				checked = !checked;
				checkBox->SetState(checked);

				if (checkBox->func.IsExecutable())
					checkBox->func.Execute();
			}

			if (scrollBar) {
				if (scrollBar->GetValuePtr().type != "float" &&
					mouse.curX >= absPos.x && mouse.curX < absPos.x + scrollBar->wndTemplate->tempScrollBar.l_width &&
					mouse.curY >= absPos.y && mouse.curY < absPos.y + scrollBar->wndTemplate->tempScrollBar.height)
				{
					float newValue = scrollBar->GetValue() - 1.0f;
					if (newValue < scrollBar->GetMinValue()) newValue = scrollBar->GetMinValue();
					scrollBar->SetValue(newValue);

					if (scrollBar->func.IsExecutable())
						scrollBar->func.Execute();
				}

				if (scrollBar->GetValuePtr().type != "float" &&
					mouse.curX >= absPos.x + scrollBar->width - scrollBar->wndTemplate->tempScrollBar.r_width && mouse.curX < absPos.x + scrollBar->width &&
					mouse.curY >= absPos.y && mouse.curY < absPos.y + scrollBar->wndTemplate->tempScrollBar.height)
				{
					float newValue = scrollBar->GetValue() + 1.0f;
					if (newValue > scrollBar->GetMaxValue()) newValue = scrollBar->GetMaxValue();
					scrollBar->SetValue(newValue);

					if (scrollBar->func.IsExecutable())
						scrollBar->func.Execute();
				}
			}

			if (minimap && !keyboard.key[VK_CONTROL]) {
				float zoom = minimap->GetZoom();
				float cx;
				float cy;
				minimap->GetOffsetPos(cx, cy);

				Map* map = game->GetMap();
				if (map) {
					Position corner = map->GetCorner();
					int x = (int)ceil(((float)(mouse.curX - absPos.x) - minimap->width / 2 - cx) / (minimap->width * zoom / 256));
					int y = (int)ceil(((float)(mouse.curY - absPos.y) - minimap->height / 2 - cy) / (minimap->height * zoom / 256));

					Player* player = game->GetPlayer();

					Position toPos = Position(corner.x + 8 + x, corner.y + 6 + y, corner.z);

					game->PlayerStopWalk();
					player->SetTargetPos(toPos);
				}
			}
			else if (minimap && keyboard.key[VK_CONTROL]) {
				Map* map = game->GetMap();
				if (map) {
					std::list<MenuData> actionList;
					minimap->OpenMenu(mouse, keyboard, actionList);
					Window* wndMenu = NULL;
					if (actionList.size())
						wndMenu = OpenWindow(WND_MENU, game, &mouse, &keyboard, (void*)&actionList);
				}
			}

			if (colormap) {
				int x = (int)((float)(mouse.curX - absPos.x) / ((float)colormap->width / 19));
				int y = (int)((float)(mouse.curY - absPos.y) / ((float)colormap->height / 7));

				colormap->SetIndex(x + y * 19);

				if (colormap->func.IsExecutable())
					colormap->func.Execute();
			}
		}

		if (holder.type == 0x00 &&
			underCursor && underCursor->GetElements() & ELEMENT_HEADER &&
			underCursor->wndTemplate->GetElements() & ELEMENT_HEADER &&
			underCursor->GetElements() & ELEMENT_WNDICONS)
		{
			ContainerWindow* cntWindow = dynamic_cast<ContainerWindow*>(underCursor);
			Container* container = (cntWindow ? cntWindow->GetContainer() : NULL);

			int offset = 0;
			if (underCursor->closeAbility) {
				if (mouse.curX >= underCursor->posX + underCursor->width - underCursor->wndTemplate->tempWndIcons.x - offset - underCursor->wndTemplate->tempWndIcons.w && mouse.curX < underCursor->posX + underCursor->width - underCursor->wndTemplate->tempWndIcons.x - offset &&
					mouse.curY >= underCursor->posY + underCursor->wndTemplate->tempWndIcons.y && mouse.curY < underCursor->posY + underCursor->wndTemplate->tempWndIcons.y + underCursor->wndTemplate->tempWndIcons.h)
				{
					underCursor->SetAction(ACT_CLOSE);
				}
			}
			offset += underCursor->wndTemplate->tempWndIcons.w;

			if (underCursor->minimizeAbility) {
				if (mouse.curX >= underCursor->posX + underCursor->width - underCursor->wndTemplate->tempWndIcons.x - offset - underCursor->wndTemplate->tempWndIcons.w && mouse.curX < underCursor->posX + underCursor->width - underCursor->wndTemplate->tempWndIcons.x - offset &&
					mouse.curY >= underCursor->posY + underCursor->wndTemplate->tempWndIcons.y && mouse.curY < underCursor->posY + underCursor->wndTemplate->tempWndIcons.y + underCursor->wndTemplate->tempWndIcons.h)
				{
					underCursor->SetAction(ACT_MINIMIZE);
				}
			}
			offset += underCursor->wndTemplate->tempWndIcons.w;

			if (container && container->IsChild()) {
				if (mouse.curX >= underCursor->posX + underCursor->width - underCursor->wndTemplate->tempWndIcons.x - offset - underCursor->wndTemplate->tempWndIcons.w && mouse.curX < underCursor->posX + underCursor->width - underCursor->wndTemplate->tempWndIcons.x - offset &&
					mouse.curY >= underCursor->posY + underCursor->wndTemplate->tempWndIcons.y && mouse.curY < underCursor->posY + underCursor->wndTemplate->tempWndIcons.y + underCursor->wndTemplate->tempWndIcons.h)
				{
					game->PlayerContainerMoveUp(container->GetIndex());
				}
				offset += underCursor->wndTemplate->tempWndIcons.w;
			}
		}
	}

	if (!(mouse.GetButtons() & MOUSE_RIGHT) && (mouse.GetButtons() & MOUSE_OLD_RIGHT) && !(mouse.GetButtons() & MOUSE_LEFT)) {
		WindowElement* element = NULL;
		WindowElementContainer* container = NULL;
		WindowElementButton* button = NULL;
		WindowElementTextarea* textarea = NULL;
		WindowElementMemo* memo = NULL;
		WindowElementTableMemo* tmemo = NULL;
		WindowElementItemContainer* itemContainer = NULL;
		WindowElementCheckBox* checkBox = NULL;
		WindowElementScrollBar* scrollBar = NULL;
		WindowElementPlayground* playground = NULL;
		WindowElementBattle* battle = NULL;
		if (underCursor) {
			RECT rect = {underCursor->posX, underCursor->posY, underCursor->posX + underCursor->width - 1, underCursor->posY + underCursor->height - 1};
			element = underCursor->GetWindowElementUnderCursor(rect, &underCursor->wndContainer, mouse.curX, mouse.curY);
			itemContainer = dynamic_cast<WindowElementItemContainer*>(element);
			playground = dynamic_cast<WindowElementPlayground*>(element);
			battle = dynamic_cast<WindowElementBattle*>(element);
		}

		if (holder.type != 0x00) {
			mouse.SetHolder(0, 0, 0x00);
		}
		else if (holder.type == 0x00 && element) {
			POINT absPos = element->GetAbsolutePosition(true);

			if (!playground && !itemContainer && !battle) {
				std::list<MenuData> actionList;
				element->OpenMenu(mouse, keyboard, actionList);
				if (actionList.size())
					OpenWindow(WND_MENU, game, &mouse, &keyboard, (void*)&actionList);
			}
		}
	}

	if (!(mouse.GetButtons() & MOUSE_LEFT_DBL) && (mouse.GetButtons() & MOUSE_OLD_LEFT_DBL) && !(mouse.GetButtons() & MOUSE_RIGHT)) {
		WindowElement* element = NULL;
		WindowElementMemo* memo = NULL;
		WindowElementTableMemo* tmemo = NULL;
		WindowElementVIP* vip = NULL;
		WindowElementTextarea* textarea = NULL;
		if (underCursor) {
			RECT rect = {underCursor->posX, underCursor->posY, underCursor->posX + underCursor->width - 1, underCursor->posY + underCursor->height - 1};
			element = underCursor->GetWindowElementUnderCursor(rect, &underCursor->wndContainer, mouse.curX, mouse.curY);
			memo = dynamic_cast<WindowElementMemo*>(element);
			tmemo = dynamic_cast<WindowElementTableMemo*>(element);
			vip = dynamic_cast<WindowElementVIP*>(element);
			textarea = dynamic_cast<WindowElementTextarea*>(element);
		}

		if (holder.type == 0x01 &&
			underCursor && underCursor->GetElements() & ELEMENT_HEADER && underCursor->wndTemplate->GetElements() & ELEMENT_HEADER &&
			mouse.curX >= underCursor->posX && mouse.curX < underCursor->posX + underCursor->width &&
			mouse.curY >= underCursor->posY && mouse.curY < underCursor->posY + underCursor->wndTemplate->tempHeader.c_height &&
			!(mouse.curX >= underCursor->posX + underCursor->width - underCursor->wndTemplate->tempWndIcons.x - underCursor->wndTemplate->tempWndIcons.w && mouse.curX < underCursor->posX + underCursor->width - underCursor->wndTemplate->tempWndIcons.x &&
			mouse.curY >= underCursor->posY + underCursor->wndTemplate->tempWndIcons.y && mouse.curY < underCursor->posY + underCursor->wndTemplate->tempWndIcons.y + underCursor->wndTemplate->tempWndIcons.h))
		{
			underCursor->SetAction(ACT_MINIMIZE);
		}

		if (holder.type == 0x02 &&
			underCursor && underCursor->wndTemplate->GetElements() & ELEMENT_BORDER &&
			mouse.curX >= underCursor->posX + underCursor->width - underCursor->wndTemplate->tempBorder.br_width && mouse.curX < underCursor->posX + underCursor->width + underCursor->wndTemplate->tempBorder.br_x &&
			mouse.curY >= underCursor->posY + underCursor->height - underCursor->wndTemplate->tempBorder.br_height && mouse.curY < underCursor->posY + underCursor->height + underCursor->wndTemplate->tempBorder.br_y)
		{
			std::list<Window*> connectedList = GetConnectedWindows(underCursor);
			int oldWidth = underCursor->width;
			int oldHeight = underCursor->height;
			underCursor->AdjustSize();
			MoveConnectedWindows(underCursor, false, true, false, true, underCursor->width - oldWidth, underCursor->height - oldHeight, &connectedList);
		}
		else if (holder.type == 0x20) {
			POINT absPos = element->GetAbsolutePosition(true);

			if (textarea) {
				int scrollW = (textarea->multiline ? textarea->wndTemplate->tempScroll.w : 0);
				if (mouse.curX >= absPos.x + textarea->wndTemplate->tempTextarea.l_width && mouse.curX < absPos.x + textarea->width - textarea->wndTemplate->tempTextarea.r_width - scrollW &&
					mouse.curY >= absPos.y + textarea->wndTemplate->tempTextarea.t_height && mouse.curY < absPos.y + textarea->height - textarea->wndTemplate->tempTextarea.b_height)
				{
					POINT pos;
					pos.x = (mouse.curX + (int)textarea->scrollH - (absPos.x + textarea->wndTemplate->tempTextarea.l_width)) / textarea->fontSize;
					pos.y = (mouse.curY + (int)textarea->scrollV - (absPos.y + textarea->wndTemplate->tempTextarea.t_height)) / textarea->fontSize;
					float pix = mouse.curX + (int)textarea->scrollH - (absPos.x + textarea->wndTemplate->tempTextarea.l_width);

					if (pos.x >= 0 && pos.y >= 0) {
						textarea->SetCursor(pos, pix);
						textarea->GetCursor(pos, pix);
						textarea->curBegin = pos;
						textarea->curEnd = pos;
						textarea->SelectWordAtCursor(pix);
					}
				}
			}
		}

		if (holder.type == 0x00 && element) {
			POINT absPos = element->GetAbsolutePosition(true);

			if (vip) {
				VIPList* viplist = vip->GetVIPList();
				std::string creatureName = viplist->GetCreatureName(vip->GetCreatureID());

				game->PlayerOpenChannel(CHANNEL_PRIVATE, creatureName);
			}
			else if (memo) {
				int mouseOption = (mouse.curY + (int)memo->scrollV - (absPos.y + memo->wndTemplate->tempMemo.t_height)) / memo->fontSize;
				int option = memo->GetOption();

			    if (option == mouseOption && memo->dblFunc.IsExecutable())
                    memo->dblFunc.Execute();
				else if (underCursor->func.IsExecutable())
					underCursor->SetAction(ACT_EXECUTE);
			}
			else if (tmemo) {
				int mouseOption = (mouse.curY + (int)tmemo->scrollV - (absPos.y + tmemo->wndTemplate->tempMemo.t_height) - tmemo->rowHeight) / tmemo->rowHeight;
				int option = tmemo->GetOption();

			    if (option == mouseOption && tmemo->dblFunc.IsExecutable())
                    tmemo->dblFunc.Execute();
				else if (underCursor->func.IsExecutable())
					underCursor->SetAction(ACT_EXECUTE);
			}
		}
	}

	if (!(mouse.GetButtons() & MOUSE_RIGHT_DBL) && (mouse.GetButtons() & MOUSE_OLD_RIGHT_DBL) && !(mouse.GetButtons() & MOUSE_LEFT)) {
		WindowElement* element = NULL;
		WindowElementMiniMap* minimap = NULL;
		if (underCursor) {
			RECT rect = {underCursor->posX, underCursor->posY, underCursor->posX + underCursor->width - 1, underCursor->posY + underCursor->height - 1};
			element = underCursor->GetWindowElementUnderCursor(rect, &underCursor->wndContainer, mouse.curX, mouse.curY);
			minimap = dynamic_cast<WindowElementMiniMap*>(element);
		}

		if (holder.type == 0x60 && element) {
			POINT absPos = element->GetAbsolutePosition(true);

			if (minimap) {
				float x = 0.0f;
				float y = 0.0f;
				minimap->SetOffsetPos(x, y);
			}
		}
	}

	if (mouse.wheel != 0) {
		CloseWindows(WND_MENU);

		WindowElement* element = NULL;
		WindowElementContainer* container = NULL;
		WindowElementTextarea* textarea = NULL;
		WindowElementMemo* memo = NULL;
		WindowElementTableMemo* tmemo = NULL;
		WindowElementScrollBar* scrollBar = NULL;
		WindowElementMiniMap* minimap = NULL;
		WindowElementBattle* battle = NULL;
		WindowElementVIP* vip = NULL;
		if (underCursor) {
			RECT rect = {underCursor->posX, underCursor->posY, underCursor->posX + underCursor->width - 1, underCursor->posY + underCursor->height - 1};
			element = underCursor->GetWindowElementUnderCursor(rect, &underCursor->wndContainer, mouse.curX, mouse.curY);
			container = dynamic_cast<WindowElementContainer*>(element);
			textarea = dynamic_cast<WindowElementTextarea*>(element);
			memo = dynamic_cast<WindowElementMemo*>(element);
			tmemo = dynamic_cast<WindowElementTableMemo*>(element);
			scrollBar = dynamic_cast<WindowElementScrollBar*>(element);
			minimap = dynamic_cast<WindowElementMiniMap*>(element);
			battle = dynamic_cast<WindowElementBattle*>(element);
			vip = dynamic_cast<WindowElementVIP*>(element);

			if (battle || vip)
				container = element->parent;

			if (!container && !textarea && !memo && !tmemo && !scrollBar && !minimap && element)
				container = element->parent;
		}

		if (holder.type == 0x00 && element) {
			if (container) {
				POINT offset = container->GetScrollOffset();
				if (offset.y) {
					container->scrollV -= (mouse.wheel > 0 ? 20.0f : -20.0f);
					if (container->scrollV < 0.0f) container->scrollV = 0.0f;
					else if (container->scrollV > (float)offset.y) container->scrollV = (float)offset.y;
				}
			}
			else if (textarea) {
				int offset = textarea->GetScrollOffset();
				if (offset) {
					textarea->scrollV -= (mouse.wheel > 0 ? 20.0f : -20.0f);
					if (textarea->scrollV < 0.0f) textarea->scrollV = 0.0f;
					else if (textarea->scrollV > (float)offset) textarea->scrollV = (float)offset;
				}
			}
			else if (memo) {
				int offset = memo->GetScrollOffset();
				if (offset) {
					memo->scrollV -= (mouse.wheel > 0 ? 20.0f : -20.0f);
					if (memo->scrollV < 0.0f) memo->scrollV = 0.0f;
					else if (memo->scrollV > (float)offset) memo->scrollV = (float)offset;
				}
			}
			else if (tmemo) {
				int offset = tmemo->GetScrollOffset();
				if (offset) {
					tmemo->scrollV -= (mouse.wheel > 0 ? 20.0f : -20.0f);
					if (tmemo->scrollV < 0.0f) tmemo->scrollV = 0.0f;
					else if (tmemo->scrollV > (float)offset) tmemo->scrollV = (float)offset;
				}
			}
			else if (scrollBar) {
				float step = (scrollBar->GetMaxValue() - scrollBar->GetMinValue()) / 20;
				float newValue = scrollBar->GetValue() + (scrollBar->GetValuePtr().type == "float" ? (mouse.wheel > 0 ? step : -step) : (mouse.wheel > 0 ? 1.0f : -1.0f));
				scrollBar->SetValue(newValue);
			}
			else if (minimap) {
				float zoom = (mouse.wheel > 0 ? 1.0f / 0.9f : 0.9f);

				float z = minimap->GetZoom();
				z *= zoom;
				if (z < 1.0f)
					z = 1.0f;
				minimap->SetZoom(z);

				float x;
				float y;
				minimap->GetOffsetPos(x, y);
				x *= zoom;
				y *= zoom;
				minimap->SetOffsetPos(x, y);
			}
		}

		mouse.wheel = 0;
	}

	InputActions inpActions;
	CheckInput(mouse, keyboard, inpActions);

	if (holder.type == 0xFF && !(mouse.GetButtons() & MOUSE_LEFT) && !(mouse.GetButtons() & MOUSE_RIGHT)) {
		if (Game::options.classicControls) {
			inpActions.actWalk = false;
			inpActions.actLook = true;
			inpActions.actOpenMenu = false;
		}

		mouse.SetHolder(0, 0, 0x00);
	}

	if (!actioned && (inpActions.actWalk || inpActions.actLook || inpActions.actUse || inpActions.actAttack || inpActions.actOpenMenu)) {
		holder = mouse.GetHolder();

		WindowElement* element = NULL;
		WindowElementItemContainer* itemContainer = NULL;
		WindowElementPlayground* playground = NULL;
		WindowElementBattle* battle = NULL;
		if (underCursor) {
			RECT rect = {underCursor->posX, underCursor->posY, underCursor->posX + underCursor->width - 1, underCursor->posY + underCursor->height - 1};
			element = underCursor->GetWindowElementUnderCursor(rect, &underCursor->wndContainer, mouse.curX, mouse.curY);
			itemContainer = dynamic_cast<WindowElementItemContainer*>(element);
			playground = dynamic_cast<WindowElementPlayground*>(element);
			battle = dynamic_cast<WindowElementBattle*>(element);
		}

		if (holder.type == 0x00 && element) {
			POINT absPos = element->GetAbsolutePosition(true);

			if (itemContainer) {
				if (inpActions.actLook) {
					Window* window = itemContainer->GetWindow();
					Container* container = itemContainer->GetContainer();
					if (container && container->GetIndex() != CONTAINER_TRADE) {
						Position pos(0, 0, 0);

						pos.x = 0xFFFF;
						if (container->GetIndex() == CONTAINER_INVENTORY)
							pos.y = itemContainer->GetSlot() + 1;
						else {
							pos.y = (container->GetIndex() & 0x3F) | 0x40;
							pos.z = itemContainer->GetSlot();
						}

						unsigned short itemId = 0;
						unsigned char stackPos = 0;

						Item* item = container->GetItem(itemContainer->GetSlot());
						if (item)
							itemId = item->GetID();

						game->PlayerLookAt(pos, itemId, stackPos);
					}
					else if (container) {
						unsigned char counter = container->GetID();
						unsigned char index = itemContainer->GetSlot();

						game->PlayerLookInTrade(counter, index);
					}
					else if (window && window->GetWindowType() == WND_SHOP) {
						Item* item = itemContainer->GetItem();
						if (item) {
							unsigned short itemID = item->GetID();
							unsigned char type = item->GetCount();

							game->PlayerLookInShop(itemID, type);
						}
					}

					if (itemContainer && itemContainer->funcLook.IsExecutable())
						itemContainer->funcLook.Execute();
				}
				else if (inpActions.actUse) {
					Container* container = itemContainer->GetContainer();
					if (container && container->GetIndex() != CONTAINER_TRADE) {
						Position* fromPos = new(M_PLACE) Position(0, 0, 0);

						fromPos->x = 0xFFFF;
						if (container->GetIndex() == CONTAINER_INVENTORY)
							fromPos->y = itemContainer->GetSlot() + 1;
						else {
							fromPos->y = (container->GetIndex() & 0x3F) | 0x40;
							fromPos->z = itemContainer->GetSlot();
						}

						Thing* useThing = NULL;
						unsigned short useItemId = 0;
						unsigned char useStackPos = 0;
						unsigned char index = (!keyboard.key[VK_SHIFT] && container->GetIndex() != CONTAINER_INVENTORY ? container->GetIndex() : Container::GetFreeIndex());
						bool useWith = false;

						Item* item = container->GetItem(itemContainer->GetSlot());
						if (item) {
							Item* newItem = new(M_PLACE) Item(item);
							useThing = newItem;
							useItemId = newItem->GetID();

							if ((*item)() && (*item)()->useable)
								useWith = true;
						}

						if (useWith) {
							mouse.SetHolder(useItemId, useStackPos,
								0x51, NULL, (void*)fromPos, (void*)useThing);
						}
						else
							game->PlayerUseThing(*fromPos, useItemId, useStackPos, index);
					}
					else if (itemContainer->GetHotKey()) {
						HotKey* hk = itemContainer->GetHotKey();
						if (hk->itemContainer)
							game->ExecuteHotKey(itemContainer->GetSlot());
					}

					if (itemContainer && itemContainer->funcUse.IsExecutable())
						itemContainer->funcUse.Execute();
				}
			}
			else if (playground) {
				if (inpActions.actWalk) {
					POINT p = playground->GetTilePos(mouse.curX - absPos.x, mouse.curY - absPos.y);
					int x = p.x;
					int y = p.y;

					Map* map = game->GetMap();
					if (map) {
						Player* player = game->GetPlayer();

						Position toPos = map->GetCorner();
						toPos.x += x + 1;
						toPos.y += y + 1;

						game->PlayerStopWalk();
						player->SetTargetPos(toPos);
					}
				}
				else if (inpActions.actLook) {
					POINT p = playground->GetTilePos(mouse.curX - absPos.x, mouse.curY - absPos.y);
					int x = p.x;
					int y = p.y;

					Position pos(0, 0, 0);

					LOCKCLASS lockClass1(Map::lockMap);
					LOCKCLASS lockClass2(Tile::lockTile);

					Map* map = game->GetMap();
					if (map) {
						POINT zPos = map->GetZ();
						Tile* tile = map->GetTopTile(x + 1, y + 1, zPos.x, zPos.y);
						if (tile) {
							pos = tile->pos;

							Thing* thing = tile->GetTopDownThing();
							if (thing) {
								Item* item = dynamic_cast<Item*>(thing);
								Creature* creature = dynamic_cast<Creature*>(thing);

								unsigned char stackPos = tile->GetThingStackPos(thing);
								unsigned short itemId = 0;
								if (item)
									itemId = item->GetID();
								else if (creature)
									itemId = 0x62;

								game->PlayerLookAt(pos, itemId, stackPos);
							}
						}
					}
				}
				else if (inpActions.actUse && inpActions.actAttack) {
					POINT p = playground->GetTilePos(mouse.curX - absPos.x, mouse.curY - absPos.y);
					int x = p.x;
					int y = p.y;

					Position* fromPos = new(M_PLACE) Position(0, 0, 0);

					Thing* useThing = NULL;
					unsigned short useItemId = 0;
					unsigned char useStackPos = 0;
					unsigned char index = Container::GetFreeIndex();
					bool useWith = false;

					Player* player = game->GetPlayer();
					Map* map = game->GetMap();
					if (map && player) {
						*fromPos = map->GetCorner();
						fromPos->x += x + 1;
						fromPos->y += y + 1;

						Tile* tile = map->GetTile(*fromPos);
						if (tile) {
							Thing* thing = NULL;
							Thing* thingItem = tile->GetUseableThing();
							Thing* thingCreature = tile->GetTopCreatureThing(true);

							if (keyboard.key[VK_MENU])
								thing = thingItem;
							else if (thingCreature && dynamic_cast<Creature*>(thingCreature) != player->GetCreature())
								thing = thingCreature;
							else
								thing = thingItem;

							if (thing) {
								useStackPos = tile->GetThingStackPos(thing);

								Item* item = dynamic_cast<Item*>(thing);
								Creature* creature = dynamic_cast<Creature*>(thing);
								if (item) {
									Item* newItem = new(M_PLACE) Item(item);
									useThing = newItem;
									useItemId = newItem->GetID();

									if ((*item)() && (*item)()->useable)
										useWith = true;

									if (useWith) {
										mouse.SetHolder(useItemId, useStackPos,
											0x51, NULL, (void*)fromPos, (void*)useThing);
									}
									else
										game->PlayerUseThing(*fromPos, useItemId, useStackPos, index);
								}
								else if (creature)
									game->PlayerAttack(creature->GetID());
							}
						}
					}
				}
				else if (inpActions.actUse) {
					POINT p = playground->GetTilePos(mouse.curX - absPos.x, mouse.curY - absPos.y);
					int x = p.x;
					int y = p.y;

					Position* fromPos = new(M_PLACE) Position(0, 0, 0);

					Thing* useThing = NULL;
					unsigned short useItemId = 0;
					unsigned char useStackPos = 0;
					unsigned char index = Container::GetFreeIndex();
					bool useWith = false;

					Player* player = game->GetPlayer();
					Map* map = game->GetMap();
					if (map && player) {
						*fromPos = map->GetCorner();
						fromPos->x += x + 1;
						fromPos->y += y + 1;

						Tile* tile = map->GetTile(*fromPos);
						if (tile) {
							Thing* thing = tile->GetUseableThing();

							if (thing) {
								useStackPos = tile->GetThingStackPos(thing);

								Item* item = dynamic_cast<Item*>(thing);
								if (item) {
									Item* newItem = new(M_PLACE) Item(item);
									useThing = newItem;
									useItemId = newItem->GetID();

									if ((*item)() && (*item)()->useable)
										useWith = true;

									if (useWith) {
										mouse.SetHolder(useItemId, useStackPos,
											0x51, NULL, (void*)fromPos, (void*)useThing);
									}
									else
										game->PlayerUseThing(*fromPos, useItemId, useStackPos, index);
								}
							}
						}
					}
				}
				else if (inpActions.actAttack) {
					POINT p = playground->GetTilePos(mouse.curX - absPos.x, mouse.curY - absPos.y);
					int x = p.x;
					int y = p.y;

					Position* fromPos = new(M_PLACE) Position(0, 0, 0);

					Thing* useThing = NULL;
					unsigned short useItemId = 0;
					unsigned char useStackPos = 0;
					unsigned char index = Container::GetFreeIndex();
					bool useWith = false;

					Player* player = game->GetPlayer();
					Map* map = game->GetMap();
					if (map && player) {
						*fromPos = map->GetCorner();
						fromPos->x += x + 1;
						fromPos->y += y + 1;

						Tile* tile = map->GetTile(*fromPos);
						if (tile) {
							Thing* thing = tile->GetTopCreatureThing(true);

							if (thing) {
								useStackPos = tile->GetThingStackPos(thing);

								Creature* creature = dynamic_cast<Creature*>(thing);
								if (creature)
									game->PlayerAttack(creature->GetID());
							}
						}
					}
				}
			}
			else if (battle) {
				if (inpActions.actLook) {
					unsigned int creatureID = battle->GetCreatureID();
					Creature* creature = Creature::GetFromKnown(creatureID);

					if (creature)
						game->PlayerLookAt(creature->pos, 0, 0);
				}
				else if (inpActions.actAttack) {
					unsigned int creatureID = battle->GetCreatureID();
					game->PlayerAttack(creatureID);
				}
			}

			if (inpActions.actOpenMenu) {
				std::list<MenuData> actionList;
				element->OpenMenu(mouse, keyboard, actionList);
				if (actionList.size())
					OpenWindow(WND_MENU, game, &mouse, &keyboard, (void*)&actionList);
			}
		}
	}

	if (underCursor) {
		RECT rect = {underCursor->posX, underCursor->posY, underCursor->posX + underCursor->width - 1, underCursor->posY + underCursor->height - 1};
		WindowElement* element = underCursor->GetWindowElementUnderCursor(rect, &underCursor->wndContainer, mouse.curX, mouse.curY);

		unsigned int creatureID = 0;
		WindowElementItemContainer* craftboxIC = NULL;
		mouse.comment = (element && holder.type == 0 ? element->GetComment() : std::string(""));

		if (underCursor->wndType == WND_MINIMAP) {
			WindowElementMiniMap* minimap = dynamic_cast<WindowElementMiniMap*>(element);

			if (minimap) {
				POINT absPos = element->GetAbsolutePosition(true);

				float zoom = minimap->GetZoom();
				float cx;
				float cy;
				minimap->GetOffsetPos(cx, cy);

				Map* map = game->GetMap();
				MiniMap* mmap = (map ? map->GetMiniMap() : NULL);
				if (mmap) {
					Position corner = map->GetCorner();
					int xs = (int)ceil(((float)(mouse.curX - 5 - absPos.x) - minimap->width / 2 - cx) / (minimap->width * zoom / 256));
					int ys = (int)ceil(((float)(mouse.curY - 5 - absPos.y) - minimap->height / 2 - cy) / (minimap->height *zoom / 256));
					int xk = (int)ceil(((float)(mouse.curX + 5 - absPos.x) - minimap->width / 2 - cx) / (minimap->width * zoom / 256));
					int yk = (int)ceil(((float)(mouse.curY + 5 - absPos.y) - minimap->height / 2 - cy) / (minimap->height * zoom / 256));

					for (int y = yk; y >= ys; y--)
					for (int x = xk; x >= xs; x--) {
						Position pos = Position(corner.x + 8 + x, corner.y + 6 + y, corner.z + mmap->GetOffsetLevel());
						Marker marker = mmap->GetMarker(pos);
						Waypoint waypoint = mmap->GetWaypoint(pos);
						unsigned short waypointNum = mmap->GetWaypointNumber(pos);

						if (marker.first != 0 && marker.second != "") {
							mouse.comment = marker.second;
							break;
						}
						else if (waypoint.first != 0) {
							mouse.comment = std::string("WP_") + value2str(waypointNum + 1);
							if (waypoint.second != "")
								mouse.comment += " (" + waypoint.second + ")";
							break;
						}
					}
				}
			}
		}
		else if (underCursor->wndType == WND_BATTLE) {
			WindowElementBattle* battle = dynamic_cast<WindowElementBattle*>(element);

			if (battle)
				creatureID = battle->GetCreatureID();
		}
		else if (underCursor->wndType == WND_CRAFTBOX) {
			WindowElementItemContainer* itemContainer = dynamic_cast<WindowElementItemContainer*>(element);

			if (itemContainer) {
				CraftBox* craftbox = game->GetCraftBox();

				WindowElement* parent = itemContainer->GetParent();
				WindowElementContainer* container = dynamic_cast<WindowElementContainer*>(parent);
				if (craftbox && container && craftbox->GetContainer() && container != craftbox->GetContainer())
					craftboxIC = itemContainer;
			}
		}
		else {
			WindowElementCooldown* cooldown = dynamic_cast<WindowElementCooldown*>(element);
			WindowElementItemContainer* itemContainer = dynamic_cast<WindowElementItemContainer*>(element);

			if (cooldown) {
				unsigned int remain = cooldown->GetTimeRemain();
				if (remain)
					mouse.comment += (mouse.comment != "" ? "\n" : "") + Text::GetText("COMMENT_1", Game::options.language) + " " + value2str(remain / 1000) + "s";
			}
			else if (itemContainer) {
				HotKey* hk = itemContainer->GetHotKey();
				if (hk) {
					std::string cmt;
					if (hk->keyChar != 0) {
						if (hk->keyChar & 0x8000) cmt += "SHIFT + ";
						if (hk->keyChar & 0x4000) cmt += "CTRL + ";

						if ((hk->keyChar & 0x3FFF) > 255)
							cmt += "F" + value2str((hk->keyChar & 0x3FFF) - 255 - 111);
						else
							cmt += (char)(hk->keyChar & 0x3FFF);
					}

					if (hk->text.length() > 0) {
						if (cmt.length() > 0) cmt += " ";
						cmt += "[" + hk->text + "]";
					}

					if (itemContainer->GetComment() != "")
						cmt += "\n" + itemContainer->GetComment();

					mouse.comment = cmt;
				}
			}
		}

		Player::SetSelectID(creatureID);

		CraftBox* craftbox = game->GetCraftBox();
		if (craftbox)
			craftbox->UpdateContainer(craftboxIC);
	}
	else {
		mouse.comment = "";

		if (Player::GetSelectID() != 0x00)
			Player::SetSelectID(0x00);

		CraftBox* craftbox = game->GetCraftBox();
		if (craftbox && craftbox->GetContainer() && craftbox->GetContainer()->GetElementsSize())
			craftbox->UpdateContainer(NULL);
	}

	Window* wclose = FindWindow(WND_CLOSE);
	Window* woptions = FindWindow(WND_OPTIONS);
	bool openMenu = keyboard.key[VK_SHIFT] && keyboard.key[VK_CONTROL] && keyboard.key[VK_MENU];
	bool oldOpenMenu = keyboard.oldKey[VK_SHIFT] && keyboard.oldKey[VK_CONTROL] && keyboard.oldKey[VK_MENU];
	if (!wclose && woptions && openMenu && !oldOpenMenu) {
		if (!woptions->GetMinimizeAbility()) {
			woptions->SetMinimizeAbility(true);
			MoveOnTop(woptions, true);
		}
		else {
			woptions->SetMinimizeAbility(false);
			MoveDown(woptions, true);
			CloseWindows(WND_MENU);
		}
	}

	std::list<Window*> toDelete;

	std::list<Window*>::iterator it = wndList.begin();
	for (it; it != wndList.end(); it++) {
		Window* wnd = *it;
		if (wnd) {
			for (std::list<unsigned char>::iterator ait = wnd->actions.begin(); ait != wnd->actions.end(); ait++) {
				unsigned char action = *ait;
				if (action == ACT_CLOSE) {
					HOLDER holder = mouse.GetHolder();
					if (holder.window == (void*)wnd)
						mouse.SetHolder(0, 0, 0, NULL, NULL);

					toDelete.push_back(wnd);
				}
				else if (action == ACT_MINIMIZE) {
					if (Game::options.fixedPositions && IsFixedPosWindow((WINDOW_TYPE)wnd->wndType)) {
						int offset = (wnd->minimized ? wnd->minimized - wnd->height : wnd->wndTemplate->tempHeader.c_height - wnd->height);
						FitFixedWindows(wnd, offset);
					}

					if (!wnd->minimized) {
						wnd->minimized = wnd->height;
						wnd->height = wnd->wndTemplate->tempHeader.c_height;
					}
					else {
						wnd->height = wnd->minimized;
						wnd->minimized = 0;
					}

					wnd->MinimizeFunction();
				}
				else if (action == ACT_EXECUTE) {
					wnd->ExecuteFunction();
				}
			}

			wnd->actions.clear();
		}
	}

	it = toDelete.begin();
	for (it; it != toDelete.end(); it++) {
		Window* wnd = *it;
		KillWindow(wnd);
	}
}

void Windows::CheckAnimations(Mouse& mouse, Keyboard& keyboard, RealTime& realTime) {
	LOCKCLASS lockClass(lockWindows);

	std::list<Window*>::iterator it = wndList.begin();
	for (it; it != wndList.end(); it++) {
		Window* wnd = *it;
		if (wnd->wndType == WND_OPTIONS) {
			if (wnd->GetMinimizeAbility() && wnd->posY < 0) {
				wnd->posY += (int)ceil((float)realTime.getPeriod() / 10);
				if (wnd->posY > 0) wnd->posY = 0;
			}
			else if (!wnd->GetMinimizeAbility() && wnd->posY > -wnd->height) {
				wnd->posY -= (int)ceil((float)realTime.getPeriod() / 10);
				if (wnd->posY < -wnd->height) wnd->posY = -wnd->height;
			}
		}
	}
}

void Windows::Print() {
	LOCKCLASS lockClass1(Game::lockGame);
	LOCKCLASS lockClass2(lockWindows);

	std::list<Window*>::iterator it = wndList.begin();
	while(it != wndList.end()) {
		Window* wnd = *it++;
		if (wnd)
			wnd->Print(*gfx, it == wndList.end());
	}
}


//Lua functions

int Windows::LuaGetWindowTemplate(lua_State* L) {
	std::string templateName = LuaScript::PopString(L);

	Windows* wnds = Windows::wnds;
	if (wnds) {
		WindowTemplate* wndTemplate = wnds->GetWindowTemplate(Game::options.templatesGroup, templateName);

		LuaScript::PushNumber(L, (unsigned long)wndTemplate);
		return 1;
	}

	lua_pushnil(L);
	return 1;
}

int Windows::LuaGetWindowTemplateFont(lua_State* L) {
	WindowTemplate* wndTemplate = (WindowTemplate*)((unsigned long)LuaScript::PopNumber(L));

	AD2D_Font* font = NULL;
	if (wndTemplate)
		font = wndTemplate->GetFont();

	LuaScript::PushNumber(L, (unsigned long)font);
	return 1;
}

int Windows::LuaCreateWindow(lua_State* L) {
	WindowTemplate* wndTemplate = NULL;
	if (lua_type(L, -1) == LUA_TSTRING) {
		std::string templateName = LuaScript::PopString(L);
		Windows* wnds = Windows::wnds;
		if (wnds)
			wndTemplate = wnds->GetWindowTemplate(Game::options.templatesGroup, templateName);
	}
	else
		wndTemplate = (WindowTemplate*)((unsigned long)LuaScript::PopNumber(L));

	int posY = (int)LuaScript::PopNumber(L);
	int posX = (int)LuaScript::PopNumber(L);

	if (wndTemplate) {
		Window* wnd = new(M_PLACE) Window(WND_CUSTOM, posX, posY, wndTemplate);

		LuaScript::PushNumber(L, (unsigned long)wnd);
		return 1;
	}

	lua_pushnil(L);
	return 1;
}

int Windows::LuaFindWindow(lua_State* L) {
	WINDOW_TYPE wndType = (WINDOW_TYPE)LuaScript::PopNumber(L);

	Game* game = Game::game;
	Windows* wnds = Windows::wnds;
	if (game && wnds) {
		Window* wnd = wnds->FindWindow(wndType);

		LuaScript::PushNumber(L, (unsigned long)wnd);
		return 1;
	}

	lua_pushnil(L);
	return 1;
}

int Windows::LuaOpenWindow(lua_State* L) {
	WINDOW_TYPE wndType = (WINDOW_TYPE)LuaScript::PopNumber(L);

	Game* game = Game::game;
	Windows* wnds = Windows::wnds;
	if (game && wnds) {
		Window* wnd = wnds->OpenWindow(wndType, game);

		LuaScript::PushNumber(L, (unsigned long)wnd);
		return 1;
	}

	lua_pushnil(L);
	return 1;
}

int Windows::LuaGetWindowByTitle(lua_State* L) {
	int params = lua_gettop(L);
	WINDOW_TYPE wndType = WND_ALL;
	if (params > 1)
		wndType = (WINDOW_TYPE)LuaScript::PopNumber(L);
	std::string title = LuaScript::PopString(L);

	Game* game = Game::game;
	Windows* wnds = Windows::wnds;
	if (game && wnds) {
		std::list<Window*>::iterator it = wnds->wndList.begin();

		Window* wndRet = NULL;
		for (it; it != wnds->wndList.end(); it++) {
			Window* wnd = *it;
			if (wnd && wnd->title == title && (wndType == WND_ALL || wndType == (WINDOW_TYPE)wnd->wndType)) {
				wndRet = wnd;
				break;
			}
		}

		LuaScript::PushNumber(L, (unsigned long)wndRet);
		return 1;
	}

	lua_pushnil(L);
	return 1;
}

int Windows::LuaAddWindow(lua_State* L) {
	Window* wnd = (Window*)((unsigned long)LuaScript::PopNumber(L));

	Windows* wnds = Windows::wnds;
	if (wnds)
		wnds->AddWindow(wnd);

	return 1;
}

int Windows::LuaKillWindow(lua_State* L) {
	Window* wnd = (Window*)((unsigned long)LuaScript::PopNumber(L));

	Windows* wnds = Windows::wnds;
	if (wnds)
		wnds->KillWindow(wnd);

	return 1;
}

int Windows::LuaCloseWindow(lua_State* L) {
	Window* wnd = (Window*)((unsigned long)LuaScript::PopNumber(L));

	if (wnd)
		wnd->SetAction(ACT_CLOSE);

	return 1;
}

int Windows::LuaMoveWindowOnTop(lua_State* L) {
	Window* wnd = (Window*)((unsigned long)LuaScript::PopNumber(L));

	Windows* wnds = Windows::wnds;
	if (wnds)
		wnds->MoveOnTop(wnd);

	return 1;
}

int Windows::LuaIsWindowOnList(lua_State* L) {
	Window* wnd = (Window*)((unsigned long)LuaScript::PopNumber(L));

	Windows* wnds = Windows::wnds;
	if (wnds) {
		LuaScript::PushNumber(L, (int)wnds->IsOnList(wnd));
		return 1;
	}

	LuaScript::PushNumber(L, 0);
	return 1;
}

int Windows::LuaSetWindowParam(lua_State* L) {
	int params = lua_gettop(L);
	int p = params;

	LuaScript* script = LuaScript::GetLuaScriptState(L);

	Window* wnd = (Window*)((unsigned long)lua_tonumber(L, -p--));
	std::string param = lua_tostring(L, -p--);

	int P = p + 1;

	if (!wnd)
		return 1;

	if (param == "title")
		wnd->SetTitle(lua_tostring(L, -P + p--));
	else if (param == "position" || param == "pos")
		wnd->SetPosition(lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), NULL, (p > 2 ? (bool)lua_tonumber(L, -P + p--) : false));
	else if (param == "size")
		wnd->SetSize(lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), NULL, (p > 2 ? (bool)lua_tonumber(L, -P + p--) : false));
	else if (param == "type")
		wnd->SetWindowType(lua_tonumber(L, -P + p--));
	else if (param == "executeaction") {
		std::string scriptName = lua_tostring(L, -p--);
		if (scriptName == "clear" && p == 0)
			wnd->AddExecuteFunction(NULL);
		else {
			std::string funct = lua_tostring(L, -p--);
			LuaArguments args;
			for (p; p > 2; p--)
				args.push_back(lua_tostring(L, -p));
			bool localDir = (bool)lua_tonumber(L, -p--);
			bool sameScript = (bool)lua_tonumber(L, -p--);

			std::string filename = LuaScript::GetFilePath(scriptName, localDir);
			boost::function<void()> func = boost::bind(&LuaScript::RunScript, filename, funct, args, false, (sameScript ? script : NULL));

			wnd->AddExecuteFunction(func);
		}
	}
	else if (param == "activateaction") {
		std::string scriptName = lua_tostring(L, -p--);
		if (scriptName == "clear" && p == 0)
			wnd->AddActivateFunction(NULL);
		else {
			std::string funct = lua_tostring(L, -p--);
			LuaArguments args;
			for (p; p > 2; p--)
				args.push_back(lua_tostring(L, -p));
			bool localDir = (bool)lua_tonumber(L, -p--);
			bool sameScript = (bool)lua_tonumber(L, -p--);

			std::string filename = LuaScript::GetFilePath(scriptName, localDir);
			boost::function<void()> func = boost::bind(&LuaScript::RunScript, filename, funct, args, false, (sameScript ? script : NULL));

			wnd->AddActivateFunction(func);
		}
	}
	else if (param == "closeaction") {
		std::string scriptName = lua_tostring(L, -p--);
		if (scriptName == "clear" && p == 0)
			wnd->AddCloseFunction(NULL);
		else {
			std::string funct = lua_tostring(L, -p--);
			LuaArguments args;
			for (p; p > 2; p--)
				args.push_back(lua_tostring(L, -p));
			bool localDir = (bool)lua_tonumber(L, -p--);
			bool sameScript = (bool)lua_tonumber(L, -p--);

			std::string filename = LuaScript::GetFilePath(scriptName, localDir);
			boost::function<void()> func = boost::bind(&LuaScript::RunScript, filename, funct, args, false, (sameScript ? script : NULL));

			wnd->AddCloseFunction(func);
		}
	}
	else if (param == "minimizeaction") {
		std::string scriptName = lua_tostring(L, -p--);
		if (scriptName == "clear" && p == 0)
			wnd->AddMinimizeFunction(NULL);
		else {
			std::string funct = lua_tostring(L, -p--);
			LuaArguments args;
			for (p; p > 2; p--)
				args.push_back(lua_tostring(L, -p));
			bool localDir = (bool)lua_tonumber(L, -p--);
			bool sameScript = (bool)lua_tonumber(L, -p--);

			std::string filename = LuaScript::GetFilePath(scriptName, localDir);
			boost::function<void()> func = boost::bind(&LuaScript::RunScript, filename, funct, args, false, (sameScript ? script : NULL));

			wnd->AddMinimizeFunction(func);
		}
	}
	else if (param == "intsize") {
		bool adjust = (p > 2 ? (bool)lua_tonumber(L, -P + p--) : false);
		wnd->SetIntSize(lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--));
		if (adjust)
			wnd->AdjustSize();
	}
	else if (param == "minsize")
		wnd->SetMinSize(lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--));
	else if (param == "maxsize")
		wnd->SetMaxSize(lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--));
	else if (param == "closeability")
		wnd->SetCloseAbility((bool)lua_tonumber(L, -P + p--));
	else if (param == "minimizeability")
		wnd->SetMinimizeAbility((bool)lua_tonumber(L, -P + p--));
	else if (param == "hideability")
		wnd->SetHideAbility((bool)lua_tonumber(L, -P + p--));
	else if (param == "alwaysactive")
		wnd->SetAlwaysActive((bool)lua_tonumber(L, -P + p--));
	else if (param == "template")
		wnd->SetWindowTemplate((WindowTemplate*)((unsigned long)lua_tonumber(L, -P + p--)));

	lua_pop(L, params);
	return 1;
}

int Windows::LuaGetWindowParam(lua_State* L) {
	std::string param = LuaScript::PopString(L);
	Window* wnd = (Window*)((unsigned long)LuaScript::PopNumber(L));

	if (!wnd) {
		lua_pushnil(L);
		return 1;
	}

	if (param == "title")
		LuaScript::PushString(L, wnd->title);
	else if (param == "position" || param == "pos") {
		lua_newtable(L);
		lua_pushstring(L, "x");
		lua_pushnumber(L, wnd->posX);
		lua_settable(L, -3);
		lua_pushstring(L, "y");
		lua_pushnumber(L, wnd->posY);
		lua_settable(L, -3);
	}
	else if (param == "size") {
		lua_newtable(L);
		lua_pushstring(L, "x");
		lua_pushnumber(L, wnd->width);
		lua_settable(L, -3);
		lua_pushstring(L, "y");
		lua_pushnumber(L, wnd->height);
		lua_settable(L, -3);
	}
	else if (param == "template")
		LuaScript::PushNumber(L, (unsigned long)wnd->GetWindowTemplate());
	else if (param == "container")
		LuaScript::PushNumber(L, (unsigned long)wnd->GetWindowContainer());
	else
		lua_pushnil(L);

	return 1;
}

int Windows::LuaCreateWindowElement(lua_State* L) {
	int params = lua_gettop(L);
	int p = params;

	std::string element = lua_tostring(L, -p--);

	int P = p + 1;

	WindowElement* ptrElement = NULL;
	if (element == "container") {
		WindowElementContainer* ptr = new(M_PLACE) WindowElementContainer;
		ptr->Create(lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), (bool)lua_tonumber(L, -P + p--), (bool)lua_tonumber(L, -P + p--), (WindowTemplate*)((unsigned long)lua_tonumber(L, -P + p--)));
		ptrElement = ptr;
	}
	else if (element == "text") {
		WindowElementText* ptr = new(M_PLACE) WindowElementText;
		ptr->Create(lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), (WindowTemplate*)((unsigned long)lua_tonumber(L, -P + p--)));
		ptrElement = ptr;
	}
	else if (element == "link") {
		WindowElementLink* ptr = new(M_PLACE) WindowElementLink;
		ptr->Create(lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), (Mouse*)((unsigned long)lua_tonumber(L, -P + p--)), NULL, (WindowTemplate*)((unsigned long)lua_tonumber(L, -P + p--)));
		ptrElement = ptr;
	}
	else if (element == "image") {
		WindowElementImage* ptr = new(M_PLACE) WindowElementImage;
		ptr->Create(lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), (AD2D_Image*)((unsigned long)lua_tonumber(L, -P + p--)), (WindowTemplate*)((unsigned long)lua_tonumber(L, -P + p--)));
		ptrElement = ptr;
	}
	else if (element == "button") {
		WindowElementButton* ptr = new(M_PLACE) WindowElementButton;
		ptr->Create(lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), (WindowTemplate*)((unsigned long)lua_tonumber(L, -P + p--)));
		ptrElement = ptr;
	}
	else if (element == "textarea") {
		WindowElementTextarea* ptr = new(M_PLACE) WindowElementTextarea;
		ptr->Create(lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), (bool)lua_tonumber(L, -P + p--), (bool)lua_tonumber(L, -P + p--), (WindowTemplate*)((unsigned long)lua_tonumber(L, -P + p--)));
		ptrElement = ptr;
	}
	else if (element == "memo") {
		WindowElementMemo* ptr = new(M_PLACE) WindowElementMemo;
		ptr->Create(lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), (WindowTemplate*)((unsigned long)lua_tonumber(L, -P + p--)));
		ptrElement = ptr;
	}
	else if (element == "tablememo") {
		WindowElementTableMemo* ptr = new(M_PLACE) WindowElementTableMemo;
		ptr->Create(lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), (WindowTemplate*)((unsigned long)lua_tonumber(L, -P + p--)));
		ptrElement = ptr;
	}
	else if (element == "list") {
		WindowElementList* ptr = new(M_PLACE) WindowElementList;
		ptr->Create(lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), (WindowTemplate*)((unsigned long)lua_tonumber(L, -P + p--)));
		ptrElement = ptr;
	}
	else if (element == "timer") {
		WindowElementTimer* ptr = new(M_PLACE) WindowElementTimer;
		ptr->Create(lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), (WindowTemplate*)((unsigned long)lua_tonumber(L, -P + p--)));
		ptrElement = ptr;
	}
	else if (element == "tab") {
		WindowElementTab* ptr = new(M_PLACE) WindowElementTab;
		ptr->Create(lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), (bool)lua_tonumber(L, -P + p--), (bool)lua_tonumber(L, -P + p--), (WindowTemplate*)((unsigned long)lua_tonumber(L, -P + p--)));
		ptrElement = ptr;
	}
	else if (element == "itemcontainer") {
		WindowElementItemContainer* ptr = new(M_PLACE) WindowElementItemContainer;
		ptr->Create(lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), (Container*)((unsigned long)lua_tonumber(L, -P + p--)), (WindowTemplate*)((unsigned long)lua_tonumber(L, -P + p--)));
		ptrElement = ptr;
	}
	else if (element == "checkbox") {
		WindowElementCheckBox* ptr = new(M_PLACE) WindowElementCheckBox;
		ptr->Create(lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), (WindowTemplate*)((unsigned long)lua_tonumber(L, -P + p--)));
		ptrElement = ptr;
	}
	else if (element == "scrollbar") {
		WindowElementScrollBar* ptr = new(M_PLACE) WindowElementScrollBar;
		ptr->Create(lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), (WindowTemplate*)((unsigned long)lua_tonumber(L, -P + p--)));
		ptrElement = ptr;
	}
	else if (element == "skillbar") {
		WindowElementSkillBar* ptr = new(M_PLACE) WindowElementSkillBar;
		ptr->Create(lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), LuaScript::ReadPointer(L, -P + p--), (WindowTemplate*)((unsigned long)lua_tonumber(L, -P + p--)));
		ptrElement = ptr;
	}
	else if (element == "colormap") {
		WindowElementColorMap* ptr = new(M_PLACE) WindowElementColorMap;
		ptr->Create(lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), (WindowTemplate*)((unsigned long)lua_tonumber(L, -P + p--)));
		ptrElement = ptr;
	}
	else if (element == "cooldown") {
		WindowElementCooldown* ptr = new(M_PLACE) WindowElementCooldown;
		ptr->Create(lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), (WindowTemplate*)((unsigned long)lua_tonumber(L, -P + p--)));
		ptrElement = ptr;
	}
	else if (element == "select") {
		WindowElementSelect* ptr = new(M_PLACE) WindowElementSelect;
		ptr->Create(lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), (Mouse*)((unsigned long)lua_tonumber(L, -P + p--)), NULL, (WindowTemplate*)((unsigned long)lua_tonumber(L, -P + p--)));
		ptrElement = ptr;
	}
	else if (element == "playground") {
		WindowElementPlayground* ptr = new(M_PLACE) WindowElementPlayground;
		ptr->Create(lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), Game::game, (WindowTemplate*)((unsigned long)lua_tonumber(L, -P + p--)));
		ptrElement = ptr;
	}
	else if (element == "minimap") {
		WindowElementMiniMap* ptr = new(M_PLACE) WindowElementMiniMap;
		ptr->Create(lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), (MiniMap*)((unsigned long)lua_tonumber(L, -P + p--)), (WindowTemplate*)((unsigned long)lua_tonumber(L, -P + p--)));
		ptrElement = ptr;
	}

	lua_pop(L, params);

	LuaScript::PushNumber(L, (unsigned long)ptrElement);
	return 1;
}

int Windows::LuaGetWindowElement(lua_State* L) {
	int params = lua_gettop(L);

	int number = 0;
	if (params > 1)
		number = LuaScript::PopNumber(L);
	WindowElementContainer* container = dynamic_cast<WindowElementContainer*>((WindowElement*)((unsigned long)LuaScript::PopNumber(L)));

	if (container) {
		WindowElement* element = NULL;
		std::list<WindowElement*>::iterator it = container->elements.begin();
		for (it; it != container->elements.end() && number >= 0; it++, number--)
			element = *it;

		LuaScript::PushNumber(L, (unsigned long)element);
		return 1;
	}

	lua_pushnil(L);
	return 1;
}

int Windows::LuaAddWindowElement(lua_State* L) {
	WindowElement* element = (WindowElement*)((unsigned long)LuaScript::PopNumber(L));
	WindowElementContainer* container = dynamic_cast<WindowElementContainer*>((WindowElement*)((unsigned long)LuaScript::PopNumber(L)));

	if (container && element)
		container->AddElement(element);

	return 1;
}

int Windows::LuaRemoveWindowElement(lua_State* L) {
	WindowElement* element = (WindowElement*)((unsigned long)LuaScript::PopNumber(L));
	WindowElementContainer* container = dynamic_cast<WindowElementContainer*>((WindowElement*)((unsigned long)LuaScript::PopNumber(L)));

	if (container && element)
		container->RemoveElement(element);

	return 1;
}

int Windows::LuaDeleteWindowElement(lua_State* L) {
	WindowElement* element = (WindowElement*)((unsigned long)LuaScript::PopNumber(L));

	if (element)
		delete_debug(element, M_PLACE);

	return 1;
}

int Windows::LuaClearWindowElements(lua_State* L) {
	WindowElementContainer* container = dynamic_cast<WindowElementContainer*>((WindowElement*)((unsigned long)LuaScript::PopNumber(L)));

	if (container)
		container->DeleteAllElements();

	return 1;
}

int Windows::LuaSetWindowElementParam(lua_State* L) {
	int params = lua_gettop(L);
	int p = params;

	LuaScript* script = LuaScript::GetLuaScriptState(L);

	WindowElement* element = (WindowElement*)((unsigned long)lua_tonumber(L, -p--));
	std::string param = lua_tostring(L, -p--);

	int P = p + 1;

	WindowElementContainer* container = dynamic_cast<WindowElementContainer*>(element);
	WindowElementText* text = dynamic_cast<WindowElementText*>(element);
	WindowElementLink* link = dynamic_cast<WindowElementLink*>(element);
	WindowElementImage* image = dynamic_cast<WindowElementImage*>(element);
	WindowElementButton* button = dynamic_cast<WindowElementButton*>(element);
	WindowElementTextarea* textarea = dynamic_cast<WindowElementTextarea*>(element);
	WindowElementMemo* memo = dynamic_cast<WindowElementMemo*>(element);
	WindowElementTableMemo* tmemo = dynamic_cast<WindowElementTableMemo*>(element);
	WindowElementList* list = dynamic_cast<WindowElementList*>(element);
	WindowElementCheckBox* checkbox = dynamic_cast<WindowElementCheckBox*>(element);
	WindowElementItemContainer* itemcontainer = dynamic_cast<WindowElementItemContainer*>(element);
	WindowElementTimer* timer = dynamic_cast<WindowElementTimer*>(element);
	WindowElementTab* tab = dynamic_cast<WindowElementTab*>(element);
	WindowElementSkillBar* skillbar = dynamic_cast<WindowElementSkillBar*>(element);
	WindowElementScrollBar* scrollbar = dynamic_cast<WindowElementScrollBar*>(element);
	WindowElementColorMap* colormap = dynamic_cast<WindowElementColorMap*>(element);
	WindowElementSelect* select = dynamic_cast<WindowElementSelect*>(element);
	WindowElementCooldown* cooldown = dynamic_cast<WindowElementCooldown*>(element);

	if (param == "position" || param == "pos")
		element->SetPosition(lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--));
	else if (param == "size")
		element->SetSize(lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--));
	else if (param == "locks")
		element->SetLocks((bool)lua_tonumber(L, -P + p--), (bool)lua_tonumber(L, -P + p--));
	else if (param == "enabled")
		element->SetEnabled((bool)lua_tonumber(L, -P + p--));
	else if (param == "shine")
		element->SetShine(lua_tonumber(L, -P + p--));
	else if (param == "comment")
		element->comment = lua_tostring(L, -P + p--);
	else if (param == "action") {
		std::string scriptName = lua_tostring(L, -p--);
		if (scriptName == "clear" && p == 0)
			element->SetAction(NULL);
		else {
			std::string funct = lua_tostring(L, -p--);
			LuaArguments args;
			for (p; p > 2; p--)
				args.push_back(lua_tostring(L, -p));
			bool localDir = (bool)lua_tonumber(L, -p--);
			bool sameScript = (bool)lua_tonumber(L, -p--);

			std::string filename = LuaScript::GetFilePath(scriptName, localDir);
			boost::function<void()> func = boost::bind(&LuaScript::RunScript, filename, funct, args, false, (sameScript ? script : NULL));

			element->SetAction(func);
		}
	}
	else if (param == "actiondbl") {
		if (memo) {
			std::string scriptName = lua_tostring(L, -p--);
			if (scriptName == "clear" && p == 0)
				memo->SetDblAction(NULL);
			else {
				std::string funct = lua_tostring(L, -p--);
				LuaArguments args;
				for (p; p > 2; p--)
					args.push_back(lua_tostring(L, -p));
				bool localDir = (bool)lua_tonumber(L, -p--);
				bool sameScript = (bool)lua_tonumber(L, -p--);

				std::string filename = LuaScript::GetFilePath(scriptName, localDir);
				boost::function<void()> func = boost::bind(&LuaScript::RunScript, filename, funct, args, false, (sameScript ? script : NULL));

				memo->SetDblAction(func);
			}
		}
		else if (tmemo) {
			std::string scriptName = lua_tostring(L, -p--);
			if (scriptName == "clear" && p == 0)
				tmemo->SetDblAction(NULL);
			else {
				std::string funct = lua_tostring(L, -p--);
				LuaArguments args;
				for (p; p > 2; p--)
					args.push_back(lua_tostring(L, -p));
				bool localDir = (bool)lua_tonumber(L, -p--);
				bool sameScript = (bool)lua_tonumber(L, -p--);

				std::string filename = LuaScript::GetFilePath(scriptName, localDir);
				boost::function<void()> func = boost::bind(&LuaScript::RunScript, filename, funct, args, false, (sameScript ? script : NULL));

				tmemo->SetDblAction(func);
			}
		}
	}
	else if (param == "actionlook") {
		if (itemcontainer) {
			std::string scriptName = lua_tostring(L, -p--);
			if (scriptName == "clear" && p == 0)
				itemcontainer->SetLookAction(NULL);
			else {
				std::string funct = lua_tostring(L, -p--);
				LuaArguments args;
				for (p; p > 2; p--)
					args.push_back(lua_tostring(L, -p));
				bool localDir = (bool)lua_tonumber(L, -p--);
				bool sameScript = (bool)lua_tonumber(L, -p--);

				std::string filename = LuaScript::GetFilePath(scriptName, localDir);
				boost::function<void()> func = boost::bind(&LuaScript::RunScript, filename, funct, args, false, (sameScript ? script : NULL));

				itemcontainer->SetLookAction(func);
			}
		}
	}
	else if (param == "actionuse") {
		if (itemcontainer) {
			std::string scriptName = lua_tostring(L, -p--);
			if (scriptName == "clear" && p == 0)
				itemcontainer->SetUseAction(NULL);
			else {
				std::string funct = lua_tostring(L, -p--);
				LuaArguments args;
				for (p; p > 2; p--)
					args.push_back(lua_tostring(L, -p));
				bool localDir = (bool)lua_tonumber(L, -p--);
				bool sameScript = (bool)lua_tonumber(L, -p--);

				std::string filename = LuaScript::GetFilePath(scriptName, localDir);
				boost::function<void()> func = boost::bind(&LuaScript::RunScript, filename, funct, args, false, (sameScript ? script : NULL));

				itemcontainer->SetUseAction(func);
			}
		}
	}
	else if (param == "intsize") {
		if (container) {
			bool adjust = (p > 2 ? (bool)lua_tonumber(L, -P + p--) : false);
			container->SetIntSize(lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--));
			if (adjust)
				container->AdjustSize();
		}
	}
	else if (param == "textalign") {
		if (text)
			text->SetAlign(lua_tonumber(L, -P + p--));
		else if (link)
			link->SetAlign(lua_tonumber(L, -P + p--));
	}
	else if (param == "text") {
		if (text)
			text->SetText(lua_tostring(L, -P + p--));
		else if (link)
			link->SetText(lua_tostring(L, -P + p--));
		else if (button)
			button->SetText(lua_tostring(L, -P + p--));
		else if (textarea)
			textarea->SetText(lua_tostring(L, -P + p--));
		else if (checkbox)
			checkbox->SetText(lua_tostring(L, -P + p--));
	}
	else if (param == "color") {
		float alpha = (p > 3 ? lua_tonumber(L, -P + p--) : 1.0);
		if (text)
			text->SetColor(lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), alpha);
		else if (memo)
			memo->SetColor(lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), alpha);
		else if (list)
			list->SetColor(lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), alpha);
		else if (textarea)
			textarea->SetColor(lua_tonumber(L, -P + p--));
		else if (button)
			button->SetColor(lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), alpha);
		else if (skillbar)
			skillbar->SetColor(lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), alpha);
		else if (tab)
			tab->SetColor(lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), alpha);
	}
	else if (param == "colorin") {
		float alpha = (p > 3 ? lua_tonumber(L, -P + p--) : 1.0);
		if (link)
			link->SetColorIn(lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), alpha);
		else if (select)
			select->SetColorIn(lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), alpha);
	}
	else if (param == "colorout") {
		float alpha = (p > 3 ? lua_tonumber(L, -P + p--) : 1.0);
		if (link)
			link->SetColorOut(lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), alpha);
		else if (select)
			select->SetColorOut(lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--), alpha);
	}
	else if (param == "background") {
		if (container)
			container->SetContainerBackground((bool)lua_tonumber(L, -P + p--));
		else if (cooldown)
			cooldown->SetBackground((AD2D_Image*)((unsigned long)lua_tonumber(L, -P + p--)));
	}
	else if (param == "border") {
		if (element)
			element->SetBorder(lua_tonumber(L, -P + p--));
	}
	else if (param == "fontsize") {
		if (element)
			element->SetFontSize(lua_tonumber(L, -P + p--));
	}
	else if (param == "precision") {
		if (text)
			text->SetPrecision(lua_tonumber(L, -P + p--));
	}
	else if (param == "scroll") {
		if (container)
			container->SetScroll((bool)lua_tonumber(L, -P + p--));
	}
	else if (param == "scrollalwaysvisible") {
		if (container)
			container->SetScrollAlwaysVisible((bool)lua_tonumber(L, -P + p--), (bool)lua_tonumber(L, -P + p--));
	}
	else if (param == "image") {
		if (image)
			image->SetImage((AD2D_Image*)((unsigned long)lua_tonumber(L, -P + p--)));
		else if (button)
			button->SetImage((AD2D_Image*)((unsigned long)lua_tonumber(L, -P + p--)));
		else if (itemcontainer)
			itemcontainer->SetImage((AD2D_Image*)((unsigned long)lua_tonumber(L, -P + p--)));
		else if (cooldown)
			cooldown->SetIcon((AD2D_Image*)((unsigned long)lua_tonumber(L, -P + p--)));
	}
	else if (param == "hotkey") {
		if (itemcontainer)
			itemcontainer->SetHotKey((HotKey*)((unsigned long)lua_tonumber(L, -P + p--)));
	}
	else if (param == "slot") {
		if (itemcontainer)
			itemcontainer->SetSlot(lua_tonumber(L, -P + p--));
	}
	else if (param == "pressed") {
		if (button)
			button->SetPressed((bool)lua_tonumber(L, -P + p--));
	}
	else if (param == "inverse") {
		if (button)
			button->SetInverse((bool)lua_tonumber(L, -P + p--));
	}
	else if (param == "offset") {
		if (textarea)
			textarea->SetOffset(lua_tonumber(L, -P + p--));
	}
	else if (param == "maxlength") {
		if (textarea)
			textarea->SetMaxLength(lua_tonumber(L, -P + p--));
	}
	else if (param == "hidden") {
		if (textarea)
			textarea->SetHidden((bool)lua_tonumber(L, -P + p--));
	}
	else if (param == "active") {
		if (textarea)
			textarea->SetActive((bool)lua_tonumber(L, -P + p--));
	}
	else if (param == "option") {
		if (memo) {
			if (lua_type(L, -p) == LUA_TSTRING)
				memo->SetOption(lua_tostring(L, -P + p--), (bool)lua_tonumber(L, -P + p--));
			else
				memo->SetOption(lua_tonumber(L, -P + p--), (bool)lua_tonumber(L, -P + p--));
		}
		else if (tmemo)
			tmemo->SetOption(lua_tonumber(L, -P + p--), (bool)lua_tonumber(L, -P + p--));
		else if (list) {
			if (lua_type(L, -p) == LUA_TSTRING)
				list->SetOption(lua_tostring(L, -P + p--));
			else
				list->SetOption(lua_tonumber(L, -P + p--));
		}
	}
	else if (param == "rowheight") {
		if (tmemo)
			tmemo->SetRowHeight(lua_tonumber(L, -P + p--));
	}
	else if (param == "columnwidth") {
		if (tmemo) {
			bool percent = (p > 2 ? (bool)lua_tonumber(L, -P + p--) : false);
			if (percent)
				tmemo->SetColumnWidthPercent(lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--));
			else
				tmemo->SetColumnWidthPX(lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--));
		}
	}
	else if (param == "columnname") {
		if (tmemo)
			tmemo->SetColumnName(lua_tonumber(L, -P + p--), lua_tostring(L, -P + p--));
	}
	else if (param == "timestop") {
		if (timer)
			timer->SetStopTime(lua_tonumber(L, -P + p--));
	}
	else if (param == "timeinterval") {
		if (timer)
			timer->SetTimeInterval(lua_tonumber(L, -P + p--));
	}
	else if (param == "state") {
		if (checkbox)
			checkbox->SetState((bool)lua_tonumber(L, -P + p--));
	}
	else if (param == "valueptr") {
		if (text)
			text->SetValuePtr(LuaScript::ReadPointer(L, -P + p--));
		else if (scrollbar)
			scrollbar->SetValuePtr(LuaScript::ReadPointer(L, -P + p--));
		else if (skillbar)
			skillbar->SetValuePtr(LuaScript::ReadPointer(L, -P + p--));
	}
	else if (param == "minvalue") {
		if (scrollbar)
			scrollbar->SetMinValue(lua_tonumber(L, -P + p--));
		else if (skillbar)
			skillbar->SetMinValue(lua_tonumber(L, -P + p--));
	}
	else if (param == "maxvalue") {
		if (scrollbar)
			scrollbar->SetMaxValue(lua_tonumber(L, -P + p--));
		else if (skillbar)
			skillbar->SetMaxValue(lua_tonumber(L, -P + p--));
	}
	else if (param == "tabsheight") {
		if (tab)
			tab->SetTabsHeight(lua_tonumber(L, -P + p--));
	}
	else if (param == "template") {
		if (element)
			element->SetWindowTemplate((WindowTemplate*)((unsigned long)lua_tonumber(L, -P + p--)));
	}
	else if (param == "spellid") {
		if (cooldown)
			cooldown->SetSpellID(lua_tonumber(L, -P + p--));
	}
	else if (param == "groupid") {
		if (cooldown)
			cooldown->SetGroupID(lua_tonumber(L, -P + p--));
	}
	else if (param == "cast") {
		if (cooldown)
			cooldown->SetCast(lua_tonumber(L, -P + p--), lua_tonumber(L, -P + p--));
	}

	lua_pop(L, params);
	return 1;
}

int Windows::LuaGetWindowElementParam(lua_State* L) {
	int params = lua_gettop(L);

	std::string temp;
	if (params > 2)
		temp = LuaScript::PopString(L);
	std::string param = LuaScript::PopString(L);
	WindowElement* element = (WindowElement*)((unsigned long)LuaScript::PopNumber(L));

	if (!element) {
		lua_pushnil(L);
		return 1;
	}

	WindowElementContainer* container = dynamic_cast<WindowElementContainer*>(element);
	WindowElementText* text = dynamic_cast<WindowElementText*>(element);
	WindowElementLink* link = dynamic_cast<WindowElementLink*>(element);
	WindowElementImage* image = dynamic_cast<WindowElementImage*>(element);
	WindowElementButton* button = dynamic_cast<WindowElementButton*>(element);
	WindowElementTextarea* textarea = dynamic_cast<WindowElementTextarea*>(element);
	WindowElementMemo* memo = dynamic_cast<WindowElementMemo*>(element);
	WindowElementTableMemo* tmemo = dynamic_cast<WindowElementTableMemo*>(element);
	WindowElementList* list = dynamic_cast<WindowElementList*>(element);
	WindowElementCheckBox* checkbox = dynamic_cast<WindowElementCheckBox*>(element);
	WindowElementItemContainer* itemcontainer = dynamic_cast<WindowElementItemContainer*>(element);
	WindowElementTimer* timer = dynamic_cast<WindowElementTimer*>(element);
	WindowElementTab* tab = dynamic_cast<WindowElementTab*>(element);
	WindowElementSkillBar* skillbar = dynamic_cast<WindowElementSkillBar*>(element);
	WindowElementScrollBar* scrollbar = dynamic_cast<WindowElementScrollBar*>(element);
	WindowElementColorMap* colormap = dynamic_cast<WindowElementColorMap*>(element);
	WindowElementSelect* select = dynamic_cast<WindowElementSelect*>(element);
	WindowElementCooldown* cooldown = dynamic_cast<WindowElementCooldown*>(element);

	if (param == "position" || param == "pos") {
		int x = element->posX;
		int y = element->posY;
		if (temp == "abs") {
			POINT absPos = element->GetAbsolutePosition(true);
			x = absPos.x;
			y = absPos.y;
		}
		lua_newtable(L);
		lua_pushstring(L, "x");
		lua_pushnumber(L, x);
		lua_settable(L, -3);
		lua_pushstring(L, "y");
		lua_pushnumber(L, y);
		lua_settable(L, -3);
	}
	else if (param == "size") {
		lua_newtable(L);
		lua_pushstring(L, "x");
		lua_pushnumber(L, element->width);
		lua_settable(L, -3);
		lua_pushstring(L, "y");
		lua_pushnumber(L, element->height);
		lua_settable(L, -3);
	}
	else if (param == "shine")
		LuaScript::PushNumber(L, element->GetShine());
	else if (param == "comment")
		LuaScript::PushString(L, element->comment);
	else if (param == "intsize") {
		if (container) {
			POINT intSize = container->GetIntSize();
			lua_newtable(L);
			lua_pushstring(L, "x");
			lua_pushnumber(L, intSize.x);
			lua_settable(L, -3);
			lua_pushstring(L, "y");
			lua_pushnumber(L, intSize.y);
			lua_settable(L, -3);
		}
	}
	else if (param == "scrolloffset") {
		if (container) {
			POINT intSize = container->GetScrollOffset();
			lua_newtable(L);
			lua_pushstring(L, "x");
			lua_pushnumber(L, intSize.x);
			lua_settable(L, -3);
			lua_pushstring(L, "y");
			lua_pushnumber(L, intSize.y);
			lua_settable(L, -3);
		}
		else if (textarea)
			LuaScript::PushNumber(L, textarea->GetScrollOffset());
		else if (memo)
			LuaScript::PushNumber(L, memo->GetScrollOffset());
	}
	else if (param == "text") {
		if (text)
			LuaScript::PushString(L, text->GetText().text);
		else if (link)
			LuaScript::PushString(L, link->GetText().text);
		else if (textarea)
			LuaScript::PushString(L, textarea->GetText());
		else if (checkbox)
			LuaScript::PushString(L, checkbox->GetText());
	}
	else if (param == "pressed") {
		if (button)
			LuaScript::PushNumber(L, (int)button->GetPressed());
	}
	else if (param == "inverse") {
		if (button)
			LuaScript::PushNumber(L, (int)button->GetInverse());
	}
	else if (param == "option") {
		if (memo)
			LuaScript::PushNumber(L, memo->GetOption());
		else if (tmemo)
			LuaScript::PushNumber(L, tmemo->GetOption());
		else if (list)
			LuaScript::PushNumber(L, list->GetOption());
	}
	else if (param == "optionssize") {
		if (memo)
			LuaScript::PushNumber(L, memo->GetOptionsSize());
		else if (tmemo)
			LuaScript::PushNumber(L, tmemo->GetOptionsSize());
		else if (list)
			LuaScript::PushNumber(L, list->GetOptionsSize());
	}
	else if (param == "element") {
		if (memo)
			LuaScript::PushString(L, memo->GetElement().text);
		else if (list)
			LuaScript::PushString(L, list->GetElement().text);
	}
	else if (param == "tabssize") {
		if (tab)
			LuaScript::PushNumber(L, tab->GetTabsSize());
	}
	else if (param == "activetab") {
		if (tab)
			LuaScript::PushNumber(L, tab->GetActiveTabNumber());
	}
	else if (param == "slot") {
		if (itemcontainer)
			LuaScript::PushNumber(L, itemcontainer->GetSlot());
	}
	else if (param == "container") {
		if (itemcontainer)
			LuaScript::PushNumber(L, (unsigned long)itemcontainer->GetContainer());
	}
	else if (param == "state") {
		if (checkbox)
			LuaScript::PushNumber(L, (int)checkbox->GetState());
	}
	else if (param == "valueptr") {
		TypePointer pointer;

		if (text)
			pointer = text->GetValuePtr();
		else if (scrollbar)
			pointer = scrollbar->GetValuePtr();
		else if (skillbar)
			pointer = skillbar->GetValuePtr();

		LuaScript::PushPointer(L, pointer);
	}
	else if (param == "period")
		LuaScript::PushNumber(L, cooldown->GetTimeRemain());
	else if (param == "template")
		LuaScript::PushNumber(L, (unsigned long)element->GetWindowTemplate());
	else
		lua_pushnil(L);

	return 1;
}

int Windows::LuaAddTableMemoRow(lua_State* L) {
	WindowElementTableMemo* tmemo = dynamic_cast<WindowElementTableMemo*>((WindowElement*)((unsigned long)LuaScript::PopNumber(L)));

	if (tmemo)
		tmemo->AddRow();

	return 1;
}

int Windows::LuaAddMemoElement(lua_State* L) {
	int params = lua_gettop(L);
	bool active = true;
	if (params > 2)
		active = (bool)LuaScript::PopNumber(L);

	std::string element = LuaScript::PopString(L);
	WindowElementMemo* memo = dynamic_cast<WindowElementMemo*>((WindowElement*)((unsigned long)LuaScript::PopNumber(L)));

	if (memo)
		memo->AddElement(element, active);

	return 1;
}

int Windows::LuaAddTableMemoColumn(lua_State* L) {
	WindowElement* element = (WindowElement*)((unsigned long)LuaScript::PopNumber(L));
	WindowElementTableMemo* tmemo = dynamic_cast<WindowElementTableMemo*>((WindowElement*)((unsigned long)LuaScript::PopNumber(L)));

	if (tmemo)
		tmemo->AddColumn(element);

	return 1;
}

int Windows::LuaAddListElement(lua_State* L) {
	int params = lua_gettop(L);
	bool active = true;
	if (params > 2)
		active = (bool)LuaScript::PopNumber(L);

	std::string element = LuaScript::PopString(L);
	WindowElementList* list = dynamic_cast<WindowElementList*>((WindowElement*)((unsigned long)LuaScript::PopNumber(L)));

	if (list)
		list->AddElement(element, active);

	return 1;
}

int Windows::LuaTimerRun(lua_State* L) {
	WindowElementTimer* timer = dynamic_cast<WindowElementTimer*>((WindowElement*)((unsigned long)LuaScript::PopNumber(L)));

	if (timer)
		timer->Run();

	return 1;
}

int Windows::LuaTimerStop(lua_State* L) {
	WindowElementTimer* timer = dynamic_cast<WindowElementTimer*>((WindowElement*)((unsigned long)LuaScript::PopNumber(L)));

	if (timer)
		timer->Stop();

	return 1;
}

int Windows::LuaGetGUIManager(lua_State* L) {
	GUIManager* guiManager = (wnds ? wnds->GetGUIManager() : NULL);

	LuaScript::PushNumber(L, (unsigned long)guiManager);
	return 1;
}


void Windows::LuaRegisterFunctions(lua_State* L) {
	lua_pushinteger(L, WND_GAME);
	lua_setglobal(L, "WND_GAME");
	lua_pushinteger(L, WND_CONSOLE);
	lua_setglobal(L, "WND_CONSOLE");
	lua_pushinteger(L, WND_INVENTORY);
	lua_setglobal(L, "WND_INVENTORY");
	lua_pushinteger(L, WND_CONTAINER);
	lua_setglobal(L, "WND_CONTAINER");
	lua_pushinteger(L, WND_CHANNELSLIST);
	lua_setglobal(L, "WND_CHANNELSLIST");
	lua_pushinteger(L, WND_MINIMAP);
	lua_setglobal(L, "WND_MINIMAP");
	lua_pushinteger(L, WND_STATISTICS);
	lua_setglobal(L, "WND_STATISTICS");
	lua_pushinteger(L, WND_BATTLE);
	lua_setglobal(L, "WND_BATTLE");
	lua_pushinteger(L, WND_VIPLIST);
	lua_setglobal(L, "WND_VIPLIST");
	lua_pushinteger(L, WND_ABOUT);
	lua_setglobal(L, "WND_ABOUT");
	lua_pushinteger(L, WND_OPTGAME);
	lua_setglobal(L, "WND_OPTGAME");
	lua_pushinteger(L, WND_OPTGRAPHIC);
	lua_setglobal(L, "WND_OPTGRAPHIC");
	lua_pushinteger(L, WND_OPTTEXT);
	lua_setglobal(L, "WND_OPTTEXT");
	lua_pushinteger(L, WND_OPTWINDOWS);
	lua_setglobal(L, "WND_OPTWINDOWS");
	lua_pushinteger(L, WND_OPTTEMPLATES);
	lua_setglobal(L, "WND_OPTTEMPLATES");
	lua_pushinteger(L, WND_OPTSOUND);
	lua_setglobal(L, "WND_OPTSOUND");
	lua_pushinteger(L, WND_REPORTERROR);
	lua_setglobal(L, "WND_REPORTERROR");
	lua_pushinteger(L, WND_STATUS);
	lua_setglobal(L, "WND_STATUS");
	lua_pushinteger(L, WND_HOTKEYS);
	lua_setglobal(L, "WND_HOTKEYS");
	lua_pushinteger(L, WND_OPTIONS);
	lua_setglobal(L, "WND_OPTIONS");
	lua_pushinteger(L, WND_CUSTOM);
	lua_setglobal(L, "WND_CUSTOM");

	lua_pushinteger(L, BUTTON_NONE);
	lua_setglobal(L, "BUTTON_NONE");
	lua_pushinteger(L, BUTTON_NORMAL);
	lua_setglobal(L, "BUTTON_NORMAL");
	lua_pushinteger(L, BUTTON_SWITCH);
	lua_setglobal(L, "BUTTON_SWITCH");
	lua_pushinteger(L, BUTTON_CONTINUE);
	lua_setglobal(L, "BUTTON_CONTINUE");
	lua_pushinteger(L, BUTTON_TAB);
	lua_setglobal(L, "BUTTON_TAB");
	lua_pushinteger(L, BUTTON_TAB_SIDE);
	lua_setglobal(L, "BUTTON_TAB_SIDE");

	//getWindowTemplate(templateName) : templatePtr
	lua_register(L, "getWindowTemplate", Windows::LuaGetWindowTemplate);

	//getWindowTemplateFont(templatePtr) : fontPtr
	lua_register(L, "getWindowTemplateFont", Windows::LuaGetWindowTemplateFont);

	//createWindow(x, y, templatePtr) : windowPtr
	//createWindow(x, y, templateName) : windowPtr
	lua_register(L, "createWindow", Windows::LuaCreateWindow);

	//findWindow(wndType) : windowPtr
	lua_register(L, "findWindow", Windows::LuaFindWindow);

	//openWindow(wndType) : windowPtr
	lua_register(L, "openWindow", Windows::LuaOpenWindow);

	//getWindowByTitle(title, wndType = WND_ALL) : windowPtr
	lua_register(L, "getWindowByTitle", Windows::LuaGetWindowByTitle);

	//addWindow(windowPtr)
	lua_register(L, "addWindow", Windows::LuaAddWindow);

	//killWindow(windowPtr)
	lua_register(L, "killWindow", Windows::LuaKillWindow);

	//closeWindow(windowPtr)
	lua_register(L, "closeWindow", Windows::LuaCloseWindow);

	//moveWindowOnTop(windowPtr)
	lua_register(L, "moveWindowOnTop", Windows::LuaMoveWindowOnTop);

	//isWindowOnList(windowPtr)
	lua_register(L, "isWindowOnList", Windows::LuaIsWindowOnList);

	//setWindowParam(windowPtr, 'title', title)
	//setWindowParam(windowPtr, 'position', x, y)
	//setWindowParam(windowPtr, 'size', width, height)
	//setWindowParam(windowPtr, 'executeaction', functionName, arg1, arg2, ...)
	//setWindowParam(windowPtr, 'activateaction', functionName, arg1, arg2, ...)
	//setWindowParam(windowPtr, 'closeaction', functionName, arg1, arg2, ...)
	//setWindowParam(windowPtr, 'intsize', width, height, adjust = 0)
	//setWindowParam(windowPtr, 'minsize', value)
	//setWindowParam(windowPtr, 'maxsize', value)
	//setWindowParam(windowPtr, 'closeability', state)
	//setWindowParam(windowPtr, 'minimizeability', state)
	//setWindowParam(windowPtr, 'alwaysactive', state)
	lua_register(L, "setWindowParam", Windows::LuaSetWindowParam);

	//getWindowParam(windowPtr, 'title') : titleStr
	//getWindowParam(windowPtr, 'position') : position{x, y}
	//getWindowParam(windowPtr, 'size') : size{x, y}
	//getWindowParam(windowPtr, 'template') : templatePtr
	//getWindowParam(windowPtr, 'container') : containerPtr
	lua_register(L, "getWindowParam", Windows::LuaGetWindowParam);

	//createWindowElement('container', align, x, y, width, hegiht, backgrounded, bordered, templatePtr) : elementPtr
	//createWindowElement('text', align, x, y, width, templatePtr) : elementPtr
	//createWindowElement('link', align, x, y, width, mousePtr, templatePtr) : elementPtr
	//createWindowElement('image', align, x, y, width, height, imagePtr, templatePtr) : elementPtr
	//createWindowElement('button', align, buttonType, x, y, width, height, templatePtr) : elementPtr
	//createWindowElement('textarea', align, x, y, width, hegiht, editable, multilined, templatePtr) : elementPtr
	//createWindowElement('memo', align, x, y, width, height, templatePtr) : elementPtr
	//createWindowElement('tablememo', align, x, y, width, height, templatePtr) : elementPtr
	//createWindowElement('list', align, x, y, width, height, templatePtr) : elementPtr
	//createWindowElement('timer', align, x, y, width, height, templatePtr) : elementPtr
	//createWindowElement('tab', align, x, y, width, height, backgrounded, bordered, templatePtr) : elementPtr
	//createWindowElement('itemcontainer', align, x, y, width, height, gameContainerPtr, templatePtr) : elementPtr
	//createWindowElement('checkbox', align, x, y, width, templatePtr) : elementPtr
	//createWindowElement('scrollbar', align, x, y, width, templatePtr) : elementPtr
	//createWindowElement('skillbar', align, x, y, width, height, valuePtrType, templatePtr) : elementPtr
	//createWindowElement('colormap', align, x, y, width, height, templatePtr) : elementPtr
	//createWindowElement('cooldown', align, x, y, width, height, templatePtr) : elementPtr
	//createWindowElement('select', align, x, y, width, height, mousePtr, templatePtr) : elementPtr
	lua_register(L, "createWindowElement", Windows::LuaCreateWindowElement);

	//getWindowElement(containerPtr, elementNumber = 0)
	lua_register(L, "getWindowElement", Windows::LuaGetWindowElement);

	//addWindowElement(containerPtr, elementPtr)
	lua_register(L, "addWindowElement", Windows::LuaAddWindowElement);

	//removeWindowElement(containerPtr, elementPtr)
	lua_register(L, "removeWindowElement", Windows::LuaRemoveWindowElement);

	//deleteWindowElement(elementPtr)
	lua_register(L, "deleteWindowElement", Windows::LuaDeleteWindowElement);

	//clearWindowElements(containerPtr)
	lua_register(L, "clearWindowElements", Windows::LuaClearWindowElements);

	//setWindowElementParam(elementPtr, 'position', x, y)
	//setWindowElementParam(elementPtr, 'size', width, height)
	//setWindowElementParam(elementPtr, 'locks', lwidth, lheight)
	//setWindowElementParam(elementPtr, 'enabled', state)
	//setWindowElementParam(elementPtr, 'comment', commentStr)
	//setWindowElementParam(elementPtr, 'action', functionName, arg1, arg2, ...)
	//setWindowElementParam(elementPtr, 'actiondbl', functionName, arg1, arg2, ...)
	//setWindowElementParam(elementPtr, 'actionlook', functionName, arg1, arg2, ...)
	//setWindowElementParam(elementPtr, 'actionuse', functionName, arg1, arg2, ...)
	//setWindowElementParam(elementPtr, 'intsize', width, height, adjust = 0)
	//setWindowElementParam(elementPtr, 'textalign', align)
	//setWindowElementParam(elementPtr, 'text', textStr)
	//setWindowElementParam(elementPtr, 'color', red, green, blue, alpha = 1.0)
	//setWindowElementParam(elementPtr, 'colorin', red, green, blue, alpha = 1.0)
	//setWindowElementParam(elementPtr, 'colorout', red, green, blue, alpha = 1.0)
	//setWindowElementParam(elementPtr, 'background', state)
	//setWindowElementParam(elementPtr, 'border', state)
	//setWindowElementParam(elementPtr, 'fontsize', size)
	//setWindowElementParam(elementPtr, 'precision', precision)
	//setWindowElementParam(elementPtr, 'scroll', state)
	//setWindowElementParam(elementPtr, 'scrollalwaysvisible', horizontalState, verticalState)
	//setWindowElementParam(elementPtr, 'image', imagePtr)
	//setWindowElementParam(elementPtr, 'slot', slot)
	//setWindowElementParam(elementPtr, 'pressed', state)
	//setWindowElementParam(elementPtr, 'inverse', state)
	//setWindowElementParam(elementPtr, 'offset', offset)
	//setWindowElementParam(elementPtr, 'maxlength', maxlength)
	//setWindowElementParam(elementPtr, 'hidden', state)
	//setWindowElementParam(elementPtr, 'active', state)
	//setWindowElementParam(elementPtr, 'option', option, executeOnSet)
	//setWindowElementParam(elementPtr, 'option', option)
	//setWindowElementParam(elementPtr, 'rowheight', height)
	//setWindowElementParam(elementPtr, 'columnwidth', columnNum, width, percent = 0)
	//setWindowElementParam(elementPtr, 'columnname', columnNum, name)
	//setWindowElementParam(elementPtr, 'timestop', timestamp)
	//setWindowElementParam(elementPtr, 'timeinterval', miliseconds)
	//setWindowElementParam(elementPtr, 'state', state)
	//setWindowElementParam(elementPtr, 'valueptr', valuePtrType)
	//setWindowElementParam(elementPtr, 'minvalue', minvalue)
	//setWindowElementParam(elementPtr, 'maxvalue', maxvalue)
	//setWindowElementParam(elementPtr, 'tabsheight', height)
	lua_register(L, "setWindowElementParam", Windows::LuaSetWindowElementParam);

	//getWindowElementParam(elementPtr, 'position') : position{x, y}
	//getWindowElementParam(elementPtr, 'size') : size{x, y}
	//getWindowElementParam(elementPtr, 'comment') : commentStr
	//getWindowElementParam(elementPtr, 'intsize') : intsize{x, y}
	//getWindowElementParam(elementPtr, 'scrolloffset') : scrolloffset{x, y}
	//getWindowElementParam(elementPtr, 'scrolloffset') : scrolloffset
	//getWindowElementParam(elementPtr, 'text') : textStr
	//getWindowElementParam(elementPtr, 'pressed') : state
	//getWindowElementParam(elementPtr, 'inverse') : state
	//getWindowElementParam(elementPtr, 'option') : option
	//getWindowElementParam(elementPtr, 'optionssize') : optionssize
	//getWindowElementParam(elementPtr, 'element') : elementStr
	//getWindowElementParam(elementPtr, 'tabssize') : tabssize
	//getWindowElementParam(elementPtr, 'activetab') : activetab
	//getWindowElementParam(elementPtr, 'slot') : slot
	//getWindowElementParam(elementPtr, 'container') : gameContainerPtr
	//getWindowElementParam(elementPtr, 'state') : state
	//getWindowElementParam(elementPtr, 'valueptr') : valuePtrType
	lua_register(L, "getWindowElementParam", Windows::LuaGetWindowElementParam);

	//addMemoElement(elementPtr, elementStr, active = 1)
	lua_register(L, "addMemoElement", Windows::LuaAddMemoElement);

	//addTableMemoRow(elementPtr)
	lua_register(L, "addTableMemoRow", Windows::LuaAddTableMemoRow);

	//addTableMemoColumn(elementPtr, columnElementPtr)
	lua_register(L, "addTableMemoColumn", Windows::LuaAddTableMemoColumn);

	//addListElement(elementPtr, elementStr, active = 1)
	lua_register(L, "addListElement", Windows::LuaAddListElement);

	//timerRun(elementPtr)
	lua_register(L, "timerRun", Windows::LuaTimerRun);

	//timerStop(elementPtr)
	lua_register(L, "timerStop", Windows::LuaTimerStop);

	//getGUIManager() : guiManagerPtr
	lua_register(L, "getGUIManager", Windows::LuaGetGUIManager);
}


POINT Windows::GetWindowMargin() {
	POINT ret;
	POINT wndSize = GetWindowSize();

	Window* wndStatus = game->GetWindowStatus();
	Window* wndHotkeys = game->GetWindowHotkeys();
	ret.x = (wndStatus ? wndStatus->width : 0);
	ret.y = (wndHotkeys ? wndHotkeys->width : 0);

	return ret;
}

POINT Windows::GetWindowSize() {
	return gfx->GetWindowSize();
}

int Windows::GetWindowWidth(WINDOW_TYPE type) {
	LOCKCLASS lockClass(lockWindows);

	INILoader iniWindow;
	std::string filename;
	if (type == WND_STATUS) filename = std::string("templates/") + Game::options.templatesGroup + std::string("/windows/status.ini");
	else if (type == WND_HOTKEYS) filename = std::string("templates/") + Game::options.templatesGroup + std::string("/windows/hotkeys.ini");
	else if (type == WND_INVENTORY) filename = std::string("templates/") + Game::options.templatesGroup + std::string("/windows/inventory.ini");
	else if (type == WND_STATISTICS) filename = std::string("templates/") + Game::options.templatesGroup + std::string("/windows/statistics.ini");
	else if (type == WND_BATTLE) filename = std::string("templates/") + Game::options.templatesGroup + std::string("/windows/battle.ini");
	else if (type == WND_VIPLIST) filename = std::string("templates/") + Game::options.templatesGroup + std::string("/windows/viplist.ini");
	else if (type == WND_MINIMAP) filename = std::string("templates/") + Game::options.templatesGroup + std::string("/windows/minimap.ini");
	else if (type == WND_CONTAINER) filename = std::string("templates/") + Game::options.templatesGroup + std::string("/windows/container.ini");

	if (!iniWindow.OpenFile(filename))
		return 0;

	std::string templateName = iniWindow.GetValue("TEMPLATE");
	int wndWidth = atoi(iniWindow.GetValue("SIZE",0).c_str());

	WindowTemplate* wndTemplate = GetWindowTemplate(Game::options.templatesGroup, templateName);

	wndWidth += wndTemplate->tempBorder.l_width + wndTemplate->tempBorder.r_width;

	return wndWidth;
}

bool Windows::IsFixedPosWindow(WINDOW_TYPE type) {
	if (type == WND_INVENTORY ||
		type == WND_CONTAINER ||
		type == WND_STATISTICS ||
		type == WND_BATTLE ||
		type == WND_VIPLIST ||
		type == WND_MINIMAP)
			return true;

	return false;
}

void Windows::FitFixedWindows() {
	LOCKCLASS lockClass(lockWindows);

	if (!Game::options.fixedPositions)
		return;

	Window* wndGame = game->GetWindowGame();
	Window* wndConsole = game->GetWindowConsole();

	POINT wndSize = GetWindowSize();
	POINT margin = GetWindowMargin();

	std::list<Window*>::iterator it = wndList.begin();
	for (it; it != wndList.end(); it++) {
		Window* wnd = *it;
		if (wnd && IsFixedPosWindow((WINDOW_TYPE)wnd->wndType)) {
			if (wnd->posX < (wndSize.x - wnd->width) / 2)
				wnd->posX = margin.x;
			else
				wnd->posX = wndSize.x - margin.y - wnd->width;
		}
	}
}

void Windows::FitFixedWindows(Window* wnd, int offset) {
	LOCKCLASS lockClass(lockWindows);

	if (!Game::options.fixedPositions)
		return;

	POINT wndSize = GetWindowSize();

	std::list<Window*> toDelete;

	std::list<Window*>::iterator it = wndList.begin();
	for (it; it != wndList.end(); it++) {
		Window* twnd = *it;
		if (IsFixedPosWindow((WINDOW_TYPE)twnd->wndType) && twnd->posX == wnd->posX && twnd->width == wnd->width &&
			((offset > 0 && twnd->posY >= wnd->posY + wnd->height && twnd->posY - offset < wnd->posY + wnd->height) ||
			(offset < 0 && twnd->posY == wnd->posY + wnd->height)))
		{
			int _offset = offset;
			if (offset > 0) _offset += wnd->posY + wnd->height - twnd->posY;

			FitFixedWindows(twnd, _offset);
			twnd->posY += _offset;
			if (twnd->posY + twnd->height > wndSize.y)
				toDelete.push_back(twnd);
		}
	}

	for (it = toDelete.begin(); it != toDelete.end(); it++) {
		Window* dwnd = *it;
		KillWindow(dwnd);
	}
}

void Windows::FitGameWindow() {
	LOCKCLASS lockClass(lockWindows);

	if (!Game::options.fixedPositions)
		return;

	Window* wndGame = game->GetWindowGame();
	Window* wndConsole = game->GetWindowConsole();

	if (!wndGame || wndGame->GetWindowType() != WND_GAME)
		return;

	POINT wndSize = GetWindowSize();
	POINT margin = GetWindowMargin();

	POINT gameMargin = margin;
	POINT gameMarginOld;

	if (wndGame) {
		gameMarginOld.x = wndGame->posX;
		gameMarginOld.y = wndSize.x - wndGame->posX - wndGame->width;
	}

	std::list<Window*>::iterator it = wndList.begin();
	for (it; it != wndList.end(); it++) {
		Window* wnd = *it;
		if (wnd && IsFixedPosWindow((WINDOW_TYPE)wnd->wndType)) {
			if (wnd->posX == margin.x) {
				if (wnd->posX + wnd->width > gameMargin.x) {
					gameMargin.x = wnd->posX + wnd->width;
				}
			}
			else {
				if (wndSize.x - margin.y - wnd->posX > gameMargin.y) {
					gameMargin.y = wndSize.x - wnd->posX;
				}
			}
		}
	}

	if (wndGame) {
		if (wndGame->height > wndSize.y - 100)
			wndGame->height = wndSize.y - 100;
		wndGame->SetSize(wndSize.x - gameMargin.x - gameMargin.y, wndGame->height, NULL, true);
		wndGame->SetPosition(gameMargin.x, 0, NULL, true);
	}
	if (wndConsole) {
		wndConsole->SetSize(wndSize.x - gameMargin.x - gameMargin.y, (wndGame ? wndSize.y - wndGame->height : wndConsole->height), NULL, true);
		wndConsole->SetPosition(gameMargin.x, (wndGame ? wndGame->height : wndConsole->posY), NULL, true);
	}
}

void Windows::MatchGameWindows() {
	LOCKCLASS lockClass(lockWindows);

	Window* wndGame = game->GetWindowGame();
	Window* wndConsole = game->GetWindowConsole();

	if (wndGame && wndConsole) {
		POINT wndSize = GetWindowSize();

		wndGame->SetMinSize(0, 100);
		wndGame->SetMaxSize(0, wndSize.y - 100);

		wndConsole->SetMinSize(0, 100);
		wndConsole->SetMaxSize(0, wndSize.y - 100);

		wndConsole->posY = wndGame->height;
		wndConsole->SetSize(wndConsole->width, wndSize.y - wndGame->height, NULL, true);

		FitGameWindow();
	}
}

Window* Windows::GetCollidingWindow(int offsetX, int offsetY, Window* wnd) {
	LOCKCLASS lockClass(lockWindows);

	if (!wnd || !IsFixedPosWindow((WINDOW_TYPE)wnd->wndType))
		return NULL;

	int diff = 0;
	Window* ret = NULL;

	std::list<Window*>::iterator it = wndList.begin();
	for (it; it != wndList.end(); it++) {
		Window* twnd = *it;

		if (!twnd || wnd == twnd)
			continue;

		if (IsFixedPosWindow((WINDOW_TYPE)twnd->wndType) && (offsetX == twnd->posX || offsetX + wnd->width == twnd->posX + twnd->width)) {
			int diff_ = (offsetY < twnd->posY ? offsetY + wnd->height - twnd->posY : twnd->posY + twnd->height - offsetY);
			if (diff_ > diff) {
				ret = twnd;
				diff = diff_;
			}
		}
	}

	return ret;
}

POINT Windows::GetFixedOpenPosition(int offset, Window* wnd) {
	LOCKCLASS lockClass(lockWindows);

	if (!wnd || !IsFixedPosWindow((WINDOW_TYPE)wnd->wndType))
		return doPOINT(0, 0);

	POINT margin = GetWindowMargin();
	POINT wndSize = GetWindowSize();

	//wndSize.x - margin.y - wnd->width
	POINT ret = {offset, 0};

	//wnd->posX = ret.x;
	//wnd->posY = ret.y;
	Window* twnd;
	int lastPosY = ret.y;
	while((twnd = GetCollidingWindow(ret.x, ret.y, wnd)) != NULL) {
		ret.y = twnd->posY + twnd->height;
		if (ret.y + wnd->height > wndSize.y) {
			ret.y = lastPosY;
			KillWindow(twnd);
		}
		lastPosY = ret.y;
	}

	return ret;
}


// ---- Window ---- //

Window::Window(unsigned char type, int pX, int pY, WindowTemplate* wTemplate) {
	wndType = type;
	alwaysActive = false;
	minimized = 0;
	posX = pX;
	posY = pY;
	wndTemplate = wTemplate;
	elements = wndTemplate->GetElements();

	defWidth = 0;
	defHeight = 0;
	minWidth = wndTemplate->tempBorder.l_width + wndTemplate->tempBorder.r_width;
	minHeight = wndTemplate->tempHeader.c_height + wndTemplate->tempBorder.t_height + wndTemplate->tempBorder.b_height;
	maxWidth = 0;
	maxHeight = 0;
	width = minWidth;
	height = minHeight;

	int intPosX = wndTemplate->tempBorder.l_width;
	int intPosY = wndTemplate->tempHeader.c_height + wndTemplate->tempBorder.t_height;
	int intWidth = width - wndTemplate->tempBorder.l_width - wndTemplate->tempBorder.r_width;
	int intHeight = height - wndTemplate->tempHeader.c_height - wndTemplate->tempBorder.t_height - wndTemplate->tempBorder.b_height;
	wndContainer.Create(0, intPosX, intPosY, intWidth, intHeight, false, false, wndTemplate);
	wndContainer.SetIntSize(200, 200);
	wndContainer.SetWindow(this);

	lockToBorderWidth = false;
	lockToBorderHeight = false;
	lockResizeWidth = false;
	lockResizeHeight = false;

	closeAbility = true;
	minimizeAbility = true;
	hideAbility = false;

	activeElement = &wndContainer;
	activeElement->active = true;
}

Window::~Window() {
}


void Window::SetWindowType(unsigned char type) {
	this->wndType = type;
}

unsigned char Window::GetWindowType() {
	return wndType;
}

void Window::SetWindowTemplate(WindowTemplate* wndTemplate) {
	this->wndTemplate = wndTemplate;
}

WindowTemplate* Window::GetWindowTemplate() {
	return wndTemplate;
}

WindowElementContainer* Window::GetWindowContainer() {
	return &wndContainer;
}

void Window::SetBackground(bool state) {
	wndContainer.SetContainerBackground(state);
}

void Window::SetElements(unsigned short elements) {
	this->elements = elements;

	int intPosX = (elements & ELEMENT_BORDER ? wndTemplate->tempBorder.l_width : 0);
	int intPosY = (elements & ELEMENT_BORDER ? wndTemplate->tempBorder.t_height : 0) + (elements & ELEMENT_HEADER ? wndTemplate->tempHeader.c_height : 0);
	int intWidth = width - (elements & ELEMENT_BORDER ? wndTemplate->tempBorder.l_width + wndTemplate->tempBorder.r_width : 0);
	int intHeight = height - (elements & ELEMENT_BORDER ? wndTemplate->tempBorder.t_height + wndTemplate->tempBorder.b_height : 0) - (elements & ELEMENT_HEADER ? wndTemplate->tempHeader.c_height : 0);

	minWidth = (elements & ELEMENT_BORDER ? wndTemplate->tempBorder.l_width + wndTemplate->tempBorder.r_width : 0);
	minHeight = (elements & ELEMENT_HEADER ? wndTemplate->tempHeader.c_height : 0) + (elements & ELEMENT_BORDER ? wndTemplate->tempBorder.t_height + wndTemplate->tempBorder.b_height : 0);

	wndContainer.SetPosition(intPosX, intPosY);
	wndContainer.SetSize(intWidth, intHeight);
}

unsigned short Window::GetElements() {
	return elements;
}


void Window::SetCloseAbility(bool state) {
	this->closeAbility = state;
}

bool Window::GetCloseAbility() {
	return closeAbility;
}

void Window::SetMinimizeAbility(bool state) {
	this->minimizeAbility = state;
}

bool Window::GetMinimizeAbility() {
	return minimizeAbility;
}

void Window::SetHideAbility(bool state) {
	this->hideAbility = state;
}

bool Window::GetHideAbility() {
	return hideAbility;
}


bool Window::IsUnderCursor(Mouse& mouse) {
	if (mouse.curX >= posX && mouse.curX < posX + width &&
		mouse.curY >= posY && mouse.curY < posY + height) return true;

	if (elements & ELEMENT_HEADER &&
		elements & ELEMENT_WNDICONS &&
		mouse.curX >= posX + width - wndTemplate->tempWndIcons.x - wndTemplate->tempWndIcons.w && mouse.curX < posX + width - wndTemplate->tempWndIcons.x &&
		mouse.curY >= posY + wndTemplate->tempWndIcons.y && mouse.curY < posY + wndTemplate->tempWndIcons.y + wndTemplate->tempWndIcons.h) return true;

	if (!minimized &&
		elements & ELEMENT_BORDER &&
		mouse.curX >= posX + width - wndTemplate->tempBorder.br_width && mouse.curX < posX + width + wndTemplate->tempBorder.br_x &&
		mouse.curY >= posY + height - wndTemplate->tempBorder.br_height && mouse.curY < posY + height + wndTemplate->tempBorder.br_y) return true;

	if (!minimized &&
		elements & ELEMENT_BORDER &&
		mouse.curX >= posX + wndTemplate->tempBorder.bl_x && mouse.curX < posX + wndTemplate->tempBorder.bl_width &&
		mouse.curY >= posY + height - wndTemplate->tempBorder.bl_height && mouse.curY < posY + height + wndTemplate->tempBorder.bl_y) return true;

	return false;
}

void Window::SetAlwaysActive(bool alwaysActive) {
	this->alwaysActive = alwaysActive;
}

void Window::SetTitle(std::string tl) {
	title = tl;
}

void Window::SetPosition(int pX, int pY, GUIConnection* excludeConnection, bool ignoreConnection) {
	Windows* wnds = Windows::wnds;
	if (!hideAbility) {
		if (wnds) {
			POINT wndSize = wnds->GetWindowSize();
			if (pX + width > wndSize.x) pX = wndSize.x - width;
			if (pY + height > wndSize.y) pY = wndSize.y - height;
		}
		if (pX < 0) pX = 0;
		if (pY < 0) pY = 0;
	}

	/*if (wnds) {
		POINT wndSize = wnds->GetWindowSize();
		if (wndType == WND_GAME) {
			Window* wndStatus = wnds->FindWindow(WND_STATUS);
			if (wndStatus && pX < wndStatus->width) pX = wndStatus->width;
		}
	}*/

	/*ConnectionMap::iterator it = connectedWindows.begin();
	for (it; it != connectedWindows.end(); it++) {
		Window* connected = it->first;

		if (posX == connected->posX + connected->width) pX = posX;
		if (posY == connected->posY + connected->height) pY = posY;

		if (posX == connected->posX) connected->posX = pX;
		if (posY == connected->posY) connected->posY = pY;
	}*/

	int oldPosX = posX;
	int oldPosY = posY;
	posX = pX;
	posY = pY;

	if (wnds && !ignoreConnection) {
		wnds->guiManager.ExecuteOnSetPosition(this);
		wnds->guiManager.OnWindowChangePosition(this, posX - oldPosX, posY - oldPosY, excludeConnection);
	}
}

void Window::SetSize(int wdh, int hgt, GUIConnection* excludeConnection, bool ignoreConnection) {
	bool negative = false;
	if (wdh < 0) { wdh = -wdh; negative = true; }
	if (hgt < 0) { hgt = -hgt; negative = true; }

	int oldWidth = width;
	int oldHeight = height;

	/*if (wndType == WND_GAME) {
		Windows* wnds = Windows::wnds;
		if (wnds) {
			POINT wndSize = wnds->GetWindowSize();
			if (posX + wdh > wndSize.x) wdh = wndSize.x - posX;
			if (posY + hgt > wndSize.y) hgt = wndSize.y - posY;

			int dx = wdh - oldWidth;
			if (!negative) {
				Window* wndHotKeys = wnds->FindWindow(WND_HOTKEYS);
				if (wndHotKeys && posX + wdh > wndSize.x - wndHotKeys->width) wdh = wndSize.x - wndHotKeys->width - posX;
			}
			else {
				Window* wndStatus = wnds->FindWindow(WND_STATUS);
				if (wndStatus && posX - dx < wndStatus->width && wdh > oldWidth) wdh = posX + oldWidth - wndStatus->width;
			}
		}
	}*/

	int _wdh = wdh;
	int _hgt = hgt;
	if (minWidth != 0 && wdh < minWidth) wdh = minWidth;
	if (maxWidth != 0 && wdh > maxWidth) wdh = maxWidth;
	if (minHeight != 0 && hgt < minHeight) hgt = minHeight;
	if (maxHeight != 0 && hgt > maxHeight) hgt = maxHeight;

	Windows* wnds = Windows::wnds;
	if (wnds && wnds->IsFixedPosWindow((WINDOW_TYPE)wndType))
		wnds->FitFixedWindows(this, hgt - oldHeight);

	/*ConnectionMap::iterator it = connectedWindows.begin();
	for (it; it != connectedWindows.end(); it++) {
		Window* connected = it->first;
		if (!wnds || !wnds->IsOnList(connected))
			continue;

		int commonWidth = oldWidth + connected->width;
		int commonHeight = oldHeight + connected->height;

		if (posX + width == connected->posX) {
			connected->width = commonWidth - wdh;
			if (connected->minWidth != 0 && connected->width < connected->minWidth) {
				connected->width = connected->minWidth;
				wdh = commonWidth - connected->minWidth;
			}
			if (connected->maxWidth != 0 && connected->width > connected->maxWidth) {
				connected->width = connected->maxWidth;
				wdh = commonWidth - connected->maxWidth;
			}

			connected->posX = posX + wdh;
		}
		if (posY + height == connected->posY) {
			connected->height = commonHeight - hgt;
			if (connected->minHeight != 0 && connected->height < connected->minHeight) {
				connected->height = connected->minHeight;
				hgt = commonHeight - connected->minHeight;
			}
			if (connected->maxHeight != 0 && connected->height > connected->maxHeight) {
				connected->height = connected->maxHeight;
				hgt = commonHeight - connected->maxHeight;
			}

			connected->posY = posY + hgt;
		}

		if (posX == connected->posX && width == connected->width) connected->width = wdh;
		if (posY == connected->posY && height == connected->height) connected->height = hgt;

		int intWidth = connected->width - (elements & ELEMENT_BORDER ? connected->wndTemplate->tempBorder.l_width + connected->wndTemplate->tempBorder.r_width : 0);
		int intHeight = connected->height - (elements & ELEMENT_BORDER ? connected->wndTemplate->tempBorder.t_height + connected->wndTemplate->tempBorder.b_height : 0) - (elements & ELEMENT_HEADER ? connected->wndTemplate->tempHeader.c_height : 0);

		connected->wndContainer.SetSize(intWidth, intHeight);
	}*/

	width = wdh;
	height = hgt;

	int intWidth = width - (elements & ELEMENT_BORDER ? wndTemplate->tempBorder.l_width + wndTemplate->tempBorder.r_width : 0);
	int intHeight = height - (elements & ELEMENT_BORDER ? wndTemplate->tempBorder.t_height + wndTemplate->tempBorder.b_height : 0) - (elements & ELEMENT_HEADER ? wndTemplate->tempHeader.c_height : 0);

	wndContainer.SetSize(intWidth, intHeight);

	if (wnds && !ignoreConnection) {
		wnds->guiManager.ExecuteOnSetSize(this);
		wnds->guiManager.OnWindowChangeSize(this, (width - oldWidth) * (negative ? -1 : 1), (height - oldHeight) * (negative ? -1 : 1), negative, excludeConnection);
	}
}

void Window::SetMinSize(int wdh, int hgt) {
	minWidth = wdh;
	minHeight = hgt;
}

void Window::SetMaxSize(int wdh, int hgt) {
	maxWidth = wdh;
	maxHeight = hgt;
}

void Window::SetIntSize(unsigned short wdh, unsigned short hgt) {
	wndContainer.SetIntSize(wdh, hgt);
}

void Window::SetLockToBorder(bool lockWidth, bool lockHeight) {
	this->lockToBorderWidth = lockWidth;
	this->lockToBorderHeight = lockHeight;
}

void Window::SetScrollAlwaysVisible(bool horizontal, bool vertical) {
	wndContainer.SetScrollAlwaysVisible(horizontal, vertical);
}

POINT Window::GetPosition() {
	POINT pos;
	pos.x = posX;
	pos.y = posY;

	return pos;
}

POINT Window::GetSize(bool internal) {
	POINT size;
	if (internal) {
		size.x = wndContainer.width;
		size.y = wndContainer.height;
	}
	else {
		size.x = width;
		size.y = height;
	}

	return size;
}

void Window::AdjustSize() {
	if (wndContainer.intWidth == 0 && wndContainer.intHeight == 0)
		return;

	int hScroll = 0;
	int vScroll = 0;
	if (wndContainer.scroll && wndTemplate->GetElements() & ELEMENT_SCROLL) {
		hScroll = (wndContainer.hAlwaysVisible ? wndTemplate->tempScroll.h : 0);
		vScroll = (wndContainer.vAlwaysVisible ? wndTemplate->tempScroll.w : 0);
	}

	int extWidth = wndContainer.intWidth + (elements & ELEMENT_BORDER ? wndTemplate->tempBorder.l_width + wndTemplate->tempBorder.r_width : 0) + vScroll;
	int extHeight = wndContainer.intHeight + (elements & ELEMENT_BORDER ? wndTemplate->tempBorder.t_height + wndTemplate->tempBorder.b_height : 0) + (elements & ELEMENT_HEADER ? wndTemplate->tempHeader.c_height : 0) + hScroll;

	SetSize(extWidth, extHeight);
}

void Window::AddExecuteFunction(boost::function<void()> f) {
	if (!f) {
		func.Clear();
		return;
	}

	func.PushFunction(f);
}

void Window::AddActivateFunction(boost::function<void()> f) {
	if (!f) {
		activateFunc.Clear();
		return;
	}

	activateFunc.PushFunction(f);
}

void Window::AddCloseFunction(boost::function<void()> f) {
	if (!f) {
		closeFunc.Clear();
		return;
	}

	closeFunc.PushFunction(f);
}

void Window::AddMinimizeFunction(boost::function<void()> f) {
	if (!f) {
		minimizeFunc.Clear();
		return;
	}

	minimizeFunc.PushFunction(f);
}

void Window::SetAction(unsigned char action) {
	actions.push_back(action);

	PostMessage(Windows::hWnd, WM_CHAR, 0, 0);
}

void Window::ConnectToWindow(Window* wnd, bool connectWidth, bool connectHeight) {
	if (!wnd) {
		Logger::AddLog("Window::ConnectToWindow()", "Window pointer is NULL!", LOG_WARNING);
		return;
	}

	this->connectedWindows[wnd] = std::pair<bool, bool>(connectWidth, connectHeight);
}

void Window::DisconnectWindow(Window* wnd) {
	if (!wnd) {
		Logger::AddLog("Window::ConnectToWindow()", "Window pointer is NULL!", LOG_WARNING);
		return;
	}

	ConnectionMap::iterator it = connectedWindows.find(wnd);
	if (it != connectedWindows.end())
		connectedWindows.erase(it);
}

bool Window::IsConnectedToWindow(Window* wnd) {
	ConnectionMap::iterator it = connectedWindows.find(wnd);
	if (it != connectedWindows.end())
		return true;

	return false;
}

void Window::SetActiveElement(WindowElement* element) {
	if (activeElement && activeElement->alwaysActive)
		return;

	if (activeElement) {
		activeElement->active = false;
		if (activeElement->funcOnDeactivate.IsExecutable())
			activeElement->funcOnDeactivate.Execute();
	}

	activeElement = element;

	if (activeElement) {
		activeElement->active = true;
		if (activeElement->funcOnActivate.IsExecutable())
			activeElement->funcOnActivate.Execute();
	}
}

void Window::SkipActiveElement() {
	if (!activeElement || activeElement->alwaysActive)
		return;

	WindowElementContainer* parent = activeElement->parent;

	if (!parent)
		parent = dynamic_cast<WindowElementContainer*>(activeElement);
	if (!parent)
		return;
	if (parent->elements.begin() == parent->elements.end())
		return;

	std::list<WindowElement*>::iterator it = std::find(parent->elements.begin(), parent->elements.end(), activeElement);
	if (it == parent->elements.end())
		it = parent->elements.begin();
	else {
		std::list<WindowElement*>::iterator it_this = it;
		bool skip = false;
		while(!skip) {
			it++;
			if (it == parent->elements.end())
				it = parent->elements.begin();

			WindowElement* element = *it;
			if ((element->enabled && element->width != 0 && element->height != 0) || it == it_this)
				skip = true;
		}
	}

	activeElement->active = false;
	if (activeElement->funcOnDeactivate.IsExecutable())
		activeElement->funcOnDeactivate.Execute();
	activeElement = *it;
	activeElement->active = true;
	if (activeElement->funcOnActivate.IsExecutable())
		activeElement->funcOnActivate.Execute();
}

void Window::AddElement(WindowElement* wndElement) {
	wndContainer.AddElement(wndElement);
	if (wndElement)
		wndElement->SetWindow(this);
}

WindowElement* Window::GetWindowElementUnderCursor(RECT& rect, WindowElementContainer* container, int x, int y) {
	WindowElement* element = NULL;

	if (container) {
		int leftWidth = (container->border ? container->wndTemplate->tempContainer.l_width : 0);
		int rightWidth = (container->border ? container->wndTemplate->tempContainer.r_width : 0);
		int topHeight = (container->border ? container->wndTemplate->tempContainer.t_height : 0);
		int bottomHeight = (container->border ? container->wndTemplate->tempContainer.b_height : 0);

		POINT scroll = container->GetParentScrollOffset();
		int scrollH = scroll.x;
		int scrollV = scroll.y;

		int tX = rect.left + container->posX - scrollH;
		int tY = rect.top + container->posY - scrollV;
		int bX = tX + container->width - 1;
		int bY = tY + container->height - 1;

		if (tX < rect.left) tX = rect.left;
		if (tY < rect.top) tY = rect.top;
		if (bX > rect.right) bX = rect.right;
		if (bY > rect.bottom) bY = rect.bottom;

		if (tX > rect.right) tX = rect.right;
		if (tY > rect.bottom) tY = rect.bottom;
		if (bX < rect.left) bX = rect.left;
		if (bY < rect.top) bY = rect.top;

		if (x >= tX && x <= bX &&
			y >= tY && y <= bY)
		{
			element = container;

			POINT offset = container->GetScrollOffset();
			if (container->scroll && container->wndTemplate->GetElements() & ELEMENT_SCROLL && (
				((offset.y || container->vAlwaysVisible) && x >= tX + container->width - leftWidth - rightWidth - container->wndTemplate->tempScroll.w && x < tX + container->width - leftWidth - rightWidth) ||
				((offset.x || container->hAlwaysVisible) && y >= tY + container->height - topHeight - bottomHeight - container->wndTemplate->tempScroll.h && y < tY + container->height - topHeight - bottomHeight)))
					return element;

			tX += leftWidth;
			tY += topHeight;
			bX -= rightWidth;
			bY -= bottomHeight;

			std::list<WindowElement*>::iterator it = container->elements.begin();
			for (it; it != container->elements.end(); it++) {
				WindowElement* _element = *it;
				if (_element) {
					if (x >= tX + _element->posX - (int)container->scrollH &&
						x <= tX + _element->posX - (int)container->scrollH + _element->width - 1 &&
						y >= tY + _element->posY - (int)container->scrollV &&
						y <= tY + _element->posY - (int)container->scrollV + _element->height - 1)
					{
						element = _element;

						WindowElementContainer* _container = dynamic_cast<WindowElementContainer*>(_element);
						if (_container) {
							RECT _rect = {tX, tY, bX, bY};
							WindowElement* el = GetWindowElementUnderCursor(_rect, _container, x, y);
							if (el)
								element = el;
						}
					}
				}
			}
		}
	}

	return element;
}

WindowElement* Window::GetWindowElementUnderCursor(Mouse& mouse) {
	RECT rect = { posX, posY, posX + width - 1, posY + height - 1 };
	return GetWindowElementUnderCursor(rect, &wndContainer, mouse.curX, mouse.curY);
}

WindowElement* Window::GetActiveElement() {
	return activeElement;
}

void Window::ExecuteFunction() {
	func.Execute();
}

void Window::ActivateFunction() {
	activateFunc.Execute();
}

void Window::CloseFunction() {
	closeFunc.Execute();
}

void Window::MinimizeFunction() {
	minimizeFunc.Execute();
}

void Window::Print(AD2D_Window& gfx, bool active) {
	if (!wndTemplate) {
		Logger::AddLog("Window::Print()", "No template!", LOG_WARNING);
		return;
	}

	COLOR color;
	if (!active && !alwaysActive && Game::options.fadeOutInactive)
		color = COLOR(0.5f, 0.5f, 0.5f, 1.0f);
	else
		color = COLOR(1.0f, 1.0f, 1.0f, 1.0f);

	AD2D_Window::SetColor(color.red, color.green, color.blue, color.alpha);

	if (!minimized && (wndTemplate->GetElements() & ELEMENT_BORDER) && (elements & ELEMENT_BORDER)) {
		int header = (elements & ELEMENT_HEADER ? wndTemplate->tempHeader.c_height : 0);

		int x = 0;
		int y = 0;

		int borderH = wndTemplate->tempBorder.l_width + wndTemplate->tempBorder.r_width;
		int borderV = header + wndTemplate->tempBorder.t_height + wndTemplate->tempBorder.b_height;

		AD2D_Viewport borderVP;
		borderVP.Create(posX + wndTemplate->tempBorder.l_width,
						posY + header + wndTemplate->tempBorder.t_height,
						posX + width - wndTemplate->tempBorder.r_width - 1,
						posY + height - wndTemplate->tempBorder.b_height - 1);
		gfx.SetViewport(borderVP);
		if (wndTemplate->tempBorder.repeatBG) {
			if (wndTemplate->tempBorder.bg.GetWidth() > 0) {
				for (x = 0; x < width; x += wndTemplate->tempBorder.bg.GetWidth())
				for (y = 0; y < height - header; y += wndTemplate->tempBorder.bg.GetHeight())
					gfx.PutImage(x, y, wndTemplate->tempBorder.bg);
			}
		}
		else {
			gfx.PutImage(0, 0, width - borderH + 1, height - borderV + 1, wndTemplate->tempBorder.bg);
		}

		borderVP.Create(posX + wndTemplate->tempBorder.tl_width,
						posY + header + wndTemplate->tempBorder.t_y,
						posX + width - wndTemplate->tempBorder.tr_width - 1,
						posY + header + wndTemplate->tempBorder.t.GetHeight() - 1);
		gfx.SetViewport(borderVP);
		if (wndTemplate->tempBorder.t.GetWidth() > 0) {
			for (x = 0; x < width; x += wndTemplate->tempBorder.t.GetWidth())
				gfx.PutImage(x, 0, wndTemplate->tempBorder.t);
		}

		borderVP.Create(posX + wndTemplate->tempBorder.l_x,
						posY + header + wndTemplate->tempBorder.tl_height,
						posX + wndTemplate->tempBorder.l.GetWidth() - 1,
						posY + height - wndTemplate->tempBorder.bl_height - 1);
		gfx.SetViewport(borderVP);
		if (wndTemplate->tempBorder.l.GetWidth() > 0) {
			for (y = 0; y < height - header; y += wndTemplate->tempBorder.l.GetHeight())
				gfx.PutImage(0, y, wndTemplate->tempBorder.l);
		}

		borderVP.Create(posX + width - wndTemplate->tempBorder.r.GetWidth() + wndTemplate->tempBorder.r_x,
						posY + header + wndTemplate->tempBorder.tr_height,
						posX + width + wndTemplate->tempBorder.r_x - 1,
						posY + height - wndTemplate->tempBorder.br_height - 1);
		gfx.SetViewport(borderVP);
		if (wndTemplate->tempBorder.r.GetWidth() > 0) {
			for (y = 0; y < height - header; y += wndTemplate->tempBorder.r.GetHeight())
				gfx.PutImage(0, y, wndTemplate->tempBorder.r);
		}

		borderVP.Create(posX + wndTemplate->tempBorder.bl_width,
						posY + height - wndTemplate->tempBorder.b.GetHeight(),
						posX + width - wndTemplate->tempBorder.br_width - 1,
						posY + height - 1);
		gfx.SetViewport(borderVP);
		if (wndTemplate->tempBorder.b.GetWidth() > 0) {
			for (x = 0; x < width; x += wndTemplate->tempBorder.b.GetWidth())
				gfx.PutImage(x, 0, wndTemplate->tempBorder.b);
		}

		borderVP.Create(posX + wndTemplate->tempBorder.tl_x,
						posY + header + wndTemplate->tempBorder.tl_y,
						posX + wndTemplate->tempBorder.tl_x + wndTemplate->tempBorder.tl.GetWidth() - 1,
						posY + header + wndTemplate->tempBorder.tl_y + wndTemplate->tempBorder.tl.GetHeight() - 1);
		gfx.SetViewport(borderVP);
		gfx.PutImage(0, 0, wndTemplate->tempBorder.tl);

		borderVP.Create(posX + width - wndTemplate->tempBorder.tr.GetWidth() + wndTemplate->tempBorder.tr_x,
						posY + header + wndTemplate->tempBorder.tr_y,
						posX + width + wndTemplate->tempBorder.tr_x - 1,
						posY + header + wndTemplate->tempBorder.tr_y + wndTemplate->tempBorder.tr.GetHeight() - 1);
		gfx.SetViewport(borderVP);
		gfx.PutImage(0, 0, wndTemplate->tempBorder.tr);

		borderVP.Create(posX + wndTemplate->tempBorder.bl_x,
						posY + height - wndTemplate->tempBorder.bl.GetHeight() + wndTemplate->tempBorder.bl_y,
						posX + wndTemplate->tempBorder.bl_x + wndTemplate->tempBorder.bl.GetWidth() - 1,
						posY + height + wndTemplate->tempBorder.bl_y - 1);
		gfx.SetViewport(borderVP);
		gfx.PutImage(0, 0, wndTemplate->tempBorder.bl);

		borderVP.Create(posX + width - wndTemplate->tempBorder.br.GetWidth() + wndTemplate->tempBorder.br_x,
						posY + height - wndTemplate->tempBorder.br.GetHeight() + wndTemplate->tempBorder.br_y,
						posX + width + wndTemplate->tempBorder.br_x - 1,
						posY + height + wndTemplate->tempBorder.br_y - 1);
		gfx.SetViewport(borderVP);
		gfx.PutImage(0, 0, wndTemplate->tempBorder.br);
	}

	if ((wndTemplate->GetElements() & ELEMENT_HEADER) && (elements & ELEMENT_HEADER)) {
		AD2D_Viewport headerVP;

		int x = 0;

		headerVP.Create(posX + wndTemplate->tempHeader.l_x,
						posY + wndTemplate->tempHeader.l_y,
						posX + wndTemplate->tempHeader.l.GetWidth() - 1,
						posY + wndTemplate->tempHeader.l.GetHeight() - 1);
		gfx.SetViewport(headerVP);
		gfx.PutImage(0, 0, wndTemplate->tempHeader.l);

		headerVP.Create(posX + wndTemplate->tempHeader.l_width,
						posY + wndTemplate->tempHeader.c_y,
						posX + width - wndTemplate->tempHeader.r_width - 1,
						posY + wndTemplate->tempHeader.c_y + wndTemplate->tempHeader.c.GetHeight() - 1);
		gfx.SetViewport(headerVP);
		if (wndTemplate->tempHeader.c.GetWidth() > 0) {
			for (x = 0; x < width; x += wndTemplate->tempHeader.c.GetWidth())
				gfx.PutImage(x, 0, wndTemplate->tempHeader.c);
		}

		headerVP.Create(posX + width - wndTemplate->tempHeader.r_width,
						posY + wndTemplate->tempHeader.r_y,
						posX + width + wndTemplate->tempHeader.r_x - 1,
						posY + wndTemplate->tempHeader.r_y + wndTemplate->tempHeader.r.GetHeight() - 1);
		gfx.SetViewport(headerVP);
		gfx.PutImage(0, 0, wndTemplate->tempHeader.r);

		headerVP.Create(posX, posY, posX + width - 1, posY + wndTemplate->tempHeader.c_height - 1);
		gfx.SetViewport(headerVP);

		int textOffsetX = wndTemplate->tempHeader.t_offsetx;
		int textOffsetY = wndTemplate->tempHeader.t_offsety;

		ContainerWindow* cntWindow = dynamic_cast<ContainerWindow*>(this);
		Container* container = (cntWindow ? cntWindow->GetContainer() : NULL);
		if (container) {
			float iwidth = 20.0f;
			float iheight = 20.0f;
			float pwidth = iwidth;
			float pheight = iheight;

			Item item;
			item.SetID(container->GetID());

			if (item()) {
				int div = (item()->m_height > item()->m_width ? item()->m_height : item()->m_width);
				float factor = (div > 1 ? (float)(32 * div) / item()->m_skip : 1.0f);
				iwidth /= factor;
				iheight /= factor;
				pwidth = (pwidth / div);
				pheight = (pheight / div);
			}

			item.PrintItem(&gfx, Position(0, 0, 0), (float)textOffsetX + (iwidth - pwidth), (float)textOffsetY + (float)(wndTemplate->tempHeader.c_height - iheight) / 2 + (iheight - pheight), pwidth, pheight, 0);

			textOffsetX += (int)iwidth + 2;
		}

		int fontSize = 14;
		int fontOffset = -7;
		gfx.Print(textOffsetX, textOffsetY + (wndTemplate->tempHeader.c_height - fontSize) / 2, fontSize, *wndTemplate->font, title.c_str());

		if (wndTemplate->GetElements() & ELEMENT_WNDICONS) {
			ContainerWindow* cntWindow = dynamic_cast<ContainerWindow*>(this);
			Container* container = (cntWindow ? cntWindow->GetContainer() : NULL);

			int size =	wndTemplate->tempWndIcons.w * 2 + (container && container->IsChild() ? wndTemplate->tempWndIcons.w : 0);

			headerVP.Create(posX + width - wndTemplate->tempWndIcons.x - size,
							posY + wndTemplate->tempWndIcons.y,
							posX + width - wndTemplate->tempWndIcons.x - 1,
							posY + wndTemplate->tempWndIcons.y + wndTemplate->tempWndIcons.h - 1);
			gfx.SetViewport(headerVP);

			int offset = 0;
			if (closeAbility)
				AD2D_Window::SetColor(color.red, color.green, color.blue, color.alpha);
			else
				AD2D_Window::SetColor(color.red, color.green, color.blue, color.alpha * 0.5f);

			gfx.PutImage(size - offset - wndTemplate->tempWndIcons.w, 0, wndTemplate->tempWndIcons.close);
			offset += wndTemplate->tempWndIcons.w;

			if (minimizeAbility)
				AD2D_Window::SetColor(color.red, color.green, color.blue, color.alpha);
			else
				AD2D_Window::SetColor(color.red, color.green, color.blue, color.alpha * 0.5f);

			gfx.PutImage(size - offset - wndTemplate->tempWndIcons.w, 0, wndTemplate->tempWndIcons.minimize);
			offset += wndTemplate->tempWndIcons.w;

			AD2D_Window::SetColor(color.red, color.green, color.blue, color.alpha);
			if (container && container->IsChild()) {
				gfx.PutImage(size - offset - wndTemplate->tempWndIcons.w, 0, wndTemplate->tempWndIcons.moveup);
				offset += wndTemplate->tempWndIcons.w;
			}
		}
	}

	if (!minimized) {
		AD2D_Viewport windowVP;
		windowVP.Create(posX, posY, posX + width - 1, posY + height - 1);
		gfx.SetViewport(windowVP);
		POINT diff = {0, 0};
		wndContainer.Print(diff, gfx);
	}
}


// ---- ContainerWindow ---- //

std::map<unsigned char, ContainerWindow*> ContainerWindow::containers;

MUTEX ContainerWindow::lockContainerWindow;


ContainerWindow::ContainerWindow(unsigned char type, int pX, int pY, Container* container, WindowTemplate* wTemplate) : Window(type, pX, pY, wTemplate) {
	this->container = container;
}

ContainerWindow::~ContainerWindow() {
	if (container) {
		unsigned char index = container->GetIndex();
		RemoveContainerWindow(index);
	}
}

Container* ContainerWindow::GetContainer() {
	LOCKCLASS lockClass(lockContainerWindow);

	return container;
}

unsigned char ContainerWindow::GetIndex() {
	LOCKCLASS lockClass(lockContainerWindow);

	if (!container) {
		Logger::AddLog("ContainerWindow::GetIndex()", "Pointer to container is NULL!", LOG_ERROR);
		return 0;
	}

	return container->GetIndex();
}

void ContainerWindow::AddContainerWindow(ContainerWindow* containerWindow) {
	LOCKCLASS lockClass(lockContainerWindow);

	if (!containerWindow) {
		Logger::AddLog("ContainerWindow::AddContainerWindow()", "Pointer to containerWindow is NULL!", LOG_ERROR);
		return;
	}

	std::map<unsigned char, ContainerWindow*>::iterator it = containers.find(containerWindow->GetIndex());
	if (it != containers.end()) {
		Logger::AddLog("ContainerWindow::AddContainerWindow()", "ContainerWindow with given index already exist!", LOG_WARNING);
		RemoveContainerWindow(containerWindow->GetIndex());
	}

	Container::AddContainer(containerWindow->GetContainer());
	containers[containerWindow->GetIndex()] = containerWindow;
}

ContainerWindow* ContainerWindow::GetContainerWindow(unsigned char index) {
	LOCKCLASS lockClass(lockContainerWindow);

	std::map<unsigned char, ContainerWindow*>::iterator it = containers.find(index);
	if (it == containers.end())
		return NULL;

	return it->second;
}

void ContainerWindow::RemoveContainerWindow(unsigned char index) {
	LOCKCLASS lockClass(lockContainerWindow);

	std::map<unsigned char, ContainerWindow*>::iterator it = containers.find(index);
	if (it == containers.end()) {
		Logger::AddLog("ContainerWindow::RemoveContainerWindow()", "ContainerWindow with given index not exist!", LOG_ERROR);
		return;
	}

	Container::RemoveContainer(index);
	containers.erase(it);
}
