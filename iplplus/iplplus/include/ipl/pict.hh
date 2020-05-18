/*****************************************************************//**
 *
 * @file
 * @author fesc
 * @date   Apr 14 2008
 * @par    Copyright: &copy; Datacon 2008-2012
 *              all rights reserved
 *
 * $Id: pict.hh 14 2012-09-05 12:46:22Z fesc $
 *
 * @brief  Header for ipl::PictImage
 *
 ********************************************************************/

#ifndef IPL_PICT_HH
#define IPL_PICT_HH

#include "ipl/config.hh"

#include <string>
#include <utility>
#include <algorithm>
#include <boost/shared_ptr.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/binary_object.hpp>

#include "ipl/ipltypes.hh"
#include "ipl/log.hh"
#include "ipl/point.hh"
#include "ipl/gapiterator.hh"
#include "ipl/region.hh"
#include "ipl/validable.hh"

IPL_NS_BEGIN

template<typename T> class PictImg;
class CamModel;

//! @relates PictImg
//@{
//! shortcut for known template instances
typedef PictImg<UN8> PictImage;
typedef PictImg<N16> PictImageN16;
//@}


//! One-Channel Image
/*! An image is organized as a 2D matrix. We denote the coordinates with
 * @a x (columns) * and @a y (rows). The origin is situated at the top left corner
 * and the count * starts with 0, i.e. the origin is at (0,0).
 *
 * @tparam T is the Pixel Type
 *
 * @section Pixel Types
 *
 * At the moment there are one-channel images for pixel types of @c UN8 and @c N16.
 *
 * @section pict_iter Iteration over an Image.
 *
 * There are several possibilities to iterate over an image and to access the
 * individual pixels.
 * - Iteration over the whole image: This is done with an iterator
 *   @c PictImg<T>::Iterator returned by ipl::PictImg<T>::begin() and
 *   ipl::PictImg<T>::end(). A @em roi, if any, will be ignored. Such an iterator
 *   is used to access or modify each pixel in an image, like
 *   @code
 *   PictImage img;
 *   PictImage::Iterator first = img.begin();
 *   PictImage::Iterator last = img.end();
 *   for ( ; first != last; ++first)
 *       *first = 126;
 *   @endcode
 * - Roi operations: If the image has a roi attached (via #setRoi) and we
 *   would take this into account, we have to do a two pass iteration, first we
 *   get an iterator to the @a rbos (#ipl::Rbo) of the roi and than to the
 *   underlying image data:
 *   @code
 *   PictImage img;
 *   img.setRoi(Region(WinP(200,200,400,400)));
 *   auto scanFirst = img.rboBegin();
 *   auto scanLast = img.rboEnd();
 *   for ( ; scanFirst != scanLast; ++scanFirst)
 *       std::fill(img.begin(scanFirst), img.end(scanFirst), 128);
 *   @endcode
 *   or alternatively
 *   @code
 *   for ( ; scanFirst != scanLast; ++scanFirst)
 *       std::fill_n(img.begin(scanFirst), scanFirst->len(), 128);
 *   @endcode
 * - Iteration over columns: First we determine the column range by a
 *   ipl::PictImg<T>::colRange, a pair of two <tt>ColIterator</tt>'s. Than we
 *   iterate over that range
 *   @code
 *   PictImage img("a image file");
 *   ConstColRange r = img.colRange(PointN16(70, 0), img.height());
 *   std::vector<PictImage::PixelType> col();
 *   std::copy(r.first, r.second, back_inserter(col));
 *   ...
 *   @endcode
 *
 * @section roi_attach The Image and the Roi.
 * The roi (<em>region of interest</em>) is that part of an image, in wich we
 * are interested in. An arbitrary ipl::Region can be set as such a roi via
 * #setRoi. All the methods and algorithms work only on the roi, not the whole
 * image, if not it will be mentioned in the documentation.
 *
 * If a new roi is set, the old one, if any, will be replaced, i.e. the new setting
 * applies to the original image, and the attached roi is <b>not</b> the
 * intersection of the old and new region.
 *
 * The method #detachRoi resets the roi to the whole image, therefore an image
 * without a roi effectively has a roi corresponding to the whole image size.
 */
template<typename T>
class PictImg : public Validable
{
public:

    //! Pixel Type
    typedef T PixelType;

    //! Pixel Iterator
    typedef T * Iterator;

    //! Smartpointer to an Image
    //@{
    typedef boost::shared_ptr<PictImg<T> > Ptr;
    typedef boost::shared_ptr<const PictImg<T> > ConstPtr;
    //@}

    //! Pixel Iterator
    typedef T const * ConstIterator;

