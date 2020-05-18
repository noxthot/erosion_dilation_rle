/*****************************************************************//**
 *
 * @file
 * @author fesc
 * @date   Jun 10, 2009
 * @par    Copyright: &copy; Datacon 2008-2012
 *              all rights reserved
 *
 * $Id: cimgboard.cc 14 2012-09-05 12:46:22Z fesc $
 *
 * @brief  Boardimplementierung mit Hilfe der CImg
 *
 ********************************************************************/

#include "ipl/cimgboard.hh"

#include <algorithm>
#include <iostream>
#include <string>
#include <sstream>
#include <boost/lambda/lambda.hpp>
#include <CImg.h>

#include "ipl/iplerr.hh"
#include "ipl/pict.hh"
#include "ipl/rect.hh"
#include "ipl/range.hh"
#include "ipl/trafo2d.hh"
#include "ipl/region.hh"

using namespace cimg_library;

IPL_NS_BEGIN

CImgBoard::CImgBoard() :
        dc_(new CImg<UN8>),
        image_(new PictImage(1024, 768)),
        display_(new CImgDisplay(image_->width(), image_->height())),
        sync_(false)
{
    image_->fill();
    setDc();
    show(false);
}

CImgBoard::CImgBoard(PictImg<UN8> const & img) :
        dc_(new CImg<UN8>),
        image_(new PictImage(img)),
        display_(new CImgDisplay(image_->width(), image_->height())),
        sync_(false)
{
    setDc();
    show(false);
}

CImgBoard::CImgBoard(PictImg<N16> const & img) :
        dc_(new CImg<UN8>),
        image_(new PictImage(img.size())),
        display_(new CImgDisplay(image_->width(), image_->height())),
        sync_(false)
{
    conv2UN8(img);
    setDc();
    show(false);
}

CImgBoard::~CImgBoard()
{}

void
CImgBoard::push()
{
    Board::push();
    syncs_.push(sync_);
}

void
CImgBoard::pop()
{
    if (syncs_.empty()) {
        return;
    }
    sync_ = syncs_.top();
    syncs_.pop();
    Board::pop();
}

void
CImgBoard::set(PictImg<UN8> const & img)
{
    *image_ = img;
    setDc();
}

void
CImgBoard::set(PictImg<N16> const & img)
{
    conv2UN8(img);
    setDc();
}

void
CImgBoard::setTitle(const char * title)
{
    display_->set_title("%s", title);
}

N32
CImgBoard::show(bool wait)
{
    if (display_->is_closed()) {
        display_->show();
    }
    if (!wait) {
        display_->display(*dc_);
        display_->paint(wait);
        display_->flush();
        return 0;
    }
    dc_->display(*display_, false);
    if (display_->is_closed()) {
        return 0;
    }
    if (display_->key() == 65307) { //ESC?
        return 2;
    }
    return 1;
}

void
CImgBoard::hide()
{
    display_->close();
}

void
CImgBoard::save(std::string const & fileName) const
{
    this->dc_->save(fileName.c_str());
}

bool
CImgBoard::inputWinP(WinP & roi)
{
    PointN16 ul, lr;
    Board::Guard guard(this);
    sync_ = true;
    dc_->draw_text(50, 50, "define the roi: corners => mouse-1, mouse-2 => abbruch",
                   cimgColor(Board::White), cimgColor(Board::Black));
    display_->display(*dc_);
    lineColor(Board::Yellow);
    opacity(0);
    if (not this->inputPoint(ul) or not this->inputPoint(lr)) {
        return false;
    }
    if (lr < ul) {
        std::swap(lr, ul);
    }
    roi = WinP(ul, lr);
    this->drawWinP(roi);
    return true;
}

bool
CImgBoard::inputCircle(ipl::Circle & cir)
{
    PointN16 c, r;
    Board::Guard guard(this);
    sync_ = true;
    dc_->draw_text(50, 50,
                   "define the circle: center and radius => mouse-1,"
                   " mouse-2 => abbruch",
                   cimgColor(Board::White), cimgColor(Board::Black));
    display_->display(*dc_);

    if (1 != this->mousePos(c) or 1 != this->mousePos(r)) {
        return false;
    }
    cir = ipl::Circle(c, (c-r).norm());
    this->drawCircle(cir);
    return true;
}

