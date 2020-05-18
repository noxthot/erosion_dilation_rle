/* EFFICIENT BINARY EROSIONS MORPHOLOGICAL */
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////
                                                                               //
/* in Matrices */                                                              //
image * naiveErosion ( image *, image *, image * );                            //
image * listErosion ( image *, image *, image * );                             //
image * CPUErosion ( image *, image *, image * );                              //
image * decompErosion ( image *, image *, image * );                           //
image * bddErosion ( image *, image *, image * );                              //
image * densityErosion ( image *, image *, image *, int, int, int );           //
image * iDensityErosion ( image *, image *, image *, int, int, int );          //
image * densityErosionImp1 ( image *, image *, image *, int, int, int);        //
image * densityErosionImp2 ( image *, image *, image *, int, int, int);        //
element * rleErosion ( element *, element *, image *, image * );                              //
image * decompAndCPUErosion ( image *, image *, image * );                     //
    int evalEro ( image *, element *, point& );                                //
    int evalEroDirect ( image *, element *, point&, int& );                    //
    int evalEroInverse ( image *, element *, point&, int& );                   //
                                                                               //
/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

/*                                    *\
   EROSIONS PROCEDURES WITH MATRICES
\*                                    */
image * naiveErosion ( image *A, image *B, image *C = NULL ) {

   /* Supremum of P(E) */
   image * out;
   if ( !C )
      out = newImage ( A->M, A->N, A->S, true );
   else
      out = C;

   for ( int s = 0; s < B->S; s++ )
      for ( int i = 0; i < B->M; i++ )
         for ( int j = 0; j < B->N; j++ )
            if ( B->f[i][j][s] > 0 ) {
               int a = B->O.x-i, b = B->O.y-j, c = B->O.z-s;
               /* translade (a,b) and aply AND OP */
               for ( int z = 0; z < A->S; z++ )
                  for ( int x = 0; x < A->M; x++ )
                     for ( int y = 0; y < A->N; y++ ) {
                        int M = x + a, N = y + b, S = z + c;
                        if ( M >= 0 && N >= 0 && S >= 0 && M < A->M && N < A->N && S < A->S )
                           out->f[x][y][z] = (A->f[M][N][S] > 0) && out->f[x][y][z];
                        else
                          /* out of range */
                          out->f[x][y][z] = false;
                     }
            }

   return out;
}

image * listErosion ( image *A, image *B, image *C = NULL ) {

   /* Infimum of P(E) */
   image * out;
   if ( !C )
      out = newImage( A->M, A->N, A->S, false );
   else
      out = C;

   element *o = NULL, *last = NULL, *lixo;
   element *aL = setElement(A);
   element *bL = setElement(B);
   point P;

   bool first = true;
   for ( element *q = bL; q; q = q->prox ) {
      P.x = (B->O.x-q->p.x);
      P.y = (B->O.y-q->p.y);
      P.z = (B->O.z-q->p.z);

      translade (aL, P);
      if ( first ) {
         element *cl = aL;
         if ( cl ) {
            o = newElement (cl);
            last = o;
            for ( cl = cl->prox; cl; cl = cl->prox ) { last->prox = newElement (cl); last = last->prox; }
            first = false;
         }             
      } else {
         element *b = intersectionElement(o,aL);
         o = b;
      }
      translade (aL, P, -1);
   }

   getElement(out,o);
   flushElement(o);
   flushElement(aL);
   flushElement(bL);

   return out;
}

image * CPUErosion ( image *A, image *B, image *C = NULL ) {

   /* Supremum of P(E) */
   image * out;
   if ( !C )
      out = newImage ( A->M, A->N, A->S, true  );
   else
      out = C;

   int round = (A->N/32) + ( (A->N%32>0) ? 1 : 0 ) + 2;
   image * temp  = newImage ( A->M, round, A->S, 0xFFFFFFFF );
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
                        int M = x + a, N = y + b/32, S = z + c;
                        if ( M >= 0 && N >= 0 && S >= 0 && M < tempA->M && N < tempA->N && S < tempA->S )
                           temp->f[x][y][z] = (unsigned int)tempA->f[M][N][S] & temp->f[x][y][z];
                        else
                           temp->f[x][y][z] = 0;
                     }
               shift ( tempA, -bMod );
            }

   /* descompress�o */
   bitVector2Int ( temp, out );

   flushImage(temp);
   flushImage(tempA);

   return out;
}

