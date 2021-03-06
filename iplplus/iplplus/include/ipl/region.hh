/*****************************************************************//**
 *
 * @file
 * @author fesc
 * @date   Apr 14 2008
 * @par    Copyright: &copy; Datacon 2008-2012
 *              all rights reserved
 *
 * $Id: region.hh 6 2012-09-05 11:06:50Z fesc $
 *
 * @brief  Header for ipl::Region
 *
 ********************************************************************/

#ifndef IPL_REGION_HH
#define IPL_REGION_HH

#include "ipl/config.hh"

#include <vector>
#include <boost/serialization/access.hpp>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/vector.hpp>

#include "ipl/ipltypes.hh"
#include "ipl/validable.hh"
#include "ipl/log.hh"
#include "ipl/point.hh"
#include "ipl/winp.hh"
#include "ipl/rbo.hh"
#include "ipl/cache.hh"
#include "ipl/polygon.hh"
#include "ipl/rect.hh"

IPL_NS_BEGIN

template<typename T> class PictImg;
class Circle;
class Ellipse;

//! An arbitrary Region, a 2d Set.
/*! A Region is a binary image, stored in a run-length encoded form.
 * Therefore a Region is a collection of #ipl::Rbo's. It consists of a set of
 * pieces of lines, and each such piece is stored as start-point
 * <em>(x,y)</em> and length.
 *
 * Normally a Region is generated by thresholding an image of from geometric
 * primitives.
 *
 * @par Iteration over a Region.
 * To iterate over all points in a Region we first iterate over all @a Rbos and
 * then through the @a Rbo by incrementing the <tt>x</tt>-value of the start point,
 * analogously to the iteration over an @ref pict_iter "image".
 * @code
 * Region r(Circle(PointF64(100, 100), 20));
 * for (auto i = r.begin();
 *     i != r.end();
 *     ++i) {
 *     N32 x = i->start().x_, y = i->start().y_;
 *     for (N32 o = 0; o < i->len(); ++o)
 *         // process (x+o, y)
 * }
 * @endcode
 * With <tt>C++11</tt> the code can be simplified to
 * @code
 * Region r(Circle(PointF64(100, 100), 20));
 * for (auto & i : r) {
 *     N32 x = i.start().x_, y = i.start().y_;
 *     for (N32 o = 0; o < i.len(); ++o)
 *         // process (x+o, y)
 * }
 * @endcode
 */
class Region : public Validable
{
public:

    //! Rbo-Iterator.
    /*! Iterates over all @a Rbo's in the Region.
     */
    typedef std::vector<Rbo>::const_iterator RboIterator;

    /***********************************/
    //! @name Constructors, Destructor und Zuweisung
    //@{

    //! ctr
    /*! Construct an empty Region.
     * @todo default
     */
    Region();

    //! cpy-ctr
    /*! Generates a deep copy of the Region @a other.
     * @a todo default
     */
    Region(Region const & other);

    //! Construct a Region by thresholding an image.
    /*! All the pixel of the image @a img with a grayvalue in the interval
     * [@a lo, @a hi] become part of the Region.
     * @sa PictImage::binarize
     */
    Region(PictImg<UN8> const & img,
           N32 lo,
           N32 hi);
    //! @overload
    Region(PictImg<N16> const & img,
           N32 lo,
           N32 hi);

    //! dtr
    /*! @todo default
     */
    virtual ~Region();

    //! Assignment.
    /*! Generates a deep copy of the Region @a rhs.
     * @a todo default
     */
    Region & operator=(Region const & rhs);
    //@}

    /***********************************/
    //! @name Generation from a geometric Primitive.
    //@{

    //! Generate a Region from a WinP.
    /*! The border of WinP @a win belongs to the Region.
     */
    Region(WinP const & win);

    //! Generate a Region from a Circle.
    /*! The border of the circle @a cir belongs to the Region.
     */
    Region(Circle const & cir);

