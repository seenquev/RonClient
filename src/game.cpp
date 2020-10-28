/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#include "game.h"

#include "allocator.h"
#include "channel.h"
#include "filemanager.h"
#include "logger.h"
#include "protocol822.h"
#include "protocol840.h"
#include "protocol842.h"
#include "protocol850.h"
#include "protocol854.h"
#include "protocol860.h"
#include "protocol870.h"
#include "protocol910.h"
#include "text.h"
#include "tools.h"
#include "window.h"


// ---- Game ---- //

MUTEX	Game::lockGame;

std::list<std::pair<std::string, std::string> >	Game::openChannels;

Signal	Game::taskManager;

bool	Game::running = false;
bool	Game::releaseSprites = false;
bool	Game::screenshot = false;

bool	Game::systemMusic = true;

Options			Game::options;
AdminOptions	Game::adminOptions;
Game*			Game::game = NULL;

std::string		Game::dataLocation = "";
std::string     Game::filesLocation = "";


Game::Game() {
	wnds = NULL;
	wndStatus = NULL;
	wndHotkeys = NULL;
	wndGame = NULL;
	wndConsole = NULL;
	wndMiniMap = NULL;
	wndInventory = NULL;
	wndStatistics = NULL;
	wndBattle = NULL;
	wndVIPList = NULL;

	consoleTab = NULL;
	cmdLine = NULL;

	gameState = GAME_IDLE;

	icons = NULL;
	particles = NULL;
#ifndef NO_SOUND
	sfx = NULL;
#endif

	protocol = NULL;
	map = NULL;
	player = NULL;
	viplist = NULL;
	shop = NULL;
	trade = NULL;
	craftbox = NULL;
	cooldowns = NULL;
	questlog = NULL;
	status = NULL;
	updater = NULL;
	servers = NULL;

	mouse = NULL;
	keyboard = NULL;

	host.account = "";
	host.password = "";
	host.host = "";
	host.port = "";
	host.update = "";

	host.name = "";
	host.protocol = "";
	host.website = "";
	host.createacc = "";

	host.PACC = 0;

	character.name = "";
	character.serv = "";
	character.servIP = 0;
	character.servPort = 0;

	options.graphicsAPI = API_DIRECT3D;
	options.limitFPS = 25;
	options.ambientLight = 30;
	options.fightModes = 0x29;
	options.language = 0;
	options.showNames = true;
	options.showBars = true;
	options.dontStrech = false;
	options.printParticles = false;
	options.printOneLevel = false;
	options.renderToTexture = true;
	options.classicControls = true;
	options.autoUpdate = true;
	options.textSize = 14;
	options.flyingSpeed = 50;
	options.disappearingSpeed = 50;
	options.fixedPositions = true;
	options.fadeOutInactive = false;
	options.musicGain = 100;
	options.soundGain = 100;
	options.updateURL = "client.ronit.pl/update";	options.protocol = 870;
	options.templatesGroup = "classic";

	adminOptions.disableBot = false;
	adminOptions.hideLevels = false;
	adminOptions.fullLight = false;
	adminOptions.simpleLogin = false;

	for (int i = 0; i < 48; i++) {
		hotkey[i].itemContainer = NULL;
		ClearHotKey(i);
	}

	running = true;

	THREAD policy(&Game::CheckPrivateData, "Game::CheckPrivateData()", "Private data corrupted!");

	Game::game = this;
}

Game::~Game() { }


void Game::CheckPrivateData(std::string func, std::string desc) {
	bool check1, check2, check3, check4, check5, check6;
	while(running) {
		check1 = CheckChecksum(APP_NAME, 6051);
		check2 = CheckChecksum(APP_COMPANY, 1753);
		check3 = CheckChecksum(APP_WEBSITE, 20870);
		check4 = CheckChecksum(APP_RIGHTS, 33542);
		check5 = CheckChecksum(APP_INTERACT_ADDRESS, 28177);
		//check6 = CheckChecksumFile(Icons::logoFilename.c_str(), 0xF7AFBD1F);

		if (!check1 || !check2 || !check3 || !check4 || !check5/* || !check6*/) {
			running = false;
			Logger::AddLog(func.c_str(), desc.c_str(), LOG_ERROR);
		}

		int i = 120;
		while(running && i != 0) {
			Sleep(500);
			i--;
		}
	}
}


Options* Game::GetOptions() {
	return &options;
}


void Game::SetParticles(Particles* particles) {
	LOCKCLASS lockClass(lockGame);

	this->particles = particles;
}

Particles* Game::GetParticles() {
	return particles;
}

#ifndef NO_SOUND
void Game::SetSFX(SFX_System* sfx) {
	LOCKCLASS lockClass(lockGame);

	this->sfx = sfx;
}

SFX_System* Game::GetSFX() {
	return sfx;
}


void Game::UpdateBackgroundSound(unsigned char level, bool firstCheck) {
	if (!sfx || !systemMusic)
		return;

	unsigned char lightLevel = game->GetWorldLightLevel();
	if (firstCheck) {
		if (level <= 7) {
			if (lightLevel > 80) sfx->PlayBackgroundSound(SAMPLE_BACKGROUND, 0);
			else sfx->PlayBackgroundSound(SAMPLE_BACKGROUND, 1);
		}
		else
			sfx->PlayBackgroundSound(SAMPLE_BACKGROUND, 2);
	}
	else {
		if (level <= 7) {
			if (lightLevel > 80 && sfx->GetBackgroundSample() != 0)
				sfx->PlayBackgroundSound(SAMPLE_BACKGROUND, 0);
			else if (lightLevel <= 80 && sfx->GetBackgroundSample() != 1)
				sfx->PlayBackgroundSound(SAMPLE_BACKGROUND, 1);
		}
		else if (level > 7 && sfx->GetBackgroundSample() != 2)
			sfx->PlayBackgroundSound(SAMPLE_BACKGROUND, 2);
	}
}
#endif

void Game::SetIcons(Icons* icons) {
	LOCKCLASS lockClass(lockGame);

	this->icons = icons;
}

Icons* Game::GetIcons() {
	return icons;
}

void Game::SetWindows(Windows* wnds) {
	LOCKCLASS lockClass(lockGame);

	this->wnds = wnds;
}

Windows* Game::GetWindows() {
	return wnds;
}

void Game::SetWindowStatus(Window* wnd) {
	LOCKCLASS lockClass(lockGame);

	this->wndStatus = wnd;
}

Window* Game::GetWindowStatus() {
	return this->wndStatus;
}

void Game::SetWindowHotkeys(Window* wnd) {
	LOCKCLASS lockClass(lockGame);

	this->wndHotkeys = wnd;
}

Window* Game::GetWindowHotkeys() {
	return wndHotkeys;
}

void Game::SetWindowGame(Window* wnd) {
	LOCKCLASS lockClass(lockGame);

	this->wndGame = wnd;
}

Window* Game::GetWindowGame() {
	return wndGame;
}

void Game::SetWindowConsole(Window* wnd) {
	LOCKCLASS lockClass(lockGame);

	this->wndConsole = wnd;
}

Window* Game::GetWindowConsole() {
	return wndConsole;
}

void Game::SetWindowMiniMap(Window* wnd) {
	LOCKCLASS lockClass(lockGame);

	this->wndMiniMap = wnd;
}

Window* Game::GetWindowMiniMap() {
	return wndMiniMap;
}

void Game::SetWindowInventory(Window* wnd) {
	LOCKCLASS lockClass(lockGame);

	this->wndInventory = wnd;
}

Window* Game::GetWindowInventory() {
	return wndInventory;
}

void Game::SetWindowStatistics(Window* wnd) {
	LOCKCLASS lockClass(lockGame);

	this->wndStatistics = wnd;
}

Window* Game::GetWindowStatistics() {
	return wndStatistics;
}

void Game::SetWindowBattle(Window* wnd) {
	LOCKCLASS lockClass(lockGame);

	this->wndBattle = wnd;
}

Window* Game::GetWindowBattle() {
	return wndBattle;
}

void Game::SetWindowVIPList(Window* wnd) {
	LOCKCLASS lockClass(lockGame);

	this->wndVIPList = wnd;
}

Window* Game::GetWindowVIPList() {
	return wndVIPList;
}

void Game::SetConsoleTab(WindowElementTab* tab) {
	this->consoleTab = tab;
}

WindowElementTab* Game::GetConsoleTab() {
	return consoleTab;
}

void Game::SetCmdLine(WindowElementTextarea* textarea) {
	this->cmdLine = textarea;
}

WindowElementTextarea* Game::GetCmdLine() {
	return cmdLine;
}


Window* Game::GetActiveWindow() {
	LOCKCLASS lockClass(lockGame);

	if (wnds) {
		Window* active = wnds->GetTopWindow();
		return active;
	}

	return NULL;
}

WindowElement* Game::GetActiveWindowElement() {
	LOCKCLASS lockClass(lockGame);

	if (wnds) {
		Window* active = wnds->GetTopWindow();
		if (active) {
			WindowElement* wndElement = active->GetActiveElement();
			return wndElement;
		}
	}

	return NULL;
}


void Game::SetHost(Host host) {
	LOCKCLASS lockClass(lockGame);

	this->host = host;
}

Host Game::GetHost() {
	return host;
}

Host* Game::GetHostPtr() {
	return &host;
}


void Game::SetCharacter(Character character) {
	LOCKCLASS lockClass(lockGame);

	this->character = character;
}

Character Game::GetCharacter() {
	return character;
}


void Game::AddCharacter(Character character) {
	LOCKCLASS lockClass(lockGame);

	charactersList.push_back(character);
}

CharactersList Game::GetCharacters() {
	LOCKCLASS lockClass(lockGame);

	return charactersList;
}

void Game::ClearCharactersList() {
	LOCKCLASS lockClass(lockGame);

	charactersList.clear();
}

void Game::AddChannel(Channel channel) {
	LOCKCLASS lockClass(lockGame);

	channelsList.push_back(channel);
}

ChannelsList Game::GetChannels() {
	LOCKCLASS lockClass(lockGame);

	return channelsList;
}

void Game::ClearChannelsList() {
	LOCKCLASS lockClass(lockGame);

	channelsList.clear();
}

void Game::OpenChannel(unsigned short channelID, std::string channelName, ChatUsers& users, ChatUsers& invited, bool closeable) {
	Channel* channel = NULL;
	if (channelID != CHANNEL_PRIVATE)
		channel = Channel::GetChannel(channelID);
	else
		channel = Channel::GetChannel(channelName);

	Window* wndConsole = GetWindowConsole();
	WindowElementTab* consoleTab = GetConsoleTab();
	if (!channel && wndConsole && consoleTab) {
		TabElement elem = consoleTab->AddTab(channelName, closeable);
		POINT size = elem.second->GetSize();

		int usersListWidth = 0;
		if (channelID < 0xFF00)
			usersListWidth = 16;

		WindowElementTextarea* textarea = new(M_PLACE) WindowElementTextarea;
		textarea->Create(0, 0, 0, size.x - usersListWidth, size.y, false, true, wndConsole->GetWindowTemplate());
		textarea->SetFontSize(options.textSize);
		textarea->SetOffset(16);
		textarea->SetBorder(1);
		textarea->SetLocks(true, true);

		elem.second->AddElement(textarea);

		channel = new(M_PLACE) Channel(channelID, channelName);
		channel->SetTextarea((void*)textarea);
		channel->SetButton((void*)elem.first);
		Channel::AddChannel(channel, (bool)(channelID == CHANNEL_PRIVATE));

		if (usersListWidth) {
			WindowElementMemo* memo = new(M_PLACE) WindowElementMemo;
			memo->Create(ALIGN_H_RIGHT, size.x - 16, 0, 0, size.y, wndConsole->GetWindowTemplate());
			memo->SetBorder(1);

			WindowElementButton* slider = new(M_PLACE) WindowElementButton;
			slider->Create(ALIGN_H_RIGHT, BUTTON_NORMAL, size.x - 16, 0, 16, size.y, wndConsole->GetWindowTemplate());
			slider->SetText("<");
			slider->SetLocks(false, true);
			slider->SetAction(boost::bind(&Game::onSlideChannelUsers, this, slider, textarea, memo));

			elem.second->AddElement(slider);
			elem.second->AddElement(memo);

			channel->SetMemo((void*)memo);

			for (ChatUsers::iterator it = users.begin(); it != users.end(); it++) {
				channel->AddUser(*it);
				memo->AddElement(*it, true);
			}
			for (ChatUsers::iterator it = invited.begin(); it != invited.end(); it++) {
				channel->AddInvited(*it);
				memo->AddElement(*it, false);
			}
		}
	}
	else if (channel && wndConsole && consoleTab) {
		if (channelID == CHANNEL_NPC)
			consoleTab->SetActiveTab((WindowElementButton*)channel->GetButton());

		consoleTab->RenameTab((WindowElementButton*)channel->GetButton(), channelName);

		WindowElementMemo* memo = (WindowElementMemo*)channel->GetMemo();
		if (memo) {
			memo->Clear();
			channel->ClearUsers();
			channel->ClearInvited();
			for (ChatUsers::iterator it = users.begin(); it != users.end(); it++) {
				channel->AddUser(*it);
				memo->AddElement(*it, true);
			}
			for (ChatUsers::iterator it = invited.begin(); it != invited.end(); it++) {
				channel->AddInvited(*it);
				memo->AddElement(*it, false);
			}
		}
	}
	else if (!wndConsole)
		Logger::AddLog("Game::OpenChannel", "Console window not found!", LOG_WARNING);
	else if (!consoleTab)
		Logger::AddLog("Game::OpenChannel", "Console tab not found!", LOG_WARNING);
}

void Game::CloseChannel(unsigned short channelID) {
	Channel* channel = Channel::GetChannel(channelID);

	Window* wndConsole = GetWindowConsole();
	WindowElementTab* consoleTab = GetConsoleTab();
	if (channel && wndConsole && consoleTab)
		consoleTab->RemoveTab((WindowElementButton*)channel->GetButton());
    else if (!channel)
        Logger::AddLog("Game::CloseChannel", "Console channel not found!", LOG_WARNING);
	else if (!wndConsole)
		Logger::AddLog("Game::CloseChannel", "Console window not found!", LOG_WARNING);
	else if (!consoleTab)
		Logger::AddLog("Game::CloseChannel", "Console tab not found!", LOG_WARNING);

	Channel::RemoveChannel(channelID);
}

void Game::CloseChannel(std::string channelName) {
	Channel* channel = Channel::GetChannel(channelName);

	Window* wndConsole = GetWindowConsole();
	WindowElementTab* consoleTab = GetConsoleTab();
	if (channel && wndConsole && consoleTab)
		consoleTab->RemoveTab((WindowElementButton*)channel->GetButton());
	else if (!wndConsole)
		Logger::AddLog("Game::CloseChannel", "Console window not found!", LOG_WARNING);
	else if (!consoleTab)
		Logger::AddLog("Game::CloseChannel", "Console tab not found!", LOG_WARNING);

	Channel::RemoveChannel(channelName);
}


void Game::SetProtocol(Protocol* protocol) {
	LOCKCLASS lockClass(lockGame);

	this->protocol = protocol;
}

Protocol* Game::GetProtocol() {
	return protocol;
}


void Game::SetMap(Map* map) {
	LOCKCLASS lockClass(lockGame);

	this->map = map;
}

Map* Game::GetMap() {
	return this->map;
}


void Game::SetPlayer(Player* player) {
	LOCKCLASS lockClass(lockGame);

	this->player = player;
}

Player* Game::GetPlayer() {
	return player;
}


void Game::SetVIPList(VIPList* viplist) {
	LOCKCLASS lockClass(lockGame);

	this->viplist = viplist;
}

VIPList* Game::GetVIPList() {
	return viplist;
}


void Game::SetShop(Shop* shop) {
	LOCKCLASS lockClass(lockGame);

	this->shop = shop;
}

Shop* Game::GetShop() {
	return shop;
}


void Game::SetTrade(Trade* trade) {
	LOCKCLASS lockClass(lockGame);

	this->trade = trade;
}

Trade* Game::GetTrade() {
	return trade;
}


void Game::SetCraftBox(CraftBox* craftbox) {
	LOCKCLASS lockClass(lockGame);

	this->craftbox = craftbox;
}

CraftBox* Game::GetCraftBox() {
	return craftbox;
}


void Game::SetCooldowns(Cooldowns* cooldowns) {
	LOCKCLASS lockClass(lockGame);

	this->cooldowns = cooldowns;
}

Cooldowns* Game::GetCooldowns() {
	return cooldowns;
}


void Game::SetQuestLog(QuestLog* questlog) {
	LOCKCLASS lockClass(lockGame);

	this->questlog = questlog;
}

QuestLog* Game::GetQuestLog() {
	return questlog;
}


void Game::SetStatus(Status* status) {
	LOCKCLASS lockClass(lockGame);

	this->status = status;
}

Status* Game::GetStatus() {
	return status;
}


void Game::SetUpdater(Updater* updater) {
	this->updater = updater;
}

Updater* Game::GetUpdater() {
	return updater;
}

void Game::SetServers(Servers* servers) {
	this->servers = servers;
}

Servers* Game::GetServers() {
	return servers;
}

void Game::SetBot(Bot* bot) {
	this->bot = bot;
}

Bot* Game::GetBot() {
	return bot;
}


void Game::SetGameState(unsigned char state) {
	LOCKCLASS lockClass(lockGame);

	gameState = state;
}

unsigned char Game::GetGameState() {
	return gameState;
}

void Game::SetWorldLight(unsigned char worldLightLevel, unsigned char worldLightColor) {
	LOCKCLASS lockClass(lockGame);

	Lights::SetGlobalColor(worldLightColor);
	Lights::SetGlobalLevel(worldLightLevel);
}

unsigned char Game::GetWorldLightLevel() {
	return Lights::GetGlobalLevel();
}

unsigned char Game::GetWorldLightColor() {
	return Lights::GetGlobalColor();
}


void Game::SetMouse(Mouse* mouse) {
	LOCKCLASS lockClass(lockGame);

	this->mouse = mouse;
}

Mouse* Game::GetMouse() {
	return mouse;
}

void Game::SetKeyboard(Keyboard* keyboard) {
	LOCKCLASS lockClass(lockGame);

	this->keyboard = keyboard;
}

Keyboard* Game::GetKeyboard() {
	return keyboard;
}


