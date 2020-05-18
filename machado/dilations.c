/* EFFICIENT BINARY EROSIONS MORPHOLOGICAL */
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
                                                                               //
/* in Matrices */                                                              //
image * naiveDilation ( image *, image *, image * );                           //
image * listDilation ( image *, image *, image * );                            //
image * CPUDilation ( image *, image *, image * );                             //
image * bddDilation ( image *, image *, image * );                             //
image * contourDilation ( image *, image *, image * );                         //
image * biContourDilation ( image *, image *, image * );                       //
image * densityDilation ( image *, image *, image *, int, int, int, int );     //
image * decompDilation ( image *, image *, image * );                          //
image * decompAndContourDilation ( image *, image *, image * );                //
image * decompAndCPUDilation ( image *, image *, image * );                    //
image * rleDilation ( image *, image *, image * );                             //
    int evalDil ( image *, element *, point& );                                //
    element * getCountour ( image *, int );                                    //
    void clearComponent( image *, point &, int );                              //
                                                                               //
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

/*                                    *\
   DILATIONS PROCEDURES WITH MATRICES
\*                                    */
image * naiveDilation ( image *A, image *B, image *C = NULL ) {

   /* Infimum of P(E) */
   image * out;
   if ( !C )
      out = newImage( A->M, A->N, A->S, false );
   else
      out = C;

   for ( int s = 0; s < B->S; s++ )
      for ( int i = 0; i < B->M; i++ )
         for ( int j = 0; j < B->N; j++ )
            if ( B->f[i][j][s] > 0 ) {
               int a = i - B->O.x, b = j - B->O.y, c = s - B->O.z;
               /* translade (a,b) and aply AND OP */
               for ( int z = 0; z < A->S; z++ )
                  for ( int x = 0; x < A->M; x++ )
                     for ( int y = 0; y < A->N; y++ ) {
                        int M = x + a, N = y + b, S = z + c;
                        if ( M >= 0 && N >= 0 && S >= 0 && M < A->M && N < A->N && S < A->S )
                           out->f[x][y][z] = (A->f[M][N][S] > 0) || out->f[x][y][z];
                     }
            }

   return out;
}

image * listDilation ( image *A, image *B, image *C = NULL ) {

   /* Infimum of P(E) */
   image * out;
   if ( !C )
      out = newImage( A->M, A->N, A->S, false );
   else
      out = C;

   element *o = NULL;
   element *aL = setElement(A);
   element *bL = setElement(B);
   point P;
   bool first = true;
   for ( element *q = bL; q; q = q->prox ) {
      P.x = (q->p.x-B->O.x);
      P.y = (q->p.y-B->O.y);
      P.z = (q->p.z-B->O.z);
      translade (aL, P);
      element *b = unionElement(o,aL);
      o = b;
      translade (aL, P, -1);
   }

   getElement(out,o);
   flushElement(o);
   flushElement(aL);
   flushElement(bL);

   return out;
}

image * CPUDilation ( image *A, image *B, image *C = NULL ) {

   /* Supremum of P(E) */
   image * out;
   if ( !C )
      out = newImage ( A->M, A->N, A->S, false );
   else
      out = C;

   int round = (A->N/32) + ( (A->N%32>0) ? 1 : 0 ) + 2;
   image * temp  = newImage ( A->M, round, A->S, 0x00000000 );
   image * tempA = newImage ( A->M, round, A->S, 0x00000000 );

   /* compress�o */
   int2BitVector ( A, tempA );

   for ( int s = 0; s < B->S; s++ )
      for ( int i = 0; i < B->M; i++ )
         for ( int j = 0; j < B->N; j++ )
            if ( B->f[i][j][s] > 0 ) {
               int a = i-B->O.x, b = j-B->O.y, c = s-B->O.z;
               int bMod = b%32;
               shift ( tempA, bMod );
               /* translade (a,b) and aply AND OP */
               for ( int z = 0; z < temp->S; z++ )
                  for ( int x = 0; x < temp->M; x++ )
                     for ( int y = 0; y < temp->N; y++ ) {
                        int M = x - a, N = y - b/32, S = z - c;
                        if ( M >= 0 && N >= 0 && S >= 0 && M < tempA->M && N < tempA->N && S < tempA->S )
                           temp->f[x][y][z] = (unsigned int)tempA->f[M][N][S] | temp->f[x][y][z];


                     }
               shift ( tempA, -bMod );
            }

   /* descompress�o */
   bitVector2Int ( temp, out );

   flushImage(temp);
   flushImage(tempA);

   return out;
}

