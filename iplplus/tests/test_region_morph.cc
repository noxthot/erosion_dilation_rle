/*****************************************************************//**
 *
 * @file
 * @author Gregor Ehrensperger
 * @date   Jul 13 2012
 * @par    Copyright: &copy; Datacon 2008-2012
 *              all rights reserved
 *
 * $Id: test_region_morph.cc 1 2012-09-05 10:26:59Z fesc $
 *
 * @brief  Unittest for class Region_morph
 *
 ********************************************************************/

#include "config.hh"

#include <algorithm>
#include <string>
#include <boost/array.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/archive/xml_iarchive.hpp>

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/XmlOutputter.h>

#include "ipl/region.hh"
#include "ipl/circle.hh"

using namespace ipl;
using namespace std;

class RegionMorphTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(RegionMorphTest);
    CPPUNIT_TEST(testEmptyPicture);
    CPPUNIT_TEST(testEqualityOperator);
    CPPUNIT_TEST(testSinglePointSE);
    CPPUNIT_TEST(testImgErodedByItself);
    CPPUNIT_TEST(testPointDilatedByImage);
    CPPUNIT_TEST_SUITE_END();
public:
    void testEmptyPicture();
    void testEqualityOperator();
    void testSinglePointSE();
    void testImgErodedByItself();
    void testPointDilatedByImage();

};

/**
 * Checks whether or not LHS equals RHS.
 */
bool operator == (Region const & LHS, Region const & RHS) {

	if (LHS.nrRbos() != RHS.nrRbos()) {
		return false;
	}

	auto r = RHS.begin();

	for(auto & l : LHS) {
		if ((l.len() != r->len()) || (l.start() != r->start())) {
			return false;
		}
		++r;
	}

	return true;
}

int testIterations = 10; // the number of test iterations


void
RegionMorphTest::testEmptyPicture()
{
	int rdmInteger1, rdmInteger2, rdmInteger3, rdmInteger4;
	std::stringstream errormessage;
	Region B1, B2, B3;
	Region X;
	Region erodedImage1, erodedImage2, erodedImage2cut, erodedImage3, dilatedImage1, dilatedImage2, dilatedImage3, dilatedImage1cut, dilatedImage2cut, dilatedImage3cut;
	srand(time(NULL));

	for (int i = 1; i < testIterations; i++) {
		rdmInteger1 = (rand()%500);
		rdmInteger2 = (rand()%500);
		rdmInteger3 = (rand()%100);
		rdmInteger4 = (rand()%100);

		errormessage << "Test failed for rdmInteger1 = " << rdmInteger1 << " - rdmInteger2 = " << rdmInteger2 << " -  rdmInteger3 = " << rdmInteger3 << " - rdmInteger4 = " << rdmInteger4 << endl;

		B1 = Region(Circle(PointF64(rdmInteger3, rdmInteger4), rdmInteger1));
		B2 = Region(Rect(PointF64(-rdmInteger3, -rdmInteger4), PointF64(rdmInteger2, rdmInteger3), Angle(rdmInteger4, Angle::InDeg)));
		B3 = B1.unions(B2);


		erodedImage1 = X.erode1(B3);
		erodedImage2 = X.erode2(B3);
		erodedImage2cut = X.erode2cut(B3);
		erodedImage3 = X.erode3(B3);
		dilatedImage1 = X.dilate(B1);
		dilatedImage2 = X.dilate(B2);
		dilatedImage3 = X.dilate(B3);
		dilatedImage1cut = X.dilatecut(B1);
		dilatedImage2cut = X.dilatecut(B2);
		dilatedImage3cut = X.dilatecut(B3);

		CPPUNIT_ASSERT_MESSAGE(errormessage.str(), X.empty());
		CPPUNIT_ASSERT_MESSAGE(errormessage.str(), erodedImage1.empty());
		CPPUNIT_ASSERT_MESSAGE(errormessage.str(), erodedImage2.empty());
		CPPUNIT_ASSERT_MESSAGE(errormessage.str(), erodedImage2cut.empty());
		CPPUNIT_ASSERT_MESSAGE(errormessage.str(), erodedImage3.empty());
		CPPUNIT_ASSERT_MESSAGE(errormessage.str(), dilatedImage1.empty());
		CPPUNIT_ASSERT_MESSAGE(errormessage.str(), dilatedImage2.empty());
		CPPUNIT_ASSERT_MESSAGE(errormessage.str(), dilatedImage3.empty());
		CPPUNIT_ASSERT_MESSAGE(errormessage.str(), dilatedImage1cut.empty());
		CPPUNIT_ASSERT_MESSAGE(errormessage.str(), dilatedImage2cut.empty());
		CPPUNIT_ASSERT_MESSAGE(errormessage.str(), dilatedImage3cut.empty());
	}
}