image * bddErosion ( image *A, image *B, image *C = NULL ) {

   /* Supremum of P(E) */
   image * out;
   if ( !C )
      out = newImage ( A->M, A->N, A->S, true );
   else
      out = C;

   /* building BDD */
   element *root = NULL, *aux;
   for ( int k = B->S-1; k >= 0; k-- )
      for ( int i = B->M-1; i >= 0; i-- )
         for ( int j = B->N-1; j >= 0; j-- )
            if ( B->f[i][j][k] > 0 ) {
               aux = newElement(i-B->O.x,j-B->O.y,k-B->O.z);
               aux->prox = root;
               root = aux;
            }

   point p;
   for ( int k = 0; k < A->S; k++ )
      for ( int i = 0; i < A->M; i++ )
         for ( int j = 0; j < A->N; j++ ) {
            p.x = i; p.y = j; p.z = k;
            out->f[i][j][k] = evalEro(A,root,p);
         }

   flushElement(root);
   return out;
}

image * decompErosion ( image *A, image *B, image *C = NULL ) {

   image * out1;
   if ( !C )
      out1 = newImage ( A->M, A->N, A->S, true );
   else
      out1 = C;
   image * out2 = newImage ( A->M, A->N, A->S, true );
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
      d3x3 = newImage( 1, 3, 1, true ); 
   } else
      return NULL;
      
   /* here can be tried any erosion method */
   for ( int i = 0; i < k; i++ ) {

      /* insert here the method */
      outCur = bddErosion ( outEval, d3x3, outCur );

      if ( outEval != out1 ) {
         outEval = out1;
         outCur = out2;
      } else {
         outEval = out2;
         outCur = out1;
      }
   }

   flushImage(outCur);
   return outEval;
}

/* De alguma forma, selecionar melhor os par�metros : DIMi = 1 significa que o eixo i � fixo na busca */
image * densityErosion ( image *A, image *B, image *C = NULL, int DIM1 = 1, int DIM2 = 0, int DIM3 = 1) {

   /* Infimum of P(E) */
   image * out;
   if ( !C )
      out = newImage ( A->M, A->N, A->S, false );
   else
      out = C;

   int DIR = DIRECT, VIZ = ORTONORMAL;
   setDensity(A,VIZ,DIM1,DIM2,DIM3,DIR);
   setMinimalShell(B,VIZ,DIM1,DIM2,DIM3,DIR);

   /* building List B-Shell */
   element *root = NULL, *aux;
   for ( int k = B->S-1; k >= 0; k-- )
      for ( int i = B->M-1; i >= 0; i-- )
         for ( int j = B->N-1; j >= 0; j-- )
            if ( B->f[i][j][k] > 0 ) {
               aux = newElement(i-B->O.x,j-B->O.y,k-B->O.z);
               aux->prox = root;
               aux->size = B->f[i][j][k];
               root = aux;
            }

   if ( !DIM1 ) {
      point p;
      for ( int k = 0; k < A->S; k++ )
         for ( int j = 0; j < A->N; j++ ) {
            int disp = 1;
            for ( int i = 0; i < A->M; i+=disp ) {
               p.x = i; p.y = j; p.z = k;
               out->f[i][j][k] = evalEroDirect(A,root,p,disp);
            }
         }
   } else if ( !DIM2 ) {
      point p;
      for ( int k = 0; k < A->S; k++ )
         for ( int i = 0; i < A->M; i++ ) {
            int disp = 1;
            for ( int j = 0; j < A->N; j+=disp ) {
               p.x = i; p.y = j; p.z = k;
               out->f[i][j][k] = evalEroDirect(A,root,p,disp);
            }
         }
   } else {
      point p;
      for ( int j = 0; j < A->N; j++ )
         for ( int i = 0; i < A->M; i++ ) {
            int disp = 1;
            for ( int k = 0; k < A->S; k+=disp ) {
               p.x = i; p.y = j; p.z = k;
               out->f[i][j][k] = evalEroDirect(A,root,p,disp);
            }
         }
   }

   unsetDensity(A);
   unsetShell(B,VIZ,DIM1,DIM2,DIM3,DIR);
   flushElement(root);
   return out;
}

