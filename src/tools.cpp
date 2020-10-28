/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#include "tools.h"

#include <dirent.h>
#include <sstream>

#include "allocator.h"
#include "filemanager.h"
#include "realtime.h"


void TypePointer::SetValue(std::string data) {
	if (type == "float")
		*(double*)ptr = atof(data.c_str());
	else if (type == "uint8")
		*(unsigned char*)ptr = atoi(data.c_str());
	else if (type == "uint16")
		*(unsigned short*)ptr = atoi(data.c_str());
	else if (type == "uint32")
		*(unsigned int*)ptr = atol(data.c_str());
	else if (type == "uint64")
		*(unsigned long*)ptr = atol(data.c_str());
	else if (type == "string")
		*(std::string*)ptr = data;
}

std::string TypePointer::GetValue(int precision) {
	if (type == "float")
		return float2str(*(double*)ptr, precision);
	else if (type == "uint8")
		return value2str(*(unsigned char*)ptr);
	else if (type == "uint16")
		return value2str(*(unsigned short*)ptr);
	else if (type == "uint32")
		return value2str(*(unsigned int*)ptr);
	else if (type == "uint64")
		return value2str(*(unsigned long*)ptr);
	else if (type == "string")
		return *(std::string*)ptr;
	else
		return "";
}


POINT doPOINT(int x, int y) {
	POINT p = {x, y};
	return p;
}

bool operator==(POINT a, POINT b) {
	if (a.x == b.x && a.y == b.y)
		return true;

	return false;
}

bool operator!=(POINT a, POINT b) {
	return !(a == b);
}

bool operator<(POINT a, POINT b) {
	if (a.y < b.y)
		return true;
	else if (a.y == b.y && a.x < b.x)
		return true;

	return false;
}

bool operator<=(POINT a, POINT b) {
	return (a < b) || (a == b);
}

bool operator>(POINT a, POINT b) {
	if (a.y > b.y)
		return true;
	else if (a.y == b.y && a.x > b.x)
		return true;

	return false;
}

bool operator>=(POINT a, POINT b) {
	return (a > b) || (a == b);
}

bool IsSpecialChar(unsigned char ch) {
	if ((ch >= 48 and ch <= 57) || (ch >= 65 and ch <= 90) || (ch >= 97 and ch <= 122) || ch >= 128)
		return false;

	return true;
}

std::string ConvertIP(unsigned int ip) {
	unsigned char* c_ip;
	c_ip = (unsigned char*)&ip;

	char s_ip[16];
	sprintf(s_ip, "%d.%d.%d.%d", c_ip[0], c_ip[1], c_ip[2], c_ip[3]);

	return s_ip;
}

TextString ScrollText(TextString txt, AD2D_Font* font, unsigned short fontSize, int length, int offset) {
	std::string text = txt.text;
	std::string color = txt.color;

	int cur = 0;
	int space = -1;
	int ncursor = 0;
	int lines = 0;
	float pix = 0.0f;
	float spacePix = 0.0f;
	float lastSpacePix = 0.0f;
	while(cur < text.length()) {
		unsigned char c = text[cur];

		if (c == 32) {
			space = cur;
			spacePix = pix;
		}
		else if (c == '\n') {
			space = cur;
			lines = 0;
			spacePix = pix;
			lastSpacePix = pix;
		}
		else if (c == 12) {
			space = cur;
			spacePix = pix;
			lastSpacePix = pix;
			lines++;
		}

		int scrollLength = (lines > 0 ? length - offset : length);
		if (pix + font->GetCharWidth(c, fontSize) - lastSpacePix >= (float)scrollLength) {
			if (pix + font->GetCharWidth(c, fontSize) - spacePix < (float)scrollLength) {
				text[space] = 12;
				lastSpacePix = spacePix;
				lines++;
			}
			else {
				text.insert(cur, 1, 13);
				color.insert(cur, 1, 0);
				lastSpacePix = pix;
				spacePix = pix;
				space = cur;
				lines++;
				cur++;
			}
		}
		cur++;
		pix += font->GetCharWidth(c, fontSize);
	}

	return TextString(text, color);
}

TextString ScrollText(TextString txt, int length, int offset) {
	std::string text = txt.text;
	std::string color = txt.color;

	int cur = 0;
	int lastSpace = -1;
	int space = -1;
	int ncursor = 0;
	int lines = 0;
	while(cur < text.length()) {
		unsigned char c = text[cur];

		if (c == 32)
			space = cur;
		else if (c == '\n') {
			lastSpace = cur;
			space = cur;
			lines = 0;
		}

		int scrollLength = (lines > 0 ? length - offset : length);
		if (cur - lastSpace >= (float)scrollLength) {
			if (cur - space < scrollLength) {
				text[space] = 12;
				lastSpace = space;
				lines++;
			}
			else {
				text.insert(cur, 1, 13);
				color.insert(cur, 1, 0);
				lastSpace = cur;
				space = cur;
				lines++;
				cur++;
			}
		}
		cur++;
	}

	return TextString(text, color);
}

