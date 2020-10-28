/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#ifndef __CHANNEL_H_
#define __CHANNEL_H_

#include <map>

#include "luascript.h"
#include "mthread.h"
#include "window.h"


enum Channels {
	CHANNEL_NPC = 0xFFFC,
	CHANNEL_SERVER_LOG = 0xFFFD,
	CHANNEL_DEFAULT = 0xFFFE,
	CHANNEL_PRIVATE = 0xFFFF,
};

typedef TextString ChatMessage;
typedef std::pair<std::string, ChatMessage> ChatCreatureMessage;
typedef std::pair<time_t, ChatCreatureMessage> ChatMessageType;
typedef std::map<unsigned int, ChatMessageType> ChatMessagesMap;
typedef std::list<std::string> ChatUsers;

class Channel {
private:
	unsigned short		id;
	std::string			name;

	void*				button;
	void*				textarea;
	void*				memo;

	ChatUsers			users;
	ChatUsers			invited;

	ChatMessagesMap		messages;

	static std::map<unsigned short, Channel*>	channels;
	static std::map<std::string, Channel*>		privates;
	static MUTEX								channelsLock;

public:
	Channel();
	Channel(unsigned short id, std::string name);
	~Channel();

	unsigned short GetID();
	void SetName(std::string name);
	std::string GetName();
	unsigned int GetSize();

	void SetButton(void* button);
	void* GetButton();
	void SetTextarea(void* textarea);
	void* GetTextarea();
	void SetMemo(void* memo);
	void* GetMemo();

	void AddMessage(time_t time, std::string creatureName, TextString text);
	ChatMessageType GetMessage(unsigned int number);
	void ClearMessages();

	void AddUser(std::string user);
	void AddInvited(std::string user);
	ChatUsers GetUsers();
	ChatUsers GetInvited();
	void ClearUsers();
	void ClearInvited();

	static void AddChannel(Channel* channel, bool isPrivate = false);
	static Channel* GetChannel(unsigned short channelID);
	static Channel* GetChannel(std::string channelName);
	static Channel* GetChannel(void* textarea, void* button = NULL, void* memo = NULL);
	static std::map<unsigned short, Channel*> GetChannels();
	static void RemoveChannel(unsigned short channelID);
	static void RemoveChannel(std::string channelName);

	static void ClearChannels();

	//Lua functions
	static int LuaOpenChannel(lua_State* L);
	static int LuaChannelAddMessage(lua_State* L);

	static void LuaRegisterFunctions(lua_State* L);
};

#endif //__CHANNEL_H_
