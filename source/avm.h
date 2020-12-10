/**
 * @file avm.h
 * @author Luis Scheurenbrand
 * @copyright libnx Authors
 */
#pragma once
#include <switch/types.h>

typedef struct {
    u64 app_id;
    u32 version;
    u32 required;
} AvmVersionListEntry;