/* com as densidades na lista do e.e. ordenada */
image * densityErosionImp1 ( image *A, image *B, image *C = NULL, int DIM1 = 1, int DIM2 = 0, int DIM3 = 1) {

   /* Infimum of P(E) */
   image * out;
   if ( !C )
      out = newImage ( A->M, A->N, A->S, false );
   else
      out = C;

   int DIR = DIRECT, VIZ = ORTONORMAL;
   setDensity(A,VIZ,DIM1,DIM2,DIM3,DIR);
   setMinimalShell(B,VIZ,DIM1,DIM2,DIM3,DIR);
   int MAXDIM = B->S < B->N ? B->N : B->S;
   MAXDIM = MAXDIM < B->M ? B->M : MAXDIM;
   MAXDIM++;

   /* building (Ordened by dens) List B-Shell */
   element **HASH = new element *[MAXDIM];
   for ( int i = 0; i < MAXDIM; i++ )
      HASH[i] = NULL;
   element *root = NULL, *aux;
   for ( int k = B->S-1; k >= 0; k-- )
      for ( int i = B->M-1; i >= 0; i-- )
         for ( int j = B->N-1; j >= 0; j-- )
            if ( B->f[i][j][k] > 0 ) {
               aux = newElement(i-B->O.x,j-B->O.y,k-B->O.z);
               aux->size = B->f[i][j][k];
               if ( !HASH[B->f[i][j][k]] ) {
                  HASH[B->f[i][j][k]] = aux;
                  aux->prox = HASH[B->f[i][j][k]];
               } else {
                  aux->prox = HASH[B->f[i][j][k]]->prox;
                  HASH[B->f[i][j][k]]->prox = aux;
               }
            }

   for ( int i = MAXDIM-1; i >= 0 ; i-- ) {
      if ( HASH[i] ) {
         if ( !root ) {
            root = HASH[i]->prox;
         } else {
            aux->prox = HASH[i]->prox;
         }
         aux = HASH[i];
      }
   }   
   aux->prox = NULL;     
   delete [] HASH;

   if ( !DIM1 ) {
      point p;
      for ( int k = 0; k < A->S; k++ )
         for ( int j = 0; j < A->N; j++ ) {
            int disp = 1;
            for ( int i = 0; i < A->M; i+=disp ) {
               p.x = i; p.y = j; p.z = k;
               out->f[i][j][k] = evalEroDirect(A,root,p,disp);
            }
         }
   } else if ( !DIM2 ) {
      point p;
      for ( int k = 0; k < A->S; k++ )
         for ( int i = 0; i < A->M; i++ ) {
            int disp = 1;
            for ( int j = 0; j < A->N; j+=disp ) {
               p.x = i; p.y = j; p.z = k;
               out->f[i][j][k] = evalEroDirect(A,root,p,disp);
            }
         }
   } else {
      point p;
      for ( int j = 0; j < A->N; j++ )
         for ( int i = 0; i < A->M; i++ ) {
            int disp = 1;
            for ( int k = 0; k < A->S; k+=disp ) {
               p.x = i; p.y = j; p.z = k;
               out->f[i][j][k] = evalEroDirect(A,root,p,disp);
            }
         }
   }

   unsetDensity(A);
   unsetShell(B,VIZ,DIM1,DIM2,DIM3,DIR);
   flushElement(root);
   return out;
}