    //! Generate a Region from an Ellipse.
    /*! The border of the ellipse @a ell belongs to the Region.
     */
    Region(Ellipse const & ell);

    //! Generate a Region from a Polygon.
    /*! The border of the polygon @a poly belongs to the Region.
     */
    Region(PolygonN16 const & pol);

    //! Generate a Region from a Rectangle.
    /*! The border of the rectangle @a rec belongs to the Region.
     */
    Region(Rect const & rec);
    //@}

    /***********************************/
    //! @name Rbo Properties and Manipulation.
    //@{

    //! Iterator to the first Rbo of a Region.
    RboIterator begin() const {
        return rbos_.begin();
    }

    //! Iterator behind the last Rbo in a Region.
    RboIterator end() const {
        return rbos_.end();
    }

    //! Number of Rbo's in the Region.
    N32 nrRbos() const {
        return rbos_.size();
    }

    //! Predicate if Region empty (as a Set).
    bool empty() const {
        return rbos_.empty();
    }
    //@}

    /***********************************/
    //! @name Geometric Information.
    //@{

    //! Returns the Bounding Box.
    /*! This is the axis-parallel rectangle including the Region.
     * @throw EmptyRegionError, if Region empty.
     */
    WinP const & boundingBox() const;

    //! Check if the Region includes the Point @a pt.
    bool includes(PointN16 const & pt) const;
    //@}

    /***********************************/
    /*! @name Geometric Manipulations.
     * This includes affine transformations.
    */
    //@{

    //! Translate the Region by a vector @a v.
    /*! @sa getTranslate
     */
    Region & translate(PointN16 const & v);

    //! Generate a new translated Region.
    /*! @sa translate
     */
    Region const getTranslate(PointN16 const & v) const;

    //! Clips to a axis-parallel Window @a win.
    Region & clip(WinP const & win);
    //@}

    /***********************************/
    /*! @name Morphological Operations
     *
     */
    //@{

    //! The different predefined structuring elements.
    enum StructuringElement {
        //! square-shaped structuring element
        StructuringElementSquare = 1,
        //! circle-shaped structuring element
        StructuringElementCircle = 2,
        //! diamond-shaped structuring element
        StructuringElementDiamond = 3,
        //! line-shaped structuring element
        StructuringElementLine = 4,
    };

    //! Computes the erosion of a Region with structuring element @em B.
    /*! algorithm - erosion - variant 1
     * Calculates the erosion of the given region by structuring element @em B.
     * Therefore it determines the skeleton of @em B and the length of the shortest run
     * within @em B. Afterwards it generates @f$X_{L_{\min}}@f$ (the set of runs within @em X which are
     * bigger than or equal to the shortest run within @em B), and the erosion-transform
     * of @f$X_{L_{\min}}@f$. With this preprocessing-steps it is possible to apply the
     * jump-miss theorem in the final step. In case of a miss the algorithm stays in the
     * line where the miss occured and checks (pixel by pixel) for more misses in
     * that particular line.
     *
     * uses: generateSkeletonB(), generateErosionTransformX()
     *
     * @param B the structuring element @em B
     * @return X eroded by @em B
     */
    Region const erode1(Region const & B) const;

    //! Computes the erosion of a Region with structuring element @em B.
    /*! algorithm - erosion - variant 2
     * Calculates the erosion of the given region by structuring element @em B.
     * Therefore it determines the skeleton of @em B and the length of the shortest run
     * within @em B. Afterwards it generates @f$X_{L_{\min}}@f$ (the set of runs within @em X which are
     * bigger than or equal to the shortest run within @em B), and the erosion-transform
     * of @f$X_{L_{\min}}@f$ with @em A aswell as with @f$A^t@f$. With this preprocessing-steps it is possible
     * to apply the jump-miss and jump-hit theorem in the final step.
     *
     * uses: generateSkeletonB(), generateErosionTransformX2()
     *
     * @param B the structuring element @em B
     * @return the region eroded by @em B
     */
    Region const erode2(Region const & B) const;

