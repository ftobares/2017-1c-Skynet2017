/*
 * utils_memoria.h
 *
 *  Created on: 1/5/2017
 *      Author: utnso
 */

#ifndef SRC_UTILS_MEMORIA_H_
#define SRC_UTILS_MEMORIA_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <commons/collections/list.h>
#include <sys/types.h>

/* PCB */
typedef struct{
	pid_t pid;
	int program_counter;
	int cantidad_paginas;
	//struct indice_de_codigo;
	//struct indice_de_stack;
	int posicion_stack;
	int exit_code;
} t_pcb; /* Definicion MUY vaga, refactorizar*/

typedef struct{
	int nro_marco;
	pid_t pid; //proceso al que pertenece la pagina
	int nro_pagina; //orden de la pagina del proceso
} t_pagina;

typedef struct{
	pid_t pid;
	int * paginas_asignadas;
	int cantidad_paginas;
} t_entrada_programa;

typedef struct{
	void * memoria_principal;
	t_pagina * paginas;
	int tamanio_marco;
	int cantidad_marcos; //es igual a la cantidad de paginas (1 pagina 1 marco)
	int cantidad_paginas_disponibles;
	t_list * programas;
} t_memoria;

typedef struct{
	pid_t pid;
	int nro_pagina;
	void * contenido_pagina; //el tipo está bien?
} t_entrada_cache;

t_memoria * new_memoria(int marcos, int marcosSize);

void * solicitar_bytes_de_una_pagina(t_memoria * memoria, int nro_pagina, int offset, int tamanio);

void almacenar_bytes_en_una_pagina(t_memoria * memoria, int nro_pagina, int offset, int tamanio, void * buffer);

void * devolver_pagina(t_memoria * memoria, int nro_pagina);

void * inicializar_programa(t_memoria * memoria, pid_t pid, int paginas_requeridas);

void asignar_pagina(t_memoria * memoria, pid_t pid, int posicion_tabla, int nro_pagina);

void liberar_pagina(t_memoria * memoria, int posicion_tabla);

void finalizar_programa(t_memoria * memoria, pid_t pid);

t_entrada_cache * new_cache(int entradas_cache, int marco_size);

void reemplazo_pagina(t_pagina pagina, t_entrada_cache * cache, t_memoria * memoria);

#endif /* SRC_UTILS_MEMORIA_H_ */
