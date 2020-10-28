/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#ifndef __GAME_H_
#define __GAME_H_

#include <list>
#include <string>

#include "bot.h"
#include "const.h"
#include "channel.h"
#include "cooldowns.h"
#include "craftbox.h"
#include "icons.h"
#include "item.h"
#include "logger.h"
#include "luascript.h"
#include "map.h"
#include "minimap.h"
#include "messages.h"
#include "mthread.h"
#include "particle.h"
#include "player.h"
#include "protocol.h"
#include "questlog.h"
#include "servers.h"
#include "shop.h"
#include "status.h"
#include "trade.h"
#include "updater.h"
#include "viplist.h"
#include "window.h"

#ifndef NO_SOUND
	#include "sound.h"
#endif

class Tile;
class Map;
class MiniMap;
class Player;
class Protocol;
class Cooldowns;
class QuestLog;
class Status;

class Particle;
class Particles;

class WindowElement;
class WindowElementTextarea;
class Windows;
class Window;


enum GAMESTATE {
	GAME_IDLE,
	GAME_INQUEUE,
	GAME_LOGGEDTOSERVER,
	GAME_LOGGEDTOGAME,
	GAME_RELOGIN,
};


struct Host {
	std::string account;
	std::string password;
	std::string host;
	std::string port;
	std::string update;

	std::string name;
	std::string protocol;
	std::string website;
	std::string createacc;
	unsigned short PACC;
};

struct Character {
	std::string name;
	std::string serv;
	unsigned int servIP;
	unsigned short servPort;
};

struct Options {
	unsigned char graphicsAPI;
	unsigned char limitFPS;
	unsigned char ambientLight;
	unsigned char fightModes;
	unsigned char language;
	bool showNames;
	bool showBars;
	bool dontStrech;
	bool printParticles;
	bool printOneLevel;
	bool renderToTexture;
	bool classicControls;
	bool autoUpdate;

	unsigned char textSize;
	unsigned char flyingSpeed;
	unsigned char disappearingSpeed;

	bool fixedPositions;
	bool fadeOutInactive;

	unsigned char musicGain;
	unsigned char soundGain;
	std::string updateURL;
	unsigned short protocol;

	std::string templatesGroup;
};

struct AdminOptions {
	bool disableBot;
	bool hideLevels;
	bool fullLight;
	bool simpleLogin;
	std::string rsa_p;
	std::string rsa_q;
	std::string rsa_d;
};

struct HotKey {
	unsigned short keyChar;
	std::string text;
	unsigned short itemID;
	unsigned char fluid;
	unsigned char spellID;
	unsigned char mode;
	bool disabledItemChange;

	WindowElementItemContainer* itemContainer;
};

struct InputActions {
	bool actWalk;
	bool actLook;
	bool actUse;
	bool actAttack;
	bool actOpenMenu;
};

typedef std::pair<std::string, std::string> SpellPair;
typedef std::list<std::pair<unsigned char, SpellPair> > SpellsList;

typedef std::list<Character> CharactersList;
typedef std::list<Channel> ChannelsList;


class Game {
private:
	Icons*		icons;
	Particles*	particles;
#ifndef NO_SOUND
	SFX_System*	sfx;
#endif

	Windows*	wnds;
	Window*		wndStatus;
	Window*		wndHotkeys;
	Window*		wndGame;
	Window*		wndConsole;
	Window*		wndMiniMap;
	Window*		wndInventory;
	Window*		wndStatistics;
	Window*		wndBattle;
	Window*		wndVIPList;

	WindowElementTab*		consoleTab;
	WindowElementTextarea*	cmdLine;

	Host			host;
	Character		character;
	CharactersList	charactersList;
	ChannelsList	channelsList;

	HotKey			hotkey[48];

	Protocol*		protocol;
	Map*			map;
	Player*			player;
	VIPList*		viplist;
	Shop*			shop;
	Trade*			trade;
	CraftBox*		craftbox;
	Cooldowns*		cooldowns;
	QuestLog*		questlog;
	Status*			status;
	Updater*		updater;
	Servers*		servers;
	Bot*			bot;

	Mouse*		mouse;
	Keyboard*	keyboard;

	unsigned char	gameState;

public:
	static MUTEX	lockGame;

	static std::list<std::pair<std::string, std::string> > openChannels;

	static Signal	taskManager;

	static bool		running;
	static bool		releaseSprites;
	static bool		screenshot;

	static bool		systemMusic;

	static std::string	dataLocation;
	static std::string  filesLocation;

	static Options		options;
	static AdminOptions	adminOptions;
	static Game*		game;

public:
	Game();
	~Game();

	static void CheckPrivateData(std::string func, std::string desc);