void Game::LoadData() {
	if (Item::version != dataLocation) {
		Item::ReleaseItems();
		if (Game::options.protocol < 860) {
			if (!Item::LoadItemsFromDatFIRST(dataLocation + "/items.dat", dataLocation))
				Item::LoadItemsFromDatFIRST(dataLocation + "/Tibia.dat", dataLocation);
		}
		else if (Game::options.protocol >= 860 && Game::options.protocol < 910) {
			if (!Item::LoadItemsFromDatSECOND(dataLocation + "/items.dat", dataLocation))
				Item::LoadItemsFromDatSECOND(dataLocation + "/Tibia.dat", dataLocation);
		}
		else {
			if (!Item::LoadItemsFromDatTHIRD(dataLocation + "/items.dat", dataLocation))
				Item::LoadItemsFromDatTHIRD(dataLocation + "/Tibia.dat", dataLocation);
		}
	}

	if (Sprites::version != dataLocation) {
		Sprites::ReleaseSprites();
		Sprites::ReleaseSpritesData();
		if (!Sprites::LoadSpritesFromSpr(dataLocation + "/items.spr", dataLocation))
			Sprites::LoadSpritesFromSpr(dataLocation + "/Tibia.spr", dataLocation);
	}

	if (particles) {
		AddTask(boost::bind(&Particles::ReleaseParticles, particles));
		AddTask(boost::bind(&Particles::LoadParticles, particles, std::string(dataLocation + "/particles/"), Item::GetEffectsCount()));
	}

	if (bot) {
		bot->ReleaseBot();
		bot->LoadBot(filesLocation + "/bot/");
	}

#ifndef NO_SOUND
	if (sfx)
		sfx->ReleaseSamples();
#endif

	LoadHotKeys();
	LoadMarkers();
	LoadWaypoints();
}


void Game::SetHotKey(unsigned char key, HotKey hotkey) {
	this->hotkey[key] = hotkey;
}

HotKey* Game::GetHotKey(unsigned char key) {
	return &hotkey[key];
}

void Game::ClearHotKey(unsigned char key) {
	if (hotkey[key].itemContainer)
		hotkey[key].itemContainer->SetItem(NULL);

	hotkey[key].keyChar = 0;
	hotkey[key].text = "";
	hotkey[key].itemID = 0;
	hotkey[key].fluid = 0;
	hotkey[key].spellID = 0;
	hotkey[key].mode = 0;

	hotkey[key].disabledItemChange = false;
}

void Game::ExecuteHotKey(unsigned char key) {
	LOCKCLASS lockClass1(lockGame);
	LOCKCLASS lockClass2(Windows::lockWindows);

	if (!mouse || !keyboard)
		return;

	WindowElementItemContainer* itemContainer = (WindowElementItemContainer*)hotkey[key].itemContainer;
	Item* item = (itemContainer ? itemContainer->GetItem() : NULL);
	if (item) {
		Position pos(0xFFFF, 0x0000, 0x00);

		if ((hotkey[key].mode & 0x03) == 1) {
			PlayerUseBattleThing(pos, item->GetID(), 0, Player::GetCreatureID());
		}
		else if ((hotkey[key].mode & 0x03) == 2) {
			PlayerUseBattleThing(pos, item->GetID(), 0, Player::GetAttackID());
		}
		else if ((hotkey[key].mode & 0x03) == 3) {
			HOLDER holder = mouse->GetHolder();
			if (holder.type == 0x00) {
				Position* newPos = new(M_PLACE) Position(pos.x, pos.y, pos.z);
				Thing* newThing = new(M_PLACE) Item(item);
				int itemId = item->GetID();
				int stackPos = 0;

				mouse->SetHolder(itemId, stackPos,
					0x51, NULL, (void*)newPos, (void*)newThing);
			}
		}
	}

	if (item && hotkey[key].text != "") {
		Channel* ch = Channel::GetChannel(CHANNEL_DEFAULT);
		PlayerSendSay(SPEAK_SAY, ch, hotkey[key].text);
	}
	else if (!item && hotkey[key].text != "") {
		if ((hotkey[key].mode & 0x03) == 1) {
			if (cmdLine)
				cmdLine->SetText(hotkey[key].text);
		}
		else if ((hotkey[key].mode & 0x03) == 2) {
			Channel* ch = Channel::GetChannel(CHANNEL_DEFAULT);
			PlayerSendSay(SPEAK_SAY, ch, hotkey[key].text);
		}
	}
}

void Game::SaveHotKeys() {
	for (int i = 0; i < 48; i++) {
		std::string filename = filesLocation + std::string("/hotkeys/hotkey") + value2str(i) + std::string(".ini");
		remove(filename.c_str());

		INILoader iniHotKey;

		HotKey* hk = GetHotKey(i);
		unsigned short keyChar = hk->keyChar;
		std::string text = hk->text;
		unsigned short itemID = hk->itemID;
		unsigned char fluid = hk->fluid;
		unsigned char spellID = hk->spellID;
		unsigned char mode = hk->mode;

		if (text != "" || itemID != 0) {
			std::string prem_text = "\"" + text;
			iniHotKey.SetValue("HOTKEY", value2str(keyChar));
		    iniHotKey.SetValue("TEXT", prem_text);
	    	iniHotKey.SetValue("ITEM", value2str(itemID), 0);
	    	if (fluid) iniHotKey.SetValue("ITEM", value2str(fluid), 1);
	    	iniHotKey.SetValue("SPELL", value2str(spellID));
		    iniHotKey.SetValue("MODE", value2str(mode));
		    iniHotKey.SaveFile(filename);
		}
	}
}

void Game::LoadHotKeys() {
	for (int i = 0; i < 48; i++) {
        std::string filename = filesLocation + std::string("/hotkeys/hotkey") + value2str(i) + std::string(".ini");

        ClearHotKey(i);

		INILoader iniHotKey;
		if (!iniHotKey.OpenFile(filename))
			continue;

        HotKey* hk = GetHotKey(i);
		hk->keyChar = atoi(iniHotKey.GetValue("HOTKEY").c_str());
		hk->text = iniHotKey.GetValue("TEXT");
		hk->itemID = atoi(iniHotKey.GetValue("ITEM", 0).c_str());
		hk->fluid = atoi(iniHotKey.GetValue("ITEM", 1).c_str());
		hk->spellID = atoi(iniHotKey.GetValue("SPELL").c_str());
		hk->mode = atoi(iniHotKey.GetValue("MODE").c_str());
	}
}

void Game::SaveMarkers() {
	LOCKCLASS lockClass(lockGame);

	MiniMap* mmap = NULL;
	if (map)
		mmap = map->GetMiniMap();

	if (!mmap)
		return;

	std::string filename = filesLocation + std::string("/minimap/markers.ini");

	INILoader iniMarkers;

	MarkersMap markers = mmap->GetMarkers();
	MarkersMap::iterator it = markers.begin();
	int i = 0;
	for (it; it != markers.end(); it++, i++) {
		Position pos = it->first;
		Marker marker = it->second;

		std::string key = std::string("MARKER") + value2str(i);
		iniMarkers.SetValue(key, value2str(pos.x), 0);
		iniMarkers.SetValue(key, value2str(pos.y), 1);
		iniMarkers.SetValue(key, value2str(pos.z), 2);
		iniMarkers.SetValue(key, value2str(marker.first), 3);
		iniMarkers.SetValue(key, marker.second, 4);
	}

	iniMarkers.SaveFile(filename);
}

void Game::LoadMarkers() {
	LOCKCLASS lockClass(lockGame);

	MiniMap* mmap = NULL;
	if (map)
		mmap = map->GetMiniMap();

	if (!mmap)
		return;

	mmap->ClearMarkers();

	std::string filename = filesLocation + std::string("/minimap/markers.ini");

	INILoader iniMarkers;
	if (!iniMarkers.OpenFile(filename))
		return;

	int i = 0;
	while(iniMarkers.GetValue(i, 0) != "") {
		int x = atoi(iniMarkers.GetValue(i, 0).c_str());
		int y = atoi(iniMarkers.GetValue(i, 1).c_str());
		int z = atoi(iniMarkers.GetValue(i, 2).c_str());
		int id = atoi(iniMarkers.GetValue(i, 3).c_str());
		std::string desc = iniMarkers.GetValue(i, 4);

		Position pos(x, y, z);
		Marker marker(id, desc);

		mmap->AddMarker(pos, marker);

		i++;
	}
}

void Game::SaveWaypoints(std::string filename) {
	LOCKCLASS lockClass(lockGame);

	MiniMap* mmap = NULL;
	if (map)
		mmap = map->GetMiniMap();

	if (!mmap)
		return;

	if (filename == "")
		filename = filesLocation + std::string("/minimap/waypoints.ini");
	else {
		CreateDirectory(std::string(filesLocation + "/minimap/waypoints").c_str(), NULL);
		filename = filesLocation + std::string("/minimap/waypoints/") + filename;
	}

	INILoader iniWaypoints;

	WaypointsList waypoints = mmap->GetWaypoints();
	WaypointsList::iterator it = waypoints.begin();
	int i = 0;
	for (it; it != waypoints.end(); it++, i++) {
		Position pos = it->first;
		Waypoint waypoint = it->second;

		std::string key = std::string("WAYPOINT") + value2str(i);
		iniWaypoints.SetValue(key, value2str(pos.x), 0);
		iniWaypoints.SetValue(key, value2str(pos.y), 1);
		iniWaypoints.SetValue(key, value2str(pos.z), 2);
		iniWaypoints.SetValue(key, value2str(waypoint.first), 3);
		iniWaypoints.SetValue(key, waypoint.second, 4);
	}

	iniWaypoints.SaveFile(filename);
}

void Game::LoadWaypoints(std::string filename) {
	LOCKCLASS lockClass(lockGame);

	MiniMap* mmap = NULL;
	if (map)
		mmap = map->GetMiniMap();

	if (!mmap)
		return;

	mmap->ClearWaypoints();

	if (filename == "")
		filename = filesLocation + std::string("/minimap/waypoints.ini");
	else
		filename = filesLocation + std::string("/minimap/waypoints/") + filename;

	INILoader iniWaypoints;
	if (!iniWaypoints.OpenFile(filename))
		return;

	int i = 0;
	while(iniWaypoints.GetValue(i, 0) != "") {
		int x = atoi(iniWaypoints.GetValue(i, 0).c_str());
		int y = atoi(iniWaypoints.GetValue(i, 1).c_str());
		int z = atoi(iniWaypoints.GetValue(i, 2).c_str());
		unsigned char type = atoi(iniWaypoints.GetValue(i, 3).c_str());
		std::string comment = iniWaypoints.GetValue(i, 4);

		Position pos(x, y, z);
		if (!type)
			type = 1;

		mmap->AddWaypoint(pos, Waypoint(type, comment));

		i++;
	}
}


void Game::SaveOptions() {
	LOCKCLASS lockClass(lockGame);

	std::string filename = "ronclient.ini";

	INILoader iniOptions;
	iniOptions.SetValue("GRAPHICSAPI", value2str(options.graphicsAPI));
	iniOptions.SetValue("FPSLIMIT", value2str(options.limitFPS));
	iniOptions.SetValue("AMBIENT", value2str(options.ambientLight));
	iniOptions.SetValue("FIGHTMODES", value2str(options.fightModes));
	iniOptions.SetValue("LANGUAGE", value2str(options.language));
	iniOptions.SetValue("SHOWNAMES", value2str((int)options.showNames));
	iniOptions.SetValue("SHOWBARS", value2str((int)options.showBars));
	iniOptions.SetValue("DONTSTRECH", value2str((int)options.dontStrech));
	iniOptions.SetValue("PRINTPARTICLES", value2str((int)options.printParticles));
	iniOptions.SetValue("PRINTONELEVEL", value2str((int)options.printOneLevel));
	iniOptions.SetValue("RENDERTOTEXTURE", value2str((int)options.renderToTexture));
	iniOptions.SetValue("CLASSICCONTROLS", value2str((int)options.classicControls));
	iniOptions.SetValue("AUTOUPDATE", value2str((int)options.autoUpdate));
	iniOptions.SetValue("TEXTSIZE", value2str(options.textSize));
	iniOptions.SetValue("FLYINGSPEED", value2str(options.flyingSpeed));
	iniOptions.SetValue("DISAPPEARINGSPEED", value2str(options.disappearingSpeed));
	iniOptions.SetValue("FIXEDPOSITIONS", value2str((int)options.fixedPositions));
	iniOptions.SetValue("FADEOUTINACTIVE", value2str((int)options.fadeOutInactive));
	iniOptions.SetValue("MUSIC", value2str(options.musicGain));
	iniOptions.SetValue("SOUND", value2str(options.soundGain));
	iniOptions.SetValue("ACCOUNT", host.account);
	iniOptions.SetValue("HOST", host.host);
	iniOptions.SetValue("PORT", host.port);
	iniOptions.SetValue("UPDATEURL_DATA", host.update);
	iniOptions.SetValue("UPDATEURL_ENGINE", options.updateURL);
	iniOptions.SetValue("NAME", host.name);
	iniOptions.SetValue("PROTOCOL", host.protocol);
	iniOptions.SetValue("WEBSITE", host.website);
	iniOptions.SetValue("CREATEACC", host.createacc);
	iniOptions.SetValue("TEMPLATESGROUP", options.templatesGroup);

	std::list<std::pair<std::string, std::string> >::iterator it = openChannels.begin();
	int i = 0;
	for (it; it != openChannels.end(); it++, i++) {
		std::string channel = it->first;
		std::string name = it->second;
		iniOptions.SetValue("CHANNELS", channel, i * 2 + 0);
		iniOptions.SetValue("CHANNELS", name, i * 2 + 1);
	}

	iniOptions.SaveFile(filename);
}

void Game::LoadOptions() {
	LOCKCLASS lockClass(lockGame);

	std::string filename = "ronclient.ini";

	INILoader iniOptions;
	if (!iniOptions.OpenFile(filename))
		return;

	if (iniOptions.GetValue("GRAPHICSAPI") != "") options.graphicsAPI = atoi(iniOptions.GetValue("GRAPHICSAPI").c_str());
	if (iniOptions.GetValue("FPSLIMIT") != "") options.limitFPS = atoi(iniOptions.GetValue("FPSLIMIT").c_str());
	if (iniOptions.GetValue("AMBIENT") != "") options.ambientLight = atoi(iniOptions.GetValue("AMBIENT").c_str());
	if (iniOptions.GetValue("FIGHTMODES") != "") options.fightModes = atoi(iniOptions.GetValue("FIGHTMODES").c_str());
	if (iniOptions.GetValue("LANGUAGE") != "") options.language = atoi(iniOptions.GetValue("LANGUAGE").c_str());
	if (iniOptions.GetValue("SHOWNAMES") != "") options.showNames = atoi(iniOptions.GetValue("SHOWNAMES").c_str());
	if (iniOptions.GetValue("SHOWBARS") != "") options.showBars = atoi(iniOptions.GetValue("SHOWBARS").c_str());
	if (iniOptions.GetValue("DONTSTRECH") != "") options.dontStrech = atoi(iniOptions.GetValue("DONTSTRECH").c_str());
	if (iniOptions.GetValue("PRINTPARTICLES") != "") options.printParticles = atoi(iniOptions.GetValue("PRINTPARTICLES").c_str());
	if (iniOptions.GetValue("PRINTONELEVEL") != "") options.printOneLevel = atoi(iniOptions.GetValue("PRINTONELEVEL").c_str());
	if (iniOptions.GetValue("RENDERTOTEXTURE") != "") options.renderToTexture = atoi(iniOptions.GetValue("RENDERTOTEXTURE").c_str());
	if (iniOptions.GetValue("CLASSICCONTROLS") != "") options.classicControls = atoi(iniOptions.GetValue("CLASSICCONTROLS").c_str());
	if (iniOptions.GetValue("AUTOUPDATE") != "") options.autoUpdate = atoi(iniOptions.GetValue("AUTOUPDATE").c_str());

	if (iniOptions.GetValue("TEXTSIZE") != "") options.textSize = atoi(iniOptions.GetValue("TEXTSIZE").c_str());
	if (iniOptions.GetValue("FLYINGSPEED") != "") options.flyingSpeed = atoi(iniOptions.GetValue("FLYINGSPEED").c_str());
	if (iniOptions.GetValue("DISSAPEARINGSPEED") != "") options.disappearingSpeed = atoi(iniOptions.GetValue("DISSAPEARINGSPEED").c_str());

	if (iniOptions.GetValue("FIXEDPOSITIONS") != "") options.fixedPositions = atoi(iniOptions.GetValue("FIXEDPOSITIONS").c_str());
	if (iniOptions.GetValue("FADEOUTINACTIVE") != "") options.fadeOutInactive = atoi(iniOptions.GetValue("FADEOUTINACTIVE").c_str());

	if (iniOptions.GetValue("MUSIC") != "") options.musicGain = atoi(iniOptions.GetValue("MUSIC").c_str());
	if (iniOptions.GetValue("SOUND") != "") options.soundGain = atoi(iniOptions.GetValue("SOUND").c_str());

	if (iniOptions.GetValue("ACCOUNT") != "") host.account = iniOptions.GetValue("ACCOUNT");
	if (iniOptions.GetValue("HOST") != "") host.host = iniOptions.GetValue("HOST");
	if (iniOptions.GetValue("PORT") != "") host.port = iniOptions.GetValue("PORT");
	if (iniOptions.GetValue("UPDATEURL_DATA") != "") host.update = iniOptions.GetValue("UPDATEURL_DATA");

	if (iniOptions.GetValue("UPDATEURL") != "") options.updateURL = iniOptions.GetValue("UPDATEURL");
	if (iniOptions.GetValue("UPDATEURL_ENGINE") != "") options.updateURL = iniOptions.GetValue("UPDATEURL_ENGINE");

	if (iniOptions.GetValue("NAME") != "") host.name = iniOptions.GetValue("NAME");
	if (iniOptions.GetValue("PROTOCOL") != "") host.protocol = iniOptions.GetValue("PROTOCOL").c_str();
	if (iniOptions.GetValue("WEBSITE") != "") host.website = iniOptions.GetValue("WEBSITE");
	if (iniOptions.GetValue("CREATEACC") != "") host.createacc = iniOptions.GetValue("CREATEACC");

	if (iniOptions.GetValue("TEMPLATESGROUP") != "") options.templatesGroup = iniOptions.GetValue("TEMPLATESGROUP");

	if (options.ambientLight > 50)
		options.ambientLight = 50;

	options.protocol = atoi(host.protocol.c_str());

	openChannels.clear();
	std::string channel;
	int i = 0;
	while((channel = iniOptions.GetValue("CHANNELS", 2 * i + 0)) != "") {
		std::string name = iniOptions.GetValue("CHANNELS", 2 * i + 1);
		openChannels.push_back(std::pair<std::string, std::string>(channel, name));

		i++;
	}

	onSounds(NULL, NULL);
}

