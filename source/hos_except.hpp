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
#include <cstdio>
#include <string>
#include <exception>
#include <switch.h>

struct hos_exception : public std::exception {
    char buffer[0x100];
    hos_exception(Result rc) noexcept {
        std::sprintf(buffer, "Result: 2%03u-%04u / 0x%x\n", R_MODULE(rc), R_DESCRIPTION(rc), R_VALUE(rc));
    }
    hos_exception(const char* desc, Result rc) noexcept {
        std::sprintf(buffer, "%s: Result: 2%03u-%04u / 0x%x\n", desc, R_MODULE(rc), R_DESCRIPTION(rc), R_VALUE(rc));
    }
    const char* what() const noexcept { return buffer; }
};

#define R_THROW(expr)              \
({                                 \
    const auto rc = (expr);        \
    if (R_FAILED(rc)) [[unlikely]] \
        throw hos_exception(rc);   \
})

#define R_THROW_DESC(expr, desc)       \
({                                     \
    const auto rc = (expr);            \
    if (R_FAILED(rc)) [[unlikely]]     \
        throw hos_exception(desc, rc); \
})
