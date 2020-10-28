/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#include "channel.h"

#include "allocator.h"
#include "game.h"
#include "logger.h"
#include "luascript.h"


// ---- Channel ---- //

std::map<unsigned short, Channel*>	Channel::channels;
std::map<std::string, Channel*>		Channel::privates;
MUTEX								Channel::channelsLock;


Channel::Channel() {
	this->id = 0;
	this->name = "";
	this->button = NULL;
	this->textarea = NULL;
	this->memo = NULL;
}

Channel::Channel(unsigned short id, std::string name) {
	this->id = id;
	this->name = name;
	this->button = NULL;
	this->textarea = NULL;
	this->memo = NULL;
}

Channel::~Channel() {
}

unsigned short Channel::GetID() {
	return id;
}

void Channel::SetName(std::string name) {
	this->name = name;
}

std::string Channel::GetName() {
	return name;
}

unsigned int Channel::GetSize() {
	return messages.size();
}

void Channel::SetButton(void* button) {
	this->button = button;
}

void* Channel::GetButton() {
	return button;
}

void Channel::SetTextarea(void* textarea) {
	this->textarea = textarea;
}

void* Channel::GetTextarea() {
	return textarea;
}

void Channel::SetMemo(void* memo) {
	this->memo = memo;
}

void* Channel::GetMemo() {
	return memo;
}


void Channel::AddMessage(time_t time, std::string creatureName, TextString text) {
	LOCKCLASS lockClass(channelsLock);

	ChatMessage message(text);
	ChatCreatureMessage cMessage(creatureName, message);
	ChatMessageType mType(time, cMessage);

	size_t lastPos = 0;
	size_t pos = 0;
	while((pos = text.text.find('\n', lastPos)) != std::string::npos) {
		text.text[pos] = 12;
		lastPos = pos;
	}

	if (textarea) {
		WindowElementButton* mButton = (WindowElementButton*)button;
		WindowElementTextarea* mTextarea = (WindowElementTextarea*)textarea;
		mTextarea->AddTextLine(text);
		if (!mButton->GetPressed()) {
			if (id == CHANNEL_PRIVATE)
				mButton->SetColor(boost::bind(RT_ConvertColorButton, RealTime::getTime(), 0.4f, 0.6f, 1.0f, 1.0f));
			else
				mButton->SetColor(boost::bind(RT_ConvertColorButton, RealTime::getTime(), 0.4f, 1.0f, 0.4f, 1.0f));
		}
	}

	unsigned int count = messages.size();
	messages[count + 1] = mType;
}

ChatMessageType Channel::GetMessage(unsigned int number) {
	LOCKCLASS lockClass(channelsLock);

	ChatMessagesMap::iterator it = messages.find(number);
	if (it != messages.end()) {
		ChatMessageType mType = it->second;
		return mType;
	}

	return ChatMessageType(0, ChatCreatureMessage("", ChatMessage()));
}

void Channel::ClearMessages() {
	LOCKCLASS lockClass(channelsLock);

	messages.clear();
}

void Channel::AddUser(std::string user) {
	LOCKCLASS lockClass(channelsLock);

	users.push_back(user);
}

void Channel::AddInvited(std::string user) {
	LOCKCLASS lockClass(channelsLock);

	invited.push_back(user);
}

ChatUsers Channel::GetUsers() {
	LOCKCLASS lockClass(channelsLock);

	return users;
}

ChatUsers Channel::GetInvited() {
	LOCKCLASS lockClass(channelsLock);

	return invited;
}

void Channel::ClearUsers() {
	LOCKCLASS lockClass(channelsLock);

	users.clear();
}

void Channel::ClearInvited() {
	LOCKCLASS lockClass(channelsLock);

	invited.clear();
}


void Channel::AddChannel(Channel* channel, bool isPrivate) {
	LOCKCLASS lockClass(channelsLock);

	if (!channel) {
		Logger::AddLog("Channel::AddChannel()", "Channel pointes NULL!", LOG_WARNING);
		return;
	}

	unsigned short channelID = channel->GetID();
	std::string channelName = channel->GetName();

	if (!isPrivate) {
		std::map<unsigned short, Channel*>::iterator it = channels.find(channelID);
		if (it == channels.end()) {
			channels[channelID] = channel;
		}
		else
			Logger::AddLog("Channel::AddChannel()", "Channel already exist!", LOG_WARNING);
	}
	else {
		std::map<std::string, Channel*>::iterator it = privates.find(channelName);
		if (it == privates.end()) {
			privates[channelName] = channel;
		}
		else
			Logger::AddLog("Channel::AddChannel()", "Channel already exist!", LOG_WARNING);
	}
}

Channel* Channel::GetChannel(unsigned short channelID) {
	LOCKCLASS lockClass(channelsLock);

	std::map<unsigned short, Channel*>::iterator it = channels.find(channelID);
	if (it != channels.end()) {
		Channel* channel = it->second;
		return channel;
	}

	return NULL;
}

