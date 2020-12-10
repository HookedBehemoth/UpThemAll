/**
 * @file ns.h
 * @brief NS services IPC wrapper.
 * @author yellows8
 * @copyright libnx Authors
 */
#pragma once
#include <switch.h>

#include "avm.h"

Result nsRequestVersionList();
Result nsListVersionList(AvmVersionListEntry *buffer, size_t count, u32 *out);
Result nsRequestVersionListData(AsyncValue *a);
