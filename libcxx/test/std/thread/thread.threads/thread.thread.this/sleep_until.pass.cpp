//===----------------------------------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// UNSUPPORTED: libcpp-has-no-threads
// FLAKY_TEST.

// <thread>

// template <class Clock, class Duration>
//   void sleep_until(const chrono::time_point<Clock, Duration>& abs_time);

#include <thread>
#include <cstdlib>
#include <cassert>

int main()
{
    typedef std::chrono::system_clock Clock;
    typedef Clock::time_point time_point;
    std::chrono::milliseconds ms(500);
    time_point t0 = Clock::now();
    std::this_thread::sleep_until(t0 + ms);
    time_point t1 = Clock::now();
    std::chrono::nanoseconds ns = (t1 - t0) - ms;
    std::chrono::nanoseconds err = 5 * ms / 100;
#ifdef _LIBCPP_SLOW_TEST_HOST
    // 25ms is sometimes not enough tolerance on a busy jenkins slave running
    // multiple QEMU instances. 100ms should hopefully be enough
    err *= 4;
#endif
    // The time slept is within 5% of 500ms
    assert(std::abs(ns.count()) < err.count());
}