    //! Computes the erosion of a Region with structuring element @em B.
    /*! algorithm - erosion - variant 2 cut
     * Calculates the erosion of the given region by structuring element @em B.
     * Therefore it determines the skeleton of @em B and the length of the shortest run aswell the longest
     * run within @em B. Afterwards it generates @f$X_{cut}@f$ (the set of runs within @em X cropped by the
     * length of the longest run within @em B), and the erosion-transform of @f$X_{L_{\min}}@f$ with @em A
     * aswell as with @f$A^t@f$. With this preprocessing-steps it is possible to apply the jump-miss and
     * jump-hit theorem in the final step.
     *
     * uses: generateSkeletonB(), generateErosionTransformX2()
     *
     * @param B the structuring element @em B
     * @return the region eroded by @em B
     */
    Region const erode2cut(Region const & B) const;

    //! Computes the erosion of a Region with structuring element @em B.
    /*! algorithm - variant 3
     * Calculates the erosion of the given region by structuring element @em B.
     * Therefore it determines the skeleton of @em B and the length of the shortest run
     * within @em B. Afterwards it generates @f$X_{L_{\min}}@f$ (the set of runs within X which are
     * bigger than or equal to the shortest run within @em B), and the extended erosion-transform
     * of @f$X_{L_{\min}}@f$ with @em A aswell as with @f$A^t@f$. With this preprocessing-steps it is possible
     * to apply the jump-miss and jump-hit theorem in the final step, where all jumps are
     * maximal.
     *
     * uses: generateSkeletonB(), generateExtErosionTransform()
     *
     * @param B the structuring element @em B
     * @return the region eroded by @em B
     */
    Region const erode3(Region const & B) const;

    //! Computes the dilation of a Region with structuring element @em B.
    /*! algorithm - dilation
     * Calculates the dilation of the object @em X with structuring element @em B.
     * Therefore it determines the skeleton of @f$B^t@f$ and the length of the shortest run
     * within @em B. Afterwards it generates @f$X^c_{L_{\min}}@f$ (the set of runs within @f$X^c@f$ which are
     * bigger than or equal to the shortest run within @em B), and the erosion-transform
     * of @f$X^c_{L_{\min}}@f$ with @em A aswell as with @f$A^t@f$. With this preprocessing-steps it is possible
     * to apply the jump-Miss and jump-hit Theorem in the final step.
     *
     * uses: generateSkeletonBtrans(), generateErosionTransformXcomp()
     *
     * @param B the structuring element @em B
     * @return the region dilated by @em B
     */
    Region const dilate(Region const & B) const;


    //! Computes the dilation of a Region with structuring element @em B.
    /*! algorithm - dilation
     * Calculates the dilation of the object @em X with structuring element @em B.
     * Therefore it determines the skeleton of @f$B^t@f$ and the length of the shortest run
     * within @em B. Afterwards it generates @f$X^c_{L_{\min}}@f$ (the set of runs within @f$X^c@f$ which are
     * bigger than or equal to the shortest run within @em B), and the erosion-transform
     * of @f$X^c_{L_{\min}}@f$ with @em A aswell as with @f$A^t@f$. With this preprocessing-steps it is possible
     * to apply the jump-Miss and jump-hit Theorem in the final step.
     *
     * uses: generateSkeletonBtrans(), generateErosionTransformXcomp()
     *
     * @param B the structuring element @em B
     * @return the region dilated by @em B
     */
    Region const dilatecut(Region const & B) const;


    //! Generates the structuring element of choice.
    /*! Generates the structuring element of choice.
     *
     * @param choice the type of structuring element to be generated.
     * @param size the size of the structuring element to be generated (square-shaped SE: width equals
     * 		       @f$(2 \cdot size + 1)@f$, circle-shaped SE: diameter equals
     * 		       @f$(2 \cdot size + 1)@f$, diamond-shaped SE: width equals
     * 		       @f$(2 \cdot size + 1)@f$).
     * @return the generated structuring element as region.
     */
    static Region const generateStructuringElement(StructuringElement choice, int size);