	Options* GetOptions();

	void SetParticles(Particles* particles);
	Particles* GetParticles();

#ifndef NO_SOUND
	void SetSFX(SFX_System* sfx);
	SFX_System* GetSFX();

	void UpdateBackgroundSound(unsigned char level, bool firstCheck = true);
#endif

	void SetIcons(Icons* icons);
	Icons* GetIcons();

	void SetWindows(Windows* wnds);
	Windows* GetWindows();

	void SetWindowStatus(Window* wnd);
	Window* GetWindowStatus();

	void SetWindowHotkeys(Window* wnd);
	Window* GetWindowHotkeys();

	void SetWindowGame(Window* wnd);
	Window* GetWindowGame();

	void SetWindowConsole(Window* wnd);
	Window* GetWindowConsole();

	void SetWindowMiniMap(Window* wnd);
	Window* GetWindowMiniMap();

	void SetWindowInventory(Window* wnd);
	Window* GetWindowInventory();

	void SetWindowStatistics(Window* wnd);
	Window* GetWindowStatistics();

	void SetWindowBattle(Window* wnd);
	Window* GetWindowBattle();

	void SetWindowVIPList(Window* wnd);
	Window* GetWindowVIPList();

	void SetConsoleTab(WindowElementTab* tab);
	WindowElementTab* GetConsoleTab();

	void SetCmdLine(WindowElementTextarea* textarea);
	WindowElementTextarea* GetCmdLine();

	Window* GetActiveWindow();
	WindowElement* GetActiveWindowElement();

	void SetHost(Host host);
	Host GetHost();
	Host* GetHostPtr();

	void SetCharacter(Character character);
	Character GetCharacter();

	void AddCharacter(Character character);
	CharactersList GetCharacters();
	void ClearCharactersList();

	void AddChannel(Channel channel);
	ChannelsList GetChannels();
	void ClearChannelsList();

	void OpenChannel(unsigned short channelID, std::string channelName, ChatUsers& users, ChatUsers& invited, bool closeable = true);
	void CloseChannel(unsigned short channelID);
	void CloseChannel(std::string channelName);

	void SetProtocol(Protocol* protocol);
	Protocol* GetProtocol();

	void SetMap(Map* map);
	Map* GetMap();

	void SetPlayer(Player* player);
	Player* GetPlayer();

	void SetVIPList(VIPList* viplist);
	VIPList* GetVIPList();

	void SetShop(Shop* shop);
	Shop* GetShop();

	void SetTrade(Trade* trade);
	Trade* GetTrade();

	void SetCraftBox(CraftBox* craftbox);
	CraftBox* GetCraftBox();

	void SetCooldowns(Cooldowns* cooldowns);
	Cooldowns* GetCooldowns();

	void SetQuestLog(QuestLog* questlog);
	QuestLog* GetQuestLog();

	void SetStatus(Status* status);
	Status* GetStatus();

	void SetUpdater(Updater* updater);
	Updater* GetUpdater();

	void SetServers(Servers* servers);
	Servers* GetServers();

	void SetBot(Bot* bot);
	Bot* GetBot();

	void SetGameState(unsigned char state);
	unsigned char GetGameState();

	void SetWorldLight(unsigned char worldLightLevel, unsigned char worldLightColor);
	unsigned char GetWorldLightLevel();
	unsigned char GetWorldLightColor();

	void SetMouse(Mouse* mouse);
	Mouse* GetMouse();

	void SetKeyboard(Keyboard* keyboard);
	Keyboard* GetKeyboard();

	void LoadData();

	void SetHotKey(unsigned char key, HotKey hotkey);
	HotKey* GetHotKey(unsigned char key);
	void ClearHotKey(unsigned char key);
	void ExecuteHotKey(unsigned char hotkey);
	void SaveHotKeys();
	void LoadHotKeys();
	void SaveMarkers();
	void LoadMarkers();
	void SaveWaypoints(std::string filename = "");
	void LoadWaypoints(std::string filename = "");

	void PreLoadOptions();
	void SaveOptions();
	void LoadOptions();
	void SaveAdminOptions();
	void LoadAdminOptions();

	void SaveChannelText(Channel* channel);
	void SaveChannels();
	void LoadChannels();

	void SaveErrorLog(std::string filename);
	void LoadErrorLog();

	void SendSystemMessage(std::string message);

