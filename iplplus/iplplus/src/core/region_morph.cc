/*****************************************************************//**
 *
 * @file
 * @author Gregor Ehrensperger
 * @date   Sep 25 2012
 * @par    Copyright: &copy; Datacon 2008-2012
 *              all rights reserved
 *
 * $Id: $
 *
 * @brief  implementation of various morphological operators
 *
 ********************************************************************/

#include "ipl/region.hh"
#include "ipl/log.hh"
#include "ipl/pict.hh"
#include "ipl/cimgboard.hh"
#include <list>
#include <algorithm>
#include <iostream>
#include "ipl/timer.hh"
#include <ios>
#include <string>

using namespace std;

IPL_NS_BEGIN

/**
 * a structure to represent the erosion-transform value at a given point
 */
struct Region::erosTransPoint {
	Point<N16> point; /**< x and y coordinate of the given point */
	int erosTrans; /**< erosion-transform-value at (x,y) */
};


/**
 * a structure to store the values returned by generateSkeletonB
 */
struct Region::RetGenerateSkeletonB {
	int lmin; /**< length of shortest run within B */
	list<erosTransPoint> skeletonB; /**< list of skeleton-points and their erosion transform values */
};


/**
 * a structure to store the values returned by generateErosionTransformX
 */
struct Region::RetGenerateErosionTransformX {
	Region Xlmin; /**< stores X with all runs removed which are shorter than L^B_min. */
	PictImageN16 erosTransXlmin; /**< erosion-transform-values of X_{L^B_min} stored in an array */
	Point<N16> translation; /**< Xlmin got translated by this value */
};

/**
 * a structure to store the values returned by generateErosionTransformX2
 */
struct Region::RetGenerateErosionTransformX2 {
	Region Xlmin; /**< stores X with all runs removed which are shorter than L^B_min. */
	PictImageN16 erosTransXlmin; /**< erosion-transform-values of X_{L^B_min} with A stored in an array */
	PictImageN16 erosTrans2Xlmin; /**< erosion-transform-values of X_{L^B_min} with A^t stored in an array */
	Point<N16> translation; /**< Xlmin got translated by this value */
};


/**
 * Generates a structuring element
 *
 * @param choice the type of structuring element to be generated.
 * @param size the size of the structuring element to be generated (square-shaped SE: width equals
 * 		       (2*size + 1), circle-shaped SE: size sets the radius, diamond-shaped SE: width equals
 * 		       (2*size + 1), line-shaped SE: width equals (2*size + 1)).
 * @return the generated structuring element as region.
 */
Region const Region::generateStructuringElement(StructuringElement choice, int size) {
	Region B;

	if (size <= 0) {
		cout << "Size has to be a positive value. Size is set to 5 now." << endl;
		size = 5;
	}

	switch(choice)
	{
		case Region::StructuringElementSquare:
		// square-shaped SE. width equals (2*size + 1)
			for (int i = -size; i <= size; ++i) {
				B.add(Rbo(Point<N16>(-size, i), 2*size + 1));
			}
		break;
		case Region::StructuringElementCircle:
			// circle-shaped SE. size sets the radius
			B = Region(Circle(Point<N16>(0,0),size));
			break;
		case Region::StructuringElementDiamond:
			// diamond-shaped SE. width of SE equals (2*size + 1)
			for (int i = -size; i <= 0; ++i) {
				B.add(Rbo(Point<N16>(-size - i, i), 2*(size + i) + 1));
			}
			for (int i = 1; i <= size; ++i) {
				B.add(Rbo(Point<N16>(-size + i, i), 2*(size - i) + 1));
			}
			break;
		case Region::StructuringElementLine:
			// line-shaped SE. width of SE equals (2*size + 1)
			B.add(Rbo(Point<N16>(-size, 0), 2*size + 1));
			break;
	}

	return B;
}


/**
 * determines the points of the skeleton of B including their erosion-transform values and
 * the length of the shortest run within B.
 * @param B run-length-encoded structuring element
 * @param transVector the input-Image B gets translated by transVector before determining
 * 		  the skeleton.
 * @return the skeleton of B plus the lengths of the according erosion transform values and
 * the length of the shortest run within B.
 */
Region::RetGenerateSkeletonB const Region::generateSkeletonB(Region B, Point<N16> transVector) {
	RetGenerateSkeletonB skeletonB;
	erosTransPoint tempErosTransPoint;
	skeletonB.lmin = 32767; // initializing L_min with maximum value

	for (auto & r : B) { // visits every run within B
		tempErosTransPoint.point = PointN16(r.start().x_ + r.len() - 1, r.start().y_); // gets the rightmost pixel within the current run
		tempErosTransPoint.erosTrans = r.len(); // erosion-transform of that particular point equals the length of the run
		tempErosTransPoint.point = tempErosTransPoint.point + transVector; // translating that point by transVector
		skeletonB.skeletonB.push_back(tempErosTransPoint); // adds that point (translated by transVector) to the skeleton including its erosion-transform value
		skeletonB.lmin = min(r.len(),skeletonB.lmin); // determines the shortest run within B.
	}

	return skeletonB;
}


/**
 * determinates the points of the skeleton from B^t including their erosion-transform values and
 * the length of the shortest run within B^t.
 * @param B run-length-encoded structuring element
 * @param transVector the input-Image B gets translated by transVector before determining
 * 		  the skeleton.
 * @return the skeleton of SE B^t plus the lengths of the according erosion
 * 		   transform values and the length of the shortest run within B^t.
 */
Region::RetGenerateSkeletonB const Region::generateSkeletonBtrans(Region B, Point<N16> transVector) {
	RetGenerateSkeletonB skeletonBtrans;
	erosTransPoint tempErosTransPoint;
	skeletonBtrans.lmin = 32767; // initializing L_min with width of B

	for (auto & r : B) {
		tempErosTransPoint.point = PointN16(-r.start().x_, -r.start().y_); // gets the leftmost pixel within the current run
		tempErosTransPoint.erosTrans = r.len(); // erosion-transform of that particular point equals the length of the run
		tempErosTransPoint.point = tempErosTransPoint.point + transVector; // translating that point by transVector
		skeletonBtrans.skeletonB.push_back(tempErosTransPoint); // adds that point to the skeleton including its erosion-transform value
		skeletonBtrans.lmin = min(r.len(),skeletonBtrans.lmin); // determinates the shortest run within B.
	}

	return skeletonBtrans;
}


/**
 * constructs X_{L_min} (drops all runs that are shorter than lmin) and generates the erosion transform of X{L_min} where X is the
 * given region.
 * @param B structuring element (needed, to determine the size of the erosion-transform-array)
 * @param lmin length of shortest run within B
 * @return X_{L_min}, the erosion transform of X_{L_min} and the value by which X_{L_min} got translated
 */
