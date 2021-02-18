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
