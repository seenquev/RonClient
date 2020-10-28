/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#include "servers.h"

#include "allocator.h"
#include "game.h"
#include "iniloader.h"
#include "text.h"
#include "tools.h"
#include "window.h"


// ---- Servers ---- //

Servers::Servers() {
	tabContainer = NULL;
	onlineContainer = NULL;
	favoritesContainer = NULL;
	websitesContainer = NULL;
}

Servers::~Servers() { }


void Servers::LoadFavorites() {
	LOCKCLASS lockClass(lockServers);

	favoritesServers.clear();

	INILoader iniPromoted;
	if (iniPromoted.OpenFile("promoted.ini")) {
		Server serv;
		unsigned short num = 0;
		while(true) {
			std::string prefix = std::string("SERVER") + value2str(num) + std::string("_");
			if (iniPromoted.GetValue(prefix + "HOST") == "")
				break;

			serv.name = iniPromoted.GetValue(prefix + "NAME");
			serv.host = iniPromoted.GetValue(prefix + "HOST");
			serv.port = iniPromoted.GetValue(prefix + "PORT");
			serv.protocol = iniPromoted.GetValue(prefix + "PROTOCOL");
			serv.updateURL = iniPromoted.GetValue(prefix + "UPDATEURL");
			serv.websiteURL = iniPromoted.GetValue(prefix + "WEBSITEURL");
			serv.createAccountURL = iniPromoted.GetValue(prefix + "CREATEACCOUNTURL");
			serv.promoted = true;

			AddFavoriteServer(serv);
			num++;
		}
	}

	bool save = false;
	INILoader iniFavorites;
	if (iniFavorites.OpenFile("favorites.ini")) {
		Server serv;
		unsigned short num = 0;
		while(true) {
			std::string prefix = std::string("SERVER") + value2str(num) + std::string("_");
			if (iniFavorites.GetValue(prefix + "HOST") == "")
				break;

			serv.name = iniFavorites.GetValue(prefix + "NAME");
			serv.host = iniFavorites.GetValue(prefix + "HOST");
			serv.port = iniFavorites.GetValue(prefix + "PORT");
			serv.protocol = iniFavorites.GetValue(prefix + "PROTOCOL");
			serv.updateURL = iniFavorites.GetValue(prefix + "UPDATEURL");
			serv.websiteURL = iniFavorites.GetValue(prefix + "WEBSITEURL");
			serv.createAccountURL = iniFavorites.GetValue(prefix + "CREATEACCOUNTURL");
			serv.promoted = false;

			int n = GetFavoriteNum(serv.name);
			if (n == -1)
				AddFavoriteServer(serv);
			else {
				Server promoted = GetFavoriteServer(n);
				std::string filesLocationOld = std::string("./data/") + serv.host + "." + serv.port;
				std::string filesLocationNew = std::string("./data/") + promoted.host + "." + promoted.port;
				if (filesLocationOld != filesLocationNew) {
					rename(filesLocationOld.c_str(), filesLocationNew.c_str());
					save = true;
				}
			}
			num++;
		}
	}

	if (save)
		SaveFavorites();
}

void Servers::SaveFavorites() {
	LOCKCLASS lockClass(lockServers);

	INILoader iniFavorites;

	unsigned short num = 0;
	ServersList::iterator it = favoritesServers.begin();
	for (it; it != favoritesServers.end(); it++) {
		std::string prefix = std::string("SERVER") + value2str(num) + std::string("_");

		Server serv = *it;
		iniFavorites.SetValue(prefix + "NAME", serv.name);
		iniFavorites.SetValue(prefix + "HOST", serv.host);
		iniFavorites.SetValue(prefix + "PORT", serv.port);
		iniFavorites.SetValue(prefix + "PROTOCOL", serv.protocol);
		iniFavorites.SetValue(prefix + "UPDATEURL", serv.updateURL);
		iniFavorites.SetValue(prefix + "WEBSITEURL", serv.websiteURL);
		iniFavorites.SetValue(prefix + "CREATEACCOUNTURL", serv.createAccountURL);
		num++;
	}

	iniFavorites.SaveFile("favorites.ini");
}


