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

using namespace std;

int main( int argc, char *argv[] )
{
    int* lista;        /* Lista que debe ser ordenada*/
    int* lista_cantidadMenor;
    int* lista_local;
    int  n;            /*  Dimension de los 2 vectores dada por el usuario */
    int m;	       /*  Limite de numeros aleatorios de la lista */ 		
    int  p;            /*  Numero de procesos que corren */
    int  my_rank;      /*  Identificacion de cada proceso*/
    int r;
    int t;

    int cantidadMenorTag = 11011;
    
    void Genera_vector(int lista[], int n, int m);   /* Genera valores aleatorios para n vector de enteros*/
    
    MPI_Init(&argc, &argv); /* Inicializacion del ambiente para MPI. */

    MPI_Comm_size(MPI_COMM_WORLD, &p); //Cantidad de procesos inicializados. 

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); //El rango del proceso "local".              
       
    MPI_Comm balancer_comm; //Este comunicador sirve entre procesos que tengan el mísmo número de items. 
    MPI_Group original_group;

    MPI_Comm_group(MPI_COMM_WORLD, &original_group); //

    if (my_rank == 0) {   /* LO QUE SIGUE LO REALIZA UNICAMENTE EL PROCESO 0 (PROCESO RAIZ o PRINCIPAL) */
      std::cout << "Digite el tamano de la lista\n" << endl;
      std::cin >> n;
    
      std::cout << "Digite el limite de valores aleatorios en la lista" << endl;
	    std::cin >> m;
    
      lista = (int *)malloc(n*sizeof(int));
      if(lista==NULL){
		    std::cout << "Error al asignar memoria a la lista" << endl;
		    return 0; 
      }
      Genera_vector(lista, n, m);

    	for(int i=0; i < n; i++) {
        std::cout <<"parent WORLD: " << lista[i] << " ";
      }
      
      r = n % p;  //Restante de la division de items en la lista entre los procesos. 
                  //También indica los r primero procesos a recibir t+1 items.  
      t = (n - r)/p;  //Cuantos items por proceso para que tengan el mismo número de procesos (no contando los restantes).

      lista_cantidadMenor = lista+r;

      if(r != 0){
      MPI_Send (lista_cantidadMenor,n-r,MPI_INT,r,cantidadMenorTag, MPI_COMM_WORLD);  //Mandamos la lista de cantidades menores al pocesador que se convertira
                                                                                      //en la raiz de su comunicador que se crea al hacer el split después.
      std::cout << "0 is done sending lista_cantidadMenor" << std::endl; 
      }
      free(lista_cantidadMenor);
    }
	         
    MPI_Bcast(&t, 1, MPI_INT, 0, MPI_COMM_WORLD);
   	MPI_Bcast(&r, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

    if(my_rank == r-1)//Soy el proceso raíz del segundo subgrupo de comm. 
    {
      MPI_Recv(lista,n-r,MPI_INT,r-1,cantidadMenorTag,MPI_COMM_WORLD,MPI_STATUS_IGNORE); //Recibir la lista de cantidades menores
    }
    

    //Vamos a dividir los procesos en dos sub comunicadores. El de color 1 que recibe t+1 items en sus listas. 
    //El de color 2 que recibe t items. Esto para garantizar una manera de recibir cualquier cantidad de n de items. 

    int color = my_rank < r ? 1 : 0; //Esto es para calcular el color (identificar procesos de un comunicador) del proceso.

    //Aquí es donde se reparte los procesos según color y se identifican con el COMM handler balancer_comm. 
    MPI_Comm_split(MPI_COMM_WORLD, color, my_rank, &balancer_comm);

    //Ahora, dependiendo del color, la lista será del tamaño t+1, o t.
    lista_local = (int *)malloc((t+color)*sizeof(int));

    if(r > 0){
      //Vamos a repartir la lista. Para los procesos en COMM_WORLD con rank menor a r, esto reciben t+1, para los otro t. 
      MPI_Scatter(lista, t + 1, MPI_INT, lista_local, t + 1, MPI_INT, 0, balancer_comm);
      MPI_Scatter(lista, t, MPI_INT, lista_local, t, MPI_INT, r-1, balancer_comm);
    }else{
      lista_local = lista;
    }


  std::cout << "Yo soy el proceso " << my_rank << " del comunicador: "<< balancer_comm << " first: " << lista_local[0] << std::endl;
  
  if(my_rank == 0 || my_rank == r-1){
	   free(lista);
  }
  
  free(lista_local);

  MPI_Finalize();
   
   return 0;
}

void Genera_vector(int lista[], int n,  int m)
{
      int i;
      for (i = 0; i < n; i++) {
        //lista[i]= 0 + rand()%(m+1-0); 
	         lista[i]= i;                 
      }
}