std::vector<std::pair<unsigned int, TextString> > DivideText(TextString txt) {
	std::vector<std::pair<unsigned int, TextString> > vec;
	size_t lastPos = 0;
	size_t pos0 = txt.text.find('\n',lastPos);
	size_t pos1 = txt.text.find(12,lastPos);
	size_t pos2 = txt.text.find(13,lastPos);

	unsigned int line = 0;
	while(lastPos < txt.text.length() && (pos0 != std::string::npos || pos1 != std::string::npos || pos2 != std::string::npos)) {
		//size_t pos = (pos1 != std::string::npos && pos1 < pos2 ? pos1 : pos2);
		size_t pos = std::min(pos0, pos1);
		pos = std::min(pos, pos2);

		std::string str = txt.text.substr(lastPos, pos - lastPos);
		std::string col = txt.color.substr(lastPos, pos - lastPos);
		vec.push_back(std::pair<unsigned int, TextString>(line, TextString(str, col)));

		if (pos == pos0) line++;

		lastPos = pos + 1;
		pos0 = txt.text.find('\n',lastPos);
		pos1 = txt.text.find(12,lastPos);
		pos2 = txt.text.find(13,lastPos);
	}
	std::string str = txt.text.substr(lastPos, txt.text.length() - lastPos);
	std::string col = txt.color.substr(lastPos, txt.text.length() - lastPos);
	vec.push_back(std::pair<unsigned int, TextString>(line, TextString(str, col)));

	return vec;
}

TextString SetTextColorMap(std::string txt, unsigned char color1, unsigned char color2) {
	std::string text;
	std::string color;

	bool active = false;
	for (int i = 0; i < txt.length(); i++) {
		if (txt[i] == '{' && !active) {
			txt.erase((size_t)i, 1);
			active = true;
			i--;
		}
		else if (txt[i] == '}' && active) {
			txt.erase((size_t)i, 1);
			active = false;
			i--;
		}
		else {
			unsigned char c = (active ? color2 : color1);
			text.push_back(txt[i]);
			color.push_back(c);
		}
	}

	return TextString(text, color);
}

std::string value2str(int64_t value, bool hex, int chars) {
	std::stringstream temp;
	if (hex && chars == 0)
		temp << std::hex << value;
	else if (hex)
		temp << std::setfill('0') << std::setw(chars) << std::hex << value;
	else
		temp << std::dec << value;

	return temp.str();
}

std::string float2str(double value, int precision) {
	std::stringstream temp;
	if (precision != 0) {
		temp.precision(precision);
		temp << std::fixed << value;
	}
	else
		temp << value;

	return temp.str();
}

std::string time2str(time_t time, bool year, bool month, bool day, bool hour, bool minute, bool second, const char* separator) {
	std::stringstream temp;
	temp << std::setfill('0');

	struct tm *t_now;
	t_now = localtime(&time);

	int iday = t_now->tm_mday;
	int imonth = t_now->tm_mon+1;
	int iyear = t_now->tm_year+1900;
	int ihour = t_now->tm_hour;
	int iminute = t_now->tm_min;
	int isecond = t_now->tm_sec;

	if (year) temp << std::setw(4) << iyear;
	if (month) temp << (year ? "-" : "") << std::setw(2) << imonth;
	if (day) temp << (year || month ? "-" : "") << std::setw(2) << iday;

	if ((year || month || day) && (hour || minute || second)) temp << " ";

	if (hour) temp << std::setw(2) << ihour;
	if (minute) temp << (hour ? separator : "") << std::setw(2) << iminute;
	if (second) temp << (hour || minute ? separator : "") << std::setw(2) << isecond;

	return temp.str();
}

std::string tolower(std::string text) {
	std::string ret;
	for (int i = 0; i < text.length(); i++) {
		char ch = text[i];
		if (ch >= 65 && ch <= 90)
			ch += 32;
		ret[i] = ch;
	}
}

bool CheckChecksumFile(const char* path, unsigned long checksum) {
	FileManager* files = FileManager::fileManager;
	if (!files)
		return false;

	FILE* file = files->OpenFile(path);
	if (!file)
		return false;

	long int size = files->GetFileSize(path);
	if (size == 0)
		return false;

	unsigned char* data = new(M_PLACE) unsigned char[size];
	fread(data, 1, size, file);
	fclose(file);

	std::string str;
	for (long int i = 0; i < size; i++)
		str.push_back(data[i]);

	delete_debug_array(data, M_PLACE);

	return CheckChecksum(str, checksum);
}