bool
CImgBoard::inputPoint(PointN16 & pt)
{
    Board::Guard guard(this);
    sync_ = true;
    dc_->draw_text(50, 50, "define cursor position, mouse-2 => abbruch",
                   cimgColor(Board::White), cimgColor(Board::Black));
    display_->display(*dc_);
    PointN16 p;
    if (1 == mousePos(p)) {
        pt = p;
        this->markerSize(5);
        this->drawMarker(pt, Board::Cross);
        return true;
    }
    return false;
}

N32
CImgBoard::mousePos(PointN16 & pt)
{
    N32 b = 0;
    display_->display(*dc_);
    display_->flush();
    while (!display_->is_closed()) {
        b = display_->button();
        if (b == 1 or b == 2) {
            pt.x_ = display_->mouse_x() * image_->width() / display_->width();
            pt.y_ = display_->mouse_y() * image_->height() / display_->height();
            break;
        }
    }
    return b;
}

bool
CImgBoard::inputRect(Rect & rect)
{
    WinP roi;
    bool ok = inputWinP(roi);
    if (ok) {
        rect = Rect(roi);
    }
    return ok;
}


void
CImgBoard::drawPointXy(F64 x, F64 y)
{
    N32 xi = mathli::roundF(x),
        yi = mathli::roundF(y);
    dc_->draw_point(xi, yi, cimgColor(lineColor()));
    if (sync_) {
        display_->display(*dc_);
    }
}

void
CImgBoard::drawLineXy(F64 x0, F64 y0, F64 x1, F64 y1)
{
    N32 xi0 = mathli::roundF(x0),
        yi0 = mathli::roundF(y0),
        xi1 = mathli::roundF(x1),
        yi1 = mathli::roundF(y1);
    dc_->draw_line(xi0, yi0, xi1, yi1,
                   cimgColor(lineColor()),
                   1.0,
                   cimgLineStyle(lineStyle()));
    if (sync_) {
        display_->display(*dc_);
    }
}

void
CImgBoard::drawCircleXy(F64 x, F64 y, F64 radius)
{
    N32 xi = mathli::roundF(x),
        yi = mathli::roundF(y),
        ri = mathli::roundF(radius);
    if (opacity() > 0) {
        dc_->draw_circle(xi, yi, ri, cimgColor(fillColor()), opacity());
    }
    dc_->draw_circle(xi, yi, ri, cimgColor(lineColor()), 1.0,
                     cimgLineStyle(lineStyle()));
    if (sync_) {
        display_->display(*dc_);
    }
}

void
CImgBoard::drawEllipseXy(F64 x, F64 y, F64 ra, F64 rb, Angle const & alpha)
{
    N32 xi = mathli::roundF(x),
        yi = mathli::roundF(y);
    if (opacity() > 0) {
        dc_->draw_ellipse(xi, yi, ra, rb, alpha.deg(),
                          cimgColor(fillColor()), opacity());
    }
    dc_->draw_ellipse(xi, yi, ra, rb, alpha.deg(),
                      cimgColor(lineColor()), 1.0, cimgLineStyle(lineStyle()));
    if (sync_) {
        display_->display(*dc_);
    }
}


void
CImgBoard::drawWinP(WinP const & win)
{
    PointN16 const & ul = win.upperLeft();
    PointN16 const & lr = win.lowerRight();
    UN8 const * lc = cimgColor(lineColor());
    UN8 const * fc = cimgColor(fillColor());
    UN32 ls = cimgLineStyle(lineStyle());
    if (opacity() > 0) {
        dc_->draw_rectangle(ul.x_, ul.y_, lr.x_, lr.y_, fc, opacity());
    }
    dc_->draw_rectangle(ul.x_, ul.y_, lr.x_, lr.y_, lc, 1.0, ls);
    if (sync_) {
        display_->display(*dc_);
    }
}

