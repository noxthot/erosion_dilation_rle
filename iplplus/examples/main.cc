#include "./config.h"
#include "ipl/pict.hh"
#include "ipl/cimgboard.hh"
#include "ipl/timer.hh"
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace std;
using namespace ipl;


/**
 * Converts a given integer to a string.
 *
 * @param number the integer to be converted.
 * @return number as string.
 */
string convertIntToString(int number)
{
   stringstream ss; //create a stringstream
   ss << number; //add number to the stream
   return ss.str(); //return a string with the contents of the stream
}


/**
 * loads an image from file (IMAGE_PATH\filename) and converts it to RLE-representation
 * @param filename
 * @return image in RLE-representation
 */
Region loadImage(string filename) {
	PictImage img(string(IMAGE_PATH)+filename);
	Region reg = img.binarize(0, 162);

	return reg;
}


/**
 * Converts toConvert from Region to Mat. (naive approach)
 */
cv::Mat convertRegionToMat ( Region toConvert ) {

   cv::Mat ret(toConvert.boundingBox().height(), toConvert.boundingBox().width(), cv::DataType<unsigned char>::type);

   for ( int i = 0; i < toConvert.boundingBox().height(); i++ )
	   for ( int j = 0; j < toConvert.boundingBox().width(); j++ )
		   ret.at<unsigned char>(i,j) = 0;

   for ( auto & r : toConvert ) {
	   for ( int i = r.start().x_; i < r.start().x_ + r.len(); ++i ) {
		   ret.at<unsigned char>(r.start().y_ - toConvert.boundingBox().upperLeft().y_, i - toConvert.boundingBox().upperLeft().x_) = 1;
	   }
   }

   return ret;
}


/**
 * Converts toConvert from Mat to Region. (naive approach)
 * Since Mat apparently doesn't take track of the image's origin, a region converted to
 * Mat and back to Region afterwards, will be translated by some vector.
 */
Region convertMatToRegion ( cv::Mat toConvert, Point<N16> upperLeft ) {

   Region ret;
   bool runStarted = false;
   int eroXstart;

   for ( int i = 0; i < toConvert.rows; i++ ) {
	   for ( int j = 0; j < toConvert.cols; j++ ) {
		   if (toConvert.at<unsigned char>(i,j) == 0) {
			   if (runStarted) {
				   ret.add(Rbo(Point<N16>(eroXstart, i), j - eroXstart));
			   }
			   runStarted = false;
		   } else if (j+1 == toConvert.cols) {
			   if (runStarted) {
				   ret.add(Rbo(Point<N16>(eroXstart, i), j - eroXstart + 1));
			   }
			   runStarted = false;
	   	   } else {
			   if (!runStarted) {
				   eroXstart = j;
			   }
			   runStarted = true;
	   	   }
	   }
   }

   ret.translate(upperLeft);

   return ret;
}


/**
 * Checks whether or not Region LHS equals Region RHS.
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


/**
 * Returns the transpose of inputRegion
 */
Region transposeRegion(Region inputRegion) {
	Region ret;

	auto r = inputRegion.end();

	while (r != inputRegion.begin()) {
		--r;
		ret.add(Rbo(PointN16(1 - r->start().x_ - r->len(), - r->start().y_), r->len()));
	}

	return ret;
}