bool CheckChecksum(std::string str, unsigned long checksum) {
	unsigned long _checksum = 0;
	int pos = 0;
	while(pos < str.length()) {
		char ch = str[pos];
		_checksum += (ch * pos);
		_checksum ^= 0xAB * pos;
		pos++;
	}

	if (checksum == 0) {
		MessageBox(NULL, value2str(_checksum).c_str(), "", MB_OK);
		return true;
	}

	if (_checksum != checksum)
		return false;

	return true;
}


uint32_t MakeChecksum(std::string str) {
	uint32_t checksum = 0;
	int pos = 0;
	while(pos < str.length()) {
		char ch = str[pos];
		checksum += (ch * pos);
		pos++;
	}

	return checksum;
}


unsigned char ConvertSpeakClasses(unsigned char speakClass, unsigned short version) {
	unsigned char ret = 0;
	switch((SpeakClasses)speakClass) {
		case SPEAK_SAY:
			if (version == 822) ret = SPEAK822_SAY;
			else if (version == 840 || version == 842 || version == 850 || version == 854 || version == 860) ret = SPEAK840_SAY;
			else if (version == 870) ret = SPEAK870_SAY;
			else if (version == 910) ret = SPEAK910_SAY;
			break;
		case SPEAK_WHISPER:
			if (version == 822) ret = SPEAK822_WHISPER;
			else if (version == 840 || version == 842 || version == 850 || version == 854 || version == 860) ret = SPEAK840_WHISPER;
			else if (version == 870) ret = SPEAK870_WHISPER;
			else if (version == 910) ret = SPEAK910_WHISPER;
			break;
		case SPEAK_YELL:
			if (version == 822) ret = SPEAK822_YELL;
			else if (version == 840 || version == 842 || version == 850 || version == 854 || version == 860) ret = SPEAK840_YELL;
			else if (version == 870) ret = SPEAK870_YELL;
			else if (version == 910) ret = SPEAK910_YELL;
			break;
		case SPEAK_PRIVATE_PN:
			if (version == 822) ret = SPEAK822_PRIVATE_PN;
			else if (version == 840 || version == 842 || version == 850 || version == 854 || version == 860) ret = SPEAK840_PRIVATE_PN;
			else if (version == 870) ret = SPEAK870_PRIVATE_PN;
			else if (version == 910) ret = SPEAK910_PRIVATE_PN;
			break;
		case SPEAK_PRIVATE_NP:
			if (version == 822) ret = SPEAK822_PRIVATE_NP;
			else if (version == 840 || version == 842 || version == 850 || version == 854 || version == 860) ret = SPEAK840_PRIVATE_NP;
			else if (version == 870) ret = SPEAK870_PRIVATE_NP;
			else if (version == 910) ret = SPEAK910_PRIVATE_NP;
			break;
		case SPEAK_PRIVATE:
			if (version == 822) ret = SPEAK822_PRIVATE;
			else if (version == 840 || version == 842 || version == 850 || version == 854 || version == 860) ret = SPEAK840_PRIVATE;
			else if (version == 870) ret = SPEAK870_PRIVATE;
			else if (version == 910) ret = SPEAK910_PRIVATE_TO;
			break;
		case SPEAK_CHANNEL_Y:
			if (version == 822) ret = SPEAK822_CHANNEL_Y;
			else if (version == 840 || version == 842 || version == 850 || version == 854 || version == 860) ret = SPEAK840_CHANNEL_Y;
			else if (version == 870) ret = SPEAK870_CHANNEL_Y;
			else if (version == 910) ret = SPEAK910_CHANNEL_Y;
			break;
		case SPEAK_CHANNEL_W:
			if (version == 822) ret = SPEAK822_CHANNEL_W;
			else if (version == 840 || version == 842 || version == 850 || version == 854 || version == 860) ret = SPEAK840_CHANNEL_W;
			else if (version == 870) ret = SPEAK870_CHANNEL_W;
			else if (version == 910) ret = SPEAK910_CHANNEL_Y;
			break;
		case SPEAK_BROADCAST:
			if (version == 822) ret = SPEAK822_BROADCAST;
			else if (version == 840 || version == 842 || version == 850 || version == 854 || version == 860) ret = SPEAK840_BROADCAST;
			else if (version == 870) ret = SPEAK870_BROADCAST;
			else if (version == 910) ret = SPEAK910_BROADCAST;
			break;
		case SPEAK_CHANNEL_R1:
			if (version == 822) ret = SPEAK822_CHANNEL_R1;
			else if (version == 840 || version == 842 || version == 850 || version == 854 || version == 860) ret = SPEAK840_CHANNEL_R1;
			else if (version == 870) ret = SPEAK870_CHANNEL_R1;
			else if (version == 910) ret = SPEAK910_CHANNEL_R1;
			break;
		case SPEAK_PRIVATE_RED:
			if (version == 822) ret = SPEAK822_PRIVATE_RED;
			else if (version == 840 || version == 842 || version == 850 || version == 854 || version == 860) ret = SPEAK840_PRIVATE_RED;
			else if (version == 870) ret = SPEAK870_PRIVATE_RED;
			else if (version == 910) ret = SPEAK910_PRIVATE_RED_TO;
			break;
		case SPEAK_CHANNEL_O:
			if (version == 822) ret = SPEAK822_CHANNEL_O;
			else if (version == 840 || version == 842 || version == 850 || version == 854 || version == 860) ret = SPEAK840_CHANNEL_O;
			else if (version == 870) ret = SPEAK870_CHANNEL_O;
			else if (version == 910) ret = SPEAK910_CHANNEL_O;
			break;
		case SPEAK_CHANNEL_R2:
			if (version == 822) ret = SPEAK822_CHANNEL_R2;
			else if (version == 840 || version == 842 || version == 850 || version == 854 || version == 860) ret = SPEAK840_CHANNEL_R2;
			else if (version == 870) ret = SPEAK870_CHANNEL_R1;
			else if (version == 910) ret = SPEAK910_CHANNEL_R1;
			break;
		case SPEAK_MONSTER_SAY:
			if (version == 822) ret = SPEAK822_MONSTER_SAY;
			else if (version == 840 || version == 842 || version == 850 || version == 854 || version == 860) ret = SPEAK840_MONSTER_SAY;
			else if (version == 870) ret = SPEAK870_MONSTER_SAY;
			else if (version == 910) ret = SPEAK910_MONSTER_SAY;
			break;
		case SPEAK_MONSTER_YELL:
			if (version == 822) ret = SPEAK822_MONSTER_YELL;
			else if (version == 840 || version == 842 || version == 850 || version == 854 || version == 860) ret = SPEAK840_MONSTER_YELL;
			else if (version == 870) ret = SPEAK870_MONSTER_YELL;
			else if (version == 910) ret = SPEAK910_MONSTER_YELL;
			break;
		default:
			break;
	}

	return ret;
}


