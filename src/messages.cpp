/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#include "messages.h"

#include "allocator.h"
#include "game.h"
#include "tools.h"


// ---- Message ---- //

Message::Message(TextString txt, int ltime) : text(txt), maxLiveTime(ltime) {
	liveTime = 0;
}

Message::~Message() { }


// ---- SystemMessage ---- //

SystemMessage::SystemMessage(TextString txt, int ltime, float pX, float pY) : Message(txt, ltime), posX(pX), posY(pY) { }

SystemMessage::~SystemMessage() { }


// ---- TextMessage ---- //

TextMessage::TextMessage(TextString txt, int ltime, Position ps, bool mlvl) : Message(txt, ltime), pos(ps), multiLevel(mlvl) { }

TextMessage::~TextMessage() { }


// ---- AnimatedMessage ---- //

AnimatedMessage::AnimatedMessage(TextString txt, int ltime, Position ps, float offX, float offY) : Message(txt, ltime), pos(ps), offsetX(offX), offsetY(offY) { }

AnimatedMessage::~AnimatedMessage() { }


// ---- Messages ---- //

MessagesList Messages::messages;
int Messages::threads = 0;
MUTEX Messages::lockMessages;

Messages::Messages() { }

Messages::~Messages() { }

void Messages::AddMessage(Message* message) {
	LOCKCLASS lockClass(lockMessages);

	if (!message) {
		Logger::AddLog("Messages::AddMessage()", "Pointer to message is NULL!", LOG_WARNING);
		return;
	}

	MessagesList::iterator it = std::find(messages.begin(), messages.end(), message);
	if (it != messages.end()) {
		Logger::AddLog("Messages::AddMessage()", "Message already in list!", LOG_WARNING);
		return;
	}
	else {
		SystemMessage* msg = dynamic_cast<SystemMessage*>(message);
		if (msg) {
			MessagesList toDelete;

			int number = 0;
			for (MessagesList::reverse_iterator rit = messages.rbegin(); rit != messages.rend(); rit++) {
				SystemMessage* smsg = dynamic_cast<SystemMessage*>(*rit);
				if (smsg && msg->posX == smsg->posX && msg->posY == smsg->posY) {
					if (number < 1)
						number++;
					else
						toDelete.push_back((Message*)smsg);
				}
			}

			for (it = toDelete.begin(); it != toDelete.end(); it++)
				RemoveMessage(*it);
		}

		messages.push_back(message);
	}
}

void Messages::RemoveMessage(Message* message) {
	LOCKCLASS lockClass(lockMessages);

	if (!message) {
		Logger::AddLog("Messages::RemoveMessage()", "Pointer to message is NULL!", LOG_WARNING);
		return;
	}

	MessagesList::iterator it = std::find(messages.begin(), messages.end(), message);
	if (it != messages.end()) {
		messages.erase(it);
		delete_debug(message, M_PLACE);
	}
	else
		Logger::AddLog("Messages::RemoveMessage()", "Message not in list!", LOG_WARNING);
}

void Messages::ClearMessages() {
	LOCKCLASS lockClass(lockMessages);

	MessagesList::iterator it = messages.begin();
	for (it; it != messages.end(); it++) {
		Message* message = *it;
		delete_debug(message, M_PLACE);
	}

	messages.clear();
}

void Messages::CheckMessages(Game* game) {
	if (threads > 0)
		return;

	threads++;

	while(game->GetGameState() == GAME_LOGGEDTOGAME) {
		lockMessages.lock();

		MessagesList toDelete;

		MessagesList::iterator it = messages.begin();
		for (it; it != messages.end(); it++) {
			Message* message = *it;
			if (message && message->liveTime + 10 <= message->maxLiveTime)
				message->liveTime += 10;
			else
				toDelete.push_back(message);
		}

		for (it = toDelete.begin(); it != toDelete.end(); it++) {
			Message* message = *it;
			RemoveMessage(message);
		}

		lockMessages.unlock();
		Sleep(10);
	}

	threads--;
}

