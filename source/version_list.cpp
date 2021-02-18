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

#include <imgui.h>
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
    if (R_FAILED(avmGetLaunchRequiredVersion(application_id, &version)))
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

void VersionList::UpdateSynchronous(ApplicationId application_id) const noexcept {
    /* Request update. */
    AsyncResult async;
    Result rc = nsRequestUpdateApplication2(&async, application_id);
    if (R_SUCCEEDED(rc)) {
        /* Wait for result. */
        const Result rc = asyncResultGet(&async);
        if (R_FAILED(rc)) {
            printf(" Update failed: 0x%x\n", rc);
        }
        asyncResultClose(&async);
    } else {
        printf(" Update in progress\n");
        printf("0x%x\n", rc);
    }
}

void VersionList::UpdateAllApplications() noexcept {
    for (const auto &[application_id, pair]: this->available) {
        UpdateSynchronous(application_id);
    }
    this->available.clear();
}

void VersionList::List(bool has_internet) noexcept {
    static DkResHandle handle = 0;

    // Left
    static ApplicationId selected = 0;
    {
        ImGui::BeginChild("left pane", ImVec2(750, 0), true);
        for (const auto &[application_id, pair]: this->available) {
            const auto &[name, required] = pair;
            if (required)
                ImGui::PushStyleColor(ImGuiCol_Text, {0.94f, 0.33f, 0.31f, 1.f});
            if (ImGui::Selectable(name.c_str(), selected == application_id)) {
                if (selected != application_id) {
                    auto jpeg = GetThumbnail(application_id);
                    /* Decode image to */
                    int w,h;
                    auto data = stbi_load_from_memory(jpeg, 0x20000, &w, &h, nullptr, 4);
                    handle = fz::gfx::create_texture(data, w, h, 1, 1);
                    free(data);
                    selected = application_id;
                }
            }
            if (required)
                ImGui::PopStyleColor();
        }
        ImGui::EndChild();
    }
    ImGui::SameLine();

    // Right
    if (selected != 0) {
        if (!this->available.contains(selected))
            selected = 0;

        auto &[name, required] = this->available.at(selected);

        ImGui::BeginGroup();
        ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing())); // Leave room for 1 line below us
        ImGui::Text(name.c_str());
        ImGui::Separator();
        if (handle != 0)
            ImGui::Image(reinterpret_cast<void *>(static_cast<std::uintptr_t>(handle)), { 256, 256 });
        ImGui::EndChild();

        if (has_internet && ImGui::Button("Update")) {
            printf("Updating: [%016lX]: %s\n", selected, GetApplicationName(selected));

            UpdateSynchronous(selected);

            this->available.erase(selected);
            selected = 0;
        }
        ImGui::SameLine();

        if (required && ImGui::Button("Remove")) {
            avmPushLaunchVersion(selected, 0);
            required = false;
        }
        ImGui::EndGroup();
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

        std::printf("Adding: %s\n", app_name);
        
        this->available[application_id] = { app_name, required > installed };
    }
}
