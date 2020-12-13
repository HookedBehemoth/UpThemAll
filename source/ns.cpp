#include "ns.h"

#include <cstring>

static Result _nsVersionNoInNoOut(u32 cmd_id) {
    Result rc=0;
    Service srv={};
    rc = nsGetApplicationVersionInterface(&srv);

    if (R_SUCCEEDED(rc)) rc = serviceDispatch(&srv, cmd_id);
    
    serviceClose(&srv);
    return rc;
}

static Result _nsVersionNoInBufOut(u32 cmd_id, void* buffer, size_t size, u32 *out) {
    Result rc=0;
    Service srv={};
    rc = nsGetApplicationVersionInterface(&srv);

    if (R_SUCCEEDED(rc)) rc = serviceDispatchOut(&srv, cmd_id, *out,
        .buffer_attrs = { SfBufferAttr_HipcMapAlias | SfBufferAttr_Out },
        .buffers = { { buffer, size } },
    );

    serviceClose(&srv);
    return rc;
}

Result nsRequestVersionList() {
    return _nsVersionNoInNoOut(800);
}

Result nsListVersionList(AvmVersionListEntry *buffer, size_t count, u32 *out) {
    return _nsVersionNoInBufOut(801, buffer, count * sizeof(*buffer), out);
}

Result nsRequestVersionListData(AsyncValue *a) {
    Result rc=0;
    Service srv={};
    
    rc = nsGetApplicationVersionInterface(&srv);

    memset(a, 0, sizeof(*a));
    Handle event = INVALID_HANDLE;
    if (R_SUCCEEDED(rc)) rc = serviceDispatch(&srv, 802,
        .out_num_objects = 1,
        .out_objects = &a->s,
        .out_handle_attrs = { SfOutHandleAttr_HipcCopy },
        .out_handles = &event,
    );

    if (R_SUCCEEDED(rc))
        eventLoadRemote(&a->event, event, false);

    serviceClose(&srv);
    return rc;
}

Result nsPerformAutoUpdate(void) {
    return _nsVersionNoInNoOut(1000);
}

Result nsListAutoUpdateSchedule(void* unk, size_t size, u32 *out) {
    return _nsVersionNoInBufOut(1001, unk, size, out);
}
