/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#ifndef __MESSAGES_H_
#define __MESSAGES_H_

#include "game.h"
#include "mthread.h"
#include "position.h"
#include "tools.h"

class Message {
public:
	TextString		text;
	int				liveTime;
	int				maxLiveTime;

public:
	Message(TextString txt, int ltime);
	virtual ~Message();
};


class SystemMessage : public Message {
public:
	float			posX;
	float			posY;

public:
	SystemMessage(TextString txt, int ltime, float pX, float pY);
	virtual ~SystemMessage();
};


class TextMessage : public Message {
public:
	Position		pos;
	bool			multiLevel;

public:
	TextMessage(TextString txt, int ltime, Position ps, bool mlvl = false);
	virtual ~TextMessage();
};


class AnimatedMessage : public Message {
public:
	Position		pos;
	float			offsetX;
	float			offsetY;

public:
	AnimatedMessage(TextString txt, int ltime, Position ps, float offX = 0, float offY = 0);
	virtual ~AnimatedMessage();
};


typedef std::list<Message*> MessagesList;

class Messages {
private:
	static MessagesList			messages;

	static int					threads;
	static MUTEX				lockMessages;

public:
	Messages();
	~Messages();

	static void AddMessage(Message* message);
	static void RemoveMessage(Message* message);
	static void ClearMessages();

	static void CheckMessages(Game* game);
	static void PrintMessages(AD2D_Window* gfx, AD2D_Font* font, Position pos, unsigned char level, float x, float y, float zoom);
};


#endif //__MESSAGES_H_
