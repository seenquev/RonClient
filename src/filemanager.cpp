/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#include "filemanager.h"

#include <dirent.h>
#include <iostream>
#include <set>
#include <stdio.h>
#include <windows.h>

#include "allocator.h"
#include "tools.h"

#include "Types.h"
#include "LzmaLib.h"


// ---- FileManager ---- //

FileManager* FileManager::fileManager = NULL;


FileManager::FileManager() {
	LOCKCLASS lockClass(lockFiles);

	MakeFilesTree();

	fileManager = this;
}

FileManager::~FileManager() { }


void FileManager::MakeFilesTree(std::string directory) {
	LOCKCLASS lockClass(lockFiles);

	DIR *dp, *child_dp;
    struct dirent *dirp;

    if ((dp = opendir(directory.c_str())) == NULL)
    	return;

	std::set<std::string> list;
	while((dirp = readdir(dp)) != NULL) {
		if (std::string(dirp->d_name) == "." || std::string(dirp->d_name) == "..")
			continue;

		list.insert(dirp->d_name);
	}

	for (std::set<std::string>::iterator lit = list.begin(); lit != list.end(); lit++) {
		std::string fullpath = directory + "/" + (*lit);
		child_dp = opendir(fullpath.c_str());

		if (child_dp) {
			MakeFilesTree(fullpath);
			closedir(child_dp);
		}
		else {
			size_t dot = fullpath.find_last_of('.');
			std::string ext = fullpath.substr(dot);

			if (ext == ".pak")
				MakeFilesTreeFromPAK(fullpath, directory);
			else {
				long int size = 0;

				FILE* file = fopen(fullpath.c_str(), "rb");
				if (file) {
					fseek(file, 0, SEEK_END);
					size = ftell(file);
					fclose(file);
				}

				if (files.find(fullpath) == files.end())
					files[fullpath] = FileInfo(size, 0);
			}
		}
	}

	closedir(dp);
}

void FileManager::MakeFilesTreeFromPAK(std::string path, std::string directory) {
	LOCKCLASS lockClass(lockFiles);

	FILE* file = fopen(path.c_str(), "rb");
	if (!file)
		return;

	FilesTree pakFiles;

	uint64_t encKey;
	fread(&encKey, 8, 1, file);
	uint64_t key = encKey ^ 0xFBFFFFFFFFFFBFFF;

	bool compression = false;
	if (key == 0)
		compression = true;

	uint32_t n;
	fread(&n, 4, 1, file);

	for (uint32_t i = 0; i < n; i++) {
		uint32_t fileHandler;
		uint32_t fileSize;
		uint32_t fileSizeCompressed = 0;
		fread(&fileHandler, 4, 1, file);
		fread(&fileSize, 4, 1, file);
		if (compression)
			fread(&fileSizeCompressed, 4, 1, file);

		uint16_t nameSize;
		std::string filePath;
		fread(&nameSize, 2, 1, file);
		for (uint16_t l = 0; l < nameSize; l++)
			filePath.push_back(fgetc(file));

		pakFiles[filePath] = FileInfo(path, fileSize, fileSizeCompressed, fileHandler, key);
	}
	uint32_t headerSize;
	fread(&headerSize, 4, 1, file);

	fclose(file);

	FilesTree::iterator it = pakFiles.begin();
	for (it; it != pakFiles.end(); it++) {
		std::string fileName = directory + "/" + it->first.substr(2);

		FileInfo info = it->second;
		info.pakHandler += headerSize;

		files[fileName] = info;
	}
}


void MakeFilesTreeForPAK(std::string startDir, std::string nextDir, FilesTree& tree, long int& handler) {
	DIR *dp, *child_dp;
    struct dirent *dirp;

	std::string temp = (nextDir == "." ? std::string("") : std::string("/") + nextDir.substr(2));
	std::string directory = startDir + temp;

    if ((dp = opendir(directory.c_str())) == NULL)
    	return;

	std::set<std::string> list;
	while((dirp = readdir(dp)) != NULL) {
		if (std::string(dirp->d_name) == "." || std::string(dirp->d_name) == "..")
			continue;

		list.insert(dirp->d_name);
	}

	for (std::set<std::string>::iterator lit = list.begin(); lit != list.end(); lit++) {
		std::string nextpath = nextDir + "/" + (*lit);
		std::string fullpath = directory + "/" + (*lit);
		child_dp = opendir(fullpath.c_str());

		if (child_dp) {
			MakeFilesTreeForPAK(startDir, nextpath, tree, handler);
			closedir(child_dp);
		}
		else {
			long int size = 0;

			FILE* file = fopen(fullpath.c_str(), "rb");
			if (file) {
				fseek(file, 0, SEEK_END);
				size = ftell(file);
				fclose(file);
			}

			tree[nextpath] = FileInfo("pak", size, 0, handler, 0);
			handler += size;
		}
	}
}

