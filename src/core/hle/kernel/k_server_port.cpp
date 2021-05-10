// Copyright 2021 yuzu emulator team
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <tuple>
#include "common/assert.h"
#include "core/hle/kernel/k_client_port.h"
#include "core/hle/kernel/k_port.h"
#include "core/hle/kernel/k_scheduler.h"
#include "core/hle/kernel/k_server_port.h"
#include "core/hle/kernel/k_server_session.h"
#include "core/hle/kernel/k_thread.h"
#include "core/hle/kernel/svc_results.h"

namespace Kernel {

KServerPort::KServerPort(KernelCore& kernel_) : KSynchronizationObject{kernel_} {}
KServerPort::~KServerPort() = default;

void KServerPort::Initialize(KPort* parent_, std::string&& name_) {
    // Set member variables.
    parent = parent_;
    name = std::move(name_);
}

bool KServerPort::IsLight() const {
    return this->GetParent()->IsLight();
}

void KServerPort::CleanupSessions() {
    // Ensure our preconditions are met.
    if (this->IsLight()) {
        UNIMPLEMENTED();
    }

    // Cleanup the session list.
    while (true) {
        // Get the last session in the list
        KServerSession* session = nullptr;
        {
            KScopedSchedulerLock sl{kernel};
            if (!session_list.empty()) {
                session = std::addressof(session_list.front());
                session_list.pop_front();
            }
        }

        // Close the session.
        if (session != nullptr) {
            session->Close();
        } else {
            break;
        }
    }
}

void KServerPort::Destroy() {
    // Note with our parent that we're closed.
    parent->OnServerClosed();

    // Perform necessary cleanup of our session lists.
    this->CleanupSessions();

    // Close our reference to our parent.
    parent->Close();
}

bool KServerPort::IsSignaled() const {
    if (this->IsLight()) {
        UNIMPLEMENTED();
        return false;
    } else {
        return !session_list.empty();
    }
}

void KServerPort::EnqueueSession(KServerSession* session) {
    ASSERT(!this->IsLight());

    KScopedSchedulerLock sl{kernel};

    // Add the session to our queue.
    session_list.push_back(*session);
    if (session_list.size() == 1) {
        this->NotifyAvailable();
    }
}

KServerSession* KServerPort::AcceptSession() {
    ASSERT(!this->IsLight());

    KScopedSchedulerLock sl{kernel};

    // Return the first session in the list.
    if (session_list.empty()) {
        return nullptr;
    }

    KServerSession* session = std::addressof(session_list.front());
    session_list.pop_front();
    return session;
}

} // namespace Kernel
