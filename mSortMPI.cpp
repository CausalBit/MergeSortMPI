/* MergeSort.c


	Este programa realiza el algoritmo paralelizado del MergeSort para ordenar una lista desordenada, el cual consiste en:
		Dividir la lista recursivamente en sublistas de aproximadamente la mitad del tamano hasta que su tamano sea 1
		Ordenar cada sublista recursivamente usando este algoritmo
		Mezclar cada sublista hasta que se forme una sola

	El programa ordena con p procesos una lista de n valores entre 0 y m, donde
		p es el numero de procesos, lo define el usuario en "mpiexcec". Debe de ser potencia de 2
		n es el tamaÃ±o de la lista a ordenas, se los pide al usuario el proceso raiz. Debe de se divisible por p
		m es el rango de posibles valores que pueden aparecer en la lista, se los pide al usuario el proceso raiz. Debe ser <=60

	Al final se despliega
		Lista ordenada, si el usuario lo desea
		Tiempo que tardo el ordenamieto
		Valores de p, n y m
		Numero de veces que aparecia en la lista cada uno de los posibles valores del 0 a m
*/

#include <stdio.h>
#include "mpi.h"
#include <stdlib.h>

#include<iostream>
#include <chrono>
#include <ctime>


int main( int argc, char *argv[] )
{
    int*  lista;        /* Lista que debe ser ordenada*/
    int* lista_local;
    int  n;            /*  Dimension de los 2 vectores dada por el usuario */
    int m;	       /*  Limite de numeros aleatorios de la lista */ 		
    int  p;            /*  Numero de procesos que corren */
    int  my_rank;      /*  Identificacion de cada proceso*/
    int r;
    int t;
    
    void Genera_vector(int lista[], int n, int m);   /* Genera valores aleatorios para n vector de enteros*/
    
    MPI_Init(&argc, &argv); /* Inicializacion del ambiente para MPI.
                            En C MPI_Init se puede usar para pasar los argumentos de la linea de comando
                            a todos los procesos, aunque no es requerido, depende de la implementacion */

    MPI_Comm_size(MPI_COMM_WORLD, &p); /* Se le pide al comunicador MPI_COMM_WORLD que 
                                          almacene en p el numero de procesos de ese comunicador.
                                          O sea para este caso, p indicara el num total de procesos que corren*/

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); /* Se le pide al comunicador MPI_COMM_WORL que devuelva en 
                                                la variable my_rank la identificacion del proceso "llamador" 
                                                la identificacion es un numero de 0 a p-1 */                          
       
       
    
    if (my_rank == 0) {   /* LO QUE SIGUE LO REALIZA UNICAMENTE EL PROCESO 0 (PROCESO RAIZ o PRINCIPAL) */
      std::cout << "Digite el tamano de la lista\n" >> endl;
      std::cin >> n;
    
      std::cout << "Digite el limite de valores aleatorios en la lista >> endl;
	    std::cin >> m;
    
      lista = (int *)malloc(n*sizeof(int));
	    if(lista==NULL){
		    cout << "Error al asignar memoria a la lista" << endl;
		    return 0; 
	    }
      Genera_vector(lista, n, m);
      
      r = n % p;
      t = (n - r)/p;
      
      MPI_Scatter(lista, t + 1, MPI_INT, lista_local, t + 1, MPI_INT, 0, MPI_COMM_WORLD);
      MPI_Scatter(lista, t, MPI_INT, lista_local, t, MPI_INT, 0, MPI_COMM_WORLD);
      
      MPI_Bcast(&t, 1, MPI_INT, 0, MPI_COMM_WORLD);
      MPI_Bcast(&r, 1, MPI_INT, 0, MPI_COMM_WORLD);
   }
   
   cout << "Yo soy el proceso " << my_rank;
   for(int i=0; i <= t+1; i++) {
        cout << lista_local[i] << "\n" << endl;
   }
   
}

void Genera_vector(int lista[], int n,  int m)
{
      int i;
      for (i = 0; i < n; i++)
        lista[i]= 0 + rand()%(m+1-0);                
}
