/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#ifndef __TRADE_H_
#define __TRADE_H_

#include "container.h"
#include "mthread.h"


class WindowElementContainer;
class WindowElementButton;


class Trade {
private:
	WindowElementContainer* container;
	WindowElementButton* button;
	
	Container*	tradeP1;
	Container*	tradeP2;
	
	MUTEX lockTrade;
	
public:
	Trade();
	~Trade();
	
	void SetContainerP1(Container* container);
	Container* GetContainerP1();
	void SetContainerP2(Container* container);
	Container* GetContainerP2();
	
	void SetButton(WindowElementButton* button);
	void SetButtonVisibility(bool state);
	
	void SetContainer(WindowElementContainer* container);
	void UpdateContainer();
};

#endif //__TRADE_H_
