/*****************************************************************//**
 *
 * @file
 * @author dast
 * @date   Feb 11 2009
 * @par    Copyright: &copy; Datacon 2008-2012
 *              all rights reserved
 *
 * $Id: board.hh 9 2012-09-05 12:06:03Z fesc $
 *
 * @brief  Header for ipl::Board
 *
 ********************************************************************/

/*! @page graphic_system The Graphic Subsystem
 *
 * The whole graphical output in the  library @c IPLplus is realized through the
 * @a interface ipl::Board, means all the @c IPLplus functions implement against
 * this interface.
 *
 * An application can provide its  own implementation of this interface and then
 * pass an object of this type to  the library. For example @c CirLoc can give a
 * graphical feedback in this form, if we use it as shown below
 * @code
 * MyBoard board;    // MyBoard inherits from Board
 * PictImage img = ...
 * std::vector<CirLoc::Result> results;
 * CirLoc cl;
 * cl.nominalRadius(120)
 *     .requiredQuality(0.8);
 * // ... other parameters
 * cl.find(image, results, &board);
 * board.show();    // we assume MyBoard can display the results
 * @endcode
 *
 * @c  IPLplus  has  Board-implementations,  that  can  be  enabled  during  the
 * configuration step (see @ref configuration)
 *
 * - ipl::CImgBoard:   This   implements    the   graphical   output   via   the
 *    <tt>CImg</tt>-library. Therefore  this board is a bitmapped  image with  a
 *    rasterised overlay. If a graphic primitive  is drawn one time, it can't be
 *    deleted or  manipulated, we  can only clear  the whole overlay.  You can't
 *    retrieve the overlay and therefore we can't use multiple overlays. @n
 *    On the  other hand this  board gives a  very simple to use  possibility to
 *    show  images under  X11 and  under Windows  and allows  in its  event loop
 *    zooming and the input  of rois. This class is only build  if it is enabled
 *    explicitly at configuration time.
 *
 * - ipl::SvgBoard: This class stores all  graphic commands and dumps these with
 *    the  method  ipl::SvgBoard::writeToSvg to  an  <tt>std::ostream</tt> or  a
 *    file.  The client then must render this stream/file in its application. @n
 *    For performance reasons  large areas and ipl::Region are  not written as a
 *    svg-object,  but written  to an  extra file  in a  binary format  with the
 *    extension @c  PXL but the  same stem as  the svg-file. This  format stores
 *    Regions  in   a  run-length  encoded  image  with   the  following  format
 *    (little-endian only):
 *    @n
 *    <table>
 *      <tr>
 *         <td><b>value</b></td>
 *         <td><b>data type</b></td>
 *         <td><b>description</b></td>
 *      </tr><tr>
 *         <td></td>
 *         <td></td>
 *         <td></td>
 *      </tr><tr>
 *         <td>0x214c5850</td>
 *         <td>UN32</td>
 *         <td>signature (PXL!)</td>
 *      </tr><tr>
 *         <td>0x00010000</td>
 *         <td>UN32</td>
 *         <td>version (1.0)</td>
 *      </tr><tr>
 *         <td>-</td>
 *         <td>N16</td>
 *         <td>image width</td>
 *      </tr><tr>
 *         <td>-</td>
 *         <td>N16</td>
 *         <td>image height</td>
 *      </tr><tr>
 *         <td>-</td>
 *         <td>N32</td>
 *         <td>number of Rbos</td>
 *      </tr><tr>
 *         <td></td>
 *         <td></td>
 *         <td></td>
 *      </tr><tr>
 *         <td>-</td>
 *         <td>N16</td>
 *         <td>y-coordinate of Rbo #1</td>
 *      </tr><tr>
 *         <td>-</td>
 *         <td>N16</td>
 *         <td>x-coordinate of Rbo #1</td>
 *      </tr><tr>
 *         <td>-</td>
 *         <td>N32</td>
 *         <td>length of Rbo #1</td>
 *      </tr><tr>
 *         <td>-</td>
 *         <td>UN32</td>
 *         <td>color (AARRGGBB) of the Rbo #1</td>
 *      </tr><tr>
 *         <td>-</td>
 *         <td>N16</td>
 *         <td>y-coordinate of Rbo #2</td>
 *      </tr><tr>
 *         <td>-</td>
 *         <td>N16</td>
 *         <td>x-coordinate of Rbo #2</td>
 *      </tr><tr>
 *         <td>-</td>
 *         <td>N32</td>
 *         <td>length of Rbo #2</td>
 *      </tr><tr>
 *         <td>-</td>
 *         <td>UN32</td>
 *         <td>color (AARRGGBB) des Rbo #2</td>
 *      </tr><tr>
 *         <td>...</td>
 *         <td>...</td>
 *         <td>...</td>
 *      </tr>
 *    </table>
 *    @n
 *    A  client must  then render  this  file and  the corresponding  svg to  an
 *    overlay and display it.
 */

