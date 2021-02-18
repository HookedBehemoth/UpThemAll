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

#include <stdexcept>

#include "gfx.hpp"
#include <imgui.h>

#ifdef DEBUG
#include <unistd.h>
static int nxlink = -1;
#endif

extern "C" void userAppInit() {
#ifdef DEBUG
    socketInitializeDefault();
    nxlink = nxlinkStdio();
#endif
    appletLockExit();

    /* Initialize Network Interface Manager. */
    nifmInitialize(NifmServiceType_System);
    plInitialize(PlServiceType_User);
    nsInitialize();
    avmInitialize();

    romfsInit();
    hidInitializeTouchScreen();
}

extern "C" void userAppExit(void) {
#ifdef DEBUG
    ::close(nxlink);
    socketExit();
#endif
    romfsExit();

    avmExit();
    nsExit();
    plExit();
    nifmExit();

    appletUnlockExit();
}

int main() {
    auto version_list = VersionList();

    NifmInternetConnectionType contype;
    u32 wifiStrength=0;
    NifmInternetConnectionStatus connectionStatus;

    std::atomic_bool has_internet = R_SUCCEEDED(nifmGetInternetConnectionStatus(&contype, &wifiStrength, &connectionStatus));
    std::atomic_bool join = false;
    auto status_thread = std::thread([&] {
        /* Make network request. */
        NifmRequest request;
        nifmCreateRequest(&request, true);

        /* Submit request. */
        nifmRequestSubmitAndWait(&request);

        do {
            /* Confirm network availability. */
            has_internet = R_SUCCEEDED(nifmGetInternetConnectionStatus(&contype, &wifiStrength, &connectionStatus));
        } while (svcSleepThread(100'000'000), !join);

        nifmRequestClose(&request);
    });

    bool net_warn = !has_internet;
    bool nuke_warn = false;

    if (!fz::gfx::init())
        return EXIT_FAILURE;

    while (fz::gfx::loop()) {
        ImGui::SetNextWindowPos(ImVec2{40.f, 22.5f}, ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(1200, 675), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("UpThemAll", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize)) {
            if (ImGui::Button("Update them all")) {
                version_list.UpdateAllApplications();
            }
            ImGui::SameLine();
            if (ImGui::Button("Refresh List")) {
                std::printf("refreshing\n");
                version_list.Refresh();
            }
            ImGui::SameLine();
            if (ImGui::Button("Clear Version List")) {
                nuke_warn = true;
            }

            version_list.List(has_internet);
            ImGui::End();
        }

        ImGui::SetNextWindowPos(ImVec2{400.f, 300.f}, ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(480.f, 120.f), ImGuiCond_FirstUseEver);
        if (net_warn && ImGui::Begin("No Internet", &net_warn, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize)) {
            ImGui::Text("No internet connection available.");
            // if (R_FAILED(rc))
            //     ImGui::Text("Result code: 2%03X-%04X, (0x%x)", R_MODULE(rc), R_DESCRIPTION(rc), R_VALUE(rc));
            ImGui::End();
        }

        ImGui::SetNextWindowPos(ImVec2{300.f, 250.f}, ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(680.f, 220.f), ImGuiCond_FirstUseEver);
        if (nuke_warn && ImGui::Begin("Clear Warning", &nuke_warn, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize)) {
            ImGui::Text("List can be reinstated by official background processes.\n\nDo you really want to proceed?");
            if (ImGui::Button("Ok")) {
                version_list.Nuke();
                nuke_warn = false;
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel")) {
                nuke_warn = false;
            }
            ImGui::End();
        }

        fz::gfx::render();
    }

    fz::gfx::exit();

    join = true;
    status_thread.join();

    return EXIT_SUCCESS;
}