int main()
{
	log4cplus::BasicConfigurator::doConfigure();
	iplLogger.setLogLevel(log4cplus::WARN_LOG_LEVEL);


	// 1: Converting example image
	printf("1: converting example image.\n");
	Region X1 = loadImage("imgprc3.tif");

	PictImage bin1(X1.boundingBox().width(), X1.boundingBox().height());
	bin1.fill(255);
	bin1.setRoi(X1);
	bin1.fill(0);
	CImgBoard board1(bin1);
	board1.save("imgprc3.png");


	// 2: Converting example image
	printf("2: converting example image.\n");
	Region B2 = Region::generateStructuringElement(Region::StructuringElementSquare, 12);

	PictImage bin2(B2.boundingBox().width(), B2.boundingBox().height());
	bin2.fill(255);
	bin2.setRoi(B2.translate(Point<N16>(12,12)));
	bin2.fill(0);
	CImgBoard board2(bin2);
	board2.save("square25.png");


	// 3: Generating example image for opening
	printf("3: generating example image for opening.\n");
	Region X3 = loadImage("imgprc3.tif");
	Region B3 = Region::generateStructuringElement(Region::StructuringElementSquare, 12);
	Region openedImage3 = X3.erode2(B3).dilate(B3);

	PictImage bin3(X3.boundingBox().width(), X3.boundingBox().height());
	bin3.fill(255);
	bin3.setRoi(openedImage3);
	bin3.fill(0);
	CImgBoard board3(bin3);
	board3.save("imgprc3_opened.png");


	// TEST
	Region Xn1 = loadImage("imgprc3.tif");
	Region dil1 = Region(WinP(0,0,0,0)).dilate(Xn1);
	Region dil2 = Region(WinP(0,0,0,0)).dilatecut(Xn1);

	PictImage binn1(Xn1.boundingBox().width(), Xn1.boundingBox().height());
	binn1.fill(255);
	binn1.setRoi(dil1);
	binn1.fill(0);
	CImgBoard boardn1(binn1);
	boardn1.save("test_dil1.png");

	PictImage binn2(Xn1.boundingBox().width(), Xn1.boundingBox().height());
	binn2.fill(255);
	binn2.setRoi(dil2);
	binn2.fill(0);
	CImgBoard boardn2(binn2);
	boardn2.save("test_dil2.png");


	// 4: Generating example image for closing
	printf("4: generating example image for closing.\n");
	Region X4 = loadImage("imgprc3.tif");
	Region B4 = Region::generateStructuringElement(Region::StructuringElementSquare, 12);
	Region closedImage = X4.dilate(B4).erode2(B4);
	PictImage bin4(X4.boundingBox().width(), X4.boundingBox().height());
	bin4.fill(255);
	bin4.setRoi(closedImage);
	bin4.fill(0);
	CImgBoard board4(bin4);
	board4.save("imgprc3_closed.png");


	// 5: runtime-comparison openCV with erosion variant 1 - 3 and dilation based on erosion variant 2
	//    Since openCV is behaving different near the border, every comparison between the output of
	//	  our algorithms and the output of openCV will fail. Please also note, that information about
	//	  the location of the origin is lost, during conversion from Mat to Region, so the output-images
	//	  are translated by some vector.
	printf("5: runtime-comparison openCV with erosion variant 1 - 3 and dilation based on erosion variant 2.\n");
	Timer tm5;  // start time measurement
	Region X5 = loadImage("mista_neg.png");
	Region B5, morphOutput5, morphOutputComp5, Btrans5;
	UN32 duration5;
	cv::Mat BMat5, CVoutput5, BtransMat5;
	cv::Mat XMat5 = convertRegionToMat(X5);

	PictImage bin5(X5.boundingBox().height(), X5.boundingBox().width());
	bin5.fill(255);
	bin5.setRoi(X5);
	bin5.fill(0);

	FILE *eOut5 = fopen("mistanegCVvsEhreCircle.dat","wt");
	fprintf(eOut5, "\nO(n)\tEhren1\tEhren2\tEhren3\topenCV\tdEhren\tdopenCV\n");

	int linha5;

	for ( int i = 3; i <= 153; i = i + 5 ) {

		B5 = Region::generateStructuringElement(Region::StructuringElementCircle, i);
		Btrans5 = transposeRegion(B5);
		BMat5 = convertRegionToMat(B5);
		BtransMat5 = convertRegionToMat(Btrans5);
		linha5 = 0;
		printf("Size. %d...\n", i);
		fprintf(eOut5, "%d", i);

	    // Comp 1: erosion variant 1
		linha5++;
		printf("Comp. %d...\n", linha5);
	    tm5.tic();
	    morphOutputComp5 = X5.erode1(B5);
	    duration5 = tm5.toc();
	    fprintf(eOut5, "\t%u", duration5);

	    // Comp 2: erosion variant 2
		linha5++;
		printf("Comp. %d...\n", linha5);
	    tm5.tic();
	    morphOutput5 = X5.erode2(B5);
	    duration5 = tm5.toc();
	    fprintf(eOut5, "\t%u", duration5);
	    if (!(morphOutput5 == morphOutputComp5)) {
	    	fprintf(eOut5,"*");
	        printf("COMPARATION ERROR: line %d, iteration %d \n", linha5, i);
	        printf("comp: %d runs \n", morphOutputComp5.nrRbos());
	        printf("this: %d runs \n", morphOutput5.nrRbos());
	    }

	    // Comp 3: erosion variant 3
		linha5++;
		printf("Comp. %d...\n", linha5);
	    tm5.tic();
	    morphOutput5 = X5.erode3(B5);
	    duration5 = tm5.toc();
	    fprintf(eOut5, "\t%u", duration5);
	    if (!(morphOutput5 == morphOutputComp5)) {
	    	fprintf(eOut5,"*");
	        printf("COMPARATION ERROR: line %d, iteration %d \n", linha5, i);
	        printf("comp: %d runs \n", morphOutputComp5.nrRbos());
	        printf("this: %d runs \n", morphOutput5.nrRbos());
	    }

		// Comp 4: openCV erosion
		linha5++;
		printf("Comp. %d...\n", linha5);
	    tm5.tic();
	    cv::erode(XMat5, CVoutput5, BMat5, cv::Point(-B5.boundingBox().upperLeft().x_, -B5.boundingBox().upperLeft().y_), 1, cv::BORDER_CONSTANT, 0);
	    duration5 = tm5.toc();
	    fprintf(eOut5, "\t%u", duration5);
	    morphOutput5 = convertMatToRegion(CVoutput5, X5.boundingBox().upperLeft());
	    morphOutputComp5.clip(X5.boundingBox());
	    if (!(morphOutput5 == morphOutputComp5)) {
			fprintf(eOut5,"*");
		    printf("COMPARATION ERROR: line %d, iteration %d \n", linha5, i);
		    printf("comp: %d runs \n", morphOutputComp5.nrRbos());
		    printf("this: %d runs \n", morphOutput5.nrRbos());
	         PictImage bin(X5.boundingBox().width() + 20, X5.boundingBox().height() + 20);
	         bin.fill(255);
	         bin.setRoi(X5.translate(Point<N16>(10,10)));
	         bin.fill(0);
	         CImgBoard board(bin);
	         board.fillColor(Board::Blue);
	         board.opacity(0.5);
	         board.drawRegion(morphOutput5.translate(Point<N16>(10,10)));
	         board.fillColor(Board::Red);
	         board.drawRegion(morphOutputComp5.translate(Point<N16>(10,10)));
	         board.save("Error_mistaneg_" + convertIntToString(linha5) + "_" + convertIntToString(i) + ".png");
	         X5.translate(Point<N16>(-10,-10));
	    }

	    // Comp 5: dilation based on erosion variant 2
		linha5++;
		printf("Comp. %d...\n", linha5);
	    tm5.tic();
	    morphOutputComp5 = X5.dilate(B5);
	    duration5 = tm5.toc();
	    fprintf(eOut5, "\t%u", duration5);

		// Comp 6: openCV dilation
		linha5++;
		printf("Comp. %d...\n", linha5);
	    tm5.tic();
	    cv::dilate(XMat5, CVoutput5, BtransMat5, cv::Point(-Btrans5.boundingBox().upperLeft().x_, -Btrans5.boundingBox().upperLeft().y_), 1, cv::BORDER_CONSTANT, 0);
	    duration5 = tm5.toc();
	    fprintf(eOut5, "\t%u", duration5);
	    morphOutput5 = convertMatToRegion(CVoutput5, X5.boundingBox().upperLeft());
	    morphOutputComp5.clip(X5.boundingBox());
	    if (!(morphOutput5 == morphOutputComp5)) {
			fprintf(eOut5,"*");
		    printf("COMPARATION ERROR: line %d, iteration %d \n", linha5, i);
		    printf("comp: %d runs \n", morphOutputComp5.nrRbos());
		    printf("this: %d runs \n", morphOutput5.nrRbos());
	        PictImage bin(X5.boundingBox().width() + 20, X5.boundingBox().height() + 20);
	        bin.fill(255);
	        bin.setRoi(X5.translate(Point<N16>(10,10)));
	        bin.fill(0);
	        CImgBoard board(bin);
	        board.fillColor(Board::Blue);
	        board.opacity(0.5);
	        board.drawRegion(morphOutput5.translate(Point<N16>(10,10)));
	        board.fillColor(Board::Red);
	        board.drawRegion(morphOutputComp5.translate(Point<N16>(10,10)));
	        board.save("Error_mistaneg_" + convertIntToString(linha5) + "_" + convertIntToString(i) + ".png");
	        X5.translate(Point<N16>(-10,-10));
	    }
		fprintf(eOut5,"\n");
		fflush(eOut5);
	}


//	// 6: runtime-comparison openCV with erosion variant 1 - 3 and dilation based on erosion variant 2
//	//    Since openCV is behaving different near the border, every comparison between the output of
//	//	  our algorithms and the output of openCV will fail. Please also note, that information about
//	//	  the location of the origin is lost, during conversion from Mat to Region, so the output-images
//	//	  are translated by some vector.
//	printf("6: runtime-comparison openCV with erosion variant 1 - 3 and dilation based on erosion variant 2.\n");
//	Timer tm6;  // start time measurement
//	Region X6 = loadImage("mista.png");
//	Region B6, morphOutput6, morphOutputComp6, Btrans6;
//	UN32 duration6;
//	cv::Mat BMat6, CVoutput6, BtransMat6;
//	cv::Mat XMat6 = convertRegionToMat(X6);
//
//	PictImage bin6(X6.boundingBox().height(), X6.boundingBox().width());
//	bin6.fill(255);
//	bin6.setRoi(X6);
//	bin6.fill(0);
//
//	FILE *eOut6 = fopen("mistaCVvsEhreCircle.dat","wt");
//	fprintf(eOut6, "\nO(n)\tEhren1\tEhren2\tEhren3\topenCV\tdEhren\tdopenCV\n");
//
//	int linha6;
//
//	for ( int i = 3; i <= 153; i = i + 5 ) {
//
//		B6 = Region::generateStructuringElement(Region::StructuringElementCircle, i);
//		Btrans6 = transposeRegion(B6);
//		BMat6 = convertRegionToMat(B6);
//		BtransMat6 = convertRegionToMat(Btrans6);
//		linha6 = 0;
//		printf("Size. %d...\n", i);
//		fprintf(eOut6, "%d", i);
//
//	    // Comp 1: erosion variant 1
//		linha6++;
//		printf("Comp. %d...\n", linha6);
//	    tm6.tic();
//	    morphOutputComp6 = X6.erode1(B6);
//	    duration6 = tm6.toc();
//	    fprintf(eOut6, "\t%u", duration6);
//
//	    // Comp 2: erosion variant 2
//		linha6++;
//		printf("Comp. %d...\n", linha6);
//	    tm6.tic();
//	    morphOutput6 = X6.erode2(B6);
//	    duration6 = tm6.toc();
//	    fprintf(eOut6, "\t%u", duration6);
//	    if (!(morphOutput6 == morphOutputComp6)) {
//	    	fprintf(eOut6,"*");
//	        printf("COMPARATION ERROR: line %d, iteration %d \n", linha6, i);
//	        printf("comp: %d runs \n", morphOutputComp6.nrRbos());
//	        printf("this: %d runs \n", morphOutput6.nrRbos());
//	    }
//
//	    // Comp 3: erosion variant 3
//		linha6++;
//		printf("Comp. %d...\n", linha6);
//	    tm6.tic();
//	    morphOutput6 = X6.erode3(B6);
//	    duration6 = tm6.toc();
//	    fprintf(eOut6, "\t%u", duration6);
//	    if (!(morphOutput6 == morphOutputComp6)) {
//	    	fprintf(eOut6,"*");
//	        printf("COMPARATION ERROR: line %d, iteration %d \n", linha6, i);
//	        printf("comp: %d runs \n", morphOutputComp6.nrRbos());
//	        printf("this: %d runs \n", morphOutput6.nrRbos());
//	    }
//
//		// Comp 4: openCV erosion
//		linha6++;
//		printf("Comp. %d...\n", linha6);
//	    tm6.tic();
//	    cv::erode(XMat6, CVoutput6, BMat6, cv::Point(-B6.boundingBox().upperLeft().x_, -B6.boundingBox().upperLeft().y_), 1, cv::BORDER_CONSTANT, 0);
//	    duration6 = tm6.toc();
//	    fprintf(eOut6, "\t%u", duration6);
//	    morphOutput6 = convertMatToRegion(CVoutput6, X6.boundingBox().upperLeft());
//	    morphOutputComp6.clip(X6.boundingBox());
//	    if (!(morphOutput6 == morphOutputComp6)) {
//			fprintf(eOut6,"*");
//		    printf("COMPARATION ERROR: line %d, iteration %d \n", linha6, i);
//		    printf("comp: %d runs \n", morphOutputComp6.nrRbos());
//		    printf("this: %d runs \n", morphOutput6.nrRbos());
//	        PictImage bin(X6.boundingBox().width() + 20, X6.boundingBox().height() + 20);
//	        bin.fill(255);
//	        bin.setRoi(X6.translate(Point<N16>(10,10)));
//	        bin.fill(0);
//	        CImgBoard board(bin);
//	        board.fillColor(Board::Blue);
//	        board.opacity(0.5);
//	        board.drawRegion(morphOutput6.translate(Point<N16>(10,10)));
//	        board.fillColor(Board::Red);
//	        board.drawRegion(morphOutputComp6.translate(Point<N16>(10,10)));
//	        board.save("Error_mista_" + convertIntToString(linha6) + "_" + convertIntToString(i) + ".png");
//	        X6.translate(Point<N16>(-10,-10));
//	    }
//
//	    // Comp 5: dilation based on erosion variant 2
//		linha6++;
//		printf("Comp. %d...\n", linha6);
//	    tm6.tic();
//	    morphOutputComp6 = X6.dilate(B6);
//	    duration6 = tm6.toc();
//	    fprintf(eOut6, "\t%u", duration6);
//
//		// Comp 6: openCV dilation
//		linha6++;
//		printf("Comp. %d...\n", linha6);
//	    tm6.tic();
//	    cv::dilate(XMat6, CVoutput6, BtransMat6, cv::Point(-Btrans6.boundingBox().upperLeft().x_, -Btrans6.boundingBox().upperLeft().y_), 1, cv::BORDER_CONSTANT, 0);
//	    duration6 = tm6.toc();
//	    fprintf(eOut6, "\t%u", duration6);
//	    morphOutput6 = convertMatToRegion(CVoutput6, X6.boundingBox().upperLeft());
//	    morphOutputComp6.clip(X6.boundingBox());
//	    if (!(morphOutput6 == morphOutputComp6)) {
//			fprintf(eOut6,"*");
//		    printf("COMPARATION ERROR: line %d, iteration %d \n", linha6, i);
//		    printf("comp: %d runs \n", morphOutputComp6.nrRbos());
//		    printf("this: %d runs \n", morphOutput6.nrRbos());
//	        PictImage bin(X6.boundingBox().width() + 20, X6.boundingBox().height() + 20);
//	        bin.fill(255);
//	        bin.setRoi(X6.translate(Point<N16>(10,10)));
//	        bin.fill(0);
//	        CImgBoard board(bin);
//	        board.fillColor(Board::Blue);
//	        board.opacity(0.5);
//	        board.drawRegion(morphOutput6.translate(Point<N16>(10,10)));
//	        board.fillColor(Board::Red);
//	        board.drawRegion(morphOutputComp6.translate(Point<N16>(10,10)));
//	        board.save("Error_mista_" + convertIntToString(linha6) + "_" + convertIntToString(i) + ".png");
//	        X6.translate(Point<N16>(-10,-10));
//	    }
//		fprintf(eOut6,"\n");
//		fflush(eOut);
//	}


	// 7: Generating example images (barcode)
	printf("7: generating example images (barcode).\n");
	Region X7 = loadImage("2D_2.png");
	Region B7a = Region::generateStructuringElement(Region::StructuringElementLine, 15);
	Region B7b = Region::generateStructuringElement(Region::StructuringElementSquare, 6);
	Region erodedImage7 = X7.erode2(B7a);
	Region openedImage7 = erodedImage7.dilate(B7a);
	Region openedAndDilatedImage7 = openedImage7.dilate(B7b);
	Region openedAndClosedImage7 = openedAndDilatedImage7.erode2(B7b);
	Region openedAndClosedAndOpenedImage7 = openedAndClosedImage7.erode2(B7b).dilate(B7b);

	PictImage bin7a(X7.boundingBox().width(), X7.boundingBox().height());
	bin7a.fill(255);
	bin7a.setRoi(erodedImage7);
	bin7a.fill(0);
	CImgBoard board7a(bin7a);
	board7a.save("2D_2_eroded.png");

	PictImage bin7b(X7.boundingBox().width(), X7.boundingBox().height());
	bin7b.fill(255);
	bin7b.setRoi(openedImage7);
	bin7b.fill(0);
	CImgBoard board7b(bin7b);
	board7b.save("2D_2_opened.png");

	PictImage bin7c(X7.boundingBox().width(), X7.boundingBox().height());
	bin7c.fill(255);
	bin7c.setRoi(openedAndDilatedImage7);
	bin7c.fill(0);
	CImgBoard board7c(bin7c);
	board7c.save("2D_2_openedanddilated.png");

	PictImage bin7d(X7.boundingBox().width(), X7.boundingBox().height());
	bin7d.fill(255);
	bin7d.setRoi(openedAndClosedImage7);
	bin7d.fill(0);
	CImgBoard board7d(bin7d);
	board7d.save("2D_2_openedandclosed.png");

	PictImage bin7e(X7.boundingBox().width(), X7.boundingBox().height());
	bin7e.fill(255);
	bin7e.setRoi(openedAndClosedAndOpenedImage7);
	bin7e.fill(0);
	CImgBoard board7e(bin7e);
	board7e.save("2D_2_openedandclosedandopened.png");

	PictImage bin7f(B7a.boundingBox().width(), B7a.boundingBox().height());
	bin7f.fill(255);
	bin7f.setRoi(B7a.translate(Point<N16>(15,0)));
	bin7f.fill(0);
	CImgBoard board7f(bin7f);
	board7f.save("2D_2_SE1.png");

	PictImage bin7g(B7b.boundingBox().width(), B7b.boundingBox().height());
	bin7g.fill(255);
	bin7g.setRoi(B7b.translate(Point<N16>(6,6)));
	bin7g.fill(0);
	CImgBoard board7g(bin7g);
	board7g.save("2D_2_SE2.png");


	// 8: Generating example images (bumps)
	printf("8: generating example images (bumps).\n");
	Region X8 = loadImage("bumps.png");
	Region B8a = Region::generateStructuringElement(Region::StructuringElementCircle, 2);
	Region B8b = Region::generateStructuringElement(Region::StructuringElementCircle, 13);
	Region closedImage8 = X8.dilate(B8a).erode2(B8a);
	Region closedAndOpenedImage8 = closedImage8.erode2(B8b).dilate(B8b);

	PictImage bin8a(X8.boundingBox().width(), X8.boundingBox().height());
	bin8a.fill(255);
	bin8a.setRoi(closedImage8);
	bin8a.fill(0);
	CImgBoard board8a(bin8a);
	board8a.save("bumps_closed.png");

	PictImage bin8b(X8.boundingBox().width(), X8.boundingBox().height());
	bin8b.fill(255);
	bin8b.setRoi(closedAndOpenedImage8);
	bin8b.fill(0);
	CImgBoard board8b(bin8b);
	board8b.save("bumps_closedandopened.png");

	PictImage bin8c(B8a.boundingBox().width(), B8a.boundingBox().height());
	bin8c.fill(255);
	bin8c.setRoi(B8a.translate(Point<N16>(2,2)));
	bin8c.fill(0);
	CImgBoard board8c(bin8c);
	board8c.save("bumps_SE1.png");

	PictImage bin8d(B8b.boundingBox().width(), B8b.boundingBox().height());
	bin8d.fill(255);
	bin8d.setRoi(B8b.translate(Point<N16>(13,13)));
	bin8d.fill(0);
	CImgBoard board8d(bin8d);
	board8d.save("bumps_SE2.png");


	// 9: Generating example images (bumps2)
	printf("9: generating example images (bumps2).\n");
	Region X9 = loadImage("fesc_img.png");
	Region B9a = Region::generateStructuringElement(Region::StructuringElementCircle, 1);
	Region B9b = Region::generateStructuringElement(Region::StructuringElementCircle, 5);
	Region closedImage9 = X9.dilate(B9a).erode2(B9a);
	Region closedAndOpenedImage9 = closedImage9.erode2(B9b).dilate(B9b);

	PictImage bin9a(X9.boundingBox().width(), X9.boundingBox().height());
	bin9a.fill(255);
	bin9a.setRoi(closedImage9);
	bin9a.fill(0);
	CImgBoard board9a(bin9a);
	board9a.save("fesc_img_closed.png");

	PictImage bin9b(X9.boundingBox().width(), X9.boundingBox().height());
	bin9b.fill(255);
	bin9b.setRoi(closedAndOpenedImage9);
	bin9b.fill(0);
	CImgBoard board9b(bin9b);
	board9b.save("fesc_img_closedandopened.png");

	PictImage bin9c(B9a.boundingBox().width(), B9a.boundingBox().height());
	bin9c.fill(255);
	bin9c.setRoi(B9a.translate(Point<N16>(1,1)));
	bin9c.fill(0);
	CImgBoard board9c(bin9c);
	board9c.save("fesc_img_SE1.png");

	PictImage bin9d(B9b.boundingBox().width(), B9b.boundingBox().height());
	bin9d.fill(255);
	bin9d.setRoi(B9b.translate(Point<N16>(5,5)));
	bin9d.fill(0);
	CImgBoard board9d(bin9d);
	board9d.save("fesc_img_SE2.png");


	// 10: Generating example images (cirloc)
	printf("10: generating example images (cirloc).\n");
	Region X10 = loadImage("CirlocBild3.png");
	Region B10 = Region::generateStructuringElement(Region::StructuringElementSquare, 3);
	Region closedImage10 = X10.dilate(B10).erode2(B10);

	PictImage bin10a(X10.boundingBox().width() + 10, X10.boundingBox().height() + 10);
	bin10a.fill(255);
	bin10a.setRoi(closedImage10);
	bin10a.fill(0);
	CImgBoard board10a(bin10a);
	board10a.save("CirlocBild3_closed.png");

	PictImage bin10b(B10.boundingBox().width(), B10.boundingBox().height());
	bin10b.fill(255);
	bin10b.setRoi(B10.translate(Point<N16>(3,3)));
	bin10b.fill(0);
	CImgBoard board10b(bin10b);
	board10b.save("CirlocBild3_SE.png");


	// 11: Generating example images (glue)
	printf("11: generating example images (glue).\n");
	Region X11 = loadImage("fs2.png");
	Region B11a = Region::generateStructuringElement(Region::StructuringElementSquare, 3);
	Region B11b = Region::generateStructuringElement(Region::StructuringElementCircle, 7);
	Region openedImage11 = X11.erode2(B11a).dilate(B11a);
	Region openedAndClosedImage11 = openedImage11.dilate(B11b).erode2(B11b);

	PictImage bin11a(X11.boundingBox().width(), X11.boundingBox().height());
	bin11a.fill(255);
	bin11a.setRoi(openedImage11);
	bin11a.fill(0);
	CImgBoard board11a(bin11a);
	board11a.save("fs2_opened.png");

	PictImage bin11b(X11.boundingBox().width(), X11.boundingBox().height());
	bin11b.fill(255);
	bin11b.setRoi(openedAndClosedImage11);
	bin11b.fill(0);
	CImgBoard board11b(bin11b);
	board11b.save("fs2_openedandclosed.png");

	PictImage bin11c(B11a.boundingBox().width(), B11a.boundingBox().height());
	bin11c.fill(255);
	bin11c.setRoi(B11a.translate(Point<N16>(3,3)));
	bin11c.fill(0);
	CImgBoard board11c(bin11c);
	board11c.save("fs2_SE1.png");

	PictImage bin11d(B11b.boundingBox().width(), B11b.boundingBox().height());
	bin11d.fill(255);
	bin11d.setRoi(B11b.translate(Point<N16>(7,7)));
	bin11d.fill(0);
	CImgBoard board11d(bin11d);
	board11d.save("fs2_SE2.png");


	return 0;
}

