/* 
 * Calculo Numerico.
 * Professor Rogilson
 * 2o Ciencias da Computacao Noturno
 *
 *
 *
 * Linguagem: ANSI C
 * Ambiente testado: Linux , Freebsd.
 * Compilacao: cc gauss-seidel.c -o gseidel -lm 
 * necessario: Gcc ou compilador ANSI
 * opcional  : math.h ( math lib )
 *
 *
 * 
 * Algoritmo: Gauss-Seidl
 * Resolucao iterativa de Ax = B
 *
 * Entrada:
 * 
 * Numero de equacoes
 * n
 * A(ij)
 * B(i)
 * X0(i)
 * numero N maximo de iteracoes.
 * Erro
 *
 * Saida:
 *
 * Impressao na tela
 *
 */
 
#include <stdio.h>
#include <math.h>
#include <stdlib.h> 



double err_n (double *X);

double ** allocmat (int a, int b, int c, int d);
double * allocvec (int a,int b);
void free_vec(double *v, int a, int b);
void free_mat(double **m, int a,int b,int c,int d);




int n;				



main() {
  
	double **A, *X, *B, *X0, *TEMP, TOL, infinite_norm();
  	int i, j, k, N;

  	fprintf(stdout, "Algoritmo : Gauss-Seidel  \n");

  	fprintf(stdout,"Digite o numero maximo de iteracoes (N): ");
  	scanf("%d", &N);
  	fprintf(stdout, "Numero de iteracoes: %d\n", N);

  	fprintf(stdout,"Fator de erro (tolerancia) E (1.0e-4): ");
  	scanf("%lf", &TOL);
  	fprintf(stdout, "Tolerancia = %lg\n", TOL);

  	do {
    		fprintf(stdout, "Entre a dimensao  (n) da matriz A: ");
    		scanf("%d", &n);
    		if (n <= 0)
      		printf("n deve ser maior que zero (0) ! digite novamente .\n");
  	} while (n <= 0);
  	
  	fprintf(stdout, "O sistema tem %d equacoes e %d incognitas\n", n, n);

  	/* Alocacao dinamica das matrizes (malloc ) */
  	
  	
  	A    = allocmat(1,n,1,n);	/* matriz A          */
  	X    = allocvec(1,n);		/* vetor solucao     */
  	B    = allocvec(1,n);		/* matriz B          */
  	X0   = allocvec(1,n);		/* vetor inicial     */
  	TEMP = allocvec(1,n);		/* matriz temporaria */


  	fprintf(stdout, "Entre o coeficiente para a matriz A:\n");
  	
  	for (i=1;i<=n;i++)
    		for (j=1;j<=n;j++) {
      			printf("\tA[%d][%d] = ", i, j);
      			scanf("%lf", &A[i][j]);
    			}

  	fprintf(stdout, "A = ");	/* imprime matriz A */
  		for (i=1;i<=n;i++) {
    			fprintf(stdout, "[ ");
    			for (j=1;j<=n;j++)
      				fprintf(stdout, "% 9.9lg ", A[i][j]);
    			fprintf(stdout, "]\n    ");
  			}
  	
  	fprintf(stdout, "\n");

  	printf("Entre o coeficiente para a matriz B:\n");
  	
  	for (i=1;i<=n;i++) {
    		printf("\tB[%d] = ", i);
    		scanf("%lf", &B[i]);
  		}

  	fprintf(stdout, "B = [ ");	/* imprime vetor B */
  	
  	for (i=1;i<=n;i++) fprintf(stdout, "% 9.9lg ", B[i]);
  	
  	fprintf(stdout, "]t\n\n");

  	fprintf(stdout, "Entre o vetor de aproximacao X0:\n");	
  	for (i=1;i<=n;i++) {
    		fprintf(stdout, "\tX0[%d] = ", i);
    		scanf("%lf", &X0[i]);
  		}

  	fprintf(stdout, "X0 = [ ");	/* imprime X0. */
  	
  	for (i=1;i<=n;i++) fprintf(stdout, "% 9.9lg ", X0[i]);
  	
  	fprintf(stdout, "]t\n");

  	fprintf(stdout, "\n k\t");		/* header   */
  	
  	for (i=1;i<=n;i++) fprintf(stdout, " X[%d]\t\t", i);
  	
  	fprintf(stdout, "\n----------");
  	
  	for (i=1;i<=n;i++) fprintf(stdout, "---------------");
  	
  	fprintf(stdout, "\n");

  	fprintf(stdout, "% d\t", 0);		/* valores inic.    */
  	
  	for (i=1;i<=n;i++) fprintf(stdout, "% .9lf\t", X0[i]);
  	
  	fprintf(stdout,"\n");

	
	/* Algoritmo */
	

  	/* 1- 1a iteracao */
  	k = 1;

  	/* 2- loop das iteracoes */
  	while (k <= N) {	/* while (3->6). */

    	/* 3- pivos */
    	for (i=1;i<=n;i++) {
      		X[i] = 0.0;
      		for (j=1;j<=i-1;j++)
        		X[i] -= A[i][j] * X[j];
      		for (j=i+1;j<=n;j++)
        		X[i] -= A[i][j] * X0[j];
      		X[i] = (X[i] + B[i]) / A[i][i];
    		}

    	/* 4 - saida */
    	
    	
    	
    	fprintf(stdout, " %d\t", k);
    	
    	for (i=1;i<=n;i++) fprintf(stdout, "% .9lf\t", X[i]);
    	
    	fprintf(stdout, "\n");

    	for (i=1;i<=n;i++) TEMP[i] = X[i] - X0[i];
    	
    	if (fabs(err_n ( TEMP)) < TOL) {	/* testa o erro ( tolerancia ) */
      	
      		/* free () ( libera a memoria usada )*/
      	
      		free_vec(TEMP,1,n);
      		free_vec(X0,1,n);
      		free_vec(B,1,n);
      		free_vec(X,1,n);
      		free_mat(A,1,n,1,n);
      
      		exit (1);/* Fim */
    		}

    	/* 5- incrementa iteracao */
    	k++;		

    	/* 6- move entre matrizes */
    	for (i=1;i<=n;i++)
      		X0[i] = X[i];
  	}

  	/* 7- problemas na funcao */	
  	
  	fprintf(stdout, "\niteracoes (%d) ultrapassadas ( seguindo fator de tolerancia ) .\n", N);

  	
  	free_vec (TEMP,1,n);
  	free_vec (X0,1,n);
  	free_vec (B,1,n);
  	free_vec (X,1,n);
  	free_mat (A,1,n,1,n);

	}


