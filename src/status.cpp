/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#include "status.h"

#include "allocator.h"
#include "icons.h"
#include "window.h"


// ---- Status ---- //

MUTEX Status::lockStatus;


Status::Status() {
	icons = 0;
	container = NULL;
}

Status::~Status() { }

void Status::SetIcons(unsigned short icons) {
	this->icons = icons;
}

unsigned short Status::GetIcons() {
	return icons;
}

void Status::SetPeriod(unsigned char id, unsigned int period) {
	LOCKCLASS lockClass(lockStatus);

	time_lt now = RealTime::getTime();

	std::map<unsigned char, StatusTime>::iterator it = times.find(id);
	if (it != times.end()) {
		StatusTime stime = it->second;
		if (abs((long)(stime.first + stime.second) - (long)(now + period)) > 1000)
			times[id] = StatusTime(now, period);
	}
	else
		times[id] = StatusTime(now, period);
}


void Status::SetContainer(WindowElementContainer* container) {
	LOCKCLASS lockClass(lockStatus);

	this->container = container;
}

void Status::UpdateContainer() {
	LOCKCLASS lockClass1(Windows::lockWindows);
	LOCKCLASS lockClass2(lockStatus);

	if (!container)
		return;

	container->DeleteAllElements();

	Window* window = container->GetWindow();
	WindowTemplate* wndTemplate = window->GetWindowTemplate();
	window->SetActiveElement(NULL);

	POINT size_int = window->GetWindowContainer()->GetIntSize();
	POINT size_ext = container->GetIntSize();

	int num = 0;
	for (int i = 0; i < 15; i++) {
		if (icons & (1 << i)) {
			StatusTime stime(0, 0);
			std::map<unsigned char, StatusTime>::iterator it = times.find(i + 1);
			if (it != times.end())
				stime = it->second;

			AD2D_Image* image = Icons::GetStatusIcon(i + 1);

			WindowElementCooldown* wstatus = new(M_PLACE) WindowElementCooldown;
			wstatus->Create(0, 0, num * 32, 32, 32, wndTemplate);
			wstatus->SetCast(stime.first, stime.second);
			wstatus->SetIcon(image);

			container->AddElement(wstatus);
			num++;
		}
	}

	container->SetPosition(0, size_int.y - num * 32);
	container->SetSize(32, num * 32);
}