void FileManager::AddToTree(std::string fileName) {
	fileName = FixFileName(fileName);

	size_t slash = fileName.find_last_of('/');
	std::string directory = fileName.substr(slash);

	size_t dot = fileName.find_last_of('.');
	std::string ext = fileName.substr(dot);

	if (ext == ".pak")
		MakeFilesTreeFromPAK(fileName, directory);
	else {
		long int size = 0;

		FILE* file = fopen(fileName.c_str(), "rb");
		if (file) {
			fseek(file, 0, SEEK_END);
			size = ftell(file);
			fclose(file);
		}

		FilesTree::iterator it = files.find(fileName);
		if (it == files.end() || it->second.pakFile == "")
			files[fileName] = FileInfo(size, 0);
	}
}

void FileManager::CompilePAK(std::string directory, std::string pakFileName, uint64_t key) {
	LOCKCLASS lockClass(lockFiles);

	long int size = 0;
	FilesTree pakFiles;
	MakeFilesTreeForPAK(directory, ".", pakFiles, size);

	FilesTree::iterator it;

	bool compressed = false;
	std::map<std::string, unsigned char*> cData;
	if (key == 0) {
		long int handler = 0;
		for (it = pakFiles.begin(); it != pakFiles.end(); it++) {
			std::string fileName = directory + "/" + it->first.substr(2);
			FileInfo& info = it->second;

			FILE* file = fopen(fileName.c_str(), "rb");
			if (!file)
				continue;

			uint32_t fileSize = info.fileSize;
			unsigned char* data = new(M_PLACE) unsigned char[fileSize];
			fread(data, 1, fileSize, file);

			std::vector<unsigned char> c_data;
			Compress(c_data, data, fileSize);

			delete_debug_array(data, M_PLACE);

			fclose(file);

			info.pakHandler = handler;
			info.fileSizeCompressed = c_data.size();
			data = new(M_PLACE) unsigned char[c_data.size()];
			memcpy(data, c_data.data(), c_data.size());
			cData[fileName] = data;

			handler += c_data.size();
		}
		compressed = true;
	}

	FILE* pakFile = fopen(pakFileName.c_str(), "wb");
	if (!pakFile)
		return;

	uint64_t encKey = key ^ 0xFBFFFFFFFFFFBFFF;
	fwrite(&encKey, 8, 1, pakFile);

	uint32_t n = pakFiles.size();
	fwrite(&n, 4, 1, pakFile);
	for (it = pakFiles.begin(); it != pakFiles.end(); it++) {
		std::string fileName = it->first;
		FileInfo info = it->second;

		uint32_t fileHandler = info.pakHandler;
		uint32_t fileSize = info.fileSize;
		uint32_t fileSizeCompressed = info.fileSizeCompressed;
		fwrite(&fileHandler, 4, 1, pakFile);
		fwrite(&fileSize, 4, 1, pakFile);
		if (compressed)
			fwrite(&fileSizeCompressed, 4, 1, pakFile);

		uint16_t nameSize = fileName.length();
		fwrite(&nameSize, 2, 1, pakFile);
		for (uint16_t l = 0; l < nameSize; l++)
			fputc(fileName[l], pakFile);
	}
	uint32_t headerSize = ftell(pakFile) + 4;
	fwrite(&headerSize, 4, 1, pakFile);

	if (!compressed) {
		for (it = pakFiles.begin(); it != pakFiles.end(); it++) {
			std::string fileName = directory + "/" + it->first.substr(2);
			FileInfo info = it->second;

			FILE* file = fopen(fileName.c_str(), "rb");
			if (!file) {
				fclose(pakFile);
				RemoveEx(pakFileName.c_str());
				return;
			}

			uint32_t fileSize = info.fileSize;
			unsigned char* data = new(M_PLACE) unsigned char[fileSize];
			fread(data, 1, fileSize, file);
			if (key)
				Encode(data, fileSize, key);
			fwrite(data, 1, fileSize, pakFile);
			delete_debug_array(data, M_PLACE);

			fclose(file);
		}
	}
	else {
		bool failed = false;
		for (it = pakFiles.begin(); it != pakFiles.end(); it++) {
			std::string fileName = directory + "/" + it->first.substr(2);
			FileInfo info = it->second;

			std::map<std::string, unsigned char*>::iterator itc;
			if ((itc = cData.find(fileName)) == cData.end()) {
				failed = true;
				continue;
			}

			unsigned char* data = itc->second;
			if (!failed) {
				uint32_t fileSize = info.fileSizeCompressed;
				fwrite(data, 1, fileSize, pakFile);
			}
			delete_debug_array(data, M_PLACE);
		}

		if (failed) {
			fclose(pakFile);
			RemoveEx(pakFileName.c_str());
			return;
		}
	}

	fclose(pakFile);
}

