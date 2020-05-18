#include "ipl/cimgboard.hh"
#include <string>
#include <iostream>

using namespace std;

void genPaperErosion (image *, image *[], int, char *);
void genPaperDilation (image *, image *[], int, char *);

/**
 * @param a Input-Image to erode
 * @param b structuring element
 * @param n number of iterations
 * @filename outputfilename
 */
void genPaperErosion (image *a, image *b[], int n, char *filename = "out.dat" ) {

   FILE *eOut;
   eOut = fopen(filename,"wt");
   printf ( "\nOpening %s\n",filename);
   Region SE, X, k1Region, kERegion;
   Timer tm;
   UN32 duration;
   element *RLEa, *RLEb, *erodedImageRLE;
   clock_t start, end;
   image *k1, *kE; // kE = result of naive Erosion; k1 is used to store the result of all other algorithms
   cv::Mat Xmat, SEmat, erodedImageMat;
   int linha;

   if ( !eOut )
      exit(0);


   fprintf(eOut, "\nO(n)\tRLE\tJmDv\tJmDvM1\tJmDM2\tJmDvM2\topenCV\tEhren2cut\tEhren2\tEhren3"); // column-names within the output-file

   for ( int i = 0; i < n; i++ ) {

      printf("\nITERATION %d...",i);
      fprintf( eOut, "\n%d", b[i]->N );

      linha = 1; // display-output-counter
      printf("\n");

//      kE = iDensityErosion ( a, b[i], 0, 1 );
//      kERegion = convertImageToRegion(kE);
//      flushImage(kE);

//      /* COMP 1 : Clássico com matrizes */
//      printf("Comp. %d...\n", linha);
//      tm.tic();  // start time measurement
//      k1 = naiveErosion ( a , b[i] );
//      duration = tm.toc();
//      fprintf(eOut, "\t%u", duration);
//      if ( !compare(k1,kE) )
//    	  fprintf(eOut,"*");

//      /* COMP 2 : Clássico com listas ligadas */
      linha++;
//      printf("Comp. %d...\n", linha);
//      start = clock();
//      k1 = listErosion ( a, b[i] );
//      end = clock();
//      fprintf(eOut, "\t%.2f", ((double)(end - start)) / CLOCKS_PER_SEC);
//      if ( !compare(k1,kE) )
//         fprintf(eOut,"\nCOMPARATION ERROR: line %d, iteration %d, fileout %s", linha, i, filename );
//
//      flushImage(k1);
//
//      /* COMP 3 : Usando BDD  */
      linha++;
//      printf("Comp. %d...\n", linha);
//      start = clock();
//      k1 = bddErosion ( a, b[i] );
//      end = clock();
//      fprintf(eOut, "\t%.2f", ((double)(end - start)) / CLOCKS_PER_SEC);
//      if ( !compare(k1,kE) )
//         fprintf(eOut,"\nCOMPARATION ERROR: line %d, iteration %d, fileout %s", linha, i, filename );
//
//      flushImage(k1);
//
//      /* COMP 4 : Usando Vetor de Bits */
      linha++;
//      start = clock();
//      printf("Comp. %d...\n", linha);
//      k1 = CPUErosion ( a, b[i] );
//      end = clock();
//      fprintf(eOut, "\t%.2f", ((double)(end - start)) / CLOCKS_PER_SEC);
//      if ( !compare(k1,kE) )
//         fprintf(eOut,"\nCOMPARATION ERROR: line %d, iteration %d, fileout %s", linha, i, filename );
//      flushImage(k1);
//
//      /* COMP 5 : Usando Decomposição por E.E. 3x3 */
      linha++;
//      printf("Comp. %d...\n", linha);
//      start = clock();
//      k1= decompErosion ( a, b[i] );
//      end = clock();
//      fprintf(eOut, "\t%.2f", ((double)(end - start)) / CLOCKS_PER_SEC);
//      if ( !compare(k1,kE) )
//         fprintf(eOut,"\nCOMPARATION ERROR: line %d, iteration %d, fileout %s", linha, i, filename );
//
//      flushImage(k1);


      /* COMP 6 : Usando RLE*/
      linha++;
      printf("Comp. %d...\n", linha);

      // -> Conversion from Image to RLE
      setMinimalShell(a,ORTONORMAL,1,0,1,DIRECT);    // Builds the skeleton and distance-table of its points
      setMinimalShell(b[i],ORTONORMAL,1,0,1,DIRECT); //

      RLEa = setElement(a);				 // Extracts the RLE from skeleton + distance-table of its points
      RLEb = setElement(b[i]);			 //

      k1 = newImage ( a->M, a->N, a->S, false );	 // initializes the output-image
      // <-

      tm.tic();
      erodedImageRLE = rleErosion ( RLEa, RLEb, b[i] ); // input: A and B as run-length-encoded images, output: eroded image as rle-image
      duration = tm.toc();

      getElement(k1,erodedImageRLE); 			// converts RLE to image
      unsetShell(a,ORTONORMAL,1,0,1,DIRECT); 	// rebuilds the original image from skeleton and distance-table of its points
      unsetShell(b[i],ORTONORMAL,1,0,1,DIRECT); //

      k1Region = convertImageToRegion(k1);
      kERegion = k1Region;

      fprintf(eOut, "\t%u", duration);
      if ( !(k1Region == kERegion) )
    	  fprintf(eOut,"*");

      flushImage(k1);
      flushElement(RLEa);
      flushElement(RLEb);
      flushElement(erodedImageRLE);


//      /* COMP 7 : Erosão direta */
      linha++;
//      printf("Comp. %d...\n", linha);
//      start = clock();
//      k1= densityErosion ( a, b[i] );
//      end = clock();
//      fprintf(eOut, "\t%.2f", ((double)(end - start)) / CLOCKS_PER_SEC);
//      if ( !compare(k1,kE) )
//         fprintf(eOut,"\nCOMPARATION ERROR: line %d, iteration %d, fileout %s", linha, i, filename );
//
//      flushImage(k1);
//
//      /* COMP 8 : Erosão inversa */
      linha++;
//      printf("Comp. %d...\n", linha);
//      start = clock();
//      k1= iDensityErosion ( a, b[i] );
//      end = clock();
//      fprintf(eOut, "\t%.2f", ((double)(end - start)) / CLOCKS_PER_SEC);
//      if ( !compare(k1,kE) )
//         fprintf(eOut,"\nCOMPARATION ERROR: line %d, iteration %d, fileout %s", linha, i, filename );
//
//      flushImage(k1);
//
//      /* COMP 9 : Erosão direta com ordenação decrescentes das cascas do EE (melhoria I)*/
      linha++;
//      printf("Comp. %d...\n", linha);
//      start = clock();
//      k1= densityErosionImp1 ( a, b[i] );
//      end = clock();
//      fprintf(eOut, "\t%.2f", ((double)(end - start)) / CLOCKS_PER_SEC);
//      if ( !compare(k1,kE) )
//         fprintf(eOut,"\nCOMPARATION ERROR: line %d, iteration %d, fileout %s", linha, i, filename );
//
//      flushImage(k1);
//
//
//      /* COMP 10 : Decomposição + vetor de bits */
      linha++;
//      printf("Comp. %d...\n", linha);
//      start = clock();
//      k1= decompAndCPUErosion ( a, b[i] );
//      end = clock();
//      fprintf(eOut, "\t%.2f", ((double)(end - start)) / CLOCKS_PER_SEC);
//      if ( !compare(k1,kE) )
//         fprintf(eOut,"\nCOMPARATION ERROR: line %d, iteration %d, fileout %s", linha, i, filename );
//
//      flushImage(k1);


      /* COMP 11 : Erosão direta com eixo 1 variável */
      linha++;
      printf("Comp. %d...\n", linha);

      tm.tic();
      k1 = iDensityErosion ( a, b[i], 0, 1 );
      duration = tm.toc();
      fprintf(eOut, "\t%u", duration);

      k1Region = convertImageToRegion(k1);

      if ( !(k1Region == kERegion) )
    	  fprintf(eOut,"*");

      flushImage(k1);


      /* COMP 12 : Erosão direta com eixo 1 variável e melhoria I */
      linha++;
      printf("Comp. %d...\n", linha);

      tm.tic();
      k1 = densityErosionImp1 ( a, b[i], 0, 1 );
      duration = tm.toc();
      fprintf(eOut, "\t%u", duration);

      k1Region = convertImageToRegion(k1);

      if ( !(k1Region == kERegion) )
    	 fprintf(eOut,"*");

      flushImage(k1);


      /* COMP 13 : Erosão direta com Melhoria II: densidades negativas */
      linha++;
      printf("Comp. %d...\n", linha);

      tm.tic();
      k1 = densityErosionImp2 ( a, b[i] );
      duration = tm.toc();
      fprintf(eOut, "\t%u", duration);

      k1Region = convertImageToRegion(k1);

      if ( !(k1Region == kERegion) )
    	  fprintf(eOut,"*");

      flushImage(k1);


      /* COMP 14 : Erosão direta com Melhoria II: densidades negativas e eixo variável 1 */
      linha++;
      printf("Comp. %d...\n", linha);

      tm.tic();
      k1= densityErosionImp2 ( a, b[i], 0, 1 );
      duration = tm.toc();
      fprintf(eOut, "\t%u", duration);

      k1Region = convertImageToRegion(k1);

      if ( !(k1Region == kERegion) )
     	 fprintf(eOut,"*");

      flushImage(k1);


      /* COMP 15 : openCV */
      linha++;
      printf("Comp. %d...\n", linha);

      X = convertImageToRegion(a);											   // Conversion from Image to Mat
      Xmat = convertRegionToMat(X);					   						   //
      SE = convertImageToRegion(b[i]);										   //
      SEmat = convertRegionToMat(SE); 										   //

      tm.tic();
      cv::erode(Xmat, erodedImageMat, SEmat, cv::Point(-SE.boundingBox().upperLeft().x_,-SE.boundingBox().upperLeft().y_), 1, cv::BORDER_CONSTANT, 0);
      duration = tm.toc();
      fprintf(eOut, "\t%u", duration);

      k1Region = convertMatToRegion(erodedImageMat, X.boundingBox().upperLeft());

      if ( !(k1Region == kERegion) )
    	 fprintf(eOut,"*");


      /* COMP 16 : Ehrensperger - Erosion - Variant 2 cut*/
      linha++;
      printf("Comp. %d...\n", linha);

      X = convertImageToRegion(a);	// Conversion from Image to RLE
      SE = convertImageToRegion(b[i]); // Conversion from Image to RLE

      tm.tic();
      k1Region = X.erode2cut(SE);
      duration = tm.toc();
      fprintf(eOut, "\t%u", duration);

      if ( !(k1Region == kERegion) )
    	 fprintf(eOut,"*");


      /* COMP 17 : Ehrensperger - Erosion - Variant 2*/
      linha++;
      printf("Comp. %d...\n", linha);

      X = convertImageToRegion(a);	// Conversion from Image to RLE
      SE = convertImageToRegion(b[i]); // Conversion from Image to RLE

      tm.tic();
      k1Region = X.erode2(SE);
      duration = tm.toc();
      fprintf(eOut, "\t%u", duration);

      if ( !(k1Region == kERegion) )
    	 fprintf(eOut,"*");


      /* COMP 18 : Ehrensperger - Erosion - Variant 3*/
      linha++;
      printf("Comp. %d...\n", linha);

      X = convertImageToRegion(a);	// Conversion from Image to RLE
      SE = convertImageToRegion(b[i]); // Conversion from Image to RLE

      tm.tic();
      k1Region = X.erode3(SE);
      duration = tm.toc();
      fprintf(eOut, "\t%u", duration);

      if ( !(k1Region == kERegion) )
    	 fprintf(eOut,"*");

      fflush(eOut);
   }

   fclose(eOut);
}