    //! Iterator over a column
    typedef GapIterator<Iterator> ColIterator;
    //! Iterator over a column
    typedef GapIterator<ConstIterator> ConstColIterator;

    //! A column as a pair of two Iterators.
    typedef std::pair<ColIterator, ColIterator> ColRange;
    //! A column as a pair of two Iterators.
    typedef std::pair<ConstColIterator, ConstColIterator> ConstColRange;

    //! A row as a pair of two Iterators.
    typedef std::pair<Iterator, Iterator> RowRange;
    //! A row as a pair of two Iterators.
    typedef std::pair<ConstIterator, ConstIterator> ConstRowRange;

    //! The different morphology kernel types
    enum MorphElement {
        //! rectangular morphology kernel
        MorphElementRectangle = 0,
        //! cross morphology kernel
        MorphElementCross = 1,
        //! elliptical morphology kernel
        MorphElementEllipse = 2,
    };

    //! The different morphology operation types
    enum MorphType {
        //! erosion - white area shrinks
        MorphTypeErode=0,
        //! dilation - white area grows
        MorphTypeDilate=1,
        //! opening (erosion followed by dilation) - white dots vanish
        MorphTypeOpen=2,
        //! closing (dilation followed by erosion) - black dots vanish
        MorphTypeClose=3,
        //! gradient (dilate - erosion, aka thick gradient) - for edge detection
        MorphTypeGradient=4,
        //! tophat (source - opened, aka white tophat) - for extracting small (< kernel size) white elements
        MorphTypeTophat=5,
        //! blackhat (closed - source, aka black tophat) - for extracting small (< kernel size) black elements
        MorphTypeBlackhat=6,
    };

    /***********************************/
    //! @name Constructors, Destructors und Copying
    //@{

    //! Generates an uninitialized Image
    /*! The image has size @a sizeX x @a sizeY
     */
    explicit PictImg(N16 sizeX = 768,
                     N16 sizeY = 576);

    //! Generates an uninitialized Image of size @a sz
    explicit PictImg(PointN16 const & sz);

    //! Generates a Hull Class to existing Image Data.
    /*! Generates a PictImg<T> of size @c sizeX x @c sizeY, that references the
     * image data in @a pData.
     * @note The image data are not copied, therefore the caller must ensure
     * that the pointer @a pData is valid through the whole lifetime of the
     * object.
     */
    PictImg(N16 sizeX, N16 sizeY, PixelType * pData,
            boost::shared_ptr<CamModel const> pCamModel
                = boost::shared_ptr<CamModel const>());

    //! CpyCtr
    PictImg(PictImg const & other);
    //todo CpyCtr to convert between different pixel types

    //! Dtr
    virtual ~PictImg();

    //! Assignment; constructs a Deep Copy of the Object.
    PictImg & operator=(PictImg const & rhs);
    //@}

    /***********************************/
    /*! @name Serialization
     * This functionality is implemented via @b CImg library. Only those
     * image formats recognized by that library are supported.
     * @sa http://cimg.sourceforge.net
     */
    //@{

    //! Reads an Image from a File
    /*! The type is deduced from the extension. If the image consists of more
     * then one channel, a RGB-image is assumed and converted to a graylevel
     * image.
     * @throw IoError if the file can't be read or the format is not supported.
     */
    explicit PictImg(std::string const & fn);

    //! Writes an Image to a File @a fn
    /*! Only the roi is written, the other part of the image is filled with a
     * black background.
     * @throw IoError if the file can't be written or the format is not supported.
     */
    void save(std::string const & fn) const;
    //@}

    /***********************************/
    /*! @name Basic Information
     * Methods to query simple properties of an image.
     *
     * If you have a histogram of the image, the use of the corresponding
     * histogram methods is much faster. To compute the average and standard
     * deviation of an image one may use
     *    @code
     *    F64 mu, si;
     *    mu = image.average(si); // ok, but slow
     *    mu = Histogram(image).average(si);
     *    @endcode
     * @sa The corresponding section in Histogram
    */
    //@{

    //! Size of the Image
    /*! @see width, height
     */
    PointN16 const & size() const {
        return size_;
    }

    //! Width of the Image
    N16 width() const {
        return size_.x_;
    }

    //! Height of the Image
    N16 height() const {
        return size_.y_;
    }

    //! Checks whether the Image (its attached roi) is empty.
    bool empty() const {
        return this->roi().empty();
    }
    //@}

