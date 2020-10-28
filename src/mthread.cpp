/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#include "mthread.h"

#include <windows.h>

#include "allocator.h"
#include "tools.h"


// ---- Signal ---- //


Signal::Signal() { }

Signal::Signal(const Signal& signal) {
	this->functions = signal.functions;
}

Signal::~Signal() { }


Functions::iterator Signal::PushFunction(boost::function<void()> func) {
	LOCKCLASS lockClass(lockSignal);

	Functions::iterator it = functions.end();
	functions.push_back(std::pair<boost::function<void()>, bool>(func, true));

	return it;
}

void Signal::BlockFunction(Functions::iterator it) {
	LOCKCLASS lockClass(lockSignal);

	if (it != functions.end())
		it->second = false;
}

void Signal::UnblockFunction(Functions::iterator it) {
	LOCKCLASS lockClass(lockSignal);

	if (it != functions.end())
		it->second = true;
}

bool Signal::IsExecutable() {
	LOCKCLASS lockClass(lockSignal);

	bool executable = false;

	Functions::iterator it = functions.begin();
	for (it; it != functions.end(); it++) {
		if (it->second)
			executable = true;
	}

	return executable;
}

void Signal::Execute(bool deleteAfter) {
	LOCKCLASS lockClass(lockSignal);

	Functions funcs = functions;

	int num = 0;
	Functions::iterator it = funcs.begin();
	for (it; it != funcs.end(); it++, num++) {
		if (it->second)
			it->first();
	}

	if (!deleteAfter)
		return;

	delete_debug(this, M_PLACE);
}

void Signal::Clear() {
	LOCKCLASS lockClass(lockSignal);

	functions.clear();
}


int Signal::GetSize() {
	LOCKCLASS lockClass(lockSignal);

	return functions.size();
}


void Signal::Swap(Signal& signal) {
	LOCKCLASS lockClass1(lockSignal);
	LOCKCLASS lockClass2(signal.lockSignal);

	Functions funcs = functions;
	functions = signal.functions;
	signal.functions = funcs;
}

void Signal::Assign(Signal& signal) {
	LOCKCLASS lockClass1(lockSignal);
	LOCKCLASS lockClass2(signal.lockSignal);

	functions = signal.functions;
}