void Game::PreLoadOptions() {
	LOCKCLASS lockClass(lockGame);

	std::string filename = "ronclient.ini";

	INILoader iniOptions;
	if (!iniOptions.OpenFile(filename))
		return;

	if (iniOptions.GetValue("GRAPHICSAPI") != "") options.graphicsAPI = atoi(iniOptions.GetValue("GRAPHICSAPI").c_str());
	if (iniOptions.GetValue("LANGUAGE") != "") options.language = atoi(iniOptions.GetValue("LANGUAGE").c_str());
}

void Game::SaveAdminOptions() {
	LOCKCLASS lockClass(lockGame);

	FileManager* files = FileManager::fileManager;
	if (!files)
		return;

	std::string filename = "ronclient_admin.ini";

	FileInfo info = files->GetFileInfo(filename);
	if (info.pakFile != "")
		return;

	INILoader iniOptions;
	iniOptions.SetValue("DISABLEBOT", value2str(adminOptions.disableBot));
	iniOptions.SetValue("HIDELEVELS", value2str(adminOptions.hideLevels));
	iniOptions.SetValue("FULLLIGHT", value2str(adminOptions.fullLight));
	iniOptions.SetValue("SIMPLELOGIN", value2str(adminOptions.simpleLogin));
	iniOptions.SetValue("RSA_P", adminOptions.rsa_p);
	iniOptions.SetValue("RSA_Q", adminOptions.rsa_q);
	iniOptions.SetValue("RSA_D", adminOptions.rsa_d);
	iniOptions.SaveFile(filename);
}

void Game::LoadAdminOptions() {
	LOCKCLASS lockClass(lockGame);

	std::string filename = "ronclient_admin.ini";

	INILoader iniOptions;
	if (!iniOptions.OpenFile(filename))
		return;

	if (iniOptions.GetValue("DISABLEBOT") != "") adminOptions.disableBot = atoi(iniOptions.GetValue("DISABLEBOT").c_str());
	if (iniOptions.GetValue("HIDELEVELS") != "") adminOptions.hideLevels = atoi(iniOptions.GetValue("HIDELEVELS").c_str());
	if (iniOptions.GetValue("FULLLIGHT") != "") adminOptions.fullLight = atoi(iniOptions.GetValue("FULLLIGHT").c_str());
	if (iniOptions.GetValue("SIMPLELOGIN") != "") adminOptions.simpleLogin = atoi(iniOptions.GetValue("SIMPLELOGIN").c_str());
	if (iniOptions.GetValue("RSA_P") != "") adminOptions.rsa_p = iniOptions.GetValue("RSA_P");
	if (iniOptions.GetValue("RSA_Q") != "") adminOptions.rsa_q = iniOptions.GetValue("RSA_Q");
	if (iniOptions.GetValue("RSA_D") != "") adminOptions.rsa_d = iniOptions.GetValue("RSA_D");
}

void Game::SaveChannelText(Channel* channel) {
	LOCKCLASS lockClass(lockGame);

	std::string filename = channel->GetName() + ".txt";
	FILE* file = fopen(filename.c_str(), "wb");
	if (!file)
		return;

	WindowElementTextarea* textarea = (WindowElementTextarea*)channel->GetTextarea();
	std::string text = textarea->GetText();
	fwrite(text.c_str(), text.length(), 1, file);

	fclose(file);
}

void Game::SaveChannels() {
	LOCKCLASS lockClass(lockGame);

	openChannels.clear();
	std::map<unsigned short, Channel*> channels = Channel::GetChannels();
	std::map<unsigned short, Channel*>::iterator it = channels.begin();
	for (it; it != channels.end(); it++) {
		Channel* ch = it->second;
		if (ch->GetID() > CHANNEL_DEFAULT && ch->GetID() < CHANNEL_NPC)
			openChannels.push_back(std::pair<std::string, std::string>(value2str(ch->GetID()), ch->GetName()));
	}
}

void Game::LoadChannels() {
	LOCKCLASS lockClass(lockGame);

	std::list<std::pair<std::string, std::string> >::iterator it = openChannels.begin();
	for (it; it != openChannels.end(); it++) {
		unsigned short channelID = atoi(it->first.c_str());
		std::string channelName = it->second;
		PlayerOpenChannel(channelID, channelName);

		ChatUsers users;
		ChatUsers invited;
		OpenChannel(channelID, channelName, users, invited);
	}

	Channel* channel = Channel::GetChannel(CHANNEL_SERVER_LOG);
	WindowElementTab* consoleTab = GetConsoleTab();
	if (channel && consoleTab)
		consoleTab->SetActiveTab((WindowElementButton*)channel->GetButton());
}

void Game::SaveErrorLog(std::string filename) {
	FILE* fromFile = fopen(filename.c_str(), "rb");
	if (!fromFile)
		return;

	FILE* toFile = fopen("ronclient.err", "wb");
	if (!toFile) {
		fclose(fromFile);
		return;
	}

	Host host = GetHost();
	Character character = GetCharacter();

	uint16_t length = character.name.length();
	fwrite(&length, 2, 1, toFile);
	fwrite(character.name.c_str(), 1, length, toFile);
	length = host.name.length();
	fwrite(&length, 2, 1, toFile);
	fwrite(host.name.c_str(), 1, length, toFile);

	fwrite(&character.servIP, 4, 1, toFile);
	fwrite(&character.servPort, 2, 1, toFile);

	fclose(toFile);

	toFile = fopen("logs\\error.log", "wb");
	if (!toFile) {
		fclose(fromFile);
		return;
	}

	while(!feof(fromFile)) {
		char ch = fgetc(fromFile);
		if (!feof(fromFile))
			fputc(ch, toFile);
	}

	fclose(fromFile);
	fclose(toFile);
}

void Game::LoadErrorLog() {
	FILE* fromFile = fopen("ronclient.err", "rb");
	if (!fromFile)
		return;

	Host host;
	Character character;

	std::string charName;
	std::string hostName;

	uint16_t length;
	char temp[4096];

	memset(temp, 0, 4096);
	fread(&length, 2, 1, fromFile);
	fread(temp, 1, length, fromFile);
	charName = temp;
	if (charName == "") charName = "undefined";

	memset(temp, 0, 4096);
	fread(&length, 2, 1, fromFile);
	fread(temp, 1, length, fromFile);
	hostName = temp;
	if (hostName == "") hostName = "undefined";

	fread(&character.servIP, 4, 1, fromFile);
	fread(&character.servPort, 2, 1, fromFile);

	fclose(fromFile);

	fromFile = fopen("logs\\error.log", "rb");
	if (!fromFile)
		return;

	std::string error_log;
	while(!feof(fromFile)) {
		char ch = fgetc(fromFile);
		if (!feof(fromFile)) {
			if (ch == '\t') error_log.insert(error_log.length(), "  ");
			else error_log.push_back(ch);
		}
	}

	fclose(fromFile);
	remove("ronclient.err");

#ifdef DEBUG_VERSION
	if (updater) {
		std::string title = "Debug Assertion: " + std::string(APP_NAME) + " " + std::string(APP_VERSION);
		updater->SendErrorReport(APP_INTERACT_ADDRESS, charName + "/" + hostName, &title, &error_log);
	}
#endif
	//Windows* wnds = GetWindows();
	//if (wnds)
	//	wnds->OpenWindow(WND_ERROR, this, &host, &character, error_log.c_str());
}


void Game::SendSystemMessage(std::string message) {
	LOCKCLASS lockClass(lockGame);

	Channel* ch = Channel::GetChannel(CHANNEL_SERVER_LOG);
	if (ch)
		ch->AddMessage(0, "", TextString(message, 215));

	int disappearingSpeed = Game::options.disappearingSpeed * 100;

	SystemMessage* sysMsg = new(M_PLACE) SystemMessage(TextString(message, 215), disappearingSpeed + message.length() * 75, 0.0f, 1.0f);
	Messages::AddMessage(sysMsg);
}


void Game::PlayerLogout() {
	LOCKCLASS lockClass(lockGame);

	protocol->SendLogout();
}

void Game::PlayerAutoWalk(std::list<Direction>& list) {
	LOCKCLASS lockClass(lockGame);

	if (list.size() > 0 && list.size() <= 255) {
		Player::walking = true;
		Player::requestStop = false;
		protocol->SendAutoWalk(list);
		player->SetAutoPath(list);
	}
	else {
		Player::walking = false;
		Player::requestStop = false;
		player->SetTargetPos(Position(0, 0, 0));

		if (list.size() > 255)
			SendSystemMessage(Text::GetText("SYSTEM_MESSAGE_1", Game::options.language));
		else
			SendSystemMessage(Text::GetText("SYSTEM_MESSAGE_0", Game::options.language));
	}
}

void Game::PlayerMove(Direction direction) {
	LOCKCLASS lockClass1(lockGame);
	LOCKCLASS lockClass2(Map::lockMap);
	LOCKCLASS lockClass3(Tile::lockTile);

	Creature* creature = player->GetCreature();

	if (!creature)
		return;

	if (creature->GetStep() != 0.0f || creature->newPos != Position(0, 0, 0)) {
		if (creature->GetDirection() != direction || (creature->GetStep() > -0.5f && creature->GetStep() < 0.5f))
			player->SetNextMove(direction);
		return;
	}

	creature->newPos = creature->pos;
	if (direction == NORTH) {
		creature->newPos.y--;
	}
	else if (direction == SOUTH) {
		creature->newPos.y++;
	}
	else if (direction == WEST) {
		creature->newPos.x--;
	}
	else if (direction == EAST) {
		creature->newPos.x++;
	}
	else if (direction == NORTHWEST) {
		creature->newPos.y--;
		creature->newPos.x--;
	}
	else if (direction == NORTHEAST) {
		creature->newPos.y--;
		creature->newPos.x++;
	}
	else if (direction == SOUTHWEST) {
		creature->newPos.y++;
		creature->newPos.x--;
	}
	else if (direction == SOUTHEAST) {
		creature->newPos.y++;
		creature->newPos.x++;
	}

	Tile* tile = map->GetTile(creature->newPos);
	if (tile && !tile->IsBlocking(!creature->GetBlocking(), true)) {
		creature->SetDirection(direction);

		map->RemoveTempCreature(creature->oldPos, creature);
		if (creature->GetLastMove() == SOUTHWEST)
			map->RemoveTempCreature(Position(creature->oldPos.x, creature->oldPos.y + 1, creature->oldPos.z), creature);
		else if (creature->GetLastMove() == NORTHEAST)
			map->RemoveTempCreature(Position(creature->oldPos.x + 1, creature->oldPos.y, creature->oldPos.z), creature);

		map->AddTempCreature(creature->newPos, creature);
		if (direction == SOUTHWEST)
			map->AddTempCreature(Position(creature->newPos.x + 1, creature->newPos.y, creature->newPos.z), creature);
		else if (direction == NORTHEAST)
			map->AddTempCreature(Position(creature->newPos.x, creature->newPos.y + 1, creature->newPos.z), creature);

		creature->SetStep(1.0f);

		protocol->SendWalk(direction);

		Player* player = GetPlayer();
		if (player) {
			player->SetTargetPos(Position(0, 0, 0));
			player->SetFollowID(0x00);
			Player::walking = true;
		}
	}
	else {
		if (!tile)
			protocol->SendWalk(direction);

		creature->newPos = Position(0, 0, 0);

		if (Player::walking)
			PlayerStopWalk();

		player->SetTargetPos(Position(0, 0, 0));
		player->SetFollowID(0x00);
	}

	player->PopNextMove();
}

void Game::PlayerTurn(Direction direction) {
	LOCKCLASS lockClass(lockGame);

	protocol->SendTurn(direction);
}

void Game::PlayerStopAutoWalk() {
	LOCKCLASS lockClass(lockGame);

	Player::walking = false;
	Player::requestStop = false;
	player->SetTargetPos(Position(0, 0, 0));

	protocol->SendStopWalk();
}

void Game::PlayerStopWalk() {
	LOCKCLASS lockClass(lockGame);

	Player::requestStop = true;
	protocol->SendStopWalk();
}

void Game::PlayerSendSay(unsigned char speakClass, Channel* channel, std::string message) {
	LOCKCLASS lockClass(lockGame);

	unsigned char sclass = (unsigned char)ConvertSpeakClasses(speakClass, protocol->GetVersion());
	protocol->SendSay(sclass, channel, message);
}

void Game::PlayerLookAt(Position pos, unsigned short itemId, unsigned char stackPos) {
	LOCKCLASS lockClass(lockGame);

	protocol->SendLookAt(pos, itemId, stackPos);
}

void Game::PlayerMoveThing(Position fromPos, unsigned short itemId, unsigned char stackPos, Position toPos, unsigned char count) {
	LOCKCLASS lockClass(lockGame);

	protocol->SendMoveThing(fromPos, itemId, stackPos, toPos, count);
}

void Game::PlayerUseThing(Position fromPos, unsigned short itemId, unsigned char stackPos, unsigned char index) {
	LOCKCLASS lockClass(lockGame);

	protocol->SendUseThing(fromPos, itemId, stackPos, index);
}

void Game::PlayerUseWithThing(Position fromPos, unsigned short fromItemId, unsigned char fromStackPos, Position toPos, unsigned short toItemId, unsigned char toStackPos) {
	LOCKCLASS lockClass(lockGame);

	protocol->SendUseWithThing(fromPos, fromItemId, fromStackPos, toPos, toItemId, toStackPos);
}

void Game::PlayerUseBattleThing(Position fromPos, unsigned short itemId, unsigned char stackPos, unsigned int creatureID) {
	LOCKCLASS lockClass(lockGame);

	protocol->SendUseBattleThing(fromPos, itemId, stackPos, creatureID);
}

void Game::PlayerRotateItem(Position pos, unsigned short itemId, unsigned char stackPos) {
	LOCKCLASS lockClass(lockGame);

	protocol->SendRotateItem(pos, itemId, stackPos);
}

void Game::PlayerRequestTrade(Position pos, unsigned short itemId, unsigned char stackPos, unsigned int creatureID) {
	LOCKCLASS lockClass(lockGame);

	protocol->SendRequestTrade(pos, itemId, stackPos, creatureID);
}

void Game::PlayerLookInTrade(unsigned char counterOffer, unsigned char index) {
	LOCKCLASS lockClass(lockGame);

	protocol->SendLookInTrade(counterOffer, index);
}

void Game::PlayerAcceptTrade() {
	LOCKCLASS lockClass(lockGame);

	protocol->SendAcceptTrade();
}

void Game::PlayerCancelTrade() {
	LOCKCLASS lockClass(lockGame);

	if (trade) {
		Container* tradeP1 = trade->GetContainerP1();
		Container* tradeP2 = trade->GetContainerP2();
		if (tradeP1)
			delete_debug(tradeP1, M_PLACE);
		if (tradeP2)
			delete_debug(tradeP2, M_PLACE);

		trade->SetContainerP1(NULL);
		trade->SetContainerP2(NULL);
	}

	protocol->SendCancelTrade();
}

void Game::PlayerLookInShop(unsigned short itemID, unsigned char type) {
	LOCKCLASS lockClass(lockGame);

	protocol->SendLookInShop(itemID, type);
}

void Game::PlayerPurchaseShop(unsigned short itemID, unsigned char type, unsigned char count, bool ignoreCap, bool inBackpack) {
	LOCKCLASS lockClass(lockGame);

	protocol->SendPurchaseShop(itemID, type, count, ignoreCap, inBackpack);
}

void Game::PlayerSaleShop(unsigned short itemID, unsigned char type, unsigned char count) {
	LOCKCLASS lockClass(lockGame);

	protocol->SendSaleShop(itemID, type, count);
}

void Game::PlayerCloseShop() {
	LOCKCLASS lockClass(lockGame);

	protocol->SendCloseShop();
}

void Game::PlayerContainerClose(unsigned char index) {
	LOCKCLASS lockClass(lockGame);

	protocol->SendContainerClose(index);
}

void Game::PlayerContainerMoveUp(unsigned char index) {
	LOCKCLASS lockClass(lockGame);

	protocol->SendContainerMoveUp(index);
}

void Game::PlayerToggleMount(bool mount) {
	LOCKCLASS lockClass(lockGame);

	protocol->SendToggleMount(mount);
}

void Game::PlayerSetFightModes(unsigned char fightModes) {
	LOCKCLASS lockClass(lockGame);

	unsigned char fight = 1;
	unsigned char follow = 0;
	unsigned char attack = 0;
	if (fightModes & (0x01 << 0)) fight = 1;
	else if (fightModes & (0x01 << 1)) fight = 2;
	else if (fightModes & (0x01 << 2)) fight = 3;
	if (fightModes & (0x01 << 3)) follow = 0;
	else if (fightModes & (0x01 << 4)) follow = 1;
	if (fightModes & (0x01 << 5)) attack = 1;
	else attack = 0;

	protocol->SendFightModes(fight, follow, attack);
}

void Game::PlayerAttack(unsigned int creatureID) {
	LOCKCLASS lockClass(lockGame);

	Player* player = GetPlayer();
	if (player) {
		if (Player::GetAttackID() == creatureID)
			creatureID = 0x00;

		Player::SetAttackID(creatureID);
	}


	protocol->SendAttack(creatureID);
}

void Game::PlayerFollow(unsigned int creatureID) {
	LOCKCLASS lockClass(lockGame);

	Player* player = GetPlayer();
	if (player) {
		if (Player::GetFollowID() == creatureID)
			creatureID = 0x00;

		Player::SetFollowID(creatureID);
	}

	protocol->SendFollow(creatureID);
}

void Game::PlayerRequestChannels() {
	LOCKCLASS lockClass(lockGame);

	protocol->SendRequestChannels();
}

void Game::PlayerOpenChannel(unsigned short channelID, std::string channelName) {
	LOCKCLASS lockClass(lockGame);

	if (channelID != CHANNEL_PRIVATE)
		protocol->SendOpenChannel(channelID);
	else
		protocol->SendOpenPrivateChannel(channelName);
}

void Game::PlayerCloseChannel(unsigned short channelID) {
	LOCKCLASS lockClass(lockGame);

	protocol->SendCloseChannel(channelID);
}

void Game::PlayerProcessRuleViolation(std::string reporter) {
	LOCKCLASS lockClass(lockGame);

	protocol->SendProcessRuleViolation(reporter);
}

void Game::PlayerCloseRuleViolation(std::string reporter) {
	LOCKCLASS lockClass(lockGame);

	protocol->SendCloseRuleViolation(reporter);
}

void Game::PlayerCancelRuleViolation() {
	LOCKCLASS lockClass(lockGame);

	protocol->SendCancelRuleViolation();
}

void Game::PlayerAddVIP(std::string creatureName) {
	LOCKCLASS lockClass(lockGame);

	protocol->SendAddVIP(creatureName);
}

