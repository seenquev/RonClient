/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#ifndef __CRAFTBOX_H_
#define __CRAFTBOX_H_

#include <map>
#include <string>

#include "mthread.h"


class WindowElementContainer;
class WindowElementItemContainer;


typedef std::pair<unsigned short, unsigned char> CraftElement;
typedef std::list<CraftElement> CraftList;
typedef std::map<CraftElement, CraftList> CraftMap;


class CraftBox {
private:
	WindowElementContainer* container;

	CraftMap	crafts;

	static MUTEX	lockCraftBox;

public:
	CraftBox();
	~CraftBox();

	void AddCraftElement(CraftElement craftElement, CraftList craftList);
	void ClearCraftBox();

	CraftMap GetCraftElements();

	void SetContainer(WindowElementContainer* container);
	WindowElementContainer* GetContainer();

	void UpdateContainer(WindowElementItemContainer* itemContainer);
};


#endif //__CRAFTBOX_H_
