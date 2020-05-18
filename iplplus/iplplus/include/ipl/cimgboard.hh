/*****************************************************************//**
 *
 * @file
 * @author dast
 * @date   Feb 11 2009
 * @par    Copyright: &copy; Datacon 2008-2012
 *              all rights reserved
 *
 * $Id: cimgboard.hh 1 2012-09-05 10:26:59Z fesc $
 *
 * @brief  Header for ipl::CImgBoard
 *
 ********************************************************************/

#ifndef IPL_CIMGBOARD_HH
#define IPL_CIMGBOARD_HH

#include "ipl/board.hh"

#include <boost/scoped_ptr.hpp>

#if !defined(IPL_BOARD_CIMG)
#   error CImgBoard not available in this configuration
#endif

namespace cimg_library {
class CImgDisplay;
template<typename T> class CImg;
}

IPL_NS_BEGIN

template<typename T> class PictImg;
class Rect;

//! A Board-Implementation that uses CImg-Library
/*! @ingroup grafik_classes
 * @sa @ref graphic_system
 */
class CImgBoard : public Board
{
public:
    //! Ctr
    CImgBoard();
    //! Construct a Board with a background image @a img.
    explicit CImgBoard(PictImg<UN8> const & img);
    //! Constructs a Board with a background image @a img.
    /*! Converts the image @a img to an UN8-bit image that uses the whole grayvalue
     * range.
     */
    explicit CImgBoard(PictImg<N16> const & img);
    //! Dtr
    virtual ~CImgBoard();

    //! Set the background image to @a img.
    /*! Clears the overlay.
     */
    void set(PictImg<UN8> const & img);
    //! Set the background image to @a img.
    /*! Converts the image to an UN8-bit image with the full grayvalue range.
     */
    void set(PictImg<N16> const & img);
    //! Set the Title of the Window to @a title.
    void setTitle(char const * title);
    //! Show or Refresh the Board.
    /*! If the flag @a wait is on, the board acts like a modal window and returns
     * the code of the selected user action. In the modal way you can
     * - CTRL+Wheel: zoom in/out
     * - CTRL+PADADD, CTRL+PADSUB: zoom in/out
     * - CTRL+D: double, enlarge the image by 3/2
     * - CTRL+C: half,   decrease the image by 3/2
     * - DoubleKlick or Backspace: reset to original size
     * - drawing a rectangle means zoom out to this rectangle
     * - Arrow keys, panning
     * - All other keys quit the modal mode
     *
     * @return Code of the user action
     * @retval 0 Cancel, the board was closed
     * @retval 1 A key was pressed
     * @retval 2 ESC was pressed
     */
    N32 show(bool wait);
    //! Close the Board.
    /*! The object remains valid and is only hidden. Use #show to show it again
     * on the display.
     */
    void hide();
    //! Write the Board and Overlay to a File.
    /*! Write background image and overlay to a file @a fileName, determines the
     * image format from the extensiion.
     */
    void save(std::string const & fileName) const;

    //! Read a WinP from two mouse clicks.
    /*! Returns @c false if the user canceled the action.
     */
    bool inputWinP(WinP & roi);
    //! Read a Circle from two mouse clicks.
    /*! Returns @c false if the user canceled the action.
     */
    bool inputCircle(ipl::Circle & c);
    //! Read the Cursor Postition.
    /*! The position is stored in @a pt. Returns @c false if the right mouse
     * button was pressed. Draws a small marker at the selected position.
     */
    bool inputPoint(PointN16 & pt);
    //! Returns the mouse positon.
    /*! The position is stored in @a pt and the code of the mouse button is returned
     * or 0 if the board was closed.
     * @sa inputPoint
     */
    N32 mousePos(PointN16 & pt);

    //! Read a Rectangle
    /*! @sa inputWinP
     */
    bool inputRect(Rect & rect);
    //! Returns Info about a selected Blob.
    /*! If the user selects a blob in the image, these blob is passed to the
     * callback @a cb.
     * If @a cb = 0 write some basic information about the blob to @c cout.
     */
    void blobInfo(Blobs const & blobs,
                  std::function<void (Region const & r)> cb = 0);

    //! set/get the sync Mode.
    /*! If sync is on, each drawing command is drawn immediately and the window
     * receives a paint event. This is slow for a raster image like this board.
     * Deactivate the sync before drawing in a loop.
     */
    //@{
    CImgBoard & sync(bool flag) {
        sync_ = flag;
        return *this;
    }
    bool sync() const {
        return sync_;
    }
    //@}

    virtual void push();
    virtual void pop();
    virtual void drawPointXy(F64 x, F64 y);
    virtual void drawLineXy(F64 x0, F64 y0, F64 x1, F64 y1);
    virtual void drawCircleXy(F64 x, F64 y, F64 radius);
    virtual void drawEllipseXy(F64 x, F64 y, F64 ra, F64 rb, Angle const & alpha);
    virtual void drawWinP(WinP const & win);
    virtual void drawRect(Rect const & rect);
    virtual void drawRegion(Region const & reg);
    virtual void drawPolygon(Polygon<N16> const & poly);
    virtual void drawPolygon(Polygon<F64> const & poly);
    virtual void drawAnnulusXy(F64 x, F64 y, F64 r1, F64 r2);
    virtual void drawText(PointF64 const & pnt, std::string const & text);

    virtual CImgBoard & add(Board const & other);
    virtual void clearOverlay();


private:
    //! board area
    boost::scoped_ptr<cimg_library::CImg<UN8> > dc_;
    //! background image
    boost::scoped_ptr<PictImg<UN8> > image_;
    //! the window object
    boost::scoped_ptr<cimg_library::CImgDisplay> display_;
    //! call paint after each drawing action
    bool sync_;
    //! stack for the sync-states
    std::stack<bool> syncs_;

    //! Copy the background image into the board area
    void setDc();
    //! Maps Board::Color to the native colors
    static UN8 const * cimgColor(Board::Color col);
    //! Maps Board::LineStyle to the native styles
    static UN32 cimgLineStyle(Board::LineStyle style);
    //! Converts a image @a img to UN8
    void conv2UN8(PictImg<N16> const & img);
};

IPL_NS_END

#endif // IPL_CIMGBOARD_HH