void
RegionMorphTest::testEqualityOperator()
{
	int rdmInteger1, rdmInteger2, rdmInteger3, rdmInteger4;
	std::stringstream errormessage;
	Region X1, X2, X3;
	srand(time(NULL));

	for (int i = 1; i < testIterations; i++) {
		rdmInteger1 = (rand()%500);
		rdmInteger2 = (rand()%500);
		rdmInteger3 = (rand()%100);
		rdmInteger4 = (rand()%100);

		errormessage << "Test failed for rdmInteger1 = " << rdmInteger1 << " - rdmInteger2 = " << rdmInteger2 << " -  rdmInteger3 = " << rdmInteger3 << " - rdmInteger4 = " << rdmInteger4 << endl;

		X1 = Region(Circle(PointF64(rdmInteger3, rdmInteger4), rdmInteger1));
		X2 = Region(Rect(PointF64(-rdmInteger3, -rdmInteger4), PointF64(rdmInteger2, rdmInteger3), Angle(rdmInteger4, Angle::InDeg)));
		X3 = X1.unions(X2);

		CPPUNIT_ASSERT_MESSAGE(errormessage.str(), X1 == X1);
		CPPUNIT_ASSERT_MESSAGE(errormessage.str(), X2 == X2);
		CPPUNIT_ASSERT_MESSAGE(errormessage.str(), X3 == X3);
	}
}

void
RegionMorphTest::testSinglePointSE()
{
	int rdmInteger1, rdmInteger2, rdmInteger3, rdmInteger4;
	std::stringstream errormessage;
	Region X1, X2, X3;
	Region erodedImage1, erodedImage2, erodedImage2cut, erodedImage3, dilatedImage, dilatedImagecut;
	srand(time(NULL));

	for (int i = 1; i < testIterations; i++) {
		rdmInteger1 = (rand()%500);
		rdmInteger2 = (rand()%500);
		rdmInteger3 = (rand()%100);
		rdmInteger4 = (rand()%100);

		errormessage << "Test failed for rdmInteger1 = " << rdmInteger1 << " :: rdmInteger2 = " << rdmInteger2 << " :: rdmInteger3 = " << rdmInteger3 << " :: rdmInteger4 = " << rdmInteger4 << endl;

		X1 = Region(Circle(PointF64(rdmInteger3, rdmInteger4), rdmInteger1));
		X2 = Region(Rect(PointF64(-rdmInteger3, -rdmInteger4), PointF64(rdmInteger2, rdmInteger3), Angle(rdmInteger4, Angle::InDeg)));
		X3 = X1.unions(X2);

		erodedImage1 = X1.erode1(Region(WinP(0,0,0,0)));
		erodedImage2 = X1.erode2(Region(WinP(0,0,0,0)));
		erodedImage2cut = X1.erode2cut(Region(WinP(0,0,0,0)));
		erodedImage3 = X1.erode3(Region(WinP(0,0,0,0)));
		dilatedImage = X1.dilate(Region(WinP(0,0,0,0)));
		dilatedImagecut = X1.dilatecut(Region(WinP(0,0,0,0)));
		CPPUNIT_ASSERT_MESSAGE(errormessage.str(), X1 == erodedImage1);
		CPPUNIT_ASSERT_MESSAGE(errormessage.str(), X1 == erodedImage2);
		CPPUNIT_ASSERT_MESSAGE(errormessage.str(), X1 == erodedImage2cut);
		CPPUNIT_ASSERT_MESSAGE(errormessage.str(), X1 == erodedImage3);
		CPPUNIT_ASSERT_MESSAGE(errormessage.str(), X1 == dilatedImage);
		CPPUNIT_ASSERT_MESSAGE(errormessage.str(), X1 == dilatedImagecut);

		erodedImage1 = X2.erode1(Region(WinP(0,0,0,0)));
		erodedImage2 = X2.erode2(Region(WinP(0,0,0,0)));
		erodedImage2cut = X2.erode2cut(Region(WinP(0,0,0,0)));
		erodedImage3 = X2.erode3(Region(WinP(0,0,0,0)));
		dilatedImage = X2.dilate(Region(WinP(0,0,0,0)));
		dilatedImagecut = X2.dilatecut(Region(WinP(0,0,0,0)));
		CPPUNIT_ASSERT_MESSAGE(errormessage.str(), X2 == erodedImage1);
		CPPUNIT_ASSERT_MESSAGE(errormessage.str(), X2 == erodedImage2);
		CPPUNIT_ASSERT_MESSAGE(errormessage.str(), X2 == erodedImage2cut);
		CPPUNIT_ASSERT_MESSAGE(errormessage.str(), X2 == erodedImage3);
		CPPUNIT_ASSERT_MESSAGE(errormessage.str(), X2 == dilatedImage);
		CPPUNIT_ASSERT_MESSAGE(errormessage.str(), X2 == dilatedImagecut);

		erodedImage1 = X3.erode1(Region(WinP(0,0,0,0)));
		erodedImage2 = X3.erode2(Region(WinP(0,0,0,0)));
		erodedImage2cut = X3.erode2cut(Region(WinP(0,0,0,0)));
		erodedImage3 = X3.erode3(Region(WinP(0,0,0,0)));
		dilatedImage = X3.dilate(Region(WinP(0,0,0,0)));
		dilatedImagecut = X3.dilatecut(Region(WinP(0,0,0,0)));
		CPPUNIT_ASSERT_MESSAGE(errormessage.str(), X3 == erodedImage1);
		CPPUNIT_ASSERT_MESSAGE(errormessage.str(), X3 == erodedImage2);
		CPPUNIT_ASSERT_MESSAGE(errormessage.str(), X3 == erodedImage2cut);
		CPPUNIT_ASSERT_MESSAGE(errormessage.str(), X3 == erodedImage3);
		CPPUNIT_ASSERT_MESSAGE(errormessage.str(), X3 == dilatedImage);
		CPPUNIT_ASSERT_MESSAGE(errormessage.str(), X3 == dilatedImagecut);
	}
}

