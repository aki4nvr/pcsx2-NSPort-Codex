// SPDX-FileCopyrightText: 2002-2026 PCSX2 Dev Team
// SPDX-License-Identifier: GPL-3.0+

#include "common/HostSys.h"
#include "common/Threading.h"

#include <chrono>
#include <ctime>
#include <thread>

u64 GetPhysicalMemory()
{
	return 0;
}

u64 GetAvailablePhysicalMemory()
{
	return 0;
}

u64 GetTickFrequency()
{
	return 1000000000ULL;
}

u64 GetCPUTicks()
{
	struct timespec ts;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return (static_cast<u64>(ts.tv_sec) * 1000000000ULL) + ts.tv_nsec;
}

std::string GetOSVersionString()
{
	return "Nintendo Switch";
}

bool Common::InhibitScreensaver(bool inhibit)
{
	(void)inhibit;
	return false;
}

void Common::SetMousePosition(int x, int y)
{
	(void)x;
	(void)y;
}

bool Common::AttachMousePositionCb(std::function<void(int, int)> cb)
{
	(void)cb;
	return false;
}

void Common::DetachMousePositionCb()
{
}

bool Common::PlaySoundAsync(const char* path)
{
	(void)path;
	return false;
}

void Threading::Sleep(int ms)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

void Threading::SleepUntil(u64 ticks)
{
	const u64 now = GetCPUTicks();
	if (ticks <= now)
		return;

	const u64 delta_ns = ticks - now;
	std::this_thread::sleep_for(std::chrono::nanoseconds(delta_ns));
}