Region::RetGenerateErosionTransformX const Region::generateErosionTransformX(Region B, N16 lmin) const {

	RetGenerateErosionTransformX erosionTransformX;
	Point<N16> tr;
	N16 ycoord, j;

	// -> creates the needed 2-dimensional array to store the erosion-transform of X
	WinP Xbbox = this->boundingBox();
	WinP Bbbox = B.boundingBox();
	tr.x_ = -Xbbox.upperLeft().x_ + Bbbox.width();
	tr.y_ = -Xbbox.upperLeft().y_ + Bbbox.height();
	PictImageN16 Xlmin(Xbbox.width() + 2*(Bbbox.width()+1), Xbbox.height() + 2*(Bbbox.height()+1));
	erosionTransformX.erosTransXlmin = Xlmin;
	erosionTransformX.erosTransXlmin.fill(0); // initializes the erosion-transform of X with zeros.
	// <-

	for (auto & r : *this) {
		if (r.len() >= lmin) { // run r of X is longer than or equal to minimum sequence in B
			j = 1;
			ycoord = r.start().y_ + tr.y_;
			for (int i = r.start().x_ + tr.x_; i < r.start().x_ + tr.x_ + r.len(); ++i) {
			// fills the erosion-transform-array with values
				erosionTransformX.erosTransXlmin(i, ycoord) = j;
				j++;
			}
			erosionTransformX.Xlmin.add(Rbo(Point<N16>(r.start().x_ + tr.x_, ycoord), r.len())); // add current run to X_{L^B_min}
		}
	}

	erosionTransformX.translation = tr;

	return erosionTransformX;
}


/**
 * constructs X_{L_min} (drops all runs that are shorter than lmin) and generates the erosion transform of X_{L_min} where X
 * is the given region.
 * @param B structuring element (needed, to determine the size of the erosion-transform-array)
 * @param lmin length of shortest run within B
 * @return X_{L_min}, the erosion transform of X_{L_min} with A and A^t, the value by which X_{L_min} got translated
 */
Region::RetGenerateErosionTransformX const Region::generateErosionTransformX2(Region B, N16 lmin) const {

	N16 j, j2, ycoord;
	RetGenerateErosionTransformX erosionTransformX;
	Point<N16> tr;

	// -> creates the needed 2-dimensional array to store the erosion-transform of X
	WinP Xbbox = this->boundingBox();
	WinP Bbbox = B.boundingBox();
	tr.x_ = -Xbbox.upperLeft().x_ + Bbbox.width();
	tr.y_ = -Xbbox.upperLeft().y_ + Bbbox.height();
	PictImageN16 Xlmin(2*(Xbbox.width() + 2*(Bbbox.width()+1)), Xbbox.height() + 2*(Bbbox.height()+1));
	erosionTransformX.erosTransXlmin = Xlmin;
	erosionTransformX.erosTransXlmin.fill(0); // initializes the erosion-transform of X with zeros.
	// <-

	for (auto & r : *this) {
		if (r.len() >= lmin) { // run r of X is longer than or equal to minimum sequence in B
			j = 1;
			j2 = r.len();
			ycoord = r.start().y_ + tr.y_;
			for (int i = r.start().x_ + tr.x_; i < r.start().x_ + tr.x_ + r.len(); ++i) {
			// fills the erosion-transform-array with values
				erosionTransformX.erosTransXlmin(i<<1, ycoord) = j;
				erosionTransformX.erosTransXlmin((i<<1) + 1, ycoord) = j2;
				j++;
				j2--;
			}
			erosionTransformX.Xlmin.add(Rbo(Point<N16>(r.start().x_ + tr.x_, ycoord), r.len())); // add current run to X_{L^B_min}
		}
	}

	erosionTransformX.translation = tr;

	return erosionTransformX;
}


/**
 * constructs X_{L_min} (drops all runs that are shorter than lmin) and generates the erosion transform of X_{L_min} where X
 * is the given region.
 * @param B structuring element (needed, to determine the size of the erosion-transform-array)
 * @param lmin length of shortest run within B
 * @param lmax length of longest run within B
 * @return X_{cut}, the erosion transform of X_{L_min} with A and A^t, the value by which X_{L_min} got translated
 */
Region::RetGenerateErosionTransformX const Region::generateErosionTransformX2cut(Region B, N16 lmin, N16 lmax) const {

	N16 j, j2, ycoord;
	RetGenerateErosionTransformX erosionTransformX;
	Point<N16> tr;

	// -> creates the needed 2-dimensional array to store the erosion-transform of X
	WinP Xbbox = this->boundingBox();
	WinP Bbbox = B.boundingBox();
	tr.x_ = -Xbbox.upperLeft().x_ + Bbbox.width();
	tr.y_ = -Xbbox.upperLeft().y_ + Bbbox.height();
	PictImageN16 Xlmin(2*(Xbbox.width() + 2*(Bbbox.width()+1)), Xbbox.height() + 2*(Bbbox.height()+1));
	erosionTransformX.erosTransXlmin = Xlmin;
	erosionTransformX.erosTransXlmin.fill(0); // initializes the erosion-transform of X with zeros.
	// <-

	for (auto & r : *this) {
		if (r.len() >= lmin) { // run r of X is longer than or equal to minimum sequence in B
			j = 1;
			j2 = r.len();
			ycoord = r.start().y_ + tr.y_;
			for (int i = r.start().x_ + tr.x_; i < r.start().x_ + tr.x_ + r.len(); ++i) {
			// fills the erosion-transform-array with values
				erosionTransformX.erosTransXlmin(i<<1, ycoord) = j;
				erosionTransformX.erosTransXlmin((i<<1) + 1, ycoord) = j2;
				j++;
				j2--;
			}

			if (r.len() >= lmax) {
				erosionTransformX.Xlmin.add(Rbo(Point<N16>(r.start().x_ + tr.x_ + (lmax - 1), ycoord), r.len() - (lmax - 1))); // add current run to X_{cut}
			}
		}
	}

	erosionTransformX.translation = tr;

	return erosionTransformX;
}


/**
 * constructs X^c_{L_min} (drops all runs that are shorter than lmin) and generates the erosion transform of X^c_{L_min} where
 * X is the given region.
 * @param B structuring element (needed, to determine the size of the erosion-transform-array)
 * @param lmin length of shortest run within B
 * @return X^c_{L_min}, the erosion transform of X^c_{L_min} with A and A^t, the value by which X^c_{L_min} got translated
 */