    //! Retrieves the Camera Calibration
    /*! Returns a 0-pointer if no calibration is set
     */
    boost::shared_ptr<CamModel const> camModel() const {
        return camModel_;
    }
    //! Sets the Camera Calibration
    void camModel(boost::shared_ptr<CamModel const> camModel) {
        camModel_ = camModel;
    }

    /***********************************/
    /*! @name Pixel Access, Iterators
     * See also: @ref pict_iter
    */
    //@{

    //! Iterator over the whole Image
    Iterator begin() {
        return data_;
    }

    //! Iterator over the whole Image
    Iterator end() {
        return data_ + capacity_;
    }

    //! Iterator over the whole Image
    ConstIterator begin() const {
        return data_;
    }
    //! Iterator over the whole Image
    ConstIterator end() const {
        return data_+capacity_;
    }

    //! Iterator over a Scanline
    Iterator begin(Region::RboIterator r) {
        return dataPtrAt(r->start().x_, r->start().y_);
    }
    //! Iterator over a Scanline
    Iterator end(Region::RboIterator r) {
        return dataPtrAt(r->start().x_+r->len(), r->start().y_);
    }

    //! Iterator over a Scanline
    ConstIterator begin(Region::RboIterator r) const {
        return dataPtrAt(r->start().x_, r->start().y_);
    }
    //! Iterator over a Scanline
    ConstIterator end(Region::RboIterator r) const {
        return dataPtrAt(r->start().x_+r->len(), r->start().y_);
    }

    //! Iterator over the Scanlines of the attached Roi (Region)
    Region::RboIterator rboBegin() const {
        return this->roi().begin();
    }
    //! Iterator over the Scanlines of the attached Roi (Region)
    Region::RboIterator rboEnd() const {
        return this->roi().end();
    }

    //! Pixel Value at Position (@a x, @a y)
    PixelType const & operator()(N16 x, N16 y) const {
        IPL_ASSERT(x >= 0 && x < this->size().x_
               && y >=0 && y < this->size().y_);
        return *dataPtrAt(x, y);
    }
    //! Pixel Value at Position @a p
    PixelType const & operator()(PointN16 const & p) const {
        IPL_ASSERT(p.x_ >= 0 && p.x_ < this->size().x_
               && p.y_ >=0 && p.y_ < this->size().y_);
        return *dataPtrAt(p.x_, p.y_);
    }

    //! Sets the Pixel at Position (@a x, @a y)
    PixelType & operator ()(N16 x, N16 y) {
        IPL_ASSERT(x >= 0 && x < this->size().x_
               && y >=0 && y < this->size().y_);
        return *dataPtrAt(x, y);
    }
    //! Sets the Pixel at Position @a p
    PixelType & operator ()(PointN16 const & p) {
        IPL_ASSERT(p.x_ >= 0 && p.x_ < this->size().x_
               && p.y_ >=0 && p.y_ < this->size().y_);
        return *dataPtrAt(p.x_, p.y_);
    }

    //! Iteration over a Row
    /*! Returns a range for a row starting at point @a p with length @a len.
     * The function clips the row to the image, but returns an empty range
     * if the starting point @p is outside the image.
     * The default of @a len means the range till the end of the image row.
     * @sa colRange
     * @sa @ref pict_iter
     */
    RowRange const rowRange(PointN16 const & p, N32 len = -1);
    /*! @overload */
    ConstRowRange const rowRange(PointN16 const & p, N32 len = -1) const;

    //! Iteration over a Column
    /*! Returns a range for a column starting at point @a p with length @a len.
     * The function clips the column to the image, but returns an empty range
     * if the starting point @p is outside the image.
     * The default of @a len means the range till the end of the image column.
     * @sa rowRange
     * @sa @ref pict_iter
     */
    ColRange const colRange(PointN16 const & p, N32 len = -1);
    /*! @overload */
    ConstColRange const colRange(PointN16 const & p, N32 len = -1) const;
    //@}

    /***********************************/
    /*! @name Roi Manipulation
     * See also: @ref roi_attach
     */
    //@{

    //! Attach a Roi
    /*! @sa roi, detachRoi, @ref roi_attach
     */
    PictImg & setRoi(Region const & roi);

    //! Returns the attached Roi
    /*!
     * @sa setRoi, detachRoi, @ref roi_attach
     */
    Region const & roi() const {
        return roi_;
    }

    //! Returns the full Image Domain as WinP
    WinP const & fullRoi() const {
        return fullRoi_;
    }

    //! Resets the Roi to the full Image Domain
    /*!
     * @sa setRoi, @ref roi_attach
     */
    PictImg & detachRoi();
    //@}

    /***********************************/
    //! @name Image Filtering
    //@{

