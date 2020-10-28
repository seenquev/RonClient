/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#include "allocator.h"
#include "tools.h"

#ifndef NO_ALLOCATOR

MUTEX Allocator::lockAllocator;
MemoryMap Allocator::memoryMap;
MemoryMap Allocator::violationMap;


std::string Allocator::MemoryDump() {
	LOCKCLASS lockClass(Allocator::lockAllocator);

	std::string memoryDump;

	MemoryMap::iterator it = memoryMap.begin();
	for (it; it != memoryMap.end(); it++)
		memoryDump += "Pointer: 0x" + value2str((uint32_t)it->first, true) + ", File: " + CutDirectory(it->second.first) + " (" + value2str(it->second.second) +")\n";

	return memoryDump;
}


void* operator new(size_t bytes, const char* function, uint16_t line) throw(std::bad_alloc) {
	LOCKCLASS lockClass(Allocator::lockAllocator);
	void* ptr = ::operator new(bytes);
	MemoryMap::iterator it = Allocator::memoryMap.find(ptr);
	if (it != Allocator::memoryMap.end())
		Logger::AddLog(std::string(function) + " [" + value2str(line) + "]", std::string("New violation (") + value2str((unsigned long)ptr, true) + ")", LOG_WARNING);
	Allocator::memoryMap[ptr] = std::pair<const char*, uint16_t>(function, line);
	return ptr;
}

void* operator new[](size_t bytes, const char* function, uint16_t line) throw(std::bad_alloc) {
	LOCKCLASS lockClass(Allocator::lockAllocator);
	void* ptr = ::operator new[](bytes);
	MemoryMap::iterator it = Allocator::memoryMap.find(ptr);
	if (it != Allocator::memoryMap.end())
		Logger::AddLog(std::string(function) + " [" + value2str(line) + "]", std::string("New array violation (") + value2str((unsigned long)ptr, true) + ")", LOG_WARNING);
	Allocator::memoryMap[ptr] = std::pair<const char*, uint16_t>(function, line);
	return ptr;
}

#else

void* operator new(size_t bytes, const char* function, uint16_t line) throw(std::bad_alloc) {
	return ::operator new(bytes);
}

void* operator new[](size_t bytes, const char* function, uint16_t line) throw(std::bad_alloc) {
	return ::operator new[](bytes);
}

#endif
