/*****************************************************************//**
 *
 * @file
 * @author felix schwitzer
 * @date   Apr 21 2008
 * @par    Copyright: &copy; Datacon 2008
 *              all rights reserved
 *
 * $Id: cache.hh 1 2012-09-05 10:26:59Z fesc $
 *
 * @brief  Template zum Cachen eines Datums
 *
 ********************************************************************/

#ifndef IPL_CACHE_HH
#define IPL_CACHE_HH

#include "ipl/config.hh"

#include "ipl/validable.hh"

IPL_NS_BEGIN

///@cond developer_docu

//! Simple class to Cache Data.
/*! @ingroup HelperTemplates
 */
template<typename T>
class Cache
{
public:
    /**************************************************************************/
    //! @name Constructors, Destructor and Assignment.
    //@{

    //! Default Ctr
    /*! Construct an out of date element of type @a T.
     * @todo default
     */
    Cache()
        : data_(T()),
          upToDate_(false)
    {}
    //! Ctr
    /*! Construct a cache with value @a datum and mark it up to date.
     */
    explicit Cache(T const & datum)
        : data_(datum),
          upToDate_(true)
    {}
    //! CpyCtr
    /*! @todo default */
    Cache(Cache const & rhs)
        : data_(rhs.data_),
          upToDate_(rhs.upToDate_)
    {}
    //! Dtr
    /*! @todo default */
    ~Cache()
    {}
    //! Assignment
    /*! @todo default */
    Cache & operator=(Cache const & rhs) {
        data_ = rhs.data_;
        upToDate_ = rhs.upToDate_;
        return *this;
    }
    //@}

    /**************************************************************************/
    //! @name Cache Operations
    //@{
    //! Invalidates the cache entry.
    void invalidate() {
        upToDate_ = false;
    }
    //! Updates the cache with the new entry @a newDatum.
    void update(T const & newDatum) {
        data_ = newDatum;
        upToDate_ = true;
    }
    //! Returns the cached value.
    T const & get() const {
        IPL_ASSERT(upToDate_);
        return data_;
    }
    //! Returns the cached value.
    T & get() {
        IPL_ASSERT(upToDate_);
        return data_;
    }
    //! Queries the cache state.
    bool upToDate() const {
        return upToDate_;
    }
    //@}
private:
    //! cached Datum
    T data_;
    //! Flag for the cache state.
    bool upToDate_;
};

//! Specialization.
/*! Disallow caching of pointers.
  @ingroup HelperTemplates
*/
template<typename T>
class Cache<T*>
{
private:
    ~Cache() {
    }
};

///@endcond developer_docu

IPL_NS_END

#endif //IPL_CACHE_HH
