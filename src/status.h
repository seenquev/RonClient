/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#ifndef __STATUS_H_
#define __STATUS_H_

#include <map>

#include "mthread.h"
#include "realtime.h"


class WindowElementContainer;


typedef std::pair<time_lt, unsigned int> StatusTime;

class Status {
private:
	WindowElementContainer*	container;
	
	unsigned short	icons;
	std::map<unsigned char, StatusTime> times;
	
	static MUTEX	lockStatus;
	
public:
	Status();
	~Status();
	
	void SetIcons(unsigned short icons);
	unsigned short GetIcons();
	
	void SetPeriod(unsigned char id, unsigned int period);
	
	void SetContainer(WindowElementContainer* container);
	void UpdateContainer();
};


#endif //__STATUS_H_
