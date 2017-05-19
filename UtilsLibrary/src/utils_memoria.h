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
	int pid; //proceso al que pertenece la pagina
	int nro_pagina; //orden de la pagina del proceso
} t_pagina;

typedef struct{
	int pid;
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
	int pid;
	int nro_pagina;
	void * contenido_pagina; //el tipo está bien?
} t_entrada_cache;

t_memoria * new_memoria(int marcos, int marcosSize);

void * solicitar_bytes_de_una_pagina(t_memoria * memoria, int nro_pagina, int offset, int tamanio);

void almacenar_bytes_en_una_pagina(t_memoria * memoria, int nro_pagina, int offset, int tamanio, void * buffer);

void * devolver_pagina(t_memoria * memoria, int nro_pagina);

void * inicializar_programa(t_memoria * memoria, int pid, int paginas_requeridas);

void asignar_pagina(t_memoria * memoria, int pid, int posicion_tabla, int nro_pagina);

void liberar_pagina(t_memoria * memoria, int posicion_tabla);

void liberar_pagina_cacheada(t_entrada_cache * cache, int posicion_tabla);

void finalizar_programa(t_memoria * memoria, t_entrada_cache * cache, int pid, int entradas_cache);

t_entrada_cache * new_cache(int entradas_cache, int marco_size);

void reemplazo_pagina(t_pagina pagina, t_entrada_cache * cache, t_memoria * memoria);

bool pertenece_al_programa(t_entrada_programa * entrada_programa, int pid);

void test();

void test_cache();

//nuevas funciones
void consola();
int obtener_comando_consola(char * comando);
void consola_comando_help();
void consola_comando_retardo();
void consola_comando_dump_cache();
void consola_comando_dump_estructuras_memoria();
void consola_comando_dump_contenido_memoria();
void consola_comando_flush();
void consola_comando_size_memory();
void consola_comando_size_pid();
bool es_igual_a_pid(int pid, t_entrada_programa * entrada_programa);

#endif /* SRC_UTILS_MEMORIA_H_ */