    //@}

    /***********************************/
    /*! @name Set Operations.
     */
    //@{
    //! Union of two Regions.
    /*! Computes the union of the object and @a other and returns a new Region.
     */
    Region const unions(Region const & other) const;

    //! Intersection of two Regions.
    /*! Computes the intersection of the object with @a other and returns a new
     * Region.
     */
    Region const intersect(Region const & other) const;

    //! Computes the Set Difference @a this - @a other.
    Region const subtract(Region const & other) const;

    //! Computes the Set Complement.
    /*! Returns a new region containing the complement of the object. For the
     * complement @a universe is used as base set. If the object is not
     * contained fully in @a universe.
     * If @a univers == 0 then we use the boundingbox of the Region.
     */
    Region const complement(WinP const * universe = 0) const;
    //@}


    /***********************************/
    //! @name Debug Output
    //@{
    std::ostream & print(std::ostream & os) const;
    virtual bool validate() const;
    //@}

    //! Insert a new Rbo @a s into the Region.
    Region & add(Rbo const & s);

protected:
    //! The class Logger.
    IPL_DECLARE_CLASS_LOGGER;

private:
    //! Container for the Rbo's.
    std::vector<Rbo> rbos_;

    //! The bounding Box.
    /*! @see boundingBox
     */
    mutable Cache<WinP> bbox_;

    //! Area, cached.
    /*! @see areaCenter
     */
    mutable Cache<N32> area_;

    //! Circumference, cached.
    /*! @see perimeter
     */
    mutable Cache<F64> perimeter_;

    //! Center of Gravity, cached.
    /*! @see areaCenter
     */
    mutable Cache<PointF64> center_;

    //! Recompute the bounding box.
    void computeBbox() const;

    //! Invalidates all cached Data.
    void invalidateCaches();

    //! Internal Template for Binarization.
    template<typename T>
    void createFromPic(PictImg<T> const & img, N32 lo, N32 hi);

    friend class PolygonRegionCreator; // necessary for 'add(Rbo(...))'
    friend class EllipticRegionCreator; // necessary for 'add(Rbo(...))'

    /*! @name Serialization
     * @sa @ref serialization
     */
    //@{
    friend class boost::serialization::access;
    template<typename Archive>
    void save(Archive & ar, UN32 /*version*/) const {
        ar << BOOST_SERIALIZATION_NVP(rbos_);
    }
    template<typename Archive>
    void load(Archive & ar, UN32 /*version*/) {
        Region tmp;
        ar >> boost::serialization::make_nvp(BOOST_PP_STRINGIZE(rbos_), tmp.rbos_);
        *this = tmp;
    }
    BOOST_SERIALIZATION_SPLIT_MEMBER();
    //@}

    /*
     *
     */
    //@{
    struct RetGenerateSkeletonB;
    struct erosTransPoint;
    struct RetGenerateErosionTransformX;
    struct RetGenerateErosionTransformX2;

    static RetGenerateSkeletonB const generateSkeletonBtrans(Region B, Point<N16> transVector);
    static RetGenerateSkeletonB const generateSkeletonB(Region B, Point<N16> transVector);
    RetGenerateErosionTransformX const generateErosionTransformX(Region B, N16 lmin) const;
    RetGenerateErosionTransformX const generateErosionTransformX2(Region B, N16 lmin) const;
    RetGenerateErosionTransformX const generateErosionTransformX2cut(Region B, N16 lmin, N16 lmax) const;
    RetGenerateErosionTransformX const generateErosionTransformXcomp(Region B, N16 lmin) const;
    RetGenerateErosionTransformX const generateErosionTransformXcompcut(Region B, N16 lmin, N16 lmax) const;
    RetGenerateErosionTransformX const generateExtErosionTransformX(Region B, N16 lmin) const;
    //@}
};

IPL_NS_END

#endif
