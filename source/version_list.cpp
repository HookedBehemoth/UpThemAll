/*
 * Copyright (c) 2020-2021 Luis Scheurenbrand
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

#include "gfx.hpp"
#include <stb_image.h>

#include "ns.h"

VersionList::VersionList() {
    this->Refresh();
}

void VersionList::Refresh() {
    this->impl.resize(0x4000);
    u32 count=0;
    nsListVersionList(this->impl.data(), this->impl.size(), &count);
    this->impl.resize(count);

    this->UpdateAvailable();
}

/* Get installed version. 0 if no patch is installed. */
u32 VersionList::GetInstalledVersion(ApplicationId application_id) const noexcept {
    s32 index=0, count=0;
    NsApplicationContentMetaStatus meta={};
    while (R_SUCCEEDED(nsListApplicationContentMetaStatus(application_id, index++, &meta, 1, &count)) && count != 0) {
        if (meta.meta_type == NcmContentMetaType_Patch) {
            return meta.version;
        }
    }
    return 0;
}

/* Get highest available version. 0 if not found. */
u32 VersionList::GetAvailableVersion(ApplicationId application_id) const noexcept {
    u64 patch_id = application_id | 0x800;
    const auto it = std::find_if(std::cbegin(impl), std::cend(impl), [&](const auto& entry) { return patch_id == entry.application_id; });
    
    if (it != std::end(impl)) {
        return it->version;
    } else {
        return 0;
    }
}

u32 VersionList::GetLaunchRequiredVersion(ApplicationId application_id) const noexcept {
    u32 version = 0;
    if (R_FAILED(nsGetLaunchRequiredVersion(application_id, &version)))
        return 0;
    return version;
}

static NsApplicationControlData nacp={};

const char* VersionList::GetApplicationName(ApplicationId application_id) const noexcept {
    NacpLanguageEntry *entry = &nacp.nacp.lang[0];
    u64 size=0;
    nsGetApplicationControlData(NsApplicationControlSource_Storage, application_id, &nacp, sizeof(nacp), &size);
    nacpGetLanguageEntry(&nacp.nacp, &entry);
    return entry->name;
}

const u8* VersionList::GetThumbnail(ApplicationId application_id) const noexcept {
    u64 size=0;
    nsGetApplicationControlData(NsApplicationControlSource_Storage, application_id, &nacp, sizeof(nacp), &size);
    return nacp.icon;
}

bool VersionList::UpdateSynchronous(ApplicationId application_id) const noexcept {
    this->log.appendf("Updating: [%016lX]: %s\n", application_id, GetApplicationName(application_id));

    /* Request update. */
    AsyncResult async;
    Result rc = nsRequestUpdateApplication2(&async, application_id);
    if (R_SUCCEEDED(rc)) {
        /* Wait for result. */
        rc = asyncResultGet(&async);
        asyncResultClose(&async);
    }

    if (R_FAILED(rc))
        this->log.appendf("Update failed: 0x%x\n", rc);

    return R_SUCCEEDED(rc);
}

void VersionList::UpdateAllApplications() noexcept {
    for (const auto &[application_id, pair]: this->available) {
        UpdateSynchronous(application_id);
    }

    this->available.clear();
    this->selected = 0;
}

void VersionList::List(bool has_internet) noexcept {
    static DkResHandle handle = 0;

    if (ImGui::BeginChild("left pane", ImVec2{750.f, 400.f}, true)) {
        for (const auto &[application_id, pair]: this->available) {
            const auto &[name, required] = pair;
            if (required)
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{0.94f, 0.33f, 0.31f, 1.f});
            if (ImGui::Selectable(name.c_str(), this->selected == application_id)) {
                if (this->selected != application_id) {
                    auto jpeg = GetThumbnail(application_id);
                    /* Decode image to */
                    int w,h;
                    auto data = stbi_load_from_memory(jpeg, 0x20000, &w, &h, nullptr, 4);
                    handle = fz::gfx::create_texture(data, w, h, 1, 1);
                    free(data);
                    this->selected = application_id;
                }
            }
            if (required)
                ImGui::PopStyleColor();
        }
        ImGui::EndChild();
    }
    ImGui::SameLine();

    {
        ImGui::BeginGroup();
        if (this->selected != 0) {
            if (!this->available.contains(this->selected))
                this->selected = 0;

            auto &[name, required] = this->available.at(this->selected);

            ImGui::BeginChild("item view", ImVec2{0.f, 400.f - ImGui::GetFrameHeightWithSpacing()});
            ImGui::Text(name.c_str());
            ImGui::Separator();
            if (handle != 0) {
                static auto ImageSize = ImVec2{256.f, 256.f};
                ImGui::SetCursorPos((ImGui::GetWindowSize() - ImageSize) * 0.5f);
                ImGui::Image(reinterpret_cast<void *>(static_cast<std::uintptr_t>(handle)), ImageSize);
            }
            ImGui::EndChild();

            if (has_internet && ImGui::Button("Update")) {
                if (UpdateSynchronous(this->selected)) {
                    this->available.erase(this->selected);
                    this->selected = 0;
                }
            }
            
            if (has_internet && required)
                ImGui::SameLine();

            if (required && ImGui::Button("Reset Launch Version")) {
                this->log.appendf("Resetting launch required version for %s [%016lX]", name.c_str(), this->selected);
                nsPushLaunchVersion(this->selected, 0);
                required = false;
            }
        }

        ImGui::EndGroup();
    }

    if (ImGui::BeginChild("Log", ImVec2{0.f, 0.f}, true)) {
        ImGui::TextUnformatted(this->log.begin(), this->log.end());
        ImGui::EndChild();
    }
}

void VersionList::Nuke() noexcept {
    AvmVersionListImporter importer={};
    avmGetVersionListImporter(&importer);
    avmVersionListImporterSetTimestamp(&importer, 1609838100);
    avmVersionListImporterSetData(&importer, nullptr, 0);
    avmVersionListImporterFlush(&importer);
    avmVersionListImporterClose(&importer);

    Refresh();
}

void VersionList::UpdateAvailable() {
    this->available.clear();
    this->selected = 0;

    /* Iterate over installed applications. */
    s32 offset=0, count=0;
    NsApplicationRecord record;
    while (R_SUCCEEDED(nsListApplicationRecord(&record, 1, offset++, &count)) && count != 0) {
        /* Skip archived and downloading applications. */
        if (record.type == NsApplicationRecordType_Archived || record.type == NsApplicationRecordType_Downloading)
            continue;

        const u64 application_id = record.application_id;
        
        const u32 installed = GetInstalledVersion(application_id);
        const u32 available = GetAvailableVersion(application_id);
        const u32 required = GetLaunchRequiredVersion(application_id);

        /* Check if latest version is already installed. */
        if (installed >= available && installed >= required)
            continue;

        const auto app_name = GetApplicationName(application_id);

        this->log.appendf("Adding: %s, installed: %d, available: %d\n", app_name, installed, available);

        this->available[application_id] = { app_name, required > installed };
    }
}
