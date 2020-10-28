/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#ifndef __LOGGER_H_
#define __LOGGER_H_

#include <stdio.h>
#include <string>

#include "mthread.h"


class NetworkMessage;


enum LoggingLevel {
	LOG_INFO	= 0,
	LOG_WARNING	= 1,
	LOG_ERROR	= 2,
};


class Logger {
private:
	static LoggingLevel		level;

	static std::list<std::pair<boost::thread::id, unsigned int> > functionBacktrace;
	static std::list<unsigned short>	packetBacktrace;

	static std::string	sessionName;
	static FILE*		session;
	static MUTEX		lockLogger;

public:
	static void OpenSession();
	static void CloseSession();
	static void SetLoggingLevel(LoggingLevel level);
	static void AddLog(std::string function, std::string comment, LoggingLevel level = LOG_INFO, NetworkMessage* msg = NULL);

	static void AddFunction(unsigned int func, bool remember = true);
	static void AddPacketType(unsigned short type);

	static void AbortFunction(int sigNum);
};


std::string PrintStack(std::string margin = "\t\t");
std::string PrintExceptionInfo(std::string margin = "\t\t");
std::string PrintMemoryDump(std::string margin = "\t\t");


#endif //__LOGGER_H_