Region::RetGenerateErosionTransformX const Region::generateErosionTransformXcomp(Region B, N16 lmin) const {

	int j, j2;
	N16 xcoord, ycoord, Xcwidth, Xcheight, xcoordtr, ycoordtr;
	RetGenerateErosionTransformX erosionTransformXc;
	Point<N16> tr;

	// -> creates the needed 2-dimensional array to store the erosion-transform of X
	WinP Xbbox = this->boundingBox();
	WinP Bbbox = B.boundingBox();
	tr.x_ = -Xbbox.upperLeft().x_ + 2*Bbbox.width();
	tr.y_ = -Xbbox.upperLeft().y_ + 2*Bbbox.height();
	Xcwidth = Xbbox.width() + 2*Bbbox.width();
	Xcheight = Xbbox.height() + 2*Bbbox.height();
	PictImageN16 Xclmin(2*(Xcwidth + 2*(Bbbox.width() + 1)), Xcheight + 2*(Bbbox.height() + 1)); // we need to embed X^c in a slightly bigger array to not cause pointer-exceptions later.
	erosionTransformXc.erosTransXlmin = Xclmin;
	erosionTransformXc.erosTransXlmin.fill(0); // initializes the erosion-transform of X with zeros.
	// <-

	ycoord = Bbbox.height();
	xcoord = Bbbox.width();
	ycoordtr = Bbbox.height();

	for (auto & r : *this) {
		xcoordtr = r.start().x_ + tr.x_; // the image gets embedded in a bigger array
		ycoordtr = r.start().y_ + tr.y_; // the image gets embedded in a bigger array
		if (ycoord < ycoordtr) { // The current run starts at a new line

			// Completes the previous line.
			if ((Xcwidth + Bbbox.width() - xcoord) >= lmin) { // The length of the investigated run is larger than the shortest run in B.
				j = 1;
				j2 = Xcwidth + Bbbox.width() - xcoord;
				for (int x = xcoord; x < Xcwidth + Bbbox.width(); ++x) {
					erosionTransformXc.erosTransXlmin(x<<1,ycoord) = j;
					erosionTransformXc.erosTransXlmin((x<<1)+1,ycoord) = j2;
					j++;
					j2--;
				}
				erosionTransformXc.Xlmin.add(Rbo(Point<N16>(xcoord, ycoord), Xcwidth  + Bbbox.width() - xcoord)); // add current run to X^c_{L^B_min}
			}

			// Fills all the empty lines (in X) between the previous run and the new run.
			if (Xcwidth >= lmin) {
				for (int y = ycoord + 1; y < ycoordtr; ++y) {
					j = 1;
					j2 = Xcwidth;
					for (int x = Bbbox.width(); x < Xcwidth + Bbbox.width(); ++x) {
						erosionTransformXc.erosTransXlmin(x<<1,y) = j;
						erosionTransformXc.erosTransXlmin((x<<1)+1,y) = j2;
						j++;
						j2--;
					}
					erosionTransformXc.Xlmin.add(Rbo(Point<N16>(Bbbox.width(), y), Xcwidth)); // add current run to X^c_{L^B_min}
				}
			}

			// Fills the beginning of the current line.
			if ((xcoordtr - Bbbox.width()) >= lmin) {
				j = 1;
				j2 = xcoordtr - Bbbox.width();
				for (int x = Bbbox.width(); x < xcoordtr; ++x) {
					erosionTransformXc.erosTransXlmin(x<<1,ycoordtr) = j;
					erosionTransformXc.erosTransXlmin((x<<1)+1,ycoordtr) = j2;
					j++;
					j2--;
				}
				erosionTransformXc.Xlmin.add(Rbo(Point<N16>(Bbbox.width(), ycoordtr), xcoordtr - Bbbox.width())); // add current run to X^c_{L^B_min}
			}

			xcoord = xcoordtr + r.len(); // beginning of the next complemented run.
			ycoord = ycoordtr;

		} else {

			// Fills the area between the last and current run.
			if ((xcoordtr - xcoord) >= lmin) {
				j = 1;
				j2 = xcoordtr - xcoord;
				for (int x = xcoord; x < xcoordtr; ++x) {
				// fills the erosion-transform-array with values
					erosionTransformXc.erosTransXlmin(x<<1,ycoordtr) = j;
					erosionTransformXc.erosTransXlmin((x<<1)+1,ycoordtr) = j2;
					j++;
					j2--;
				}
				erosionTransformXc.Xlmin.add(Rbo(Point<N16>(xcoord, ycoordtr), xcoordtr - xcoord)); // add current run to X^c_{L^B_min}
			}
			xcoord = xcoordtr + r.len();
		}
	}

	// Ends the line of the last run.
	if ((Xcwidth + Bbbox.width() - xcoord) >= lmin) {
		j = 1;
		j2 = Xcwidth + Bbbox.width() - xcoord;
		for (int x = xcoord; x < Xcwidth + Bbbox.width(); ++x) {
			erosionTransformXc.erosTransXlmin(x<<1,ycoordtr) = j;
			erosionTransformXc.erosTransXlmin((x<<1) + 1,ycoordtr) = j2;
			j2--;
			j++;
		}
		erosionTransformXc.Xlmin.add(Rbo(Point<N16>(xcoord, ycoordtr), Xcwidth + Bbbox.width() - xcoord)); // add current run to X^c_{L^B_min}
	}

	// Fills the rest of the lines (below the last run).
	if (Xcwidth >= lmin) {
		for (int y = ycoordtr + 1; y < Xcheight + Bbbox.height(); ++y) {
			j = 1;
			j2 = Xcwidth;
			for (int x = Bbbox.width(); x < Xcwidth + Bbbox.width(); ++x) {
				erosionTransformXc.erosTransXlmin(x<<1,y) = j;
				erosionTransformXc.erosTransXlmin((x<<1) + 1,y) = j2;
				j++;
				j2--;
			}
		erosionTransformXc.Xlmin.add(Rbo(Point<N16>(Bbbox.width(), y), Xcwidth)); // add current run to X^c_{L^B_min}
		}
	}

	erosionTransformXc.translation = tr;

	return erosionTransformXc;
}


/**
 * constructs X^c_{cut} (X^c cropped by the length of the longest run within B) and generates the erosion transform of
 * X^c_{L_min} where X is the given region.
 * @param B structuring element (needed, to determine the size of the erosion-transform-array)
 * @param lmin length of shortest run within B
 * @return X^c_{L_min}, the erosion transform of X^c_{L_min} with A and A^t, the value by which X^c_{L_min} got translated
 */
