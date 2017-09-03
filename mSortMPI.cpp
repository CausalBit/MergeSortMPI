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
      int num_elementos_to_send = n - (2*r);

      lista_cantidadMenor = lista+2*r;

      for(int i = 0; i < num_elementos_to_send; i++){
        std::cout <<"tesnting lista cantidad menor: " << lista_cantidadMenor[i] << std::endl;
      }

      if(r != 0){
      std::cout << "About to send lista_cantidadMenor to process " << r << std::endl; 
      //MPI_Send (&buf,count,datatype,dest,tag,comm) 
      MPI_Send (lista_cantidadMenor,num_elementos_to_send,MPI_INT,r,cantidadMenorTag, MPI_COMM_WORLD);  //Mandamos la lista de cantidades menores al pocesador que se convertira
                                                                                      //en la raiz de su comunicador que se crea al hacer el split después.
      std::cout << "0 is done sending lista_cantidadMenor" << std::endl; 
      }

      //free(lista_cantidadMenor);
      lista_cantidadMenor = NULL;
    }
	       

    MPI_Bcast(&t, 1, MPI_INT, 0, MPI_COMM_WORLD);
   	MPI_Bcast(&r, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Barrier(MPI_COMM_WORLD); 
    if(my_rank == 0 ){std::cout << "Donde broadcasting with r = "<< r << std::endl; }



    if(my_rank == r)//Soy el proceso raíz del segundo subgrupo de comm. 
    {
      std::cout << "preparing list with n = " << n << std::endl;
      int num_elementos_to_rcv = n - 2*r;

      lista = (int *)malloc(num_elementos_to_rcv*sizeof(int));
          if(lista==NULL){
            std::cout << "Error al asignar memoria a la lista" << endl;
            return 0; 
          }
      std::cout << my_rank <<  " is bout to receive " << std::endl;
      //MPI_Recv (&buf,count,datatype,source,tag,comm,&status) 
      MPI_Recv(lista,n-2*r,MPI_INT,0,cantidadMenorTag,MPI_COMM_WORLD,MPI_STATUS_IGNORE); //Recibir la lista de cantidades menores
      std::cout << "Done receiving, starting testing: ";
      for(int i = 0; i < n - 2*r; i++){
        std::cout << "--" << lista[i] << std::endl;
      } 
      std::cout << "la lista está terminada" <<std::endl;
    }
     
    

    //Vamos a dividir los procesos en dos sub comunicadores. El de color 1 que recibe t+1 items en sus listas. 
    //El de color 2 que recibe t items. Esto para garantizar una manera de recibir cualquier cantidad de n de items.

   // std::cout << "Starting to calculate color for "<< my_rank <<std::endl; 

    int color = my_rank < r ? 1 : 0; //Esto es para calcular el color (identificar procesos de un comunicador) del proceso.
    std::cout << " pro "<< my_rank << "with color "<< color << std::endl; 
    MPI_Barrier(MPI_COMM_WORLD);

    //Aquí es donde se reparte los procesos según color y se identifican con el COMM handler balancer_comm. 
    MPI_Comm_split(MPI_COMM_WORLD, color, my_rank, &balancer_comm);
    std::cout << "After Splitiing about to allocate "<< my_rank << " with balancer: "<< balancer_comm <<std::endl; 

    //Ahora, dependiendo del color, la lista será del tamaño t+1, o t.
    lista_local = (int *)malloc((t+color)*sizeof(int));
     std::cout << "Done allocating "<< my_rank <<std::endl; 

     MPI_Barrier(MPI_COMM_WORLD);

    if(r > 0){
      //Vamos a repartir la lista. Para los procesos en COMM_WORLD con rank menor a r, esto reciben t+1, para los otro t. 
       // MPI_Scatter (&sendbuf,sendcnt,sendtype,&recvbuf,recvcnt,recvtype,root,comm)

        int size_to_scatter = color == 1 ? t+1: t;
      
       // MPI_Scatter(lista, t + 1, MPI_INT, lista_local, t + 1, MPI_INT, 0, balancer_comm);
      
    
        MPI_Scatter(lista, size_to_scatter, MPI_INT, lista_local, size_to_scatter, MPI_INT, 0, balancer_comm);

      
    }else{
      lista_local = lista;
    }

MPI_Barrier(MPI_COMM_WORLD);
  std::cout << "Yo soy el proceso " << my_rank <<  " first: " << lista_local[0] << std::endl;
  
  if(my_rank == 0 || my_rank == r){
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
