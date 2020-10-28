/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#include "logger.h"

#include "allocator.h"
#include "backtrace.h"
#include "game.h"
#include "network.h"
#include "realtime.h"
#include "tools.h"


// ---- Logger ---- //

LoggingLevel	Logger::level = LOG_INFO;

std::list<std::pair<boost::thread::id, unsigned int> >	Logger::functionBacktrace;
std::list<unsigned short>								Logger::packetBacktrace;

std::string	Logger::sessionName = "";
FILE*		Logger::session = NULL;
MUTEX		Logger::lockLogger;


void Logger::OpenSession() {
	LOCKCLASS lockClass(lockLogger);

	if (session)
		fclose(session);

    time_lt sessionTime = RealTime::getTime();

    std::string timeFir = time2str(sessionTime / 1000) + "_" + time2str(sessionTime / 1000, false, false, false, true, true, true, "-");
    std::string timeSec = value2str((unsigned short)(sessionTime % 1000));
    for (int i = 0; i < 4 - timeSec.length(); i++)
        timeSec.insert(timeSec.begin(), '0');

	std::string fname = "session" + timeFir + "_" + timeSec + ".log";
	std::string path = "logs\\" + fname;
	sessionName = fname;
	session = fopen(path.c_str(), "wb");

	AddLog("Logger::OpenSession()", "Session has been started.", LOG_INFO);

	backtrace_register(boost::bind(&Logger::AddLog, "ExceptionFilter()", "Unhandled exception occured!", LOG_ERROR, (NetworkMessage*)NULL));

	//signal(SIGABRT, AbortFunction);
	//signal(SIGSEGV, AbortFunction);
	//signal(SIGILL, AbortFunction);
	//signal(SIGFPE, AbortFunction);
}

void Logger::CloseSession() {
	LOCKCLASS lockClass(lockLogger);

	AddLog("Logger::CloseSession()", "Session has been closed.", LOG_INFO);

	if (session) {
		fclose(session);
		session = NULL;
	}

	backtrace_unregister();
}

void Logger::SetLoggingLevel(LoggingLevel level) {
	AddLog("Logger::SetLoggingLevel()", "LoggingLevel has been changed.", LOG_INFO);

	Logger::level = level;
}

void Logger::AddLog(std::string function, std::string comment, LoggingLevel level, NetworkMessage* msg) {
	LOCKCLASS lockClass(lockLogger);

	if (Logger::level > level)
		return;

	if (!session)
		return;

	std::string line = (ftell(session) != 0 ? "\n" : "") +
		time2str(RealTime::getTime() / 1000, false, false, false, true, true, true) +
		" <" + (level == LOG_INFO ? "INFO" : (level == LOG_WARNING ? "WARNING" : "ERROR")) + ">" +
		" [" + function + "] " +
		comment;

	if (level >= LOG_WARNING) {
		if (packetBacktrace.size() > 0) {
			line += "\n\tLast Packets: ";
			std::list<unsigned short>::iterator it = packetBacktrace.begin();
			for (it; it != packetBacktrace.end(); it++) {
				if (it != packetBacktrace.begin()) line += ", ";
				char hex[5];
				sprintf(hex,"%X", (unsigned short)*it);
				line += std::string(hex);
			}
		}

		if (functionBacktrace.size() > 0) {
			line += "\n\tLast Functions: ";
			std::list<std::pair<boost::thread::id, unsigned int> >::iterator sit = functionBacktrace.begin();
			for (sit; sit != functionBacktrace.end(); sit++) {
				if (sit != functionBacktrace.begin()) line += ", ";
				std::stringstream function;
				function << "[" << sit->first << "]" << sit->second;
				line += function.str();
			}
		}

		if (level == LOG_ERROR) {
			Game* game = Game::game;
			Player* player = (game ? game->GetPlayer() : NULL);
			Creature* creature = (player ? player->GetCreature() : NULL);
			if (creature) {
				line += "\n\tPlayer:";
				line += "\n\t\tName: " + creature->GetName();
				line += "\n\t\tPosition: " + value2str(creature->pos.x) + "\\" + value2str(creature->pos.y) + "\\" + value2str(creature->pos.z);
			}

			if (msg) {
				line += "\n\tPacket:\n";
				for (int i = 0; i < msg->buffSize; i++) {
					if (i > 0) line += ", ";
					if (i == msg->readPos) {
						line += "<";
						line += value2str(msg->buffer[msg->startPos + i], true);
						line += ">";
					}
					else
						line += value2str(msg->buffer[msg->startPos + i], true);
				}
			}

			if (exceptionInfo)
				line += "\n\tException info: " + PrintExceptionInfo();
			if (backtrace_output)
				line += "\n\tStack trace: " + PrintStack();
			if (exceptionInfo)
				line += "\n\tMemory dump: " + PrintMemoryDump();
		}
	}

	fwrite(line.c_str(), strlen(line.c_str()), 1, session);
	fflush(session);

	if (level == LOG_ERROR) {
		Game* game = Game::game;
		if (game) {
			std::string filename = "logs\\" + sessionName;
			game->SaveErrorLog(filename);

			std::string message = "Debug Assertion!\n";
			message += "    " + function + "\n";
			message += "    " + comment + "\n";
			message += "See error.log file to get more informations.\n\n";
			if (exceptionInfo)
				message += "Exception info: " + PrintExceptionInfo("    ") + "\n";
			MessageBox(NULL, message.c_str(), "Error", MB_OK | MB_ICONERROR);
			Protocol* protocol = game->GetProtocol();
			if (protocol)
				protocol->CloseConnection();
			game->SetGameState(GAME_IDLE);
			game->PostQuit();
			exit(0);
		}
	}
}