void genPaperDilation (image *a, image *b[], int n, char *filename = "out.dat" ) {

   FILE *eOut;
   eOut = fopen(filename,"wt");
   printf ( "\nOpening %s\n",filename);

   if ( !eOut )
      exit(0);

   clock_t start, end;
   Region X, SE, dilatedImage, k1Region, kDRegion;
   image *k1, *kD;
   element *dilatedImageRLE, *RLEa, *RLEb;
   cv::Mat Xmat, SEmat, dilatedImageMat;
   int linha;
   Timer tm;
   UN32 duration;

   fprintf(eOut, "\n\nO(n)\tdRLE\tdopenCV\tdEhren\tdEhrencut");

   for ( int i = 0; i < n; i++ ) {

      printf("\nITERATION %d...",i);
      fprintf( eOut, "\n%d", b[i]->N );
      linha = 1;
      printf("\n");

//     kD = biContourDilation ( a, b[i] ); 	// every other algorithm compares its result with kD.
//     kDRegion = convertImageToRegion(kD);
//     flushImage(kD);

      /* COMP 1 : Clássico com matriz */
//      printf("Comp. %d...\n", linha);
//      tm.tic();  // start time measurement
//      k1 = naiveDilation( a, b[i] );
//      duration = tm.toc();
//      fprintf(eOut, "\t%u", duration);
//      if ( !compare(k1,kD) )
//    	  fprintf(eOut,"*");

//      /* COMP 2 : Clássico com Listas */
      linha++;
//      printf("Comp. %d...\n", linha);
//      start = clock();
//      k1 = listDilation ( a, b[i] );
//      end = clock();
//      fprintf(eOut, "\t%.2f", ((double)(end - start)) / CLOCKS_PER_SEC);
//      if ( !compare(k1,kD) )
//         fprintf(eOut,"\nCOMPARATION ERROR: line %d, iteration %d, fileout %s", linha, i, filename );
//
//      flushImage(k1);
//
//      /* COMP 3 : BDD */
      linha++;
//      start = clock();
//      printf("Comp. %d...\n", linha);
//      k1 = bddDilation ( a, b[i] );
//      end = clock();
//      fprintf(eOut, "\t%.2f", ((double)(end - start)) / CLOCKS_PER_SEC);
//      if ( !compare(k1,kD) )
//         fprintf(eOut,"\nCOMPARATION ERROR: line %d, iteration %d, fileout %s", linha, i, filename );
//
//      flushImage(k1);
//
//      /* COMP 4 : Com vetor de bits */
      linha++;
//      printf("Comp. %d...\n", linha);
//      start = clock();
//      k1 = CPUDilation ( a, b[i] );
//      end = clock();
//      fprintf(eOut, "\t%.2f", ((double)(end - start)) / CLOCKS_PER_SEC);
//      if ( !compare(k1,kD) )
//         fprintf(eOut,"\nCOMPARATION ERROR: line %d, iteration %d, fileout %s", linha, i, filename );
//
//      flushImage(k1);
//
//      /* COMP 5 : Usando decomposição do EE (3x3) */
      linha++;
//      start = clock();
//      printf("Comp. %d...\n", linha);
//      k1= decompDilation ( a, b[i] );
//      end = clock();
//      fprintf(eOut, "\t%.2f", ((double)(end - start)) / CLOCKS_PER_SEC);
//      if ( !compare(k1,kD) )
//         fprintf(eOut,"\nCOMPARATION ERROR: line %d, iteration %d, fileout %s", linha, i, filename );
//
//      flushImage(k1);
//
//      /* COMP 6 : Usando contorno da img entrada */
      linha++;
//      printf("Comp. %d...\n", linha);
//      start = clock();
//      k1 = contourDilatio//      /* COMP 1 : Clássico com matriz */
//      printf("Comp. %d...\n", linha);
//      tm.tic();  // start time measurement
//      kD = naiveDilation( a, b[i] );
//      duration = tm.toc();
//      fprintf(eOut, "\t%u", duration);n ( a, b[i] );
//      end = clock();
//      fprintf(eOut, "\t%.2f", ((double)(end - start)) / CLOCKS_PER_SEC);
//      if ( !compare(k1,kD) )
//         fprintf(eOut,"\nCOMPARATION ERROR: line %d, iteration %d, fileout %s", linha, i, filename );
//
//      flushImage(k1);
//
//      /* COMP 7 : Usando contorno do EE e da img entrada */
      linha++;
//      printf("Comp. %d...\n", linha);
//      start = clock();
//      k1 = biContourDilation ( a, b[i] );
//      end = clock();
//      fprintf(eOut, "\t%.2f", ((double)(end - start)) / CLOCKS_PER_SEC);
//      if ( !compare(k1,kD) )
//         fprintf(eOut,"\nCOMPARATION ERROR: line %d, iteration %d, fileout %s", linha, i, filename );
//
//      flushImage(k1);


      /* COMP 8 : RLE */
      linha++;
      printf("Comp. %d...\n", linha);

      // -> Conversion from Image to RLE
      setMinimalShell(a,ORTONORMAL,1,0,1,DIRECT);    // Builds the skeleton and distance-table of its points
      setMinimalShell(b[i],ORTONORMAL,1,0,1,DIRECT); //

      RLEa = setElement(a);				 // Extracts the RLE from skeleton + distance-table of its points
      RLEb = setElement(b[i]);				 //

      k1 = newImage ( a->M, a->N, a->S, false );	 // initializes the output-image
      // <-

      tm.tic();
      dilatedImageRLE = rleDilation ( RLEa, RLEb, b[i] ); // input: A and B as run-length-encoded images, output: eroded image as rle-image
      duration = tm.toc();

      getElement(k1,dilatedImageRLE); 			// converts RLE to image
      unsetShell(a,ORTONORMAL,1,0,1,DIRECT); 	// rebuilds the original image from skeleton and distance-table of its points
      unsetShell(b[i],ORTONORMAL,1,0,1,DIRECT); //

      fprintf(eOut, "\t%u", duration);

      k1Region = convertImageToRegion(k1);
      kDRegion = k1Region;

      if ( !(k1Region == kDRegion) )
    	  fprintf(eOut,"*");

      flushImage(k1);
      flushElement(RLEa);
      flushElement(RLEb);
      flushElement(dilatedImageRLE);


//      /* COMP 9 : Dilatação por cascas horizontais */
      linha++;
//      printf("Comp. %d...\n", linha);
//      start = clock();
//      k1= densityDilation ( a, b[i] );
//      end = clock();
//      fprintf(eOut, "\t%.2f", ((double)(end - start)) / CLOCKS_PER_SEC);
//      if ( !compare(k1,kD) )
//         fprintf(eOut,"\nCOMPARATION ERROR: line %d, iteration %d, fileout %s", linha, i, filename );
//
//      flushImage(k1);
//
//      /* COMP 10 : Dilatação por cascas 8-conexas */
      linha++;
//      printf("Comp. %d...\n", linha);
//      start = clock();
//      k1= densityDilation ( a, b[i], 0, 0 );
//      end = clock();
//      fprintf(eOut, "\t%.2f", ((double)(end - start)) / CLOCKS_PER_SEC);
//      if ( !compare(k1,kD) )
//         fprintf(eOut,"\nCOMPARATI						ON ERROR: line %d, iteration %d, fileout %s", linha, i, filename );
//
//      flushImage(k1);
//
//      /* COMP 11 : Decomposição + contorno */
      linha++;
//      start = clock();
//      printf("Comp. %d...\n", linha);
//      k1= decompAndContourDilation ( a, b[i] );
//      end = clock();
//      fprintf(eOut, "\t%.2f", ((double)(end - start)) / CLOCKS_PER_SEC);
//      if ( !compare(k1,kD) )
//         fprintf(eOut,"\nCOMPARATION ERROR: line %d, iteration %d, fileout %s", linha, i, filename );
//
//      flushImage(k1);
//
//      /* COMP 12 : Decomposição + vetor de bits */
      linha++;
//      start = clock();
//      printf("Comp. %d...\n", linha);
//      k1= decompAndCPUDilation ( a, b[i] );
//      end = clock();
//      fprintf(eOut, "\t%.2f", ((double)(end - start)) / CLOCKS_PER_SEC);
//      if ( !compare(k1,kD) )
//         fprintf(eOut,"\nCOMPARATION ERROR: line %d, iteration %d, fileout %s", linha, i, filename );
//
//      flushImage(k1);


      /* COMP 13 : openCV */
      linha++;
      printf("Comp. %d...\n", linha);
      X = convertImageToRegion(a);											   // Conversion from Image to Mat
      Xmat = convertRegionToMat(X);					   						   //
      SE = transposeRegion(convertImageToRegion(b[i]));						   //
      SEmat = convertRegionToMat(SE); 										   //

      tm.tic();
      cv::dilate(Xmat, dilatedImageMat, SEmat, cv::Point(-SE.boundingBox().upperLeft().x_, -SE.boundingBox().upperLeft().y_), 1, cv::BORDER_CONSTANT, 0);
      duration = tm.toc();

      fprintf(eOut, "\t%u", duration);

      k1Region = convertMatToRegion(dilatedImageMat, X.boundingBox().upperLeft());
      kDRegion.clip(X.boundingBox());

      if ( !(k1Region == kDRegion) )
    	  fprintf(eOut,"*");


      /* COMP 14 : Ehrensperger - Dilation (based on Erosion Variant 2) */
      linha++;
      printf("Comp. %d...\n", linha);

      X = convertImageToRegion(a);	// Conversion from Image to RLE
      SE = convertImageToRegion(b[i]); // Conversion from Image to RLE

      tm.tic();
      k1Region = X.dilate(SE);
      duration = tm.toc();
      fprintf(eOut, "\t%u", duration);

      k1Region.clip(X.boundingBox());

      if ( !(k1Region == kDRegion) )
    	  fprintf(eOut,"*");

      if ( !(k1Region == kDRegion) )
    	  fprintf(eOut,"*");


      /* COMP 15 : Ehrensperger - Dilation cut (based on Erosion Variant 2) */
      linha++;
      printf("Comp. %d...\n", linha);

      X = convertImageToRegion(a);	// Conversion from Image to RLE
      SE = convertImageToRegion(b[i]); // Conversion from Image to RLE

      tm.tic();
      k1Region = X.dilatecut(SE);
      duration = tm.toc();
      fprintf(eOut, "\t%u", duration);

      k1Region.clip(X.boundingBox());

      fflush(eOut);
   }
   fclose(eOut);
}