void
RegionMorphTest::testImgErodedByItself()
{
	int rdmInteger1, rdmInteger2, rdmInteger3, rdmInteger4;
	std::stringstream errormessage;
	Region X1, X2, X3;
	Region erodedImage1, erodedImage2, erodedImage2cut, erodedImage3;
	srand(time(NULL));

	for (int i = 1; i < testIterations; i++) {
		rdmInteger1 = (rand()%500);
		rdmInteger2 = (rand()%500);
		rdmInteger3 = (rand()%100);
		rdmInteger4 = (rand()%100);

		errormessage << "Test failed for rdmInteger1 = " << rdmInteger1 << " :: rdmInteger2 = " << rdmInteger2 << " :: rdmInteger3 = " << rdmInteger3 << " :: rdmInteger4 = " << rdmInteger4 << endl;

		X1 = Region(Circle(PointF64(rdmInteger3, rdmInteger4), rdmInteger1));
		X2 = Region(Rect(PointF64(-rdmInteger3, -rdmInteger4), PointF64(rdmInteger2, rdmInteger3), Angle(rdmInteger4, Angle::InDeg)));
		X3 = X1.unions(X2);

		erodedImage1 = X1.erode1(X1);
		erodedImage2 = X1.erode2(X1);
		erodedImage2cut = X1.erode2cut(X1);
		erodedImage3 = X1.erode3(X1);
		CPPUNIT_ASSERT_MESSAGE(errormessage.str(), Region(WinP(0,0,0,0)) == erodedImage1);
		CPPUNIT_ASSERT_MESSAGE(errormessage.str(), Region(WinP(0,0,0,0)) == erodedImage2);
		CPPUNIT_ASSERT_MESSAGE(errormessage.str(), Region(WinP(0,0,0,0)) == erodedImage2cut);
		CPPUNIT_ASSERT_MESSAGE(errormessage.str(), Region(WinP(0,0,0,0)) == erodedImage3);

		erodedImage1 = X2.erode1(X2);
		erodedImage2 = X2.erode2(X2);
		erodedImage2cut = X2.erode2cut(X2);
		erodedImage3 = X2.erode3(X2);
		CPPUNIT_ASSERT_MESSAGE(errormessage.str(), Region(WinP(0,0,0,0)) == erodedImage1);
		CPPUNIT_ASSERT_MESSAGE(errormessage.str(), Region(WinP(0,0,0,0)) == erodedImage2);
		CPPUNIT_ASSERT_MESSAGE(errormessage.str(), Region(WinP(0,0,0,0)) == erodedImage2cut);
		CPPUNIT_ASSERT_MESSAGE(errormessage.str(), Region(WinP(0,0,0,0)) == erodedImage3);

		erodedImage1 = X3.erode1(X3);
		erodedImage2 = X3.erode2(X3);
		erodedImage2cut = X3.erode2cut(X3);
		erodedImage3 = X3.erode3(X3);
		CPPUNIT_ASSERT_MESSAGE(errormessage.str(), Region(WinP(0,0,0,0)) == erodedImage1);
		CPPUNIT_ASSERT_MESSAGE(errormessage.str(), Region(WinP(0,0,0,0)) == erodedImage2);
		CPPUNIT_ASSERT_MESSAGE(errormessage.str(), Region(WinP(0,0,0,0)) == erodedImage2cut);
		CPPUNIT_ASSERT_MESSAGE(errormessage.str(), Region(WinP(0,0,0,0)) == erodedImage3);
	}

}

