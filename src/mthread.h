/* -------------------------------------------------------------------------- */
/* ------------- RonClient --- Oficial client for RonOTS servers ------------ */
/* -------------------------------------------------------------------------- */

#ifndef __MTHREAD_H_
#define __MTHREAD_H_

#include <map>
#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>


#define LOCKCLASS boost::recursive_mutex::scoped_lock

typedef boost::thread THREAD;
typedef boost::recursive_mutex MUTEX;

typedef std::list<std::pair<boost::function<void()>, bool> > Functions;


class Signal {
public:
	Functions	functions;

	MUTEX		lockSignal;

public:
	Signal();
	Signal(const Signal& signal);
	~Signal();

	Functions::iterator PushFunction(boost::function<void()> func);
	void BlockFunction(Functions::iterator it);
	void UnblockFunction(Functions::iterator it);
	bool IsExecutable();
	void Execute(bool deleteAfter = false);
	void Clear();

	void Swap(Signal& signal);
	void Assign(Signal& signal);

	int GetSize();
};

#endif //__MTHREAD_H_
