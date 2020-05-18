/*****************************************************************//**
 *
 * @file
 * @author fesc
 * @date   Apr 14 2008
 * @par    Copyright: &copy; Datacon 2008
 *              all rights reserved
 *
 * $Id: timer.cc 1 2012-09-05 10:26:59Z fesc $
 *
 * @brief  Implementation for ipl::Timer
 *
 ********************************************************************/

#include "ipl/timer.hh"
#include <cstdio>

#if defined(_MSC_VER) || defined(__CYGWIN__)
#    include <windows.h>
#elif defined(__GNUC__)
#    include <sys/time.h>
#    include <time.h>
#endif

IPL_NS_BEGIN

#if defined(_MSC_VER) || defined(__CYGWIN__)
// Implementierungsklasse für einen Windows-Timer
class TimerImpl
{
public:
    TimerImpl() {
        QueryPerformanceFrequency(&frequency_);
        QueryPerformanceCounter(&start_);
    }
    void tic() {
        QueryPerformanceCounter(&start_);
    }
    UN32 toc() const {
        LARGE_INTEGER now;
        QueryPerformanceCounter(&now);
        return static_cast<UN32>(
            static_cast<double>(now.QuadPart - start_.QuadPart) /
                static_cast<double>(frequency_.QuadPart) * 1000000.0);
    }
private:
    LARGE_INTEGER frequency_, start_;
};
#elif defined(__GNUC__)
// Implementierungsklasse für einen IntelCPU-Timer
class TimerImpl
{
public:
    TimerImpl()
    {
        gettimeofday(&start_, 0);
    }
    void tic() {
        gettimeofday(&start_, 0);
    }
    UN32 toc() const {
        struct timeval now;
        gettimeofday(&now, 0);
        return (now.tv_sec - start_.tv_sec) * 1000000
            + (now.tv_usec - start_.tv_usec);
    }
private:
    struct timeval start_;
};
#else
// dummy timer, tut nichts
#    warning "no timer aviable, timings will be incorrect"
//! Implementierungsklasse für einen Timer
/*! @sa Timer
 */
class TimerImpl
{
public:
    //! ctr
    TimerImpl() {}
    //! startet die Stoppuhr
    /*! @sa Timer::tic
     */
    void tic() {
        start_ = 0;
    }
    //! Zeitmessung
    /*! @sa Timer::toc
     */
    UN32 toc() const {
        return start_;
    }
private:
    UN32 start_;
};
#endif

Timer::Timer()
    : pImpl_(new TimerImpl)
{}

Timer::~Timer()
{}

void
Timer::tic()
{
    pImpl_->tic();
}

UN32
Timer::toc() const
{
    return pImpl_->toc();
}

std::string
Timer::report(char const * head) const
{
    char buf[100];
    snprintf(buf, sizeof(buf), "%s: took %u musecs", head, this->toc());
    return std::string(buf);
}

IPL_NS_END