void
RegionMorphTest::testPointDilatedByImage()
{
	int rdmInteger1, rdmInteger2, rdmInteger3, rdmInteger4;
	std::stringstream errormessage;
	Region X1, X2, X3;
	Region dilatedImage, dilatedImagecut;
	srand(time(NULL));

	for (int i = 1; i < testIterations; i++) {
		rdmInteger1 = (rand()%500);
		rdmInteger2 = (rand()%500);
		rdmInteger3 = (rand()%100);
		rdmInteger4 = (rand()%100);

		errormessage << "Test failed for rdmInteger1 = " << rdmInteger1 << " :: rdmInteger2 = " << rdmInteger2 << " :: rdmInteger3 = " << rdmInteger3 << " :: rdmInteger4 = " << rdmInteger4 << endl;

		X1 = Region(Circle(PointF64(rdmInteger3, rdmInteger4), rdmInteger1));
		X2 = Region(Rect(PointF64(-rdmInteger3, -rdmInteger4), PointF64(rdmInteger2, rdmInteger3), Angle(rdmInteger4, Angle::InDeg)));
		X3 = X1.unions(X2);

		dilatedImage = Region(WinP(0,0,0,0)).dilate(X1);
		CPPUNIT_ASSERT_MESSAGE(errormessage.str(), X1 == dilatedImage);
		dilatedImagecut = Region(WinP(0,0,0,0)).dilatecut(X1);
		CPPUNIT_ASSERT_MESSAGE(errormessage.str(), X1 == dilatedImagecut);

		dilatedImage = Region(WinP(0,0,0,0)).dilate(X2);
		CPPUNIT_ASSERT_MESSAGE(errormessage.str(), X2 == dilatedImage);
		dilatedImagecut = Region(WinP(0,0,0,0)).dilatecut(X2);
		CPPUNIT_ASSERT_MESSAGE(errormessage.str(), X2 == dilatedImagecut);

		dilatedImage = Region(WinP(0,0,0,0)).dilate(X3);
		CPPUNIT_ASSERT_MESSAGE(errormessage.str(), X3 == dilatedImage);
		dilatedImagecut = Region(WinP(0,0,0,0)).dilatecut(X3);
		CPPUNIT_ASSERT_MESSAGE(errormessage.str(), X3 == dilatedImagecut);
	}

}


int test_region_morph(int, char*[])
{
    std::ofstream of("test_region.xml");
    CppUnit::TextTestRunner runner;
    if (localTesting()) {
        runner.setOutputter(new CppUnit::CompilerOutputter(&runner.result(),
                                                           std::cerr));
    } else {
        runner.setOutputter(new CppUnit::XmlOutputter(&runner.result(), of));
    }
    runner.addTest(RegionMorphTest::suite());
    return runner.run() ? 0 : 1;
}
