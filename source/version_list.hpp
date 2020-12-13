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

#include <vector>

#include "avm.h"

using ApplicationId = u64;

class VersionList {
  private:
    std::vector<AvmVersionListEntry> impl;

  public:
    VersionList();

    void FetchFromCDN();

    u32 GetInstalledVersion(u64 application_id) const noexcept;
    u32 GetAvailableVersion(u64 application_id) const noexcept;
    const char* GetApplicationName(u64 app_id) const noexcept;
    u32 UpdateApplications() const noexcept;
};
