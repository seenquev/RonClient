/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#ifndef __UPDATER_H_
#define __UPDATER_H_

#include <string>

#include "mthread.h"
#include "network.h"


class WindowElementContainer;


class Updater {
private:
	Client		httpClient;

	WindowElementContainer* container;

	Signal	signal;
	THREAD	thread;

public:
	static bool		running;
	static bool		completed;

	static MUTEX	lockUpdater;

public:
	Updater();
	~Updater();

	void StartUpdating(std::string server, std::string location, std::string directory, bool checkVersion);
	void StopUpdating();
	void UpdateLoop(std::string server, std::string location, std::string directory, bool checkVersion);
	void SignalAddFunction(boost::function<void()> f);
	void SignalExecute();
	void SignalClear();

	void IncCounter(std::string server);
	void SendErrorReport(std::string server, std::string player, std::string* titlePtr, std::string* descPtr);
	bool CheckFile(std::string server, std::string location, std::string filename);
	bool DownloadFile(std::string server, std::string location, std::string directory, std::string filename, unsigned long* downloaded, unsigned char* progress);

	void SetContainer(WindowElementContainer* container);
	void UpdateContainer(std::string filename, unsigned long* downloaded, unsigned char* progress);
};

#endif //__UPDATER_H_
