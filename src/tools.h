/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#ifndef __TOOLS_H_
#define __TOOLS_H_

#include <ctime>
#include <iostream>
#include <iomanip>
#include <vector>
#include <list>
#include <sstream>
#include <string>
#include <windows.h>
#include <stdint.h>

#include "ad2d.h"
#include "const.h"

class TypePointer {
public:
	std::string type;
	void* ptr;

public:
	TypePointer() { type = ""; ptr = NULL; }
	TypePointer(std::string type) { this->type = type; this->ptr = NULL; }
	TypePointer(std::string type, void* ptr) { this->type = type; this->ptr = ptr; }

	void SetValue(std::string data);
	std::string GetValue(int precision = 2);
};


class TextString {
public:
	std::string text;
	std::string color;

	TextString() { text = ""; color = ""; }
	TextString(const TextString& txt) : text(txt.text), color(txt.color) { }
	TextString(std::string txt, unsigned char col = 215) : text(txt) { color.insert(0, txt.length(), col); }
	TextString(std::string txt, std::string col) : text(txt), color(col) { }
};

POINT doPOINT(int x, int y);
bool operator==(POINT a, POINT b);
bool operator!=(POINT a, POINT b);
bool operator<(POINT a, POINT b);
bool operator<=(POINT a, POINT b);
bool operator>(POINT a, POINT b);
bool operator>=(POINT a, POINT b);

typedef std::vector<std::pair<unsigned int, TextString> > DividedText;

bool IsSpecialChar(unsigned char ch);

std::string ConvertIP(unsigned int ip);
TextString ScrollText(TextString txt, AD2D_Font* font, unsigned short fontSize, int length, int offset = 0);
TextString ScrollText(TextString txt, int length, int offset = 0);
DividedText DivideText(TextString txt);
TextString SetTextColorMap(std::string txt, unsigned char color1, unsigned char color2);
std::string value2str(int64_t value, bool hex = false, int chars = 0);
std::string float2str(double value, int precision = 2);
std::string time2str(time_t time, bool year = true, bool month = true, bool day = true, bool hour = false, bool minute = false, bool second = false, const char* separator = ":");
std::string tolower(std::string text);
bool CheckChecksumFile(const char* path, unsigned long checksum);
bool CheckChecksum(std::string str, unsigned long checksum);

uint32_t MakeChecksum(std::string str);

unsigned char ConvertSpeakClasses(unsigned char speakClass, unsigned short version);

COLOR ConvertColorSTD(unsigned char color);

COLOR RT_ConvertColorHP(unsigned char* hp);
COLOR RT_ConvertColorButton(unsigned long time, float r, float g, float b, float a);

void RemoveEx(std::string directory);
void CreateDirectoryEx(std::string directory);
std::string CutDirectory(std::string name);

#endif //__TOOLS_H_
