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

#include "scope_guard.hpp"
#include "hos_except.hpp"
#include <stdexcept>

int main() {
    consoleInit(nullptr);

    PadState pad;
    padConfigureInput(8, HidNpadStyleSet_NpadStandard);
    padInitializeAny(&pad);

    try {
        /* Initialize Nintendo Shell. */
        R_THROW(nsInitialize());
        auto ns_guard = scope_exit(nsExit);

        /* Initialize Network Interface Manager. */
        R_THROW(nifmInitialize(NifmServiceType_System));
        auto nifm_guard = scope_exit(nifmExit);

        /* Request network connection. */
        NifmRequest request;
        R_THROW(nifmCreateRequest(&request, true));
        auto req_guard = scope_exit([&] { nifmRequestClose(&request); });

        /* Submit request. */
        R_THROW(nifmRequestSubmitAndWait(&request));

        /* Confirm network availability. */
        if (!nifmIsAnyInternetRequestAccepted(nifmGetClientId()))
            throw std::runtime_error("Network connectivity couldn't be established.");

        /* Load Version list. */
        const auto version_list = VersionList();

        /* Request update for outdated titles. */
        const auto updated = version_list.UpdateApplications();

        printf("Updated %u Applications!\n", updated);
    } catch (std::exception &e) {
        printf("%s\n", e.what());
    }

    printf("Press + to exit.\n");
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