Region::RetGenerateErosionTransformX const Region::generateErosionTransformXcompcut(Region B, N16 lmin, N16 lmax) const {

	int j, j2;
	N16 xcoord, ycoord, Xcwidth, Xcheight, xcoordtr, ycoordtr;
	RetGenerateErosionTransformX erosionTransformXc;
	Point<N16> tr;

	// -> creates the needed 2-dimensional array to store the erosion-transform of X
	WinP Xbbox = this->boundingBox();
	WinP Bbbox = B.boundingBox();
	tr.x_ = -Xbbox.upperLeft().x_ + 2*Bbbox.width();
	tr.y_ = -Xbbox.upperLeft().y_ + 2*Bbbox.height();
	Xcwidth = Xbbox.width() + 2*Bbbox.width();
	Xcheight = Xbbox.height() + 2*Bbbox.height();
	PictImageN16 Xclmin(2*(Xcwidth + 2*(Bbbox.width() + 1)), Xcheight + 2*(Bbbox.height() + 1)); // we need to embed X^c in a slightly bigger array to not cause pointer-exceptions later.
	erosionTransformXc.erosTransXlmin = Xclmin;
	erosionTransformXc.erosTransXlmin.fill(0); // initializes the erosion-transform of X with zeros.
	// <-

	ycoord = Bbbox.height();
	xcoord = Bbbox.width();
	ycoordtr = Bbbox.height();

	for (auto & r : *this) {
		xcoordtr = r.start().x_ + tr.x_; // the image gets embedded in a bigger array
		ycoordtr = r.start().y_ + tr.y_; // the image gets embedded in a bigger array
		if (ycoord < ycoordtr) { // The current run starts at a new line

			// Completes the previous line.
			if ((Xcwidth + Bbbox.width() - xcoord) >= lmin) { // The length of the investigated run is larger than the shortest run in B.
				j = 1;
				j2 = Xcwidth + Bbbox.width() - xcoord;
				for (int x = xcoord; x < Xcwidth + Bbbox.width(); ++x) {
					erosionTransformXc.erosTransXlmin(x<<1,ycoord) = j;
					erosionTransformXc.erosTransXlmin((x<<1)+1,ycoord) = j2;
					j++;
					j2--;
				}
				if ((Xcwidth + Bbbox.width() - xcoord) >= lmax) {
					erosionTransformXc.Xlmin.add(Rbo(Point<N16>(xcoord + (lmax - 1), ycoord), Xcwidth  + Bbbox.width() - xcoord - (lmax - 1))); // add current run to X^c_{cut}
				}

			}

			// Fills all the empty lines (in X) between the previous run and the new run.
			if (Xcwidth >= lmin) {
				for (int y = ycoord + 1; y < ycoordtr; ++y) {
					j = 1;
					j2 = Xcwidth;
					for (int x = Bbbox.width(); x < Xcwidth + Bbbox.width(); ++x) {
						erosionTransformXc.erosTransXlmin(x<<1,y) = j;
						erosionTransformXc.erosTransXlmin((x<<1)+1,y) = j2;
						j++;
						j2--;
					}
					if (Xcwidth >= lmax) {
						erosionTransformXc.Xlmin.add(Rbo(Point<N16>(Bbbox.width() + (lmax - 1), y), Xcwidth - (lmax - 1))); // add current run to X^c_{cut}
					}
				}
			}

			// Fills the beginning of the current line.
			if ((xcoordtr - Bbbox.width()) >= lmin) {
				j = 1;
				j2 = xcoordtr - Bbbox.width();
				for (int x = Bbbox.width(); x < xcoordtr; ++x) {
					erosionTransformXc.erosTransXlmin(x<<1,ycoordtr) = j;
					erosionTransformXc.erosTransXlmin((x<<1)+1,ycoordtr) = j2;
					j++;
					j2--;
				}
				if ((xcoordtr - Bbbox.width()) >= lmax) {
					erosionTransformXc.Xlmin.add(Rbo(Point<N16>(Bbbox.width() + (lmax - 1), ycoordtr), xcoordtr - Bbbox.width() - (lmax - 1))); // add current run to X^c_{cut}
				}
			}

			xcoord = xcoordtr + r.len(); // beginning of the next complemented run.
			ycoord = ycoordtr;

		} else {

			// Fills the area between the last and current run.
			if ((xcoordtr - xcoord) >= lmin) {
				j = 1;
				j2 = xcoordtr - xcoord;
				for (int x = xcoord; x < xcoordtr; ++x) {
				// fills the erosion-transform-array with values
					erosionTransformXc.erosTransXlmin(x<<1,ycoordtr) = j;
					erosionTransformXc.erosTransXlmin((x<<1)+1,ycoordtr) = j2;
					j++;
					j2--;
				}

				if ((xcoordtr - xcoord) >= lmax) {
					erosionTransformXc.Xlmin.add(Rbo(Point<N16>(xcoord + (lmax - 1), ycoordtr), xcoordtr - xcoord - (lmax - 1))); // add current run to X^c_{cut}
				}
			}
			xcoord = xcoordtr + r.len();
		}
	}

	// Ends the line of the last run.
	if ((Xcwidth + Bbbox.width() - xcoord) >= lmin) {
		j = 1;
		j2 = Xcwidth + Bbbox.width() - xcoord;
		for (int x = xcoord; x < Xcwidth + Bbbox.width(); ++x) {
			erosionTransformXc.erosTransXlmin(x<<1,ycoordtr) = j;
			erosionTransformXc.erosTransXlmin((x<<1) + 1,ycoordtr) = j2;
			j2--;
			j++;
		}

		if ((Xcwidth + Bbbox.width() - xcoord) >= lmax) {
			erosionTransformXc.Xlmin.add(Rbo(Point<N16>(xcoord + (lmax - 1), ycoordtr), Xcwidth + Bbbox.width() - xcoord - (lmax - 1))); // add current run to X^c_{cut}
		}
	}

	// Fills the rest of the lines (below the last run).
	if (Xcwidth >= lmin) {
		for (int y = ycoordtr + 1; y < Xcheight + Bbbox.height(); ++y) {
			j = 1;
			j2 = Xcwidth;
			for (int x = Bbbox.width(); x < Xcwidth + Bbbox.width(); ++x) {
				erosionTransformXc.erosTransXlmin(x<<1,y) = j;
				erosionTransformXc.erosTransXlmin((x<<1) + 1,y) = j2;
				j++;
				j2--;
			}

			if (Xcwidth >= lmax) {
				erosionTransformXc.Xlmin.add(Rbo(Point<N16>(Bbbox.width() + (lmax - 1), y), Xcwidth - (lmax - 1))); // add current run to X^c_{cut}
			}
		}
	}

	erosionTransformXc.translation = tr;

	return erosionTransformXc;
}



/**
 * constructs X_{L_min} (drops all runs that are shorter than lmin) and generates the erosion transform of X_{L_min}
 * with A^t and the extended erosion transform of X_{L_min} with A where X is the given region.
 * @param B structuring element (needed, to determine the size of the erosion-transform-array)
 * @param lmin length of shortest run within B
 * @return X_{L_min}, the extended erosion transform of X_{L_min} with A and the erosion transform of X_{L_min} with A^t,
 * the value lmin by which X_{L_min} got translated
 */
Region::RetGenerateErosionTransformX const Region::generateExtErosionTransformX(Region B, N16 lmin) const {

	int j;
	int j2;
	N16 xcoord1, ytemp, xcoordtr, ycoordtr;
	RetGenerateErosionTransformX erosionTransformX;
	Point<N16> tr;

	// -> creates the needed 2-dimensional array to store the erosion-transform of X
	WinP Xbbox = this->boundingBox();
	WinP Bbbox = B.boundingBox();
	tr.x_ = -Xbbox.upperLeft().x_ + Bbbox.width();
	tr.y_ = -Xbbox.upperLeft().y_ + Bbbox.height();
	ytemp = 0;
	xcoord1 = 0;
	PictImageN16 Xlmin(2*(Xbbox.width() + 2*(Bbbox.width()+1)), Xbbox.height() + 2*(Bbbox.height()+1));
	erosionTransformX.erosTransXlmin = Xlmin;
	erosionTransformX.erosTransXlmin.fill(-32768); // initializes the extended erosion-transform of X with minimum value.
	// <-

	for (auto & r : *this) {
		ycoordtr = r.start().y_ + tr.y_;
		if (!(ytemp == ycoordtr)) {
			ytemp = ycoordtr;
			xcoord1 = 0;
		}
		xcoordtr = r.start().x_ + tr.x_;
		if (r.len() >= lmin) { // run r of X is longer than or equal to minimum sequence in B
			j = 1;
			j2 = r.len();
			for (int i = xcoordtr; i < xcoordtr + r.len(); ++i) {
			// fills the erosion-transform-array with values where (i,r->start().y_) is an element of X.
				erosionTransformX.erosTransXlmin(i<<1,ycoordtr) = j;
				erosionTransformX.erosTransXlmin((i<<1) + 1,ycoordtr) = j2;
				j++;
				j2--;
			}
			j = 0;
			for (int i = xcoordtr - 1; xcoord1 <= i; --i) {
			// fills the extended erosion-transform array with values where (i, r->start().y_) is not an element of X.
				erosionTransformX.erosTransXlmin(i<<1,ycoordtr) = j;
				j--;
			}
			erosionTransformX.Xlmin.add(Rbo(Point<N16>(xcoordtr, ycoordtr), r.len())); // add current run to X_{L^B_min}
			xcoord1 = xcoordtr + r.len();
		}
	}

	erosionTransformX.translation = tr;

	return erosionTransformX;
}


