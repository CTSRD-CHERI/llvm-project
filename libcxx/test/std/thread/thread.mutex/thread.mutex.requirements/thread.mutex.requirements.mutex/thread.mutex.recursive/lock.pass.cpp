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

// <mutex>

// class recursive_mutex;

// void lock();

#include <mutex>
#include <thread>
#include <cstdlib>
#include <cassert>

#include <iostream>

std::recursive_mutex m;

typedef std::chrono::system_clock Clock;
typedef Clock::time_point time_point;
typedef Clock::duration duration;
typedef std::chrono::milliseconds ms;
typedef std::chrono::nanoseconds ns;

#if !defined(TEST_SLOW_HOST)
ms WaitTime = ms(250);
ms Tolerance = ms(200)
#else
ms WaitTime = ms(750);
ms Tolerance = ms(500)
#endif

void f()
{
    time_point t0 = Clock::now();
    m.lock();
    time_point t1 = Clock::now();
    m.lock();
    m.unlock();
    m.unlock();
    ns d = t1 - t0 - WaitTime;
    assert(d < Tolerance);  // within 200ms
}

int main()
{
    m.lock();
    std::thread t(f);
    std::this_thread::sleep_for(WaitTime);
    m.unlock();
    t.join();
}
