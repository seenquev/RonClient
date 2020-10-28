/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#ifndef __SERVERS_H_
#define __SERVERS_H_

#include <vector>
#include <string>

#include "mthread.h"


class WindowElementContainer;
class WindowElementMemo;
class WindowElementTableMemo;


struct Server {
	std::string name;
	std::string host;
	std::string port;
	std::string protocol;
	std::string updateURL;
	std::string websiteURL;
	std::string createAccountURL;
	bool promoted;
};


typedef std::vector<Server> ServersList;


class Servers {
private:
	WindowElementContainer*	tabContainer;
	WindowElementContainer*	onlineContainer;
	WindowElementContainer*	favoritesContainer;
	WindowElementContainer*	websitesContainer;

	ServersList	onlineServers;
	ServersList	favoritesServers;
	ServersList websitesServers;

public:
	MUTEX	lockServers;

public:
	Servers();
	~Servers();

	void LoadFavorites();
	void SaveFavorites();

	void LoadWebsites();

	void AddOnlineServer(Server server);
	void RemoveOnlineServer(int num);
	void ClearOnlineServers();
	Server GetOnlineServer(int num);

	void AddFavoriteServer(Server server);
	void RemoveFavoriteServer(int num);
	void MoveFavoriteServer(int num, int step);
	void ClearFavoriteServers();
	Server GetFavoriteServer(int num);
	int GetFavoriteNum(std::string name);
	void SortFavorites(int column);

	void AddWebsiteServer(Server server);
	void RemoveWebsiteServer(int num);
	void ClearWebsiteServers();
	Server GetWebsiteServer(int num);

	void SetContainers(WindowElementContainer* tabContainer, WindowElementContainer* onlineContainer, WindowElementContainer* favoritesContainer, WindowElementContainer* websitesContainer);
	void UpdateContainers(std::vector<void*> pointers);
	void UpdateFavoritesMemo(WindowElementTableMemo* memo);
};


#endif //__SERVERS_H_