    /*! Morphological operations
     * @param type type of the morphological operation
     * @param iterations number of times the operation is repeated
     * @param element kind of morphological element (e.g. rectangle)
     * @param ksz Size of the morphological kernel (must be odd, e.g. 3x3)
     * @return SmartPtr to the new image
     */
    Ptr morph(MorphType type, N32 iterations, MorphElement element, PointN16 const & ksz) const;

    //@}

    /***********************************/
    //! @name Mathematics
    //@{

    //! Pixel Transformation
    /*! Replaces each pixel @a p by @a op(p). Therefore the functor @a op must
     * be of type
     * @code
     * std::unary_function<PictImage::PixelType, PictImage::PixelType>
     * @endcode
     */
    template<typename Op>
    void pixTransform(Op op) {
        Region::RboIterator scan = this->rboBegin(),
            last = this->rboEnd();
        for ( ; scan != last; ++scan) {
            PictImg::Iterator i = this->begin(scan),
                e = this->end(scan);
            for ( ; i != e; ++i)
                *i = op(*i);
        }
    }

    //! LUT Transformation
    /*! Applies a LUT-transformation to each individual pixel.
     * Each pixel @a g is replaced with @a lut[g].
     * @param lut A vector of size 256
     * @return *this
     * @note Works only for @c UN8 pixel type.
     */
    PictImg & applyLut(std::vector<PixelType> const & lut);

    //! Inverts the Image
    PictImg & invert();

    //! Addition with Saturation
    /*! Adds to each pixel in the roi the value @a val and saturates the result.
     * @sa sub, absDiff
     */
    PictImg & add(PictImg::PixelType val);

    //! Sum of two Images
    /*! Adds to each pixel in the bounding box of the object the corresponding
     * from @a rhs and saturates the result.
     * The roi of @a rhs will be ignored but the @a bbox of the object is moved
     * to the corresponding of @a rhs.
     * @throw ParameterError if the moved @a bbox is not inside the image
     */
    PictImg & add(PictImg const & rhs);

    //! Subtraction with Saturation
    /*! Subtracts @a val for each pixel in the roi and saturates the result.
     * @sa add, absDiff
     */
    PictImg & sub(PictImg::PixelType val);

    //! Subtracts two Images
    /*! Die Subtraction is saturated. The roi handling is analog to add.
     * @sa add, absDiff
     */
    PictImg & sub(PictImg const & rhs);

    //! Absolute Difference
    /*! Calculates for each pixel @a p in the roi @f$ |val - p| @f$
     * @sa add, sub
     */
    PictImg & absDiff(PictImg::PixelType val);

    //! Absolute Difference of two Images
    /*! The roi handling is analog to add.
     * @sa add, sub
     */
    PictImg & absDiff(PictImg const & rhs);

    //@}

    /***********************************/
    //! @name Rendering
    //@{

    //! Fills the Image with grayvalue @a c
    PictImg & fill(PixelType c = PixelType(0));
    //@}

    /***********************************/
    /*! @name Cropping
     * Reading parts of an image, like a line or a window.
     */
    //@{
    //! Crops a horizontal Line.
    /*! Read the grayvalues of a row in the image. The row starts at @a start
     * and has length @a len and copies the values to @a out. The function clips
     * to the whole image but if @a start ist outside the image, nothing is read.
     * @a len defaults to the end of the row in the image.
     * @return end of the output sequence
     * @sa cropCol cropLine
     */
    template<typename OutIt>
    OutIt cropRow(PointN16 const & start,
                  OutIt out,
                  N32 len = -1) const;
    //! Crops a vertical Line.
    /*! This function is analog to cropRow but reads a column.
     * @return end of the output sequence
     * @sa cropRow cropLine
     */
    template<typename OutIt>
    OutIt cropCol(PointN16 const & start,
                  OutIt out,
                  N32 len = -1) const;

    //! Generates a new Image as as Section of this Image.
    /*! The window @a winp must be contained in the image. The roi, if any,
     * will be clipped and attached to the new image.
     * @param winp rectangle to crop to
     * @param pTrBack If != 0 modify pTrBack by multiplying a Trafo2D
     *    from the result picture to the source (this) picture
     * @return Smartptr to the new image
     */
    Ptr crop(WinP const & winp, Trafo2D * pTrBack = 0) const;
    //@}

    /***********************************/
    //! @name Segmentation
    //@{

    //! Generates a Region by Thresholding.
    /*! All the pixels of the image in the closed range [@a lo, @a hi] are
     * collected into the region.
     * @sa Region::Region
     */
    Region const
    binarize(PixelType lo,
             PixelType hi) const;
    //@}