image * bddDilation ( image *A, image *B, image *C = NULL ) {

   /* Infimum of P(E) */
   image * out;
   if ( !C )
      out = newImage ( A->M, A->N, A->S, false );
   else
      out = C;

   /* building BDD */
   element *root = NULL, *aux;
   for ( int k = 0; k < B->S; k++ )
      for ( int i = 0; i < B->M; i++ )
         for ( int j = 0; j < B->N; j++ )
            if ( B->f[i][j][k] > 0 ) {
               aux = newElement(B->O.x-i,B->O.y-j,B->O.z-k);
               aux->prox = root;
               root = aux;
            }

   point p;
   for ( int k = 0; k < out->S; k++ )
      for ( int i = 0; i < out->M; i++ )
         for ( int j = 0; j < out->N; j++ ) {
            p.x = i-out->O.x+A->O.x; p.y = j-out->O.y+A->O.y; p.z = k-out->O.z+A->O.z;
            out->f[i][j][k] = evalDil(A,root,p);
         }

   flushElement(root);
   return out;
}

element * getCountour ( image * A, int viz = EXTREMAL ) {

   /* processing contour of A */
   element *C = NULL;
   point * vectors = viz == EXTREMAL ? vector_EXT : vector_ORT;

   /* Borda INTERNA das componentes conexas 2D*/
   if ( A->S == 1 ) {
      for ( int i = A->M-1; i >= 0; i-- )
         for ( int j = A->N-1; j >= 0 ; j-- )
            if ( A->f[i][j][0] > 0 ) {
               for ( int v = 0; v < d2Vector[viz]; v ++ ) {
                  int M = i+vectors[v].x, N = j+vectors[v].y;
                  if ( M == -1 || M == A->M || N == -1 || N == A->N || !A->f[M][N][0] ) {
                     element *p = newElement(i-A->O.x,j-A->O.y,0);
                     p->prox = C;
                     C = p;
                  }
               }
            }
   /* Borda INTERNA das componentes conexas 3D*/
	} else {
      for ( int i = A->M-1; i >= 0; i-- )
         for ( int j = A->N-1; j >= 0 ; j-- )
            for ( int s = A->S-1; s >= 0 ; s-- )
               if ( A->f[i][j][s] > 0 ) {
                  for ( int v = 0; v < d3Vector[viz]; v ++ ) {
                     int M = i+vectors[v].x, N = j+vectors[v].y, S = s+vectors[v].z;
                     if ( M == -1 || M == A->M || N == -1 || N == A->N || S == -1 || S == A->S || !A->f[M][N][S] ) {
                        element *p = newElement(i-A->O.x,j-A->O.y,s-A->O.z);
                        p->prox = C;
                        C = p;
                     }
                  }
               }
   }
   return C;
}

void clearComponent( image *A, point &Q, int viz = EXTREMAL ) {

   element *FILA = newElement(Q.x+A->O.x,Q.y+A->O.y,Q.z+A->O.z);
   FILA->prox = FILA;
   point * vectors = viz == EXTREMAL ? vector_EXT : vector_ORT;
   int * dimVect = A->S == 1 ? d2Vector : d3Vector;

   while (FILA) {
      element *q = FILA->prox; /*topo*/

      A->f[q->p.x][q->p.y][q->p.z] = 0;
      for ( int v = 0; v < dimVect[viz]; v ++ ) {
         int M = q->p.x+vectors[v].x, N = q->p.y+vectors[v].y, S = q->p.z+vectors[v].z;
         if ( M >= 0 && M < A->M && N >= 0 && N < A->N && S >= 0 && S < A->S && A->f[M][N][S] ) {
            A->f[M][N][S] = 0;         
            element *p = newElement(M,N,S);
            p->prox = q->prox;
            if ( q->prox == FILA ) {
               q->prox = p;
               FILA = p;
            }
         }
      }

      FILA->prox = q->prox;
      if ( FILA == q )
         FILA = NULL;
      delete q;
   }
}