void Game::PlayerRemoveVIP(unsigned int creatureID) {
	LOCKCLASS lockClass(lockGame);

	protocol->SendRemoveVIP(creatureID);
	if (viplist) {
		viplist->RemoveCreature(creatureID);
		viplist->UpdateContainer();
	}
}

void Game::PlayerTextWindow(unsigned int textID, std::string text) {
	LOCKCLASS lockClass(lockGame);

	protocol->SendTextWindow(textID, text);
}

void Game::PlayerHouseWindow(unsigned char listID, unsigned int textID, std::string text) {
	LOCKCLASS lockClass(lockGame);

	protocol->SendHouseWindow(listID, textID, text);
}

void Game::PlayerRequestOutfit() {
	LOCKCLASS lockClass(lockGame);

	protocol->SendRequestOutfit();
}

void Game::PlayerSetOutfit(Outfit outfit) {
	LOCKCLASS lockClass(lockGame);

	protocol->SendSetOutfit(outfit);
}

void Game::PlayerPartyInvite(unsigned int creatureID) {
	LOCKCLASS lockClass(lockGame);

	protocol->SendPartyInvite(creatureID);
}

void Game::PlayerPartyJoin(unsigned int creatureID) {
	LOCKCLASS lockClass(lockGame);

	protocol->SendPartyJoin(creatureID);
}

void Game::PlayerPartyRevoke(unsigned int creatureID) {
	LOCKCLASS lockClass(lockGame);

	protocol->SendPartyRevoke(creatureID);
}

void Game::PlayerPartyPassLeadership(unsigned int creatureID) {
	LOCKCLASS lockClass(lockGame);

	protocol->SendPartyPassLeadership(creatureID);
}

void Game::PlayerPartyLeave() {
	LOCKCLASS lockClass(lockGame);

	protocol->SendPartyLeave();
}

void Game::PlayerPartyEnableShared(unsigned char active) {
	LOCKCLASS lockClass(lockGame);

	protocol->SendPartyEnableShared(active);
}

void Game::PlayerRequestQuestLog() {
	LOCKCLASS lockClass(lockGame);

	protocol->SendRequestQuestLog();
}

void Game::PlayerRequestQuestLine(unsigned short questID) {
	LOCKCLASS lockClass(lockGame);

	protocol->SendRequestQuestLine(questID);
}

void Game::PlayerCloseNPC() {
	LOCKCLASS lockClass(lockGame);

	protocol->SendCloseNPC();
}

void Game::PlayerRequestSpells() {
	LOCKCLASS lockClass(lockGame);

	protocol->SendRequestSpells();
}


void Game::CheckCommand(std::string& text, unsigned char& speakClass) {
	LOCKCLASS lockClass(lockGame);

	size_t pos = text.find(' ');
	std::string command = text.substr(0, pos);
	if (command == "#w") {
		speakClass = SPEAK_WHISPER;
		std::string _text = text.substr(3, std::string::npos);
		text = _text;
	}
	else if (command == "#y") {
		speakClass = SPEAK_YELL;
		std::string _text = text.substr(3, std::string::npos);
		text = _text;
	}
	else if (command == "#b") {
		speakClass = SPEAK_BROADCAST;
		std::string _text = text.substr(3, std::string::npos);
		text = _text;
	}
	else if (command == "#script") {
		std::string execution = text.substr(command.length() + 1);
		if (execution != "")
			LuaScript::RunScript(execution, true);

		text = "";
	}
	else if (command == "#error") {
		Game* ggg = (Game*)NULL;
		ggg->SetPlayer(NULL);
	}
}


void Game::CheckCreatures(float factor) {
	Creature::CheckCreatures(map, factor);
}

void Game::CheckMovingItems(float factor) {
	Item::CheckMovingItems(factor);
}

void Game::CheckPlayer() {
	LOCKCLASS lockClass(lockGame);

	Creature* creature = player->GetCreature();
	if (!creature)
		return;

	Direction nextMove = player->GetNextMove();
	if (creature->GetStep() == 0.0f && creature->newPos == Position(0, 0, 0) && nextMove != NONE)
		PlayerMove(nextMove);

	if (!Player::walking) {
		Position targetPos = player->GetTargetPos();
		if (targetPos != Position(0, 0, 0)) {
			player->ClearMoves();

			std::list<Direction> list;
			map->FindWay(player, targetPos, list);
			PlayerAutoWalk(list);
		}
	}
}



void Game::onMakeAction(Windows* wnds, Window* wnd, unsigned char action, void* data) {
	LOCKCLASS lockClass(lockGame);

	Mouse* mouse = GetMouse();
	Keyboard* keyboard = GetKeyboard();

	if (!wnds) {
		Logger::AddLog("Game::onMakeAction()", "Pointer to windows is NULL!", LOG_ERROR);
		return;
	}

	if (action == ACTION_LOOK) {
		ThingData* moveData = (ThingData*)data;

		if (moveData->fromPos != Position(0, 0, 0))
			PlayerLookAt(moveData->fromPos, moveData->itemId, moveData->fromStackPos);
		else if (moveData->toPos != Position(0, 0, 0)) {
			unsigned char counterOffer = (unsigned char)moveData->toPos.y;
			unsigned char index = moveData->toPos.z;
			PlayerLookInTrade(counterOffer, index);
		}
		else {
			unsigned short itemID = moveData->itemId;
			unsigned char type = moveData->count;
			PlayerLookInShop(itemID, type);
		}
	}
	else if (action == ACTION_USE) {
		ThingData* moveData = (ThingData*)data;

		PlayerUseThing(moveData->fromPos, moveData->itemId, moveData->fromStackPos, 0);
	}
	else if (action == ACTION_USEWITH) {
		ThingData* moveData = (ThingData*)data;
		Position* fromPos = new(M_PLACE) Position(moveData->fromPos.x, moveData->fromPos.y, moveData->fromPos.z);
		Item* useItem = new(M_PLACE) Item;
		useItem->SetID(moveData->itemId);
		useItem->SetCount(moveData->count);

		mouse->SetHolder(moveData->itemId, moveData->fromStackPos,
			0x51, NULL, (void*)fromPos, (void*)dynamic_cast<Thing*>(useItem));
	}
	else if (action == ACTION_OPEN) {
		ThingData* moveData = (ThingData*)data;

		unsigned char index = (unsigned char)(unsigned int)moveData->scroll;
		PlayerUseThing(moveData->fromPos, moveData->itemId, moveData->fromStackPos, index);
	}
	else if (action == ACTION_OPENNEW) {
		ThingData* moveData = (ThingData*)data;

		unsigned char index = Container::GetFreeIndex();
		PlayerUseThing(moveData->fromPos, moveData->itemId, moveData->fromStackPos, index);
	}
	else if (action == ACTION_ROTATE) {
		ThingData* moveData = (ThingData*)data;

		PlayerRotateItem(moveData->fromPos, moveData->itemId, moveData->fromStackPos);
	}
	else if (action == ACTION_TRADEWITH) {
		ThingData* moveData = (ThingData*)data;
		Position* fromPos = new(M_PLACE) Position(moveData->fromPos.x, moveData->fromPos.y, moveData->fromPos.z);
		Item* tradeItem = new(M_PLACE) Item;
		tradeItem->SetID(moveData->itemId);
		tradeItem->SetCount(moveData->count);

		mouse->SetHolder(moveData->itemId, moveData->fromStackPos,
			0x52, NULL, (void*)fromPos, (void*)dynamic_cast<Thing*>(tradeItem));
	}
	else if (action == ACTION_MOVETO) {
		ThingData* moveData = (ThingData*)data;

		if (wnd != NULL) {
			moveData->count = (*moveData->scroll);
			PlayerMoveThing(moveData->fromPos, moveData->itemId, moveData->fromStackPos, moveData->toPos, moveData->count);
			wnd->SetAction(ACT_CLOSE);
		}
		else
			delete_debug(moveData, M_PLACE);
	}
	else if (action == ACTION_ATTACK) {
		Creature* creature = (Creature*)data;

		PlayerAttack(creature->GetID());
	}
	else if (action == ACTION_FOLLOW) {
		Creature* creature = (Creature*)data;

		PlayerFollow(creature->GetID());
	}
	else if (action == ACTION_COPYNAME) {
		std::string creatureName = *((std::string*)data);
		Game::SetClipboard(creatureName);
	}
	else if (action == ACTION_SENDMESSAGE) {
		std::string creatureName = *((std::string*)data);

		PlayerOpenChannel(CHANNEL_PRIVATE, creatureName);
	}
	else if (action == ACTION_IGNORE) {
		std::string* creatureName = (std::string*)data;
		Creature::Ignore(*creatureName);
	}
	else if (action == ACTION_UNIGNORE) {
		std::string* creatureName = (std::string*)data;
		Creature::Unignore(*creatureName);
	}
	else if (action == ACTION_ADDTOVIP) {
		std::string* creatureName = (std::string*)data;
		if (*creatureName == "!@#$%")
			wnds->OpenWindow(WND_ADDVIP, this);
		else
			PlayerAddVIP(*creatureName);
	}
	else if (action == ACTION_REMOVEFROMVIP) {
		unsigned int creatureID = (unsigned int)data;
		PlayerRemoveVIP(creatureID);
	}
	else if (action == ACTION_PARTYINVITE) {
		Creature* creature = (Creature*)data;
		PlayerPartyInvite(creature->GetID());
	}
	else if (action == ACTION_PARTYJOIN) {
		Creature* creature = (Creature*)data;
		PlayerPartyJoin(creature->GetID());
	}
	else if (action == ACTION_PARTYREVOKE) {
		Creature* creature = (Creature*)data;
		PlayerPartyRevoke(creature->GetID());
	}
	else if (action == ACTION_PARTYPASSLEADER) {
		Creature* creature = (Creature*)data;
		PlayerPartyPassLeadership(creature->GetID());
	}
	else if (action == ACTION_SETOUTFIT) {
		PlayerRequestOutfit();
	}
	else if (action == ACTION_MOUNT) {
		PlayerToggleMount(true);
	}
	else if (action == ACTION_DISMOUNT) {
		PlayerToggleMount(false);
	}
	else if (action == ACTION_PARTYENABLESHARED) {
		PlayerPartyEnableShared(0x01);
	}
	else if (action == ACTION_PARTYDISABLESHARED) {
		PlayerPartyEnableShared(0x00);
	}
	else if (action == ACTION_PARTYLEAVE) {
		PlayerPartyLeave();
	}
	else if (action == ACTION_CLOSECHANNEL) {
		WindowElementButton* button = (WindowElementButton*)data;

		if (button && wndConsole && consoleTab) {
			Channel* channel = Channel::GetChannel(NULL, button);

			if (channel && (channel->GetID() == CHANNEL_DEFAULT || channel->GetID() == CHANNEL_SERVER_LOG))
				return;
			else if (channel && channel->GetID() == CHANNEL_NPC) {
				PlayerCloseNPC();
				CloseChannel(channel->GetID());
			}
			else if (channel && channel->GetID() != CHANNEL_PRIVATE) {
				PlayerCloseChannel(channel->GetID());
				CloseChannel(channel->GetID());
            }
			else if (channel)
				CloseChannel(channel->GetName());
		}
		else if (!wndConsole)
			Logger::AddLog("Game::onMakeAction(ACTION_CLOSECHANNEL)", "Console window not found!", LOG_WARNING);
		else if (!consoleTab)
			Logger::AddLog("Game::onMakeAction(ACTION_CLOSECHANNEL)", "Console tab not found!", LOG_WARNING);
	}
	else if (action == ACTION_SAVECHANNEL) {
		WindowElementButton* button = (WindowElementButton*)data;

		if (button && wndConsole && consoleTab) {
			Channel* channel = Channel::GetChannel(NULL, button);

			SaveChannelText(channel);
		}
		else if (!wndConsole)
			Logger::AddLog("Game::onMakeAction(ACTION_SAVECHANNEL)", "Console window not found!", LOG_WARNING);
		else if (!consoleTab)
			Logger::AddLog("Game::onMakeAction(ACTION_SAVECHANNEL)", "Console tab not found!", LOG_WARNING);
	}
	else if (action == ACTION_PASTE) {
		WindowElementTextarea* textarea = (WindowElementTextarea*)data;
		Keyboard keyboard;
		keyboard.keyChar = 22;
		textarea->CheckInput(keyboard);
	}
	else if (action == ACTION_COPY) {
		std::string text = *((std::string*)data);
		Game::SetClipboard(text);
	}
	else if (action == ACTION_CUT) {
		WindowElementTextarea* textarea = (WindowElementTextarea*)data;
		Keyboard keyboard;
		keyboard.keyChar = 24;
		textarea->CheckInput(keyboard);
	}
	else if (action == ACTION_COPYMESSAGE) {
		std::string text = *((std::string*)data);
		Game::SetClipboard(text);
	}
	else if (action == ACTION_CLEARCHANNEL) {
		Channel* ch = (Channel*)data;
		ch->ClearMessages();
		WindowElementTextarea* textarea = (WindowElementTextarea*)ch->GetTextarea();
		if (textarea)
			textarea->SetText("");
	}
	else if (action == ACTION_SETHOTKEY) {
		unsigned char key = (unsigned char)(unsigned int)data;
		wnds->OpenWindow(WND_SETHOTKEY, this, keyboard, key);
	}
	else if (action == ACTION_SETHOTKEYTEXT) {
		unsigned char key = (unsigned char)(unsigned int)data;
		wnds->OpenWindow(WND_SETHOTKEYTEXT, this, key);
	}
	else if (action == ACTION_SETHOTKEYMODE1) {
		unsigned char key = (unsigned char)(unsigned int)data;
		HotKey* hk = GetHotKey(key);
		hk->mode = 1;
	}
	else if (action == ACTION_SETHOTKEYMODE2) {
		unsigned char key = (unsigned char)(unsigned int)data;
		HotKey* hk = GetHotKey(key);
		hk->mode = 2;
	}
	else if (action == ACTION_SETHOTKEYMODE3) {
		unsigned char key = (unsigned char)(unsigned int)data;
		HotKey* hk = GetHotKey(key);
		hk->mode = 3;
	}
	else if (action == ACTION_SETHOTKEYMODE4) {
		unsigned char key = (unsigned char)(unsigned int)data;
		HotKey* hk = GetHotKey(key);
		hk->mode = 1;
	}
	else if (action == ACTION_SETHOTKEYMODE5) {
		unsigned char key = (unsigned char)(unsigned int)data;
		HotKey* hk = GetHotKey(key);
		hk->mode = 2;
	}
	else if (action == ACTION_CLEARHOTKEY) {
		unsigned char key = (unsigned char)(unsigned int)data;
		ClearHotKey(key);
	}
	else if (action == ACTION_CLEARFREE) {
		ThingData* moveData = (ThingData*)data;

		WindowElementItemContainer* itemCont = (WindowElementItemContainer*)moveData->scroll;
		Container* container = itemCont->GetContainer();
		if (container)
			container->TransformItem(itemCont->GetSlot(), NULL);
	}
	else if (action == ACTION_ADDMARKER) {
		Position* pos = (Position*)data;
		MiniMap* mmap = (map ? map->GetMiniMap() : NULL);

		wnds->CloseWindows(WND_EDITMARKER);
		wnds->OpenWindow(WND_EDITMARKER, this, pos, NULL);
	}
	else if (action == ACTION_EDITMARKER) {
		Position* pos = (Position*)data;
		MiniMap* mmap = (map ? map->GetMiniMap() : NULL);

		Marker marker(0, "");
		if (mmap)
			marker = mmap->GetMarker(*pos);

		wnds->CloseWindows(WND_EDITMARKER);
		wnds->OpenWindow(WND_EDITMARKER, this, pos, &marker);
	}
	else if (action == ACTION_REMOVEMARKER) {
		Position* pos = (Position*)data;
		MiniMap* mmap = (map ? map->GetMiniMap() : NULL);

		if (mmap)
			mmap->RemoveMarker(*pos);
	}
	else if (action == ACTION_ADDWAYPOINT) {
		Position* pos = (Position*)data;
		MiniMap* mmap = (map ? map->GetMiniMap() : NULL);

		if (mmap)
			mmap->AddWaypoint(*pos, Waypoint(1, ""));
	}
	else if (action == ACTION_EDITWAYPOINT) {
		Position* pos = (Position*)data;
		MiniMap* mmap = (map ? map->GetMiniMap() : NULL);

		Waypoint waypoint(0, "");
		if (mmap)
			waypoint = mmap->GetWaypoint(*pos);

		wnds->CloseWindows(WND_EDITWAYPOINT);
		wnds->OpenWindow(WND_EDITWAYPOINT, this, pos, &waypoint);
	}
	else if (action == ACTION_REMOVEWAYPOINT) {
		Position* pos = (Position*)data;
		MiniMap* mmap = (map ? map->GetMiniMap() : NULL);

		if (mmap)
			mmap->RemoveWaypoint(*pos);
	}
	else if (action == ACTION_CLEARWAYPOINTS) {
		MiniMap* mmap = (map ? map->GetMiniMap() : NULL);

		if (mmap)
			mmap->ClearWaypoints();
	}
	else if (action == ACTION_SAVEWAYPOINTS) {
		wnds->CloseWindows(WND_SAVEWAYPOINTS);
		wnds->OpenWindow(WND_SAVEWAYPOINTS, this);
	}
	else if (action == ACTION_LOADWAYPOINTS) {
		wnds->CloseWindows(WND_LOADWAYPOINTS);
		wnds->OpenWindow(WND_LOADWAYPOINTS, this);
	}
	else if (action == ACTION_CHANGEFIGHTMODE) {
		WindowElementButton** buttons = (WindowElementButton**)data;

		Player* player = GetPlayer();
		if (wnd != NULL) {
			if (buttons[0]->GetPressed() != player->GetFightMode(0)) {
				buttons[0]->SetPressed(true);
				buttons[1]->SetPressed(false);
				buttons[2]->SetPressed(false);
			}
			else if (buttons[1]->GetPressed() != player->GetFightMode(1)) {
				buttons[0]->SetPressed(false);
				buttons[1]->SetPressed(true);
				buttons[2]->SetPressed(false);
			}
			else if (buttons[2]->GetPressed() != player->GetFightMode(2)) {
				buttons[0]->SetPressed(false);
				buttons[1]->SetPressed(false);
				buttons[2]->SetPressed(true);
			}

			if (buttons[3]->GetPressed() != player->GetFightMode(3)) {
				buttons[3]->SetPressed(true);
				buttons[4]->SetPressed(false);
			}
			else if (buttons[4]->GetPressed() != player->GetFightMode(4)) {
				buttons[3]->SetPressed(false);
				buttons[4]->SetPressed(true);
			}

			player->SetFightMode(buttons[0]->GetPressed(), 0);
			player->SetFightMode(buttons[1]->GetPressed(), 1);
			player->SetFightMode(buttons[2]->GetPressed(), 2);
			player->SetFightMode(buttons[3]->GetPressed(), 3);
			player->SetFightMode(buttons[4]->GetPressed(), 4);
			player->SetFightMode(buttons[5]->GetPressed(), 5);

			options.fightModes = player->GetFightModes();
			PlayerSetFightModes(player->GetFightModes());
		}
		else
			delete_debug_array(buttons, M_PLACE);
	}
	else if (action == ACTION_OPENWINDOW) {
		WINDOW_TYPE wndType = (WINDOW_TYPE)(unsigned int)data;
		Window* wnd = wnds->FindWindow(wndType);
		if (wnd)
			wnds->CloseWindows(wndType);
		else
			wnd = wnds->OpenWindow(wndType, this);
	}
	else if (action == ACTION_CHANNELS) {
		PlayerRequestChannels();
	}
	else if (action == ACTION_QUESTLOG) {
		PlayerRequestQuestLog();
	}
	else if (action == ACTION_SPELLS) {
		PlayerRequestSpells();
	}
	else if (action == ACTION_EXECUTE) {
		Signal* signal = (Signal*)data;
		if (signal && signal->IsExecutable())
			signal->Execute();
	}
	else if (action == ACTION_GAME) {
		wnds->CloseWindows(WND_OPTGAME);
		wnds->OpenWindow(WND_OPTGAME, this);
	}
	else if (action == ACTION_LOGOUT) {
		if (status && !(status->GetIcons() & ICON_PZBLOCK)) {
			if (protocol)
				protocol->SendLogout();
		}
	}
	else if (action == ACTION_QUIT) {
		PostQuitMessage(0);
	}
}