/**
 * algorithm - erosion - variant 1
 * Calculates the erosion of the given region by structuring element B.
 * Therefore it determines the skeleton of B and the length of the shortest run
 * within B. Afterwards it generates X_{L_min} (the set of runs within X which are
 * bigger than or equal to the shortest run within B), and the erosion-transform
 * of X_{L_min}. With this preprocessing-steps it is possible to apply the Jump-
 * Miss-theorem in the final step. In case of a miss, the algorithm stays in the
 * line where the miss occured, and checks (pixel by pixel) for more misses in
 * that particular line.
 *
 * uses: generateSkeletonB(), generateErosionTransformX()
 *
 * @param B the structuring element B
 * @return X eroded by B
 */
Region const Region::erode1(Region const & B) const {

	if (this->empty() || B.empty()) {
		return *this;
	}

	bool breakl;
	N16 Diff, eroStartx, xend, eroEndx;
	Point<N16> origTranslate;
	int xcoord;
	list<erosTransPoint>::iterator iterSkelB;
	Region erodedImage;

//	FILE *eOut = fopen("erode1.dat","a");															// Time-Measurement: should be commented in release
//	Timer tm;																						//
//	UN32 duration;																					//

	origTranslate = -B.rbos_.begin().base()->start(); // this vector will be used to translate B such that the structuring element contains
												 	  // its origin

	// -> preprocessing skeleton of B and erosion-transform of X plus some additional information
//    tm.tic(); // Time-Measurement: should be commented in release
	RetGenerateSkeletonB skelB = generateSkeletonB(B,origTranslate);
//    duration = tm.toc();				// Time-Measurement: should be commented in release
//    fprintf(eOut, "\t%u", duration);	//
//
//    tm.tic(); // Time-Measurement: should be commented in release
	RetGenerateErosionTransformX erosTransX = generateErosionTransformX(B,skelB.lmin);
//    duration = tm.toc();				// Time-Measurement: should be commented in release
//    fprintf(eOut, "\t%u", duration);	//
	// <-

//    tm.tic(); // Time-Measurement: should be commented in release

	Region Xlmin = erosTransX.Xlmin; // X_{L_min} as RLE image
	list<erosTransPoint> skeletonB = skelB.skeletonB; // skeleton of B plus their erosion-transform-values as list

	for (auto & r : Xlmin) {
		xcoord = r.start().x_; // basically a horizontal run-index pointing at the point to be investigated
		eroStartx = r.start().x_; // keeps track of the start-pixel of our eroded run
		eroEndx = -1; // keeps track of the end-pixel of our eroded run
		xend = r.start().x_ + r.len() - 1; // rightmost pixel of the current run
		while (xcoord <= xend) { // we are investigating the pixel at xcoord as long as it is contained in the current run
			breakl = false;
			iterSkelB = skeletonB.begin();
			while ((!breakl) && iterSkelB != skeletonB.end()) {
				// the pixel h := (xcoord, r->start().y) is contained in X eroded by B iff f^A_B(s) <= f^X_B(s + h) for all s in S^A_B.
				while ((xcoord <= xend) && ((Diff = iterSkelB->erosTrans - erosTransX.erosTransXlmin(iterSkelB->point.x_ + xcoord, iterSkelB->point.y_ + r.start().y_)) > 0)) { // Jump-And-Miss
				    // this loop is entered in case there's a miss according to the jump-miss-theorem
					breakl = true; // a miss occured
					xcoord = xcoord + Diff; // checks for more misses within the current line
				}
				iterSkelB++;
			}

			if (breakl) {
				// we had a miss or finished the investigation of the current run
				if (eroEndx >= eroStartx) {
				// since eroEndx >= eroStartx, we found a run which is subset of X eroded by B.
					erodedImage.add(Rbo(Point<N16>(eroStartx - erosTransX.translation.x_, r.start().y_ - erosTransX.translation.y_), eroEndx - eroStartx + 1));
				}
				eroStartx = xcoord; // our next eroded run starts at xcoord or >xcoord
				eroEndx = -1;
			} else {
				eroEndx = xcoord; // pixel h is a hit
				xcoord++;	// investigate the next pixel
			}
		}
		if (eroEndx >= eroStartx) {
		// since eroEndx >= eroStartx, we found a run which is subset of X eroded by B.
			erodedImage.add(Rbo(Point<N16>(eroStartx - erosTransX.translation.x_, r.start().y_ - erosTransX.translation.y_), eroEndx - eroStartx + 1));
		}
	}

//    duration = tm.toc();				// Time-Measurement: should be commented in release
//    fprintf(eOut, "\t%u", duration);	//
//    tm.tic();							//

	erodedImage.translate(origTranslate); // since the structuring element got translated by origTranslate,
										  // also the eroded Image needs to be translated by the same vector.

//    duration = tm.toc();				// Time-Measurement: should be commented in release
//    fprintf(eOut, "\t%u", duration);	//
//	  fprintf(eOut,"\n");				//
//	  fflush(eOut);						//

	return erodedImage;
}


/**
 * algorithm - erosion - variant 2
 * Calculates the erosion of the given region by structuring element B.
 * Therefore it determines the skeleton of B and the length of the shortest run
 * within B. Afterwards it generates X_{L_min} (the set of runs within X which are
 * bigger than or equal to the shortest run within B), and the erosion-transform
 * of X_{L_min} with A aswell as with A^t. With this preprocessing-steps it is possible
 * to apply the Jump-Miss- and Jump-Hit-Theorem in the final step.
 *
 * uses: generateSkeletonB(), generateErosionTransformX2cut()
 *
 * @param B the structuring element B
 * @return the region eroded by B
 */
