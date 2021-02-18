/**
 * @file ns.h
 * @brief NS services IPC wrapper.
 * @author yellows8
 * @copyright libnx Authors
 */
#pragma once
#include <switch.h>

typedef enum {
    NsApplicationRecordType_Running         = 0x0,
    NsApplicationRecordType_Installed       = 0x3,
    NsApplicationRecordType_Downloading     = 0x4,
    NsApplicationRecordType_GamecardMissing = 0x5,
    NsApplicationRecordType_Downloaded      = 0x6,
    NsApplicationRecordType_Updated         = 0xa,
    NsApplicationRecordType_Archived        = 0xb,
    NsApplicationRecordType_AlreadyStarted  = 0x10,
} NsApplicationRecordType;

Result nsRequestVersionList(void);
Result nsListVersionList(AvmVersionListEntry *buffer, size_t count, u32 *out);
Result nsRequestVersionListData(AsyncValue *a);
Result nsPerformAutoUpdate(void);
Result nsListAutoUpdateSchedule(void* unk, size_t size, u32 *out);
