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

static Result _nsVersionNoInBufOut(void* buffer, size_t size, u32 *out, u32 cmd_id) {
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

static Result _nsPushVersion(u64 application_id, u32 version, u32 cmd_id) {
    Result rc=0;
    Service srv={};
    rc = nsGetApplicationVersionInterface(&srv);

    const struct {
        u32 version;
        u64 application_id;
    } in = { version, application_id };

    if (R_SUCCEEDED(rc)) rc = serviceDispatchIn(&srv, cmd_id, in);

    serviceClose(&srv);
    return rc;
}

Result nsGetLaunchRequiredVersion(u64 application_id, u32 *version) {
    Result rc=0;
    Service srv={};
    rc = nsGetApplicationVersionInterface(&srv);

    if (R_SUCCEEDED(rc)) rc = serviceDispatchInOut(&srv, 0, application_id, *version);

    serviceClose(&srv);
    return rc;
}

Result nsUpgradeLaunchRequiredVersion(u64 application_id, u32 version) {
    return _nsPushVersion(application_id, version, 1);
}

Result nsUpdateVersionList(AvmVersionListEntry *buffer, size_t count) {
    Result rc=0;
    Service srv={};
    rc = nsGetApplicationVersionInterface(&srv);

    if (R_SUCCEEDED(rc)) rc = serviceDispatch(&srv, 35,
        .buffer_attrs = { SfBufferAttr_HipcMapAlias | SfBufferAttr_In },
        .buffers = { { buffer, count * sizeof(*buffer) } },
    );

    serviceClose(&srv);
    return rc;
}

Result nsPushLaunchVersion(u64 application_id, u32 version) {
    return _nsPushVersion(application_id, version, 36);
}

Result nsListRequiredVersion(AvmRequiredVersionEntry *buffer, size_t count, u32 *out) {
    return _nsVersionNoInBufOut(buffer, count * sizeof(*buffer), out, 37);
}

Result nsRequestVersionList() {
    return _nsVersionNoInNoOut(800);
}

Result nsListVersionList(AvmVersionListEntry *buffer, size_t count, u32 *out) {
    return _nsVersionNoInBufOut(buffer, count * sizeof(*buffer), out, 801);
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
    return _nsVersionNoInBufOut(unk, size, out, 1001);
}