Region const Region::erode2(Region const & B) const {

	if (this->empty() || B.empty()) {
		return *this;
	}

	bool breakl;
	int xcoord;
	N16 Diff, minDist, xend;
	Point<N16> origTranslate;
	list<erosTransPoint>::iterator iterSkelB;
	Region erodedImage;

//	FILE *eOut = fopen("erode2.dat","a");															// Time-Measurement: should be commented in release
//	Timer tm;																						//
//	UN32 duration;																					//

	origTranslate = -B.rbos_.begin().base()->start(); // this vector will be used to translate B such that the structuring element contains
												 	  // its origin

	// -> preprocessing skeleton of B and erosion-transform of X plus some additional information
//    tm.tic(); // Time-Measurement: should be commented in release
	RetGenerateSkeletonB skelB = generateSkeletonB(B,origTranslate);
//    duration = tm.toc();				// Time-Measurement: should be commented in release
//    fprintf(eOut, "\t%u", duration);	//
//
//    tm.tic(); // Time-Measurement: should be commented in release
	RetGenerateErosionTransformX erosTransX = generateErosionTransformX2(B,skelB.lmin);
//    duration = tm.toc();				// Time-Measurement: should be commented in release
//    fprintf(eOut, "\t%u", duration);	//
	// <-

//    tm.tic(); // Time-Measurement: should be commented in release
	Region Xlmin = erosTransX.Xlmin; // X_{L_min} as RLE image
	list<erosTransPoint> skeletonB = skelB.skeletonB; // skeleton of B plus their erosion-transform-values as list

	for (auto & r : Xlmin) {
		xcoord = r.start().x_; // basically a horizontal run-index pointing at the point to be investigated
		xend = r.start().x_ + r.len() - 1; // rightmost pixel of the current run
		while (xcoord <= xend) { // we are investigating the pixel at xcoord as long as it is contained in the current run
			breakl = false;
			iterSkelB = skeletonB.begin();
			while ((!breakl) && iterSkelB != skeletonB.end()) {
				// the pixel h := (xcoord, r->start().y) is contained in X eroded by B iff f^A_B(s) <= f^X_B(s + h) for all s in S^A_B.
				while ((xcoord <= xend) && ((Diff = iterSkelB->erosTrans - erosTransX.erosTransXlmin((iterSkelB->point.x_ + xcoord)<<1, iterSkelB->point.y_ + r.start().y_)) > 0)) { // Jump-And-Miss
				    // this loop is entered in case there's a miss according to the jump-miss-theorem
					breakl = true; // a miss occured
					xcoord = xcoord + Diff; // checks for more misses within the current line
				}
				iterSkelB++;
			}

			if (!breakl) {
				minDist = 32767; // minDist is set to maximum value;
				for (list<erosTransPoint>::const_iterator iterSkel = skeletonB.begin(); iterSkel != skeletonB.end(); ++iterSkel) {
					minDist = min(minDist,erosTransX.erosTransXlmin(((iterSkel->point.x_ + xcoord)<<1)+1, iterSkel->point.y_ + r.start().y_)); // this variable is needed to apply the jump-hit-theorem
				}

				// apply jump-hit-theorem
				erodedImage.add(Rbo(Point<N16>(xcoord - erosTransX.translation.x_, r.start().y_ - erosTransX.translation.y_), minDist));
				xcoord = xcoord + minDist + 1; // our next eroded run starts at xcoord or >xcoord
			}
		}
	}

//    duration = tm.toc();				// Time-Measurement: should be commented in release
//    fprintf(eOut, "\t%u", duration);	//
//    tm.tic(); 							//

	erodedImage.translate(origTranslate); // since the structuring element got translated by origTranslate,
										  // also the eroded Image needs to be translated by the same vector.

//	duration = tm.toc();				// Time-Measurement: should be commented in release
//  fprintf(eOut, "\t%u", duration);	//
//	fprintf(eOut,"\n");					//
//	fflush(eOut);						//

	return erodedImage;
}

/**
 * algorithm - erosion - variant 2 cut
 * Calculates the erosion of the given region by structuring element B.
 * Therefore it determines the skeleton of B and the length of the shortest aswell as
 * the longest run within B. Afterwards it generates X_cut (the set of runs within X
 * cropped by the length of the longest run within B), and the erosion-transform
 * of X_{L_min} with A aswell as with A^t. With this preprocessing-steps it is
 * possible to apply the Jump-Miss- and Jump-Hit-Theorem in the final step.
 *
 * uses: generateSkeletonB(), generateErosionTransformX2()
 *
 * @param B the structuring element B
 * @return the region eroded by B
 */
Region const Region::erode2cut(Region const & B) const {

	if (this->empty() || B.empty()) {
		return *this;
	}

	bool breakl;
	int xcoord;
	N16 Diff, minDist, xend, lmax;
	Point<N16> origTranslate;
	list<erosTransPoint>::iterator iterSkelB;
	Region erodedImage;

//	FILE *eOut = fopen("erode2.dat","a");															// Time-Measurement: should be commented in release
//	Timer tm;																						//
//	UN32 duration;

	lmax = 0; // initializing L_max with minimum value

	for (auto & r : B) { // visits every run within B
		if (r.len() > lmax) {
			lmax = r.len();
			origTranslate = -Point<N16>(r.start().x_ + r.len() - 1, r.start().y_); // this vector will be used to translate B such that the structuring
																				 //  element contains its origin
		}
	}

	// -> preprocessing skeleton of B and erosion-transform of X plus some additional information
//    tm.tic(); // Time-Measurement: should be commented in release
	RetGenerateSkeletonB skelB = generateSkeletonB(B,origTranslate);
//    duration = tm.toc();				// Time-Measurement: should be commented in release
//    fprintf(eOut, "\t%u", duration);	//
//
//    tm.tic(); // Time-Measurement: should be commented in release
	RetGenerateErosionTransformX erosTransX = generateErosionTransformX2cut(B,skelB.lmin,lmax);
//    duration = tm.toc();				// Time-Measurement: should be commented in release
//    fprintf(eOut, "\t%u", duration);	//
	// <-

//    tm.tic(); // Time-Measurement: should be commented in release
	Region Xlmin = erosTransX.Xlmin; // X_{L_min} as RLE image
	list<erosTransPoint> skeletonB = skelB.skeletonB; // skeleton of B plus their erosion-transform-values as list

	for (auto & r : Xlmin) {
		xcoord = r.start().x_; // basically a horizontal run-index pointing at the point to be investigated
		xend = r.start().x_ + r.len() - 1; // rightmost pixel of the current run
		while (xcoord <= xend) { // we are investigating the pixel at xcoord as long as it is contained in the current run
			breakl = false;
			iterSkelB = skeletonB.begin();
			while ((!breakl) && iterSkelB != skeletonB.end()) {
				// the pixel h := (xcoord, r->start().y) is contained in X eroded by B iff f^A_B(s) <= f^X_B(s + h) for all s in S^A_B.
				while ((xcoord <= xend) && ((Diff = iterSkelB->erosTrans - erosTransX.erosTransXlmin((iterSkelB->point.x_ + xcoord)<<1, iterSkelB->point.y_ + r.start().y_)) > 0)) { // Jump-And-Miss
				    // this loop is entered in case there's a miss according to the jump-miss-theorem
					breakl = true; // a miss occured
					xcoord = xcoord + Diff; // checks for more misses within the current line
				}
				iterSkelB++;
			}

			if (!breakl) {
				minDist = 32767; // minDist is set to maximum value;
				for (list<erosTransPoint>::const_iterator iterSkel = skeletonB.begin(); iterSkel != skeletonB.end(); ++iterSkel) {
					minDist = min(minDist,erosTransX.erosTransXlmin(((iterSkel->point.x_ + xcoord)<<1)+1, iterSkel->point.y_ + r.start().y_)); // this variable is needed to apply the jump-hit-theorem
				}

				// apply jump-hit-theorem
				erodedImage.add(Rbo(Point<N16>(xcoord - erosTransX.translation.x_, r.start().y_ - erosTransX.translation.y_), minDist));
				xcoord = xcoord + minDist + 1; // our next eroded run starts at xcoord or >xcoord
			}
		}
	}

//    duration = tm.toc();				// Time-Measurement: should be commented in release
//    fprintf(eOut, "\t%u", duration);	//
//    tm.tic(); 							//

	erodedImage.translate(origTranslate); // since the structuring element got translated by origTranslate,
										  // also the eroded Image needs to be translated by the same vector.

//	duration = tm.toc();				// Time-Measurement: should be commented in release
//  fprintf(eOut, "\t%u", duration);	//
//	fprintf(eOut,"\n");					//
//	fflush(eOut);						//

	return erodedImage;
}


/**
 * algorithm - variant 3
 * Calculates the erosion of the given region by structuring element B.
 * Therefore it determines the skeleton of B and the length of the shortest run
 * within B. Afterwards it generates X_{L_min} (the set of runs within X which are
 * bigger than or equal to the shortest run within B), and the extended erosion-transform
 * of X_{L_min} with A aswell as with A^t. With this preprocessing-steps it is possible
 * to apply the Jump-Miss- and Jump-Hit-Theorem in the final step, where all jumps are
 * maximal.
 *
 * uses: generateSkeletonB(), generateErosionTransformX2()
 *
 * @param B the structuring element B
 * @return the region eroded by B
 */
