/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#ifndef __BACKTRACE_H_
#define __BACKTRACE_H_

#include <windows.h>

#include <boost/function.hpp>

extern char* backtrace_output;
extern LPEXCEPTION_POINTERS exceptionInfo;

void backtrace_register(boost::function<void()> exceptionFunction);
void backtrace_unregister();

#endif
