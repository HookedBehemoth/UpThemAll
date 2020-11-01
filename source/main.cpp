#include <switch.h>
#include <cstdio>
#include <cstring>

int main()
{
    consoleInit(nullptr);

    /* Initiate Services. */
    nsInitialize();
    nifmInitialize(NifmServiceType_System);

    /* Make network request. */
    NifmRequest request;
    if (R_SUCCEEDED(nifmCreateRequest(&request, true))) {
        nifmRequestSubmitAndWait(&request);
        
        /* Confirm network availability. */
        if (nifmIsAnyInternetRequestAccepted(nifmGetClientId())) {

            /* Iterate over installed applications. */
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
                if (R_SUCCEEDED(nsRequestUpdateApplication2(&async, record.application_id))) {
                    
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
        } else {
            printf("Network connectivity couldn't be established.\nAborting!\n");
        }
        
        /* Close network request. */
        nifmRequestClose(&request);
    } else {
        printf("Failed to make a network request.\nAborting!\n");
    }

    /* Exit services. */
    nifmExit();
    nsExit();

    printf("Done!\nPress + to exit.\n");
    consoleUpdate(nullptr);

    while (appletMainLoop()) {
        hidScanInput();
        u64 kDown = hidKeysDown(CONTROLLER_P1_AUTO);

        if (kDown & KEY_PLUS)
            break;
    }

    consoleExit(nullptr);

    return 0;
}
