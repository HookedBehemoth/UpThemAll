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

#include "hos_except.hpp"
#include "ns.h"
#include "scope_guard.hpp"
#include "simdjson.h"

VersionList::VersionList() {
    impl.resize(0x4000);
    u32 count=0;
    R_THROW(nsListVersionList(impl.data(), impl.size(), &count));
    impl.resize(count);
}

void VersionList::FetchFromCDN() {
    /* Request version list. */
    AsyncValue async;
    R_THROW(nsRequestVersionListData(&async));
    scope_exit async_guard([&] { asyncValueClose(&async); });

    /* Wait for finish. */
    R_THROW(asyncValueWait(&async, UINT64_MAX));

    /* Get version list length. */
    u64 size=0;
    R_THROW(asyncValueGetSize(&async, &size));

    /* Allocate buffer and read version list. */
    auto json_buffer = std::make_unique<char[]>(size);
    R_THROW(asyncValueGet(&async, json_buffer.get(), size));

    /* Parse result as JSON data. */
    auto parser = simdjson::dom::parser();
    auto list = parser.parse(json_buffer.get(), size);

    /* Validate format version. */
    if (list["format_version"].get_uint64() != 1)
        throw std::runtime_error("Unknown Version!\n");

    auto array = list["titles"].get_array();
    impl.resize(array.size());
    auto ptr = impl.data();
    for (const auto &title : array) {
        const std::string_view id_string = title["id"];
        const uint64_t version = title["version"];
        const uint64_t required = title["required_version"];
        *ptr++ = AvmVersionListEntry {
            .app_id   = std::strtoull(id_string.data(), nullptr, 0x10),
            .version  = static_cast<uint32_t>(version),
            .required = static_cast<uint32_t>(required),
        };
    }
}

/* Get installed version. 0 if no patch is installed. */
u32 VersionList::GetInstalledVersion(u64 application_id) const noexcept {
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
u32 VersionList::GetAvailableVersion(u64 application_id) const noexcept {
    u64 patch_id = application_id | 0x800;
    const auto it = std::find_if(std::cbegin(impl), std::cend(impl), [&](const auto& entry) { return patch_id == entry.app_id; });
    
    if (it != std::end(impl)) {
        return it->version;
    } else {
        return 0;
    }
}

const char* VersionList::GetApplicationName(u64 app_id) const noexcept {
    static NsApplicationControlData nacp={};
    NacpLanguageEntry *entry = &nacp.nacp.lang[0];
    u64 size=0;
    nsGetApplicationControlData(NsApplicationControlSource_Storage, app_id, &nacp, sizeof(nacp), &size);
    nacpGetLanguageEntry(&nacp.nacp, &entry);
    return entry->name;
}

u32 VersionList::UpdateApplications() const noexcept {
    u32 updated_count=0;

    /* Iterate over installed applications. */
    s32 offset=0, count=0;
    NsApplicationRecord record;
    while (R_SUCCEEDED(nsListApplicationRecord(&record, 1, offset++, &count)) && count != 0) {
        /* Skip archived applications. */
        if (record.type == 0xb || record.type == 0x4)
            continue;

        const u64 app_id = record.application_id;
        
        /* Check if latest version is already installed. */
        if (GetInstalledVersion(app_id) >= GetAvailableVersion(app_id))
            continue;

        printf("Updating: %s\n", GetApplicationName(app_id));
        consoleUpdate(nullptr);

        /* Request update. */
        AsyncResult async;
        if (R_SUCCEEDED(nsRequestUpdateApplication2(&async, app_id))) {
            /* Wait for result. */
            const Result rc = asyncResultGet(&async);
            if (R_FAILED(rc)) {
                printf(" Update failed: 0x%x\n", rc);
            } else {
                updated_count++;
            }
            asyncResultClose(&async);
        } else {
            printf(" Update in progress\n");
        }
    }

    return updated_count;
}
