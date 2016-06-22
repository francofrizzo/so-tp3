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
	int ack_flag = 0;

	// Mientras no haya recibido un ACK
	while (! ack_flag) {
		// Intento enviar a mi siguiente
		MPI_Isend(&token, 4, MPI_PID, siguiente, TAG_ELECCION_TOKEN,
		          MPI_COMM_WORLD, &req);

		// Si no soy yo mismo, espero al ACK
		if (siguiente != pid) {
			double ahora = MPI_Wtime();
			double tiempo_maximo = ahora + ACK_TIMEOUT;

			MPI_Status ack_status;
			while (! ack_flag && ahora < tiempo_maximo) {
				MPI_Iprobe(siguiente, TAG_ELECCION_ACK, MPI_COMM_WORLD,
				           &ack_flag, &ack_status);
				ahora = MPI_Wtime();
			}
		}
		// Si soy yo mismo no espero un ACK
		else {
			ack_flag = 1;
		}

		printf("[%hd, %hd] (%u -> %u) i:%hd  c:%hd\n", token[0], token[1], pid, siguiente, token[2], token[3]);

		// Aumento el valor de siguiente para enviarle al próximo
		siguiente++;
	}
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

	// Repito hasta que haya un líder
	while (ahora < tiempo_maximo){
		// Reviso si llegó un mensaje nuevo
		MPI_Iprobe(MPI_ANY_SOURCE, TAG_ELECCION_TOKEN, MPI_COMM_WORLD,
		           &token_flag, &token_status);

		// Si llegó un mensaje nuevo lo cargo en token
		if (token_flag) {
			// Leo el mensaje recibido
			MPI_Irecv(&token, 4, MPI_PID, MPI_ANY_SOURCE, TAG_ELECCION_TOKEN,
			          MPI_COMM_WORLD, &req);

			// Tomo el pid del emisor y le envío ACK
			ack_pid = token_status.MPI_SOURCE;
			MPI_Isend(&token_flag, 1, MPI_PID, ack_pid, TAG_ELECCION_ACK,
			          MPI_COMM_WORLD, &req);

			// Si ya soy lider, no propago el mensaje
			if (status != LIDER) {
				// Si llegó un mensaje en el cual soy el iniciador
				if (token[0] == pid) {
					// Si yo sigo siendo el candidato
					if (token[1] == pid) {
						// Soy líder
						status = LIDER;
					}
					else {
						// Reemplazo al iniciador por el candidato actual
						token[0] = token[1];
					}
				}
				// Si no soy el iniciador pero tengo un pid mayor al del candidato
				else if(token[0] != pid && token[1] < pid) {
					// Me denomino como nuevo candidato
					token[1] = pid;
				}
				token[3]++;

				// Mientras no haya recibido un ACK
				while (! ack_flag) {
					// Envío el nuevo token al siguiente
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

					// Si no recib. ACK, intento con el próximo
					if (! ack_flag) {
						siguiente++;
					}
				}
			}
		}

		/* Actualizo valor de la hora. */
		ahora = MPI_Wtime();
	}

	/* Reporto mi status al final de la ronda. */
	printf("Proceso %u %s líder.\n", pid, (status==LIDER ? "es" : "no es"));
}
