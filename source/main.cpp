#include <switch.h>
#include <cstdio>
#include <cstring>

static Result _nsCmdInU64OutAsyncResult(Service* srv, AsyncResult *a, u64 inval, u32 cmd_id) {
    memset(a, 0, sizeof(*a));
    Handle event = INVALID_HANDLE;
    Result rc = serviceDispatchIn(srv, cmd_id, inval,
        .out_num_objects = 1,
        .out_objects = &a->s,
        .out_handle_attrs = { SfOutHandleAttr_HipcCopy },
        .out_handles = &event,
    );

    if (R_SUCCEEDED(rc))
        eventLoadRemote(&a->event, event, false);

    return rc;
}

static Result nsRequestUpdateApplication2Shim(AsyncResult *a, u64 application_id) {
    Service srv={0};
    Result rc=0;
    rc = nsGetApplicationManagerInterface(&srv);

    if (R_SUCCEEDED(rc)) rc = _nsCmdInU64OutAsyncResult(&srv, a, application_id, 85);

    serviceClose(&srv);
    return rc;
}

int main()
{
    consoleInit(nullptr);

    nsInitialize();

    s32 offset=0, count=0;
    NsApplicationRecord record;
    while (R_SUCCEEDED(nsListApplicationRecord(&record, 1, offset++, &count)) && count != 0) {
        /* Skip archived applications. */
        if (record.type == 0xb)
            continue;

        printf("Checking %016lX...", record.application_id);
        consoleUpdate(nullptr);

        /* Request update. */
        AsyncResult async;
        if (R_SUCCEEDED(nsRequestUpdateApplication2Shim(&async, record.application_id))) {
            
            /* Wait for result. */
            asyncResultWait(&async, UINT64_MAX);
            if (R_SUCCEEDED(asyncResultGet(&async))) {
                static NsApplicationControlData nacp;
                NacpLanguageEntry *entry = &nacp.nacp.lang[0];
                u64 size=0;
                nsGetApplicationControlData(NsApplicationControlSource_Storage, record.application_id, &nacp, sizeof(nacp), &size);
                nacpGetLanguageEntry(&nacp.nacp, &entry);
                printf("\nUpdating: %s\n", entry->name);
            } else {
                printf(" No update found\n");
            }
            asyncResultClose(&async);
        } else {
            printf(" Update in progress\n");
        }
    }

    printf("Done!\nPress + to exit.\n");
    consoleUpdate(nullptr);

    nsExit();

    while (appletMainLoop()) {
        hidScanInput();
        u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);

        if (kDown & KEY_PLUS)
            break;
    }

    consoleExit(nullptr);

    return 0;
}