/* com as densidades na lista do e.e. ordenada e com links negativos*/
image * densityErosionImp2 ( image *A, image *B, image *C = NULL, int DIM1 = 1, int DIM2 = 0, int DIM3 = 1 ) {

   /* Infimum of P(E) */
   image * out;
   if ( !C )
      out = newImage ( A->M, A->N, A->S, false );
   else
      out = C;

   int DIR = DIRECT, VIZ = ORTONORMAL;
   setDensity(A,VIZ,DIM1,DIM2,DIM3,DIR);
   setMinimalShell(B,VIZ,DIM1,DIM2,DIM3,DIR);
   int MAXDIM = B->S < B->N ? B->N : B->S;
   MAXDIM = MAXDIM < B->M ? B->M : MAXDIM;
   MAXDIM++;

   /* building (Ordened by dens) List B-Shell */
   element **HASH = new element *[MAXDIM];
   for ( int i = 0; i < MAXDIM; i++ )
      HASH[i] = NULL;
   element *root = NULL, *aux;
   for ( int k = B->S-1; k >= 0; k-- )
      for ( int i = B->M-1; i >= 0; i-- )
         for ( int j = B->N-1; j >= 0; j-- )
            if ( B->f[i][j][k] > 0 ) {
               aux = newElement(i-B->O.x,j-B->O.y,k-B->O.z);
               aux->size = B->f[i][j][k];
               if ( !HASH[B->f[i][j][k]] ) {
                  HASH[B->f[i][j][k]] = aux;
                  aux->prox = HASH[B->f[i][j][k]];
               } else {
                  aux->prox = HASH[B->f[i][j][k]]->prox;
                  HASH[B->f[i][j][k]]->prox = aux;
               }
            }

   for ( int i = MAXDIM-1; i >= 0 ; i-- ) {
      if ( HASH[i] ) {
         if ( !root ) {
            root = HASH[i]->prox;
         } else {
            aux->prox = HASH[i]->prox;
         }
         aux = HASH[i];
      }
   }   
   aux->prox = NULL;     
   delete [] HASH;

   if ( !DIM1 ) {
      setDensityNegativeLink(A,1);
      point p;
      for ( int k = 0; k < A->S; k++ )
         for ( int j = 0; j < A->N; j++ ) {
            int disp = 1;
            for ( int i = 0; i < A->M; i+=disp ) {
               p.x = i; p.y = j; p.z = k;
               out->f[i][j][k] = evalEroDirect(A,root,p,disp);
            }
         }
   } else if ( !DIM2 ) {
      setDensityNegativeLink(A,2);
      point p;
      for ( int k = 0; k < A->S; k++ )
         for ( int i = 0; i < A->M; i++ ) {
            int disp = 1;
            for ( int j = 0; j < A->N; j+=disp ) {
               p.x = i; p.y = j; p.z = k;
               out->f[i][j][k] = evalEroDirect(A,root,p,disp);
            }
         }
   } else {
      setDensityNegativeLink(A,3);
      point p;
      for ( int j = 0; j < A->N; j++ )
         for ( int i = 0; i < A->M; i++ ) {
            int disp = 1;
            for ( int k = 0; k < A->S; k+=disp ) {
               p.x = i; p.y = j; p.z = k;
               out->f[i][j][k] = evalEroDirect(A,root,p,disp);
            }
         }
   }

   unsetDensity(A);
   unsetShell(B,VIZ,DIM1,DIM2,DIM3,DIR);
   flushElement(root);
   return out;
}

image * iDensityErosion ( image *A, image *B, image *C = NULL, int DIM1 = 1, int DIM2 = 0, int DIM3 = 1  ) {

   /* Infimum of P(E) */
   image * out;
   if ( !C )
      out = newImage ( A->M, A->N, A->S, false );
   else
      out = C;

   int DIR = INVERSE, VIZ = ORTONORMAL;
   setDensity(A,VIZ,DIM1,DIM2,DIM3,DIR);
   setMinimalShell(B,VIZ,DIM1,DIM2,DIM3,DIR);

   /* building List B-Shell */
   element *root = NULL, *aux;
   for ( int k = B->S-1; k >= 0; k-- )
      for ( int i = B->M-1; i >= 0; i-- )
         for ( int j = B->N-1; j >= 0; j-- )
            if ( B->f[i][j][k] > 0 ) {
               aux = newElement(i-B->O.x,j-B->O.y,k-B->O.z);
               aux->prox = root;
               aux->size = B->f[i][j][k];
               root = aux;
            }

   point p;
   if ( !DIM1 ) {
      for ( int k = 0; k < A->S; k++ )
         for ( int j = 0; j < A->N; j++ ) {
            for ( int i = 0; i < A->M; ) {
               int disp = A->M;
               p.x = i; p.y = j; p.z = k;
               int v = evalEroInverse(A,root,p,disp);
               if ( v )
                  for ( int D = 0; D < disp; D ++ )
                     out->f[i+D][j][k] = true;
               i+=disp;
            }
         }
   } else if ( !DIM2 ) {
      for ( int k = 0; k < A->S; k++ )
         for ( int i = 0; i < A->M; i++ ) {
            for ( int j = 0; j < A->N; ) {
               int disp = A->N;
               p.x = i; p.y = j; p.z = k;
               int v = evalEroInverse(A,root,p,disp);
               if ( v )
                  for ( int D = 0; D < disp; D ++ )
                     out->f[i][j+D][k] = true;
               j+=disp;
            }
         }
   } else {

         for ( int i = 0; i < A->M; i++ )
            for ( int j = 0; j < A->N; j++ ) {
               for ( int k = 0; k < A->S; ) {
               int disp = A->S;
               p.x = i; p.y = j; p.z = k;
               int v = evalEroInverse(A,root,p,disp);
               if ( v )
                  for ( int D = 0; D < disp; D ++ )
                     out->f[i][j][k+D] = true;
               k+=disp;
            }
         }
   }
   unsetDensity(A);
   unsetShell(B,ORTONORMAL,DIM1,DIM2,DIM3,DIR);
   flushElement(root);
   return out;
}

