/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#ifndef __FILEMANAGER_H_
#define __FILEMANAGER_H_


#include <map>
#include <string>

#include "mthread.h"


struct FileInfo {
	long int fileSize;
	long int fileSizeCompressed;

	std::string pakFile;
	long int pakHeader;
	long int pakHandler;
	uint64_t pakKey;

	FileInfo() {
		fileSize = 0;
		fileSizeCompressed = 0;
		pakFile = "";
		pakHandler = 0;
		pakKey = 0;
	};

	FileInfo(long int fSize, long int fSizeC) {
		fileSize = fSize;
		fileSizeCompressed = fSizeC;
		pakFile = "";
		pakHandler = 0;
		pakKey = 0;
	};

	FileInfo(std::string pFile, long int fSize, long int fSizeC, long int pHandler, uint64_t pKey) {
		fileSize = fSize;
		fileSizeCompressed = fSizeC;
		pakFile = pFile;
		pakHandler = pHandler;
		pakKey = pKey;
	}
};

typedef std::map<std::string, FileInfo> FilesTree;


class FileManager {
private:
	FilesTree	files;

	MUTEX	lockFiles;

public:
	static FileManager*		fileManager;

public:
	FileManager();
	~FileManager();

	void MakeFilesTree(std::string directory = ".");
	void MakeFilesTreeFromPAK(std::string path, std::string directory);

	void AddToTree(std::string fileName);

	void CompilePAK(std::string directory, std::string pakFileName, uint64_t key);
	void ExtractPAK(std::string pakFileName, std::string directory);

	FILE* OpenFile(std::string fileName);
	unsigned char* GetFileData(std::string fileName);
	long int GetFileSize(std::string fileName);
	//long int Tell(std::string fileName, FILE* file);
	FileInfo GetFileInfo(std::string fileName);

	static std::string FixFileName(std::string fileName);

	static void Encode(unsigned char* data, long int size, uint64_t key);
	static void Decode(unsigned char* data, long int size, uint64_t key);

	static void Compress(std::vector<unsigned char> &outBuf, unsigned char *inBuf, size_t inSize);
	static void Uncompress(std::vector<unsigned char> &outBuf, unsigned char *inBuf, size_t inSize);
};

#endif //__FILEMANAGER_H_