void Servers::LoadWebsites() {
	LOCKCLASS lockClass(lockServers);

	websitesServers.clear();

	INILoader iniWebsites;
	if (!iniWebsites.OpenFile("websites.ini"))
		return;

	Server serv;
	unsigned short num = 0;
	while(true) {
		std::string prefix = std::string("SERVER") + value2str(num) + std::string("_");
		if (iniWebsites.GetValue(prefix + "WEBSITEURL") == "")
			break;

		serv.name = iniWebsites.GetValue(prefix + "NAME");
		serv.host = "";
		serv.port = "";
		serv.protocol = "";
		serv.updateURL = "";
		serv.websiteURL = iniWebsites.GetValue(prefix + "WEBSITEURL");
		serv.createAccountURL = "";
		serv.promoted = false;

		websitesServers.push_back(serv);
		num++;
	}
}


void Servers::AddOnlineServer(Server server) {
	LOCKCLASS lockClass(lockServers);

	ServersList::iterator it = onlineServers.begin();
	for (it; it != onlineServers.end(); it++) {
		Server serv = *it;
		if (serv.host == server.host && serv.port == server.port) {
			*it = server;
			return;
		}
	}

	if (it == onlineServers.end())
		onlineServers.push_back(server);
}

void Servers::RemoveOnlineServer(int num) {
	LOCKCLASS lockClass(lockServers);

	ServersList::iterator it = onlineServers.begin() + num;
	if (it < onlineServers.end())
		onlineServers.erase(it);
}

void Servers::ClearOnlineServers() {
	LOCKCLASS lockClass(lockServers);

	onlineServers.clear();
}

Server Servers::GetOnlineServer(int num) {
	LOCKCLASS lockClass(lockServers);

	ServersList::iterator it = onlineServers.begin() + num;
	if (it < onlineServers.end())
		return *it;

	Server serv;
	serv.name = "";
	serv.host = "";
	return serv;
}


void Servers::AddFavoriteServer(Server server) {
	LOCKCLASS lockClass(lockServers);

	if (server.name == "")
		return;

	ServersList::iterator it = favoritesServers.begin();
	for (it; it != favoritesServers.end(); it++) {
		Server serv = *it;
		if (serv.name == server.name) {
			if (!serv.promoted)
				*it = server;
			else {
				Game* game = Game::game;
				Windows* wnds = (game ? game->GetWindows() : NULL);
				if (wnds)
					wnds->OpenWindow(WND_MESSAGE, Text::GetText("ERROR_MESSAGE_7", Game::options.language).c_str());
			}
			return;
		}
	}

	if (it == favoritesServers.end())
		favoritesServers.push_back(server);
}

void Servers::RemoveFavoriteServer(int num) {
	LOCKCLASS lockClass(lockServers);

	ServersList::iterator it = favoritesServers.begin() + num;
	if (it < favoritesServers.end()) {
		if (!it->promoted)
			favoritesServers.erase(it);
		else {
			Game* game = Game::game;
			Windows* wnds = (game ? game->GetWindows() : NULL);
			if (wnds)
				wnds->OpenWindow(WND_MESSAGE, Text::GetText("ERROR_MESSAGE_7", Game::options.language).c_str());
		}
	}
}

void Servers::MoveFavoriteServer(int num, int step) {
	LOCKCLASS lockClass(lockServers);

	int fromNum = num;
	int toNum = num + step;

	if (fromNum < 0 || fromNum >= favoritesServers.size() || toNum < 0 || toNum >= favoritesServers.size())
		return;

	Server serv = favoritesServers[fromNum];
	favoritesServers[fromNum] = favoritesServers[toNum];
	favoritesServers[toNum] = serv;
}

void Servers::ClearFavoriteServers() {
	LOCKCLASS lockClass(lockServers);

	favoritesServers.clear();
}

Server Servers::GetFavoriteServer(int num) {
	LOCKCLASS lockClass(lockServers);

	ServersList::iterator it = favoritesServers.begin() + num;
	if (it < favoritesServers.end())
		return *it;

	Server serv;
	serv.name = "";
	serv.host = "";
	return serv;
}

