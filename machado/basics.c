/* BASIS MORPHOLOGICAL */
#define LINUX 1
/* especif in basics.c */
#if LINUX
  #include "pngwriter.h"
#else
/* in Win */

#include<conio.h>
#endif

#include "/usr/include/opencv2/core/core.hpp"
#include <string>

using namespace std;


/////////////////////////////////////////
/* data structs */                     //
typedef struct point {                 //
   int x, y, z;                        //
} point;                               //
                                       //
typedef struct element {               //
   int type, size;                     //
   point p;                            //
   struct element * prox;              //
} element;                             //
                                       //
typedef struct image {                 //
   int ***f;                           //
   int M, N, S;                        //
   int maxDens;                        //
   point O;                            //
} image;                               //
                                       //
/////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
                                                                             //
/* prototipes of functions */                                                //
element *newElement ();                                                      //
element *newElement ( int, int, int, int );                                  //
element *newElement ( point & );                                             //
element *newElement ( point &, int );                                        //
element *newElement ( element * );                                           //
element *newElement ( element *, int );                                      //
element *setElement ( image *, int );                                        //
void     getElement ( image *, element* );                                   //
element* unionElement ( element *, element *, element * );                   //
element* intersectElement ( element *, element *, element * );               //
element* unionRLE ( element *, element *, element * );                       //
element* intersectRLE ( element *, element *, element * );                   //
int      ptComp(element *, element *);                                       //
void     translade (element *, point &, int );                               //
void  flushElement ( element * );                                         //
                                                                             //
image * newImage ();                                                         //
image * newImage ( int, int );                                               //
image * newImage ( int, int, int, int );                                     //
image * newImage ( image * );                                                //
image * newImageFromFile ( char * );                                         //
int     printImageAtFile ( image *, char * );                                //
bool    compare ( image *, image * );                                        //
int     setDensity ( image *, int, bool, bool, bool, int );                  //
void    setDensityNegativeLink(image *, int);                                //
void    unsetDensity ( image * );                                            //
int     setShell ( image *, int, bool, bool, bool, int );                    //
void    unsetShell ( image *, int, bool, bool, bool, int );                  //
void    complement ( image * );                                              //
void    shift ( image *, int );                                              //
void    int2BitVector ( image *, image * );                                  //
void    bitVector2Int ( image *, image * );                                  //
void    transpose ( image * );                                               //
image * getSquare ( int );                                                   //
image * getDiamond ( int );                                                  //
image * getLine ( int );                                                     //
   void flushImage( image * );                                               //
                                                                             //
int    ***  createArray(int, int, int);                                      //
void       deleteArray(int ***, int, int);                                   //
                                                                             //
///////////////////////////////////////////////////////////////////////////////
void bostinha (element *a) {int i = 0;element*p;for ( p = a; p; p= p->prox ){if ( i%6 )printf("\n");printf ( "<%.2d,%.2d, size:%.3d> ",p->p.x,p->p.y,p->size);}printf("\n");scanf("%d",&i);}

enum { EXTREMAL, ORTONORMAL };
enum { DIRECT, INVERSE };
point vector_EXT[26] = { {-1,0,0}, {0,-1,0}, {-1,-1,0}, {1,0,0}, {0,1,0}, {1,1,0}, {-1,1,0}, {1,-1,0},
                        {0,0,-1}, {-1,0,-1}, {0,-1,-1}, {-1,-1,-1}, {1,0,-1}, {0,1,-1}, {1,1,-1}, {-1,1,-1}, {1,-1,-1},
                        {0,0,1}, {-1,0,1}, {0,-1,1}, {-1,-1,1}, {1,0,1}, {0,1,1}, {1,1,1}, {-1,1,1}, {1,-1,1} };
point vector_EXT_a[7] = { {-1,0,0}, {0,-1,0}, {-1,-1,0}, {0,0,-1}, {-1,0,-1}, {0,-1,-1}, {-1,-1,-1} };
point vector_ORT[6] = { {-1,0,0}, {0,-1,0}, {0,1,0}, {1,0,0}, {0,0,-1}, {0,0,1} };
point vector_ORT_a[3] = { {-1,0,0}, {0,-1,0}, {0,0,-1} };
int d1Vector[2] = { 2, 2 },
    d2Vector[2] ={ 8, 4 },
    d3Vector[2] = { 26, 6 },
    d1Vector_a[2] = { 1, 1 },
    d2Vector_a[2] = { 3, 2 },
    d3Vector_a[2] = { 7, 3 };

/*                    *\
   IMAGES PROCEDURES
\*                    */
image * newImage () {
   image * I = new image;
   if ( ! I ) {
      printf("Memo Extended..\n");
      exit(0);
   }
   
   I->O.x = I->O.y = I->O.z = I->M = I->N = I-> S = I->maxDens = 0;
   I->f = NULL;

   return I;
}

image * newImage ( int M, int N, int S = 1 ) {
   image * I = new image;
   if ( ! I ) {
      printf("Memo Extended..\n");
      exit(0);
   }
   I->M = M;
   I->O.x = M/2;
   I->N = N;
   I->O.y = N/2;
   I->S = S;
   I->O.z = S/2;
   I->maxDens = 0;
   I->f = createArray(M,N,S);

   return I;
}

image * newImage ( int M, int N, int S, int val ) {
   image * I = new image;
   if ( ! I ) {
      printf("Memo Extended..\n");
      exit(0);
   }
   I->M = M;
   I->O.x = M/2;
   I->N = N;
   I->O.y = N/2;
   I->S = S;
   I->O.z = S/2;
   I->f = createArray(M,N,S);
   I->maxDens = 0;
   for ( int k = 0; k < I->S; k++ )
      for ( int i = 0; i < I->M; i++ )
         for ( int j = 0; j < I->N; j++ )
            I->f[i][j][k] = val;

   return I;
}

image * newImage ( image * A ) {
   image * I = new image;
   if ( ! I ) {
      printf("Memo Extended..\n");
      exit(0);
   }
   I->M = A->M;
   I->O.x = A->O.x;
   I->N = A->N;
   I->O.y = A->O.y;
   I->S = A->S;
   I->O.z = A->O.z;
   I->f = createArray(I->M,I->N,I->S);
   I->maxDens = A->maxDens;
   for ( int k = 0; k < I->S; k++ )
      for ( int i = 0; i < I->M; i++ )
         for ( int j = 0; j < I->N; j++ )
            I->f[i][j][k] = A->f[i][j][k];

   return I;
}

image * convertRegionToImage ( Region toConvert ) {

   image * I = new image;
   if ( ! I ) {
      printf("Memo Extended..\n");
      exit(0);
   }

   if (toConvert.nrRbos() == 0) {
	   I->M = 1;
	   I->N = 1;
	   I->S = 1;
	   I->O.x = 1;
	   I->O.y = 1;
	   I->O.z = 1;
	   I->f = createArray(I->M,I->N,I->S);
	   I->f[0][0][0] = 1;
	   return I;
   }

   I->M = toConvert.boundingBox().height();  	// number of lines
   I->N = toConvert.boundingBox().width();		// number of columns
   I->S = 1;									// depth

   I->O.x = -toConvert.boundingBox().upperLeft().x_;	// position of the origin within the image-array
   I->O.y = -toConvert.boundingBox().upperLeft().y_;	//
   I->O.z = 1;											//

   I->f = createArray(I->M,I->N,I->S);

   for ( int k = 0; k < I->S; k++ )
      for ( int i = 0; i < I->M; i++ )
         for ( int j = 0; j < I->N; j++ )
            I->f[i][j][k] = 0;


   for (auto & r : toConvert) {
	for (int i = r.start().x_; i < r.start().x_ + r.len(); ++i) {
				I->f[r.start().y_ - toConvert.boundingBox().upperLeft().y_][i - toConvert.boundingBox().upperLeft().x_][0] = 1;
	}
   }

//   printf("Filled the array \n");

   return I;
}

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

