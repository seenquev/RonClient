/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#include <windows.h>
#include <iostream>
#include <string>
#include <sstream>
#include <stdio.h>

#include "allocator.h"
#include "ad2d.h"
#include "bot.h"
#include "const.h"
#include "container.h"
#include "cooldowns.h"
#include "game.h"
#include "icons.h"
#include "iniloader.h"
#include "input.h"
#include "item.h"
#include "filemanager.h"
#include "light.h"
#include "logger.h"
#include "luascript.h"
#include "map.h"
#include "minimap.h"
#include "mthread.h"
#include "particle.h"
#include "protocol.h"
#include "protocol822.h"
#include "protocol840.h"
#include "protocol842.h"
#include "protocol850.h"
#include "protocol854.h"
#include "protocol860.h"
#include "protocol870.h"
#include "protocol910.h"
#include "questlog.h"
#include "realtime.h"
#include "shop.h"
#include "status.h"
#include "text.h"
#include "trade.h"
#include "updater.h"
#include "window.h"
#include "viplist.h"
#include "tools.h"

#ifndef NO_SOUND
	#include "adal.h"
	#include "sound.h"
#endif


/*  Declare Windows procedure  */
LRESULT CALLBACK WindowProcedure (HWND, UINT, WPARAM, LPARAM);


/*  Make the class name into a global variable  */
char szClassName[ ] = "RonClientClass";