int Servers::GetFavoriteNum(std::string name) {
	LOCKCLASS lockClass(lockServers);

	int num = 0;

	ServersList::iterator it = favoritesServers.begin();
	for (it; it != favoritesServers.end(); it++) {
		Server serv = *it;
		if (serv.name == name)
			return num;

		num++;
	}

	return -1;
}


class ServSorter {
public:
	int column;
	int n;

public:
	ServSorter(int col) : column(col) {	n = 0; }
	bool operator()(Server serv1, Server serv2) {
		if (serv1.promoted && !serv2.promoted) return true;
		else if (!serv1.promoted && serv2.promoted) return false;

		if (column == 0) {
			for (int i = 0; i < serv1.name.length() && i < serv2.name.length(); i++) {
				if (serv1.name[i] < serv2.name[i]) return true;
				else if (serv1.name[i] > serv2.name[i]) return false;
			}
			if (serv1.name.length() < serv2.name.length()) return true;
			else return false;
		}
		else if (column == 1) {
			for (int i = 0; i < serv1.host.length() && i < serv2.host.length(); i++) {
				if (serv1.host[i] < serv2.host[i]) return true;
				else if (serv1.host[i] > serv2.host[i]) return false;
			}
			if (serv1.host.length() < serv2.host.length()) return true;
			else return false;
		}
		else if (column == 2) {
			for (int i = 0; i < serv1.port.length() && i < serv2.port.length(); i++) {
				if (serv1.port[i] < serv2.port[i]) return true;
				else if (serv1.port[i] > serv2.port[i]) return false;
			}
			if (serv1.port.length() < serv2.port.length()) return true;
			else return false;
		}
		else if (column == 3) {
			for (int i = 0; i < serv1.protocol.length() && i < serv2.protocol.length(); i++) {
				if (serv1.protocol[i] < serv2.protocol[i]) return true;
				else if (serv1.protocol[i] > serv2.protocol[i]) return false;
			}
			if (serv1.protocol.length() < serv2.protocol.length()) return true;
			else return false;
		}

		return false;
	}
};

void Servers::SortFavorites(int column) {
	LOCKCLASS lockClass(lockServers);

	std::sort(favoritesServers.begin(), favoritesServers.end(), ServSorter(column));
}


void Servers::AddWebsiteServer(Server server) {
	LOCKCLASS lockClass(lockServers);

	ServersList::iterator it = websitesServers.begin();
	for (it; it != websitesServers.end(); it++) {
		Server serv = *it;
		if (serv.host == server.host && serv.port == server.port) {
			*it = server;
			return;
		}
	}

	if (it == websitesServers.end())
        websitesServers.push_back(server);
}

void Servers::RemoveWebsiteServer(int num) {
	LOCKCLASS lockClass(lockServers);

	ServersList::iterator it = websitesServers.begin() + num;
	if (it < websitesServers.end())
		 websitesServers.erase(it);
}

void Servers::ClearWebsiteServers() {
	LOCKCLASS lockClass(lockServers);

    websitesServers.clear();
}

Server Servers::GetWebsiteServer(int num) {
	LOCKCLASS lockClass(lockServers);

	ServersList::iterator it = websitesServers.begin() + num;
	if (it < websitesServers.end())
		return *it;

	Server serv;
	serv.name = "";
	serv.host = "";
	return serv;
}


void Servers::SetContainers(WindowElementContainer* tabContainer, WindowElementContainer* onlineContainer, WindowElementContainer* favoritesContainer, WindowElementContainer* websitesContainer) {
	LOCKCLASS lockClass(lockServers);

	this->tabContainer = tabContainer;
	this->onlineContainer = onlineContainer;
	this->favoritesContainer = favoritesContainer;
	this->websitesContainer = websitesContainer;
}