void LoginThread(Game* game, Protocol* protocol, Updater* updater) {
	game->LoadData();
	protocol->LoginServer();
	protocol->StartReceiveLoop();
	if (updater)
		updater->IncCounter(APP_INTERACT_ADDRESS);
}

void Game::onLogin(Windows* wnds, Window* wnd, std::vector<void*> pointers) {
	LOCKCLASS lockClass(lockGame);

	WindowElementTextarea* account = (WindowElementTextarea*)pointers[0];
	WindowElementTextarea* password = (WindowElementTextarea*)pointers[1];

	wnds->CloseWindows(WND_MESSAGE);
	wnds->CloseWindows(WND_CHARACTERSLIST);
	wnds->CloseWindows(WND_QUEUE);

	SetGameState(GAME_IDLE);

	Host h = GetHost();
	h.account = account->GetText();
	h.password = password->GetText();
	SetHost(h);

	SaveOptions();

	password->SetText("");

	Game::options.protocol = atoi(h.protocol.c_str());

	Protocol* protocol = GetProtocol();
	if (protocol)
		THREAD thread(boost::bind(&delete_debug<Protocol>, protocol, M_PLACE));

	if (Game::options.protocol == 822) protocol = new(M_PLACE) Protocol822(this);
	else if (Game::options.protocol == 840) protocol = new(M_PLACE) Protocol840(this);
	else if (Game::options.protocol == 842) protocol = new(M_PLACE) Protocol842(this);
	else if (Game::options.protocol == 850) protocol = new(M_PLACE) Protocol850(this);
	else if (Game::options.protocol == 854) protocol = new(M_PLACE) Protocol854(this);
	else if (Game::options.protocol == 860) protocol = new(M_PLACE) Protocol860(this);
	else if (Game::options.protocol == 870) protocol = new(M_PLACE) Protocol870(this);
	else if (Game::options.protocol == 910) protocol = new(M_PLACE) Protocol910(this);
	else protocol = new(M_PLACE) Protocol910(this);

	SetProtocol(protocol);

	std::string updateURL;
	std::string location;
	if (h.update != "") {
		updateURL = h.update;
		location = "data";
		//Check old folder tree
		std::string dataLocationOld = std::string("./data/") + h.host;
		dataLocation = std::string("./data/") + h.host + "." + h.port;
		DIR *dp = opendir(dataLocation.c_str());
		if (dp) {
			rename(dataLocationOld.c_str(), dataLocation.c_str());
			closedir(dp);
		}
	}
	else {
		updateURL = options.updateURL;
		location = std::string("data/") + value2str(Game::options.protocol);
		dataLocation = std::string("./data/") + value2str(Game::options.protocol);
	}

	std::string filesLocationOld = std::string("./data/") + h.host;
	filesLocation = std::string("./data/") + h.host + "." + h.port;
	DIR *dp = opendir(filesLocationOld.c_str());
	if (dp) {
		rename(filesLocationOld.c_str(), filesLocation.c_str());
		closedir(dp);
	}

	CreateDirectory(filesLocation.c_str(), NULL);
	CreateDirectory(std::string(filesLocation + "/minimap").c_str(), NULL);
	CreateDirectory(std::string(filesLocation + "/hotkeys").c_str(), NULL);

	if (Game::options.autoUpdate && updater) {
		updater->SignalClear();
		updater->SignalAddFunction(boost::bind(&Game::LoadData, this));
		updater->SignalAddFunction(boost::bind(&Protocol::LoginServer, protocol));
		updater->SignalAddFunction(boost::bind(&Protocol::StartReceiveLoop, protocol));
		updater->SignalAddFunction(boost::bind(&Updater::IncCounter, updater, APP_INTERACT_ADDRESS));
		updater->StartUpdating(updateURL, location, dataLocation, false);
	}
	else {
		THREAD thread(boost::bind(&LoginThread, game, protocol, updater));
	}
}

void Game::onEnterGame(Windows* wnds, Window* wnd, unsigned short* number) {
	LOCKCLASS lockClass(lockGame);

	if (number) {
		CharactersList list = GetCharacters();
		CharactersList::iterator it = list.begin();
		unsigned short opt = 0;
		for (it, opt; it != list.end() && opt < *number; it++, opt++);

		if (it != list.end())
			SetCharacter(*it);
		else {
			wnds->OpenWindow(WND_MESSAGE, Text::GetText("ERROR_MESSAGE_0", Game::options.language).c_str());
			return;
		}
	}

	if (GetGameState() == GAME_LOGGEDTOGAME) {
		if (status && (status->GetIcons() & ICON_SWORDS)) {
			PlayerLogout();
			return;
		}

		protocol->CloseConnection();
		Game::AddTask(boost::bind(&Game::onEnterGame, this, wnds, wnd, (unsigned short*)NULL));
		return;
	}
	else
		SetGameState(GAME_LOGGEDTOSERVER);

	wnds->OpenWindow(WND_MESSAGE, Text::GetText("CONNECTINGGAME", Game::options.language).c_str());
	if (protocol->LoginGame())
		protocol->StartReceiveLoop();

	if (player) {
		player->SetFightModes(options.fightModes);
		PlayerSetFightModes(player->GetFightModes());
	}

	lockGame.unlock();
}

void Game::onQueue(Windows* wnds, Window* wnd) {
	LOCKCLASS lockClass(lockGame);

	SetGameState(GAME_LOGGEDTOSERVER);

	wnd->SetAction(ACT_CLOSE);

	wnds->OpenWindow(WND_MESSAGE, Text::GetText("CONNECTINGGAME", Game::options.language).c_str());
	if (protocol->LoginGame())
		protocol->StartReceiveLoop();
}

void Game::onConsoleEnter(Windows* wnds, WindowElementTextarea* from_textarea, TabElement* element) {
	LOCKCLASS lockClass(lockGame);

	std::string text = from_textarea->GetText();
	from_textarea->SetText("");

	WindowElementContainer* container = element->second;
	if (container) {
		WindowElementTextarea* to_textarea = container->Find<WindowElementTextarea>();
		if (to_textarea) {
			Channel* ch = Channel::GetChannel((void*)to_textarea);
			if (ch) {
				unsigned char sclass = SPEAK_SAY;
				CheckCommand(text, sclass);
				if (text == "")
					return;

				if (ch->GetID() == CHANNEL_DEFAULT || ch->GetID() == CHANNEL_SERVER_LOG) {
					PlayerSendSay(sclass, ch, text);
				}
				else if (ch->GetID() == CHANNEL_NPC) {
					std::string creatureName = "";
					Creature* creature = (player ? player->GetCreature() : NULL);
					if (creature)
						creatureName = creature->GetName();

					PlayerSendSay(SPEAK_PRIVATE_PN, ch, text);

					time_t now = RealTime::getTime() / 1000;
					std::string message = time2str(now, false, false, false, true, true, false) + " " + player->GetCreature()->GetName() + " (" + value2str(player->GetStatistics()->level) + "): " + text;
					ch->AddMessage(now, creatureName, TextString(message, 131));
				}
				else if (ch->GetID() != CHANNEL_PRIVATE) {
					PlayerSendSay(SPEAK_CHANNEL_Y, ch, text);
				}
				else {
					std::string creatureName = "";
					Creature* creature = (player ? player->GetCreature() : NULL);
					if (creature)
						creatureName = creature->GetName();

					PlayerSendSay(SPEAK_PRIVATE, ch, text);

					time_t now = RealTime::getTime() / 1000;
					std::string message = time2str(now, false, false, false, true, true, false) + " " + player->GetCreature()->GetName() + " (" + value2str(player->GetStatistics()->level) + "): " + text;
					ch->AddMessage(now, creatureName, TextString(message, 131));
				}
			}
		}
	}
}

void Game::onOpenChannel(Windows* wnds, Window* wnd, unsigned short* number, std::string* name) {
	LOCKCLASS lockClass(lockGame);

	wnd->SetAction(ACT_CLOSE);

	if (*name != "") {
		PlayerOpenChannel(CHANNEL_PRIVATE, *name);
		return;
	}

	ChannelsList list = GetChannels();

	ChannelsList::iterator it = list.begin();
	unsigned short opt = 0;
	for (it, opt; it != list.end() && opt < *number; it++, opt++);

	if (it != list.end()) {
		unsigned short channelID = it->GetID();
		std::string channelName = it->GetName();

		if (channelID < 0xFF00)
			PlayerOpenChannel(channelID, channelName);
		else if (channelID != CHANNEL_PRIVATE) {
			ChatUsers users;
			ChatUsers invited;
			OpenChannel(channelID, channelName, users, invited);
		}
	}
	else
		wnds->OpenWindow(WND_MESSAGE, Text::GetText("ERROR_MESSAGE_1", Game::options.language).c_str());
}

void Game::onAddVIP(Windows* wnds, Window* wnd, std::string* name) {
	LOCKCLASS lockClass(lockGame);

	wnd->SetAction(ACT_CLOSE);

	std::string creatureName = *name;
	PlayerAddVIP(creatureName);
}

void Game::onTextWindow(Windows* wnds, Window* wnd, unsigned int textID, std::string* text) {
	LOCKCLASS lockClass(lockGame);

	wnd->SetAction(ACT_CLOSE);

	PlayerTextWindow(textID, *text);
}

void Game::onHouseWindow(Windows* wnds, Window* wnd, unsigned char listID, unsigned int textID, std::string* text) {
	LOCKCLASS lockClass(lockGame);

	wnd->SetAction(ACT_CLOSE);

	PlayerHouseWindow(listID, textID, *text);
}

void Game::onShop(Windows* wnds, Window* wnd, std::vector<void*> pointers, int sw, bool scroll, bool accept, bool checkboxes) {
	LOCKCLASS lockClass(lockGame);

	WindowElementMemo* memo = (WindowElementMemo*)pointers[0];
	WindowElementScrollBar* scrollBar = (WindowElementScrollBar*)pointers[1];
	WindowElementItemContainer* itemCont = (WindowElementItemContainer*)pointers[2];
	WindowElementButton* btBuy = (WindowElementButton*)pointers[3];
	WindowElementButton* btSell = (WindowElementButton*)pointers[4];
	WindowElementCheckBox* cbIgnoreCap = (WindowElementCheckBox*)pointers[5];
	WindowElementCheckBox* cbInBackpack = (WindowElementCheckBox*)pointers[6];
	unsigned char* count = (unsigned char*)pointers[7];
	unsigned int* price = (unsigned int*)pointers[8];
	unsigned int* money = (unsigned int*)pointers[9];

	if (wnd) {
		if (count)
			delete_debug(count, M_PLACE);
		if (price)
			delete_debug(price, M_PLACE);

		Shop* shop = GetShop();
		if (shop) {
			shop->SetName("");
			shop->SetMemo(NULL);
		}

		PlayerCloseShop();

		return;
	}

	if (sw == 1) {
		if (btSell->GetPressed()) {
			itemCont->SetItem(NULL);
			*count = 0;
			scrollBar->SetMaxValue(0);
			memo->Clear();

			btBuy->SetPressed(true);
			btSell->SetPressed(false);

			Shop* shop = GetShop();
			if (shop) {
				ShopItemsList purchaseItems = shop->GetPurchaseItems();
				ShopItemsList::iterator it = purchaseItems.begin();
				for (it; it != purchaseItems.end(); it++) {
					ShopItem purchaseItem = *it;

					ShopDetailItem item = shop->GetShopDetailItemByID(purchaseItem.itemID, purchaseItem.count);
					std::string text = item.name + " / " + value2str(item.buyPrice) + " gp / " + float2str((float)item.weight / 100) + "oz.";

					memo->AddElement(text);
				}

				shop->SetPurchase(true);
			}
			memo->SetOption(0);
		}

		btBuy->SetPressed(true);
	}
	else if (sw == 2) {
		if (btBuy->GetPressed()) {
			itemCont->SetItem(NULL);
			*count = 0;
			scrollBar->SetMaxValue(0);
			memo->Clear();

			btBuy->SetPressed(false);
			btSell->SetPressed(true);

			Shop* shop = GetShop();
			if (shop) {
				ShopItemsList saleItems = shop->GetSaleItems();
				ShopItemsList::iterator it = saleItems.begin();
				for (it; it != saleItems.end(); it++) {
					ShopItem saleItem = *it;

					ShopDetailItem item = shop->GetShopDetailItemByID(saleItem.itemID, saleItem.count);
					std::string text = item.name + " / " + value2str(item.sellPrice) + " gp / " + float2str((float)item.weight / 100) + "oz.";

					memo->AddElement(text, saleItem.amount > 0);
				}

				shop->SetPurchase(false);
			}
			memo->SetOption(0);
		}

		btSell->SetPressed(true);
	}
	else if (!sw && !scroll && !accept && !checkboxes && memo->GetOptionsSize() > 0) {
		int option = memo->GetOption();

		int _price = 0;
		int _max = 100;

		ShopDetailItem sitem;
		if (btBuy->GetPressed()) {
			ShopItem purchase = shop->GetPurchaseItem(option);
			sitem = shop->GetShopDetailItemByID(purchase.itemID, purchase.count);
			if (sitem.buyPrice > 0)
				_max = std::min(100, (int)(*money / sitem.buyPrice));
			_price = sitem.buyPrice;
		}
		else if (btSell->GetPressed()) {
			ShopItem sale = shop->GetSaleItem(option);
			sitem = shop->GetShopDetailItemByID(sale.itemID, sale.count);
			_max = std::min(100, (int)sale.amount);
			_price = sitem.sellPrice;
		}

		if (_max == 0) {
			*count = 0;
			*price = 0;
			scrollBar->SetMaxValue(0);
		}
		else {
			scrollBar->SetMaxValue(_max);
			scrollBar->SetValue(*count);
			*price = *count * _price;
		}

		Item* item = new(M_PLACE) Item;
		item->SetID(sitem.itemID);
		if ((*item)() && (*item)()->stackable)
			item->SetCount(sitem.count);
		else if ((*item)() && ((*item)()->fluid || (*item)()->multiType))
			item->SetCount(sitem.count);

		itemCont->SetItem(item);
	}
	else if (!sw && !scroll && !accept && !checkboxes && memo->GetOptionsSize() == 0) {
		*count = 0;
		itemCont->SetItem(NULL);
		scrollBar->SetMaxValue(0);

		*price = 0;
	}
	else if (!sw && !scroll && !accept && checkboxes) {
		bool ignoreCap = cbIgnoreCap->GetState();
		bool inBackpack = cbInBackpack->GetState();

		Shop* shop = GetShop();
		if (shop) {
			shop->SetIgnoreCap(ignoreCap);
			shop->SetInBackpack(inBackpack);
		}
	}
	else if (scroll && !accept && !checkboxes && memo->GetOptionsSize() > 0) {
		int option = memo->GetOption();

		ShopDetailItem sitem;
		if (btBuy->GetPressed()) {
			ShopItem purchase = shop->GetPurchaseItem(option);
			sitem = shop->GetShopDetailItemByID(purchase.itemID, purchase.count);
			*price = *count * sitem.buyPrice;
		}
		else if (btSell->GetPressed()) {
			ShopItem sale = shop->GetSaleItem(option);
			sitem = shop->GetShopDetailItemByID(sale.itemID, sale.count);
			*price = *count * sitem.sellPrice;
		}
	}
	else if (accept && memo->GetOptionsSize() > 0) {
		int option = memo->GetOption();

		bool ignoreCap = false;
		bool inBackpack = false;

		Shop* shop = GetShop();
		if (shop) {
			ignoreCap = shop->GetIgnoreCap();
			inBackpack = shop->GetInBackpack();
		}

		ShopDetailItem sitem;
		if (btBuy->GetPressed()){
			ShopItem purchase = shop->GetPurchaseItem(option);
			sitem = shop->GetShopDetailItemByID(purchase.itemID, purchase.count);

			if (sitem.itemID != 0 && *count > 0)
				PlayerPurchaseShop(sitem.itemID, sitem.count, *count, ignoreCap, inBackpack);
		}
		else if (btSell->GetPressed()) {
			ShopItem sale = shop->GetSaleItem(option);
			sitem = shop->GetShopDetailItemByID(sale.itemID, sale.count);

			if (sitem.itemID != 0 && *count > 0)
				PlayerSaleShop(sitem.itemID, sitem.count, *count);
		}
	}
}

void Game::onTrade(Windows* wnds, Window* wnd, WindowElementButton* accept, bool reject) {
	LOCKCLASS lockClass(lockGame);

	if (wnd)
		wnd->SetAction(ACT_CLOSE);

	if (trade) {
		if (reject) {
			trade->SetContainer(NULL);
			if (trade->GetContainerP1() || trade->GetContainerP2())
				PlayerCancelTrade();
		}
		else if (accept) {
			if (accept->GetPressed())
				PlayerAcceptTrade();
			else
				accept->SetPressed(true);
		}
	}
}

