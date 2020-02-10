// Copyright 2018 yuzu emulator team
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "core/arm/exclusive_monitor.h"
#include "core/core.h"
#include "core/core_manager.h"
#include "core/core_timing.h"
#include "core/cpu_manager.h"
#include "core/gdbstub/gdbstub.h"
#include "core/settings.h"

namespace Core {

CpuManager::CpuManager(System& system) : system{system} {}
CpuManager::~CpuManager() = default;

void CpuManager::Initialize() {
    for (std::size_t index = 0; index < core_managers.size(); ++index) {
        core_managers[index] = std::make_unique<CoreManager>(system, index);
    }
}

void CpuManager::Shutdown() {
    for (auto& cpu_core : core_managers) {
        cpu_core.reset();
    }
}

CoreManager& CpuManager::GetCoreManager(std::size_t index) {
    return *core_managers.at(index);
}

const CoreManager& CpuManager::GetCoreManager(std::size_t index) const {
    return *core_managers.at(index);
}

CoreManager& CpuManager::GetCurrentCoreManager() {
    // Otherwise, use single-threaded mode active_core variable
    return *core_managers[active_core];
}

const CoreManager& CpuManager::GetCurrentCoreManager() const {
    // Otherwise, use single-threaded mode active_core variable
    return *core_managers[active_core];
}

void CpuManager::RunLoop(bool tight_loop) {
    if (GDBStub::IsServerEnabled()) {
        GDBStub::HandlePacket();
    }

    auto& core_timing = system.CoreTiming();
    core_timing.ResetRun();
    bool keep_running{};
    int num_loops = 0;
    const int max_loops = Settings::values.gdbstub_loops;
    do {
        keep_running = false;
        for (active_core = 0; active_core < NUM_CPU_CORES; ++active_core) {
            core_timing.SwitchContext(active_core);
            if (core_timing.CanCurrentContextRun()) {
                core_managers[active_core]->RunLoop(tight_loop);
            }
            keep_running |= core_timing.CanCurrentContextRun();
        }
        if (GDBStub::IsConnected()) {
            num_loops++;
        }
    } while (keep_running && (num_loops < max_loops));
}

} // namespace Core
