#ifndef __THREAD_H
#define __THREAD_H
#include "Headers.hpp"

class Thread
{
public:
	Thread(uint thread_id) 
	{
        // Only places thread_id
	    this->m_thread_id = thread_id;
	} 
	virtual ~Thread() {} // Does nothing 

	/** Returns true if the thread was successfully started, false if there was an error starting the thread */
	bool start()
	{
	    return (pthread_create(&m_thread, NULL, entry_func, (void*)this) == 0);
	}

	/** Will not return until the internal thread has exited. */
	void join()
	{
	    pthread_join(m_thread, NULL);
        //pthread_exit(0);
	}

	/** Returns the thread_id **/
	uint thread_id()
	{
        return this->m_thread_id;
	}
protected:
	
    virtual void thread_workload() = 0;
    uint m_thread_id; // A number from 0 -> Number of threads initialized, providing a simple numbering for you to use

private:
    static void * entry_func(void * thread) { ((Thread *)thread)->thread_workload(); return NULL; }
    pthread_t m_thread;
};

#endif
