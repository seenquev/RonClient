/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#include "craftbox.h"

#include "allocator.h"
#include "window.h"


// ---- CraftBox ---- //

MUTEX	CraftBox::lockCraftBox;


CraftBox::CraftBox() {
	container = NULL;
}

CraftBox::~CraftBox() { }


void CraftBox::AddCraftElement(CraftElement craftElement, CraftList craftList) {
	LOCKCLASS lockClass(lockCraftBox);

	crafts[craftElement] = craftList;
}

void CraftBox::ClearCraftBox() {
	LOCKCLASS lockClass(lockCraftBox);

	crafts.clear();
}

CraftMap CraftBox::GetCraftElements() {
	LOCKCLASS lockClass(lockCraftBox);

	return crafts;
}


void CraftBox::SetContainer(WindowElementContainer* container) {
	LOCKCLASS lockClass(lockCraftBox);

	this->container = container;
}

WindowElementContainer* CraftBox::GetContainer() {
	LOCKCLASS lockClass(lockCraftBox);

	return container;
}


void CraftBox::UpdateContainer(WindowElementItemContainer* itemContainer) {
	LOCKCLASS lockClass1(Windows::lockWindows);
	LOCKCLASS lockClass2(lockCraftBox);

	if (!container)
		return;

	container->DeleteAllElements();

	if (!itemContainer)
		return;

	Window* window = container->GetWindow();
	WindowTemplate* wndTemplate = window->GetWindowTemplate();

	Item* item = itemContainer->GetItem();
	if (!item)
		return;

	CraftElement craftElement(item->GetID(), item->GetCount());
	CraftMap::iterator it = crafts.find(craftElement);
	if (it == crafts.end())
		return;

	CraftList craftList = it->second;
	int count = 0;
	for (CraftList::iterator lit = craftList.begin(); lit != craftList.end(); lit++) {
		Item* item = new(M_PLACE) Item;
		item->SetID(lit->first);
		item->SetCount(lit->second);

		WindowElementItemContainer* itemCont = new(M_PLACE) WindowElementItemContainer;
		itemCont->Create(0, count * 32, 0, 32, 32, NULL, wndTemplate);
		itemCont->SetItem(item);
		container->AddElement(itemCont);

		count++;
	}
}
