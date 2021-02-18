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

#pragma once

#include <switch.h>

#include <string>
#include <unordered_map>
#include <vector>

using ApplicationId = u64;

class VersionList {
  private:
    std::vector<AvmVersionListEntry> impl;
    std::unordered_map<ApplicationId, std::pair<std::string, bool>> available;

  public:
    VersionList();

    void Refresh();
    void FetchFromCDN();

    u32 GetInstalledVersion(ApplicationId application_id) const noexcept;
    u32 GetAvailableVersion(ApplicationId application_id) const noexcept;
    u32 GetLaunchRequiredVersion(ApplicationId application_id) const noexcept;
    const char* GetApplicationName(ApplicationId application_id) const noexcept;
    const u8* GetThumbnail(ApplicationId application_id) const noexcept;
    void UpdateSynchronous(ApplicationId application_id) const noexcept;
    void UpdateAllApplications() noexcept;
    
    void List(bool has_internet) noexcept;
    void Nuke() noexcept;

  private:
    void UpdateAvailable();
};