void
CImgBoard::drawRect(Rect const & rect)
{
    F64 const dx = rect.size().x_ / 2,
              dy = rect.size().y_ / 2;
    PointF64 quad[4];
    quad[0] = rect.center()+PointF64(-dx, -dy);
    quad[1] = rect.center()+PointF64(+dx, -dy);
    quad[2] = rect.center()+PointF64(+dx, +dy);
    quad[3] = rect.center()+PointF64(-dx, +dy);
    Trafo2D tr;
    tr.rotate(rect.angle(), rect.center());
    cimg_library::CImg<N16> p(4,2);
    for (N32 i = 0; i < 4; ++i) {
        quad[i] = tr.apply(quad[i]);
        p(i,0) = mathli::roundF(quad[i].x_);
        p(i,1) = mathli::roundF(quad[i].y_);
    }
    if (opacity() > 0) {
        dc_->draw_polygon(p, cimgColor(fillColor()), opacity());
    }
    dc_->draw_polygon(p, cimgColor(lineColor()), 1.0, cimgLineStyle(lineStyle()));
    if (sync_) {
        display_->display(*dc_);
    }
}

void
CImgBoard::drawRegion(Region const & reg)
{
    using ipl::Region;
    if (reg.empty())
        return;
    Board::Guard guard(this);

    bool oldsync = sync_;
    sync_ = false;
    if (opacity() > 0) {
        auto scan = reg.begin();
        auto const last = reg.end();
        UN8 const * fc = cimgColor(fillColor());
        UN32 ls = cimgLineStyle(Board::Solid);
        for (N32 i = 0; scan != last; ++scan, ++i) {
            N16 sx = scan->start().x_;
            N16 sy = scan->start().y_;
            N16 ex = sx + scan->len()-1;
            dc_->draw_line(sx, sy, ex, sy, fc, opacity(), ls);
        }
    }
    sync_ = oldsync;
    if (sync_) {
        display_->display(*dc_);
    }
}

void
CImgBoard::drawPolygon(Polygon<N16> const & poly)
{
    if (poly.size() == 0) {
        return;
    }
    if (poly.size() == 1) {
        this->drawPoint(*poly.begin());
        return;
    }
    cimg_library::CImg<N16> p(poly.size(),2);
    auto i = poly.begin(),
        e = poly.end();
    for (N32 c = 0; i != e; ++i, ++c) {
        p(c,0) = i->x_;
        p(c,1) = i->y_;
    }

    if (opacity() > 0) {
        dc_->draw_polygon(p, cimgColor(fillColor()), opacity());
    }
    dc_->draw_polygon(p, cimgColor(lineColor()), 1.0, cimgLineStyle(lineStyle()));
    if (sync_) {
        display_->display(*dc_);
    }
}

void
CImgBoard::drawPolygon(Polygon<F64> const & poly)
{
    if (poly.size() == 0) {
        return;
    }
    if (poly.size() == 1) {
        this->drawPoint(*poly.begin());
        return;
    }
    cimg_library::CImg<N32> p(poly.size(),2);
    auto i = poly.begin(),
        e = poly.end();
    for (N32 c = 0; i != e; ++i, ++c) {
        p(c,0) = mathli::roundF(i->x_);
        p(c,1) = mathli::roundF(i->y_);
    }

    if (opacity() > 0) {
        dc_->draw_polygon(p, cimgColor(fillColor()), opacity());
    }
    dc_->draw_polygon(p, cimgColor(lineColor()), 1.0, cimgLineStyle(lineStyle()));
    if (sync_) {
        display_->display(*dc_);
    }
}

void
CImgBoard::drawAnnulusXy(F64 x, F64 y, F64 r1, F64 r2)
{
    Board::Guard guard(this);
    bool oldsync = sync_;
    sync_ = false;

    if (opacity() > 0) {
        Region const reg(Region(ipl::Circle(PointF64(x, y), r1))
            .subtract(Region(ipl::Circle(PointF64(x, y), r2))));
        drawRegion(reg);
    }
    opacity(0);
    drawCircleXy(x, y, r1);
    drawCircleXy(x, y, r2);

    sync_ = oldsync;
    if (sync_) {
        display_->display(*dc_);
    }
}

