/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#include "versioner.h"

#include <dirent.h>

#include "allocator.h"
#include "iniloader.h"
#include "tools.h"


// ---- Versioner ---- //

std::map<std::string, int>	Versioner::files;

MUTEX	Versioner::lockVersioner;


void Versioner::UpdateFilesList(std::string startDir, std::string directory) {
	LOCKCLASS lockClass(lockVersioner);

	if (directory.length() == 0) {
		files.clear();
		directory = startDir;
	}

	INILoader iniLoader;
	iniLoader.OpenFile(startDir + "/version.ini");

	DIR *dp, *child_dp;
    struct dirent *dirp;

    if((dp = opendir(directory.c_str())) == NULL)
    	return;

    while ((dirp = readdir(dp)) != NULL) {
		if (std::string(dirp->d_name) == "." || std::string(dirp->d_name) == "..")
			continue;

		std::string fullpath = std::string(directory + "/" + dirp->d_name).substr(2, std::string::npos);
		std::string filename = std::string(directory + "/" + dirp->d_name).substr(startDir.length() + 1, std::string::npos);

		child_dp = opendir(fullpath.c_str());

		int ver = 0;
		if (atoi(iniLoader.GetValue("RONCLIENT_UPDATER").c_str()) != 0)
			ver = atoi(iniLoader.GetValue(filename, 0).c_str());
		else
			ver = atoi(iniLoader.GetValue(filename, 1).c_str());

		files[fullpath] = ver;

        if (child_dp) {
			UpdateFilesList(startDir, directory + "/" + dirp->d_name);
        	closedir(child_dp);
		}
    }
    closedir(dp);
}

bool Versioner::IsFileOnList(std::string filename) {
	LOCKCLASS lockClass(lockVersioner);

	std::map<std::string, int>::iterator it = files.find(filename);
	if (it != files.end())
		return true;

	return false;
}

int Versioner::GetFileVersion(std::string filename) {
	LOCKCLASS lockClass(lockVersioner);

	std::map<std::string, int>::iterator it = files.find(filename);
	if (it != files.end())
		return it->second;

	return 0;
}

void Versioner::ConvertOldToNewVersion(std::string filename) {
	LOCKCLASS lockClass(lockVersioner);

	INILoader iniLoader;
	if (!iniLoader.OpenFile(filename))
		return;

	if (atoi(iniLoader.GetValue("RONCLIENT_UPDATER").c_str()) != 0)
		return;

	INILoader iniLoaderNew;
	iniLoaderNew.SetValue("RONCLIENT_UPDATER", "1");

	int i = 0;
	std::string key;
	while((key = iniLoader.GetVarName(i)) != "") {
		std::string value = iniLoader.GetValue(i, 0);
		std::string value2 = iniLoader.GetValue(i, 1);
		if (value2 == "")
			value2 = "0";

		if (key == "VERSION")
			iniLoaderNew.SetValue("VERSION", value);
		else if (key != "RONCLIENT_UPDATER") {
			if (value == "DIR")
				iniLoaderNew.SetValue(key, value);
			else if (value != "DEL")
				iniLoaderNew.SetValue(key, value2);
		}
		i++;
	}

	iniLoaderNew.SaveFile(filename);
}
