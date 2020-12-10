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

template <typename T>
struct scope_success {
    T f;
    inline scope_success(T _f): f(_f) { }
    inline ~scope_success() { if (std::uncaught_exceptions() == 0) f(); }
};

template <typename T>
struct scope_failure {
    T f;
    inline scope_failure(T _f) : f(_f) { }
    inline ~scope_failure() { if (std::uncaught_exceptions() > 0) f(); }
};

template <typename T>
struct scope_exit {
    T f;
    inline scope_exit(T _f): f(_f) { }
    inline ~scope_exit() { f(); }
};
