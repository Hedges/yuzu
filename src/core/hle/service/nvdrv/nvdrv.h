// Copyright 2018 yuzu emulator team
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <memory>
#include <unordered_map>
#include <vector>
#include "common/common_types.h"
#include "core/hle/kernel/writable_event.h"
#include "core/hle/service/nvdrv/nvdata.h"
#include "core/hle/service/service.h"

namespace Core {
class System;
}

namespace Service::NVFlinger {
class NVFlinger;
}

namespace Service::Nvidia {

namespace Devices {
class nvdevice;
}

struct EventsInterface {
    u64 events_mask{};
    std::array<Kernel::EventPair, MaxNvEvents> events;
    std::array<EventState, MaxNvEvents> status{};
    std::array<bool, MaxNvEvents> registered{};
    std::array<u32, MaxNvEvents> assigned_syncpt{};
    std::array<u32, MaxNvEvents> assigned_value{};
    u32 GetFreeEvent() {
        u64 mask = events_mask;
        for (u32 i = 0; i < MaxNvEvents; i++) {
            const bool is_free = (mask & 0x1) == 0;
            if (is_free) {
                if (status[i] == EventState::Registered || status[i] == EventState::Free) {
                    return i;
                }
            }
            mask = mask >> 1;
        }
        return 0xFFFFFFFF;
    }
    void SetEventStatus(const u32 event_id, EventState new_status) {
        EventState old_status = status[event_id];
        if (old_status == new_status)
            return;
        status[event_id] = new_status;
        if (new_status == EventState::Registered) {
            registered[event_id] = true;
        }
        if (new_status == EventState::Waiting || new_status == EventState::Busy) {
            events_mask |= (1 << event_id);
        }
    }
    void RegisterEvent(const u32 event_id) {
        registered[event_id] = true;
        if (status[event_id] == EventState::Free) {
            status[event_id] = EventState::Registered;
        }
    }
    void UnregisterEvent(const u32 event_id) {
        registered[event_id] = false;
        if (status[event_id] == EventState::Registered) {
            status[event_id] = EventState::Free;
        }
    }
    void LiberateEvent(const u32 event_id) {
        status[event_id] = registered[event_id] ? EventState::Registered : EventState::Free;
        events_mask &= ~(1 << event_id);
    }
};

class Module final {
public:
    Module(Core::System& system);
    ~Module();

    /// Returns a pointer to one of the available devices, identified by its name.
    template <typename T>
    std::shared_ptr<T> GetDevice(const std::string& name) {
        auto itr = devices.find(name);
        if (itr == devices.end())
            return nullptr;
        return std::static_pointer_cast<T>(itr->second);
    }

    /// Opens a device node and returns a file descriptor to it.
    u32 Open(const std::string& device_name);
    /// Sends an ioctl command to the specified file descriptor.
    u32 Ioctl(u32 fd, u32 command, const std::vector<u8>& input, std::vector<u8>& output);
    /// Closes a device file descriptor and returns operation success.
    ResultCode Close(u32 fd);

    void SignalEvent(const u32 event_id);

    Kernel::SharedPtr<Kernel::ReadableEvent> GetEvent(const u32 event_id);

private:
    /// Id to use for the next open file descriptor.
    u32 next_fd = 1;

    /// Mapping of file descriptors to the devices they reference.
    std::unordered_map<u32, std::shared_ptr<Devices::nvdevice>> open_files;

    /// Mapping of device node names to their implementation.
    std::unordered_map<std::string, std::shared_ptr<Devices::nvdevice>> devices;

    EventsInterface events_interface;
};

/// Registers all NVDRV services with the specified service manager.
void InstallInterfaces(SM::ServiceManager& service_manager, NVFlinger::NVFlinger& nvflinger,
                       Core::System& system);

} // namespace Service::Nvidia
