/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#include "trade.h"

#include "allocator.h"
#include "window.h"


// ---- Trade ---- //

Trade::Trade() {
	tradeP1 = NULL;
	tradeP2 = NULL;
}

Trade::~Trade() { }

void Trade::SetContainerP1(Container* container) {
	LOCKCLASS lockClass(lockTrade);

	tradeP1 = container;
}

Container* Trade::GetContainerP1() {
	LOCKCLASS lockClass(lockTrade);

	return tradeP1;
}

void Trade::SetContainerP2(Container* container) {
	LOCKCLASS lockClass(lockTrade);

	tradeP2 = container;
}

Container* Trade::GetContainerP2() {
	LOCKCLASS lockClass(lockTrade);

	return tradeP2;
}


void Trade::SetButton(WindowElementButton* button) {
	LOCKCLASS lockClass(lockTrade);

	this->button = button;
}

void Trade::SetButtonVisibility(bool state) {
	LOCKCLASS lockClass1(Windows::lockWindows);
	LOCKCLASS lockClass2(lockTrade);

	if (button)
		button->SetVisible(state);
}

void Trade::SetContainer(WindowElementContainer* container) {
	LOCKCLASS lockClass(lockTrade);

	this->container = container;
}

void Trade::UpdateContainer() {
	LOCKCLASS lockClass1(Windows::lockWindows);
	LOCKCLASS lockClass2(lockTrade);

	if (!container)
		return;

	container->DeleteAllElements();

	Window* window = container->GetWindow();
	WindowTemplate* wndTemplate = window->GetWindowTemplate();
	window->SetActiveElement(NULL);

	POINT size_ext = container->GetIntSize();

	if (tradeP1 && tradeP2)
		SetButtonVisibility(true);

	WindowElementContainer* cont = new(M_PLACE) WindowElementContainer;
	cont->Create(0, 0, 28, size_ext.x, size_ext.y - 28, true, true, wndTemplate);
	cont->SetScroll(true);
	cont->SetScrollAlwaysVisible(false, true);
	cont->SetLocks(false, true);
	POINT size = cont->GetIntSize();

	int height = 0;
	if (tradeP1) {
		WindowElementText* text = new(M_PLACE) WindowElementText;
		text->Create(0, 5, 0, 0xFFFF, wndTemplate);
		text->SetText(tradeP1->GetName());
		text->SetBorder(1);
		text->SetColor(0.8f, 0.8f, 0.8f);
		container->AddElement(text);

		int places = tradeP1->GetPlaces();
		for (int i = 0; i < places; i++) {
			Item* item = tradeP1->GetItem(i);

			int posX = (i % 2) * 32;
			int posY = (i / 2) * 32;
			WindowElementItemContainer* itemCont = new(M_PLACE) WindowElementItemContainer;
			itemCont->Create(0, 5 + posX, 5 + posY, 32, 32, tradeP1, wndTemplate);
			itemCont->SetSlot(i);

			cont->AddElement(itemCont);
		}

		int _height = 10 + (places / 2) * 32 + (places % 2 > 0 ? 32 : 0);
		if (height <_height)
			height = _height;
	}

	if (tradeP2) {
		int offset = wndTemplate->GetFont()->GetTextWidth(tradeP2->GetName(), 14) + 5;

		WindowElementText* text = new(M_PLACE) WindowElementText;
		text->Create(0, size_ext.x - offset, 14, 0xFFFF, wndTemplate);
		text->SetText(tradeP2->GetName());
		text->SetBorder(1);
		text->SetColor(0.8f, 0.8f, 0.8f);
		container->AddElement(text);

		int places = tradeP2->GetPlaces();
		for (int i = 0; i < places; i++) {
			Item* item = tradeP2->GetItem(i);

			int posX = (i % 2) * 32;
			int posY = (i / 2) * 32;
			WindowElementItemContainer* itemCont = new(M_PLACE) WindowElementItemContainer;
			itemCont->Create(0, size.x - 69 + posX, 5 + posY, 32, 32, tradeP2, wndTemplate);
			itemCont->SetSlot(i);

			cont->AddElement(itemCont);
		}

		int _height = 10 + (places / 2) * 32 + (places % 2 > 0 ? 32 : 0);
		if (height <_height)
			height = _height;
	}

	cont->SetIntSize(0, height);

	container->AddElement(cont);
}
