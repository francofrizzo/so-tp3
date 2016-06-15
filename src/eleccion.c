#include <stdio.h>
#include "mpi.h"
#include "eleccion.h"

static t_pid siguiente_pid(t_pid pid, int es_ultimo){
	t_pid res= 0; /* Para silenciar el warning del compilador. */

	if (es_ultimo)
		res= 1;
	else
		res= pid+1;

	return res;
}

void iniciar_eleccion(t_pid pid, int es_ultimo){
	MPI_Request req;

	t_pid token[2];
	token[0] = pid;
	token[1] = pid;
	
	t_pid siguiente = siguiente_pid(pid, es_ultimo);

	MPI_Isend(&token, 2, MPI_PID, siguiente, TAG_ELECCION_TOKEN,
		MPI_COMM_WORLD, &req);

	double ahora = MPI_Wtime();
	double tiempo_maximo = ahora + ACK_TIMEOUT;

	int ack_flag = 0;
	MPI_Status ack_status;
	while (! ack_flag && ahora < tiempo_maximo) {
		MPI_Iprobe(siguiente, TAG_ELECCION_ACK, MPI_COMM_WORLD,
			&ack_flag, &ack_status);
		ahora = MPI_Wtime();
	}
	printf("pasoeltiempo");
	// acá estoy seguro de que recibí el ACK
}

void eleccion_lider(t_pid pid, int es_ultimo, unsigned int timeout){
	static t_status status = NO_LIDER;
	double ahora = MPI_Wtime();
	double tiempo_maximo = ahora + timeout;
	t_pid proximo = siguiente_pid(pid, es_ultimo);

	int token_flag = 0;
	MPI_Status token_status;

	MPI_Request req;
	t_pid token[2];

	while (ahora < tiempo_maximo){
		MPI_Iprobe(MPI_ANY_SOURCE, TAG_ELECCION_TOKEN, MPI_COMM_WORLD,
			&token_flag, &token_status);

		if (token_flag) {
			MPI_Irecv(&token, 2, MPI_PID, MPI_ANY_SOURCE, TAG_ELECCION_TOKEN,
				MPI_COMM_WORLD, &req);

			printf("%hd %hd\n", token[0], token[1]);
		}

		/* Actualizo valor de la hora. */
		ahora = MPI_Wtime();
	}

	/* Reporto mi status al final de la ronda. */
	printf("Proceso %u %s líder.\n", pid, (status==LIDER ? "es" : "no es"));
}