#ifndef IPL_BOARD_HH
#define IPL_BOARD_HH

#include "ipl/config.hh"
#include "ipl/ipltypes.hh"
#include "ipl/point.hh"
#include "ipl/circle.hh"
#include "ipl/ellipse.hh"
#include "ipl/range.hh"

#include <iosfwd>
#include <stack>

IPL_NS_BEGIN

class WinP;
class Region;
class Rect;
class Blobs;
template<typename T> class Polygon;
template<typename T> class PictImg;

/*! @defgroup grafik_classes Classes of the Graphic Subsystem.
 * See @ref graphic_system "here" for an introduction.
 */

//! The Interface Class for the Graphic Subsystem.
/*! @ingroup grafik_classes
 * This is the abstract class. All functions of @c IPLplus implement against
 * this interface.
 * @sa @ref graphic_system
 */
class Board
{
public:
    //! Class to save a Board.
    /*! @ingroup grafik_classes
     * This class is a @a guard and can be used to save the state of a Board and
     * restore it later without using the pair ipl::Board::push and
     * ipl::Board::pop.
     *
     * If a method changes the state of the Board during drawing actions, set
     * a Board::Guard before the first change. When the guard goes out of scope,
     * the Board will be restored.
     * @code
     * void
     * Klass::doIt(PictImage const & img, Board * pBoard)
     * {
     *     Board::Guard guard(pBoard);
     *     Region r(img, 0, 128);
     *     pBoard->fillColor(Board::Red).fillStyle(Board::SolidPattern);
     *     pBoard->drawRegion(r);
     *     if (r.empty()) {
     *         return;
     *     }
     *     N32 a = r.area();
     *     // ...
     * } // restore the board
     * @endcode
     */
    class Guard
    {
    public:
        //! Ctr
        /*! @c board is a pointer to the controlled board.
         */
        explicit Guard(Board * board);
        //! Dtr
        ~Guard();
    private:
        //! Board to control.
        Board * board_;
    };

