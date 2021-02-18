/*
 * Copyright (c) 2021 Luis Scheurenbrand
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
    if (!fz::gfx::init())
        return EXIT_FAILURE;

    auto version_list = VersionList();

    NifmInternetConnectionType contype;
    u32 wifiStrength=0;
    NifmInternetConnectionStatus connectionStatus;

    bool has_internet = R_SUCCEEDED(nifmGetInternetConnectionStatus(&contype, &wifiStrength, &connectionStatus));
    bool net_warn = !has_internet;
    bool join = false;

    auto status_thread = std::thread([&] {
        /* Make network request. */
        NifmRequest request;
        nifmCreateRequest(&request, true);

        /* Submit request. */
        nifmRequestSubmitAndWait(&request);

        bool previous = has_internet;

        do {
            /* Confirm network availability. */
            has_internet = R_SUCCEEDED(nifmGetInternetConnectionStatus(&contype, &wifiStrength, &connectionStatus));
            if (previous != has_internet) {
                net_warn = !has_internet;
                previous = has_internet;
            }
        } while (svcSleepThread(100'000'000), !join);

        nifmRequestClose(&request);
    });

    while (fz::gfx::loop()) {
        ImGui::SetNextWindowPos(ImVec2{40.f, 22.5f}, ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2{1200.f, 675.f}, ImGuiCond_FirstUseEver);
        if (ImGui::Begin("UpThemAll", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBringToFrontOnFocus)) {
            if (ImGui::Button("Update them all")) {
                version_list.UpdateAllApplications();
            }
            ImGui::SameLine();
            if (ImGui::Button("Refresh List")) {
                version_list.Refresh();
            }
            ImGui::SameLine();
            if (ImGui::Button("Clear Version List")) {
                version_list.Nuke();
            }

            version_list.List(has_internet);
            ImGui::End();
        }

        ImGui::SetNextWindowPos(ImVec2{400.f, 280.f}, ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2{480.f, 160.f}, ImGuiCond_FirstUseEver);
        if (net_warn && ImGui::Begin("No Internet", &net_warn, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize)) {
            ImGui::Text("No internet connection available.\n\nUpdate functionality disabled.");
            if (ImGui::Button("Ok"))
                net_warn = false;
            ImGui::End();
        }

        fz::gfx::render();
    }

    fz::gfx::exit();

    join = true;
    status_thread.join();

    return EXIT_SUCCESS;
}