void FileManager::ExtractPAK(std::string pakFileName, std::string directory) {
	LOCKCLASS lockClass(lockFiles);

	FILE* pakFile = fopen(pakFileName.c_str(), "rb");
	if (!pakFile)
		return;

	FilesTree pakFiles;

	uint64_t encKey;
	fread(&encKey, 8, 1, pakFile);
	uint64_t key = encKey ^ 0xFBFFFFFFFFFFBFFF;

	bool compressed = false;
	if (key == 0)
		compressed = true;

	uint32_t n;
	fread(&n, 4, 1, pakFile);

	for (uint32_t i = 0; i < n; i++) {
		uint32_t fileHandler;
		uint32_t fileSize;
		uint32_t fileSizeCompressed;
		fread(&fileHandler, 4, 1, pakFile);
		fread(&fileSize, 4, 1, pakFile);
		if (compressed)
			fread(&fileSizeCompressed, 4, 1, pakFile);

		uint16_t nameSize;
		std::string filePath;
		fread(&nameSize, 2, 1, pakFile);
		for (uint16_t l = 0; l < nameSize; l++)
			filePath.push_back(fgetc(pakFile));

		pakFiles[filePath] = FileInfo(pakFileName, fileSize, fileSizeCompressed, fileHandler, key);
	}
	uint32_t headerSize;
	fread(&headerSize, 4, 1, pakFile);

	FilesTree::iterator it = pakFiles.begin();
	for (it; it != pakFiles.end(); it++) {
		std::string fileName = directory + "/" + it->first.substr(2);
		FileInfo info = it->second;

		size_t slash = fileName.find_last_of('/');
		std::string dir = fileName.substr(0, slash);

		CreateDirectoryEx(dir);

		FILE* file = fopen(fileName.c_str(), "wb");
		if (!file) {
			fileName += "_";
			file = fopen(fileName.c_str(), "wb");
			if (!file)
				continue;
		}

		if (!compressed) {
			uint32_t fileHandler = info.pakHandler + headerSize;
			uint32_t fileSize = info.fileSize;
			fseek(pakFile, fileHandler, SEEK_SET);
			unsigned char* data = new(M_PLACE) unsigned char[fileSize];
			size_t size = fread(data, 1, fileSize, pakFile);
			if (key)
				Decode(data, fileSize, key);
			fwrite(data, 1, fileSize, file);
			delete_debug_array(data, M_PLACE);
		}
		else {
			uint32_t fileHandler = info.pakHandler + headerSize;
			uint32_t fileSize = info.fileSizeCompressed;
			fseek(pakFile, fileHandler, SEEK_SET);
			unsigned char* data = new(M_PLACE) unsigned char[fileSize];
			size_t size = fread(data, 1, fileSize, pakFile);

			std::vector<unsigned char> c_data;
			Uncompress(c_data, data, fileSize);

			fwrite(c_data.data(), 1, c_data.size(), file);
			delete_debug_array(data, M_PLACE);
		}

		fclose(file);

		AddToTree(fileName);
	}

	fclose(pakFile);
}


std::string FileManager::FixFileName(std::string fileName) {
	if (fileName.length() == 0 || fileName[0] != '.') fileName.insert(0, 1, '.');
	if (fileName.length() <= 1 || fileName[1] != '/') fileName.insert(1, 1, '/');
	return fileName;
}


FILE* FileManager::OpenFile(std::string fileName) {
	LOCKCLASS lockClass(lockFiles);

	fileName = FixFileName(fileName);

	FileInfo info;
	FilesTree::iterator it = files.find(fileName);
	if (it != files.end())
		info = it->second;

	if (info.pakFile == "") {
		FILE* file = fopen(fileName.c_str(), "rb");
		return file;
	}

	FILE* file = fopen(info.pakFile.c_str(), "rb");
	fseek(file, info.pakHandler, SEEK_SET);
	return file;
}