Region convertImageToRegion ( image *toConvert ) {

   Region ret;
   bool runStarted = false;
   int eroXstart;

   for ( int i = 0; i < toConvert->M; i++ ) {
	   for ( int j = 0; j < toConvert->N; j++ ) {
		   if (toConvert->f[i][j][0] == 0) {
			   if (runStarted) {
				   ret.add(Rbo(Point<N16>(eroXstart, i), j - eroXstart));
			   }
			   runStarted = false;
		   } else if (j+1 == toConvert->N) {
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

   ret.translate(Point<N16>(-toConvert->O.x, -toConvert->O.y));

   return ret;
}


/**
 * Converts toConvert from Region to Mat.
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
 * Converts toConvert from Mat to Region.
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


void complement ( image *A ) {
   for ( int s = 0; s < A->S; s++ )
      for ( int i = 0; i < A->M; i++ )
         for ( int j = 0; j < A->N; j++ )
            if ( A->f[i][j][s] == 1 )
               A->f[i][j][s] = 0;
            else if ( A->f[i][j][s] == 0 )
               A->f[i][j][s] = 1;
}

void transpose ( image * A ) {

   image *AUX = newImage(A->M, A->N, A->S);
   if ( ! AUX ) {
      printf("Memo Extended..\n");
      exit(0);
   }
   for ( int k = 0, u = A->S - 1; k < A->S; k++, u-- )
      for ( int i = 0, s = A->M - 1; i < A->M; i++, s-- )
         for ( int j = 0, t = A->N - 1; j < A->N; j++, t-- )
            AUX->f[s][t][u] = A->f[i][j][k];

   for ( int k = 0; k < A->S; k++ )
      for ( int i = 0; i < A->M; i++)
         for ( int j = 0; j < A->N; j++)
            A->f[i][j][k] = AUX->f[i][j][k];

   A->O.x = A->M - 1 - A->O.x;
   A->O.y = A->N - 1 - A->O.y;
   A->O.z = A->S - 1 - A->O.z;
   
   flushImage(AUX);
}

void int2BitVector ( image *A, image *tempA ) {
   for ( int z = 0; z < A->S; z++ )
      for ( int x = 0; x < A->M; x++ ) {
         int j = 1, y = 0;
         unsigned int value = 0;
         for ( ; y < A->N; y++ ) {
            value <<= 1;
            value |= (unsigned int) A->f[x][y][z];
            if ( !((y+1) % 32) ) {
               tempA->f[x][j++][z] = value;
               value = 0;
            }
         }
         value <<= 32-(y)%32;
         tempA->f[x][j++][z] = value;
      }
}

void bitVector2Int ( image *temp, image *out ) {
   for ( int z = 0; z < out->S; z++ )
      for ( int x = 0; x < out->M; x++ ) {
         int radix = 32;
         for ( int j = 0, y = 1; j < out->N; j++ ) {
            radix--;
            out->f[x][j][z] = ((unsigned int)temp->f[x][y][z] >> radix) & 1;
            if (radix == 0) {
            	radix = 32;
               y++;
            }
         }
      }
}

void shift ( image *array32, int s ) {

   /* translade (a,b,c) in N*/
   if ( s < 0 ) {
      for ( int z = 0; z < array32->S; z++ )
         for ( int x = 0; x < array32->M; x++ ) {
            unsigned int over = 0x00000000;
            for ( int y = array32->N-1; y >= 0; y-- ) {
               unsigned int tmp = (unsigned int)array32->f[x][y][z];
               array32->f[x][y][z] = ((unsigned int)array32->f[x][y][z] << -s);
               array32->f[x][y][z] |= over;
               over = tmp >> (32 + s);
            }
         }
   } else if ( s > 0 ) {
      int pot2 = 0;
      for ( int i = 0; i < s; i++ )
         pot2 = 2*pot2+1;

      for ( int z = 0; z < array32->S; z++ )
         for ( int x = 0; x < array32->M; x++ ) {
            unsigned int over = 0x00000000;
            for ( int y = 0; y < array32->N; y++ ) {
               unsigned int tmp = (unsigned int)array32->f[x][y][z];
               array32->f[x][y][z] = ((unsigned int)array32->f[x][y][z] >> s);
               array32->f[x][y][z] |= over;
               over = ((tmp&pot2)<<(32-s));
            }
         }
   }
}

image * newImageFromFile ( char * name ) {

   char tipo[4];
   image * I = new image;
   if ( ! I ) {
      printf("Memo Extended..\n");
      exit(0);
   }

   for ( int i = strlen(name) - 3, j = 0; j < 3; i++, j++ )
      tipo[j] = name[i];
   tipo[3] = '\0';

   if ( !strcmp (tipo,"txt") ) {
      FILE * filein = fopen(name,"r");
      if ( !filein )
         return NULL;

      fscanf(filein, "%d", &I->M);
      fscanf(filein, "%d", &I->N);
      fscanf(filein, "%d", &I->S);
      I->O.x = I->M/2;
      I->O.y = I->N/2;
      I->O.z = I->S/2;

      I->f = createArray(I->M,I->N,I->S);

      for ( int s = 0; s < I->S; s++ )
         for ( int i = 0; i < I->M; i++ )
            for ( int j = 0; j < I->N; j++ ) {
               fscanf(filein, "%d", &I->f[i][j][s]);
            }
      I->maxDens = 0;
      fclose(filein);

   } else if ( !strcmp (tipo,"png") ) {

   #if(LINUX)

      pngwriter img_in(1,1,1,name);
      img_in.readfromfile(name);

      I->M = img_in.getwidth() + 1;
      I->N = img_in.getheight() + 1;
      I->S = 1;
      I->O.x = I->M/2;
      I->O.y = I->N/2;
      I->O.z = 0;

      I->f = createArray(I->M,I->N,1);
      for ( int i = 0; i < I->M; i++ )
         for ( int j = 0; j < I->N; j++ )
            I->f[i][j][0] = img_in.read(i,j) < 127 ? 0 : 1;
      I->maxDens = 0;
      img_in.close();

   #else
   /* in Windows */
      printf( "Formato \"%s\" invalido!\n",tipo);
      exit(0);
   #endif
   /* end */
   } else {
      printf( "Formato \"%s\" invalido!\n",tipo);
      exit(0);
   }

   return I;
}

image * getSquare ( int n ) {
	image *B = newImage(n,n,1,1);
}

image * getDiamond ( int n ) {

	image *B = newImage(n,n,1);
   if ( !n%2 ) {
   	printf("\nO valor de n deve ser impar\n");
      return NULL;
   }

   for ( int i = 0, I = B->M-1; i <= n/2; i ++, I-- )
	   for ( int j = 0; j < n; j ++ )
      	if ( j >= B->O.x - i && j <= B->O.x + i ) {
         	B->f[i][j][0] = 1;
         	B->f[I][j][0] = 1;
         } else {
         	B->f[i][j][0] = 0;
         	B->f[I][j][0] = 0;
         }
   return B;
}


image * getLine ( int n ) {
	image *B = newImage(1,n,1,1);
   return B;
}

void flushImage ( image * A ) {
   if ( A ) {
      deleteArray(A->f, A->M, A->N);
      delete A;
   }
}


int printImageAtFile ( image * I, char * name ) {

   char tipo[4];

   for ( int i = strlen(name) - 3, j = 0; j < 3; i++, j++ )
      tipo[j] = name[i];
   tipo[3] = '\0';

   if ( !strcmp (tipo,"txt") ) {

      FILE * filein = fopen(name,"w");
      if ( !filein || !I )
         return 0;

      fprintf(filein, "%d ", I->M);
      fprintf(filein, "%d ", I->N);
      fprintf(filein, "%d", I->S);
      for ( int s = 0; s < I->S; s++ ) {
         fprintf(filein, "\n");
         for ( int i = 0; i < I->M; i++ ) {
            fprintf(filein, "\n");
            for ( int j = 0; j < I->N; j++ )
               fprintf(filein, "%d ", I->f[i][j][s] );
         }
      }
      fclose(filein);

   } else if ( !strcmp (tipo,"png") ) {
   #if(LINUX)
      /* in Linux */
      pngwriter img_out(I->M-1, I->N-1, 0, name);
      for (int u = 0; u < I->M; u ++)
         for (int v = 0; v < I->N; v ++) {
           double s = I->f[u][v][0] ? 1.0 : 0.0;
           img_out.plot(u,v,s,s,s);
         }
      img_out.close();
   #else
      /* in Windows */
      printf( "Formato \"%s\" invalido!\n",tipo);
      exit(0);
   #endif
      /* end */
   } else {
      printf( "Formato \"%s\" invalido!\n",tipo);
      exit(0);
   }
   return 1;
}

bool compare ( image *A, image *B ) {

   if ( !A || !B || A->M != B->M || A->N != B->N || A->S != B->S )
      return false;

   for ( int s = 0; s < A->S; s++ )
      for ( int i = 0; i < A->M; i++ )
         for ( int j = 0; j < A->N; j++ )
            if ( A->f[i][j][s] != B->f[i][j][s] ) {
//               printf ( "\n#<%d,%d,%d>",i,j,s );
               return false;
            }
   return true;
}

/* d-dimensional : dx � a dim a ser fixada. */
int setDensity ( image * A, int viz, bool d1, bool d2, bool d3, int type = DIRECT ) {

   int max = -1;

   /* eixo M : unidimensional */
   if ( !d1 && d2 && d3 ) {
      /* TYPE 0: direct */
      if ( type == DIRECT ) {
         for ( int s = 0; s < A->S; s++ ) {
            for ( int j = 0; j < A->N; j++ ) {
               int i = 0;
               if ( A->f[i][j][s] <= 0 )
                  A->f[i][j][s] = 0;
               else {
                  if ( max < 1 )
                     max = 1;
                  A->f[i][j][s] = 1;
               }
               for ( i++; i < A->M; i++ ) {
                  if ( A->f[i][j][s] > 0 )
                     A->f[i][j][s] = A->f[i-1][j][s] + 1;
                  else
                     A->f[i][j][s] = 0;
                  max = max < A->f[i][j][s] ? A->f[i][j][s] : max;
               }
            }
         }
      /* TYPE 1: inverse */
      } else if ( type == INVERSE ) {
         for ( int s = A->S - 1; s >= 0 ; s-- ) {
            for ( int j = A->N - 1; j >= 0 ; j-- ) {
               int i = A->M - 1;
               if ( A->f[i][j][s] <= 0 )
                  A->f[i][j][s] = 0;
               else {
                  if ( max < 1 )
                     max = 1;
                  A->f[i][j][s] = 1;
               }
               for ( i--; i >= 0 ; i-- ) {
                  if ( A->f[i][j][s] > 0 )
                     A->f[i][j][s] = A->f[i+1][j][s] + 1;
                  else
                     A->f[i][j][s] = 0;
                  max = max < A->f[i][j][s] ? A->f[i][j][s] : max;
               }
            }
         }
      }
   /* eixo N */
   } else if ( d1 && !d2 && d3 ) {
      /* TYPE 0: direct */
      if ( type == DIRECT ) {
         for ( int s = 0; s < A->S; s++ ) {
            for ( int i = 0; i < A->M; i++ ) {
               int j = 0;
               if ( A->f[i][j][s] <= 0 )
                  A->f[i][j][s] = 0;
               else {
                  if ( max < 1 )
                     max = 1;
                  A->f[i][j][s] = 1;
               }
               for ( j++; j < A->N; j++ ) {
                  if ( A->f[i][j][s] > 0 )
                     A->f[i][j][s] = A->f[i][j-1][s] + 1;
                  else
                     A->f[i][j][s] = 0;
                  max = max < A->f[i][j][s] ? A->f[i][j][s] : max;
               }
            }
         }
      /* TYPE 1: inverse */
      } else if ( type == INVERSE ) {
         for ( int s = A->S - 1; s >= 0 ; s-- ) {
            for ( int i = A->M - 1; i >= 0 ; i-- ) {
               int j = A->N - 1;
               if ( A->f[i][j][s] <= 0 )
                  A->f[i][j][s] = 0;
               else {
                  if ( max < 1 )
                     max = 1;
                  A->f[i][j][s] = 1;
               }
               for ( j--; j >= 0 ; j-- ) {
                  if ( A->f[i][j][s] > 0 )
                     A->f[i][j][s] = A->f[i][j+1][s] + 1;
                  else
                     A->f[i][j][s] = 0;
                  max = max < A->f[i][j][s] ? A->f[i][j][s] : max;
               }
            }
         }
      }
   /* eixo S */
   } else if ( d1 && d2 && !d3 ) {
      /* TYPE 0: direct */
      if ( type == DIRECT ) {
         for ( int j = 0; j < A->N; j++ ) {
            for ( int i = 0; i < A->M; i++ ) {
               int s = 0;
               if ( A->f[i][j][s] <= 0 )
                  A->f[i][j][s] = 0;
               else {
                  if ( max < 1 )
                     max = 1;
                  A->f[i][j][s] = 1;
               }
               for ( s++; s < A->S; s++ ) {
                  if ( A->f[i][j][s] > 0 )
                     A->f[i][j][s] = A->f[i][j][s-1] + 1;
                  else
                     A->f[i][j][s] = 0;
                  max = max < A->f[i][j][s] ? A->f[i][j][s] : max;
               }
            }
         }
      /* TYPE 1: inverse */
      } else if ( type == INVERSE ) {
         for ( int j = A->N - 1; j >= 0 ; j-- ) {
            for ( int i = A->M - 1; i >= 0 ; i-- ) {
               int s = A->S - 1;
               if ( A->f[i][j][s] <= 0 )
                  A->f[i][j][s] = 0;
               else {
                  if ( max < 1 )
                     max = 1;
                  A->f[i][j][s] = 1;
               }
               for ( s--; s >= 0 ; s-- ) {
                  if ( A->f[i][j][s] > 0 )
                     A->f[i][j][s] = A->f[i][j][s+1] + 1;
                  else
                     A->f[i][j][s] = 0;
                  max = max < A->f[i][j][s] ? A->f[i][j][s] : max;
               }
            }
         }
      }
   /* eixo MN : bidimensional */
   } else if ( !d1 && !d2 && d3 ) {
      /* TYPE 0: direct */
      if ( type == DIRECT ) {
         /* inicializando a base da recorr�ncia */
         for ( int s = 0; s < A->S; s++ ) {
            if ( A->f[0][0][s] <= 0 )
               A->f[0][0][s] = 0;
            else
               A->f[0][0][s] = 1;
            for ( int i = 1; i < A->M; i++ )
               if ( A->f[i][0][s] <= 0 )
                  A->f[i][0][s] = 0;
               else
                  A->f[i][0][s] = 1;
            for ( int j = 1; j < A->N; j++ )
               if ( A->f[0][j][s] <= 0 )
                  A->f[0][j][s] = 0;
               else
                  A->f[0][j][s] = 1;
         }

         for ( int s = 0; s < A->S; s++ ) {
            for ( int i = 1; i < A->M; i++ ) {
               for ( int j = 1; j < A->N; j++ ) {
                  if ( A->f[i][j][s] > 0 ) {
                     point * vectors = viz == EXTREMAL ? vector_EXT_a : vector_ORT_a;
                     int min = A->f[i+vectors[0].x][j+vectors[0].y][s];
                     for ( int v = 1; v < d2Vector_a[viz]; v ++ )
                        if ( min > A->f[i+vectors[v].x][j+vectors[v].y][s] )
                           min = A->f[i+vectors[v].x][j+vectors[v].y][s];
                     A->f[i][j][s] = min + 1;
                  } else
                     A->f[i][j][s] = 0;
                  max = max < A->f[i][j][s] ? A->f[i][j][s] : max;
               }
            }
         }
      /* TYPE 1: inverse */
      } else if ( type == INVERSE ) {
               /* inicializando a base da recorr�ncia */
         for ( int s = 0; s < A->S; s++ ) {
            if ( A->f[A->M-1][A->N-1][s] <= 0 )
               A->f[A->M-1][A->N-1][s] = 0;
            else
               A->f[A->M-1][A->N-1][s] = 1;
            for ( int i = 1; i < A->M; i++ )
               if ( A->f[i][A->N-1][s] <= 0 )
                  A->f[i][A->N-1][s] = 0;
               else
                  A->f[i][A->N-1][s] = 1;
            for ( int j = 1; j < A->N; j++ )
               if ( A->f[A->M-1][j][s] <= 0 )
                  A->f[A->M-1][j][s] = 0;
               else
                  A->f[A->M-1][j][s] = 1;
         }

         for ( int s = A->S-1; s >= 0; s-- ) {
            for ( int i = A->M-2; i >= 0 ; i-- ) {
               for ( int j = A->N-2; j >= 0 ; j-- ) {
                  if ( A->f[i][j][s] > 0 ) {
                     point * vectors = viz == EXTREMAL ? vector_EXT_a : vector_ORT_a;
                     int min = A->f[i-vectors[0].x][j-vectors[0].y][s];
                     for ( int v = 1; v < d2Vector_a[viz]; v ++ )
                        if ( min > A->f[i-vectors[v].x][j-vectors[v].y][s] )
                           min = A->f[i-vectors[v].x][j-vectors[v].y][s];
                     A->f[i][j][s] = min + 1;
                  } else
                     A->f[i][j][s] = 0;
                  max = max < A->f[i][j][s] ? A->f[i][j][s] : max;
               }
            }
         }
      }
   /* eixo MS */
   } else if ( !d1 && d2 && !d3 ) {
      /* TYPE 0: direct */
      if ( type == DIRECT ) {
         /* inicializando a base da recorr�ncia */
         for ( int j = 0; j < A->N; j++ ) {
            if ( A->f[0][j][0] <= 0 )
               A->f[0][j][0] = 0;
            else
               A->f[0][j][0] = 1;
            for ( int i = 1; i < A->M; i++ )
               if ( A->f[i][j][0] <= 0 )
                  A->f[i][j][0] = 0;
               else
                  A->f[i][j][0] = 1;
            for ( int s = 1; s < A->S; s++ )
               if ( A->f[0][j][s] <= 0 )
                  A->f[0][j][s] = 0;
               else
                  A->f[0][j][s] = 1;
         }

         for ( int j = 0; j < A->N; j++ ) {
            for ( int i = 1; i < A->M; i++ ) {
               for ( int s = 1; s < A->S; s++ ) {
                  if ( A->f[i][j][s] > 0 ) {
                     point * vectors = viz == EXTREMAL ? vector_EXT_a : vector_ORT_a;
                     int min = A->f[i+vectors[0].x][j][s+vectors[0].y];
                     for ( int v = 1; v < d2Vector_a[viz]; v ++ )
                        if ( min > A->f[i+vectors[v].x][j][s+vectors[v].y] )
                           min = A->f[i+vectors[v].x][j][s+vectors[v].y];
                     A->f[i][j][s] = min + 1;
                  } else
                     A->f[i][j][s] = 0;
                  max = max < A->f[i][j][s] ? A->f[i][j][s] : max;
               }
            }
         }
      /* TYPE 1: inverse */
      } else if ( type == INVERSE ) {
               /* inicializando a base da recorr�ncia */
         for ( int j = 0; j < A->N; j++ ) {
            if ( A->f[A->M-1][j][A->S-1] <= 0 )
               A->f[A->M-1][j][A->S-1] = 0;
            else
               A->f[A->M-1][j][A->S-1] = 1;
            for ( int i = 1; i < A->M; i++ )
               if ( A->f[i][j][A->S-1] <= 0 )
                  A->f[i][j][A->S-1] = 0;
               else
                  A->f[i][j][A->S-1] = 1;
            for ( int s = 1; s < A->S; s++ )
               if ( A->f[A->M-1][j][s] <= 0 )
                  A->f[A->M-1][j][s] = 0;
               else
                  A->f[A->M-1][j][s] = 1;
         }

         for ( int j = A->N-1; j >= 0; j-- ) {
            for ( int i = A->M-2; i >= 0 ; i-- ) {
               for ( int s = A->S-2; s >= 0 ; s-- ) {
                  if ( A->f[i][j][s] > 0 ) {
                     point * vectors = viz == EXTREMAL ? vector_EXT_a : vector_ORT_a;
                     int min = A->f[i-vectors[0].x][j][s-vectors[0].y];
                     for ( int v = 1; v < d2Vector_a[viz]; v ++ )
                        if ( min > A->f[i-vectors[v].x][j][s-vectors[v].y] )
                           min = A->f[i-vectors[v].x][j][s-vectors[v].y];
                     A->f[i][j][s] = min + 1;
                  } else
                     A->f[i][j][s] = 0;
                  max = max < A->f[i][j][s] ? A->f[i][j][s] : max;
               }
            }
         }
      }
   /* eixo NS */
   } else if ( d1 && !d2 && !d3 ) {
      /* TYPE 0: direct */
      if ( type == DIRECT ) {
         /* inicializando a base da recorr�ncia */
         for ( int i = 0; i < A->M; i++ ) {
            if ( A->f[i][0][0] <= 0 )
               A->f[i][0][0] = 0;
            else
               A->f[i][0][0] = 1;
            for ( int s = 1; s < A->S; s++ )
               if ( A->f[i][0][s] <= 0 )
                  A->f[i][0][s] = 0;
               else
                  A->f[i][0][s] = 1;
            for ( int j = 1; j < A->N; j++ )
               if ( A->f[i][j][0] <= 0 )
                  A->f[i][j][0] = 0;
               else
                  A->f[i][j][0] = 1;
         }

         for ( int i = 0; i < A->M; i++ ) {
            for ( int s = 1; s < A->S; s++ ) {
               for ( int j = 1; j < A->N; j++ ) {
                  if ( A->f[i][j][s] > 0 ) {
                     point * vectors = viz == EXTREMAL ? vector_EXT_a : vector_ORT_a;
                     int min = A->f[i][j+vectors[0].x][s+vectors[0].y];
                     for ( int v = 1; v < d2Vector_a[viz]; v ++ )
                        if ( min > A->f[i][j+vectors[v].x][s+vectors[v].y] )
                           min = A->f[i][j+vectors[v].x][s+vectors[v].y];
                     A->f[i][j][s] = min + 1;
                  } else
                     A->f[i][j][s] = 0;
                  max = max < A->f[i][j][s] ? A->f[i][j][s] : max;
               }
            }
         }
      /* TYPE 1: inverse */
      } else if ( type == INVERSE ) {
         /* inicializando a base da recorr�ncia */
         for ( int i = 0; i < A->M; i++ ) {
            if ( A->f[i][A->N-1][A->S-1] <= 0 )
               A->f[i][A->N-1][A->S-1] = 0;
            else
               A->f[i][A->N-1][A->S-1] = 1;
            for ( int s = 1; s < A->S; s++ )
               if ( A->f[i][A->N-1][s] <= 0 )
                  A->f[i][A->N-1][s] = 0;
               else
                  A->f[i][A->N-1][s] = 1;
            for ( int j = 1; j < A->N; j++ )
               if ( A->f[i][j][A->S-1] <= 0 )
                  A->f[i][j][A->S-1] = 0;
               else
                  A->f[i][j][A->S-1] = 1;
         }

         for ( int i = A->M-1; i >= 0; i-- ) {
            for ( int s = A->S-2; s >= 0 ; s-- ) {
               for ( int j = A->N-2; j >= 0 ; j-- ) {
                  if ( A->f[i][j][s] > 0 ) {
                     point * vectors = viz == EXTREMAL ? vector_EXT_a : vector_ORT_a;
                     int min = A->f[i][j-vectors[0].x][s-vectors[0].y];
                     for ( int v = 1; v < d2Vector_a[viz]; v ++ )
                        if ( min > A->f[i][j-vectors[v].x][s-vectors[v].y] )
                           min = A->f[i][j-vectors[v].x][s-vectors[v].y];
                     A->f[i][j][s] = min + 1;
                  } else
                     A->f[i][j][s] = 0;
                  max = max < A->f[i][j][s] ? A->f[i][j][s] : max;
               }
            }
         }
      }
   /* eixo MNS : tridimensional */
   } else if ( !d1 && !d2 && !d3 ) {
      /* TYPE 0: direct */
      if ( type == DIRECT ) {
         /* inicializando a base da recorr�ncia */
         for ( int s = 0; s < A->S; s++ )
            for ( int i = 0; i < A->M; i++ )
               if ( A->f[i][0][s] <= 0 )
                  A->f[i][0][s] = 0;
               else
                  A->f[i][0][s] = 1;
         for ( int s = 0; s < A->S; s++ )
            for ( int j = 0; j < A->N; j++ )
               if ( A->f[0][j][s] <= 0 )
                  A->f[0][j][s] = 0;
               else
                  A->f[0][j][s] = 1;
         for ( int i = 0; i < A->M; i++ )
            for ( int j = 0; j < A->N; j++ )
               if ( A->f[i][j][0] <= 0 )
                  A->f[i][j][0] = 0;
               else
                  A->f[i][j][0] = 1;

         for ( int s = 1; s < A->S; s++ ) {
            for ( int i = 1; i < A->M; i++ ) {
               for ( int j = 1; j < A->N; j++ ) {
                  if ( A->f[i][j][s] > 0 ) {
                     point * vectors = viz == EXTREMAL ? vector_EXT_a : vector_ORT_a;
                     int min = A->f[i+vectors[0].x][j+vectors[0].y][s+vectors[0].z];
                     for ( int v = 1; v < d3Vector_a[viz]; v ++ )
                        if ( min > A->f[i+vectors[v].x][j+vectors[v].y][s+vectors[v].z] )
                           min = A->f[i+vectors[v].x][j+vectors[v].y][s+vectors[v].z];
                     A->f[i][j][s] = min + 1;
                  } else
                     A->f[i][j][s] = 0;
                  max = max < A->f[i][j][s] ? A->f[i][j][s] : max;
               }
            }
         }
      /* TYPE 1: inverse */
      } else if ( type == INVERSE ) {
         /* inicializando a base da recorr�ncia */
         for ( int s = 0; s < A->S; s++ )
            for ( int i = 0; i < A->M; i++ )
               if ( A->f[i][A->N-1][s] <= 0 )
                  A->f[i][A->N-1][s] = 0;
               else
                  A->f[i][A->N-1][s] = 1;
         for ( int s = 0; s < A->S; s++ )
            for ( int j = 0; j < A->N; j++ )
               if ( A->f[A->M-1][j][s] <= 0 )
                  A->f[A->M-1][j][s] = 0;
               else
                  A->f[A->M-1][j][s] = 1;
         for ( int i = 0; i < A->M; i++ )
            for ( int j = 0; j < A->N; j++ )
               if ( A->f[i][j][A->S-1] <= 0 )
                  A->f[i][j][A->S-1] = 0;
               else
                  A->f[i][j][A->S-1] = 1;

         for ( int s = A->S-2; s >= 0 ; s-- ) {
            for ( int i = A->M-2; i >= 0; i-- ) {
               for ( int j = A->N-2; j >= 0; j-- ) {
                  if ( A->f[i][j][s] > 0 ) {
                     point * vectors = viz == EXTREMAL ? vector_EXT_a : vector_ORT_a;
                     int min = A->f[i-vectors[0].x][j-vectors[0].y][s-vectors[0].z];
                     for ( int v = 1; v < d3Vector_a[viz]; v ++ )
                        if ( min > A->f[i-vectors[v].x][j-vectors[v].y][s-vectors[v].z] )
                           min = A->f[i-vectors[v].x][j-vectors[v].y][s-vectors[v].z];
                     A->f[i][j][s] = min + 1;
                  } else
                     A->f[i][j][s] = 0;
                  max = max < A->f[i][j][s] ? A->f[i][j][s] : max;
               }
            }
         }
      }
   }

   A->maxDens = max;
   return 1;

}

void setDensityNegativeLink (image *A, int d = 1) {

   if ( d <= 1 ) {
      for ( int s = 0; s < A->S; s++ )
         for ( int j = 0; j < A->N; j++ )
            for ( int i = A->M-2; i >= 0 ; i-- )
               if ( A->f[i][j][s] <= 0 ) {
                  if ( A->f[i+1][j][s] <= 0 )
                     A->f[i][j][s] = A->f[i+1][j][s]-1;
                  else
                     A->f[i][j][s] = 0;
               }
   } else if ( d == 2 ) {
      for ( int s = 0; s < A->S; s++ )
         for ( int i = 0; i < A->M; i++ )
            for ( int j = A->N-2; j >= 0 ; j-- )
               if ( A->f[i][j][s] <= 0 ) {
                  if ( A->f[i][j+1][s] <= 0 )
                     A->f[i][j][s] = A->f[i][j+1][s]-1;
                  else
                     A->f[i][j][s] = 0;
               }
   } else {
      for ( int i = 0; i < A->M; i++ )
         for ( int j = 0; j < A->N; j++ )
            for ( int s = A->S-2; s >= 0 ; s-- )
               if ( A->f[i][j][s] <= 0 ) {
                  if ( A->f[i][j][s+1] <= 0 )
                     A->f[i][j][s] = A->f[i][j][s+1]-1;
                  else
                     A->f[i][j][s] = 0;
               }
   }
}

int setShell ( image * A, int viz, bool d1, bool d2, bool d3, int type = DIRECT ) {

   setDensity(A,viz,d1,d2,d3,type);
   /* eixo M : unidimensional */
   if ( !d1 && d2 && d3 ) {
      /* TYPE 0: direct */
      if ( type == DIRECT ) {
         for ( int s = 0; s < A->S; s++ )
            for ( int j = 0; j < A->N; j++ )
               for ( int i = 1; i < A->M; i++ )
                  if ( A->f[i][j][s] > 0 )
                     A->f[i-1][j][s] = 0;
      /* TYPE 1: inverse */
      } else if ( type == INVERSE ) {
         for ( int s = A->S - 1; s >= 0 ; s-- )
            for ( int j = A->N - 1; j >= 0 ; j-- )
               for ( int i = A->M - 2; i >= 0 ; i-- )
                  if ( A->f[i][j][s] > 0 )
                     A->f[i+1][j][s] = 0;
      }
   /* eixo N */
   } else if ( d1 && !d2 && d3 ) {
      /* TYPE 0: direct */
      if ( type == DIRECT ) {
         for ( int s = 0; s < A->S; s++ )
            for ( int i = 0; i < A->M; i++ )
               for ( int j = 1; j < A->N; j++ )
                  if ( A->f[i][j][s] > 0 )
                     A->f[i][j-1][s] = 0;
      /* TYPE 1: inverse */
      } else if ( type == INVERSE ) {
         for ( int s = A->S - 1; s >= 0 ; s-- )
            for ( int i = A->M - 1; i >= 0 ; i-- )
               for ( int j = A->N - 2; j >= 0 ; j-- )
                  if ( A->f[i][j][s] > 0 )
                     A->f[i][j+1][s] = 0;
      }
   /* eixo S */
   } else if ( d1 && d2 && !d3 ) {
      /* TYPE 0: direct */
      if ( type == DIRECT ) {
         for ( int i = 0; i < A->M; i++ )
            for ( int j = 0; j < A->N; j++ )
               for ( int s = 1; s < A->S; s++ )
                  if ( A->f[i][j][s] > 0 )
                     A->f[i][j][s-1] = 0;
      /* TYPE 1: inverse */
      } else if ( type == INVERSE ) {
         for ( int i = A->M - 1; i >= 0 ; i-- )
            for ( int j = A->N - 1; j >= 0 ; j-- )
               for ( int s = A->S - 2; s >= 0 ; s-- )
                  if ( A->f[i][j][s] > 0 )
                     A->f[i][j][s+1] = 0;
      }
   /* eixo MN : bidimensional */
   } else if ( !d1 && !d2 && d3 ) {
      point * vectors = viz == EXTREMAL ? vector_EXT_a : vector_ORT_a;
      /* TYPE 0: direct */
      if ( type == DIRECT ) {
         for ( int s = 0; s < A->S; s++ )
            for ( int i = 1; i < A->M; i++ )
               for ( int j = 1; j < A->N; j++ )
                  if ( A->f[i][j][s] > 0 )
                     for ( int v = 0; v < d2Vector_a[viz]; v ++ )
                        if ( A->f[i+vectors[v].x][j+vectors[v].y][s] < A->f[i][j][s] )
                           A->f[i+vectors[v].x][j+vectors[v].y][s] = 0;
      /* TYPE 1: inverse */
      } else if ( type == INVERSE ) {
         for ( int s = A->S - 1; s >= 0 ; s-- )
            for ( int i = A->M - 2; i >= 0 ; i-- )
               for ( int j = A->N - 2; j >= 0 ; j-- )
                  if ( A->f[i][j][s] > 0 )
                     for ( int v = 0; v < d2Vector_a[viz]; v ++ )
                        if ( A->f[i-vectors[v].x][j-vectors[v].y][s] < A->f[i][j][s] )
                           A->f[i-vectors[v].x][j-vectors[v].y][s] = 0;
      }
   /* eixo MS */
   } else if ( !d1 && d2 && !d3 ) {
      point * vectors = viz == EXTREMAL ? vector_EXT_a : vector_ORT_a;
      /* TYPE 0: direct */
      if ( type == DIRECT ) {
         for ( int j = 0; j < A->N; j++ )
            for ( int i = 1; i < A->M; i++ )
               for ( int s = 1; s < A->S; s++ )
                  if ( A->f[i][j][s] > 0 )
                     for ( int v = 0; v < d2Vector_a[viz]; v ++ )
                        if ( A->f[i+vectors[v].x][j][s+vectors[v].y] < A->f[i][j][s] )
                           A->f[i+vectors[v].x][j][s+vectors[v].y] = 0;
      /* TYPE 1: inverse */
      } else if ( type == INVERSE ) {
         for ( int j = A->N - 1; j >= 0 ; j-- )
            for ( int i = A->M - 2; i >= 0 ; i-- )
               for ( int s = A->S - 2; s >= 0 ; s-- )
                  if ( A->f[i][j][s] > 0 )
                     for ( int v = 0; v < d2Vector_a[viz]; v ++ )
                        if ( A->f[i-vectors[v].x][j][s-vectors[v].y] < A->f[i][j][s] )
                           A->f[i-vectors[v].x][j][s-vectors[v].y] = 0;
      }
   /* eixo NS */
   } else if ( d1 && !d2 && !d3 ) {
      point * vectors = viz == EXTREMAL ? vector_EXT_a : vector_ORT_a;
      /* TYPE 0: direct */
      if ( type == DIRECT ) {
         for ( int i = 0; i < A->M; i++ )
            for ( int j = 1; j < A->N; j++ )
               for ( int s = 1; s < A->S; s++ )
                  if ( A->f[i][j][s] > 0 )
                     for ( int v = 0; v < d2Vector_a[viz]; v ++ )
                        if ( A->f[i][j+vectors[v].x][s+vectors[v].y] < A->f[i][j][s] )
                           A->f[i][j+vectors[v].x][s+vectors[v].y] = 0;
      /* TYPE 1: inverse */
      } else if ( type == INVERSE ) {
         for ( int i = A->M - 1; i >= 0 ; i-- )
            for ( int j = A->N - 2; j >= 0 ; j-- )
               for ( int s = A->S - 2; s >= 0 ; s-- )
                  if ( A->f[i][j][s] > 0 )
                     for ( int v = 0; v < d2Vector_a[viz]; v ++ )
                        if ( A->f[i][j-vectors[v].x][s-vectors[v].y] < A->f[i][j][s] )
                           A->f[i][j-vectors[v].x][s-vectors[v].y] = 0;
      }
   /* eixo MNS : tridimensional */
   } else if ( !d1 && !d2 && !d3 ) {
      point * vectors = viz == EXTREMAL ? vector_EXT_a : vector_ORT_a;
      /* TYPE 0: direct */
      if ( type == DIRECT ) {
         for ( int i = 1; i < A->M; i++ )
            for ( int j = 1; j < A->N; j++ )
               for ( int s = 1; s < A->S; s++ )
                  if ( A->f[i][j][s] > 0 )
                     for ( int v = 0; v < d3Vector_a[viz]; v ++ )
                        if ( A->f[i+vectors[v].x][j+vectors[v].y][s+vectors[v].z] < A->f[i][j][s] )
                           A->f[i+vectors[v].x][j+vectors[v].y][s+vectors[v].z] = 0;
      /* TYPE 1: inverse */
      } else if ( type == INVERSE ) {
         for ( int i = A->M - 2; i >= 0 ; i-- )
            for ( int j = A->N - 2; j >= 0 ; j-- )
               for ( int s = A->S - 2; s >= 0 ; s-- )
                  if ( A->f[i][j][s] > 0 )
                     for ( int v = 0; v < d3Vector_a[viz]; v ++ )
                        if ( A->f[i-vectors[v].x][j-vectors[v].y][s-vectors[v].z] < A->f[i][j][s] )
                           A->f[i-vectors[v].x][j-vectors[v].y][s-vectors[v].z] = 0;
      }
   }

   return 1;
}

int setMinimalShell ( image * A, int viz, bool d1, bool d2, bool d3, int type = DIRECT ) {

   setShell ( A, viz, d1, d2, d3, type );
   /* only 2d and next *
   if ( viz == EXTREMAL ) {
      if ( type == INVERSE ) {
         for ( int s = 0; s < A->S; s++ ) {
            for ( int i = 0; i < A->M; i++ )
               for ( int j = 0; j < A->N; j++ )
                  if ( A->f[i][j][s] > 1 ) {
                     int m = A->f[i][j][s];
                     if ( ! d1 )
                        for ( int v = 2; i+v < A->N && m == A->f[i+v][j][s]; v ++ )
                           if ( i+v-1%m != i%m )
                              A->f[i+v-1][j][s] = 0;
                     if ( ! d2 )
                        for ( int v = 2; j+v < A->N && m == A->f[i][j+v][s]; v ++ )
                           if ( j+v-1%m != j%m )
                              A->f[i][j+v-1][s] = 0;
                     if ( ! d3 )
                        for ( int v = 2; s+v < A->S && m == A->f[i][j][s+v]; v ++ )
                           if ( s+v-1%m != s%m )
                              A->f[i][j][s+v-1] = 0;
                  }
         }
      } else {
         for ( int s = A->S-1; s >= 0 ; s-- ) {
            for ( int i = A->M-1; i >= 0; i-- )
               for ( int j = A->N-1; j >= 0; j-- )
                  if ( A->f[i][j][s] > 0 ) {
                     int m = A->f[i][j][s];
                     if ( ! d1 )
                        for ( int v = -2; i+v >= 0 && m == A->f[i+v][j][s]; v -- )
                           if ( i+v+1%m != i%m )
                              A->f[i+v+1][j][s] = 0;
                     if ( ! d2 )
                        for ( int v = -2; j+v >= 0 && m == A->f[i][j+v][s]; v -- )
                           if ( j+v+1%m != j%m )
                              A->f[i][j+v+1][s] = 0;
                     if ( ! d3 )
                        for ( int v = -2; s+v >= 0 && m == A->f[i][j][s+v]; v -- )
                           if ( s+v+1%m != s%m )
                              A->f[i][j][s+v+1] = 0;
                  }
         }
      }
   }
   */
}

void unsetDensity ( image * A ) {
   for ( int s = 0; s < A->S; s++ )
      for ( int i = 0; i < A->M; i++ )
         for ( int j = 0; j < A->N; j++ )
            if ( A->f[i][j][s] <= 0 )
               A->f[i][j][s] = 0;
            else
               A->f[i][j][s] = 1;
}

void unsetShell ( image * A, int viz, bool d1, bool d2, bool d3, int type = DIRECT ) {

   /* eixo M : unidimensional */
   if ( !d1 && d2 && d3 ) {
      /* TYPE 1: inverse */
      if ( type == INVERSE ) {
         for ( int s = 0; s <= A->S - 1; s++ )
            for ( int j = 0; j <= A->N - 1; j++ ) {
               for ( int i = 0; i <= A->M - 2; i++ )
                  if ( A->f[i][j][s] > 0 ) {
                     A->f[i+1][j][s] = (A->f[i][j][s] - 1) > A->f[i+1][j][s] ? (A->f[i][j][s] - 1) : A->f[i+1][j][s];
                     A->f[i][j][s] = 1;
                  } else
                     A->f[i][j][s] = 0;
               A->f[A->M-1][j][s] = A->f[A->M-1][j][s] > 0 ? 1 : 0;
            }
      /* TYPE 0: direct */
      } else if ( type == DIRECT ) {
         for ( int s = A->S - 1; s >= 0 ; s-- )
            for ( int j = A->N - 1; j >= 0 ; j-- ) {
               for ( int i = A->M - 1; i > 0 ; i-- )
                  if ( A->f[i][j][s] > 0 ) {
                     A->f[i-1][j][s] = (A->f[i][j][s] - 1) > A->f[i-1][j][s] ? (A->f[i][j][s] - 1) : A->f[i-1][j][s];
                     A->f[i][j][s] = 1;
                  } else
                     A->f[i][j][s] = 0;
               A->f[0][j][s] = A->f[0][j][s] > 0 ? 1 : 0;
            }
      }
   /* eixo N */
   } else if ( d1 && !d2 && d3 ) {
      /* TYPE 1: inverse */
      if ( type == INVERSE ) {
         for ( int s = 0; s <= A->S - 1; s++ )
            for ( int i = 0; i <= A->M - 1; i++ ) {
               for ( int j = 0; j <= A->N - 2; j++ )
                  if ( A->f[i][j][s] > 0 ) {
                     A->f[i][j+1][s] = (A->f[i][j][s] - 1) > A->f[i][j+1][s] ? (A->f[i][j][s] - 1) : A->f[i][j+1][s];
                     A->f[i][j][s] = 1;
                  } else
                     A->f[i][j][s] = 0;
               A->f[i][A->N-1][s] = A->f[i][A->N-1][s] > 0 ? 1 : 0;
            }
      /* TYPE 0: direct */
      } else if ( type == DIRECT ) {
         for ( int s = A->S - 1; s >= 0 ; s-- )
            for ( int i = A->M - 1; i >= 0 ; i-- ) {
               for ( int j = A->N - 1; j > 0 ; j-- )
                  if ( A->f[i][j][s] > 0 ) {
                     A->f[i][j-1][s] = (A->f[i][j][s] - 1) > A->f[i][j-1][s] ? (A->f[i][j][s] - 1) : A->f[i][j-1][s];
                     A->f[i][j][s] = 1;
                  } else
                     A->f[i][j][s] = 0;
               A->f[i][0][s] = A->f[i][0][s] > 0 ? 1 : 0;
            }
      }
   /* eixo S */
   } else if ( d1 && d2 && !d3 ) {
      /* TYPE 1: inverse */
      if ( type == INVERSE ) {
         for ( int j = 0; j <= A->N - 1; j++ )
            for ( int i = 0; i <= A->M - 1; i++ ) {
               for ( int s = 0; s <= A->S - 2; s++ )
                  if ( A->f[i][j][s] > 0 ) {
                     A->f[i][j][s+1] = (A->f[i][j][s] - 1) > A->f[i][j][s+1] ? (A->f[i][j][s] - 1) : A->f[i][j][s+1];
                     A->f[i][j][s] = 1;
                  } else
                     A->f[i][j][s] = 0;
               A->f[i][j][A->S-1] = A->f[i][j][A->S-1] > 0 ? 1 : 0;
            }
      /* TYPE 0: direct */
      } else if ( type == DIRECT ) {
         for ( int j = A->N - 1; j >= 0 ; j-- )
            for ( int i = A->M - 1; i >= 0 ; i-- ) {
               for ( int s = A->S - 1; s > 0 ; s-- )
                  if ( A->f[i][j][s] > 0 ) {
                     A->f[i][j][s-1] = (A->f[i][j][s] - 1) > A->f[i][j][s-1] ? (A->f[i][j][s] - 1) : A->f[i][j][s-1];
                     A->f[i][j][s] = 1;
                  } else
                     A->f[i][j][s] = 0;
               A->f[i][j][0] = A->f[i][j][0] > 0  ? 1 : 0;
            }
      }
   /* eixo MN : bidimensional */
   } else if ( !d1 && !d2 && d3 ) {
      point * vectors = viz == EXTREMAL ? vector_EXT_a : vector_ORT_a;
      /* TYPE 1: inverse */
      if ( type == INVERSE ) {
         for ( int s = 0; s <= A->S - 1; s++ )
            for ( int i = 0; i <= A->M - 1 ; i++ )
               for ( int j = 0; j <= A->N - 1; j++ )
                  if ( i == A->M - 1 || j == A->N - 1 )
                     A->f[i][j][s] = A->f[i][j][s] > 0 ? 1 : 0;
                  else if ( A->f[i][j][s] > 0 ) {
                     for ( int v = 0; v < d2Vector_a[viz]; v ++ )
                        if ( A->f[i-vectors[v].x][j-vectors[v].y][s] < A->f[i][j][s] )
                           A->f[i-vectors[v].x][j-vectors[v].y][s] = A->f[i][j][s] - 1;
                     A->f[i][j][s] = 1;
                  } else
                     A->f[i][j][s] = 0;
      /* TYPE 0: direct */
      } else if ( type == DIRECT ) {
         for ( int s = A->S - 1; s >= 0 ; s-- )
            for ( int i = A->M - 1; i >= 0 ; i-- )
               for ( int j = A->N - 1; j >= 0 ; j-- )
                  if ( i == 0 || j == 0 )
                     A->f[i][j][s] = A->f[i][j][s] > 0 ? 1 : 0;
                  else if ( A->f[i][j][s] > 0 ) {
                     for ( int v = 0; v < d2Vector_a[viz]; v ++ )
                        if ( A->f[i+vectors[v].x][j+vectors[v].y][s] < A->f[i][j][s] )
                           A->f[i+vectors[v].x][j+vectors[v].y][s] = A->f[i][j][s] - 1;
                     A->f[i][j][s] = 1;
                  } else
                     A->f[i][j][s] = 0;
      }
   /* eixo MS */
   } else if ( !d1 && d2 && !d3 ) {
      point * vectors = viz == EXTREMAL ? vector_EXT_a : vector_ORT_a;
      /* TYPE 1: inverse */
      if ( type == INVERSE ) {
         for ( int s = 0; s <= A->S - 1; s++ )
            for ( int i = 0; i <= A->M - 1 ; i++ )
               for ( int j = 0; j <= A->N - 1; j++ )
                  if ( i == A->M - 1 || s == A->S - 1 )
                     A->f[i][j][s] = A->f[i][j][s] > 0 ? 1 : 0;
                  else if ( A->f[i][j][s] > 0 ) {
                     for ( int v = 0; v < d2Vector_a[viz]; v ++ )
                        if ( A->f[i-vectors[v].x][j][s-vectors[v].y] < A->f[i][j][s] )
                           A->f[i-vectors[v].x][j][s-vectors[v].y] = A->f[i][j][s] - 1;
                     A->f[i][j][s] = 1;
                  } else
                     A->f[i][j][s] = 0;
      /* TYPE 0: direct */
      } else if ( type == DIRECT ) {
         for ( int s = A->S - 1; s >= 0 ; s-- )
            for ( int i = A->M - 1; i >= 0 ; i-- )
               for ( int j = A->N - 1; j >= 0 ; j-- )
                  if ( i == 0 || s == 0 )
                     A->f[i][j][s] = A->f[i][j][s] > 0 ? 1 : 0;
                  else if ( A->f[i][j][s] > 0 ) {
                     for ( int v = 0; v < d2Vector_a[viz]; v ++ )
                        if ( A->f[i+vectors[v].x][j][s+vectors[v].y] < A->f[i][j][s] )
                           A->f[i+vectors[v].x][j][s+vectors[v].y] = A->f[i][j][s] - 1;
                     A->f[i][j][s] = 1;
                  } else
                     A->f[i][j][s] = 0;
      }
   /* eixo NS */
   } else if ( d1 && !d2 && !d3 ) {
      point * vectors = viz == EXTREMAL ? vector_EXT_a : vector_ORT_a;
      /* TYPE 1: inverse */
      if ( type == INVERSE ) {
         for ( int s = 0; s <= A->S - 1; s++ )
            for ( int i = 0; i <= A->M - 1 ; i++ )
               for ( int j = 0; j <= A->N - 1; j++ )
                  if ( j == A->N - 1 || s == A->S - 1 )
                     A->f[i][j][s] = A->f[i][j][s] > 0 ? 1 : 0;
                  else if ( A->f[i][j][s] > 0 ) {
                     for ( int v = 0; v < d2Vector_a[viz]; v ++ )
                        if ( A->f[i][j-vectors[v].x][s-vectors[v].y] < A->f[i][j][s] )
                           A->f[i][j-vectors[v].x][s-vectors[v].y] = A->f[i][j][s] - 1;
                     A->f[i][j][s] = 1;
                  } else
                     A->f[i][j][s] = 0;
      /* TYPE 0: direct */
      } else if ( type == DIRECT ) {
         for ( int s = A->S - 1; s >= 0 ; s-- )
            for ( int i = A->M - 1; i >= 0 ; i-- )
               for ( int j = A->N - 1; j >= 0 ; j-- )
                  if ( j == 0 || s == 0 )
                     A->f[i][j][s] = A->f[i][j][s] > 0 ? 1 : 0;
                  else if ( A->f[i][j][s] > 0 ) {
                     for ( int v = 0; v < d2Vector_a[viz]; v ++ )
                        if ( A->f[i][j+vectors[v].x][s+vectors[v].y] < A->f[i][j][s] )
                           A->f[i][j+vectors[v].x][s+vectors[v].y] = A->f[i][j][s] - 1;
                     A->f[i][j][s] = 1;
                  }  else
                     A->f[i][j][s] = 0;
      }
   /* eixo MNS : tridimensional */
   } else if ( !d1 && !d2 && !d3 ) {
      point * vectors = viz == EXTREMAL ? vector_EXT_a : vector_ORT_a;
      /* TYPE 1: inverse */
      if ( type == INVERSE ) {
         for ( int s = 0; s <= A->S - 1; s++ )
            for ( int i = 0; i <= A->M - 1 ; i++ )
               for ( int j = 0; j <= A->N - 1; j++ )
                  if ( i == A->M - 1 || j == A->N - 1 || s == A->S - 1 )
                     A->f[i][j][s] = A->f[i][j][s] > 0 ? 1 : 0;
                  else if ( A->f[i][j][s] > 0 ) {
                     for ( int v = 0; v < d3Vector_a[viz]; v ++ )
                        if ( A->f[i-vectors[v].x][j-vectors[v].y][s-vectors[v].z] < A->f[i][j][s] )
                           A->f[i-vectors[v].x][j-vectors[v].y][s-vectors[v].z] = A->f[i][j][s] - 1;
                     A->f[i][j][s] = 1;
                  }
      /* TYPE 0: direct */
      } else if ( type == DIRECT ) {
         for ( int s = A->S - 1; s >= 0 ; s-- )
            for ( int i = A->M - 1; i >= 0 ; i-- )
               for ( int j = A->N - 1; j >= 0 ; j-- )
                  if ( i == 0 || j == 0 || s == 0 )
                     A->f[i][j][s] = A->f[i][j][s] > 0 ? 1 : 0;
                  else if ( A->f[i][j][s] > 0 ) {
                     for ( int v = 0; v < d3Vector_a[viz]; v ++ )
                        if ( A->f[i+vectors[v].x][j+vectors[v].y][s+vectors[v].z] < A->f[i][j][s] )
                           A->f[i+vectors[v].x][j+vectors[v].y][s+vectors[v].z] = A->f[i][j][s] - 1;
                     A->f[i][j][s] = 1;
                  } else
                     A->f[i][j][s] = 0;
      }
   }
}

/*                    *\
   elements PROCEDURES
\*                    */

element *newElement () {
   element *N = new element;
   if ( ! N ) {
      printf("Memo Extended..\n");
      exit(0);
   }
   N->p.x = 0;
   N->p.y = 0;
   N->p.z = 0;
   N->type = 1;
   N->size = 1;
   N->prox = NULL;

   return N;
}

element *newElement ( point &p ) {
   element *N = new element;
   if ( ! N ) {
      printf("Memo Extended..\n");
      exit(0);
   }
   N->p.x = p.x;
   N->p.y = p.y;
   N->p.z = p.z;
   N->type = 1;
   N->size = 1;
   N->prox = NULL;

   return N;
}

element *newElement ( point &p, int v ) {
   element *N = new element;   
   if ( ! N ) {
      printf("Memo Extended..\n");
      exit(0);
   }
   N->p.x = p.x;
   N->p.y = p.y;
   N->p.z = p.z;
   N->type = 1;
   N->size = v;
   N->prox = NULL;

   return N;
}

element *newElement ( int x, int y, int z, int v = 1 ) {
   element *N = new element;
   if ( ! N ) {
      printf("Memo Extended..\n");
      exit(0);
   }
   N->p.x = x;
   N->p.y = y;
   N->p.z = z;
   N->type = v;
   N->size = 1;
   N->prox = NULL;

   return N;
}

element *newElement ( element *p ) {
   element *N = new element;
   if ( ! N ) {
      printf("Memo Extended..\n");
      exit(0);
   }
   N->p.x = p->p.x;
   N->p.y = p->p.y;
   N->p.z = p->p.z;
   N->type = p->type;
   N->size = p->size;
   N->prox = NULL;

   return N;
}

element *newElement ( element *p, int v ) {
   element *N = new element;
   if ( ! N ) {
      printf("Memo Extended..\n");
      exit(0);
   }
   N->p.x = p->p.x;
   N->p.y = p->p.y;
   N->p.z = p->p.z;
   N->type = 1;
   N->size = v;
   N->prox = NULL;

   return N;
}

element * setElement ( image *A, int type = 0 ) {

   element *last = NULL, *Point = NULL;

   /* type 0: only 1 values */
   if ( type == 0 ) {
      for ( int s = 0; s < A->S; s++ )
         for ( int i = 0; i < A->M; i++ )
            for ( int j = 0; j < A->N; j++ )
               if ( A->f[i][j][s] >= 1 ) {
                  if ( !last ) {
                     Point = newElement(i,j,s);
                     Point->size = A->f[i][j][s];
                     last = Point;
                  } else {
                     last->prox = newElement(i,j,s);
                     last->prox->size = A->f[i][j][s];
                     last = last->prox;
                  }
               }
   /* type 1: both 0 and 1 values */
   } else if ( type == 1 ) {
      for ( int s = 0; s < A->S; s++ )
         for ( int i = 0; i < A->M; i++ )
            for ( int j = 0; j < A->N; j++ )
               if ( A->f[i][j][s] == 1 || A->f[i][j][s] == 0 ) {
                  if ( !last ) {
                     Point = newElement(i,j,s,A->f[i][j][s]);
                     last = Point;
                  } else {
                     last->prox = newElement(i,j,s,A->f[i][j][s]);
                     last = last->prox;
                  }
               }
   }

   return Point;
}

void getElement ( image *A, element *P ) {
   for ( int s = 0; s < A->S; s++ )
      for ( int i = 0; i < A->M; i++ )
         for ( int j = 0; j < A->N; j++ )
            A->f[i][j][s] = 0;

   for ( element *p = P; p; p = p->prox ) {
      for ( int i = 0; i < p->size; i ++ )
         if ( p->p.x>=0 && p->p.y-i >= 0 && p->p.z >= 0 && p->p.x<A->M && p->p.y-i<A->N && p->p.z<A->S )
            A->f[p->p.x][p->p.y-i][p->p.z] = 1;
   }

}

void flushElement ( element *n ) {

   element *aux;
   for ( element *p = n; p; ) {
      aux = p;
      p = p->prox;
      delete aux;
   }
}

int leftRLE ( element *p ) {
   return p->p.y - p->size + 1;
}

void translade (element *a, point &p, int sign = 1) {

   for ( element *q = a; q; q = q->prox ) {
      q->p.x = q->p.x+(sign*p.x);
      q->p.y = q->p.y+(sign*p.y);
      q->p.z = q->p.z+(sign*p.z);
   }
}

element* intersectionElement ( element *a, element *b ) {
   /* o seg. argumento n�o se exclui */
   element *o = NULL, *last = NULL, *lixo;
   element *p = a, *q = b;
   for ( ; p && q; ) {
      if ( ptComp(p, q) > 0 )
      {  q = q->prox;}
      else if ( ptComp(p, q) < 0 )
      {  lixo = p; p = p->prox; delete lixo; }
      else {
         if (!o) { o = p; last = o;}
         else { last->prox = p; last = p; }
         p = p->prox; q = q->prox; 
         last->prox = NULL;
      }
   }
   /* j� o primeiro se decrepta */
   for ( ; p;  ) {
      lixo = p;
      p = p->prox;
      delete lixo;
   }

   return o;
}

element* unionElement ( element *a, element *b ) {
   /* o seg. argumento n�o se exclui */
   element *o = NULL, *last = NULL;
   for ( element *p = a, *q = b; p || q; ) {
      if ( q && p ) {
         if ( ptComp(p, q) > 0 ) {  
            if (!o) { o = newElement(q); last = o;}
            else { last->prox = newElement(q); last = last->prox; }
            q = q->prox;
         } else if ( ptComp(p, q) < 0 ) {
            if (!o) {  o = p; last = o; }
            else { last->prox = p; last = p; }
            p = p->prox;
         } else {
            if (!o) { last = o = p; }
            else { last->prox = p; last = p; }
            p = p->prox; q = q->prox; 
         }
      } else if ( !p ) {
         if (!o) { o = newElement(q); last = o;}
         else { last->prox = newElement(q); last = last->prox; }
         q = q->prox;
      } else {
            if (!o) { last = o = p; }
            else { last->prox = p; last = p; }
            p = p->prox; q = q->prox; 
      }      
   }
   if (last) last->prox = NULL;
   return o;
}


element* unionRLE ( element *a, element *b ) {

   element *out = NULL, *last = NULL, *aux = NULL;

   /* considerando que as listas de rle estejam ordenadas crescentemente */
   if ( a && b)
      aux = ( a->p.z < b->p.z ||  (a->p.z == b->p.z &&  a->p.x < b->p.x) || (a->p.z == b->p.z && (a->p.x == b->p.x && a->p.y < b->p.y)) ) ? a : b;
   else if ( !a )
      return b;
   else if ( !b )
      return a;

   element *p, *q;

   if ( aux == a ) {
      p = a->prox;
      q = b;
   } else {
      p = b->prox;
      q = a;
   }

   aux->prox = NULL;
   for ( ; p || q; )
      if ( !q ) {
         for ( element *r = p; r; ) {
            last = r;
            r = r->prox;
            last->prox = aux;
            aux = last;
         }
         p = NULL;
      } else if ( !p ) {
         for ( element *r = q; r; ) {
            last = r;
            r = r->prox;
            last->prox = aux;
            aux = last;
         }
         q = NULL;
      } else if (p->p.z < q->p.z || 
                (p->p.z == q->p.z &&  p->p.x < q->p.x) || 
                (p->p.z == q->p.z && (p->p.x == q->p.x && p->p.y <= q->p.y)) ) {
         last = p;
         p = p->prox;
         last->prox = aux;
         aux = last;
      } else {
         last = q;
         q = q->prox;
         last->prox = aux;
         aux = last;
      }

   p = last->prox;
   last->prox = NULL;

   for ( ; p; ) {
      /* caso haja interse�ao entre last e p */
      if ( p->p.z == last->p.z && p->p.x == last->p.x && leftRLE(last) <= p->p.y+1 ) {
         /* caso p esta contido em last */
         if ( leftRLE(last) <= leftRLE(p) ) {
                aux = p;
         	p = p->prox;
                delete aux;
         /* senao aumenta tamanho de last: R(last) - E(p) + 1 */
         } else {
            last->size = last->p.y - leftRLE(p) + 1;
            aux = p;
            p = p->prox;
            delete aux;
         }
      } else {
         aux = last;
         last = p;

         p = last->prox;
         last->prox = aux;
      }
   }
   out = last;

   return out;
}

element* intersectRLE ( element *a, element *b, element *c = NULL ) {

   element *out = NULL, *last = NULL;
   if (c)
      flushElement (c);

   /* considerando que as listas de rle estejam ordenadas crescentemente */
   for ( element *p = a, *q = b; p && q; ) {
      /* mesma linha */
      if ( p->p.x == q->p.x && p->p.z == q->p.z && p->type == q->type ) {
         /* caso 0.1: q inter p � vazio */
         if ( q->p.y < leftRLE(p) ) {
            q = q->prox;
         /* caso 0.1: q inter p � vazio */
         } else if ( p->p.y < leftRLE(q) ) {
            p = p->prox;
         } else if ( p->p.y <= q->p.y ) {
            /* caso 1.1: p est� contido em q: L(x)*/
            if ( leftRLE(p) >= leftRLE(q) ) {
               if ( !last ) {
                  out = newElement(p);
                  last = out;
               } else {
                  last->prox = newElement(p);
                  last = last->prox;
               }
            /* caso 1.2: q inter p pela direita */
            } else {
               if ( !last ) {
                  out = newElement(p, p->p.y-leftRLE(q)+1);
                  last = out;
               } else {
                  last->prox = newElement(p, p->p.y-leftRLE(q)+1);
                  last = last->prox;
               }
            }
            p = p->prox;
         } else if ( q->p.y <= p->p.y ) {
            /* caso 2.0: q est� contido em p */
            if ( leftRLE(q) >= leftRLE(p) ) {
               if ( !last ) {
                  out = newElement(q);
                  last = out;
               } else {
                  last->prox = newElement(q);
                  last = last->prox;
               }
            /* caso 2.1: q inter p pela esquerda */
            } else {
               if ( !last ) {
                  out = newElement(q, q->p.y-leftRLE(p)+1);
                  last = out;
               } else {
                  last->prox = newElement(q, q->p.y-leftRLE(p)+1);
                  last = last->prox;
               }
            }
            q = q->prox;
         }
      } else if ( p->type < q->type ) {
         p = p->prox;
      } else if ( q->type < p->type ) {
         q = q->prox;
      } else if ( p->p.z < q->p.z || (p->p.z == q->p.z && p->p.x < q->p.x) )
         p = p->prox;
      else
         q = q->prox;
   }
   return out;
}

/* create a integer array ( matrix ) of l lines and c columns */
int *** createArray(int d1, int d2, int d3) {
   int i, j;
   int ***array;
   array = new int**[d1];
   if(!array)
      return 0;

   for( i = 0; i < d1; i++) {
      array[i] = new int*[d2];
      if(!array[i])
         return 0;
      for( j = 0; j < d2; j++) {
         array[i][j] = new int[d3];
         if(!array[i][j])
            return 0;
      }
   }
   return array;
}

/* delete a integer array ( matrix ) of l lines */
void deleteArray(int ***array, int d1, int d2) {
   if ( array ) {
      int i,j;
      for(i = 0; i < d1; i++) {
         for(j = 0; j < d2; j++)
            if(array[i][j])
               delete [] array[i][j];
         if(array)
            delete [] array[i];
      }
      delete [] array;
   }
}

int ptComp(element *a, element *b) {
   if ( a->p.x == b->p.x && a->p.y == b->p.y && a->p.z == b->p.z )
      return 0;
  if ( a->p.z > b->p.z )
      return 1;
   if ( a->p.z < b->p.z )
      return -1;
   if ( a->p.x > b->p.x )
      return 1;
   if ( a->p.x < b->p.x )
      return -1;
   if ( a->p.y > b->p.y )
      return 1;
   return -1;
}
