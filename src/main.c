#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mpi.h"
#include "eleccion.h"
#include "control.h"

#define	PERIODO_ELECCION		10
#define	DENOMINADOR_PROB_INICIO		2

/* Variables globales. */
int np, rank; /* Variables de MPI. */

/* Procesa el alta y baja de procesos.
 * Si devuelve 1 significa que debe terminar.
 */
static int ab_procesos(t_pid pid, int *es_ultimo)
{
 t_pid pid_ultimo;
 int terminar;
 MPI_Status status;

 MPI_Recv(&pid_ultimo, 1, MPI_PID, ID_PROC_CONTROL, TAG_CONTROL,
	MPI_COMM_WORLD, &status);

 *es_ultimo= (pid_ultimo==pid);

 MPI_Recv(&terminar, 1, MPI_INT, ID_PROC_CONTROL, TAG_CONTROL,
	MPI_COMM_WORLD, &status);

 if (*es_ultimo)
	printf("Proceso con PID %u es ahora el último.\n", pid);

 return terminar;
}

void elector(t_pid pid)
{
 int es_ultimo;
 int fin= 0;
 int empezar;
 MPI_Status status;

 /* Inicializo generador de números al azar. */
 srandom(time(NULL)+rank);

 /* Espero a recibir el comando para empezar. */
 MPI_Recv(&empezar, 1, MPI_INT, ID_PROC_CONTROL, TAG_CONTROL,
	MPI_COMM_WORLD, &status);

 printf("Comienza proceso con PID %u.\n", pid);

 while (!fin)
	{
	 /* Etapa 1: alta y baja de procesos. */
	 fin= ab_procesos(pid, &es_ultimo);

	 if (fin)
		{
		 printf("Proceso %u termina.\n", pid);
		 break;
		}

	 printf("Comienza la elección de líder en proceso %u.\n", pid);
	 /* Etapa 2: corro la elección de líder. */
	 if ((random() % DENOMINADOR_PROB_INICIO)==0)
		{
		 printf("Poniendo a circular una elección en proceso %u.\n",
			pid);
		 iniciar_eleccion(pid, es_ultimo);
		}

	 eleccion_lider(pid, es_ultimo, PERIODO_ELECCION);
	 printf("Finalizada la elección de líder en proceso %u.\n", pid);
	}
}

int main(int argc, char *argv[])
{
 int status;

 /* Inicializo MPI. */
 status= MPI_Init(&argc, &argv);
 if (status!=MPI_SUCCESS)
	{
	 fprintf(stderr, "Error de MPI al inicializar.\n");
	 MPI_Abort(MPI_COMM_WORLD, status);
	}
 MPI_Comm_size(MPI_COMM_WORLD, &np);
 MPI_Comm_rank(MPI_COMM_WORLD, &rank);
 /* Control el buffering: sin buffering. */
 setbuf(stdout, NULL);
 setbuf(stderr, NULL);
 printf("Lanzando proceso %u\n\n", rank);

 if (rank==0)
	{
	 /* Soy el proceso de control. */
	 control(np-1);
	}
 else
	{
	 /* Soy un elector más. */
	 elector(rank);
	}

 /* Limpio MPI. */
 MPI_Finalize();

 return 0;
}