void Game::onChangeOutfit(Windows* wnds, Window* wnd, unsigned char action, std::vector<void*> pointers) {
	LOCKCLASS lockClass(lockGame);

	if (wnd)
		wnd->SetAction(ACT_CLOSE);

	if (action == 0xFF) {
		WindowElementScrollBar* outfitScroll = (WindowElementScrollBar*)pointers[0];
		WindowElementScrollBar* mountScroll = (WindowElementScrollBar*)pointers[1];
		std::vector<OutfitType>* outfits = (std::vector<OutfitType>*)pointers[2];
		std::vector<OutfitType>* mounts = (std::vector<OutfitType>*)pointers[3];

		unsigned char* outfitIt = (unsigned char*)outfitScroll->GetValuePtr().ptr;
		unsigned char* mountIt = (unsigned char*)mountScroll->GetValuePtr().ptr;

		if (action == 0xFF) {
			if (outfitIt)
				delete_debug(outfitIt, M_PLACE);
			if (mountIt)
				delete_debug(mountIt, M_PLACE);
			if (outfits)
				delete_debug(outfits, M_PLACE);
			if (mounts)
				delete_debug(mounts, M_PLACE);
		}
	}
	else if (action == 0xFE) {
		Creature* outfitCreature = (Creature*)pointers[0];
		Creature* mountCreature = (Creature*)pointers[1];

		Outfit outfit = outfitCreature->GetOutfit();
		Outfit mount = mountCreature->GetOutfit();
		outfit.lookMount = mount.lookType;

		PlayerSetOutfit(outfit);
	}
	else if (action == 0x00) {
		WindowElementScrollBar* outfitScroll = (WindowElementScrollBar*)pointers[0];
		WindowElementText* outfitText = (WindowElementText*)pointers[1];
		Creature* outfitCreature = (Creature*)pointers[2];
		std::vector<OutfitType>* outfits = (std::vector<OutfitType>*)pointers[3];
		WindowElementCheckBox* addon1 = (WindowElementCheckBox*)pointers[4];
		WindowElementCheckBox* addon2 = (WindowElementCheckBox*)pointers[5];
		WindowElementCheckBox* addon3 = (WindowElementCheckBox*)pointers[6];

		unsigned char* outfitIt = (unsigned char*)outfitScroll->GetValuePtr().ptr;

		if (outfitCreature && outfitIt) {
			Outfit outfit = outfitCreature->GetOutfit();
			OutfitType type = (*outfits)[*outfitIt];
			outfit.lookType = type.lookType;
			outfitText->SetText(type.name);

			outfit.lookAddons = 0x00;
			addon1->SetEnabled((type.lookAddons & 0x01) == 0x01);
			addon2->SetEnabled((type.lookAddons & 0x02) == 0x02);
			addon3->SetEnabled((type.lookAddons & 0x04) == 0x04);
			if (addon1->GetState() && (type.lookAddons & 0x01)) outfit.lookAddons |= 0x01;
			if (addon2->GetState() && (type.lookAddons & 0x02)) outfit.lookAddons |= 0x02;
			if (addon3->GetState() && (type.lookAddons & 0x04)) outfit.lookAddons |= 0x04;

			outfitCreature->SetOutfit(outfit);
		}
	}
	else if (action == 0x01) {
		WindowElementScrollBar* mountScroll = (WindowElementScrollBar*)pointers[0];
		WindowElementText* mountText = (WindowElementText*)pointers[1];
		Creature* mountCreature = (Creature*)pointers[2];
		std::vector<OutfitType>* mounts = (std::vector<OutfitType>*)pointers[3];

		unsigned char* mountIt = (unsigned char*)mountScroll->GetValuePtr().ptr;

		if (mountCreature && mountIt) {
			Outfit outfit = mountCreature->GetOutfit();
			OutfitType type = (*mounts)[*mountIt];
			outfit.lookType = type.lookType;
			outfit.lookAddons = type.lookAddons;
			mountText->SetText(type.name);
			mountCreature->SetOutfit(outfit);
		}
	}
	else if (action == 0x02) {
		WindowElementCheckBox* addon1 = (WindowElementCheckBox*)pointers[0];
		WindowElementCheckBox* addon2 = (WindowElementCheckBox*)pointers[1];
		WindowElementCheckBox* addon3 = (WindowElementCheckBox*)pointers[2];
		Creature* outfitCreature = (Creature*)pointers[3];

		Outfit outfit = outfitCreature->GetOutfit();
		outfit.lookAddons = 0x00;
		if (addon1->GetState()) outfit.lookAddons |= 0x01;
		if (addon2->GetState()) outfit.lookAddons |= 0x02;
		if (addon3->GetState()) outfit.lookAddons |= 0x04;
		outfitCreature->SetOutfit(outfit);
	}
	else if (action == 0x10 || action == 0x11 || action == 0x12 || action == 0x13 || action == 0x14) {
		WindowElementColorMap* colormap = (WindowElementColorMap*)pointers[0];
		WindowElementButton* head = (WindowElementButton*)pointers[1];
		WindowElementButton* body = (WindowElementButton*)pointers[2];
		WindowElementButton* legs = (WindowElementButton*)pointers[3];
		WindowElementButton* feet = (WindowElementButton*)pointers[4];
		Creature* outfitCreature = (Creature*)pointers[5];

		Outfit outfit = outfitCreature->GetOutfit();
		if (action == 0x10) {
			if (head->GetPressed()) outfit.lookHead = colormap->GetIndex();
			else if (body->GetPressed()) outfit.lookBody = colormap->GetIndex();
			else if (legs->GetPressed()) outfit.lookLegs = colormap->GetIndex();
			else if (feet->GetPressed()) outfit.lookFeet = colormap->GetIndex();
			outfitCreature->SetOutfit(outfit);
		}
		else if (action == 0x11) {
			head->SetPressed(true);
			body->SetPressed(false);
			legs->SetPressed(false);
			feet->SetPressed(false);
			colormap->SetIndex(outfit.lookHead);
		}
		else if (action == 0x12) {
			head->SetPressed(false);
			body->SetPressed(true);
			legs->SetPressed(false);
			feet->SetPressed(false);
			colormap->SetIndex(outfit.lookBody);
		}
		else if (action == 0x13) {
			head->SetPressed(false);
			body->SetPressed(false);
			legs->SetPressed(true);
			feet->SetPressed(false);
			colormap->SetIndex(outfit.lookLegs);
		}
		else if (action == 0x14) {
			head->SetPressed(false);
			body->SetPressed(false);
			legs->SetPressed(false);
			feet->SetPressed(true);
			colormap->SetIndex(outfit.lookFeet);
		}
	}
}

void Game::onQuestLog(Windows* wnds, Window* wnd, WindowElementMemo* memo1, WindowElementMemo* memo2, WindowElementTextarea* textarea) {
	LOCKCLASS lockClass(lockGame);

	if (wnd) {
		wnd->SetAction(ACT_CLOSE);
		return;
	}

	if (memo1 && memo1->GetOptionsSize() > 0 && questlog) {
		int option = memo1->GetOption();
		QuestPair questPair = questlog->GetQuestByNumber(option);

		PlayerRequestQuestLine(questPair.first);
	}

	if (textarea && memo2 && memo2->GetOptionsSize() > 0 && questlog) {
		int option = memo2->GetOption();
		Mission mission = questlog->GetMissionByNumber(option);

		textarea->SetText(mission.description);
	}
}

void Game::onSetHotKey(Windows* wnds, Window* wnd, Keyboard* keyboard, unsigned char key, std::string* text) {
	LOCKCLASS lockClass(lockGame);

	if (wnd)
		wnd->SetAction(ACT_CLOSE);

	HotKey* hk = GetHotKey(key);
	if (keyboard) {
		unsigned short m = 0x00;
		if (!keyboard->capsLock) {
			if (keyboard->key[VK_SHIFT]) m |= 0x8000;
			if (keyboard->key[VK_CONTROL]) m |= 0x4000;
		}
		hk->keyChar = m + keyboard->keyChar;
	}
	if (text) {
		hk->text = *text;
		if (!hk->itemID && hk->mode == 0)
			hk->mode = 2;
	}
}

void Game::onOptions(Windows* wnds, Window* wnd, unsigned char option) {
	LOCKCLASS lockClass(lockGame);

	Mouse* mouse = GetMouse();
	Keyboard* keyboard = GetKeyboard();

	Window* woption = NULL;
	if (option == 0) {
		std::list<MenuData> actionList;
		actionList.push_back(MenuData(ACTION_GAME, Text::GetText("ACTION_GAME", Game::options.language), NULL));
		actionList.push_back(MenuData(0x00, "-", NULL));
		actionList.push_back(MenuData(ACTION_LOGOUT, Text::GetText("ACTION_LOGOUT", Game::options.language), NULL));
		actionList.push_back(MenuData(ACTION_QUIT, Text::GetText("ACTION_QUIT", Game::options.language), NULL));
		woption = wnds->OpenWindow(WND_MENU, this, mouse, keyboard, (void*)&actionList);
	}
	else if (option == 1) {
		Windows* wnds = GetWindows();
		if (wnds) {
			wnds->CloseWindows(WND_OPTGRAPHIC);
			wnds->OpenWindow(WND_OPTGRAPHIC, this);
		}
	}
	else if (option == 2) {
		Windows* wnds = GetWindows();
		if (wnds) {
			wnds->CloseWindows(WND_OPTSOUND);
			wnds->OpenWindow(WND_OPTSOUND, this);
		}
	}
	else if (option == 3) {
		Windows* wnds = GetWindows();
		if (GetGameState() == GAME_LOGGEDTOGAME) {
			Window* minimapWnd = GetWindowMiniMap();
			Window* inventoryWnd = GetWindowInventory();
			Window* statisticsWnd = GetWindowStatistics();
			Window* battleWnd = GetWindowBattle();
			Window* viplistWnd = GetWindowVIPList();
			Window* botWnd = (wnds ? wnds->FindWindow(WND_BOT) : NULL);
			std::list<MenuData> actionList;
			actionList.push_back(MenuData(ACTION_OPENWINDOW, Text::GetText((!minimapWnd ? "ACTION_OPENWINDOW_MINIMAP" : "ACTION_CLOSEWINDOW_MINIMAP"), Game::options.language), (void*)WND_MINIMAP));
			//actionList.push_back(MenuData(ACTION_OPENWINDOW, Text::GetText((!inventoryWnd ? "ACTION_OPENWINDOW_INVENTORY" : "ACTION_CLOSEWINDOW_INVENTORY"), Game::options.language), (void*)WND_INVENTORY));
			//actionList.push_back(MenuData(ACTION_OPENWINDOW, Text::GetText((!statisticsWnd ? "ACTION_OPENWINDOW_STATISTICS" : "ACTION_CLOSEWINDOW_STATISTICS"), Game::options.language), (void*)WND_STATISTICS));
			//actionList.push_back(MenuData(ACTION_OPENWINDOW, Text::GetText((!battleWnd ? "ACTION_OPENWINDOW_BATTLE" : "ACTION_CLOSEWINDOW_BATTLE"), Game::options.language), (void*)WND_BATTLE));
			//actionList.push_back(MenuData(ACTION_OPENWINDOW, Text::GetText((!viplistWnd ? "ACTION_OPENWINDOW_VIPLIST" : "ACTION_CLOSEWINDOW_VIPLIST"), Game::options.language), (void*)WND_VIPLIST));
			//actionList.push_back(MenuData(ACTION_OPENWINDOW, Text::GetText((!botWnd ? "ACTION_OPENWINDOW_BOT" : "ACTION_CLOSEWINDOW_BOT"), Game::options.language), (void*)WND_BOT));
			//actionList.push_back(MenuData(0x00, "-", NULL));
			actionList.push_back(MenuData(ACTION_CHANNELS, Text::GetText("ACTION_CHANNELS", Game::options.language), NULL));
			actionList.push_back(MenuData(ACTION_QUESTLOG, Text::GetText("ACTION_QUESTLOG", Game::options.language), NULL));
			actionList.push_back(MenuData(ACTION_SPELLS, Text::GetText("ACTION_SPELLS", Game::options.language), NULL));
			woption = wnds->OpenWindow(WND_MENU, this, mouse, keyboard, (void*)&actionList);
		}
	}
	else if (option == 4) {
		Windows* wnds = GetWindows();
		if (wnds) {
			wnds->CloseWindows(WND_ABOUT);
			wnds->OpenWindow(WND_ABOUT, this);
		}
	}
	else if (option == 0xFF) {
		Windows* wnds = GetWindows();
		if (wnds) {
			Window* wclose = wnds->FindWindow(WND_CLOSE);
			Window* woptions = wnds->FindWindow(WND_OPTIONS);
			if (!wclose && woptions) {
				if (!woptions->GetMinimizeAbility()) {
					woptions->SetMinimizeAbility(true);
					wnds->MoveOnTop(woptions, true);
				}
				else {
					woptions->SetMinimizeAbility(false);
					wnds->MoveDown(woptions, true);
					wnds->CloseWindows(WND_MENU);
				}
			}
		}
	}

	if (woption) {
		POINT pos = wnd->GetPosition();
		POINT size = wnd->GetSize();
		woption->SetPosition(option * 80, pos.y + size.y);
	}
}

void Game::onGame(Windows* wnds, Window* wnd, WindowElementMemo* memo, WindowElementCheckBox* cb_autoupdate, WindowElementCheckBox* cb_controls) {
	LOCKCLASS lockClass(lockGame);

	if (cb_autoupdate) {
		options.autoUpdate = cb_autoupdate->GetState();
		return;
	}

	if (cb_controls) {
		options.classicControls = cb_controls->GetState();
		return;
	}

	if (wnd) {
		if (wnds)
			wnds->OpenWindow(WND_MESSAGE, Text::GetText("INFO_MESSAGE_0", Game::options.language).c_str());

		if (memo)
			options.language = memo->GetOption();

		wnd->SetAction(ACT_CLOSE);
	}
}

void Game::onTemplate(Windows* wnds, Window* wnd, WindowElementMemo* memo) {
	LOCKCLASS lockClass(lockGame);

	if (wnd) {
		if (wnds)
			wnds->OpenWindow(WND_MESSAGE, Text::GetText("INFO_MESSAGE_0", Game::options.language).c_str());

		if (memo)
			options.templatesGroup = memo->GetElement().text;

		wnd->SetAction(ACT_CLOSE);
	}
}

void Game::onGraphics(Windows* wnds, Window* wnd, std::vector<void*> pointers) {
	LOCKCLASS lockClass(lockGame);

	WindowElementCheckBox* cb_names = (WindowElementCheckBox*)pointers[0];
	WindowElementCheckBox* cb_bars = (WindowElementCheckBox*)pointers[1];
	WindowElementCheckBox* cb_strech = (WindowElementCheckBox*)pointers[2];
	WindowElementCheckBox* cb_particles = (WindowElementCheckBox*)pointers[3];
	WindowElementCheckBox* cb_level = (WindowElementCheckBox*)pointers[4];
	WindowElementCheckBox* cb_render = (WindowElementCheckBox*)pointers[5];
	WindowElementList* ls_api = (WindowElementList*)pointers[6];

	if (cb_names)
		options.showNames = cb_names->GetState();
	if (cb_bars)
		options.showBars = cb_bars->GetState();
	if (cb_strech) {
		options.dontStrech = cb_strech->GetState();
		if (wndGame) {
			WindowElementContainer* container = wndGame->GetWindowContainer();
			POINT wndSize = container->GetIntSize();
			if (options.dontStrech) {
				POINT size = wndGame->GetSize();
				wndGame->SetIntSize(wndSize.x, 352);
				wndGame->AdjustSize();
				size = wndGame->GetSize();
				wndGame->SetMaxSize(0xFFFF, size.y);
				if (wndSize.y <= 352) {
					wndGame->SetIntSize(wndSize.x, wndSize.y);
					wndGame->AdjustSize();
				}
				wndGame->SetIntSize(0, 0);
			}
		}
	}
	if (cb_particles)
		options.printParticles = cb_particles->GetState();
	if (cb_level) {
		options.printOneLevel = cb_level->GetState();

		Player* player = GetPlayer();
		Map* map = GetMap();
		if (player && map)
			map->CalculateZ(player->GetPosition());
	}
	if (cb_render)
		options.renderToTexture = cb_render->GetState();
	if (ls_api) {
		if (wnds && options.graphicsAPI != ls_api->GetOption())
			wnds->OpenWindow(WND_MESSAGE, Text::GetText("INFO_MESSAGE_0", Game::options.language).c_str());

		options.graphicsAPI = ls_api->GetOption();
	}
}


void Game::onText(Windows* wnds, Window* wnd, std::vector<void*> pointers) {
	LOCKCLASS lockClass(lockGame);

	WindowElementCheckBox* cb_textsize = (WindowElementCheckBox*)pointers[0];

	std::map<unsigned short, Channel*> channels = Channel::GetChannels();
	std::map<unsigned short, Channel*>::iterator it = channels.begin();
	for (it; it != channels.end(); it++) {
		Channel* channel = it->second;
		WindowElementTextarea* textarea = (WindowElementTextarea*)channel->GetTextarea();
		if (textarea)
			textarea->SetFontSize(options.textSize);
	}
}

void Game::onWindows(Windows* wnds, Window* wnd, std::vector<void*> pointers) {
	LOCKCLASS lockClass(lockGame);

	WindowElementCheckBox* cb_fixedpos = (WindowElementCheckBox*)pointers[0];
	WindowElementCheckBox* cb_fadeout = (WindowElementCheckBox*)pointers[1];

	if (cb_fixedpos) {
		options.fixedPositions = cb_fixedpos->GetState();
		if (wnds) {
			wnds->FitFixedWindows();
			wnds->FitGameWindow();
		}
	}
	if (cb_fadeout)
		options.fadeOutInactive = cb_fadeout->GetState();
}

void Game::onSounds(Windows* wnds, Window* wnd) {
#ifndef NO_SOUND
	if (sfx)
		sfx->SetGains((float)options.musicGain / 100, (float)options.soundGain / 100);
#endif
}

void Game::onError(Windows* wnds, Window* wnd, Host host, Character character, std::string error_log, std::string* comment) {
	LOCKCLASS lockClass(lockGame);

	if (wnd)
		wnd->SetAction(ACT_CLOSE);

	if (protocol && error_log != "") {
		Host oldHost = GetHost();
		Character oldCharacter = GetCharacter();

		SetHost(host);
		SetCharacter(character);

		protocol->LoginGame();

		std::string sdebug = "Debug Assertion: " + std::string(APP_NAME) + " " + std::string(APP_VERSION);
		std::string sdate = time2str(RealTime::getTime() / 1000, true, true, true, false, false, false);
		std::string scomment = "Comment: " + (comment ? *comment : "");
		protocol->SendError(sdebug, sdate, error_log, scomment);

		protocol->CloseConnection();

		SetHost(oldHost);
		SetCharacter(oldCharacter);
	}
}

