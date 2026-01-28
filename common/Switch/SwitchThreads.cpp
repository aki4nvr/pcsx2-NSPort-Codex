// SPDX-FileCopyrightText: 2002-2026 PCSX2 Dev Team
// SPDX-License-Identifier: GPL-3.0+

#include "common/Threading.h"
#include "common/Assertions.h"

#include <memory>
#include <pthread.h>
#include <time.h>
#include <thread>

__forceinline void Threading::Timeslice()
{
	std::this_thread::yield();
}

__forceinline void Threading::SpinWait()
{
#if defined(_M_ARM64)
	__asm__ __volatile__("yield");
#endif
}

__forceinline void Threading::EnableHiresScheduler()
{
}

__forceinline void Threading::DisableHiresScheduler()
{
}

u64 Threading::GetThreadTicksPerSecond()
{
	return 1000000;
}

static u64 get_thread_time()
{
	struct timespec ts;
	if (clock_gettime(CLOCK_THREAD_CPUTIME_ID, &ts) != 0)
		return 0;

	return (u64)ts.tv_sec * (u64)1000000 + (u64)ts.tv_nsec / (u64)1000;
}

u64 Threading::GetThreadCpuTime()
{
	return get_thread_time();
}

Threading::ThreadHandle::ThreadHandle() = default;

Threading::ThreadHandle::ThreadHandle(const ThreadHandle& handle)
	: m_native_handle(handle.m_native_handle)
{
}

Threading::ThreadHandle::ThreadHandle(ThreadHandle&& handle)
	: m_native_handle(handle.m_native_handle)
{
	handle.m_native_handle = nullptr;
}

Threading::ThreadHandle::~ThreadHandle() = default;

Threading::ThreadHandle Threading::ThreadHandle::GetForCallingThread()
{
	ThreadHandle ret;
	ret.m_native_handle = (void*)pthread_self();
	return ret;
}

Threading::ThreadHandle& Threading::ThreadHandle::operator=(ThreadHandle&& handle)
{
	m_native_handle = handle.m_native_handle;
	handle.m_native_handle = nullptr;
	return *this;
}

Threading::ThreadHandle& Threading::ThreadHandle::operator=(const ThreadHandle& handle)
{
	m_native_handle = handle.m_native_handle;
	return *this;
}

u64 Threading::ThreadHandle::GetCPUTime() const
{
	return m_native_handle ? get_thread_time() : 0;
}

bool Threading::ThreadHandle::SetAffinity(u64 processor_mask) const
{
	(void)processor_mask;
	return false;
}

Threading::Thread::Thread() = default;

Threading::Thread::Thread(Thread&& thread)
	: ThreadHandle(thread)
	, m_stack_size(thread.m_stack_size)
{
	thread.m_stack_size = 0;
}

Threading::Thread::Thread(EntryPoint func)
	: ThreadHandle()
{
	if (!Start(std::move(func)))
		pxFailRel("Failed to start implicitly started thread.");
}

Threading::Thread::~Thread()
{
	if (m_native_handle)
		Detach();
}

Threading::ThreadHandle& Threading::Thread::operator=(Thread&& thread)
{
	ThreadHandle::operator=(std::move(thread));
	m_stack_size = thread.m_stack_size;
	thread.m_stack_size = 0;
	return *this;
}

void Threading::Thread::SetStackSize(u32 size)
{
	m_stack_size = size;
}

static void* ThreadProcThunk(void* param)
{
	std::unique_ptr<Threading::Thread::EntryPoint> func(static_cast<Threading::Thread::EntryPoint*>(param));
	(*func)();
	return nullptr;
}

bool Threading::Thread::Start(EntryPoint func)
{
	if (m_native_handle)
		return false;

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	if (m_stack_size)
		pthread_attr_setstacksize(&attr, m_stack_size);

	auto* heap_func = new EntryPoint(std::move(func));
	pthread_t thread;
	const int res = pthread_create(&thread, &attr, ThreadProcThunk, heap_func);
	pthread_attr_destroy(&attr);

	if (res != 0)
	{
		delete heap_func;
		return false;
	}

	m_native_handle = reinterpret_cast<void*>(thread);
	return true;
}

void Threading::Thread::Detach()
{
	if (!m_native_handle)
		return;

	pthread_detach(reinterpret_cast<pthread_t>(m_native_handle));
	m_native_handle = nullptr;
}

void Threading::Thread::Join()
{
	if (!m_native_handle)
		return;

	pthread_join(reinterpret_cast<pthread_t>(m_native_handle), nullptr);
	m_native_handle = nullptr;
}

void Threading::SetNameOfCurrentThread(const char* name)
{
	(void)name;
}

Threading::KernelSemaphore::KernelSemaphore()
{
	sem_init(&m_sema, 0, 0);
}

Threading::KernelSemaphore::~KernelSemaphore()
{
	sem_destroy(&m_sema);
}

void Threading::KernelSemaphore::Post()
{
	sem_post(&m_sema);
}

void Threading::KernelSemaphore::Wait()
{
	while (sem_wait(&m_sema) != 0)
		;
}

bool Threading::KernelSemaphore::TryWait()
{
	return (sem_trywait(&m_sema) == 0);
}
