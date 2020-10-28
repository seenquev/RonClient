/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#ifndef __VERSIONER_H_
#define __VERSIONER_H_

#include <map>
#include <string>

#include "mthread.h"


class Versioner {
public:
	static std::map<std::string, int>	files;

	static MUTEX	lockVersioner;

public:
	static void UpdateFilesList(std::string startDir = ".", std::string directory = "");
	static bool IsFileOnList(std::string filename);
	static int GetFileVersion(std::string filename);
	static void ConvertOldToNewVersion(std::string filename);
};

#endif //__VERSIONER_H_
