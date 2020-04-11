// Copyright 2013 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

// Originally written by Sven Peter <sven@fail0verflow.com> for anergistic.

#pragma once

#include <string>
#include "common/common_types.h"
#include "core/hle/kernel/thread.h"

namespace GDBStub {

/// Breakpoint Method
enum class BreakpointType {
    None,    ///< None
    Execute, ///< Execution Breakpoint
    Read,    ///< Read Breakpoint
    Write,   ///< Write Breakpoint
    Access   ///< Access (R/W) Breakpoint
};

struct BreakpointAddress {
    VAddr address;
    BreakpointType type;
};

/**
 * Set the port the gdbstub should use to listen for connections.
 *
 * @param port Port to listen for connection
 */
void SetServerPort(u16 port);

/**
 * Starts or stops the server if possible.
 *
 * @param status Set the server to enabled or disabled.
 */
void ToggleServer(bool status);

/// Start the gdbstub server.
void Init();

/**
 * Defer initialization of the gdbstub to the first packet processing functions.
 * This avoids a case where the gdbstub thread is frozen after initialization
 * and fails to respond in time to packets.
 */
void DeferStart();

/// Stop gdbstub server.
void Shutdown(int status = 0);

/// Checks if the gdbstub server is enabled.
bool IsServerEnabled();

/// Returns true if there is an active socket connection.
bool IsConnected();

/// Register module.
void RegisterModule(std::string name, VAddr beg, VAddr end, bool add_elf_ext = true);

/**
 * Signal to the gdbstub server that it should halt CPU execution.
 *
 * @param is_memory_break If true, the break resulted from a memory breakpoint.
 */
void Break(bool is_memory_break = false);

/// Determine if there was a memory breakpoint.
bool IsMemoryBreak();

/// Read and handle packet from gdb client.
void HandlePacket();

/**
 * Get the nearest breakpoint of the specified type at the given address.
 *
 * @param addr Address to search from.
 * @param type Type of breakpoint.
 */
BreakpointAddress GetNextBreakpointFromAddress(VAddr addr, GDBStub::BreakpointType type);

/**
 * Check if a breakpoint of the specified type exists at the given address.
 *
 * @param addr Address of breakpoint.
 * @param type Type of breakpoint.
 */
bool CheckBreakpoint(VAddr addr, GDBStub::BreakpointType type);

/// Returns whether the CPU shall halt at the beginning of the next CPU loop.
bool GetCpuHaltFlag();

/**
 * Returns whether GDB is issuing a single-step command to the given thread.
 *
 * @param thread Thread the CPU is about to execute (or has just executed).
 */
bool GetThreadStepFlag(Kernel::Thread* thread);

/**
 * Send trap signal from thread back to the gdbstub server.
 *
 * @param thread Sending thread.
 * @param trap Trap no.
 */
void SendTrap(Kernel::Thread* thread, int trap);
} // namespace GDBStub