void Servers::UpdateContainers(std::vector<void*> pointers) {
	LOCKCLASS lockClass1(Windows::lockWindows);
	LOCKCLASS lockClass2(lockServers);

	if (!tabContainer || !onlineContainer || !favoritesContainer || !websitesContainer)
		return;

	WindowElementTextarea* ta_account = (WindowElementTextarea*)pointers[0];
	WindowElementTextarea* ta_password = (WindowElementTextarea*)pointers[1];
	WindowElementList* ls_servers = (WindowElementList*)pointers[2];
	WindowElementTextarea* ta_server = (WindowElementTextarea*)pointers[3];

	Game* game = Game::game;
	Mouse* mouse = game->GetMouse();
	Host host = game->GetHost();

	Window* window = tabContainer->GetWindow();

	onlineContainer->DeleteAllElements();
	favoritesContainer->DeleteAllElements();
	websitesContainer->DeleteAllElements();

	POINT onlineSize = onlineContainer->GetSize();
	POINT favoritesSize = favoritesContainer->GetSize();
	POINT websitesSize = websitesContainer->GetSize();

	WindowElementMemo* onlineCntMemo = new(M_PLACE) WindowElementMemo;
	onlineCntMemo->Create(0, 0, 0, onlineSize.x, onlineSize.y - 20, window->GetWindowTemplate());
	onlineCntMemo->SetLocks(true, true);

	WindowElementTableMemo* favoritesCntMemo = new(M_PLACE) WindowElementTableMemo;
	favoritesCntMemo->Create(0, 0, 0, favoritesSize.x, favoritesSize.y - 20, window->GetWindowTemplate());
	favoritesCntMemo->SetLocks(true, true);

	WindowElementMemo* websitesCntMemo = new(M_PLACE) WindowElementMemo;
	websitesCntMemo->Create(0, 0, 0, websitesSize.x, websitesSize.y - 20, window->GetWindowTemplate());
	websitesCntMemo->SetLocks(true, true);

	std::vector<void*> pointers_null;

	int posY = 0;
	ServersList::iterator it = onlineServers.begin();
	for (it; it != onlineServers.end(); it++) {
		Server serv = *it;
		std::string header = (serv.name != "" ? serv.name : serv.host) + " (Port: " + serv.port + ", Protocol: " + serv.protocol + ")";

		onlineCntMemo->AddElement(TextString(header));
	}
	onlineCntMemo->SetAction(boost::bind(&Game::onAddFavoriteServer, game, true, pointers, pointers_null));
	onlineCntMemo->SetDblAction(boost::bind(&Game::SaveOptions, game));

	posY = 0;
	it = favoritesServers.begin();
	for (it; it != favoritesServers.end(); it++) {
		Server serv = *it;
		std::string header = (serv.name != "" ? serv.name : serv.host) + " (Port: " + serv.port + ", Protocol: " + serv.protocol + ")";

		COLOR color(1.0f, 1.0f, 1.0f);
		if (serv.promoted)
			color = COLOR(1.0f, 0.2f, 0.2f);

		WindowElementText* sname = new(M_PLACE) WindowElementText;
		sname->Create(0, 5, 0, 0xFFFF, window->GetWindowTemplate());
		sname->SetFontSize(14);
		sname->SetColor(color.red, color.green, color.blue);
		sname->SetText(serv.name);
		sname->SetBorder(1);

		WindowElementText* shost = new(M_PLACE) WindowElementText;
		shost->Create(0, 5, 0, 0xFFFF, window->GetWindowTemplate());
		shost->SetFontSize(14);
		shost->SetColor(color.red, color.green, color.blue);
		shost->SetText(serv.host);
		shost->SetBorder(1);

		WindowElementText* sport = new(M_PLACE) WindowElementText;
		sport->Create(0, 5, 0, 0xFFFF, window->GetWindowTemplate());
		sport->SetFontSize(14);
		sport->SetColor(color.red, color.green, color.blue);
		sport->SetText(serv.port);
		sport->SetBorder(1);

		WindowElementText* sprotocol = new(M_PLACE) WindowElementText;
		sprotocol->Create(0, 5, 0, 0xFFFF, window->GetWindowTemplate());
		sprotocol->SetFontSize(14);
		sprotocol->SetColor(color.red, color.green, color.blue);
		sprotocol->SetText(serv.protocol);
		sprotocol->SetBorder(1);

		favoritesCntMemo->AddRow();
		favoritesCntMemo->AddColumn(sname);
		favoritesCntMemo->AddColumn(shost);
		favoritesCntMemo->AddColumn(sport);
		favoritesCntMemo->AddColumn(sprotocol);
	}
	favoritesCntMemo->SetColumnWidthPercent(0, 30);
	favoritesCntMemo->SetColumnName(0, Text::GetText("SERVERS_7", Game::options.language));
	favoritesCntMemo->SetColumnWidthPercent(1, 40);
	favoritesCntMemo->SetColumnName(1, Text::GetText("SERVERS_8", Game::options.language));
	favoritesCntMemo->SetColumnWidthPX(2, 55);
	favoritesCntMemo->SetColumnName(2, Text::GetText("SERVERS_9", Game::options.language));
	favoritesCntMemo->SetColumnWidthPX(3, 45);
	favoritesCntMemo->SetColumnName(3, Text::GetText("SERVERS_10", Game::options.language));

	favoritesCntMemo->SetAction(boost::bind(&Game::onSetServer, game, favoritesCntMemo, pointers));
	favoritesCntMemo->SetDblAction(boost::bind(&Window::SetAction, window, ACT_CLOSE));
	favoritesCntMemo->SetDblAction(boost::bind(&Game::SaveOptions, game));

	posY = 0;
	it = websitesServers.begin();
	for (it; it != websitesServers.end(); it++) {
		Server serv = *it;
		std::string header = serv.name + " (" + serv.websiteURL + ")";

		websitesCntMemo->AddElement(TextString(header));
	}
	websitesCntMemo->SetDblAction(boost::bind(&Game::onGoToURLServer, game, websitesCntMemo, 2, true));
	websitesCntMemo->SetDblAction(boost::bind(&Window::SetAction, window, ACT_CLOSE));

	onlineContainer->AddElement(onlineCntMemo);
	favoritesContainer->AddElement(favoritesCntMemo);
	websitesContainer->AddElement(websitesCntMemo);

	int online_bt_width = (onlineSize.x - 40) / 4;
	int favorites_bt_width = (favoritesSize.x - 40) / 4;
	int websites_bt_width = (websitesSize.x - 40) / 4;

	WindowElementButton* bt_addtofav1 = new(M_PLACE) WindowElementButton;
	bt_addtofav1->Create(ALIGN_H_LEFT | ALIGN_V_BOTTOM, BUTTON_NORMAL, online_bt_width * 0, onlineSize.y - 20, online_bt_width, 20, window->GetWindowTemplate());
	bt_addtofav1->SetText(Text::GetText("SERVERS_3", Game::options.language));
	bt_addtofav1->SetFontSize(12);
	bt_addtofav1->SetAction(boost::bind(&Game::onAddFavoriteServer, game, true, pointers, pointers_null));
	onlineContainer->AddElement(bt_addtofav1);

	WindowElementButton* bt_gotowebsite1 = new(M_PLACE) WindowElementButton;
	bt_gotowebsite1->Create(ALIGN_H_RIGHT | ALIGN_V_BOTTOM, BUTTON_NORMAL, online_bt_width * 2, onlineSize.y - 20, online_bt_width, 20, window->GetWindowTemplate());
	bt_gotowebsite1->SetText(Text::GetText("SERVERS_5", Game::options.language));
	bt_gotowebsite1->SetFontSize(12);
	bt_gotowebsite1->SetAction(boost::bind(&Game::onGoToURLServer, game, onlineCntMemo, 0, true));
	onlineContainer->AddElement(bt_gotowebsite1);

	WindowElementButton* bt_createaccount1 = new(M_PLACE) WindowElementButton;
	bt_createaccount1->Create(ALIGN_H_RIGHT | ALIGN_V_BOTTOM, BUTTON_NORMAL, online_bt_width * 3, onlineSize.y - 20, online_bt_width, 20, window->GetWindowTemplate());
	bt_createaccount1->SetText(Text::GetText("SERVERS_6", Game::options.language));
	bt_createaccount1->SetFontSize(12);
	bt_createaccount1->SetAction(boost::bind(&Game::onGoToURLServer, game, onlineCntMemo, 0, false));
	onlineContainer->AddElement(bt_createaccount1);

	WindowElementButton* bt_addtofav2 = new(M_PLACE) WindowElementButton;
	bt_addtofav2->Create(ALIGN_H_LEFT | ALIGN_V_BOTTOM, BUTTON_NORMAL, favorites_bt_width * 0, favoritesSize.y - 20, favorites_bt_width, 20, window->GetWindowTemplate());
	bt_addtofav2->SetText(Text::GetText("SERVERS_3", Game::options.language));
	bt_addtofav2->SetFontSize(12);
	bt_addtofav2->SetAction(boost::bind(&Game::onAddFavoriteServer, game, true, pointers, pointers_null));
	favoritesContainer->AddElement(bt_addtofav2);

	WindowElementButton* bt_removefromfav = new(M_PLACE) WindowElementButton;
	bt_removefromfav->Create(ALIGN_H_LEFT | ALIGN_V_BOTTOM, BUTTON_NORMAL, favorites_bt_width * 1, favoritesSize.y - 20, favorites_bt_width, 20, window->GetWindowTemplate());
	bt_removefromfav->SetText(Text::GetText("SERVERS_4", Game::options.language));
	bt_removefromfav->SetFontSize(12);
	bt_removefromfav->SetAction(boost::bind(&Game::onRemoveFavoriteServer, game, favoritesCntMemo, pointers));
	bt_removefromfav->SetAction(boost::bind(&Servers::SaveFavorites, this));
	favoritesContainer->AddElement(bt_removefromfav);

	WindowElementButton* bt_gotowebsite2 = new(M_PLACE) WindowElementButton;
	bt_gotowebsite2->Create(ALIGN_H_RIGHT | ALIGN_V_BOTTOM, BUTTON_NORMAL, favorites_bt_width * 2, favoritesSize.y - 20, favorites_bt_width, 20, window->GetWindowTemplate());
	bt_gotowebsite2->SetText(Text::GetText("SERVERS_5", Game::options.language));
	bt_gotowebsite2->SetFontSize(12);
	bt_gotowebsite2->SetAction(boost::bind(&Game::onGoToURLServer, game, favoritesCntMemo, 1, true));
	favoritesContainer->AddElement(bt_gotowebsite2);

	WindowElementButton* bt_createaccount2 = new(M_PLACE) WindowElementButton;
	bt_createaccount2->Create(ALIGN_H_RIGHT | ALIGN_V_BOTTOM, BUTTON_NORMAL, favorites_bt_width * 3, favoritesSize.y - 20, favorites_bt_width, 20, window->GetWindowTemplate());
	bt_createaccount2->SetText(Text::GetText("SERVERS_6", Game::options.language));
	bt_createaccount2->SetFontSize(12);
	bt_createaccount2->SetAction(boost::bind(&Game::onGoToURLServer, game, favoritesCntMemo, 1, false));
	favoritesContainer->AddElement(bt_createaccount2);

	WindowElementButton* bt_moveup = new(M_PLACE) WindowElementButton;
	bt_moveup->Create(ALIGN_H_RIGHT | ALIGN_V_BOTTOM, BUTTON_NORMAL, favoritesSize.x - 40, favoritesSize.y - 20, 20, 20, window->GetWindowTemplate());
	bt_moveup->SetText("/\\");
	bt_moveup->SetFontSize(12);
	bt_moveup->SetAction(boost::bind(&Game::onMoveFavoriteServer, game, favoritesCntMemo, -1, pointers));
	bt_moveup->SetAction(boost::bind(&Servers::SaveFavorites, this));
	favoritesContainer->AddElement(bt_moveup);

	WindowElementButton* bt_movedown = new(M_PLACE) WindowElementButton;
	bt_movedown->Create(ALIGN_H_RIGHT | ALIGN_V_BOTTOM, BUTTON_NORMAL, favoritesSize.x - 20, favoritesSize.y - 20, 20, 20, window->GetWindowTemplate());
	bt_movedown->SetText("\\/");
	bt_movedown->SetFontSize(12);
	bt_movedown->SetAction(boost::bind(&Game::onMoveFavoriteServer, game, favoritesCntMemo, 1, pointers));
	bt_movedown->SetAction(boost::bind(&Servers::SaveFavorites, this));
	favoritesContainer->AddElement(bt_movedown);

	WindowElementButton* bt_gotowebsite3 = new(M_PLACE) WindowElementButton;
	bt_gotowebsite3->Create(ALIGN_H_LEFT | ALIGN_V_BOTTOM, BUTTON_NORMAL, websites_bt_width * 0, websitesSize.y - 20, websites_bt_width, 20, window->GetWindowTemplate());
	bt_gotowebsite3->SetText(Text::GetText("SERVERS_5", Game::options.language));
	bt_gotowebsite3->SetFontSize(12);
	bt_gotowebsite3->SetAction(boost::bind(&Game::onGoToURLServer, game, websitesCntMemo, 2, true));
	websitesContainer->AddElement(bt_gotowebsite3);

	int num = GetFavoriteNum(host.name);
	if (num != -1)
		favoritesCntMemo->SetOption(num);
	else
		favoritesCntMemo->SetOption(0);
}