void Logger::AddFunction(unsigned int func, bool remember) {
	if (!remember)
		return;

	LOCKCLASS lockClass(lockLogger);

	boost::thread::id ID = boost::this_thread::get_id();

	functionBacktrace.push_front(std::pair<boost::thread::id, unsigned int>(ID, func));
	if (functionBacktrace.size() > 32)
		functionBacktrace.pop_back();
}

void Logger::AddPacketType(unsigned short type) {
	LOCKCLASS lockClass(lockLogger);

	packetBacktrace.push_back(type);
	if (packetBacktrace.size() > 16)
		packetBacktrace.pop_front();
}


void Logger::AbortFunction(int sigNum) {
	std::string name;
	switch(sigNum) {
		case SIGABRT:	name = "SIGNAL ABORT";	break;
		case SIGSEGV:	name = "SIGNAL ILLEGAL MEMORY ACCESS";	break;
		case SIGILL:	name = "SIGNAL EXECUTING MALFORMED INSTRUCTION";	break;
		case SIGFPE:	name = "SIGNAL ILLEGAL FLOATING POINT INSTRUCTION";	break;
	}

	AddLog((char*)__FUNCTION__, name, LOG_ERROR, NULL);
}


std::string PrintStack(std::string margin) {
	std::string stack = (backtrace_output ? std::string("\n") + backtrace_output : std::string("none"));

	int i = 0;
	while(i < stack.length()) {
		char ch = stack[i];
		if (ch == '\n') {
			stack.insert(i + 1, margin);
			i += margin.length();
		}
		i++;
	}

	return stack;
}

std::string PrintExceptionInfo(std::string margin) {
	if (!exceptionInfo)
		return "";

	PEXCEPTION_RECORD exceptionRecord = exceptionInfo->ExceptionRecord;

	std::string exception = "\nException code: ";
	switch(exceptionRecord->ExceptionCode) {
		case EXCEPTION_ACCESS_VIOLATION: exception += "EXCEPTION_ACCESS_VIOLATION"; break;
		case EXCEPTION_ARRAY_BOUNDS_EXCEEDED: exception += "EXCEPTION_ARRAY_BOUNDS_EXCEEDED"; break;
		case EXCEPTION_BREAKPOINT: exception += "EXCEPTION_BREAKPOINT"; break;
		case EXCEPTION_DATATYPE_MISALIGNMENT: exception += "EXCEPTION_DATATYPE_MISALIGNMENT"; break;
		case EXCEPTION_FLT_DENORMAL_OPERAND: exception += "EXCEPTION_FLT_DENORMAL_OPERAND"; break;
		case EXCEPTION_FLT_DIVIDE_BY_ZERO: exception += "EXCEPTION_FLT_DIVIDE_BY_ZERO"; break;
		case EXCEPTION_FLT_INEXACT_RESULT: exception += "EXCEPTION_FLT_INEXACT_RESULT"; break;
		case EXCEPTION_FLT_INVALID_OPERATION: exception += "EXCEPTION_FLT_INVALID_OPERATION"; break;
		case EXCEPTION_FLT_OVERFLOW: exception += "EXCEPTION_FLT_OVERFLOW"; break;
		case EXCEPTION_FLT_STACK_CHECK: exception += "EXCEPTION_FLT_STACK_CHECK"; break;
		case EXCEPTION_FLT_UNDERFLOW: exception += "EXCEPTION_FLT_UNDERFLOW"; break;
		case EXCEPTION_ILLEGAL_INSTRUCTION: exception += "EXCEPTION_ILLEGAL_INSTRUCTION"; break;
		case EXCEPTION_IN_PAGE_ERROR: exception += "EXCEPTION_IN_PAGE_ERROR"; break;
		case EXCEPTION_INT_DIVIDE_BY_ZERO: exception += "EXCEPTION_INT_DIVIDE_BY_ZERO"; break;
		case EXCEPTION_INT_OVERFLOW: exception += "EXCEPTION_INT_OVERFLOW"; break;
		case EXCEPTION_INVALID_DISPOSITION: exception += "EXCEPTION_INVALID_DISPOSITION"; break;
		case EXCEPTION_NONCONTINUABLE_EXCEPTION: exception += "EXCEPTION_NONCONTINUABLE_EXCEPTION"; break;
		case EXCEPTION_PRIV_INSTRUCTION: exception += "EXCEPTION_PRIV_INSTRUCTION"; break;
		case EXCEPTION_SINGLE_STEP: exception += "EXCEPTION_SINGLE_STEP"; break;
		case EXCEPTION_STACK_OVERFLOW: exception += "EXCEPTION_STACK_OVERFLOW"; break;
		default: exception += "unknown"; break;
	}

	exception += "\nException address: 0x" + value2str((uint32_t)exceptionRecord->ExceptionAddress, true);
	exception += "\n";

	int i = 0;
	while(i < exception.length()) {
		char ch = exception[i];
		if (ch == '\n') {
			exception.insert(i + 1, margin);
			i += margin.length();
		}
		i++;
	}

	return exception;
}

std::string PrintMemoryDump(std::string margin) {
#ifndef NO_ALLOCATOR
	std::string memoryDump = "\n" + Allocator::MemoryDump();
#else
	std::string memoryDump = "none";
#endif

	int i = 0;
	while(i < memoryDump.length()) {
		char ch = memoryDump[i];
		if (ch == '\n') {
			memoryDump.insert(i + 1, margin);
			i += margin.length();
		}
		i++;
	}

	return memoryDump;
}