	void PlayerLogout();
	void PlayerAutoWalk(std::list<Direction>& list);
	void PlayerMove(Direction direction);
	void PlayerTurn(Direction direction);
	void PlayerStopAutoWalk();
	void PlayerStopWalk();
	void PlayerSendSay(unsigned char speakClass, Channel* channel, std::string message);
	void PlayerLookAt(Position pos, unsigned short itemId, unsigned char stackPos);
	void PlayerMoveThing(Position fromPos, unsigned short itemId, unsigned char stackPos, Position toPos, unsigned char count);
	void PlayerUseThing(Position fromPos, unsigned short itemId, unsigned char stackPos, unsigned char index);
	void PlayerUseWithThing(Position fromPos, unsigned short fromItemId, unsigned char fromStackPos, Position toPos, unsigned short toItemId, unsigned char toStackPos);
	void PlayerUseBattleThing(Position fromPos, unsigned short itemId, unsigned char stackPos, unsigned int creatureID);
	void PlayerRotateItem(Position pos, unsigned short itemId, unsigned char stackPos);
	void PlayerRequestTrade(Position pos, unsigned short itemId, unsigned char stackPos, unsigned int creatureID);
	void PlayerLookInTrade(unsigned char counterOffer, unsigned char index);
	void PlayerAcceptTrade();
	void PlayerCancelTrade();
	void PlayerLookInShop(unsigned short itemID, unsigned char type);
	void PlayerPurchaseShop(unsigned short itemID, unsigned char type, unsigned char count, bool ignoreCap, bool inBackpack);
	void PlayerSaleShop(unsigned short itemID, unsigned char type, unsigned char count);
	void PlayerCloseShop();
	void PlayerContainerClose(unsigned char index);
	void PlayerContainerMoveUp(unsigned char index);
	void PlayerToggleMount(bool mount);
	void PlayerSetFightModes(unsigned char fightModes);
	void PlayerAttack(unsigned int creatureID);
	void PlayerFollow(unsigned int creatureID);
	void PlayerRequestChannels();
	void PlayerOpenChannel(unsigned short channelID, std::string channelName);
	void PlayerCloseChannel(unsigned short channelID);
	void PlayerProcessRuleViolation(std::string reporter);
	void PlayerCloseRuleViolation(std::string reporter);
	void PlayerCancelRuleViolation();
	void PlayerAddVIP(std::string creatureName);
	void PlayerRemoveVIP(unsigned int creatureID);
	void PlayerTextWindow(unsigned int textID, std::string text);
	void PlayerHouseWindow(unsigned char listID, unsigned int textID, std::string text);
	void PlayerRequestOutfit();
	void PlayerSetOutfit(Outfit outfit);
	void PlayerPartyInvite(unsigned int creatureID);
	void PlayerPartyJoin(unsigned int creatureID);
	void PlayerPartyRevoke(unsigned int creatureID);
	void PlayerPartyPassLeadership(unsigned int creatureID);
	void PlayerPartyLeave();
	void PlayerPartyEnableShared(unsigned char active);
	void PlayerRequestQuestLog();
	void PlayerRequestQuestLine(unsigned short questID);
	void PlayerCloseNPC();
	void PlayerRequestSpells();

	void CheckCommand(std::string& text, unsigned char& speakClass);

	void CheckCreatures(float factor);
	void CheckMovingItems(float factor);
	void CheckPlayer();

