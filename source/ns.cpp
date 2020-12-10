#include "ns.h"

#include <cstring>

Result nsRequestVersionList() {
    Result rc=0;
    Service srv={};
    rc = nsGetApplicationVersionInterface(&srv);

    if (R_SUCCEEDED(rc)) rc = serviceDispatch(&srv, 800);
    
    serviceClose(&srv);
    return rc;
}

Result nsListVersionList(AvmVersionListEntry *buffer, size_t count, u32 *out) {
    Result rc=0;
    Service srv={};
    rc = nsGetApplicationVersionInterface(&srv);

    if (R_SUCCEEDED(rc)) rc = serviceDispatchOut(&srv, 801, *out,
        .buffer_attrs = { SfBufferAttr_HipcMapAlias | SfBufferAttr_Out },
        .buffers = { { buffer, count * sizeof(*buffer) } },
    );

    serviceClose(&srv);
    return rc;
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