unsigned char* FileManager::GetFileData(std::string fileName) {
	LOCKCLASS lockClass(lockFiles);

	fileName = FixFileName(fileName);

	FileInfo info;
	FilesTree::iterator it = files.find(fileName);
	if (it != files.end())
		info = it->second;

	if (info.pakFile == "") {
		FILE* file = fopen(fileName.c_str(), "rb");
		if (file) {
			fseek(file, 0, SEEK_END);
			long int size = ftell(file);
			fseek(file, 0, SEEK_SET);
			unsigned char* data = new unsigned char[size];
			fread(data, 1, size, file);
			fclose(file);
			return data;
		}
	}
	else {
		FILE* file = fopen(info.pakFile.c_str(), "rb");
		if (file) {
			bool compressed = (info.pakKey == 0);
			fseek(file, info.pakHandler, SEEK_SET);
			long int fileSize = (compressed ? info.fileSizeCompressed : info.fileSize);
			unsigned char* data = new unsigned char[fileSize];
			fread(data, 1, fileSize, file);
			if (!compressed)
				Decode(data, info.fileSize, info.pakKey);
			else {
				std::vector<unsigned char> c_data;
				Uncompress(c_data, data, fileSize);
				delete_debug_array(data, M_PLACE);

				data = new unsigned char[c_data.size()];
				memcpy(data, c_data.data(), c_data.size());
			}
			fclose(file);
			return data;
		}
	}

	return NULL;
}

long int FileManager::GetFileSize(std::string fileName) {
	LOCKCLASS lockClass(lockFiles);

	fileName = FixFileName(fileName);

	FileInfo info;
	FilesTree::iterator it = files.find(fileName);
	if (it != files.end())
		info = it->second;

	if (info.pakFile == "") {
		FILE* file = fopen(fileName.c_str(), "rb");
		if (file) {
			fseek(file, 0, SEEK_END);
			long int ret = ftell(file);
			fclose(file);
			return ret;
		}
	}

	return info.fileSize;
}

/*long int FileManager::Tell(std::string fileName, FILE* file) {
	LOCKCLASS lockClass(lockFiles);

	fileName = FixFileName(fileName);

	FileInfo info;
	FilesTree::iterator it = files.find(fileName);
	if (it != files.end())
		info = it->second;

	return ftell(file) - info.pakHandler;
}*/

FileInfo FileManager::GetFileInfo(std::string fileName) {
	LOCKCLASS lockClass(lockFiles);

	fileName = FixFileName(fileName);

	FileInfo info;
	FilesTree::iterator it = files.find(fileName);
	if (it != files.end())
		info = it->second;

	return info;
}

void FileManager::Encode(unsigned char* data, long int size, uint64_t key) {
	for (long int i = 0; i < size; i++)
		data[i] = data[i] ^ (((uint64_t)(i * key) >> ((i % 8) * 8)) % 256);

	long int sizer = 1 + size % 7;
	for (long int i = 0; i + 8 < size; i += 8) {
		unsigned char t = data[i];
		data[i] = data[i+sizer];
		data[i+sizer] = t;
	}
}

void FileManager::Decode(unsigned char* data, long int size, uint64_t key) {
	long int sizer = 1 + size % 7;
	for (long int i = 0; i + 8 < size; i += 8) {
		unsigned char t = data[i];
		data[i] = data[i+sizer];
		data[i+sizer] = t;
	}

	for (long int i = 0; i < size; i++)
		data[i] = data[i] ^ (((uint64_t)(i * key) >> ((i % 8) * 8)) % 256);
}


void FileManager::Compress(std::vector<unsigned char> &outBuf, unsigned char *inBuf, size_t inSize) {
	unsigned propsSize = LZMA_PROPS_SIZE;
	unsigned destLen = inSize + inSize / 3 + 128;
	outBuf.resize(propsSize + destLen);

	int res = LzmaCompress(
		&outBuf[LZMA_PROPS_SIZE], &destLen,
		inBuf, inSize,
		&outBuf[0], &propsSize,
		-1, 0, -1, -1, -1, -1, -1);

	outBuf.resize(propsSize + destLen);

	unsigned char *sz = (unsigned char*)&inSize;
	outBuf.push_back(sz[0]);
	outBuf.push_back(sz[1]);
	outBuf.push_back(sz[2]);
	outBuf.push_back(sz[3]);
}

void FileManager::Uncompress(std::vector<unsigned char> &outBuf, unsigned char *inBuf, size_t inSize) {
	size_t sz = *(size_t*)(inBuf + inSize - 4);
	outBuf.resize(sz);
	inSize -= 4;

	unsigned dstLen = outBuf.size();
	unsigned srcLen = inSize - LZMA_PROPS_SIZE;
	SRes res = LzmaUncompress(
		&outBuf[0], &dstLen,
		inBuf + LZMA_PROPS_SIZE, &srcLen,
		inBuf, LZMA_PROPS_SIZE);
	outBuf.resize(dstLen); // If uncompressed data can be smaller
}