/*! @todo @a opacity() unterstützen?
 */
void
CImgBoard::drawText(PointF64 const & pnt, std::string const & text)
{
    dc_->draw_text(pnt.x_, pnt.y_, text.c_str(), cimgColor(lineColor()));
    if (sync_) {
        display_->display(*dc_);
    }
}

CImgBoard &
CImgBoard::add(Board const & /*other*/)
{
    throw NotImplementedYetError(IPL_FNC_NAME);
}

void
CImgBoard::clearOverlay()
{
    this->setDc();
    if (sync_) {
        display_->display(*dc_);
    }
}

void
CImgBoard::setDc()
{
    UN32 sx = image_->size().x_,
         sy = image_->size().y_,
         sz = sx*sy;

    UN8 * buf = new UN8[sx*sy*3];
    std::copy(image_->begin(), image_->end(), buf);
    std::copy(image_->begin(), image_->end(), buf+sz);
    std::copy(image_->begin(), image_->end(), buf+2*sz);

    dc_->assign(buf, sx, sy, 1, 3);
    delete [] buf;
}

UN8 const *
CImgBoard::cimgColor(Board::Color col) {
    static struct {
        UN8 color[3];
        char const name[20];
    }colorTable[] = {
        {{0xff, 0x00, 0x00}, "red"},
        {{0x00, 0xff, 0x00}, "green"},
        {{0x00, 0x00, 0xff}, "blue"},
        {{0xff, 0x00, 0xff}, "magenta"},
        {{0xff, 0xff, 0x00}, "yellow"},
        {{0x00, 0xff, 0xff}, "cyan"},
        {{0xda, 0xa5, 0x20}, "goldenrod"},
        {{0xff, 0xc8, 0xcb}, "pink"},
        {{0xff, 0xa5, 0x00}, "orange"},
        {{0xff, 0x7f, 0x50}, "coral"},
        {{0x40, 0xe0, 0xd0}, "turquoise"},
        {{0x80, 0x00, 0x80}, "purple"},
        {{0x4b, 0x00, 0x82}, "indigo"},
        {{0xee, 0x82, 0xee}, "violet"},
        {{0x80, 0x00, 0x00}, "maroon"},
        {{0x00, 0x00, 0x80}, "navy"},
        {{0x00, 0x00, 0x00}, "black"},
        {{0x80, 0x80, 0x80}, "gray"},
        {{0xd3, 0xd3, 0xd3}, "lightgray"},
        {{0xa9, 0xa9, 0xa9}, "darkgray"},
        {{0x2f, 0x4f, 0x4f}, "darkestgray"},
        {{0x00, 0xff, 0x7f}, "lightgreen"},
        {{0xff, 0x63, 0x47}, "lightred"},
        {{0xff, 0xff, 0xff}, "white"},
    };
    if (col < 0 || col > Board::White) {
        col = Board::White;
    }
    return colorTable[col].color;
}

UN32
CImgBoard::cimgLineStyle(Board::LineStyle style)
{
    switch (style) {
    case Board::Solid:
        return 0xffffffff; // 11111111...
    case Board::Dotted:
        return 0xaaaaaaaa; // 11001100...
    case Board::DashDotted:
        return 0xc9e4f279; // 11001001111001001111001001111001
    default:
        return 0xffffffff;
    }
}

IPL_ANON_NS_BEGIN

//! Calculate @a min and @a max grayvalue of an image @a img.
void min_max(PictImg<N16> const & img,
             N16 & mi, N16 & ma)
{
    mi = *std::min_element(img.begin(), img.end());
    ma = *std::max_element(img.begin(), img.end());
}

IPL_ANON_NS_END

void
CImgBoard::conv2UN8(PictImg<N16> const & img)
{
    N16 mi, ma;
    min_max(img, mi, ma);
    if (mi == ma) {
        image_->fill(0);
    } else {
        namespace bl = boost::lambda;
        F64 b = 255.0 / (ma-mi);
        F64 a = - mi * b;
        std::transform(img.begin(), img.end(), image_->begin(), a + b * bl::_1);
    }
}

IPL_NS_END
