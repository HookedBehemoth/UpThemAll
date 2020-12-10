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
#include <exception>
#include <switch.h>

struct hos_exception : public std::exception {
    char rc_buff[0x25];
    hos_exception(Result rc) noexcept {
        std::sprintf(rc_buff, "Result: 2%03u-%04u / 0x%x\n", R_MODULE(rc), R_DESCRIPTION(rc), R_VALUE(rc));
    }
    const char* what() const noexcept { return rc_buff; }
};

#define R_TRY(expr)              \
({                               \
    const auto rc = (expr);      \
    if (R_FAILED(rc))            \
        throw hos_exception(rc); \
})
