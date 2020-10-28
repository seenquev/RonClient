/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#include "updater.h"

#include "allocator.h"
#include "const.h"
#include "filemanager.h"
#include "game.h"
#include "iniloader.h"
#include "text.h"
#include "tools.h"
#include "window.h"


// ---- Updater ---- //

bool	Updater::running = false;
bool	Updater::completed = false;

MUTEX	Updater::lockUpdater;


Updater::Updater() {
	container = NULL;
}

Updater::~Updater() { }


void Updater::StartUpdating(std::string server, std::string location, std::string directory, bool checkVersion) {
	Game* game = Game::game;
	Windows* wnds = Windows::wnds;
	if (wnds && game)
		wnds->OpenWindow(WND_UPDATE, game);

	running = true;
	THREAD th(boost::bind(&Updater::UpdateLoop, this, server, location, directory, checkVersion));
	thread.swap(th);
}

void Updater::StopUpdating() {
	running = false;
}

void Updater::UpdateLoop(std::string server, std::string location, std::string directory, bool checkVersion) {
	INILoader iniLoader;
	iniLoader.OpenFile(directory + "/updater.ini");

	CreateDirectory(directory.c_str(), NULL);

	int version = atoi(iniLoader.GetValue("VERSION").c_str());

	int files = 0;
	while(running) {
		std::string filename = "update" + value2str(version + 1) + ".pak";
		bool exist = CheckFile(server, location, filename);
		if (exist) {
			unsigned long downloaded = 0;
			unsigned char progress = 0;

			UpdateContainer(filename, &downloaded, &progress);
			DownloadFile(server, location, directory, filename, &downloaded, &progress);

			if (running) {
				std::string fullpath = std::string(directory + "/" + filename).substr(2, std::string::npos);

				FileManager* fileManager = FileManager::fileManager;
				if (fileManager)
					fileManager->ExtractPAK(fullpath, directory);

				RemoveEx(fullpath.c_str());

				iniLoader.SetValue("VERSION", value2str(version + 1));
				iniLoader.SaveFile(directory + "/updater.ini");

				version++;
				files++;
			}
		}
		else
			break;
	}

	if (checkVersion && files) {
		completed = true;
		//Game::PostQuit();
		Game::running = false;
	}

	Windows* wnds = Windows::wnds;
	if (wnds)
		wnds->CloseWindows(WND_UPDATE);
}

void Updater::SignalAddFunction(boost::function<void()> f) {
	signal.PushFunction(f);
}

void Updater::SignalExecute() {
	if (signal.IsExecutable())
		signal.Execute();

	signal.Clear();
}

void Updater::SignalClear() {
	signal.Clear();
}


void Updater::IncCounter(std::string server) {
	LOCKCLASS lockClass(lockUpdater);

	if (httpClient.Connected())
		httpClient.doClose();

	Host gameHost;
	if (Game::game)
		gameHost = Game::game->GetHost();

	if (server == "" || gameHost.host == "")
		return;

	std::string url = server;
	std::string host = url.substr(0, url.find('/'));
	std::string port = "80";
	std::string username = url.substr(host.length(), std::string::npos);

	httpClient.Connect(host.c_str(), port.c_str());
	if (!httpClient.Connected())
		return;

	std::string hostPost = "host=" + gameHost.host;
	std::string message;
	message = "POST " + username + "/count.php HTTP/2.0\r\n";
	message += "Host: " + host + "\r\n";
	message += "Connection: Close\r\n";
	message += "Content-Type: application/x-www-form-urlencoded\r\n";
	message += "Content-Length: " + value2str(hostPost.length()) + "\r\n\r\n";
	message += hostPost + "\r\n\r\n";
	httpClient.syncWrite(message.c_str(), message.length(), false);

	char ch;

	unsigned int size;

	httpClient.doClose();
}

