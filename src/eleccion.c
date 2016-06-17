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

	t_pid token[4];
	token[0] = pid;
	token[1] = pid;
	token[2] = pid;
	token[3] = 0;
	
	t_pid siguiente = siguiente_pid(pid, es_ultimo);

	MPI_Isend(&token, 4, MPI_PID, siguiente, TAG_ELECCION_TOKEN,
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
	printf("[%hd, %hd] (%u -> %u) i:%hd  c:%hd\n", token[0], token[1], pid, siguiente, token[2], token[3]);
}

void eleccion_lider(t_pid pid, int es_ultimo, unsigned int timeout){
	static t_status status = NO_LIDER;
	double ahora = MPI_Wtime();
	double tiempo_maximo = ahora + timeout;
	double ahora_ack = 0;
	double tiempo_maximo_ack = 0;
	int ack_flag = 0;

	t_pid siguiente = siguiente_pid(pid, es_ultimo);
	t_pid ack_pid;

	int token_flag = 0;
	MPI_Status token_status;
	MPI_Status ack_status;

	MPI_Request req;
	t_pid token[4];

	// Repito hasta qye haya un l�der
	while (ahora < tiempo_maximo){
		// Reviso si lleg� un mensaje nuevo
		MPI_Iprobe(MPI_ANY_SOURCE, TAG_ELECCION_TOKEN, MPI_COMM_WORLD,
		           &token_flag, &token_status);

		// Si lleg� un mensaje nuevo lo cargo en token
		if (token_flag) {
			// Leo el mensaje recibido
			MPI_Irecv(&token, 4, MPI_PID, MPI_ANY_SOURCE, TAG_ELECCION_TOKEN,
			          MPI_COMM_WORLD, &req);

			// Tomo el pid del emisor y le env�o ACK
			ack_pid = token_status.MPI_SOURCE;
			MPI_Isend(&token_flag, 1, MPI_PID, ack_pid, TAG_ELECCION_ACK,
			          MPI_COMM_WORLD, &req);

			// Si lleg� un mensaje en el cual soy el inciador
			if (token[0] == pid) {
				// Si yo sigo siendo el candidato
				if (token[1] == pid) {
					// Soy l�der
					status = LIDER;
					break;
				}
				else {
					// Reemplazo al inciciador por el candidato actual
					token[0] = token[1];
				}
			}
			// Si no soy el iniciador pero tengo un pid mayor al del candidato
			else if(token[0] != pid && token[1] < pid) {
				// Me denomino como nuevo candidato
				token[1] = pid;
			}
			token[3]++;

			// Env�o el nuevo token al pr�ximo
			MPI_Isend(&token, 4, MPI_PID, siguiente, TAG_ELECCION_TOKEN,
			          MPI_COMM_WORLD, &req);

			ahora_ack = MPI_Wtime();
			tiempo_maximo_ack = ahora_ack + ACK_TIMEOUT;

			// Espero el ACK
			ack_flag = 0;
			while (! ack_flag && ahora_ack < tiempo_maximo_ack) {
				MPI_Iprobe(siguiente, TAG_ELECCION_ACK, MPI_COMM_WORLD,
				           &ack_flag, &ack_status);
				ahora_ack = MPI_Wtime();
			}
			printf("[%hd, %hd] (%u -> %u) i:%hd  c:%hd\n", token[0], token[1], pid, siguiente, token[2], token[3]);
		}

		/* Actualizo valor de la hora. */
		ahora = MPI_Wtime();
	}

	/* Reporto mi status al final de la ronda. */
	printf("Proceso %u %s l�der.\n", pid, (status==LIDER ? "es" : "no es"));
}