    /***********************************/
    //! @name Debug Output
    //@{
    virtual bool validate() const;
    virtual std::ostream & print(std::ostream & os) const;
    //@}

protected:
    //! the class logger
    IPL_DECLARE_CLASS_LOGGER;

private:
    //! the size of the image
    PointN16 size_;

    //! capacity in element size, not bytes
    N32 capacity_;

    //! pointer to the image data
    PixelType * data_;

    /*! Signalizes if we have ownership of the data. If not, the object is only
     * a hull to foreign data and we must not free the data pointer on destruction.
     */
    bool myOwnership_;

    //! the full image domain
    WinP fullRoi_;

    //! the roi
    Region roi_;

    //! the camera calibration parameters
    boost::shared_ptr<CamModel const> camModel_;

    //! @name iterator into the image
    //@{

    //! returns a pointer to the imagedata at position (x,y)
    Iterator dataPtrAt(N16 x, N16 y) {
        return data_ + x + y*size_.x_;
    }

    ConstIterator dataPtrAt(N16 x, N16 y) const {
        return data_ + x + y*size_.x_;
    }
    //@}

    //! Adapts @a len to the bounding box of the roi
    /*! Adapts @a len such that p.x()+len respectively p.y()+len() are inside the
     * roi bounding box. Returns 0 if @a p lies outside the box. The function
     * clips to the full image domain.
     * The flag @a vertical indicates if we clip vertically (p.y()+len) or
     * horizontally (p.x()+len).
     * @return the adapted length
     */
    N32 adaptLen(PointN16 const & p, N32 len, bool vertical) const;

    //! Implementation of add/sub/absDiff
    PictImg &
    doPictOp(PictImg const & rhs, N32 op);

    /*! @name Serialization
     * See also: @ref serialization
     */
    //@{
    friend class boost::serialization::access;
    template<typename Archive>
    void save(Archive & ar, UN32 /*version*/) const {
        using boost::serialization::make_nvp;
        size_t bpp = sizeof(PixelType);
        ar << BOOST_SERIALIZATION_NVP(size_)
           << make_nvp(BOOST_PP_STRINGIZE(bpp_), bpp)
           << BOOST_SERIALIZATION_NVP(capacity_)
           << make_nvp("data_",
                    boost::serialization::make_binary_object(data_, capacity_*sizeof(T)))
           // myOwnership_
           << BOOST_SERIALIZATION_NVP(fullRoi_)
           << BOOST_SERIALIZATION_NVP(roi_);
    }
    template<typename Archive>
    void load(Archive & ar, UN32 /*version*/) {
        using boost::serialization::make_nvp;
        PointN16 sz;
        ar >> make_nvp(BOOST_PP_STRINGIZE(size_), sz);
        PictImg tmp(sz);
        size_t bpp;
        N32 cap;
        ar >> make_nvp(BOOST_PP_STRINGIZE(bpp_), bpp)
           >> make_nvp(BOOST_PP_STRINGIZE(capacity_), cap);
        if (bpp != sizeof(PixelType)) {
            throw IoError("invalid pixel type in serialized model");
        }
        if (cap != tmp.capacity_) {
            throw IoError("invalid data size in serialized model");
        }
        ar >> make_nvp(BOOST_PP_STRINGIZE(data_),
                  boost::serialization::make_binary_object(tmp.data_, tmp.capacity_*sizeof(T)))
        //myOwnership_
           >> make_nvp(BOOST_PP_STRINGIZE(fullRoi_), tmp.fullRoi_)
           >> make_nvp(BOOST_PP_STRINGIZE(roi_), tmp.roi_);
        *this = tmp;
    }
    BOOST_SERIALIZATION_SPLIT_MEMBER()
    //@}
};

//! Debug Output @relates PictImg
/*!
 * @note Both @c std::stringstream and @c log4cplus::StringBuffer inherit from
 * @c std::ostream, therefore we need only one overload.
 */
template<typename T>
std::ostream & operator<<(std::ostream & b,
                          typename PictImg<T>::PixelType t);

template<typename T>
template<typename OutIt>
inline
OutIt
PictImg<T>::cropRow(PointN16 const & start,
                    OutIt out,
                    N32 len /*= -1*/) const
{
    ConstRowRange r(this->rowRange(start, len));
    return std::copy(r.first, r.second, out);
}

template<typename T>
template<typename OutIt>
inline
OutIt
PictImg<T>::cropCol(PointN16 const & start,
                    OutIt out,
                    N32 len /*= -1*/) const
{
    ConstColRange r(this->colRange(start, len));
    return std::copy(r.first, r.second, out);
}

IPL_NS_END

#endif
