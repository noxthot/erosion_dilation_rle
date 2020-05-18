/*****************************************************************//**
 *
 * @file
 * @author fesc
 * @date   Apr 14 2008
 * @par    Copyright: &copy; Datacon 2008-2011
 *              all rights reserved
 *
 * $Id: timer.hh 1 2012-09-05 10:26:59Z fesc $
 *
 * @brief  Header for ipl::Timer
 *
 ********************************************************************/

#ifndef IPL_TIMER_HH
#define IPL_TIMER_HH

#include "ipl/config.hh"

#include <string>

#include "ipl/ipltypes.hh"
#include <boost/scoped_ptr.hpp>

IPL_NS_BEGIN

class TimerImpl;

//! Timer to Measure Time-Spans
/*! This class is used only for time measurements, so it isn't really a timer
 *
 * @code
 * Timer tm;
 * img.fill(128);
 * U32 el = tm.toc();
 * cout << tm.report("fill") << endl;
 *
 * tm.tic();
 * img.min();
 * cout << tm.report("min") << endl;
 * @endcode
*/
class Timer {
public:

    //! Ctr
    Timer();

    // CpyCtr and op= deactivated because of scoped_ptr

    //! Dtr
    ~Timer();

    //! Start a Measurement
    void tic();

    //! Returns the Time in mu-sec
    /*! Returns the elapsed time since last #ipl::Timer.tic
     */
    UN32 toc() const;

    //! Reports a Time-Span
    /*! Reports in the form "'head' took 'xx' musec".
     */
    std::string report(char const * head) const;
private:

    //! Pointer to the Implementation Class
    boost::scoped_ptr<TimerImpl> pImpl_;
};

IPL_NS_END

#endif