	void onMakeAction(Windows* wnds, Window* wnd, unsigned char action, void* data);
	void onLogin(Windows* wnds, Window* wnd, std::vector<void*> pointers);
	void onEnterGame(Windows* wnds, Window* wnd, unsigned short* number);
	void onQueue(Windows* wnds, Window* wnd);
	void onConsoleEnter(Windows* wnds, WindowElementTextarea* from_textarea, TabElement* element);
	void onOpenChannel(Windows* wnds, Window* wnd, unsigned short* number, std::string* name);
	void onAddVIP(Windows* wnds, Window* wnd, std::string* name);
	void onTextWindow(Windows* wnds, Window* wnd, unsigned int textID, std::string* text);
	void onHouseWindow(Windows* wnds, Window* wnd, unsigned char listID, unsigned int textID, std::string* text);
	void onShop(Windows* wnds, Window* wnd, std::vector<void*> pointers, int sw, bool scroll, bool accept, bool checkboxes);
	void onTrade(Windows* wnds, Window* wnd, WindowElementButton* accept, bool reject);
	void onChangeOutfit(Windows* wnds, Window* wnd, unsigned char action, std::vector<void*> pointers);
	void onQuestLog(Windows* wnds, Window* wnd, WindowElementMemo* memo1, WindowElementMemo* memo2, WindowElementTextarea* textarea);
	void onSetHotKey(Windows* wnds, Window* wnd, Keyboard* keyboard, unsigned char key, std::string* text);
	void onOptions(Windows* wnds, Window* wnd, unsigned char option);
	void onGame(Windows* wnds, Window* wnd, WindowElementMemo* memo, WindowElementCheckBox* cb_autoupdate, WindowElementCheckBox* cb_controls);
	void onTemplate(Windows* wnds, Window* wnd, WindowElementMemo* memo);
	void onGraphics(Windows* wnds, Window* wnd, std::vector<void*> pointers);
	void onText(Windows* wnds, Window* wnd, std::vector<void*> pointers);
	void onWindows(Windows* wnds, Window* wnd, std::vector<void*> pointers);
	void onSounds(Windows* wnds, Window* wnd);
	void onError(Windows* wnds, Window* wnd, Host host, Character character, std::string error_log, std::string* comment);
	void onUpdate(Windows* wnds, Window* wnd);
	void onServers(std::vector<void*> pointers);
	void onAddFavoriteServer(bool openWindow, std::vector<void*> pointers1, std::vector<void*> pointers2);
	void onRemoveFavoriteServer(WindowElementTableMemo* memo, std::vector<void*> pointers);
	void onGoToURLServer(WindowElement* memo, int number, bool website);
	void onMoveFavoriteServer(WindowElementTableMemo* memo, int step, std::vector<void*> pointers);
	void onSetServer(WindowElementTableMemo* memo, std::vector<void*> pointers);
	void onSlideChannelUsers(WindowElementButton* slider, WindowElementTextarea* textarea, WindowElementMemo* memo);
	void onMarker(Window* wnd, std::list<void*>* pointers, WindowElementTextarea* comment, Position pos, int button, bool release);
	void onWaypoint(Window* wnd, std::list<void*>* pointers, WindowElementTextarea* comment, Position pos, int button, bool release);
	void onSaveWaypoints(Windows* wnds, Window* wnd, std::string* name);
	void onLoadWaypoints(Windows* wnds, Window* wnd, WindowElementMemo* memo);
	void onBotApply(std::vector<void*> pointers);
	void onBotLists(WindowElementMemo* me_friends, WindowElementMemo* me_enemies, WindowElementMemo* me_foodlist, WindowElementMemo* me_lootlist);
	void onOldHotkey(Windows* wnds, Window* wnd, std::vector<void*> pointers, int action);
	void onHomePage();

	static void SetClipboard(std::string text);
	static std::string GetClipboard();

	static void AddTask(boost::function<void()> func);
	static void ExecuteTasks();

	static void PostExecuteFile(std::string file, std::string directory);
	static void PostOpenBrowser(std::string link);
	static void PostQuit();

	//Lua functions
	static int LuaMessageBox(lua_State* L);
	static int LuaGetLanguage(lua_State* L);
	static int LuaGetMouse(lua_State* L);
	static int LuaGetKeyboard(lua_State* L);
	static int LuaGetResolution(lua_State* L);
	static int LuaTakeScreenshot(lua_State* L);

	static int LuaSetPointer(lua_State* L);
	static int LuaGetPointer(lua_State* L);

	static int LuaGetHotkeyPtr(lua_State* L);
	static int LuaSetHotkey(lua_State* L);
	static int LuaGetHotkey(lua_State* L);
	static int LuaClearHotkey(lua_State* L);
	static int LuaSetHotkeyDisabledItemChange(lua_State* L);
	static int LuaGetHotkeyItemContainer(lua_State* L);
	static int LuaSetHotkeyItemContainer(lua_State* L);

	//Lua tools
	static int LuaDivideText(lua_State* L);

	//Lua interactive
	static int LuaDoPlayerLogout(lua_State* L);
	static int LuaDoPlayerSay(lua_State* L);
	static int LuaDoPlayerMove(lua_State* L);
	static int LuaDoPlayerTurn(lua_State* L);
	static int LuaDoPlayerAttack(lua_State* L);
	static int LuaDoPlayerFollow(lua_State* L);
	static int LuaDoMoveThing(lua_State* L);
	static int LuaDoUseThing(lua_State* L);
	static int LuaDoUseWithThing(lua_State* L);
	static int LuaDoUseThingBattle(lua_State* L);
	static int LuaDoUseHotkey(lua_State* L);
	static int LuaDoContainerClose(lua_State* L);
	static int LuaDoContainerMoveUp(lua_State* L);
	static int LuaDoAddMagicEffect(lua_State* L);
	static int LuaDoAddDistanceShot(lua_State* L);
	static int LuaDoAddAnimatedText(lua_State* L);
	static int LuaDoRequestChannels(lua_State* L);
	static int LuaDoRequestOutfit(lua_State* L);
	static int LuaDoRequestSpells(lua_State* L);
	static int LuaDoRequestQuestLog(lua_State* L);
	static int LuaDoQuit(lua_State* L);

	static void LuaRegisterFunctions(lua_State* L);
};

#endif //__LOGGER_H_