void Game::onUpdate(Windows* wnds, Window* wnd) {
	LOCKCLASS lockClass(lockGame);

	Updater* updater = GetUpdater();
	if (updater) {
		updater->StopUpdating();
		updater->SetContainer(NULL);
		updater->SignalExecute();
	}
}

void Game::onServers(std::vector<void*> pointers) {
	LOCKCLASS lockClass(lockGame);

	Windows* wnds = GetWindows();
	wnds->CloseWindows(WND_SERVERS);
	wnds->OpenWindow(WND_SERVERS, this, &pointers);
}

void Game::onAddFavoriteServer(bool openWindow, std::vector<void*> pointers1, std::vector<void*> pointers2) {
	LOCKCLASS lockClass(lockGame);

	Windows* wnds = GetWindows();
	if (openWindow) {
		WindowElementTextarea* ta_account = (WindowElementTextarea*)pointers1[0];
		WindowElementTextarea* ta_password = (WindowElementTextarea*)pointers1[1];
		WindowElementList* ls_servers = (WindowElementList*)pointers1[2];
		WindowElementTextarea* ta_server = (WindowElementTextarea*)pointers1[3];

		Host h = GetHost();
		h.account = ta_account->GetText();
		h.password = ta_password->GetText();
		SetHost(h);

		wnds->CloseWindows(WND_ADDFAVORITE);
		Window* wnd = wnds->OpenWindow(WND_ADDFAVORITE, this, &pointers1);
	}
	else {
		WindowElementTextarea* name = (WindowElementTextarea*)pointers2[0];
		WindowElementTextarea* host = (WindowElementTextarea*)pointers2[1];
		WindowElementTextarea* port = (WindowElementTextarea*)pointers2[2];
		WindowElementList* protocol = (WindowElementList*)pointers2[3];
		WindowElementTextarea* update = (WindowElementTextarea*)pointers2[4];
		WindowElementTextarea* website = (WindowElementTextarea*)pointers2[5];
		WindowElementTextarea* createacc = (WindowElementTextarea*)pointers2[6];

		Server server;
		server.name = name->GetText();
		server.host = host->GetText();
		server.port = port->GetText();
		server.protocol = protocol->GetElement().text;
		server.updateURL = update->GetText();
		server.websiteURL = website->GetText();
		server.createAccountURL = createacc->GetText();
		server.promoted = false;

		Host h = GetHost();

		if (h.name == server.name) {
			std::string filesLocationOld = std::string("./data/") + h.host + "." + h.port;
			std::string filesLocationNew = std::string("./data/") + server.host + "." + server.port;
			if (filesLocationOld != filesLocationNew)
				rename(filesLocationOld.c_str(), filesLocationNew.c_str());
		}

		h.name = server.name;
		SetHost(h);

		servers->AddFavoriteServer(server);
		servers->UpdateContainers(pointers1);

		wnds->CloseWindows(WND_ADDFAVORITE);
	}
}

void Game::onRemoveFavoriteServer(WindowElementTableMemo* memo, std::vector<void*> pointers) {
	LOCKCLASS lockClass(lockGame);

	int option = memo->GetOption();

	servers->RemoveFavoriteServer(option);
	servers->UpdateContainers(pointers);
}

void Game::onGoToURLServer(WindowElement* memo, int number, bool website) {
	LOCKCLASS lockClass(lockGame);

	int option = 0;
	int optionSize = 0;

	WindowElementMemo* memo1 = dynamic_cast<WindowElementMemo*>(memo);
	WindowElementTableMemo* memo2 = dynamic_cast<WindowElementTableMemo*>(memo);
	if (memo1) {
		option = memo1->GetOption();
		optionSize = memo1->GetOptionsSize();
	}
	else if (memo2) {
		option = memo2->GetOption();
		optionSize = memo2->GetOptionsSize();
	}

    if (optionSize == 0)
        return;

	Server server;
	if (number == 0)
		server = servers->GetOnlineServer(option);
	else if (number == 1)
		server = servers->GetFavoriteServer(option);
    else if (number == 2)
        server = servers->GetWebsiteServer(option);

	if (website && server.websiteURL != "")
		PostOpenBrowser(server.websiteURL);
	else if (server.createAccountURL != "")
		PostOpenBrowser(server.createAccountURL);
}

void Game::onMoveFavoriteServer(WindowElementTableMemo* memo, int step, std::vector<void*> pointers) {
	LOCKCLASS lockClass(lockGame);

	int option = memo->GetOption();
	if (option + step >= memo->GetOptionsSize() || option + step < 0)
		return;

	servers->MoveFavoriteServer(memo->GetOption(), step);
	servers->UpdateFavoritesMemo(memo);
	memo->SetOption(option + step);
}

void Game::onSetServer(WindowElementTableMemo* memo, std::vector<void*> pointers) {
	LOCKCLASS lockClass(lockGame);

	WindowElementTextarea* ta_account = (WindowElementTextarea*)pointers[0];
	WindowElementTextarea* ta_password = (WindowElementTextarea*)pointers[1];
	WindowElementList* ls_servers = (WindowElementList*)pointers[2];
	WindowElementTextarea* ta_server = (WindowElementTextarea*)pointers[3];

	Server server;
	if (!memo && ls_servers) {
		server = servers->GetFavoriteServer(ls_servers->GetOption());
		if (server.host == "")
			return;
	}
	else if (memo && ta_server) {
		server = servers->GetFavoriteServer(memo->GetOption());
		if (server.host == "")
			return;
	}

	Host h = GetHost();
	h.account = "";
	h.password = "";
	h.host = server.host;
	h.port = server.port;
	h.update = server.updateURL;

	h.name = server.name;
	h.protocol = server.protocol;
	h.website = server.websiteURL;
	h.createacc = server.createAccountURL;
	SetHost(h);

	if (memo && ta_server) {
		ta_account->SetText("");
		ta_password->SetText("");
		ta_server->SetText(server.name);
	}
}

void Game::onSlideChannelUsers(WindowElementButton* slider, WindowElementTextarea* textarea, WindowElementMemo* memo) {
	LOCKCLASS lockClass(lockGame);

	if (!slider || !textarea || !memo)
		return;

	POINT memo_size = memo->GetSize();
	if (memo_size.x == 0) {
		slider->SetText(">");
		POINT textarea_size = textarea->GetSize();
		textarea->SetSize(textarea_size.x - 105, textarea_size.y);
		POINT memo_pos = memo->GetPosition();
		memo->SetPosition(memo_pos.x - 100, memo_pos.y);
		memo->SetSize(100, memo_size.y);
	}
	else {
		slider->SetText("<");
		POINT textarea_size = textarea->GetSize();
		textarea->SetSize(textarea_size.x + 105, textarea_size.y);
		POINT memo_pos = memo->GetPosition();
		memo->SetPosition(memo_pos.x + 100, memo_pos.y);
		memo->SetSize(0, memo_size.y);
	}
}

void Game::onMarker(Window* wnd, std::list<void*>* pointers, WindowElementTextarea* comment, Position pos, int button, bool release) {
	LOCKCLASS lockClass(lockGame);

	if (wnd)
		wnd->SetAction(ACT_CLOSE);

	if (!release && pointers) {
		std::list<void*>::iterator it;
		if (button == 0) {
			int marker = 1;
			for (it = pointers->begin(); it != pointers->end(); it++, marker++) {
				WindowElementButton* bt = (WindowElementButton*)(*it);
				if (bt && bt->GetPressed())
					break;
			}

			MiniMap* mmap = (map ? map->GetMiniMap() : NULL);
			if (mmap)
				mmap->AddMarker(pos, Marker(marker, comment->GetText()));
		}
		else if (button > 0) {
			int marker = 1;
			for (it = pointers->begin(); it != pointers->end(); it++, marker++) {
				WindowElementButton* bt = (WindowElementButton*)(*it);
				if (bt) {
					if (marker == button)
						bt->SetPressed(true);
					else
						bt->SetPressed(false);
				}
			}
		}
	}

	if (release && pointers)
		delete_debug(pointers, M_PLACE);
}

void Game::onWaypoint(Window* wnd, std::list<void*>* pointers, WindowElementTextarea* comment, Position pos, int button, bool release) {
	LOCKCLASS lockClass(lockGame);

	if (wnd)
		wnd->SetAction(ACT_CLOSE);

	if (!release && pointers) {
		std::list<void*>::iterator it;
		if (button == 0) {
			int waypoint = 1;
			for (it = pointers->begin(); it != pointers->end(); it++, waypoint++) {
				WindowElementButton* bt = (WindowElementButton*)(*it);
				if (bt && bt->GetPressed())
					break;
			}

			MiniMap* mmap = (map ? map->GetMiniMap() : NULL);
			if (mmap) {
				mmap->AddWaypoint(pos, Waypoint(waypoint, comment->GetText()), 0xFFFE);
			}
		}
		else if (button > 0) {
			int waypoint = 1;
			for (it = pointers->begin(); it != pointers->end(); it++, waypoint++) {
				WindowElementButton* bt = (WindowElementButton*)(*it);
				if (bt) {
					if (waypoint == button)
						bt->SetPressed(true);
					else
						bt->SetPressed(false);
				}
			}
		}
	}

	if (release && pointers)
		delete_debug(pointers, M_PLACE);
}

void Game::onSaveWaypoints(Windows* wnds, Window* wnd, std::string* name) {
	LOCKCLASS lockClass(lockGame);

	wnd->SetAction(ACT_CLOSE);

	std::string filename = *name;
	SaveWaypoints(filename);
}

void Game::onLoadWaypoints(Windows* wnds, Window* wnd, WindowElementMemo* memo) {
	LOCKCLASS lockClass(lockGame);

	wnd->SetAction(ACT_CLOSE);

	std::string filename = memo->GetElement().text;
	LoadWaypoints(filename);
}

void Game::onBotApply(std::vector<void*> pointers) {
	LOCKCLASS lockClass(lockGame);

	WindowElementCheckBox* cb_selfHealing = (WindowElementCheckBox*)pointers[0];
	WindowElementCheckBox* cb_friendHealing = (WindowElementCheckBox*)pointers[1];
	WindowElementTextarea* ta_minSelfHealth = (WindowElementTextarea*)pointers[2];
	WindowElementTextarea* ta_minFriendHealth = (WindowElementTextarea*)pointers[3];
	WindowElementItemContainer* ic_healItem = (WindowElementItemContainer*)pointers[4];
	WindowElementTextarea* ta_selfHealWords = (WindowElementTextarea*)pointers[5];
	WindowElementTextarea* ta_friendHealWords = (WindowElementTextarea*)pointers[6];

	WindowElementCheckBox* cb_manaRefill = (WindowElementCheckBox*)pointers[7];
	WindowElementTextarea* ta_manaStart = (WindowElementTextarea*)pointers[8];
	WindowElementTextarea* ta_manaFinish = (WindowElementTextarea*)pointers[9];
	WindowElementItemContainer* ic_manaRefillItem = (WindowElementItemContainer*)pointers[10];

	WindowElementCheckBox* cb_aimBot = (WindowElementCheckBox*)pointers[11];
	WindowElementCheckBox* cb_autoTargeting = (WindowElementCheckBox*)pointers[12];
	WindowElementItemContainer* ic_aimBotItem = (WindowElementItemContainer*)pointers[13];
	WindowElementTextarea* ta_offensiveWords = (WindowElementTextarea*)pointers[14];

	WindowElementCheckBox* cb_caveBot = (WindowElementCheckBox*)pointers[15];
	WindowElementCheckBox* cb_checkBody = (WindowElementCheckBox*)pointers[16];
	WindowElementCheckBox* cb_moveToBody = (WindowElementCheckBox*)pointers[17];
	WindowElementCheckBox* cb_eatFood = (WindowElementCheckBox*)pointers[18];
	WindowElementCheckBox* cb_takeLoot = (WindowElementCheckBox*)pointers[19];
	WindowElementMemo* me_foodList = (WindowElementMemo*)pointers[20];
	WindowElementMemo* me_lootList = (WindowElementMemo*)pointers[21];
	WindowElementTextarea* ta_lootBag = (WindowElementTextarea*)pointers[22];

	WindowElementCheckBox* cb_script[8];
	WindowElementTextarea* ta_script[8];
	cb_script[0] = (WindowElementCheckBox*)pointers[23];
	ta_script[0] = (WindowElementTextarea*)pointers[24];
	cb_script[1] = (WindowElementCheckBox*)pointers[25];
	ta_script[1] = (WindowElementTextarea*)pointers[26];
	cb_script[2] = (WindowElementCheckBox*)pointers[27];
	ta_script[2] = (WindowElementTextarea*)pointers[28];
	cb_script[3] = (WindowElementCheckBox*)pointers[29];
	ta_script[3] = (WindowElementTextarea*)pointers[30];
	cb_script[4] = (WindowElementCheckBox*)pointers[31];
	ta_script[4] = (WindowElementTextarea*)pointers[32];
	cb_script[5] = (WindowElementCheckBox*)pointers[33];
	ta_script[5] = (WindowElementTextarea*)pointers[34];
	cb_script[6] = (WindowElementCheckBox*)pointers[35];
	ta_script[6] = (WindowElementTextarea*)pointers[36];
	cb_script[7] = (WindowElementCheckBox*)pointers[37];
	ta_script[7] = (WindowElementTextarea*)pointers[38];

	if (bot) {
		HealingStruct hs;
		hs.selfHealing = cb_selfHealing->GetState();
		hs.friendHealing = cb_friendHealing->GetState();
		hs.minSelfHealth = atoi(ta_minSelfHealth->GetText().c_str());
		hs.minFriendHealth = atoi(ta_minFriendHealth->GetText().c_str());
		Item* itemH = ic_healItem->GetContainer()->GetItem(ic_healItem->GetSlot());
		hs.itemID = (itemH ? itemH->GetID() : 0);
		hs.itemSubType = (itemH ? itemH->GetCount() : 0);
		hs.selfHealWords = ta_selfHealWords->GetText();
		hs.friendHealWords = ta_friendHealWords->GetText();

		ManaRefillStruct ms;
		ms.manaRefill = cb_manaRefill->GetState();
		ms.manaStart = atoi(ta_manaStart->GetText().c_str());
		ms.manaFinish = atoi(ta_manaFinish->GetText().c_str());
		Item* itemMR = ic_manaRefillItem->GetContainer()->GetItem(ic_manaRefillItem->GetSlot());
		ms.itemID = (itemMR ? itemMR->GetID() : 0);
		ms.itemSubType = (itemMR ? itemMR->GetCount() : 0);

		AimBotStruct as;
		as.aimBot = cb_aimBot->GetState();
		as.autoTargeting = cb_autoTargeting->GetState();
		Item* itemAB = ic_aimBotItem->GetContainer()->GetItem(ic_aimBotItem->GetSlot());
		as.itemID = (itemAB ? itemAB->GetID() : 0);
		as.itemSubType = (itemAB ? itemAB->GetCount() : 0);
		as.offensiveWords = ta_offensiveWords->GetText();

		CaveBotStruct cs;
		cs.caveBot = cb_caveBot->GetState();
		cs.checkBody = cb_checkBody->GetState();
		cs.moveToBody = cb_moveToBody->GetState();
		cs.eatFood = cb_eatFood->GetState();
		cs.takeLoot = cb_takeLoot->GetState();
		cs.foodList = me_foodList->GetElements();
		cs.lootList = me_lootList->GetElements();
		cs.lootBag = atoi(ta_lootBag->GetText().c_str());

		ScriptsBotStruct ss;
		for (int i = 0; i < 8; i++) {
			ss.scriptsState[i] = cb_script[i]->GetState();
			ss.filenames[i] = ta_script[i]->GetText();
		}

		bot->SetHealingData(hs);
		bot->SetManaRefillData(ms);
		bot->SetAimBotData(as);
		bot->SetCaveBotData(cs);
		bot->SetScriptsBotData(ss);
		THREAD thread(&Bot::Restart);
	}
}

void Game::onBotLists(WindowElementMemo* me_friends, WindowElementMemo* me_enemies, WindowElementMemo* me_foodlist, WindowElementMemo* me_lootlist) {
	LOCKCLASS lockClass(lockGame);

	if (bot) {
		if (me_friends) {
			bot->ClearFriends();
			if (me_friends && me_friends->GetOptionsSize()) {
				unsigned short oldOption = me_friends->GetOption();
				for (int i = 0; i < me_friends->GetOptionsSize(); i++) {
					me_friends->SetOption(i, false);
					std::string creatureName = me_friends->GetElement().text;
					bot->AddFriend(creatureName);
				}
				me_friends->SetOption(oldOption, false);
			}
		}

		if (me_enemies) {
			bot->ClearEnemies();
			if (me_enemies && me_enemies->GetOptionsSize()) {
				unsigned short oldOption = me_enemies->GetOption();
				for (int i = 0; i < me_enemies->GetOptionsSize(); i++) {
					me_enemies->SetOption(i, false);
					std::string creatureName = me_enemies->GetElement().text;
					bot->AddEnemie(creatureName);
				}
				me_enemies->SetOption(oldOption, false);
			}
		}

		if (me_foodlist) {
			CaveBotStruct caveBotData = bot->GetCaveBotData();
			caveBotData.foodList = me_foodlist->GetElements();
			bot->SetCaveBotData(caveBotData);
		}

		if (me_lootlist) {
			CaveBotStruct caveBotData = bot->GetCaveBotData();
			caveBotData.lootList = me_lootlist->GetElements();
			bot->SetCaveBotData(caveBotData);
		}
	}
}

void Game::onOldHotkey(Windows* wnds, Window* wnd, std::vector<void*> pointers, int action) {
	LOCKCLASS lockClass(lockGame);

	HotKey* hotkeys = (HotKey*)pointers[0];
	WindowElementMemo* me_keylist = (WindowElementMemo*)pointers[1];
	WindowElementTextarea* ta_keytext = (WindowElementTextarea*)pointers[2];
	WindowElementCheckBox* cb_immediately = (WindowElementCheckBox*)pointers[3];
	WindowElementItemContainer* ic_keyitem = (WindowElementItemContainer*)pointers[4];
	WindowElementCheckBox* cb_use0 = (WindowElementCheckBox*)pointers[5];
	WindowElementCheckBox* cb_use1 = (WindowElementCheckBox*)pointers[6];
	WindowElementCheckBox* cb_use2 = (WindowElementCheckBox*)pointers[7];

	if (action == 0xFF) {
		if (hotkeys)
			delete_debug_array(hotkeys, M_PLACE);
		return;
	}
}

