/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#ifndef __ALLOCATOR_H
#define __ALLOCATOR_H

#include <iostream>
#include <map>
#include <stdio.h>
#include <string>
#include <windows.h>

#include "logger.h"
#include "mthread.h"
#include "realtime.h"
#include "tools.h"

#define M_PLACE __FILE__, __LINE__

#ifndef NO_ALLOCATOR


typedef std::map<void*, std::pair<const char*, uint16_t> > MemoryMap;


class Allocator {
public:
	static MUTEX lockAllocator;
	static MemoryMap memoryMap;
	static MemoryMap violationMap;

public:
	static std::string MemoryDump();
};


void* operator new(size_t bytes, const char* function, uint16_t line) throw(std::bad_alloc);
void* operator new[](size_t bytes, const char* function, uint16_t line) throw(std::bad_alloc);


template<class T>
void delete_debug(T* ptr, const char* function, uint16_t line) {
	Allocator::lockAllocator.lock();
	MemoryMap::iterator it = Allocator::memoryMap.find((void*)ptr);
	if (it != Allocator::memoryMap.end()) {
		Allocator::memoryMap.erase(it);
		Allocator::lockAllocator.unlock();
		delete ptr;
	}
	else {
		Allocator::violationMap[(void*)ptr] = std::pair<const char*, uint16_t>(function, line);
		Logger::AddLog(std::string(function) + " [" + value2str(line) + "]", std::string("Delete debug violation (0x") + value2str((unsigned long)ptr, true) + ")", LOG_WARNING);
		Allocator::lockAllocator.unlock();
	}
}

template<class T>
void delete_debug_array(T* ptr, const char* function, uint16_t line) {
    Allocator::lockAllocator.lock();
	MemoryMap::iterator it = Allocator::memoryMap.find((void*)ptr);
	if (it != Allocator::memoryMap.end()) {
		Allocator::memoryMap.erase(it);
		Allocator::lockAllocator.unlock();
		delete ptr;
	}
	else {
		Allocator::violationMap[(void*)ptr] = std::pair<const char*, uint16_t>(function, line);
		Logger::AddLog(std::string(function) + " [" + value2str(line) + "]", std::string("Delete debug array violation (0x") + value2str((unsigned long)ptr, true) + ")", LOG_WARNING);
		Allocator::lockAllocator.unlock();
	}
}

#else

void* operator new(size_t bytes, const char* function, uint16_t line) throw(std::bad_alloc);
void* operator new[](size_t bytes, const char* function, uint16_t line) throw(std::bad_alloc);


template<class T>
void delete_debug(T* ptr, const char* function, uint16_t line) {
	delete ptr;
}

template<class T>
void delete_debug_array(T* ptr, const char* function, uint16_t line) {
	delete[] ptr;
}

#endif

#endif
