// SPDX-FileCopyrightText: 2002-2026 PCSX2 Dev Team
// SPDX-License-Identifier: GPL-3.0+

#include "common/HostSys.h"
#include "common/Assertions.h"
#include "common/BitUtils.h"
#include "common/Console.h"
#include "common/Error.h"

#include <cstdlib>
#include <cstring>

void* HostSys::Mmap(void* base, size_t size, const PageProtectionMode& mode)
{
	pxAssertMsg((size & (__pagesize - 1)) == 0, "Size is page aligned");

	if (mode.IsNone())
		return nullptr;

	void* ptr = nullptr;
	if (posix_memalign(&ptr, __pagesize, Common::AlignUpPow2(size, __pagesize)) != 0)
		return nullptr;

	if (base)
	{
		// We do not support fixed mappings yet; return nullptr to force fallback paths.
		std::free(ptr);
		return nullptr;
	}

	return ptr;
}

void HostSys::Munmap(void* base, size_t size)
{
	(void)size;
	if (base)
		std::free(base);
}

void HostSys::MemProtect(void* baseaddr, size_t size, const PageProtectionMode& mode)
{
	(void)baseaddr;
	(void)size;
	(void)mode;
}

std::string HostSys::GetFileMappingName(const char* prefix)
{
	return std::string(prefix) + "_switch";
}

void* HostSys::CreateSharedMemory(const char* name, size_t size)
{
	(void)name;
	(void)size;
	return nullptr;
}

void HostSys::DestroySharedMemory(void* ptr)
{
	(void)ptr;
}

void* HostSys::MapSharedMemory(void* handle, size_t offset, void* baseaddr, size_t size, const PageProtectionMode& mode)
{
	(void)handle;
	(void)offset;
	(void)baseaddr;
	(void)size;
	(void)mode;
	return nullptr;
}

void HostSys::UnmapSharedMemory(void* baseaddr, size_t size)
{
	(void)baseaddr;
	(void)size;
}

size_t HostSys::GetRuntimePageSize()
{
	return __pagesize;
}

size_t HostSys::GetRuntimeCacheLineSize()
{
	return __cachelinesize;
}

std::unique_ptr<SharedMemoryMappingArea> SharedMemoryMappingArea::Create(size_t size)
{
	(void)size;
	return nullptr;
}

SharedMemoryMappingArea::SharedMemoryMappingArea(u8* base_ptr, size_t size, size_t num_pages)
	: m_base_ptr(base_ptr)
	, m_size(size)
	, m_num_pages(num_pages)
{
}

SharedMemoryMappingArea::~SharedMemoryMappingArea()
{
}

u8* SharedMemoryMappingArea::Map(void* file_handle, size_t file_offset, void* map_base, size_t map_size, const PageProtectionMode& mode)
{
	(void)file_handle;
	(void)file_offset;
	(void)map_base;
	(void)map_size;
	(void)mode;
	return nullptr;
}

bool SharedMemoryMappingArea::Unmap(void* map_base, size_t map_size)
{
	(void)map_base;
	(void)map_size;
	return false;
}

namespace PageFaultHandler
{
	bool Install(Error* error)
	{
		Error::SetStringView(error, "Page fault handler is not implemented on Nintendo Switch yet.");
		return false;
	}
} // namespace PageFaultHandler

#ifdef _M_ARM64
void HostSys::FlushInstructionCache(void* address, u32 size)
{
	__builtin___clear_cache(reinterpret_cast<char*>(address), reinterpret_cast<char*>(address) + size);
}
#endif