image * contourDilation ( image *A, image *B, image *C = NULL ) {

   image * out;
   if ( !C )
      out = newImage ( A->M, A->N, A->S, false );
   else
      out = C;

   /* copy image A in out */
   for ( int i = 0; i < A->M; i++ )
      for ( int j = 0; j < A->N; j++ )
         for ( int k = 0; k < A->S; k++ )
            out->f[i-A->O.x+out->O.x][j-A->O.y+out->O.y][k-A->O.z+out->O.z] =  A->f[i][j][k];

   element *p, *countA = getCountour(A), *allB = NULL;

   /* A princ�pio a origem est� em B */
   if ( !B->f[B->O.x][B->O.y][B->O.z] ) {
      printf ( "\nThe origin don't is in Structuring Element" );
      return NULL;
   }
   transpose(B);
   for ( int i = B->M - 1; i >= 0 ; i-- )
      for ( int j = B->N - 1; j >= 0 ; j-- )
         for ( int s = B->S - 1; s >= 0 ; s-- )
            if ( B->f[i][j][s] ) {
               p = newElement(i-B->O.x,j-B->O.y,s-B->O.z);
               p->prox = allB;
               allB = p;
            }

   /* aplicando somente os pontos de B nos cont. de A */
   for ( element *a = countA; a; a = a->prox )
      for ( element *b = allB; b; b = b->prox ) {
         /* transposta de B + ponto do contorno */
         int m = a->p.x + b->p.x + out->O.x, n = a->p.y + b->p.y + out->O.y, s = a->p.z + b->p.z + out->O.z;
         if ( m >= 0 && n >= 0 && s >= 0 && m < out->M && n < out->N && s < out->S )
            out->f[m][n][s] = true;
      }

   transpose(B);

   flushElement ( countA );
   flushElement ( allB );

   return out;
}

image * biContourDilation ( image *A, image *B, image *C = NULL ) {

   image * out;
   if ( !C )
      out = newImage ( A->M, A->N, A->S, false );
   else
      out = C;

   /* copy image A in out */
   for ( int i = 0; i < A->M; i++ )
      for ( int j = 0; j < A->N; j++ )
         for ( int k = 0; k < A->S; k++ )
            out->f[i-A->O.x+out->O.x][j-A->O.y+out->O.y][k-A->O.z+out->O.z] =  A->f[i][j][k];

   element *p, *countA = getCountour(A),
              *countB = getCountour(B),
              *allB = NULL, *ptosCompA = NULL;

   /* A princ�pio a origem est� em B */
   if ( !B->f[B->O.x][B->O.y][B->O.z] ) {
      printf ( "\nThe origin don't is in Structuring Element" );
      return NULL;
   }
   transpose(B);
   for ( int i = B->M - 1; i >= 0 ; i-- )
      for ( int j = B->N - 1; j >= 0 ; j-- )
         for ( int s = B->S - 1; s >= 0 ; s-- )
            if ( B->f[i][j][s] ) {
               p = newElement(i-B->O.x,j-B->O.y,s-B->O.z);
               p->prox = allB;
               allB = p;
            }

   /* Extra��o de componentes conexas */
   /* O objetivo eh pegar um ponto de cada componente conexa */
   image * Cimage = newImage(A->M,A->N,A->S,false);
   for ( element *a = countA; a; a = a->prox )
	   Cimage->f[a->p.x+A->O.x][a->p.y+A->O.y][a->p.z+A->O.z] = true;
   for ( int i = 0; i < Cimage->M; i++ )
      for ( int j = 0; j < Cimage->N; j++ )
         for ( int k = 0; k < Cimage->S; k++ )
            if ( Cimage->f[i][j][k] ) {
               point q;
               q.x = i-A->O.x; q.y = j-A->O.y; q.z = k-A->O.z;
               p = newElement(q);
               p->prox = ptosCompA;
               ptosCompA = p;
               /* exluindo demais pontos da componente */
               clearComponent(Cimage,q);
            }

   /* inicialmente, varrendo listas que usam a borracha allB */
   for ( element *a = ptosCompA; a; a = a->prox ) {
      for ( element *b = allB; b; b = b->prox ) {
         /* transposta de B + ponto do contorno */
         int m = a->p.x + b->p.x + out->O.x, n = a->p.y + b->p.y + out->O.y, s = a->p.z + b->p.z + out->O.z;
         if ( m >= 0 && n >= 0 && s >= 0 && m < out->M && n < out->N && s < out->S )
            out->f[m][n][s] = true;
      }
   }

   /* agora sim: aplicando somente os contornos de B nos cont. de A */
   for ( element *a = countA; a; a = a->prox )
      for ( element *b = countB; b; b = b->prox ) {
         /* transposta de B + ponto do contorno */
         int m = a->p.x + b->p.x + out->O.x, n = a->p.y + b->p.y + out->O.y, s = a->p.z + b->p.z + out->O.z;
         if ( m >= 0 && n >= 0 && s >= 0 && m < out->M && n < out->N && s < out->S )
            out->f[m][n][s] = true;
      }

   transpose(B);

   flushElement ( countA );
   flushElement ( countB );
   flushElement ( allB );
   flushElement ( ptosCompA );
   flushImage( Cimage );

   return out;

}