Channel* Channel::GetChannel(std::string channelName) {
	LOCKCLASS lockClass(channelsLock);

	std::map<std::string, Channel*>::iterator it = privates.find(channelName);
	if (it != privates.end()) {
		Channel* channel = it->second;
		return channel;
	}

	return NULL;
}

Channel* Channel::GetChannel(void* textarea, void* button, void* memo) {
	LOCKCLASS lockClass(channelsLock);

	std::map<unsigned short, Channel*>::iterator cit = channels.begin();
	for (cit; cit != channels.end(); cit++) {
		Channel* channel = cit->second;
		if (channel && ((textarea && channel->textarea == textarea) || (button && channel->button == button)))
			return channel;
	}

	std::map<std::string, Channel*>::iterator pit = privates.begin();
	for (pit; pit != privates.end(); pit++) {
		Channel* channel = pit->second;
		if (channel && ((textarea && channel->textarea == textarea) || (button && channel->button == button) || (memo && channel->memo == memo)))
			return channel;
	}

	return NULL;
}

std::map<unsigned short, Channel*> Channel::GetChannels() {
	LOCKCLASS lockClass(channelsLock);

	return channels;
}

void Channel::RemoveChannel(unsigned short channelID) {
	LOCKCLASS lockClass(channelsLock);

	std::map<unsigned short, Channel*>::iterator it = channels.find(channelID);
	if (it != channels.end()) {
		Channel* channel = it->second;
		delete_debug(channel, M_PLACE);
		channels.erase(it);
	}
	else
		Logger::AddLog("Channel::RemoveChannel()", "Channel not exist!", LOG_WARNING);
}

void Channel::RemoveChannel(std::string channelName) {
	LOCKCLASS lockClass(channelsLock);

	std::map<std::string, Channel*>::iterator it = privates.find(channelName);
	if (it != privates.end()) {
		Channel* channel = it->second;
		delete_debug(channel, M_PLACE);
		privates.erase(it);
	}
	else
		Logger::AddLog("Channel::RemoveChannel()", "Channel not exist!", LOG_WARNING);
}

void Channel::ClearChannels() {
	LOCKCLASS lockClass(channelsLock);

	std::map<unsigned short, Channel*>::iterator cit = channels.begin();
	for (cit; cit != channels.end(); cit++) {
		Channel* channel = cit->second;
		delete_debug(channel, M_PLACE);
	}
	channels.clear();

	std::map<std::string, Channel*>::iterator pit = privates.begin();
	for (pit; pit != privates.end(); pit++) {
		Channel* channel = pit->second;
		delete_debug(channel, M_PLACE);
	}
	privates.clear();
}


//Lua funcitons

int Channel::LuaOpenChannel(lua_State* L) {
	int params = lua_gettop(L);
	bool closeable = true;
	if (params > 2)
		closeable = (bool)LuaScript::PopNumber(L);
	std::string channelName = LuaScript::PopString(L);
	unsigned short channelID = LuaScript::PopNumber(L);

	Game* game = Game::game;
	if (game) {
		ChatUsers empty;
		game->OpenChannel(channelID, channelName, empty, empty, closeable);
	}
	return 1;
}

int Channel::LuaChannelAddMessage(lua_State* L) {
	int params = lua_gettop(L);

	bool byName = false;
	if (lua_type(L, -params) == LUA_TSTRING)
		byName = true;

	std::string creatureName;
	if (params > 3)
		creatureName = LuaScript::PopString(L);

	std::string message = LuaScript::PopString(L);
	int color = LuaScript::PopNumber(L);

	Channel* ch = (byName ? Channel::GetChannel(LuaScript::PopString(L)) : Channel::GetChannel(LuaScript::PopNumber(L)));

	if (ch)
		ch->AddMessage(0, creatureName, TextString(message, color));

	return 1;
}

void Channel::LuaRegisterFunctions(lua_State* L) {
	lua_pushinteger(L, CHANNEL_NPC);
	lua_setglobal(L, "CHANNEL_NPC");
	lua_pushinteger(L, CHANNEL_SERVER_LOG);
	lua_setglobal(L, "CHANNEL_SERVER_LOG");
	lua_pushinteger(L, CHANNEL_DEFAULT);
	lua_setglobal(L, "CHANNEL_DEFAULT");
	lua_pushinteger(L, CHANNEL_PRIVATE);
	lua_setglobal(L, "CHANNEL_PRIVATE");

	//openChannel(channelID, channelName)
	lua_register(L, "openChannel", Channel::LuaOpenChannel);

	//channelAddMessage(channelID, color, message)
	//channelAddMessage(channelName, color, message)
	lua_register(L, "channelAddMessage", Channel::LuaChannelAddMessage);
}
