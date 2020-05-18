/*****************************************************************//**
 *
 * @file
 * @author dast
 * @date   Feb 11 2009
 * @par    Copyright: &copy; Datacon 2008-2012
 *              all rights reserved
 *
 * $Id: board.cc 1 2012-09-05 10:26:59Z fesc $
 *
 * @brief  Implementation for ipl::Board
 *
 ********************************************************************/

#include "ipl/board.hh"

#include "ipl/polygon.hh"

IPL_NS_BEGIN

Board::State::State()
    : lineColor_(Board::White),
      fillColor_(Board::Black),
      lineStyle_(Board::Solid),
      opacity_(0),
      lineScaling_(false),
      markerSize_(1),
      fontFamily_(monospace),
      fontSize_(16)
{}

Board::Board()
{}

Board::~Board()
{}

void
Board::push()
{
    states_.push(state_);
}

void
Board::pop()
{
    if (states_.empty())
        return;
    state_ = states_.top();
    states_.pop();
}

void
Board::drawMarkerXy(F64 x, F64 y, MarkerStyle style)
{
    Board::Guard guard(this);
    lineScaling(false);
    switch (style) {
        case Board::Cross:
        case Board::Star:
        {
            drawLineXy(x - markerSize() / 2, y, x + markerSize() / 2, y);
            drawLineXy(x, y - markerSize() / 2, x, y + markerSize() / 2);
        }
        if (style == Board::Cross)
            break;
        case Board::Ex:
        {
            F64 const d = markerSize() / (2 * mathli::SqrtTwo);
            drawLineXy(x - d, y - d, x + d, y + d);
            drawLineXy(x + d, y - d, x - d, y + d);
        }
        break;
        case Board::Circle:
        {
            drawCircleXy(x, y, markerSize() / 2);
        }
        break;
        case Board::Triangle:
        {
            F64 const h = mathli::sqrt(3) / 2 * markerSize();
            PolygonF64 poly;
            poly.push_back(PointF64(x, y - 2 * h / 3));
            poly.push_back(PointF64(x - markerSize() / 2, y + h / 3));
            poly.push_back(PointF64(x + markerSize() / 2, y + h / 3));
            drawPolygon(poly);
        }
        break;
        case Board::Diamond:
        {
            PolygonF64 poly;
            poly.push_back(PointF64(x, y - markerSize() / 2));
            poly.push_back(PointF64(x - markerSize() / 2, y));
            poly.push_back(PointF64(x, y + markerSize() / 2));
            poly.push_back(PointF64(x + markerSize() / 2, y));
            drawPolygon(poly);
        }
        break;
        case Board::Dot:
        {
            drawCircleXy(x, y, markerSize() / 6);
        }
        break;
        default:
        {
            IPL_ASSERT(!"Invalid marker style.");
        }
        break;
    }
}


/*************************************************************************/

Board::Guard::Guard(Board * board) :
    board_(board)
{
    if (board_) {
        board_->push();
    }
}

Board::Guard::~Guard()
{
    if (board_) {
        board_->pop();
    }
}

IPL_NS_END