COLOR ConvertColorSTD(unsigned char color) {
	COLOR ret;
	ret.red = (float)((int)((color / 6) / 6) % 6) / 5.0f;
	ret.green = (float)((int)(color / 6) % 6) / 5.0f;
	ret.blue = (float)((int)color % 6) / 5.0f;
	ret.alpha = 1.0f;

	return ret;
}

COLOR RT_ConvertColorHP(unsigned char* hp) {
	float nx = (float)(*hp) / 100;

	COLOR ret;
	ret.red = (nx < 0.5f ? 0.3 + 0.7f * (2 * nx) : 1.0f - 2 * (nx - 0.5f));
	ret.green = nx;
	ret.blue = 0.0f;
	ret.alpha = 1.0f;

	return ret;
}

COLOR RT_ConvertColorButton(unsigned long time, float r, float g, float b, float a) {
	float nx = (float)(RealTime::getTime() - time) / 1000;
	if (nx > 1.0f) nx = 1.0f;

	COLOR ret;
	ret.red = (1.0f * (1.0f - nx)) + (r * nx);
	ret.green = (0.0f * (1.0f - nx)) + (g * nx);
	ret.blue = (0.0f * (1.0f - nx)) + (b * nx);
	ret.alpha = a;

	return ret;
}


void RemoveEx(std::string path) {
	DIR *dp, *child_dp;
    struct dirent *dirp;

    if((dp = opendir(path.c_str())) == NULL) {
    	remove(path.c_str());
    	return;
    }

    while ((dirp = readdir(dp)) != NULL) {
		if (std::string(dirp->d_name) == "." || std::string(dirp->d_name) == "..")
			continue;

		std::string filename = std::string(path + "/" + dirp->d_name);

		child_dp = opendir(filename.c_str());
        if (child_dp) {
        	closedir(child_dp);
        	RemoveEx(filename);
		}
		else
			remove(filename.c_str());
    }
    closedir(dp);
    RemoveDirectory(path.c_str());
}

void CreateDirectoryEx(std::string path) {
	size_t pos = 0;
	while((pos = path.find('/', pos)) != std::string::npos) {
		std::string dir = path.substr(0, pos);
		if (dir == ".") {
			pos++;
			continue;
		}

		CreateDirectory(dir.c_str(), NULL);
		pos++;
	}

	CreateDirectory(path.c_str(), NULL);
}

std::string CutDirectory(std::string name) {
	std::string ret = name;
	size_t pos = ret.find_last_of("/\\");
	if (pos != std::string::npos)
		ret = ret.substr(pos + 1);

	return ret;
}
