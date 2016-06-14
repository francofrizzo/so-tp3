#include <stdio.h>
#include "mpi.h"
#include "eleccion.h"

static t_pid siguiente_pid(t_pid pid, int es_ultimo)
{
 t_pid res= 0; /* Para silenciar el warning del compilador. */

 if (es_ultimo)
	res= 1;
 else
	res= pid+1;

 return res;
}

void iniciar_eleccion(t_pid pid, int es_ultimo)
{
 /* Completar acá el algoritmo de inicio de la elección.
  * Si no está bien documentado, no aprueba.
  */
}

void eleccion_lider(t_pid pid, int es_ultimo, unsigned int timeout)
{
 static t_status status= NO_LIDER;
 double ahora= MPI_Wtime();
 double tiempo_maximo= ahora+timeout;
 t_pid proximo= siguiente_pid(pid, es_ultimo);

 while (ahora<tiempo_maximo)
	{
	 /* Completar acá el algoritmo de elección de líder.
	  * Si no está bien documentado, no aprueba.
          */

	 /* Actualizo valor de la hora. */
	 ahora= MPI_Wtime();
	}

 /* Reporto mi status al final de la ronda. */
 printf("Proceso %u %s líder.\n", pid, (status==LIDER ? "es" : "no es"));
}