double err_n (double *X) {
  	double max = 0.0;
  	int i;

  	for (i=1;i<=n;i++)
    		if (fabs(X[i]) > fabs(max))
      			max = X[i];

  	return (max);
	}

/* rotinas de alocacao dinamica de matrizes 	*/
/* necessarias para otimizar a performance e 	*/
/* economia de memoria				*/

double ** allocmat (int a, int b, int c, int d) {
	
	int i;
  	double **m;

  	/* alloc pointers para colunas */
  	m = (double **) calloc ((unsigned) (b - a + 1), sizeof (double *));
  	if (!m) {
  		fprintf  (stderr, "calloc falhou: (1)  dmatrix()");
  		exit(1);
  		}
  		
  	m -= a;

  	/* alloc colunas e seta pointers a elas */
  	
  	for (i = a; i <= b; i++) {
      		m[i] = (double *) calloc ((unsigned) (d - c + 1), sizeof (double));
      		if (!m[i]) {
			fprintf (stderr,"calloc falhou :  (2) dmatrix()");
      			exit(1);
      			}
      		
      		m[i] -= c;
    		}

  	/* retorna um array de ponteiros para colunas */
  	
  	return (m);
	}

double * allocvec(int a,int b) {
  	double *v;
  
    	v = (double *) calloc((unsigned) (b-a+1), sizeof(double));
      	
      	if (!v) {
        	fprintf (stderr,"calloc falhou : dvector()\n");
          	exit(1);
          	}
        return (v-a);
	}


void free_vec (double *v, int a, int b) {
  /* b para compatibilidade com mathlib */
  
  free((char *) (v+a));
  }


void free_mat (double **m, int a,int b,int c,int d) {
	int i;
  
    	for(i=b;i>=a;i--)
        	free((char *) (m[i]+c));
        free((char *) (m+a));
	}
                        