image * densityDilation ( image *A, image *B, image *C = NULL, int DIM1 = 1, int DIM2 = 0, int DIM3 = 1, int VIZ = EXTREMAL ) {

   /* Infimum of P(E) */
   image * out, * temp;
   if ( !C )
      out = newImage ( A->M, A->N, A->S, false );
   else
      out = C;

   temp = newImage ( A->M+B->M, A->N+B->N, A->S+B->S, false );

   /* De alguma forma, selecionar melhor os par�metros */
   int DIR = DIRECT;

   setMinimalShell(A,VIZ,DIM1,DIM2,DIM3,DIR);
   setMinimalShell(B,VIZ,DIM1,DIM2,DIM3,DIR);

   /* building List A- e B-Shell */
   element *rootB = NULL, *rootA = NULL, *aux;
   for ( int k = A->S-1; k >= 0; k-- )
      for ( int i = A->M-1; i >= 0; i-- )
         for ( int j = A->N-1; j >= 0; j-- )
            if ( A->f[i][j][k] > 0 ) {
               aux = newElement(i,j,k);
               aux->prox = rootA;
               aux->size = A->f[i][j][k];
               rootA = aux;
            }
   for ( int k = B->S-1; k >= 0; k-- )
      for ( int i = B->M-1; i >= 0; i-- )
         for ( int j = B->N-1; j >= 0; j-- )
            if ( B->f[i][j][k] > 0 ) {
               aux = newElement(i,j,k);
               aux->prox = rootB;
               aux->size = B->f[i][j][k];
               rootB = aux;
            }

   /* origem identica ao conj. A */
   point w;
   for ( element *p = rootA; p; p = p->prox )
      for ( element *q = rootB; q; q = q->prox ) {
         w.x = p->p.x+q->p.x-B->O.x;
         w.y = p->p.y+q->p.y-B->O.y;
         w.z = p->p.z+q->p.z-B->O.z;
         if ( w.x >= 0 && w.y >= 0 && w.z >= 0 && w.x < temp->M && w.y < temp->N && w.z < temp->S ) {
            int val = A->f[p->p.x][p->p.y][p->p.z] + B->f[q->p.x][q->p.y][q->p.z] - 1;
            temp->f[w.x][w.y][w.z] = temp->f[w.x][w.y][w.z] < val ? val : temp->f[w.x][w.y][w.z];
         }
      }

   unsetShell(A,VIZ,DIM1,DIM2,DIM3,DIR);
   unsetShell(B,VIZ,DIM1,DIM2,DIM3,DIR);
   unsetShell(temp,VIZ,DIM1,DIM2,DIM3,DIR);

   for ( int k = out->S-1; k >= 0; k-- )
      for ( int i = out->M-1; i >= 0; i-- )
         for ( int j = out->N-1; j >= 0; j-- )
             out->f[i][j][k] = temp->f[i][j][k];

   flushElement(rootA);
   flushElement(rootB);
   flushImage(temp);
   return out;
}

element * rleDilation ( element *RLEa, element *RLEb, image *B, image *C = NULL ) {

   element *RLE = NULL;
   point P;

   bool first = true;
   for ( element *q = RLEb; q; q = q->prox ) {
      element *a = NULL, *b = RLE, *last = NULL;
      for ( element *p = RLEa; p; p = p->prox ) {

         P.x = p->p.x + (q->p.x-B->O.x);
         P.y = p->p.y + (q->p.y-B->O.y);
         P.z = p->p.z + (q->p.z-B->O.z);
         if ( !last ) {
            a = newElement(P, p->size + q->size - 1);
            last = a;
         } else {
            last->prox = newElement(P, p->size + q->size - 1);
            last = last->prox;
         }
      }

      RLE = unionRLE(a,b);
   }

   return RLE;
}

