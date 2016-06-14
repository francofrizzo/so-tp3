#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "eleccion.h"
#include "control.h"
#include "mpi.h"

#define	MAX_PROCESOS	100
static int vivos[MAX_PROCESOS];

#define	CMD_FIN		"fin"
#define	CMD_LANZAR	"lanzar"
#define	CMD_MATAR	"matar"
#define	CMD_ETAPA2	"etapa2"

static t_pid ultimo_vivo(int vivos[MAX_PROCESOS], unsigned int np)
{
 t_pid i;
 t_pid ultimo_vivo;

 for (i= 1; i<=np; i++)
	{
	 if (vivos[i])
		ultimo_vivo= i;
	}

 return ultimo_vivo;
}

static int procesar_comandos(unsigned int np)
{
 int fin= 0;
 char buffer[128];
 int cte_0= 0, cte_1= 1;
 t_pid pid_0= 0;
 char *res, *primer_param, *segundo_param;
 size_t long_buffer;

 res= fgets(buffer, sizeof(buffer), stdin);

 /* Permitimos salir con EOF. */
 if (res==NULL)
	return 1;

 long_buffer= strlen(buffer); 
 /* Si es un ENTER, continuamos. */
 if (long_buffer<=1)
	return 0;

 /* Sacamos último caracter. */
 buffer[long_buffer-1]= '\0';

 primer_param= strtok(buffer, " ");
 segundo_param= strtok(NULL, " ");

 if (strncmp(primer_param, CMD_FIN, sizeof(buffer))==0)
	fin= 1;
 else if (strncmp(primer_param, CMD_LANZAR, sizeof(CMD_LANZAR))==0)
	{
	 t_pid quien= atoi(segundo_param);
	 if (quien>np)
		{
		 printf("Hay sólo %u procesos.\n", np);
		 return 0;
		}
	 else if (vivos[quien])
		{
		 printf("El proceso %u ya está vivo.\n", quien);
		 return 0;
		}
	 printf("Lanzando proceso %u.\n", quien);
	 vivos[quien]= 1;
	 MPI_Send(&cte_1, 1, MPI_INT, quien, TAG_CONTROL, MPI_COMM_WORLD);
	}
 else if (strncmp(primer_param, CMD_MATAR, sizeof(CMD_MATAR))==0)
	{
	 t_pid quien= atoi(segundo_param);
	 if (quien>np)
		{
		 printf("Hay sólo %u procesos.\n", np);
		 return 0;
		}
	 else if (!vivos[quien])
		{
		 printf("El proceso %u no está vivo.\n", quien);
		 return 0;
		}
	 printf("Matando proceso %u.\n", quien);
	 vivos[quien]= 0; /* Ya no está vivo. */
	 /* Le digo que no es el último y luego que termine. */
	 MPI_Send(&pid_0, 1, MPI_PID, quien, TAG_CONTROL, MPI_COMM_WORLD);
	 MPI_Send(&cte_1, 1, MPI_INT, quien, TAG_CONTROL, MPI_COMM_WORLD);
	}
 else if (strncmp(primer_param, CMD_ETAPA2, sizeof(CMD_ETAPA2))==0)
	{
	 t_pid i;
	 t_pid ultimo= ultimo_vivo(vivos, np);
	 for (i= 1; i<=np; i++)
		{
		 if (!vivos[i])
			continue;
		 MPI_Send(&ultimo, 1, MPI_PID, i, TAG_CONTROL,
			MPI_COMM_WORLD);
		 /* No tiene que terminar. */
		 MPI_Send(&cte_0, 1, MPI_INT, i, TAG_CONTROL,
			MPI_COMM_WORLD);
		}
	}

 return fin;
}

void control(unsigned int np)
{
 int fin= 0;
 t_pid i;

 if (np>MAX_PROCESOS)
	{
	 fprintf(stderr, "Incremente MAX_PROCESOS y recompile.\n");
	 exit(1);
	}

 /* Al principio ninguno está vivo. */
 for (i= 1; i<=np; i++)
	vivos[i]= 0;

 printf("Comandos:\n"CMD_LANZAR" N\n"CMD_MATAR" N\n"CMD_ETAPA2
	" - comienza la etapa 2\n"CMD_FIN" - salir (de la consola "
	"solamente).\n");

 while (!fin)
	{
	 printf("> ");
	 fflush(stdout);
	 fin= procesar_comandos(np);
	}
 exit(0);
}