void Updater::SendErrorReport(std::string server, std::string player, std::string* titlePtr, std::string* descPtr) {
	LOCKCLASS lockClass(lockUpdater);

	if (!titlePtr || !descPtr) {
		Windows* wnds = Windows::wnds;
		if (wnds)
			wnds->OpenWindow(WND_MESSAGE, Text::GetText("REPORTERROR_FAILED", Game::options.language).c_str());
		return;
	}

	std::string fromPlayer;
	std::string title;
	std::string desc;

	for (int i = 0; i < player.length(); i++) {
		unsigned char ch = player[i];
		fromPlayer.append(std::string("%") + value2str(ch, true, 2));
	}

	for (int i = 0; i < titlePtr->length(); i++) {
		unsigned char ch = (*titlePtr)[i];
		title.append(std::string("%") + value2str(ch, true, 2));
	}

	for (int i = 0; i < descPtr->length(); i++) {
		unsigned char ch = (*descPtr)[i];
		desc.append(std::string("%") + value2str(ch, true, 2));
	}

	if (httpClient.Connected())
		httpClient.doClose();

	if (server == "") {
		Windows* wnds = Windows::wnds;
		if (wnds)
			wnds->OpenWindow(WND_MESSAGE, Text::GetText("REPORTERROR_FAILED", Game::options.language).c_str());
		return;
	}

	std::string url = server;
	std::string host = url.substr(0, url.find('/'));
	std::string port = "80";
	std::string username = url.substr(host.length(), std::string::npos);

	httpClient.Connect(host.c_str(), port.c_str());
	if (!httpClient.Connected()) {
		Windows* wnds = Windows::wnds;
		if (wnds)
			wnds->OpenWindow(WND_MESSAGE, Text::GetText("REPORTERROR_FAILED", Game::options.language).c_str());
		return;
	}

	std::string post = "player=" + player + "&title=" + title + "&desc=" + desc;
	std::string message;
	message = "POST " + username + "/reporter.php HTTP/2.0\r\n";
	message += "Host: " + host + "\r\n";
	message += "Connection: Close\r\n";
	message += "Content-Type: application/x-www-form-urlencoded\r\n";
	message += "Content-Length: " + value2str(post.length()) + "\r\n\r\n";
	message += post + "\r\n\r\n";
	httpClient.syncWrite(message.c_str(), message.length(), false);

	char ch;

	unsigned int size;

	httpClient.doClose();

	Windows* wnds = Windows::wnds;
	if (wnds)
		wnds->OpenWindow(WND_MESSAGE, Text::GetText("REPORTERROR_SENT", Game::options.language).c_str());
}

bool Updater::CheckFile(std::string server, std::string location, std::string filename) {
	LOCKCLASS lockClass(lockUpdater);

	if (httpClient.Connected())
		httpClient.doClose();

	if (server == "")
		return false;

	std::string url = server;
	std::string host = url.substr(0, url.find('/'));
	std::string port = "80";
	std::string username = url.substr(host.length(), std::string::npos);

	httpClient.Connect(host.c_str(), port.c_str());
	if (!httpClient.Connected())
		return false;

	std::string message;
	message = "GET " + username + "/" + location + "/" + filename + " HTTP/2.0\r\n";
	message += "Host: " + host + "\r\n";
	message += "Connection: Close\r\n\r\n";
	httpClient.syncWrite(message.c_str(), message.length(), false);

	char ch;

	size_t length = 0;
	size_t size = 0;

	message.clear();
	while((size = httpClient.syncRead(&ch, 1, false)) != 0) {
		message.push_back(ch);

		if (message.length() >= 4 && message.substr(message.length() - 4, 4) == "\r\n\r\n")
			break;
	}

	httpClient.doClose();

	for (std::string::iterator it = message.begin(); it != message.end(); it++) {
		char ch = *it;
		ch = toupper(ch);
		*it = ch;
	}

	if (message.find("200 OK", 0) == 9)
		return true;

	return false;
}

