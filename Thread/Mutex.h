//************************************************************************************
//Description: 对linux的锁及条件变量进行了封装
////1.多次唤醒是没有问题的
////2.对于正在运行的线程进行唤醒，将不起作用
////3.尽量采用先放入数据，再唤醒的顺序--可以不必这么做，因为有锁的存在，所以不会产生问题
//Author: jychen
//************************************************************************************


#ifndef MUTEXT_H
#define MUTEXT_H

#include <pthread.h>

class CMutex
{
public:
	CMutex()
	{
		pthread_mutex_init(&m_Mutex, NULL);
	}

	~CMutex()
	{
		pthread_mutex_destroy(&m_Mutex);
	}

	pthread_mutex_t* GetMutex()
	{
		return &m_Mutex;
	}

	void Lock()
	{
		pthread_mutex_lock(&m_Mutex);
	}

	void Unlock()
	{
		pthread_mutex_unlock(&m_Mutex);
	}

private:
	pthread_mutex_t m_Mutex;
};

//used for auto unlock
class CMutexLock
{
public:
	CMutexLock(CMutex& mutex):m_Mutex(mutex)
	{
		m_Mutex.Lock();
	}

	~CMutexLock()
	{
		m_Mutex.Unlock();
	}

private:
	CMutex& m_Mutex;
};

#define CMutexLock(x) static_assert(false, "missing mutex guard var name")


class CCondition
{
public:
	CCondition(CMutex& mutex):m_Mutex(mutex)
	{
		pthread_cond_init(&m_Cond, NULL);
	}

	~CCondition()
	{
		pthread_cond_destroy(&m_Cond);
	}

	void Wait()
	{
		pthread_cond_wait(&m_Cond, m_Mutex.GetMutex());
	}

	void Wake()
	{
		pthread_cond_signal(&m_Cond);
	}

	void WakeAll()
	{
		pthread_cond_broadcast(&m_Cond);
	}

private:
	CMutex& m_Mutex;

	pthread_cond_t m_Cond;
};


#endif //MUTEXT_H

