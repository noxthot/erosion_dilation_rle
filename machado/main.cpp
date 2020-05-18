#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <math.h>
#include <ipl/pict.hh>
#include <ipl/log.hh>
#include <ipl/timer.hh>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace ipl;


enum { DIAMOND, QUAD, LINE, NONE };
int typeES;

#include "basics.c"
#include "erosions.c"
#include "dilations.c"
#include "teste.c"

void copyMatChar(char *E[100], char V[100][100] ) {
   int i = 0;
   for ( int i = 0; i < 100; i ++ ) {
      E[i] = (char*)V[i];
   }
}

int main(int argc, char** argv) {

   log4cplus::BasicConfigurator::doConfigure();
   iplLogger.setLogLevel(log4cplus::WARN_LOG_LEVEL);

   image *A, *B[500];
   int n;
   char input_file[100] = "datas/finais/";
   char output_file[100] = "results/";


   if ( argc >= 3 ) { // number of arguments must be larger than 3

	  // -> sets output-filenames
      if ( !strcmp("-d", argv[1]) )
         strcat(output_file,"Dilation_");
      else if ( !strcmp("-e", argv[1]) )
         strcat(output_file,"Erosion_");
      else
         goto ERROR_MESSAGE;

      strcat(output_file,argv[2]);
      strcat(output_file,"_");
      // <-

      // -> finalizes the output-filename
      strcat(output_file,argv[3]);
      strcat(output_file,".dat");
      for ( int i = strlen(output_file), j = 3; j >= 0; j -- )
         output_file[i-4-j] = output_file[i-j];
      for ( int i = 0, j = 1; i < strlen(output_file); i++ )
         if ( output_file[i] == '/' ) {
            if ( j == 1 ) // especificando sobre o diretório results "results/...
               j = 0;
            else
               output_file[i] = '-';
         }
      // <-

      strcat(input_file,argv[3]); // input file

      A = newImageFromFile ( input_file ); // loads input file
      if ( !A )
         return -1;

      // gets the desired structuring element in different sizes
      int inc = 10, vinic = 3, vfin = 303; // inc = stepsize, vinic = starting size, vfin = maximum size
      n = 0;
      if ( !strcmp("square", argv[2]) )
    	  for ( int i = vinic, j = 0; i <= vfin; i+=inc, j++, n++ )
    		  B[j] = getSquare(i);
      else if ( !strcmp("diamond", argv[2]) )
          for ( int i = vinic, j = 0; i <= vfin; i+=inc, j++, n++ )
        	  B[j] = getDiamond(i);

     if ( !strcmp("-e", argv[1]) )
        genPaperErosion (A,B,n,output_file);

     if ( !strcmp("-d", argv[1]) )
        genPaperDilation (A,B,n,output_file);
   
     flushImage(A);
     for ( int i = 0; i < n; i++ )
         flushImage(B[i]);
   } else {
      ERROR_MESSAGE:
      #if LINUX
         char opt[10] = " or png-";
      #else
         char opt[10] = "";
      #endif
      printf ( "\nUsage: %s \'id_operator\' \'type_of_SE\' \'input_image.xxx\' \nWhere \'id_operator\' = {-e,-d},\n      \'type_of_SE\' = { square, diamond } and\n      \'input_image.xxx\' is a txt-%simage in \'datas/finals/\' folder.\n" ,argv[0],opt);
   }

   #if !LINUX
      /* in Win*/
      printf ("\npress key...");
      getch();
   #endif

   printf ("DONE \n");
   return 1;
}