element * rleErosion ( element *RLEa, element *RLEb, image *B, image *C = NULL ) {

   /* Infimum of P(E) */
   element *RLE = NULL;

   point P;

   bool first = true;
   for ( element *q = RLEb; q; q = q->prox ) {
      element *a = NULL, *b = RLE, *last = NULL;
      for ( element *p = RLEa; p; p = p->prox ) {
         if ( q->size <= p->size ) {
            P.x = p->p.x + (B->O.x - q->p.x);
            P.y = p->p.y + (B->O.y - q->p.y);
            P.z = p->p.z + (B->O.z - q->p.z);
            if ( !last ) {
               a = newElement(P, p->size - q->size + 1);
               last = a;
            } else {
               last->prox = newElement(P, p->size - q->size + 1);
               last = last->prox;
            }
         }
      }

      if ( first ) {
         RLE = a;
         first = false;
      } else {
         RLE = intersectRLE(a,b);
         flushElement(a);
         flushElement(b);
      }
   }

   return RLE;
}

int evalEro ( image *A, element *root, point &delta ) {
   if ( !root ) {
   	return 1;
   /* caso se encontre na faixa */
   } else if ( delta.x + root->p.x >= 0 && delta.y + root->p.y >= 0 && delta.z + root->p.z >= 0 &&
        delta.x + root->p.x < A->M && delta.y + root->p.y < A->N && delta.z + root->p.z < A->S ) {
      if ( A->f[delta.x + root->p.x][delta.y + root->p.y][delta.z + root->p.z])
         return evalEro ( A, root->prox, delta );
   }
   return 0;
}

int evalEroDirect ( image *A, element *root, point &delta, int &disp ) {
   if ( !root ) {
      disp = 1;
   	return 1;
   /* caso se encontre na faixa */
   } else if ( delta.x + root->p.x >= 0 && delta.y + root->p.y >= 0 && delta.z + root->p.z >= 0 &&
        delta.x + root->p.x < A->M && delta.y + root->p.y < A->N && delta.z + root->p.z < A->S ) {
      if ( A->f[delta.x + root->p.x][delta.y + root->p.y][delta.z + root->p.z] >= root->size )
         return evalEroDirect ( A, root->prox, delta, disp );
      else
         disp = root->size - A->f[delta.x + root->p.x][delta.y + root->p.y][delta.z + root->p.z];
   } else
	   disp = 1;
   return 0;
}

int evalEroInverse ( image *A, element *root, point &delta, int &disp ) {

   if ( !root )
   	return 1;

   int X = delta.x + root->p.x, Y = delta.y + root->p.y, Z = delta.z + root->p.z;
   /* caso se encontre na faixa */
   if ( X >= 0 && Y >= 0 && Z >= 0 && X < A->M && Y < A->N && Z < A->S ) {
      if ( A->f[X][Y][Z] >= root->size ) {
         disp = disp < A->f[X][Y][Z] - root->size + 1 ? disp : A->f[X][Y][Z] - root->size + 1;
         return evalEroInverse ( A, root->prox, delta, disp );
      } else {
         /* n�o se pode garantir tal propriedade em cascas 2 ou 3d. */
         disp = A->f[X][Y][Z]+1;
      }
   } else
	   disp = 1;
   return 0;
}

image * decompAndCPUErosion ( image *A, image *B, image *C = NULL ) {

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
               temp->f[x][y][z] = 0xFFFFFFFF;

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
                           int M = x + a, N = y + b/32, S = z + c;
                           if ( M >= 0 && N >= 0 && S >= 0 && M < tempA->M && N < tempA->N && S < tempA->S )
                              temp->f[x][y][z] = (unsigned int)tempA->f[M][N][S] & temp->f[x][y][z];
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