int WINAPI WinMain (HINSTANCE hThisInstance,
                    HINSTANCE hPrevInstance,
                    LPSTR lpszArgument,
                    int nFunsterStil)
{
	FileManager fileManager;

	HWND hwnd;               /* This is the handle for our window */
	MSG messages;            /* Here messages to the application are saved */
	WNDCLASSEX wincl;        /* Data structure for the windowclass */

	/* The Window structure */
	wincl.hInstance = hThisInstance;
	wincl.lpszClassName = szClassName;
	wincl.lpfnWndProc = WindowProcedure;      /* This function is called by windows */
	wincl.style = CS_DBLCLKS;                 /* Catch double-clicks */
	wincl.cbSize = sizeof (WNDCLASSEX);

	/* Use default icon and mouse-pointer */
	wincl.hIcon = LoadIcon(hThisInstance, "Icon");
	wincl.hIconSm = LoadIcon(hThisInstance, "Icon");
	wincl.hCursor = NULL;
	wincl.lpszMenuName = NULL;                 /* No menu */
	wincl.cbClsExtra = 0;                      /* No extra bytes after the window class */
	wincl.cbWndExtra = 0;                      /* structure or the window instance */
	/* Use Windows's default color as the background of the window */
	wincl.hbrBackground = (HBRUSH) COLOR_BACKGROUND;

	/* Register the window class, and if it fails quit the program */
	if (!RegisterClassEx (&wincl))
		return 0;

	/* The class is registered, let's create the program*/
	hwnd = CreateWindowEx (
		   WS_EX_APPWINDOW,			/* Extended possibilites for variation */
		   szClassName,				/* Classname */
		   "RonClient           ",	/* Title Text */
		   WS_OVERLAPPEDWINDOW,		/* default window */
		   0,						/* Windows decides the position */
		   0,						/* where the window ends up on the screen */
		   64,						/* The programs width */
		   64,						/* and height in pixels */
		   HWND_DESKTOP,			/* The window is a child-window to desktop */
		   NULL,					/* No menu */
		   hThisInstance,			/* Program Instance handler */
		   NULL						/* No Window Creation data */
		);

	ShowWindow(hwnd, nFunsterStil);

	if (GetKeyState(VK_CAPITAL) & 0x0001)
		PostMessage(hwnd, WM_KEYDOWN, VK_CAPITAL, 0);

	HCURSOR idc_arrow = LoadCursor(NULL, IDC_ARROW);
	HCURSOR idc_move = LoadCursor(NULL, IDC_SIZEALL);
	HCURSOR idc_resize1 = LoadCursor(NULL, IDC_SIZENWSE);
	HCURSOR idc_resize2 = LoadCursor(NULL, IDC_SIZENESW);
	HCURSOR idc_resize_vert = LoadCursor(NULL, IDC_SIZENS);
	HCURSOR idc_resize_hori = LoadCursor(NULL, IDC_SIZEWE);
	HCURSOR idc_cross = LoadCursor(NULL, IDC_CROSS);

	SetCursor(idc_arrow);

//---------------------------------------//

	Game game;
	game.PreLoadOptions();

//---------------------------------------//

	int PosX = 0;
	int PosY = 0;
	int ResX = 800;
	int ResY = 600;

	INILoader iniLoader;
	if (iniLoader.OpenFile("templates/gui.ini")) {
		PosX = atoi(iniLoader.GetValue("POS_X").c_str());
		PosY = atoi(iniLoader.GetValue("POS_Y").c_str());
		ResX = (iniLoader.GetValue("RES_X", 1) != "" ? atoi(iniLoader.GetValue("RES_X", 1).c_str()) : atoi(iniLoader.GetValue("RES_X").c_str()));
		ResY = (iniLoader.GetValue("RES_Y", 1) != "" ? atoi(iniLoader.GetValue("RES_Y", 1).c_str()) : atoi(iniLoader.GetValue("RES_Y").c_str()));
	}

	if (PosX + ResX <= 0) PosX = 0;
	if (PosY + ResY <= 0) PosY = 0;

	RealTime	RT;
	RT.Calculate();

//---------------------------------------//

	Logger::OpenSession();

	AD2D_Window gfx(hwnd, PosX, PosY, ResX, ResY, 32, (APIMode)Game::options.graphicsAPI);

	if (atoi(iniLoader.GetValue("MAXIMIZED").c_str()) != 0) {
		gfx.SetMaximized(true);
		ShowWindow(hwnd, SW_MAXIMIZE);
	}
	else
		gfx.SetMaximized(false);

	AD2D_Viewport mainVP;
	mainVP.Create(0, 0, ResX, ResY);
	gfx.SetViewport(mainVP);

//---------------------------------------//

	AD2D_Font font;
	font.CreatePNG_(fileManager.GetFileData("font.png"), fileManager.GetFileSize("font.png"));
	font.ReadWidthMap_(fileManager.GetFileData("font.dat"), fileManager.GetFileSize("font.dat"));

//---------------------------------------//

	AD2D_Image backgroundImage;
	backgroundImage.CreatePNG_(fileManager.GetFileData("background.png"), fileManager.GetFileSize("background.png"));
	AD2D_Window::SetColor(0.5f, 0.5f, 0.5f);
	if (backgroundImage.GetWidth() > 256)
		gfx.PutImage(0, 0, ResX, ResY, backgroundImage);
	else {
		for (int x = 0; x <= ResX / backgroundImage.GetWidth(); x++)
		for (int y = 0; y <= ResY / backgroundImage.GetHeight(); y++)
			gfx.PutImage(x * backgroundImage.GetWidth(), y * backgroundImage.GetHeight(), backgroundImage);
	}

	std::string text = Text::GetText("LOADING", Game::options.language);
	int width = font.GetTextWidth(text, 16);
	AD2D_Window::SetColor(1.0f, 1.0f, 1.0f);
	gfx.Print((ResX - width) / 2, (ResY - 16) / 2, 16, font, text.c_str());
	gfx.SwapBuffers();

//---------------------------------------//

	Icons icons;
	icons.LoadIcons("icons");

//---------------------------------------//

	FPS			fps;
	Mouse		mouse;
	Keyboard	keyboard;

	Particles particles;

	fps.Create(25);

	srand(RT.getTime());

	game.SetIcons(&icons);
	game.SetParticles(&particles);
#ifndef NO_SOUND
	SFX_System sfx;
	game.SetSFX(&sfx);
#endif

	MiniMap minimap;
	Battle battle;
	VIPList viplist;
	Shop shop;
	Trade trade;
	CraftBox craftbox;
	Cooldowns cooldowns;
	QuestLog questlog;
	Status status;
	Updater updater;
	Servers servers;
	Bot bot;

	Map map;
	map.SetMiniMap(&minimap);
	map.SetBattle(&battle);

	Player player;
	game.SetMap(&map);
	game.SetPlayer(&player);
	game.SetVIPList(&viplist);
	game.SetShop(&shop);
	game.SetTrade(&trade);
	game.SetCraftBox(&craftbox);
	game.SetCooldowns(&cooldowns);
	game.SetQuestLog(&questlog);
	game.SetStatus(&status);
	game.SetUpdater(&updater);
	game.SetServers(&servers);
	game.SetBot(&bot);
	game.SetMouse(&mouse);
	game.SetKeyboard(&keyboard);

	servers.LoadFavorites();
	servers.LoadWebsites();
	game.LoadOptions();
	game.LoadAdminOptions();

	Windows wnds(hwnd, &gfx, &game);

	game.SetWindows(&wnds);
	game.SetProtocol(NULL);

	wnds.LoadTemplates(&font);

	Window* wnd = wnds.OpenWindow(WND_OPTIONS, &game);
	wnd->SetMinimizeAbility(false);

	wnds.OpenWindow(WND_STARTUP, &game);
	game.LoadErrorLog();

	if (Game::options.autoUpdate)
		updater.StartUpdating(Game::options.updateURL, "engine", ".", true);

	std::string appReview = std::string(APP_NAME) + " " + std::string(APP_VERSION);

	LuaScript::RunScript("startup", true, false);

	int tick = 0;
	int PERIOD = 0;
	float retFPS = 0.0f;
	while(Game::running)
	{
		Game::ExecuteTasks();

		gfx.SetViewport(mainVP);
		while(PeekMessage(&messages, NULL, 0, 0, PM_REMOVE)) {
			keyboard.UpdateKeyboard(messages);
			/* Translate virtual-key messages into character messages */
			TranslateMessage(&messages);

			keyboard.UpdateKeyChar(messages);
			mouse.UpdateMouse(messages);

			if (messages.message == WM_KEYDOWN && messages.wParam == VK_F10)
				messages.message = 0;

			if (messages.message == WM_QUIT)
				Game::running = false;

			wnds.Check(mouse, keyboard, RT);

			if (keyboard.key[VK_MENU] && keyboard.keyChar == 255 + 115) {
				if (game.GetGameState() == GAME_LOGGEDTOGAME) {
					wnds.CloseWindows(WND_CLOSE);
					wnds.OpenWindow(WND_CLOSE, &game);
					break;
				}
				else
					PostQuitMessage(0);
				keyboard.keyChar = 0;
			}
			else if (keyboard.keyChar == 255 + VK_INSERT) {
				Game::screenshot = true;
				keyboard.keyChar = 0;
			}
			else if (game.GetGameState() == GAME_LOGGEDTOGAME) {
				if ((keyboard.keyChar >= 255 + 112 && keyboard.keyChar <= 255 + 123) ||
					(keyboard.capsLock && ((keyboard.keyChar >= 48 && keyboard.keyChar <= 57) || (keyboard.keyChar >= 65 && keyboard.keyChar <= 90)) &&
					(keyboard.keyChar != 'W' && keyboard.keyChar != 'S' && keyboard.keyChar != 'A' && keyboard.keyChar != 'D')))
				{
					Window* activeWindow = game.GetActiveWindow();
					if (activeWindow && activeWindow->GetWindowType() == WND_SETHOTKEY)
						activeWindow->ExecuteFunction();
					else {
						unsigned short m = 0x00;
						if (keyboard.key[VK_SHIFT]) m |= 0x8000;
						if (keyboard.key[VK_CONTROL]) m |= 0x4000;

						Window* wnd = wnds.GetWindowUnderCursor(mouse);
						WindowElement* element = (wnd ? wnd->GetWindowElementUnderCursor(mouse) : NULL);

						WindowElementItemContainer* itemContainer = dynamic_cast<WindowElementItemContainer*>(element);
						if (itemContainer && itemContainer->GetHotKey()) {
							HotKey* hk = itemContainer->GetHotKey();

							if (hk->text != "" || hk->itemID != 0 || itemContainer->funcUse.IsExecutable()) {
								if (hk->keyChar != m + keyboard.keyChar)
									hk->keyChar = m + keyboard.keyChar;
								else
									hk->keyChar = 0;
							}
						}
						else {
							for (int i = 0; i < 48; i++) {
								HotKey* hk = game.GetHotKey(i);
								if (hk->keyChar != 0 && hk->keyChar == m + keyboard.keyChar) {
									game.ExecuteHotKey(i);

									WindowElementItemContainer* itemContainer = hk->itemContainer;
									if (itemContainer && itemContainer->funcUse.IsExecutable())
										itemContainer->funcUse.Execute();
								}
							}
						}
					}
				}
				else {
					Window* window = wnds.GetTopWindow();
					if (window && window->GetWindowType() == WND_REPORTERROR)
						keyboard.keyChar = 0;

					if (keyboard.key[VK_MENU] && keyboard.keyChar >= '1' && keyboard.keyChar <= '9') {
						int number = keyboard.keyChar - 48;
						WindowElementTab* consoleTab = game.GetConsoleTab();
						if (consoleTab)
							consoleTab->SetActiveTabByNumber(number);

						keyboard.keyChar = 0;
					}
					else if (!keyboard.key[VK_SHIFT] && !keyboard.key[VK_CONTROL] && keyboard.keyChar == 9) {
						WindowElementTab* consoleTab = game.GetConsoleTab();
						if (consoleTab) {
							int number = consoleTab->GetActiveTabNumber() + 1;
							if (number > consoleTab->GetTabsSize())
								number = 1;

							consoleTab->SetActiveTabByNumber(number);
						}

						keyboard.keyChar = 0;
					}
					else if (keyboard.key[VK_SHIFT] && !keyboard.key[VK_CONTROL] && keyboard.keyChar == 9) {
						WindowElementTab* consoleTab = game.GetConsoleTab();
						if (consoleTab) {
							int number = consoleTab->GetActiveTabNumber() - 1;
							if (number < 1)
								number = consoleTab->GetTabsSize();

							consoleTab->SetActiveTabByNumber(number);
						}

						keyboard.keyChar = 0;
					}

					GUIManager* guiManager = wnds.GetGUIManager();

					WindowElement* element = game.GetActiveWindowElement();
					WindowElementTextarea* textarea = (element ? dynamic_cast<WindowElementTextarea*>(element) : NULL);
					if (!textarea || !textarea->IsEditable()) {
						if ((keyboard.keyChar == 255 + VK_UP || keyboard.keyChar == 255 + VK_NUMPAD8 || (keyboard.capsLock && keyboard.keyChar == 'W')) && (!keyboard.key[VK_SHIFT] && !keyboard.key[VK_CONTROL])) {
							game.PlayerMove(NORTH);
						}
						else if ((keyboard.keyChar == 255 + VK_DOWN || keyboard.keyChar == 255 + VK_NUMPAD2 || (keyboard.capsLock && keyboard.keyChar == 'S')) && (!keyboard.key[VK_SHIFT] && !keyboard.key[VK_CONTROL])) {
							game.PlayerMove(SOUTH);
						}
						else if ((keyboard.keyChar == 255 + VK_LEFT || keyboard.keyChar == 255 + VK_NUMPAD4 || (keyboard.capsLock && keyboard.keyChar == 'A')) && (!keyboard.key[VK_SHIFT] && !keyboard.key[VK_CONTROL])) {
							game.PlayerMove(WEST);
						}
						else if ((keyboard.keyChar == 255 + VK_RIGHT || keyboard.keyChar == 255 + VK_NUMPAD6 || (keyboard.capsLock && keyboard.keyChar == 'D')) && (!keyboard.key[VK_SHIFT] && !keyboard.key[VK_CONTROL])) {
							game.PlayerMove(EAST);
						}
						else if ((keyboard.keyChar == 255 + VK_UP || keyboard.keyChar == 255 + VK_NUMPAD8 || (keyboard.capsLock && keyboard.keyChar == 23)) && (!keyboard.key[VK_SHIFT] && keyboard.key[VK_CONTROL])) {
							game.PlayerTurn(NORTH);
						}
						else if ((keyboard.keyChar == 255 + VK_DOWN || keyboard.keyChar == 255 + VK_NUMPAD2 || (keyboard.capsLock && keyboard.keyChar == 19)) && (!keyboard.key[VK_SHIFT] && keyboard.key[VK_CONTROL])) {
							game.PlayerTurn(SOUTH);
						}
						else if ((keyboard.keyChar == 255 + VK_LEFT || keyboard.keyChar == 255 + VK_NUMPAD4 || keyboard.keyChar == 255 + VK_NUMPAD2 || (keyboard.capsLock && keyboard.keyChar == 1)) && (!keyboard.key[VK_SHIFT] && keyboard.key[VK_CONTROL])) {
							game.PlayerTurn(WEST);
						}
						else if ((keyboard.keyChar == 255 + VK_RIGHT || keyboard.keyChar == 255 + VK_NUMPAD6 || (keyboard.capsLock && keyboard.keyChar == 4)) && (!keyboard.key[VK_SHIFT] && keyboard.key[VK_CONTROL])) {
							game.PlayerTurn(EAST);
						}
						else if (keyboard.keyChar == 255 + VK_NUMPAD9 && !keyboard.key[VK_SHIFT] && !keyboard.key[VK_CONTROL]) {
							game.PlayerMove(NORTHEAST);
						}
						else if (keyboard.keyChar == 255 + VK_NUMPAD3 && !keyboard.key[VK_SHIFT] && !keyboard.key[VK_CONTROL]) {
							game.PlayerMove(SOUTHEAST);
						}
						else if (keyboard.keyChar == 255 + VK_NUMPAD7 && !keyboard.key[VK_SHIFT] && !keyboard.key[VK_CONTROL]) {
							game.PlayerMove(NORTHWEST);
						}
						else if (keyboard.keyChar == 255 + VK_NUMPAD1 && !keyboard.key[VK_SHIFT] && !keyboard.key[VK_CONTROL]) {
							game.PlayerMove(SOUTHWEST);
						}
						else if (keyboard.keyChar == 255 + VK_NUMPAD9 && !keyboard.key[VK_SHIFT] && keyboard.key[VK_CONTROL]) {
							game.PlayerTurn(NORTHEAST);
						}
						else if (keyboard.keyChar == 255 + VK_NUMPAD3 && !keyboard.key[VK_SHIFT] && keyboard.key[VK_CONTROL]) {
							game.PlayerTurn(SOUTHEAST);
						}
						else if (keyboard.keyChar == 255 + VK_NUMPAD7 && !keyboard.key[VK_SHIFT] && keyboard.key[VK_CONTROL]) {
							game.PlayerTurn(NORTHWEST);
						}
						else if (keyboard.keyChar == 255 + VK_NUMPAD1 && !keyboard.key[VK_SHIFT] && keyboard.key[VK_CONTROL]) {
							game.PlayerTurn(SOUTHWEST);
						}
						else if (keyboard.key[VK_CONTROL] && keyboard.keyChar == 12) {
							Protocol* protocol = game.GetProtocol();
							if (protocol)
								protocol->SendLogout();
						}
						else if (keyboard.key[VK_CONTROL] && keyboard.keyChar == 7) {
							if (status.GetIcons() & ICON_SWORDS) {
								Protocol* protocol = game.GetProtocol();
								if (protocol)
									protocol->SendLogout();
							}
							else
								game.onMakeAction(&wnds, NULL, ACTION_OPENWINDOW, (void*)(unsigned int)WND_CHARACTERSLIST);
						}
						else if (keyboard.key[VK_CONTROL] && keyboard.keyChar == 32) {
							Game::options.printOneLevel = !Game::options.printOneLevel;
							map.CalculateZ(player.GetPosition());
							game.SaveOptions();
						}
						else if (!guiManager->CheckGUIKey(&keyboard) && !keyboard.capsLock && keyboard.keyChar != 0) {
							LOCKCLASS lockClass(game.lockGame);
							if (game.GetWindowConsole()) {
								WindowElementTextarea* cmdLine = game.GetCmdLine();
								textarea = NULL;
								WindowElementTab* consoleTab = game.GetConsoleTab();
								if (consoleTab) {
									TabElement tab = consoleTab->GetActiveTab();
									if (tab.second)
										textarea = tab.second->Find<WindowElementTextarea>();
								}
								if (textarea && (textarea->IsSelection() || !cmdLine->IsActive()))
									textarea->CheckInput(keyboard);

								if (cmdLine && keyboard.keyChar != 0) {
									cmdLine->SetActive(true);
									cmdLine->CheckInput(keyboard);
								}
							}
						}
					}
				}
			}

			/* Send message to WindowProcedure */
			DispatchMessage(&messages);
		}

		wnds.CheckAnimations(mouse, keyboard, RT);
		wnds.CheckHolder(mouse, keyboard, RT);

		AD2D_Viewport vp = gfx.GetCurrentViewport();
		if (mainVP != vp)
			mainVP = vp;

		POINT size = gfx.GetWindowSize();

		if (!gfx.GetMinimized()) {
			gfx.ClearBuffers();

			//RenderToTexture
			if (Game::options.renderToTexture)
				map.RenderMap(&gfx, &player);

			//RenderToScreen
			if (game.GetGameState() < GAME_LOGGEDTOGAME) {
				if (backgroundImage.GetWidth() > 256)
					gfx.PutImage(0, 0, size.x, size.y, backgroundImage);
				else {
					for (int x = 0; x <= size.x / backgroundImage.GetWidth(); x++)
					for (int y = 0; y <= size.y / backgroundImage.GetHeight(); y++)
						gfx.PutImage(x * backgroundImage.GetWidth(), y * backgroundImage.GetHeight(), backgroundImage);
				}
			}
			else if (game.GetGameState() >= GAME_LOGGEDTOGAME) {
				AD2D_Image* bgImage = wnds.GetWindowTemplateGroupBackground(Game::options.templatesGroup);
				if (bgImage) {
					if (bgImage->GetWidth() > 256)
						gfx.PutImage(0, 0, size.x, size.y, *bgImage);
					else {
						for (int x = 0; x <= size.x / bgImage->GetWidth(); x++)
						for (int y = 0; y <= size.y / bgImage->GetHeight(); y++)
							gfx.PutImage(x * bgImage->GetWidth(), y * bgImage->GetHeight(), *bgImage);
					}
				}
			}

			wnds.Print();

			gfx.SetViewport(mainVP);
			HOLDER holder = mouse.GetHolder();
			if (holder.type == 0x01) {
				HCURSOR current = GetCursor();
				if (current != idc_move)
					SetCursor(idc_move);
			}
			else if (holder.type == 0x02 || holder.type == 0x03) {
				HCURSOR current = GetCursor();
				if (holder.type == 0x02 && holder.posX != 0 && holder.posY != 0) {
					if (current != idc_resize1)
						SetCursor(idc_resize1);
				}
				else if (holder.type == 0x03 && holder.posX != 0 && holder.posY != 0) {
					if (current != idc_resize2)
						SetCursor(idc_resize2);
				}
				else if (holder.posX == 0 && holder.posY != 0) {
					if (current != idc_resize_vert)
						SetCursor(idc_resize_vert);
				}
				else if (holder.posX != 0 && holder.posY == 0) {
					if (current != idc_resize_hori)
						SetCursor(idc_resize_hori);
				}
			}
			else if (holder.type == 0x50 || holder.type == 0x51 || holder.type == 0x52 || holder.type == 0x53 || holder.type == 0x80) {
				HCURSOR current = GetCursor();
				if (current != idc_cross)
					SetCursor(idc_cross);
			}
			else {
				HCURSOR current = GetCursor();
				if (mouse.cursor == 0) {
					if (current != idc_arrow)
						SetCursor(idc_arrow);
				}
				else if (mouse.cursor == 1) {
					if (current != idc_resize1)
						SetCursor(idc_resize1);
				}
				else if (mouse.cursor == 2) {
					if (current != idc_resize_vert)
						SetCursor(idc_resize_vert);
				}
				else if (mouse.cursor == 3) {
					if (current != idc_resize_hori)
						SetCursor(idc_resize_hori);
				}
				else if (mouse.cursor == 4) {
					if (current != idc_resize2)
						SetCursor(idc_resize2);
				}
			}

			if (holder.type == 0x50 || holder.type == 0x51 || holder.type == 0x52) {
				Item* item = dynamic_cast<Item*>((Thing*)holder.variable);
				Creature* creature = dynamic_cast<Creature*>((Thing*)holder.variable);
				if (item) {
					float period = (float)(RealTime::getTime() % 1000) / 1000;
					float offset = 4.0f * sin(2 * 3.141592654 * period);

					if (holder.type == 0x50) AD2D_Window::SetColor(1.0f, 1.0f, 1.0f, 0.6f);
					else if (holder.type == 0x51) AD2D_Window::SetColor(1.0f, 0.3f, 0.3f, 0.6f);
					else if (holder.type == 0x52) AD2D_Window::SetColor(0.3f, 0.3f, 1.0f, 0.8f);

					item->PrintItem(&gfx, Position(0, 0, 0), (float)(mouse.curX - 16) - offset, (float)(mouse.curY - 16) - offset, 32.0f + offset * 2, 32.0f + offset * 2, 0);
					AD2D_Window::SetColor(1.0f, 1.0f, 1.0f, 1.0f);
				}
				else if (creature) {
					float period = (float)(RealTime::getTime() % 1000) / 1000;
					float offset = 4.0f * sin(2 * 3.141592654 * period);

					AD2D_Window::SetColor(1.0f, 1.0f, 1.0f, 0.4f);
					creature->PrintCreature(&gfx, Position(0, 0, 0), (float)(mouse.curX - 16) - offset, (float)(mouse.curY - 16) - offset, 32.0f + offset * 2, 32.0f + offset * 2);
					AD2D_Window::SetColor(1.0f, 1.0f, 1.0f, 1.0f);
				}
			}
			else if (holder.type == 0x53) {
				HotKey* hk = (HotKey*)holder.variable;
				if (hk && hk->itemID != 0) {
					Item item;
					item.SetID(hk->itemID);
					item.SetCount(0);

					float period = (float)(RealTime::getTime() % 1000) / 1000;
					float offset = 4.0f * sin(2 * 3.141592654 * period);

					AD2D_Window::SetColor(1.0f, 1.0f, 1.0f, 0.6f);
					item.PrintItem(&gfx, Position(0, 0, 0), (float)(mouse.curX - 16) - offset, (float)(mouse.curY - 16) - offset, 32.0f + offset * 2, 32.0f + offset * 2, 0);
					AD2D_Window::SetColor(1.0f, 1.0f, 1.0f, 1.0f);
				}
				else if (hk && hk->spellID) {
					AD2D_Image* image = Icons::GetSpellIcon(hk->spellID);

					if (image) {
						float period = (float)(RealTime::getTime() % 1000) / 1000;
						float offset = 4.0f * sin(2 * 3.141592654 * period);

						AD2D_Window::SetColor(1.0f, 1.0f, 1.0f, 0.6f);
						gfx.PutImage((float)(mouse.curX - 16) - offset, (float)(mouse.curY - 16) - offset, (float)(mouse.curX + 16) + offset, (float)(mouse.curY + 16) + offset, *image);
						AD2D_Window::SetColor(1.0f, 1.0f, 1.0f, 1.0f);
					}
				}

				if (hk->text != "") {
					int width = font.GetTextWidth(hk->text, 14);
					int x = mouse.curX - width / 2;
					int y = mouse.curY - 14;

					AD2D_Window::SetColor(1.0f, 1.0f, 1.0f, 1.0f);
					gfx.Print((float)x, (float)y, 14, font, hk->text.c_str(), true);
				}
			}
			else if (holder.type == 0x70 || holder.type == 0x71) {
				float period = (float)(RealTime::getTime() % 1000) / 1000;
				float offset = 1.0f * sin(2 * 3.141592654 * period);

				AD2D_Image* image = NULL;
				if (holder.type == 0x70)
					image = Icons::GetMinimapIcon(holder.posX);
				else
					image = Icons::GetWaypointIcon(holder.posX);

				if (image) {
					AD2D_Window::SetColor(1.0f, 1.0f, 1.0f, 0.6f);
					gfx.PutImage((float)(mouse.curX - image->GetWidth() / 2) - offset, (float)(mouse.curY - image->GetHeight() / 2) - offset, (float)(mouse.curX + image->GetWidth() / 2) + offset, (float)(mouse.curY + image->GetHeight() / 2) + offset, *image);
					AD2D_Window::SetColor(1.0f, 1.0f, 1.0f, 1.0f);
				}
			}
			else if (holder.type == 0x00 && mouse.comment != "") {
				DividedText divText = DivideText(TextString(mouse.comment, 0));
				int gWidth = 0;
				int gHeight = divText.size() * 14;
				for (int i = 0; i < divText.size(); i++) {
					std::string msg = divText[i].second.text;

					int width = font.GetTextWidth(msg, 14);
					if (gWidth < width)
						gWidth = width;
				}
				int pX = mouse.curX - gWidth / 2;
				int pY = mouse.curY - gHeight;
				if (pX < 0) pX = 0;
				else if (pX > size.x - gWidth) pX = size.x - gWidth;
				if (pY < 0) pY = 0;
				else if (pY > size.y - gHeight) pY = size.y - gHeight;

				for (int i = 0; i < divText.size(); i++) {
					std::string msg = divText[i].second.text;

					int width = font.GetTextWidth(msg, 14);
					int x = pX + (gWidth - width) / 2;
					int y = pY + (14 * i);

					AD2D_Window::SetColor(1.0f, 1.0f, 1.0f, 1.0f);
					gfx.Print((float)x, (float)y, 14, font, msg.c_str(), true);
				}
			}

			gfx.PPrint(0, 0, 12, font, "FPS: %0.3f", true, retFPS);
			gfx.Print(size.x - 80, size.y - 12, 12, font, appReview.c_str(), true);

			gfx.SwapBuffers();
		}

		if (Game::releaseSprites) {
			Sprites::ReleaseSprites();

			Game::releaseSprites = false;
		}

		if (Game::screenshot) {
			AD2D_Image* logo = Icons::GetLogoIcon();
			gfx.PutImage(size.x - logo->GetWidth(), size.y - logo->GetHeight() - 12, *logo);

			time_lt milliSec = RealTime::getTime() % 1000;
			std::string filename = "screens\\" + time2str(RealTime::getTime() / 1000, true, true, true, true, true, true, "_") + "_" + value2str(milliSec) + ".jpg";
			gfx.ScreenShot(filename.c_str());

			Channel* ch = Channel::GetChannel(CHANNEL_SERVER_LOG);
			if (ch) {
				time_t now = RealTime::getTime() / 1000;
				std::string message = time2str(now, false, false, false, true, true, false) + " " + Text::GetText("SCREENSAVED", Game::options.language) + " (" + filename + ").";
				ch->AddMessage(now, "", TextString(message, 215));
			}

			Game::screenshot = false;
		}

		game.CheckCreatures(RT.getFactor());
		game.CheckMovingItems(RT.getFactor());
		game.CheckPlayer();

		RT.Calculate();
		fps.Insert(RT.getFPS());
		float rtFPS = RT.getFPS();
		retFPS = fps.Return();
		if (rtFPS < Game::options.limitFPS) PERIOD--;
		else if (rtFPS > Game::options.limitFPS) PERIOD++;
		if (PERIOD < 0) PERIOD = 0;
		else if (PERIOD > 200) PERIOD = 200;

		float period = PERIOD;
		Sleep((int)floor(period));
	}

	bool playing = (game.GetGameState() == GAME_LOGGEDTOGAME ? true : false);

	if (playing) {
		game.SaveChannels();
		game.SaveHotKeys();
		game.SaveMarkers();
		game.SaveWaypoints();

		bot.SaveBot(Game::filesLocation + "/bot/");
		bot.Stop();
	}

	/*if (!Updater::completed) {
		servers.SaveFavorites();
		game.SaveOptions();
	}*/
	wnds.SaveGUIWindows(playing);

	game.SetGameState(GAME_IDLE);
	wnds.CloseWindows(WND_ALL, true);
	wnds.ReleaseTemplates();

#ifndef NO_SOUND
	game.SetSFX(NULL);
#endif
	game.SetParticles(NULL);
	game.SetIcons(NULL);
	game.SetWindows(NULL);
	game.SetMap(NULL);
	game.SetPlayer(NULL);
	game.SetVIPList(NULL);
	game.SetShop(NULL);
	game.SetTrade(NULL);
	game.SetCraftBox(NULL);
	game.SetCooldowns(NULL);
	game.SetQuestLog(NULL);
	game.SetStatus(NULL);
	game.SetUpdater(NULL);
	game.SetMouse(NULL);
	game.SetKeyboard(NULL);

	Protocol* protocol = game.GetProtocol();
	if (protocol)
		delete_debug(protocol, M_PLACE);
	game.SetProtocol(NULL);

	map.SetMiniMap(NULL);
	map.SetBattle(NULL);

	Sprites::ReleaseSpritesData();
	Sprites::ReleaseSprites();

	LuaScript::ClearLuaScriptStates();
	LuaScript::ClearFiles();

	Logger::CloseSession();

	if (Updater::running)
		Updater::running = false;

	if (Updater::completed)
		Game::PostExecuteFile("patcher\\patcher.exe", "");

	Sleep(250);

    /* The program return-value is 0 - The value that PostQuitMessage() gave */
    return messages.wParam;
}