void Servers::UpdateFavoritesMemo(WindowElementTableMemo* memo) {
	LOCKCLASS lockClass1(Windows::lockWindows);
	LOCKCLASS lockClass2(lockServers);

	Game* game = Game::game;

	memo->Clear();

	int posY = 0;
	ServersList::iterator it = favoritesServers.begin();
	for (it; it != favoritesServers.end(); it++) {
		Server serv = *it;
		std::string header = (serv.name != "" ? serv.name : serv.host) + " (Port: " + serv.port + ", Protocol: " + serv.protocol + ")";

		COLOR color(1.0f, 1.0f, 1.0f);
		if (serv.promoted)
			color = COLOR(1.0f, 0.2f, 0.2f);

		WindowElementText* sname = new(M_PLACE) WindowElementText;
		sname->Create(0, 5, 0, 0xFFFF, memo->GetWindowTemplate());
		sname->SetFontSize(14);
		sname->SetColor(color.red, color.green, color.blue);
		sname->SetText(serv.name);
		sname->SetBorder(1);

		WindowElementText* shost = new(M_PLACE) WindowElementText;
		shost->Create(0, 5, 0, 0xFFFF, memo->GetWindowTemplate());
		shost->SetFontSize(14);
		shost->SetColor(color.red, color.green, color.blue);
		shost->SetText(serv.host);
		shost->SetBorder(1);

		WindowElementText* sport = new(M_PLACE) WindowElementText;
		sport->Create(0, 5, 0, 0xFFFF, memo->GetWindowTemplate());
		sport->SetFontSize(14);
		sport->SetColor(color.red, color.green, color.blue);
		sport->SetText(serv.port);
		sport->SetBorder(1);

		WindowElementText* sprotocol = new(M_PLACE) WindowElementText;
		sprotocol->Create(0, 5, 0, 0xFFFF, memo->GetWindowTemplate());
		sprotocol->SetFontSize(14);
		sprotocol->SetColor(color.red, color.green, color.blue);
		sprotocol->SetText(serv.protocol);
		sprotocol->SetBorder(1);

		memo->AddRow();
		memo->AddColumn(sname);
		memo->AddColumn(shost);
		memo->AddColumn(sport);
		memo->AddColumn(sprotocol);
	}

	memo->SetColumnWidthPercent(0, 30);
	memo->SetColumnName(0, Text::GetText("SERVERS_7", Game::options.language));
	memo->SetColumnWidthPercent(1, 40);
	memo->SetColumnName(1, Text::GetText("SERVERS_8", Game::options.language));
	memo->SetColumnWidthPX(2, 55);
	memo->SetColumnName(2, Text::GetText("SERVERS_9", Game::options.language));
	memo->SetColumnWidthPX(3, 45);
	memo->SetColumnName(3, Text::GetText("SERVERS_10", Game::options.language));
}