image * decompDilation ( image *A, image *B, image *C = NULL ) {

   image * out1;
   if ( !C )
      out1 = newImage ( A->M, A->N, A->S, false );
   else
      out1 = C;
   image * out2 = newImage ( A->M, A->N, A->S, false );
   image *outEval = A, *outCur = out1;
   int k = B->N/2;

   image *d3x3;

   if ( typeES == QUAD ) {
      if ( B->S == 1 ) {
         d3x3 = newImage( 3, 3, B->S, true );
      }  else if ( B->S == 3 ) {
         d3x3 = newImage( 3, 3, B->S, true );
      } else
         return NULL;
   } else if ( typeES == DIAMOND ) {
      d3x3 = newImage( 3, 3, B->S, false );
      if ( B->S == 1 ) {
         d3x3->f[0][1][0] = d3x3->f[1][0][0] = d3x3->f[1][1][0] = d3x3->f[1][2][0] = d3x3->f[2][1][0] = true;
      }  else if ( B->S == 3 ) {
         d3x3->f[1][1][0] = true;
         d3x3->f[0][1][1] = d3x3->f[1][0][1] = d3x3->f[1][1][1] = d3x3->f[1][2][1] = d3x3->f[2][1][1] = true;
         d3x3->f[2][1][2] = true;
      } else
         return NULL;
   } else if ( typeES == LINE ) {
      d3x3 = newImage( 1, 3, 1, false );
      d3x3->f[0][0][0] = d3x3->f[0][1][0] = d3x3->f[0][2][0] = true;
   } else
      return NULL;

   /* here can be tried any erosion method */
   for ( int i = 0; i < k; i++ ) {

      /* insert here the method */
      outCur = bddDilation ( outEval, d3x3, outCur );

      if ( outEval != out1 ) {
         outEval = out1;
         outCur = out2;
      } else {
         outEval = out2;
         outCur = out1;
      }
   }

   flushImage(d3x3);
   flushImage(outCur);
   return outEval;
}

int evalDil ( image *A, element *root, point &delta ) {
   if ( !root ) {
   	return 0;
   } else if ( delta.x + root->p.x >= 0 && delta.y + root->p.y >= 0 && delta.z + root->p.z >= 0 &&
        delta.x + root->p.x < A->M && delta.y + root->p.y < A->N && delta.z + root->p.z < A->S ) {
      if ( A->f[delta.x + root->p.x][delta.y + root->p.y][delta.z + root->p.z] >= root->size )
         return 1;
   }
   return evalDil ( A, root->prox, delta );
}

image * decompAndContourDilation ( image *A, image *B, image *C = NULL ) {

   image * out;
   if ( !C )
      out = newImage ( A );
   else
      out = C;

   int k = B->N/2;

   element *p, *countA = getCountour(A), *allB = NULL;
   image *d3x3;

   if ( typeES == QUAD ) {
      if ( B->S == 1 ) {
         d3x3 = newImage( 3, 3, B->S, true );
      }  else if ( B->S == 3 ) {
         d3x3 = newImage( 3, 3, B->S, true );
      } else
         return NULL;
   } else if ( typeES == DIAMOND ) {
      d3x3 = newImage( 3, 3, B->S, false );
      if ( B->S == 1 ) {
         d3x3->f[0][1][0] = d3x3->f[1][0][0] = d3x3->f[1][1][0] = d3x3->f[1][2][0] = d3x3->f[2][1][0] = true;
      }  else if ( B->S == 3 ) {
         d3x3->f[1][1][0] = true;
         d3x3->f[0][1][1] = d3x3->f[1][0][1] = d3x3->f[1][1][1] = d3x3->f[1][2][1] = d3x3->f[2][1][1] = true;
         d3x3->f[2][1][2] = true;
      } else
         return NULL;
   } else if ( typeES == LINE ) {
      d3x3 = newImage( 1, 3, 1, false );
      d3x3->f[0][0][0] = d3x3->f[0][1][0] = d3x3->f[0][2][0] = true;
   } else
      return NULL;

   /* A princ�pio a origem est� em B */
   if ( !B->f[B->O.x][B->O.y][B->O.z] ) {
      printf ( "\nThe origin don't is in Structuring Element" );
      return NULL;
   }

   transpose(B);
   for ( int i = d3x3->M - 1; i >= 0 ; i-- )
      for ( int j = d3x3->N - 1; j >= 0 ; j-- )
         for ( int s = d3x3->S - 1; s >= 0 ; s-- )
            if ( d3x3->f[i][j][s] ) {
               p = newElement(i-d3x3->O.x,j-d3x3->O.y,s-d3x3->O.z);
               p->prox = allB;
               allB = p;
            }
   transpose(B);

   /* here can be tried any erosion method */
   for ( int i = 0; i < k; i++ ) {

      /* insert here the method */
      for ( element *a = countA; a;  ) {
         for ( element *b = allB; b; b = b->prox ) {
            /* transposta de B + ponto do contorno */
            int m = a->p.x + b->p.x + out->O.x, n = a->p.y + b->p.y + out->O.y, s = a->p.z + b->p.z + out->O.z;
            if ( m >= 0 && n >= 0 && s >= 0 && m < out->M && n < out->N && s < out->S )
               out->f[m][n][s] = true;
         }

         element *lixo = a;
         a = a->prox;
         delete lixo;
      }
      countA = getCountour(out);
   }

   flushImage(d3x3);
   flushElement ( countA );
   flushElement ( allB );
   return out;
}