/*  This function is called by the Windows function DispatchMessage()  */

LRESULT CALLBACK WindowProcedure (HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)                  /* handle the messages */
    {
		case WM_ACTIVATE: {
			Game* game = Game::game;
			if (game) {
				Keyboard* keyboard = game->GetKeyboard();
				if (GetKeyState(VK_CAPITAL) & 0x0001)
					keyboard->capsLock = true;
				else
					keyboard->capsLock = false;

				keyboard->ResetKeyboardStatus();
			}

			break;
		}
        case WM_CLOSE: {
			Windows* wnds = Windows::wnds;
			Game* game = Game::game;
			if (wnds && game && game->GetGameState() == GAME_LOGGEDTOGAME) {
				wnds->CloseWindows(WND_CLOSE);
				wnds->OpenWindow(WND_CLOSE, game);
				break;
			}

			DestroyWindow(hwnd);
            break;
		}
		case WM_DESTROY: {
			PostQuitMessage(0);
			break;
		}
        case WM_MOVE: {
			RECT rc;
			GetWindowRect(hwnd, &rc);

			AD2D_Window* ad2d_wnd = AD2D_Window::GetPointer();
			if (ad2d_wnd)
				ad2d_wnd->MoveWindow(rc.left, rc.top);
			break;
		}
        case WM_SIZE: {
			AD2D_Window* ad2d_wnd = AD2D_Window::GetPointer();
			if (wParam != 1) {
				POINT oldWndSize = {0, 0};
				POINT newWndSize = {LOWORD(lParam), HIWORD(lParam)};

				if (newWndSize.x < 1) newWndSize.x = 1;
				if (newWndSize.y < 1) newWndSize.y = 1;

				if (ad2d_wnd) {
					oldWndSize = ad2d_wnd->GetWindowSize();
					ad2d_wnd->ResizeWindow(newWndSize.x, newWndSize.y);

					Windows* wnds = Windows::wnds;
					if (wnds)
						wnds->UpdateWindows(oldWndSize);
				}
			}

			if (wParam != 1 && ad2d_wnd && ad2d_wnd->GetMinimized())
				ad2d_wnd->SetMinimized(false);

			if (wParam == 1 && ad2d_wnd && !ad2d_wnd->GetMinimized())
				ad2d_wnd->SetMinimized(true);

			if (wParam != 2 && ad2d_wnd && ad2d_wnd->GetMaximized())
				ad2d_wnd->SetMaximized(false);

			if (wParam == 2 && ad2d_wnd && !ad2d_wnd->GetMaximized())
				ad2d_wnd->SetMaximized(true);

			break;
		}
		case WM_GETMINMAXINFO: {
			MINMAXINFO* mmi = (MINMAXINFO*)lParam;
			mmi->ptMinTrackSize.x = 640;
			mmi->ptMinTrackSize.y = 480;
			break;
		}
		default:                      /* for messages that we don't deal with */
            return DefWindowProc (hwnd, message, wParam, lParam);
    }

    return 0;
}