Region const Region::erode3(Region const & B) const {

	if (this->empty() || B.empty()) {
		return *this;
	}

	N16 xend, minDist;
	Point<N16> origTranslate;
	int xcoord, Diff;
	list<erosTransPoint>::iterator iterSkelB;
	Region erodedImage;
	bool breakl;

//	FILE *eOut = fopen("erode3.dat","a");																// Time-Measurement: should be commented in release
//	Timer tm;																							//
//	UN32 duration;																						//

	origTranslate = -B.rbos_.begin().base()->start(); // this vector will be used to translate B such that the structuring element contains
												 	  // its origin

	// -> preprocessing skeleton of B and erosion-transform of X plus some additional information
//    tm.tic(); 							// Time-Measurement: should be commented in release
	RetGenerateSkeletonB skelB = generateSkeletonB(B,origTranslate);
//    duration = tm.toc();				// Time-Measurement: should be commented in release
//    fprintf(eOut, "\t%u", duration);	//
//    tm.tic(); 							// Time-Measurement: should be commented in release
	RetGenerateErosionTransformX erosTransX = generateExtErosionTransformX(B,skelB.lmin);
//    duration = tm.toc();				// Time-Measurement: should be commented in release
//    fprintf(eOut, "\t%u", duration);	//
	// <-

//    tm.tic(); 							// Time-Measurement: should be commented in release
	Region Xlmin = erosTransX.Xlmin; // X_{L_min} as RLE image
	list<erosTransPoint> skeletonB = skelB.skeletonB; // skeleton of B plus their erosion-transform-values as list

	for (auto & r : Xlmin) {
		xcoord = r.start().x_; // basically a horizontal run-index pointing at the point to be investigated
		xend = r.start().x_ + r.len() - 1; // rightmost pixel of the current run
		while (xcoord <= xend) { // we are investigating the pixel at xcoord as long as it is contained in the current run
			breakl = false;
			iterSkelB = skeletonB.begin();
			while ((!breakl) && iterSkelB != skeletonB.end()) {
				// the pixel h := (xcoord, r->start().y) is contained in X eroded by B iff f^A_B(s) <= f^X_B(s + h) for all s in S^A_B.
				Diff = iterSkelB->erosTrans - erosTransX.erosTransXlmin((iterSkelB->point.x_ + xcoord)<<1, iterSkelB->point.y_ + r.start().y_);  // Jump-And-Miss
				if (Diff > 0) { // this if-clause is entered in case there's a miss according to the jump-miss-theorem
					breakl = true; // a miss occured
					xcoord = xcoord + Diff; // checks for more misses within the current line
				}
				iterSkelB++;
			}

			if (!breakl) {
				minDist = 32767; // minDist is set to maximum value;
				for (list<erosTransPoint>::const_iterator iterSkel = skeletonB.begin(); iterSkel != skeletonB.end(); ++iterSkel) {
					minDist = min(minDist,erosTransX.erosTransXlmin(((iterSkel->point.x_ + xcoord)<<1)+1, iterSkel->point.y_ + r.start().y_)); // this variable is needed to apply the jump-hit-theorem
				}

				// apply jump-hit-theorem
				erodedImage.add(Rbo(Point<N16>(xcoord - erosTransX.translation.x_, r.start().y_ - erosTransX.translation.y_), minDist));
				xcoord = xcoord + minDist + 1; // our next eroded run starts at xcoord or >xcoord
			}
		}
	}

//    duration = tm.toc();				// Time-Measurement: should be commented in release
//    fprintf(eOut, "\t%u", duration);	//
//    tm.tic(); 							//

	erodedImage.translate(origTranslate); // since the structuring element got translated by origTranslate,
										  // also the eroded Image needs to be translated by the same vector.

//    duration = tm.toc();				// Time-Measurement: should be commented in release
//    fprintf(eOut, "\t%u", duration);	//
//	  fprintf(eOut,"\n");				//
//	  fflush(eOut);						//

	return erodedImage;
}


/**
 * algorithm - dilation
 * Calculates the dilation of the object X with structuring element B.
 * Therefore it determines the skeleton of B^t and the length of the shortest run
 * within B. Afterwards it generates X^c_{L_min} (the set of runs within X^c which are
 * bigger than or equal to the shortest run within B), and the erosion-transform
 * of X^c_{L_min} with A aswell as with A^t. With this preprocessing-steps it is possible
 * to apply the Jump-Miss- and Jump-Hit-Theorem in the final step.
 *
 * uses: generateSkeletonBtrans(), generateErosionTransformXcomp()
 *
 * @param B the structuring element B
 * @return the region dilated by B
 */
Region const Region::dilate(Region const & B) const {

	if (this->empty() || B.empty()) {
		return *this;
	}

	bool breakl, firstrun;
	int xcoord, Diff;
	N16 xend, minDist, runbegin, ytemp;
	Point<N16> origTranslate;
	list<erosTransPoint>::iterator iterSkelB;
	Region dilatedImage;

//	FILE *eOut = fopen("dilate.dat","a");																		// Time-Measurement: should be commented in release
//	Timer tm;																									//
//	UN32 duration;																								//

	origTranslate = B.rbos_.begin().base()->start(); // this vector will be used to translate B such that the structuring element contains
												 	  // its origin

	// -> preprocessing skeleton of B^t and erosion-transform of X^c plus some additional information
//    tm.tic(); 							// Time-Measurement: should be commented in release
	RetGenerateSkeletonB skelBtrans = generateSkeletonBtrans(B,origTranslate);
//    duration = tm.toc();				// Time-Measurement: should be commented in release
//    fprintf(eOut, "\t%u", duration);	//
//    tm.tic(); 							// Time-Measurement: should be commented in release
	RetGenerateErosionTransformX erosTransXc = generateErosionTransformXcomp(B,skelBtrans.lmin);
//    duration = tm.toc();				// Time-Measurement: should be commented in release
//    fprintf(eOut, "\t%u", duration);	//
	// <-

//    tm.tic(); 							// Time-Measurement: should be commented in release

	Region Xclmin = erosTransXc.Xlmin; // X^c_{L_min} as RLE image
	list<erosTransPoint> skeletonBtrans = skelBtrans.skeletonB; // skeleton of B plus their erosion-transform-values as list

	ytemp = 0;
	firstrun = true;
	runbegin = 0;

	for (auto & r : Xclmin) {
		xcoord = r.start().x_; // basically a horizontal run-index pointing at the point to be investigated
		xend = r.start().x_ + r.len() - 1; // rightmost pixel of the current run
		if (ytemp != r.start().y_) {
			firstrun = true;
			ytemp = r.start().y_;
		}
		while (xcoord <= xend) { // we are investigating the pixel at xcoord as long as it is contained in the current run
			breakl = false;
			iterSkelB = skeletonBtrans.begin();
			while ((!breakl) && iterSkelB != skeletonBtrans.end()) {
				// the pixel h := (xcoord, r->start().y) is contained in X^c eroded by B^t iff f^A_B(s) <= f^X_B(s + h) for all s in S^A_B.
				while ((xcoord <= xend) && ((Diff = iterSkelB->erosTrans - erosTransXc.erosTransXlmin((iterSkelB->point.x_ + xcoord)<<1, iterSkelB->point.y_ + r.start().y_)) > 0)) { // Jump-And-Miss
				    // this loop is entered in case there's a miss according to the jump-miss-theorem
					breakl = true; // a miss occured
					xcoord = xcoord + Diff; // checks for more misses within the current line
				}
				iterSkelB++;
			}

			if (!breakl) {
				minDist = 32767; // minDist is set to maximum value;
				for (list<erosTransPoint>::const_iterator iterSkel = skeletonBtrans.begin(); iterSkel != skeletonBtrans.end(); ++iterSkel) {
					minDist = min(minDist,erosTransXc.erosTransXlmin(((iterSkel->point.x_ + xcoord)<<1)+1, iterSkel->point.y_ + r.start().y_)); // this variable is needed to apply the jump-hit-theorem
				}
				if (!firstrun) {
					dilatedImage.add(Rbo(Point<N16>(runbegin - erosTransXc.translation.x_, r.start().y_ - erosTransXc.translation.y_), xcoord - runbegin)); // Misses in X^c are 1-runs in (delta_B(X).
				}
				runbegin = xcoord + minDist;
				firstrun = false;
				xcoord = xcoord + minDist + 1; // our next hit occurs at xcoord or >xcoord
			}
		}
	}


//    duration = tm.toc();				// Time-Measurement: should be commented in release
//    fprintf(eOut, "\t%u", duration);	//
//    tm.tic(); 						//

	dilatedImage.translate(origTranslate);	  // since the structuring element got translated by origTranslate,
	  	  	  	  	  	  	  	  	  	  	  // also the dilated Image needs to be translated by the same vector.

//    duration = tm.toc();				// Time-Measurement: should be commented in release
//    fprintf(eOut, "\t%u", duration);	//
//	  fprintf(eOut,"\n");				//
//	  fflush(eOut);						//

	return dilatedImage;
}


