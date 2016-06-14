#ifndef	_ELECCION_H
#define	_ELECCION_H

/* Enumerado para ver si soy o no líder. */
enum status { NO_LIDER, LIDER };
typedef enum status t_status;

/* PID de elección de líder. */
typedef unsigned short int t_pid;
#define	MPI_PID	MPI_UNSIGNED_SHORT

/* Función que maneja la elección de líder por no más
 * de timeout segundos.
 * pid es el PID del proceso local.
 * es_ultimo indica si éste es el último proceso del anillo o no.
 */
void eleccion_lider(t_pid pid, int es_ultimo, unsigned int timeout);

/* Función que se encarga de iniciar la elección de líder.
 * Los parámetros tienen la misma semántica que eleccion_lider().
 */
void iniciar_eleccion(t_pid pid, int es_ultimo);

#endif	/* ELECCION_H */
 