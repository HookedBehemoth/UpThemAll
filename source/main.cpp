/**
 * Copyright (c) 2020 Luis Scheurenbrand
 * 
 * All Rights Reserved
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#include "version_list.hpp"

#include <switch.h>
#include <cstdio>
#include <cstring>

int main() {
    consoleInit(nullptr);

    PadState pad;
    padConfigureInput(8, HidNpadStyleTag_NpadHandheld);
    padInitializeAny(&pad);
    hidSetNpadHandheldActivationMode(HidNpadHandheldActivationMode_Single);

    /* Initiate Services. */
    nsInitialize();
    nifmInitialize(NifmServiceType_System);

    /* Make network request. */
    NifmRequest request;
    if (R_SUCCEEDED(nifmCreateRequest(&request, true))) {
        nifmRequestSubmitAndWait(&request);
        
        /* Confirm network availability. */
        if (nifmIsAnyInternetRequestAccepted(nifmGetClientId())) {
            try {
                printf("Receiving version list.\n");
                consoleUpdate(nullptr);
                const auto version_list = VersionList();
                
                printf("Updating Applciations\n");
                consoleUpdate(nullptr);
                version_list.UpdateApplications();
            } catch (std::exception &e) {
                printf("Failed to load version list: %s\n", e.what());
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
        padUpdate(&pad);
        u64 kDown = padGetButtonsDown(&pad);

        if (kDown & KEY_PLUS)
            break;
    }

    consoleExit(nullptr);

    return 0;
}
