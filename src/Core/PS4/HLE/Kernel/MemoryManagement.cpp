#include "MemoryManagement.h"

#include <bit>
#include <magic_enum.hpp>

#include "../../../../Debug.h"
#include "../../../../Util/Log.h"
#include "../../../../Util/Singleton.h"
#include "../ErrorCodes.h"
#include "../Libs.h"
#include "MemMngCodes.h"
#include "Objects/physical_memory.h"

namespace HLE::Libs::LibKernel::MemoryManagement {

bool isPowerOfTwo(u64 n) { return std::popcount(n) == 1; }

bool is16KBAligned(u64 n) { return ((n % (16ull * 1024) == 0)); }

u64 PS4_SYSV_ABI sceKernelGetDirectMemorySize() {
    PRINT_FUNCTION_NAME();
    return SCE_KERNEL_MAIN_DMEM_SIZE;
}

int PS4_SYSV_ABI sceKernelAllocateDirectMemory(s64 searchStart, s64 searchEnd, u64 len, u64 alignment, int memoryType, s64* physAddrOut) {
    PRINT_FUNCTION_NAME();

    if (searchStart < 0 || searchEnd <= searchStart) {
        // TODO debug logging
        return SCE_KERNEL_ERROR_EINVAL;
    }
    bool isInRange = (searchStart < len && searchEnd > len);
    if (len <= 0 || !is16KBAligned(len) || !isInRange) {
        // TODO debug logging
        return SCE_KERNEL_ERROR_EINVAL;
    }
    if ((alignment != 0 || is16KBAligned(alignment)) && !isPowerOfTwo(alignment)) {
        // TODO debug logging
        return SCE_KERNEL_ERROR_EINVAL;
    }
    if (physAddrOut == nullptr) {
        // TODO debug logging
        return SCE_KERNEL_ERROR_EINVAL;
    }
    auto memtype = magic_enum::enum_cast<memory_types>(memoryType);

    LOG_INFO_IF(true, "search_start = {:#018x}\n", searchStart);
    LOG_INFO_IF(true, "search_end   = {:#018x}\n", searchEnd);
    LOG_INFO_IF(true, "len          = {:#018x}\n", len);
    LOG_INFO_IF(true, "alignment    = {:#018x}\n", alignment);
    LOG_INFO_IF(true, "memory_type  = {}\n", magic_enum::enum_name(memtype.value()));

    u64 physical_addr = 0;
    auto* physical_memory = Singleton<HLE::Kernel::Objects::PhysicalMemory>::Instance();
    if (!physical_memory->Alloc(searchStart, searchEnd, len, alignment, &physical_addr, memoryType)) {
        // TODO debug logging
        return SCE_KERNEL_ERROR_EAGAIN;
    }
    *physAddrOut = static_cast<s64>(physical_addr);
    LOG_INFO_IF(true, "physAddrOut    = {:#018x}\n", physical_addr);
    return SCE_OK;
}

}  // namespace HLE::Libs::LibKernel::MemoryManagement