void Game::onHomePage() {
	Host h = GetHost();
	PostOpenBrowser(h.website);
}


void Game::SetClipboard(std::string text) {
	OpenClipboard(NULL);
	EmptyClipboard();

	HGLOBAL hGlobal;
	hGlobal = GlobalAlloc(GMEM_MOVEABLE, text.length() + 1);
	char *txt = (char*)GlobalLock(hGlobal);
	memcpy(txt, text.c_str(), text.length() + 1);
	GlobalUnlock(hGlobal);
	SetClipboardData(CF_TEXT, hGlobal);

	CloseClipboard();
}

std::string Game::GetClipboard() {
	std::string text;
	if (IsClipboardFormatAvailable(CF_TEXT))
	{
		OpenClipboard(NULL);

		HGLOBAL hGlobal=GetClipboardData(CF_TEXT);
		char *txt=(char*)GlobalLock(hGlobal);
		text = txt;
		GlobalUnlock(hGlobal);

		CloseClipboard();
	}

	return text;
}


void Game::AddTask(boost::function<void()> func) {
	taskManager.PushFunction(func);
}

void Game::ExecuteTasks() {
	LOCKCLASS lockClass(lockGame);
	taskManager.lockSignal.lock();
	if (taskManager.IsExecutable()) {
		taskManager.Execute();
		taskManager.Clear();
	}
	taskManager.lockSignal.unlock();
}


void Game::PostExecuteFile(std::string file, std::string directory) {
	ShellExecute(0, "open", file.c_str(), 0, directory.c_str(), 1);
}

void Game::PostOpenBrowser(std::string link) {
	ShellExecute(0, "open", link.c_str(), 0, 0, 1);
}

void Game::PostQuit() {
	PostQuitMessage(0);
}


//Lua functions

int Game::LuaMessageBox(lua_State* L) {
	std::string text = LuaScript::PopString(L);
	MessageBox(NULL, text.c_str(), "LuaMessageBox", MB_OK);

	return 1;
}

int Game::LuaGetLanguage(lua_State* L) {
	LuaScript::PushNumber(L, (unsigned char)game->GetOptions()->language);

	return 1;
}

int Game::LuaGetMouse(lua_State* L) {
	Game* game = Game::game;

	if (game)
		LuaScript::PushNumber(L, (unsigned long)game->GetMouse());
	else
		lua_pushnil(L);

	return 1;
}

int Game::LuaGetKeyboard(lua_State* L) {
	Game* game = Game::game;

	if (game)
		LuaScript::PushNumber(L, (unsigned long)game->GetKeyboard());
	else
		lua_pushnil(L);

	return 1;
}

int Game::LuaGetResolution(lua_State* L) {
	POINT wndSize = {0, 0};
	Windows* wnds = Windows::wnds;
	if (wnds)
		wndSize = wnds->GetWindowSize();

	lua_newtable(L);
	lua_pushstring(L, "x");
	lua_pushnumber(L, wndSize.x);
	lua_settable(L, -3);
	lua_pushstring(L, "y");
	lua_pushnumber(L, wndSize.y);
	lua_settable(L, -3);
	return 1;
}

int Game::LuaTakeScreenshot(lua_State* L) {
	Game::screenshot = true;
	return 1;
}


int Game::LuaSetPointer(lua_State* L) {
	void* ptr = (void*)((unsigned long)LuaScript::PopNumber(L));
	std::string type = LuaScript::PopString(L);

	Game* game = Game::game;
	if (!game)
		return 1;

	if (type == "window_game")
		game->SetWindowGame((Window*)ptr);
	else if (type == "window_console")
		game->SetWindowConsole((Window*)ptr);
	else if (type == "element_consoletab")
		game->SetConsoleTab((WindowElementTab*)ptr);
	else if (type == "element_cmdline") {
		WindowElementTextarea* cmdLine = (WindowElementTextarea*)ptr;
		game->SetCmdLine(cmdLine);

		WindowElementTab* consoleTab = game->GetConsoleTab();
		if (cmdLine && consoleTab)
			cmdLine->SetAction(boost::bind(&Game::onConsoleEnter, game, game->GetWindows(), cmdLine, consoleTab->GetActiveTabPtr()));
	}

	return 1;
}

int Game::LuaGetPointer(lua_State* L) {
	std::string type = LuaScript::PopString(L);

	Game* game = Game::game;
	if (!game) {
		lua_pushnil(L);
		return 1;
	}

	void* ptr = NULL;
	if (type == "window_game")
		ptr = (void*)game->GetWindowGame();
	else if (type == "window_console")
		ptr = (void*)game->GetWindowConsole();
	else if (type == "element_consoletab")
		ptr = (void*)game->GetConsoleTab();
	else if (type == "element_cmdline")
		ptr = (void*)game->GetCmdLine();

	LuaScript::PushNumber(L, (unsigned long)ptr);
	return 1;
}


int Game::LuaGetHotkeyPtr(lua_State* L) {
	int id = LuaScript::PopNumber(L);

	Game* game = Game::game;
	HotKey* hk = (game ? game->GetHotKey(id) : NULL);

	LuaScript::PushNumber(L, (unsigned long)hk);
	return 1;
}

int Game::LuaSetHotkey(lua_State* L) {
	int key = LuaScript::PopNumber(L);
	int id = LuaScript::PopNumber(L);

	Game* game = Game::game;
	HotKey* hk = (game ? game->GetHotKey(id) : NULL);
	if (hk)
		hk->keyChar = key;

	return 1;
}

int Game::LuaGetHotkey(lua_State* L) {
	int id = LuaScript::PopNumber(L);

	Game* game = Game::game;
	HotKey* hk = (game ? game->GetHotKey(id) : NULL);

	unsigned short keyChar = 0;
	if (hk)
		keyChar = hk->keyChar;

	LuaScript::PushNumber(L, keyChar);
	return 1;
}

int Game::LuaClearHotkey(lua_State* L) {
	int id = LuaScript::PopNumber(L);

	Game* game = Game::game;
	if (game)
		game->ClearHotKey(id);

	return 1;
}

int Game::LuaSetHotkeyDisabledItemChange(lua_State* L) {
	int value = LuaScript::PopNumber(L);
	int id = LuaScript::PopNumber(L);

	Game* game = Game::game;
	HotKey* hk = (game ? game->GetHotKey(id) : NULL);

	if (hk)
		hk->disabledItemChange = (bool)value;

	return 1;
}

int Game::LuaGetHotkeyItemContainer(lua_State* L) {
	int id = LuaScript::PopNumber(L);

	Game* game = Game::game;
	HotKey* hk = (game ? game->GetHotKey(id) : NULL);

	WindowElementItemContainer* itemContainer = NULL;
	if (hk)
		itemContainer = hk->itemContainer;

	LuaScript::PushNumber(L, (unsigned long)itemContainer);
	return 1;
}

int Game::LuaSetHotkeyItemContainer(lua_State* L) {
	WindowElementItemContainer* itemContainer = (WindowElementItemContainer*)((unsigned long)LuaScript::PopNumber(L));
	int id = LuaScript::PopNumber(L);

	Game* game = Game::game;
	HotKey* hk = (game ? game->GetHotKey(id) : NULL);

	if (hk)
		hk->itemContainer = itemContainer;

	return 1;
}

int Game::LuaDivideText(lua_State* L) {
	int lineWidth = LuaScript::PopNumber(L);
	int fontSize = LuaScript::PopNumber(L);
	AD2D_Font* font = (AD2D_Font*)((unsigned long)LuaScript::PopNumber(L));
	std::string text = LuaScript::PopString(L);

	TextString scrolled = ScrollText(TextString(text), font, fontSize, lineWidth);
	DividedText divided = DivideText(scrolled);

	lua_newtable(L);
	int i = 1;
	for (DividedText::iterator it = divided.begin(); it != divided.end(); it++, i++) {
		TextString text = it->second;

		lua_pushnumber(L, i);
		lua_pushstring(L, text.text.c_str());
		lua_settable(L, -3);
	}
	return 1;
}


//Lua interactive

int Game::LuaDoPlayerLogout(lua_State* L) {
	Game* game = Game::game;
	if (game)
		game->PlayerLogout();

	return 1;
}

int Game::LuaDoPlayerSay(lua_State* L) {
	std::string words = LuaScript::PopString(L);

	Game* game = Game::game;
	if (game) {
		Channel* ch = Channel::GetChannel(CHANNEL_DEFAULT);
		unsigned char sclass = SPEAK_SAY;

		game->CheckCommand(words, sclass);
		game->PlayerSendSay(sclass, ch, words);
	}

	return 1;
}

int Game::LuaDoPlayerMove(lua_State* L) {
	int direction = LuaScript::PopNumber(L);

	Game* game = Game::game;
	if (game)
		game->PlayerMove((Direction)direction);

	return 1;
}

int Game::LuaDoPlayerTurn(lua_State* L) {
	int direction = LuaScript::PopNumber(L);

	Game* game = Game::game;
	if (game)
		game->PlayerTurn((Direction)direction);

	return 1;
}

int Game::LuaDoPlayerAttack(lua_State* L) {
	unsigned int creatureID = LuaScript::PopNumber(L);

	Game* game = Game::game;
	if (game)
		game->PlayerAttack(creatureID);

	return 1;
}

int Game::LuaDoPlayerFollow(lua_State* L) {
	unsigned int creatureID = LuaScript::PopNumber(L);

	Game* game = Game::game;
	if (game)
		game->PlayerFollow(creatureID);

	return 1;
}

int Game::LuaDoMoveThing(lua_State* L) {
	int count = LuaScript::PopNumber(L);
	Position posTo = LuaScript::PopPosition(L);
	int stackPos = LuaScript::PopNumber(L);
	int itemID = LuaScript::PopNumber(L);
	Position posFrom = LuaScript::PopPosition(L);

	Game* game = Game::game;
	if (game)
		game->PlayerMoveThing(posFrom, itemID, stackPos, posTo, count);

	return 1;
}

int Game::LuaDoUseThing(lua_State* L) {
	int params = lua_gettop(L);

	int lastIndex = Container::GetFreeIndex();
	if (params > 3)
		lastIndex = LuaScript::PopNumber(L);
	int stackPos = LuaScript::PopNumber(L);
	int itemID = LuaScript::PopNumber(L);
	Position pos = LuaScript::PopPosition(L);

	Game* game = Game::game;
	if (game)
		game->PlayerUseThing(pos, itemID, stackPos, lastIndex);

	return 1;
}

int Game::LuaDoUseWithThing(lua_State* L) {
	int toStackPos = LuaScript::PopNumber(L);
	int toItemID = LuaScript::PopNumber(L);
	Position posTo = LuaScript::PopPosition(L);
	int fromStackPos = LuaScript::PopNumber(L);
	int fromItemID = LuaScript::PopNumber(L);
	Position posFrom = LuaScript::PopPosition(L);

	Game* game = Game::game;
	if (game)
		game->PlayerUseWithThing(posFrom, fromItemID, fromStackPos, posTo, toItemID, toStackPos);

	return 1;
}

int Game::LuaDoUseThingBattle(lua_State* L) {
	unsigned int creatureID = LuaScript::PopNumber(L);
	int count = LuaScript::PopNumber(L);
	int itemID = LuaScript::PopNumber(L);

	Game* game = Game::game;
	if (game) {
		Position pos(0xFFFF, 0x00, 0x00);
		game->PlayerUseBattleThing(pos, itemID, count, creatureID);
	}

	return 1;
}

int Game::LuaDoUseHotkey(lua_State* L) {
	int id = LuaScript::PopNumber(L);

	Game* game = Game::game;
	if (game)
		game->ExecuteHotKey(id);

	return 1;
}

int Game::LuaDoContainerClose(lua_State* L) {
	int index = LuaScript::PopNumber(L);

	Game* game = Game::game;
	if (game)
		game->PlayerContainerClose(index);

	return 1;
}

int Game::LuaDoContainerMoveUp(lua_State* L) {
	int index = LuaScript::PopNumber(L);

	Game* game = Game::game;
	if (game)
		game->PlayerContainerMoveUp(index);

	return 1;
}

int Game::LuaDoAddMagicEffect(lua_State* L) {
	int type = LuaScript::PopNumber(L);
	Position pos = LuaScript::PopPosition(L);

	MagicEffect* me = new(M_PLACE) MagicEffect;
	if (!me->Create(type, pos)) {
		delete_debug(me, M_PLACE);
		return 0;
	}

	return 1;
}

int Game::LuaDoAddDistanceShot(lua_State* L) {
	int type = LuaScript::PopNumber(L);
	Position toPos = LuaScript::PopPosition(L);
	Position fromPos = LuaScript::PopPosition(L);

	Distance* dist = new(M_PLACE) Distance(type, fromPos, toPos);

	return 1;
}

int Game::LuaDoAddAnimatedText(lua_State* L) {
	std::string text = LuaScript::PopString(L);
	int color = LuaScript::PopNumber(L);
	Position position = LuaScript::PopPosition(L);
	unsigned int interval = LuaScript::PopNumber(L);

	Messages::AddMessage(new(M_PLACE) AnimatedMessage(TextString(text, color), interval, position));
	return 1;
}

int Game::LuaDoRequestChannels(lua_State* L) {
	Game* game = Game::game;
	if (game)
		game->PlayerRequestChannels();

	return 1;
}

int Game::LuaDoRequestOutfit(lua_State* L) {
	Game* game = Game::game;
	if (game)
		game->PlayerRequestOutfit();

	return 1;
}

int Game::LuaDoRequestSpells(lua_State* L) {
	Game* game = Game::game;
	if (game)
		game->PlayerRequestSpells();

	return 1;
}

int Game::LuaDoRequestQuestLog(lua_State* L) {
	Game* game = Game::game;
	if (game)
		game->PlayerRequestQuestLog();

	return 1;
}

int Game::LuaDoQuit(lua_State* L) {
	Game::PostQuit();

	return 1;
}


void Game::LuaRegisterFunctions(lua_State* L) {
//System
	//messageBox(text)
	lua_register(L, "messageBox", Game::LuaMessageBox);

	//getLanguage()
	lua_register(L, "getLanguage", Game::LuaGetLanguage);

	//getMouse() : mousePtr
	lua_register(L, "getMouse", Game::LuaGetMouse);

	//getKeyboard() : keyboardPtr
	lua_register(L, "getKeyboard", Game::LuaGetKeyboard);

	//getResolution() : resolution{x, y}
	lua_register(L, "getResolution", Game::LuaGetResolution);

	//takeScreenshot()
	lua_register(L, "takeScreenshot", Game::LuaTakeScreenshot);

//Pointer
	//setPointer(pointerTypeStr, pointerPtr)
	lua_register(L, "setPointer", Game::LuaSetPointer);

	//getPointer(pointerTypeStr) : pointerPtr
	lua_register(L, "getPointer", Game::LuaGetPointer);


//Hotkeys
	//getHotkeyPtr(id) : hotkeyPtr
	lua_register(L, "getHotkeyPtr", Game::LuaGetHotkeyPtr);

	//setHotkey(id, key)
	lua_register(L, "setHotkey", Game::LuaSetHotkey);

	//getHotkey(id) : key
	lua_register(L, "getHotkey", Game::LuaGetHotkey);

	//clearHotkey(id)
	lua_register(L, "clearHotkey", Game::LuaClearHotkey);

	//setHotkeyDisabledItemChange(id, value)
	lua_register(L, "setHotkeyDisabledItemChange", Game::LuaSetHotkeyDisabledItemChange);

	//getHotkeyItemContainer(id) : itemcontainerPtr
	lua_register(L, "getHotkeyItemContainer", Game::LuaGetHotkeyItemContainer);

	//setHotkeyItemContainer(id, itemcontainerPtr)
	lua_register(L, "setHotkeyItemContainer", Game::LuaSetHotkeyItemContainer);

//Tools
	//divideText(textStr, fontPtr, fontSize, lineWidth) : lines{lineStr1, lineStr2, ...}
	lua_register(L, "divideText", Game::LuaDivideText);

//Interactive
	//doPlayerLogout()
	lua_register(L, "doPlayerLogout", Game::LuaDoPlayerLogout);

	//doPlayerSay(wordsStr)
	lua_register(L, "doPlayerSay", Game::LuaDoPlayerSay);

	//doPlayerMove(direction)
	lua_register(L, "doPlayerMove", Game::LuaDoPlayerMove);

	//doPlayerTurn(direction)
	lua_register(L, "doPlayerTurn", Game::LuaDoPlayerTurn);

	//doPlayerAttack(creatureID)
	lua_register(L, "doPlayerAttack", Game::LuaDoPlayerAttack);

	//doPlayerFollow(creatureID)
	lua_register(L, "doPlayerFollow", Game::LuaDoPlayerFollow);

	//doMoveThing(fromPosition, fromItemId, fromStackPos, toPosition, count)
	lua_register(L, "doMoveThing", Game::LuaDoMoveThing);

	//doUseThing(position, itemID, stackPos)
	lua_register(L, "doUseThing", Game::LuaDoUseThing);

	//doUseWithThing(fromPosition, fromItemID, fromStackPos, toPosition, toItemID, toStackPos)
	lua_register(L, "doUseWithThing", Game::LuaDoUseWithThing);

	//doUseThingBattle(itemID, count, creatureID)
	lua_register(L, "doUseThingBattle", Game::LuaDoUseThingBattle);

	//doUseHotkey(id)
	lua_register(L, "doUseHotkey", Game::LuaDoUseHotkey);

	//doContainerClose(index)
	lua_register(L, "doContainerClose", Game::LuaDoContainerClose);

	//doContainerMoveUp(index)
	lua_register(L, "doContainerMoveUp", Game::LuaDoContainerMoveUp);

	//doAddMagicEffect(position, type)
	lua_register(L, "doAddMagicEffect", Game::LuaDoAddMagicEffect);

	//doAddDistanceShot(fromPosition, toPosition, type)
	lua_register(L, "doAddDistanceShot", Game::LuaDoAddDistanceShot);

	//doAddAnimatedText(interval, position, color, textStr)
	lua_register(L, "doAddAnimatedText", Game::LuaDoAddAnimatedText);

	//doRequestChannels()
	lua_register(L, "doRequestChannels", Game::LuaDoRequestChannels);

	//doRequestOutfit()
	lua_register(L, "doRequestOutfit", Game::LuaDoRequestOutfit);

	//doRequestSpells()
	lua_register(L, "doRequestSpells", Game::LuaDoRequestSpells);

	//doRequestQuestLog()
	lua_register(L, "doRequestQuestLog", Game::LuaDoRequestQuestLog);

	//doQuit()
	lua_register(L, "doQuit", Game::LuaDoQuit);
}
