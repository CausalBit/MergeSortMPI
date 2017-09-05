/* MergeSort.c


	Este programa realiza el algoritmo paralelizado del MergeSort para ordenar una lista desordenada, el cual consiste en:
		Dividir la lista recursivamente en sublistas de aproximadamente la mitad del tamano hasta que su tamano sea 1
		Ordenar cada sublista recursivamente usando este algoritmo
		Mezclar cada sublista hasta que se forme una sola

	El programa ordena con p procesos una lista de n valores entre 0 y m, donde
		p es el numero de procesos, lo define el usuario en "mpiexcec".
		n es el tamaÃ±o de la lista a ordenar, se los pide al usuario el proceso raiz.
		m es el rango de posibles valores que pueden aparecer en la lista, se los pide al usuario el proceso raiz. Debe ser <=500

	Al final se despliega
		Lista ordenada, si el usuario lo desea
		Tiempo que tardo el ordenamieto
		Valores de p, n y m
		Numero de veces que aparecia en la lista cada uno de los posibles valores del 0 a m
*/

#include <stdio.h>
#include "mpi.h"
#include <stdlib.h>

#include <string>       // std::string
#include <iostream>     // std::cout
#include <sstream>

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
    int signal = 0;

    int cantidadMenorTag = 11011;
    
    void Genera_vector(int lista[], int n, int m);   /* Genera valores aleatorios para n vector de enteros*/
    int* mergeSort(int lista[], int numElem); /*Metodo que realiza el mergeSort*/
    int* merge(int mitad1[], int mitad2[], int nMitad1, int nMitad2);/*Metodo que hace merge a dos listas, y las une en una lista ordenada*/
    void cantidadValores(int lista[], int m, int tamanoLista);/*Metodo para imprimir la cantidad de veces que aparece cada numero en la lista */
    void mostrarLista(int lista[],int tamanoLista);/*Metodo que muestra la lista ordenada*/
    
    MPI_Init(&argc, &argv); /* Inicializacion del ambiente para MPI. */

    MPI_Comm_size(MPI_COMM_WORLD, &p); //Cantidad de procesos inicializados. 

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank); //El rango del proceso "local".              
       
    MPI_Comm balancer_comm, alive_comm; //Este comunicador sirve entre procesos que tengan el mísmo número de items. 
    MPI_Group original_group;

    MPI_Comm_group(MPI_COMM_WORLD, &original_group); //

    if (my_rank == 0) {   /* LO QUE SIGUE LO REALIZA UNICAMENTE EL PROCESO 0 (PROCESO RAIZ o PRINCIPAL) */

      n = 0;
      while( n <= 0 ) //Nunca se puede seguir si n no es mayor a 0. 
      {
        std::cout << "Digite el tamano de la lista, que sea mayor a 0." << endl;
        std::cin >> n;
      }
    
      std::cout << "Digite el limite de valores aleatorios en la lista" << endl;
	    std::cin >> m;
    
      lista = (int *)malloc(n*sizeof(int));
      if(lista==NULL){
		    std::cout << "Error al asignar memoria a la lista" << endl;
		    return 0; 
      }
      Genera_vector(lista, n, m);
      /*
    	for(int i=0; i < n; i++) {
        std::cout <<"parent WORLD: " << lista[i] << " ";
      }*/
      std::cout<<"numero de procesos " << p << std::endl;
      
      r = n % p;  //Restante de la division de items en la lista entre los procesos. 
                  //También indica los r primero procesos a recibir t+1 items.  
      r = r == n ? 0: r;
      t = n < p ? 1 : (n - r)/p;  //Cuantos items por proceso para que tengan el mismo número de procesos (no contando los restantes).
      if(r != 0){
        int elementos_r = ((t+1)*r);
        int num_elementos_to_send = n - elementos_r;
        lista_cantidadMenor = lista+elementos_r;

        for(int i = 0; i < num_elementos_to_send; i++){
          std::cout <<"testing lista cantidad menor: " << lista_cantidadMenor[i] << std::endl;
        }
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

    if(my_rank == r && r != 0 )//Soy el proceso raíz del segundo subgrupo de comm. 
    {
      std::cout << "preparing list with n = " << n << std::endl;
      int num_elementos_to_rcv = n - ((t+1)*r);

      lista = (int *)malloc(num_elementos_to_rcv*sizeof(int));
          if(lista==NULL){
            std::cout << "Error al asignar memoria a la lista" << endl;
            return 0; 
          }
      std::cout << my_rank <<  " is bout to receive " << std::endl;
      //MPI_Recv (&buf,count,datatype,source,tag,comm,&status) 
      MPI_Recv(lista,num_elementos_to_rcv ,MPI_INT,0,cantidadMenorTag,MPI_COMM_WORLD,MPI_STATUS_IGNORE); //Recibir la lista de cantidades menores
      std::cout << "Done receiving, starting testing: ";
      for(int i = 0; i < num_elementos_to_rcv; i++){
        std::cout << "--" << lista[i] << std::endl;
      } 
      std::cout << "la lista está terminada" <<std::endl;
    }
	
    //Vamos a dividir los procesos en dos sub comunicadores. El de color 1 que recibe t+1 items en sus listas. 
    //El de color 2 que recibe t items. Esto para garantizar una manera de recibir cualquier cantidad de n de items.

   // std::cout << "Starting to calculate color for "<< my_rank <<std::endl; 

    int color = my_rank < r ? 1 : 0; //Esto es para calcular el color (identificar procesos de un comunicador) del proceso.
    //std::cout << " pro "<< my_rank << "with color "<< color << std::endl; 
    MPI_Barrier(MPI_COMM_WORLD);

    //Aquí es donde se reparte los procesos según color y se identifican con el COMM handler balancer_comm. 
    MPI_Comm_split(MPI_COMM_WORLD, color, my_rank, &balancer_comm);
    //std::cout << "After Splitiing about to allocate "<< my_rank << " with balancer: "<< balancer_comm <<std::endl; 

    //Ahora, dependiendo del color, la lista será del tamaño t+1, o t.
    lista_local = (int *)malloc((t+color)*sizeof(int));
     //std::cout << "Done allocating "<< my_rank <<std::endl; 
  MPI_Barrier(MPI_COMM_WORLD);

   
      //Vamos a repartir la lista. Para los procesos en COMM_WORLD con rank menor a r, esto reciben t+1, para los otro t. 
       // MPI_Scatter (&sendbuf,sendcnt,sendtype,&recvbuf,recvcnt,recvtype,root,comm)

  int size_to_scatter = color == 1 ? t+1: t;
      
       // MPI_Scatter(lista, t + 1, MPI_INT, lista_local, t + 1, MPI_INT, 0, balancer_comm);
      
    
  MPI_Scatter(lista, size_to_scatter, MPI_INT, lista_local, size_to_scatter, MPI_INT, 0, balancer_comm);


  MPI_Barrier(MPI_COMM_WORLD);

  //**  
  int pro_id = my_rank + 1;
  int numero_nodos = n>=p? p: n;
  bool working = true;
  int tamanio_lista = size_to_scatter;
  int tamanio_tag = 11110;
  int lista_recibir_tag = 77777;

  //Merge-Sort la lista local. 
  /*int * lista_unida = (int *)malloc(n*sizeof(int));
  if(lista_unida==NULL){
    std::cout << "Error al asignar memoria a la lista" << endl;
    return 0; 
  }
  for(int i=0;i < tamanio_lista; i++){
    lista_unida[i] = lista_local[i];
  }*/

    std::stringstream sstp;
    sstp << "***SOy " << my_rank << " valores :";
    
    for(int x = 0; x < tamanio_lista; x++){
      sstp <<lista_local[x] << " , ";
    }
    sstp << std::endl;
    //result = sstm.str();
    std::cout << sstp.str();



  int* lista_unida = mergeSort(lista_local, tamanio_lista); 
  if(tamanio_lista > 1){free(lista_local);}

  int distance = 1;
 
 // MPI_Barrier(MPI_COMM_WORLD);
  while(working){

    
    
    if(pro_id%2 == 0 )
    {
      
      //Enviar mi lista al proceso previo: my_rank - 1;
      std::stringstream ssout;
      //<<" pro_id: "<<pro_id
      ssout << "SEND soy rank: "<<my_rank<<" TO: "<<my_rank-distance<<", tamanio_lista "<<tamanio_lista<< " total nodos: "<<numero_nodos <<std::endl;
      std::cout << ssout.str();

      //Primero mandar el tamaño de la lista. 
      //MPI_Send (&buf,count,datatype,dest,tag,comm) 
      MPI_Send(&tamanio_lista,1,MPI_INT,my_rank-distance,tamanio_tag, MPI_COMM_WORLD);
      //Mandar lista :D
      MPI_Send(lista_unida,tamanio_lista,MPI_INT,my_rank-distance,lista_recibir_tag, MPI_COMM_WORLD);

      working = false;
      
     
   
    }else{
      //MPI_Comm balancer_comm;
      //team = numero_nodos;
      //MPI_Comm_split(MPI_COMM_WORLD, team, my_rank, &balancer_comm);
      //Recibir la lista del proceso siguiente si es que existe:
      //Si my_rank + 2 <= numero_nodo.
      //Si recibo, entonces merge con la lista en local, la recibida. 
      //resulta en nueva lista local.
      if(pro_id+1 <= numero_nodos){
        //Recibir del siguiente nodo. 
        //Primero recibir el tamaños de la lista a recibir. 
        int tamanio_lista_recibir = 0;
        //MPI_Recv (&buf,count,datatype,source,tag,comm,&status) 
        MPI_Recv(&tamanio_lista_recibir,1,MPI_INT,my_rank+distance,tamanio_tag,MPI_COMM_WORLD,MPI_STATUS_IGNORE);

        int * lista_a_recibir = (int *)malloc(tamanio_lista_recibir*sizeof(int));
        if(lista_a_recibir==NULL){
          std::cout << "Error al asignar memoria a la lista" << endl;
          return 0; 
        }
        
        std::stringstream ssout;
        //tamanio_lista += tamanio_lista_recibir;
        //ssout << "RECV soy rank: "<<my_rank<<" pro_id: "<<pro_id<<" tamanio_lista "<<tamanio_lista<<" tamaio a racibir "<< tamanio_lista_recibir << " total nodos: "<<numero_nodos <<std::endl;
        //std::cout << ssout.str();
        //Recibir la lista
        MPI_Recv(lista_a_recibir, tamanio_lista_recibir, MPI_INT, my_rank+distance, lista_recibir_tag,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
        //<< " pro_id: "<< pro_id 
        ssout<<"RECV: pro: "<<my_rank<< " from: "<<my_rank+distance<<", tamaño de lista "<<tamanio_lista<<" ";
        ssout<<" lista_unida ";
        for(int i = 0; i < tamanio_lista; i++){
          ssout<<" "<<lista_unida[i];
        }
        ssout << " tamanio lista a recibir " << tamanio_lista_recibir << " lista_a_recibir: ";
        for(int i = 0; i < tamanio_lista_recibir; i++){
          ssout <<" "<<lista_a_recibir[i];
        }
        ssout<<std::endl;
        

        //MERGE IT!!!
        int * lista_local_unida = merge(lista_unida, lista_a_recibir,tamanio_lista,tamanio_lista_recibir);
        free(lista_unida);
        free(lista_a_recibir);
        lista_unida = lista_local_unida;
        //free(lista_local_unida);
        tamanio_lista += tamanio_lista_recibir;

        //std::stringstream ssxx;
       /* ssout << "***merged list: ";
        
        for(int x = 0; x < tamanio_lista; x++){
          ssout << lista_unida[x] << " ";
        }*/
        std::cout << ssout.str();
        

      }

      //Para todos los impares. 
      pro_id = (pro_id +1 )/ 2;
      numero_nodos = numero_nodos % 2 == 0 ? numero_nodos/2 :  (numero_nodos+1 ) / 2;
      distance <<= 1;
      working = 1 < numero_nodos;

      
    }
   

      
    
  }

  std::cout << "I AM OUT!" <<std::endl;
  
  if(my_rank == 0 || my_rank == r){
	   free(lista);
  }

  if(my_rank == 0){
    /*std::stringstream sstm;
    sstm << ">>>";
    
    for(int x = 0; x < tamanio_lista; x++){
      sstm <<lista_unida[x] << " , ";
    }
    //result = sstm.str();
    std::cout << sstm.str();*/
    std::cout << "\nEl valor de p es: " << p << "\nEl valor de n es: " << n << "\nEl valor de m es: " << m;
    char mostrar;
    std::cout << "\nDesea mostrar la lista ordenada (Y/N): ";
    std::cin >> mostrar;
    if(mostrar == 'Y') 
	    mostrarLista(lista_unida,tamanio_lista);
    std::cout << "\nNUMERO DE VECES QUE APARECE CADA NUMERO";
    std::cout << "\n"; 
    cantidadValores(lista_unida,m,tamanio_lista);
    
  }

 
  
    free(lista_unida);


  MPI_Finalize();
   return 0;
}

void Genera_vector(int lista[], int n,  int m)
{
      int i;
      for (i = 0; i < n; i++) {
        lista[i]= 0 + rand()%(m+1-0);              
      }
}

/*Metodo auxiliar del mergeSort que ordena dos listas en una sola
Recibe dos listas y el tamaño de cada una de ellas
*/
int* merge(int mitad1[], int mitad2[], int nMitad1, int nMitad2) {
    int *result = (int *) malloc((nMitad1+nMitad2)*sizeof(int)); //Vector donde se guarda la lista ordenada de las dos listas recibidad
    int apuntador1 = 0; //Posicion inicial del apuntador al primer vector
    int apuntador2 = 0; //Posicion inicial del apuntador al segundo vector
    
    int numResult = 0; //Entero para saber por cual posicion de vector de resultado se van insertando valores

    //Ciclo que recorre ambas listas, los apuntadores deben ser menores que el tamaño total de cada lista
    while (apuntador1 < nMitad1 && apuntador2 < nMitad2) {
	//Si los valores de ambas listas son iguales se ingresan los dos valores al resultado
        if (mitad1[apuntador1] == mitad2[apuntador2]) {
            result[numResult] = mitad1[apuntador1]; //Ingresamos al resultado el valor de la primera lista
            numResult++;//Se incrementa la posicion para que el siguiente valor se ingrese en la siguiente posicion del vector de resultado
            result[numResult] = mitad2[apuntador2]; //Ingresamos al resultado el valor de la segunda lista
            numResult++;//Se incrementa la posicion para que el siguiente valor se ingrese en la siguiente posicion del vector de resultado
            apuntador1++;//Se incrementa el apuntador del vector de la primera lista
            apuntador2++;//Se incrementa el apuntador del vector de la segunda lista
	//Si el valor de la primera lista es mayor al de la segunda, se ingresa el de la segunda lista porque es menor
        } else if (mitad1[apuntador1] > mitad2[apuntador2]) {
            result[numResult] = mitad2[apuntador2];//Ingresamos el valor de la segunda lista en la lista de resultados
            apuntador2++;//Se incrementa el apuntador del vector de la segunda lista
	    numResult++;//Se incrementa la posicion para que el siguiente valor se ingrese en la siguiente posicion del vector de resultado
        //Si el valor de la segunda lista es maor al de la primera, se ingresa el de la primera lista porque es menor
	} else {
            result[numResult] = mitad1[apuntador1];//Ingresamos el valor de la primera lista en la lista de resultados
            apuntador1++;//Se incrementa el apuntador del vector de la primera lista
            numResult++;//Se incrementa la posicion para que el siguiente valor se ingrese en la siguiente posicion del vector de resultado
        }
    }
    //Si se sale del ciclo anterior y no se han recorrido todas las posiciones de las listas, se debe de ingresar al resultado los faltantes
    if (apuntador1 < nMitad1 || apuntador2 != nMitad2) {
	//Si el apuntador1 es igual al tamaño de la primera lista, significa que todavia faltan valores de la segunda lista
        if (apuntador1 == nMitad1) {
	    //Ciclo que recorre la segunda lista hasta que se llega al final
            while (apuntador2 < nMitad2) {
                result[numResult] = mitad2[apuntador2];//Ingresamos el valor de la segunda lista en la lista de resultados
                numResult++;//Se incrementa la posicion para que el siguiente valor se ingrese en la siguiente posicion del vector de resultado
                apuntador2++;//Se incrementa el apuntador del vector de la segunda lista
            }
	//Si el apuntador2 es igual al tamaño de la segunda lista, significa que todavia faltan valores de la primera lista
        } else {
            //Ciclo que recorre la primera lista hasta que se llega al final
            while (apuntador1 != nMitad1) {
                result[numResult] = mitad1[apuntador1];//Ingresamos el valor de la primera lista en la lista de resultados
                numResult++;//Se incrementa la posicion para que el siguiente valor se ingrese en la siguiente posicion del vector de resultado
                apuntador1++;//Se incrementa el apuntador del vector de la primera lista
            }
        }
    }
    return result;
}

/*Metodo principal del mergeSort. 
Recibe la lista a ordenar y el número de elementos de la lista
*/
int* mergeSort(int lista[], int numElem) 
{
	int mitad=0; //Variable que se usa para calcular la mitad de la lista
	int modulo=0; //Variable que se usa para calcular el módulo
	int m2=0; //Variable que se usa si la mitad no es exacta, sería mitad+1
	int recorridoL1 = 0; //Variable para saber por cual posición del vector mitad1 se van ingresando los valores 
	int recorridoL2 = 0; //Variable para saber por cual posición del vector mitad2 se van ingresando los valores de la lista que dividimos

	//Si la lista ordenada es de tamaño 1, nada mas retorna la lista porque ya se encuentra ordenada
	if(numElem==1)
		return lista;	
	//Si no es de tamaño 1 debe de dividir la lista en dos partes
	else {
		mitad = numElem/2; //Se calcula la mitad de la lista
		modulo = numElem%2; //Se calcula el módulo de la lista para ver si es divisible por 2

		//Si el modulo es 0 significa que la lista se puede dividir en dos partes
		if(modulo==0)
			m2=  mitad;

		//Si el modulo no es 0 significa que una de las dos mitades debe tener una posición más que la otra
		else 
			m2=  mitad+1;
			
		//Se crean los vectores que simbolizan las dos mitades de la lista para hacer el mergeSort
		int mitad1[mitad];
		int mitad2[m2];
		
		//Ciclo que reparte los valores de la lista entre las dos mitades
		for(int j=0; j<numElem; j++) {
			//Si la posicion actual de la lista es menor que la mitad significa que el valor se ubica en la primera mitad
			if(j<mitad) { 
				mitad1[recorridoL1] = lista[j];
				recorridoL1++;
			}
			
			//Si la posicion actual de la lista es mayor o igual que la mitad significa que el valor se ubica en la segunda mitad
			else {
				mitad2[recorridoL2] = lista[j];
				recorridoL2++;
			}
		}

		//Si el modulo es igual 0 entonces se llama el merge, y se envia que las dos mitades poseen el mismo tamaño
		if(modulo==0)
			return merge(mergeSort(mitad1,mitad),mergeSort(mitad2,mitad),mitad,mitad);

		//Si el modulo es diferente que 0 entonces se llama el merge pero se envia que el tamaño de las dos mitades es diferente
		else
			return merge(mergeSort(mitad1,mitad),mergeSort(mitad2,mitad+1),mitad,mitad+1);
	}
}

/*Metodo que se encarga de mostrar cuantas veces aparece cada numero en la lista
Recibe la lista a la que se desea contar los numeros, el numero maximo de valor que ingreso el usuario y el tamano de la lista
*/
void cantidadValores(int lista[], int m, int tamanoLista) 
{
  int cantidad = 0; //Variable que se usa para contar las veces que aparece un numero
  //Ciclo que recorre todos los numeros desde 0 hasta el maximo valor
  for(int i=0; i<=m; i++) {
	//Ciclo que recorre la lista ordenada
  	for(int j=0; j<tamanoLista; j++) {
		//Si el valor en la posicion de la lista es igual al numero actual se aumenta el contador
    		if(lista[j]==i)
    			cantidad++;
	}
	//Si la cantidad es cero no se imprime
  	if(cantidad != 0)
  		std::cout<<"El numero " << i << " aparece " << cantidad << " veces \n";
  	cantidad = 0; //Se reinica el contador para el siguiente numero
  }
}

/*Metodo que muestra la lista ordenada
Recibe la lista que se desea mostrar y el tamaño de la lista
*/
void mostrarLista(int lista[],int tamanoLista) 
{
    std::cout<<"LISTA ORDENADA \n";
    //Ciclo que recorre la lista que se desea mostrar
    for(int i = 0; i < tamanoLista; i++) {
	//Si no es la ultima posicion de la lista, se muestra el numero y se separa con una ,
    	if(i != tamanoLista-1)
		std::cout<<lista[i]<<" , ";
	//Si es la ultima solo muestra el valor de la lista
	else
		std::cout<<lista[i]<<" \n";
    }
}
