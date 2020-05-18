/*****************************************************************//**
 *
 * @file
 * @author fesc
 * @date   Apr 14 2008
 * @par    Copyright: &copy; Datacon 2008-2012
 *              all rights reserved
 *
 * $Id: validable.hh 1 2012-09-05 10:26:59Z fesc $
 *
 * @brief  Assertions and Object Validation
 *
 ********************************************************************/

/*!
 * @page assertions Assertions
 *
 * The @c IPLplus  library provides its own ASSERT-macro  #IPL_ASSERT. Like with
 * the standard  macro assertions  are only active,  if the library  is compiled
 * with the symbol @c IPL_ASSERT_ENABLED.
 *
 * The library also  has a macro #IPL_STATIC_ASSERT for  static assertions. This
 * macro checks  at compile  time the conditions,  so the condition  must depend
 * only on constants, for example assumptions on a template parameter.
 *
 * @cond developer_docu
 *
 * For complex objects you must provide  a check to verify the integrity for the
 * object. Inherit  your class from the interface  #ipl::Validable and implement
 * the interface.
 *
 * Methods in  such a class  can (and should)  check their own state  by calling
 * #IPL_ASSERT_VALID if the method is more then a trivial line of code.
 *
 * For assumptions on template  Parameters use traits and partial specialization
 * or check those assumptions with a static assert like
 *
 * @code
 * F64
 * PictImg<T>::average() const
 * {
 *   // the implementation of this function uses N64 for the sum type. This works
 *   // only for Pixels of integer type
 *   IPL_STATIC_ASSERT(std::numeric_limits<PixelType>::is_integer,
 *                     "need integral pixel type");
 *   N64 s = std::accumulate(this->begin(), this->end(), 0);
 *   return static_cast<F64>(s) / (this->size().x() * this->size().y());
 * }
 * @endcode
 * @endcond
 */

#ifndef IPL_VALIDABLE_HH
#define IPL_VALIDABLE_HH

#include "ipl/config.hh"

#if defined(IPL_ASSERT_ENABLED)
#   if defined (NDEBUG)
#       undef NDEBUG
#   endif
#endif

#include <cassert>
#include <iostream>

#include "ipl/ipltypes.hh"
#include "ipl/log.hh"

IPL_NS_BEGIN

//! Macro for Object Validation
/*! This macro calls assert, if the validation of an object @a obj fails.
 * @cond developer_docu
 * @see Validable and @ref assertions
 * @endcond
 */
#if defined (IPL_ASSERT_ENABLED)
#   define IPL_ASSERT_VALID(obj) \
           {if (!(obj).validate()) { \
                   IPLLOG_FATAL("invalid obj: ");                       \
                   IPLLOG_FATAL((obj));                                 \
                   IPL_ASSERT(0); }                                     \
           }
#else
#   define IPL_ASSERT_VALID(obj)
#endif

//! The Classical assert Macro
#if defined (IPL_ASSERT_ENABLED)
#   define IPL_ASSERT(A) assert(A)
#else
#   define IPL_ASSERT(A)
#endif

//! Static ASSERT, compile time check, see also static_assert in the c++11 standard
/*! macro IPL_STATIC_ASSERT
 * Invocation: IPL_STATIC_ASSERT(expr, msg)
 *
 * where:
 * - expr is a compile-time integral or pointer expression
 * - msg is a string with the error text
 * If expr is zero, id will appear in a compile-time error message.
 * @cond developer_docu
 * @sa @ref assertions
 * @endcond
 */
#define IPL_STATIC_ASSERT(expr, msg) static_assert(expr, msg)

//! Interface for Object Validation
/*! Complex classes should provide a method to validate their own state.
 * This interface abstracts this process and such classes should inherit from
 * it. Normally not the interface is used, but the validation occurs with the
 * help of the macro #IPL_ASSERT_VALID.
 * @cond developer_docu
 * @sa @ref assertions
 * @endcond
 */
class Validable
{
public:
    /**************************************************************************/
    //! @name Constructors, Destructor and Assignment
    //@{

    //! Ctr
    /*! @todo default */
    //Validable() = default;

    //! CpyCtr
    /*! @todo default */
    //Validable(Validable const &) = default;

    //! Dtr
    virtual ~Validable()
    {}

    //! Assignment
    /*! @todo default */
    //Validable & operator=(Validable const &) = default;
    //@}

    /***********************************/
    //! @name Debug Output
    //@{

    //! Validation
    /*! Returns @c true, if the object is in a consistent state, else @c false.
     */
    virtual bool validate() const = 0;

    //! Dumps the object to a stream
    /*! Almost used for logging purposes.
     */
    virtual std::ostream & print(std::ostream & os) const = 0;
    //@}
};

//! @name Debug Output
//@{
/*! @relates Validable
 */
inline
std::ostream &
operator<<(std::ostream & os, Validable const & v) {
    return v.print(os);
}
//@}

IPL_NS_END

#endif