bool Updater::DownloadFile(std::string server, std::string location, std::string directory, std::string filename, unsigned long* downloaded, unsigned char* progress) {
	LOCKCLASS lockClass(lockUpdater);

	if (httpClient.Connected())
		httpClient.doClose();

	if (server == "")
		return false;

	std::string url = server;
	std::string host = url.substr(0, url.find('/'));
	std::string port = "80";
	std::string username = url.substr(host.length(), std::string::npos);

	httpClient.Connect(host.c_str(), port.c_str());
	if (!httpClient.Connected())
		return false;

	std::string message;
	message = "GET " + username + "/" + location + "/" + filename + " HTTP/2.0\r\n";
	message += "Host: " + host + "\r\n";
	message += "Request-Header: Content-Length\r\n";
	message += "Connection: Close\r\n\r\n";
	httpClient.syncWrite(message.c_str(), message.length(), false);

	char ch[1025];

	size_t length = 0;
	size_t size = 0;

	char last_ch;

	message.clear();
	memset(ch, 0, 1025);
	while((size = httpClient.syncRead(ch, 1, false)) != 0) {
		message += ch;
		memset(ch, 0, 2);

		if (message.length() >= 4 && message.substr(message.length() - 4, 4) == "\r\n\r\n")
			break;
	}

	for (std::string::iterator it = message.begin(); it != message.end(); it++) {
		char ch = *it;
		ch = toupper(ch);
		*it = ch;
	}

	size_t pos1;
	size_t pos2;
	if ((pos1 = message.find("CONTENT-LENGTH:", 0)) != std::string::npos) {
		pos2 = message.find("\r\n", pos1);
		length = atol(message.substr(pos1 + 15, pos2 - (pos1 + 15)).c_str());
	}

	size_t globalSize = 0;

	std::string fullfilename = directory + "/" + filename;
	FILE* file = fopen(fullfilename.c_str(), "wb");
	if (!file) {
		fullfilename += "_";
		file = fopen(fullfilename.c_str(), "wb");
		if (!file)
			return false;
	}

	memset(ch, 0, 1025);
	while(running && globalSize < length && (size = httpClient.syncRead(ch, 1024, false)) != 0) {
		fwrite(ch, size, 1, file);
		memset(ch, 0, 1025);

		globalSize += size;
		if (downloaded)
			*downloaded = globalSize;
		if (progress)
			*progress = (unsigned char)(100 * (float)globalSize / length);
	}

	fclose(file);
	if (!running)
		remove(fullfilename.c_str());

	httpClient.doClose();
}


void Updater::SetContainer(WindowElementContainer* container) {
	LOCKCLASS lockClass(lockUpdater);

	this->container = container;
}

void Updater::UpdateContainer(std::string filename, unsigned long* downloaded, unsigned char* progress) {
	LOCKCLASS lockClass1(Windows::lockWindows);
	LOCKCLASS lockClass2(lockUpdater);

	if (!container)
		return;

	container->DeleteAllElements();

	Window* window = container->GetWindow();
	WindowTemplate* wndTemplate = window->GetWindowTemplate();
	window->SetActiveElement(NULL);

	POINT size_ext = container->GetIntSize();

	WindowElementText* tx_file = new(M_PLACE) WindowElementText;
	tx_file->Create(0, 10, 0, size_ext.x - 20, wndTemplate);
	tx_file->SetText(filename + ":");
	tx_file->SetValuePtr(TypePointer("uint64", (void*)downloaded));

	WindowElementSkillBar* sb_progress = new(M_PLACE) WindowElementSkillBar;
	sb_progress->Create(0, 10, 16, size_ext.x - 20, 10, TypePointer("uint8", (void*)progress), wndTemplate);
	sb_progress->SetColor(0.5f, 1.0f, 0.5f);

	container->AddElement(tx_file);
	container->AddElement(sb_progress);
}