image * decompAndCPUDilation ( image *A, image *B, image *C = NULL ) {

   image * out;
   if ( !C )
      out = newImage ( A->M, A->N, A->S );
   else
      out = C;

   int k = B->N/2;
   int round = (A->N/32) + ( (A->N%32>0) ? 1 : 0 ) + 2;
   image * temp  = newImage ( A->M, round, A->S );
   image * tempA = newImage ( A->M, round, A->S, 0x00000000 );
   image *d3x3;

   /* compress�o */
   int2BitVector ( A, tempA );

   if ( typeES == QUAD ) {
      if ( B->S == 1 ) {
         d3x3 = newImage( 3, 3, B->S, true );
      }  else if ( B->S == 3 ) {
         d3x3 = newImage( 3, 3, B->S, true );
      } else
         return NULL;
   } else if ( typeES == DIAMOND ) {
      d3x3 = newImage( 3, 3, B->S, false );
      if ( B->S == 1 ) {
         d3x3->f[0][1][0] = d3x3->f[1][0][0] = d3x3->f[1][1][0] = d3x3->f[1][2][0] = d3x3->f[2][1][0] = true;
      }  else if ( B->S == 3 ) {
         d3x3->f[1][1][0] = true;
         d3x3->f[0][1][1] = d3x3->f[1][0][1] = d3x3->f[1][1][1] = d3x3->f[1][2][1] = d3x3->f[2][1][1] = true;
         d3x3->f[2][1][2] = true;
      } else
         return NULL;
   } else if ( typeES == LINE ) {
      d3x3 = newImage( 1, 3, 1, false );
      d3x3->f[0][0][0] = d3x3->f[0][1][0] = d3x3->f[0][2][0] = true;
   } else
      return NULL;

   /* here can be tried any erosion method */
   for ( int K = 0; K < k; K++ ) {

      /* insert here the method */
      for ( int z = 0; z < tempA->S; z++ )
         for ( int x = 0; x < tempA->M; x++ )
            for ( int y = 0; y < tempA->N; y++ )
               temp->f[x][y][z] = 0x00000000;

      for ( int s = 0; s < d3x3->S; s++ )
         for ( int i = 0; i < d3x3->M; i++ )
            for ( int j = 0; j < d3x3->N; j++ )
               if ( d3x3->f[i][j][s] > 0 ) {
                  int a = i-d3x3->O.x, b = j-d3x3->O.y, c = s-d3x3->O.z;
                  int bMod = b%32;
                  shift ( tempA, bMod );
                  /* translade (a,b) and aply AND OP */
                  for ( int z = 0; z < temp->S; z++ )
                     for ( int x = 0; x < temp->M; x++ )
                        for ( int y = 0; y < temp->N; y++ ) {
                           int M = x - a, N = y - b/32, S = z - c;
                           if ( M >= 0 && N >= 0 && S >= 0 && M < tempA->M && N < tempA->N && S < tempA->S )
                              temp->f[x][y][z] = (unsigned int)tempA->f[M][N][S] | temp->f[x][y][z];
                      }
                  shift ( tempA, -bMod );
               }

      for ( int z = 0; z < tempA->S; z++ )
         for ( int x = 0; x < tempA->M; x++ )
            for ( int y = 0; y < tempA->N; y++ )
               tempA->f[x][y][z] = (unsigned int)temp->f[x][y][z];
   }

   /* descompress�o */
   bitVector2Int ( temp, out );

   flushImage(d3x3);
   flushImage(temp);
   flushImage(tempA);
   return out;
}