    //! Predefined Colors.
    enum Color {
        Red,                    // #ff0000
        Green,                  // #00ff00
        Blue,                   // #0000ff
        Magenta,                // #ff00ff
        Yellow,                 // #ffff00
        Cyan,                   // #00ffff
        Goldenrod,              // #daa520
        Pink,                   // #ffc8cb
        Orange,                 // #ffa500
        Coral,                  // #ff7f50
        Turquoise,              // #40E0D0
        Purple,                 // #800080
        Indigo,                 // #4B0082
        Violet,                 // #EE82EE
        Maroon,                 // #800000
        Navy,                   // #000080
        Black,                  // #000000
        Gray,                   // #808080
        LightGray,              // #D3D3D3
        DarkGray,               // #A9A9A9
        DarkestGray,            // #2F4F4F
        LightGreen,             // #00FF7F
        LightRed,               // #FF6347
        White,                  // #ffffff
    };
    //! Styles for Lines.
    enum LineStyle {
        Solid,
        Dotted,
        DashDotted,
    };
    //! Styles for Markers
    enum MarkerStyle {
        Cross,
        Star,
        Ex,
        Circle,
        Triangle,
        Diamond,
        Dot,
    };
    //! Fonts for Text Output.
    enum FontFamily {
        monospace,
    };
    //! Ctr
    Board();
    //! Dtr
    virtual ~Board();
    //! setters und getters for Colors and Styles.
    //@{
    Board & lineColor(Color color) {
        state_.lineColor_ = color;
        return *this;
    }
    Color lineColor() const {
        return state_.lineColor_;
    }
    Board & lineStyle(LineStyle style) {
        state_.lineStyle_ = style;
        return *this;
    }
    LineStyle lineStyle() const {
        return state_.lineStyle_;
    }
    Board & fillColor(Color color) {
        state_.fillColor_ = color;
        return *this;
    }
    Color fillColor() const {
        return state_.fillColor_;
    }
    Board & lineScaling(bool scaling) {
        state_.lineScaling_ = scaling;
        return *this;
    }
    bool lineScaling() const {
        return state_.lineScaling_;
    }
    Board & markerSize(F64 size) {
        state_.markerSize_ = size;
        return *this;
    }
    F64 markerSize() const {
        return state_.markerSize_;
    }
    Board & fontFamily(FontFamily font) {
        state_.fontFamily_ = font;
        return *this;
    }
    FontFamily fontFamily() const {
        return state_.fontFamily_;
    }
    Board & fontSize(F64 size) {
        state_.fontSize_ = size;
        return *this;
    }
    F64 fontSize() const {
        return state_.fontSize_;
    }
    F64 fontPitch() const {
        return (state_.fontSize_ * 0.6); // ok for most monospace fonts
    }
    //@}
    //! Transparency.
    /*! Sets the transparency for filled areas. If @a opacity == 0 do not draw
     * areas, if @a opacity == 1, draw opaque.
     */
    //@{
    Board & opacity(F64 opacity) {
        state_.opacity_ = Range<F64>(0,1).clip(opacity);
        return *this;
    }
    F64 opacity() const {
        return state_.opacity_;
    }
    //@}

    //! Save the actual State of the Board on the Stack.
    /*! Save color and styles. Board::pop restores this state later.
     * @sa pop, Board::Guard
     * @internal If you override this method in a derived class, call first
     *   the push-method of the baseclass.
     */
    virtual void push();
    //! Restores the last State from the Stack.
    /*! Restores the former state of the board before calling Board::push
     * @sa push, Board::Guard
     * @internal If you override this method in a derived class, call the pop-method
     *   of the baseclass before leaving the method.
     */
    virtual void pop();

    //! Clears the Content of the Board.
    virtual void clearOverlay() = 0;

    //! @name Drawing of Geometric Primitives.
    /*! Draws several geometric primitives with the actual styles and colors.
     */
    //@{
    virtual void drawMarkerXy(F64 x, F64 y, MarkerStyle style);
    template<typename T>
    void drawMarker(Point<T> const & p, MarkerStyle style) {
        drawMarkerXy(p.x_, p.y_, style);
    }
    template<typename InIt>
    void drawMarkers(InIt first, InIt last, MarkerStyle style) {
        while (first != last) {
            drawMarker(*first, style);
            ++first;
        };
    }

    virtual void drawPointXy(F64 x, F64 y) = 0;
    template<typename T>
    void drawPoint(Point<T> const & p) {
        drawPointXy(p.x_, p.y_);
    }
    template<typename InIt>
    void drawPoints(InIt first, InIt last) {
        while (first != last) {
            drawPoint(*first);
            ++first;
        };
    }
    template<typename InIt>
    void drawPointsAsLines(InIt first, InIt last) {
        while (first != last) {
            InIt start = first;
            ++first;
            if(first != last) drawLine(*start, *first);
        };
    }

    virtual void drawLineXy(F64 x0, F64 y0, F64 x1, F64 y1) = 0;
    template<typename T>
    void drawLine(Point<T> const & start, Point<T> const & end) {
        drawLineXy(start.x_, start.y_, end.x_, end.y_);
    }