/**
 * algorithm - dilation - cut
 * Calculates the dilation of the object X with structuring element B.
 * Therefore it determines the skeleton of B^t and the length of the shortest run
 * within B. Afterwards it generates X^c_{cut} (the set of runs within X^c cropped
 * by the length of the longest run within B), and the erosion-transform
 * of X^c_{L_min} with A aswell as with A^t. With this preprocessing-steps it is possible
 * to apply the Jump-Miss- and Jump-Hit-Theorem in the final step.
 *
 * uses: generateSkeletonBtrans(), generateErosionTransformXcompcut()
 *
 * @param B the structuring element B
 * @return the region dilated by B
 */
Region const Region::dilatecut(Region const & B) const {

	if (this->empty() || B.empty()) {
		return *this;
	}

	bool breakl, firstrun;
	int xcoord, Diff;
	N16 xend, minDist, runbegin, ytemp, lmax;
	Point<N16> origTranslate;
	list<erosTransPoint>::iterator iterSkelB;
	Region dilatedImage;

//	FILE *eOut = fopen("dilate.dat","a");																		// Time-Measurement: should be commented in release
//	Timer tm;																									//
//	UN32 duration;

	lmax = 0;

	for (auto & r : B) { // visits every run within B
		if (r.len() > lmax) {
			lmax = r.len();
			origTranslate = Point<N16>(r.start().x_, r.start().y_);  // this vector will be used to translate B such that the structuring
																			   	   	  //  element contains its origin
		}
	}

//	origTranslate = B.rbos_.begin().base()->start(); // this vector will be used to translate B such that the structuring element contains
												 	  // its origin

	// -> preprocessing skeleton of B^t and erosion-transform of X^c plus some additional information
//    tm.tic(); 							// Time-Measurement: should be commented in release
	RetGenerateSkeletonB skelBtrans = generateSkeletonBtrans(B,origTranslate);
//    duration = tm.toc();				// Time-Measurement: should be commented in release
//    fprintf(eOut, "\t%u", duration);	//
//    tm.tic(); 							// Time-Measurement: should be commented in release
	RetGenerateErosionTransformX erosTransXc = generateErosionTransformXcompcut(B,skelBtrans.lmin,lmax);
//    duration = tm.toc();				// Time-Measurement: should be commented in release
//    fprintf(eOut, "\t%u", duration);	//
	// <-

//    tm.tic(); 							// Time-Measurement: should be commented in release

	Region Xclmin = erosTransXc.Xlmin; // X^c_{L_min} as RLE image
	list<erosTransPoint> skeletonBtrans = skelBtrans.skeletonB; // skeleton of B plus their erosion-transform-values as list

	ytemp = 0;
	firstrun = true;
	runbegin = 0;

	for (auto & r : Xclmin) {
		xcoord = r.start().x_; // basically a horizontal run-index pointing at the point to be investigated
		xend = r.start().x_ + r.len() - 1; // rightmost pixel of the current run
		if (ytemp != r.start().y_) {
			firstrun = true;
			ytemp = r.start().y_;
		}
		while (xcoord <= xend) { // we are investigating the pixel at xcoord as long as it is contained in the current run
			breakl = false;
			iterSkelB = skeletonBtrans.begin();
			while ((!breakl) && iterSkelB != skeletonBtrans.end()) {
				// the pixel h := (xcoord, r->start().y) is contained in X^c eroded by B^t iff f^A_B(s) <= f^X_B(s + h) for all s in S^A_B.
				while ((xcoord <= xend) && ((Diff = iterSkelB->erosTrans - erosTransXc.erosTransXlmin((iterSkelB->point.x_ + xcoord)<<1, iterSkelB->point.y_ + r.start().y_)) > 0)) { // Jump-And-Miss
				    // this loop is entered in case there's a miss according to the jump-miss-theorem
					breakl = true; // a miss occured
					xcoord = xcoord + Diff; // checks for more misses within the current line
				}
				iterSkelB++;
			}

			if (!breakl) {
				minDist = 32767; // minDist is set to maximum value;
				for (list<erosTransPoint>::const_iterator iterSkel = skeletonBtrans.begin(); iterSkel != skeletonBtrans.end(); ++iterSkel) {
					minDist = min(minDist,erosTransXc.erosTransXlmin(((iterSkel->point.x_ + xcoord)<<1)+1, iterSkel->point.y_ + r.start().y_)); // this variable is needed to apply the jump-hit-theorem
				}
				if (!firstrun) {
					dilatedImage.add(Rbo(Point<N16>(runbegin - erosTransXc.translation.x_, r.start().y_ - erosTransXc.translation.y_), xcoord - runbegin)); // Misses in X^c are 1-runs in (delta_B(X).
				}
				runbegin = xcoord + minDist;
				firstrun = false;
				xcoord = xcoord + minDist + 1; // our next hit occurs at xcoord or >xcoord
			}
		}
	}


//    duration = tm.toc();				// Time-Measurement: should be commented in release
//    fprintf(eOut, "\t%u", duration);	//
//    tm.tic(); 						//

	dilatedImage.translate(origTranslate);	  // since the structuring element got translated by origTranslate,
	  	  	  	  	  	  	  	  	  	  	  // also the dilated Image needs to be translated by the same vector.

//    duration = tm.toc();				// Time-Measurement: should be commented in release
//    fprintf(eOut, "\t%u", duration);	//
//	  fprintf(eOut,"\n");				//
//	  fflush(eOut);						//

	return dilatedImage;
}

IPL_NS_END
