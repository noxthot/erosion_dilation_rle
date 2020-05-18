/*****************************************************************//**
 *
 * @file
 * @author fesc
 * @date   Apr 14 2008
 * @par    Copyright: &copy; Datacon 2008-2012
 *              all rights reserved
 *
 * $Id: winp.hh 1 2012-09-05 10:26:59Z fesc $
 *
 * @brief  Header for ipl::WinP
 *
 ********************************************************************/

#ifndef IPL_WINP_HH
#define IPL_WINP_HH

#include "ipl/config.hh"

#include <iostream>
#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>

#include "ipl/ipltypes.hh"
#include "ipl/point.hh"
#include "ipl/validable.hh"

IPL_NS_BEGIN

//! A simple Region, a rectangular Window.
/*! This region is defined through 2 points, the upper-left corner and the
 * lower-right corner. The corner points and the boundary are part of the
 * window.
 */
class WinP : public Validable
{
public:
    /**************************************************************************/
    //! @name Constructors, Destructor and Assignment
    //@{

    //! Ctr, a window with (0,0)-(0,0)
    WinP();

    //! Constructs a window from two points.
    /*! @a ul is the upper-left, @a lr the lower-right corner.
     */
    WinP(PointN16 const & ul,
         PointN16 const & lr);

    //! Constructs a window from 4 coordinates.
    /*! The upper-left corner is (@a ulX, @a ulY), the lower right is
     * (@a lrX, @a lrY).
     */
    WinP(N16 ulX, N16 ulY, N16 lrX, N16 lrY);

    //! CpyCtr
    /*! @todo default */
    //WinP(WinP const & other) = default;

    //! Dtr
    virtual ~WinP()
    {
        IPL_ASSERT_VALID(*this);
    }

    //! Assignment
    /*! @todo default */
    //WinP & operator=(WinP const &) = default;
    //@}

    /**************************************************************************/
    //! @name Basic Information
    //@{

    //! Returns the upper-left corner of the window.
    PointN16 const & upperLeft() const {
        return ul_;
    }

    //! Returns the lower-right corner of the window.
    PointN16 const & lowerRight() const {
        return lr_;
    }

    //! Returns the width of the window
    /*! This is the dimension in @a x - direction.
     */
    N16 width() const {
        return (lr_.x_ - ul_.x_ + 1);
    }

    //! Returns the height of the window
    /*! This is the dimension in @a y - direction.
     */
    N16 height() const {
        return (lr_.y_ - ul_.y_ + 1);
    }

    //! Inclusion check.
    /*! Checks if a point @a pt is inside the window.
     */
    bool includes(PointN16 const & pt) const {
        IPL_ASSERT_VALID(*this);
        return pt.x_ >= ul_.x_ and pt.x_ <= lr_.x_
            and pt.y_ >= ul_.y_ and pt.y_ <= lr_.y_;
    }

    //! Inclusion check.
    /*! Check if @a rhs is inside the window.
     */
    bool includes(WinP const & rhs) const {
        return this->includes(rhs.upperLeft())
                        and this->includes(rhs.lowerRight());
    }

    //! Returns the corners.
    /*! Returns the 4 corners of the window, starting from upper-left and running
     * clockwise. Write these points to the output-iterator @a res.
     */
    template<typename OutIt>
    void corners(OutIt res) const {
        *res++ = this->upperLeft();
        *res++ = PointN16(this->lowerRight().x_, this->upperLeft().y_);
        *res++ = this->lowerRight();
        *res++ = PointN16(this->upperLeft().x_, this->lowerRight().y_);
    }
    //@}

    /**************************************************************************/
    //! @name Comparision Operators
    //@{

    //! Equality
    /*! Two windows are equal if they have the same two defining corners.
     */
    bool
    operator==(WinP const & rhs) const {
        return this->upperLeft() == rhs.upperLeft() and
            this->lowerRight() == rhs.lowerRight();
    }

    //! Inequality
    bool
    operator!=(WinP const & rhs) const {
        return not (*this == rhs);
    }
    //@}

    /**************************************************************************/
    //! @name Geometric Manipulatin
    //@{

    //! Translate the window by a vector @a v.
    WinP & translate(PointN16 const & v);

    //! Create a new, by @a v translated, window.
    WinP const getTranslate(PointN16 const & v) const;

    //! Clip the window to @a rhs.
    /*! Returns false if the object would become empty. Do not modify the
     * object in this case.
     */
    bool clip(WinP const & other);
    //@}

    /**************************************************************************/
    //! @name Debug Output
    //@{
    virtual std::ostream & print(std::ostream & os) const;
    virtual bool validate() const;
    //@}

private:
IPL_INTERNAL_USE
    //! upper left corner
    PointN16 ul_;
    //! lower right corner
    PointN16 lr_;

    /**************************************************************************/
    /*! @name Serialization
     * @sa @ref serialization
     */
    //@{
    friend class boost::serialization::access;
    template<typename Archive>
    void save(Archive & ar, UN32 /*version*/) const {
        ar << BOOST_SERIALIZATION_NVP(ul_)
           << BOOST_SERIALIZATION_NVP(lr_);
    }
    template<typename Archive>
    void load(Archive & ar, UN32 /*version*/) {
        using boost::serialization::make_nvp;
        WinP tmp;
        ar >> make_nvp(BOOST_PP_STRINGIZE(ul_), tmp.ul_)
           >> make_nvp(BOOST_PP_STRINGIZE(lr_), tmp.lr_);
        *this = tmp;
    }
    BOOST_SERIALIZATION_SPLIT_MEMBER();
    //@}
};

IPL_NS_END

#endif