void Messages::PrintMessages(AD2D_Window* gfx, AD2D_Font* font, Position pos, unsigned char level, float x, float y, float zoom) {
	LOCKCLASS lockClass(lockMessages);

	float fontSize = Game::options.textSize;

	std::map<const Position, float> textOffset;
	std::map<std::pair<float, float>, int> sysLine;
	std::map<std::pair<float, float>, int> sysLines;

	MessagesList textMessages;
	MessagesList animMessages;
	MessagesList sysMessages;

	for (MessagesList::iterator it = messages.begin(); it != messages.end(); it++) {
		Message* message = *it;

		TextMessage* txtMsg = dynamic_cast<TextMessage*>(message);
		AnimatedMessage* animMsg = dynamic_cast<AnimatedMessage*>(message);
		SystemMessage* sysMsg = dynamic_cast<SystemMessage*>(message);

		if (txtMsg) {
			if (Game::options.flyingSpeed != 0)
				textMessages.push_back(txtMsg);
			else
				textMessages.push_front(txtMsg);
		}
		else if (animMsg)
			animMessages.push_back(animMsg);
		else if (sysMsg) {
			std::vector<std::pair<unsigned int, TextString> > lines = DivideText(sysMsg->text);
			sysLines[std::pair<float, float>(sysMsg->posX, sysMsg->posY)] += lines.size();

			sysMessages.push_front(sysMsg);
		}
	}

	MessagesList::iterator it;
	for (it = textMessages.begin(); it != textMessages.end(); it++) {
		Message* message = *it;

		TextMessage* txtMsg = dynamic_cast<TextMessage*>(message);

		if (txtMsg && (txtMsg->pos.z == level || txtMsg->multiLevel)) {
			int blender = (txtMsg->maxLiveTime - txtMsg->liveTime > 2000 ? 0 : 2000 - (txtMsg->maxLiveTime - txtMsg->liveTime));
			float alpha = 1.0f - ((float)blender / 2000);
			float flyingSpeed = (float)Game::options.flyingSpeed * 0.01f;

			std::vector<std::pair<unsigned int, TextString> > lines = DivideText(txtMsg->text);
			for (int i = 0; i < lines.size(); i++) {
				std::string printText = lines[i].second.text;
				std::string printColor = lines[i].second.color;

				float width = font->GetTextWidth(printText, fontSize);
				float offset = fontSize * (lines.size() - i - 1);
				float px = x + (txtMsg->pos.x - pos.x) * zoom + zoom / 2 - width / 2;
				float py = y + (txtMsg->pos.y - pos.y) * zoom - zoom - offset - (float)(flyingSpeed * (zoom / 32) * txtMsg->liveTime) / 150 - textOffset[txtMsg->pos];
				if (px < 0.0f) px = 0.0f;
				else if (px > (15 * zoom) - width) px = (15 * zoom) - width;
				if (py < 0.0f + i * fontSize) py = 0.0f + i * fontSize;
				else if (py > (11 * zoom) - (lines.size() - i) * fontSize) py = (11 * zoom) - (lines.size() - i) * fontSize;

				AD2D_Window::SetColor(1.0f, 1.0f, 1.0f, alpha);
				gfx->CPrint(px, py, fontSize, *font, printText.c_str(), printColor.c_str(), true);
			}

			if (Game::options.flyingSpeed == 0)
				textOffset[txtMsg->pos] = textOffset[txtMsg->pos] + lines.size() * fontSize;
		}
	}

	for (it = animMessages.begin(); it != animMessages.end(); it++) {
		Message* message = *it;

		AnimatedMessage* animMsg = dynamic_cast<AnimatedMessage*>(message);

		if (animMsg && animMsg->pos.z == level) {
			std::string printText = animMsg->text.text;
			std::string printColor = animMsg->text.color;

			float _fontSize = fontSize * (1.0f + (float)animMsg->liveTime / animMsg->maxLiveTime);
			float px = x + (float)(animMsg->pos.x - pos.x) * zoom + zoom / 2 - font->GetTextWidth(printText, _fontSize) / 2 + animMsg->offsetX * _fontSize;
			float py = y + (float)(animMsg->pos.y - pos.y) * zoom - (float)((zoom / 32) * animMsg->liveTime) / 50 + animMsg->offsetY * _fontSize;

			float alpha = 1.0f - ((float)animMsg->liveTime / animMsg->maxLiveTime);

			AD2D_Window::SetColor(1.0f, 1.0f, 1.0f, alpha);
			gfx->CPrint(px, py, _fontSize, *font, printText.c_str(), printColor.c_str(), true);
		}
	}

	for (it = sysMessages.begin(); it != sysMessages.end(); it++) {
		Message* message = *it;

		SystemMessage* sysMsg = dynamic_cast<SystemMessage*>(message);

		if (sysMsg) {
			int blender = (sysMsg->maxLiveTime - sysMsg->liveTime > 2000 ? 0 : 2000 - (sysMsg->maxLiveTime - sysMsg->liveTime));
			float alpha = 1.0f - ((float)blender / 2000);

			std::vector<std::pair<unsigned int, TextString> > lines = DivideText(sysMsg->text);
			for (int i = 0; i < lines.size(); i++) {
				std::string printText = lines[i].second.text;
				std::string printColor = lines[i].second.color;

				float offset = fontSize * sysLine[std::pair<float, float>(sysMsg->posX, sysMsg->posY)];
				float px = ((sysMsg->posX + 1.0f) / 2) * (15 * zoom) - ((sysMsg->posX + 1.0f) / 2) * font->GetTextWidth(printText, fontSize);
				float py = ((sysMsg->posY + 1.0f) / 2) * (11 * zoom) - ((sysMsg->posY + 1.0f) / 2) * (float)(fontSize * (sysLines[std::pair<float, float>(sysMsg->posX, sysMsg->posY)])) + offset;

				AD2D_Window::SetColor(1.0f, 1.0f, 1.0f, alpha);
				gfx->CPrint(px, py, fontSize, *font, printText.c_str(), printColor.c_str(), true);
				sysLine[std::pair<float, float>(sysMsg->posX, sysMsg->posY)]++;
			}
		}
	}
}