    virtual void drawCircleXy(F64 x, F64 y, F64 radius) = 0;
    void drawCircle(ipl::Circle const & circle) {
        drawCircleXy(circle.center().x_, circle.center().y_, circle.radius());
    }

    virtual void drawEllipseXy(F64 x, F64 y, F64 ra, F64 rb, Angle const & alpha) = 0;
    void drawEllipse(Ellipse const & ell) {
        drawEllipseXy(ell.center().x_, ell.center().y_,
                      ell.ra(), ell.rb(), ell.angle());
    }

    virtual void drawWinP(WinP const & win) = 0;
    virtual void drawRect(Rect const & rect) = 0;
    virtual void drawRegion(Region const & reg) = 0;
    virtual void drawPolygon(Polygon<N16> const & poly) = 0;
    virtual void drawPolygon(Polygon<F64> const & poly) = 0;

    virtual void drawAnnulusXy(F64 x, F64 y, F64 r1, F64 r2) = 0;
    template<typename T>
    void drawAnnulus(Point<T> const & p, F64 r1, F64 r2) {
        drawAnnulusXy(p.x_, p.y_, r1, r2);
    }
    //@}
    //! Draw a Sequence of Regions.
    /*! Draws each region in the sequence <em>[first, last)</em>. If the flag
     * @a cycled is on, draw in different colors, cycling through all predefined
     * colors starting from Board::Red.
     */
    template<typename FwdIt>
    void drawRegions(FwdIt first, FwdIt last, bool cycled = false) {
        Color lc = this->lineColor(), fc = this->fillColor();
        Color act = Board::Red;
        if (cycled) {
            this->lineColor(act).fillColor(act);
        }
        while (first != last) {
            this->drawRegion(*first);
            if (cycled) {
                act = Color((act+1) % Board::Black);
                this->lineColor(act).fillColor(act);
            }
            ++first;
        }
        this->lineColor(lc).fillColor(fc);
    }
    //! Draws a List of Blobs.
    /*! Draws all blobs in the blob list with different colors.
     * @sa drawRegions
     */
    void drawBlobs(Blobs const & blobs);
    //! Draw a text @a text at Position @a pnt.
    virtual void drawText(PointF64 const & pnt, std::string const & text) = 0;

    //! Paste a other Board into the Object.
    /*! An implementation may assume that @a other is an instance of the same
     * board type as this board itself. It is an error to call MyBoard::add with
     * an argument of type CImgBoard. In this case the implementation must throw
     * an exception of type <tt>std::bad_cast</tt>. This happens if we downcast
     * @a other.
     * @code
     * MyBoard & MyBord::add(Board const & other) {
     *     MyBoard const & r(dynamic_cast<MyBoard const &>(other));
     *     ...
     * @endcode
     */
    virtual Board & add(Board const & other) = 0;

private:
    //! Disallow Cpy-Ctr.
    /*! @todo delete this method */
    Board(Board const & rhs);
    //! Assignment disallowed.
    /*! @todo delete this method */
    Board & operator=(Board const & rhs);

    //! Colors and Styles.
    /*! colors and styles are a state of the board and all drawing functions uses
     * these actual state.
     */
    struct State {
        //! Line Color
        Color lineColor_;
        //! Fill Color
        Color fillColor_;
        //! Line Style
        LineStyle lineStyle_;
        //! Opacity
        F64 opacity_;
        //! linescaling?
        bool lineScaling_;
        //! Marker Size
        F64 markerSize_;
        //! Font
        FontFamily fontFamily_;
        //! Font Size
        F64 fontSize_;
        //! Ctr
        State();
        /*! Generierter Cpy-Ctr und Op= sind ok
         * @todo default cpy-ctr and op= */
    };
    //! The actual settings.
    State state_;

    //! Stack for the states.
    std::stack<State> states_;
};

IPL_NS_END

#endif // IPL_BOARD_HH
