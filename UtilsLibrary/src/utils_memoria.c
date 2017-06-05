/*
 * utils_memoria.c
 *
 *  Created on: 1/5/2017
 *      Author: utnso
 */

#include "utils_memoria.h"

t_memoria * new_memoria(int marcos, int marcosSize){

	t_memoria * memoria = malloc(sizeof(t_memoria));
	if (memoria == NULL) {
		return NULL;
	}

	memoria->memoria_principal = (void *) calloc (marcos, marcosSize); //ponele
	if (memoria->memoria_principal == NULL) {
		free(memoria);
		return NULL;
	}

	memoria->paginas = malloc(sizeof(t_pagina) * marcos);
	if (memoria->paginas == NULL) {
		free(memoria->memoria_principal);
		free(memoria);
		return NULL;
	}
	int i;
	for(i = 0; i < marcos; i++){
		memoria->paginas[i].nro_marco = i;
		memoria->paginas[i].pid = -1; //no está asignada a ningún proceso
		memoria->paginas[i].nro_pagina = i; //le asignamos un nro como para poner algo
	}

	memoria->programas = list_create();
	if(memoria->programas == NULL){
		free(memoria->paginas);
		free(memoria->memoria_principal);
		free(memoria);
		return NULL;
	}

	memoria->cantidad_marcos = marcos;
	memoria->tamanio_marco = marcosSize;
	memoria->cantidad_paginas_disponibles = marcos;


	return memoria;
}

//PREGUNTAR EN LA CACHÉ
void * solicitar_bytes_de_una_pagina(t_memoria * memoria, int nro_pagina, int offset, int tamanio){
	void * bytes = malloc(tamanio);
	memcpy(bytes, devolver_pagina(memoria, nro_pagina) + offset, tamanio);
	return bytes;
}

//ACTUALIZAR EN CACHÉ TMB
void almacenar_bytes_en_una_pagina(t_memoria * memoria, int nro_pagina, int offset, int tamanio, void * buffer){
	if(tamanio > memoria->tamanio_marco || (offset + tamanio) > memoria->tamanio_marco){ //fijarse
		printf("tamanio mayor al esperado");
		return;
	} else {
		memcpy(devolver_pagina(memoria, nro_pagina) + offset, buffer, tamanio);
	}
}

//Dado un idPagina, busca la pagina en la memoria y la devuelve
void * devolver_pagina(t_memoria * memoria, int nro_pagina){
	int i = 0;
	while(memoria->paginas[i].nro_pagina != nro_pagina){
		i++;
	}
	int nro_marco = memoria->paginas[i].nro_marco;
	int tamanio_marco = memoria->tamanio_marco;
	void * pagina = memoria->memoria_principal + nro_marco * tamanio_marco; //chequear
	return pagina;
}

//Probar
//cuando un proceso te pide mas paginas ademas de las que ya se le asignó al principio
void * asignar_paginas_a_proceso(t_memoria * memoria, pid_t pid, int paginas_requeridas){
	if(paginas_requeridas <= memoria->cantidad_paginas_disponibles){
		int i = 0;
		t_entrada_programa * entrada_programa = list_find(memoria->programas, (void *) pertenece_al_programa(memoria->programas->head, pid));
		int j = entrada_programa->cantidad_paginas;
		//entrada_programa->paginas_asignadas = realloc(sizeof(int) *(entrada_programa->cantidad_paginas + paginas_requeridas)); //seria un realloc se le suma las pag requeridas a las q ya habia
		 if (entrada_programa->paginas_asignadas == NULL) {
			printf("paginas asignadas null");
			return NULL;
		}
		while (paginas_requeridas > 0) {
			if (memoria->paginas[i].pid == -1) {
				asignar_pagina(memoria, pid, i, j);
				paginas_requeridas--;
				entrada_programa->paginas_asignadas[j] = memoria->paginas[i].nro_marco;
				j++;
			}
			i++;
		}
	} else{
		return NULL;
	}
}

void * inicializar_programa(t_memoria * memoria, int pid, int paginas_requeridas){ //, void * codigoPrograma
	if(paginas_requeridas <= memoria->cantidad_paginas_disponibles){

		int i = 0;
		int j = 0;
		int * paginas_asignadas = malloc(sizeof(int) * paginas_requeridas);
		if(paginas_asignadas == NULL){
			printf("paginas asignadas null");
			return NULL;
		}
		while(paginas_requeridas > 0){
			if(memoria->paginas[i].pid > -1){
				asignar_pagina(memoria, pid, i, j);
				paginas_requeridas --;
				paginas_asignadas[j] = memoria->paginas[i].nro_marco;
				j++;
			}
			i++;
		}
		t_entrada_programa * nuevo_programa = malloc(sizeof(t_entrada_programa));
		if(nuevo_programa == NULL){
			printf("nuevo programa null");
			free(paginas_asignadas);
			return NULL;
		}
		nuevo_programa->pid = pid;
		nuevo_programa->cantidad_paginas = paginas_requeridas;
		nuevo_programa->paginas_asignadas = paginas_asignadas;
		list_add(memoria->programas, nuevo_programa);
		return nuevo_programa;
	}
	else{
		return NULL;
	}
}

void asignar_pagina(t_memoria * memoria, int pid, int posicion_tabla, int nro_pagina){//nro_pagina es dentro del programa
	memoria->paginas[posicion_tabla].pid = pid;
	memoria->paginas[posicion_tabla].nro_pagina = nro_pagina;
	memoria->cantidad_paginas_disponibles--;
}

void liberar_pagina(t_memoria * memoria, int posicion_tabla){
	memoria->paginas[posicion_tabla].pid = -1;
	memoria->cantidad_paginas_disponibles++;
}

void liberar_pagina_cacheada(t_entrada_cache * cache, int posicion_tabla){
	cache->pid = -1;
	cache->nro_pagina = -1;
	cache->contenido_pagina = NULL;
}

void finalizar_programa(t_memoria * memoria, t_entrada_cache * cache, int pid, int entradas_cache){
	int i;
	for(i = 0; i < memoria->cantidad_marcos; i++){
		if(memoria->paginas[i].pid == pid){
			liberar_pagina(memoria, i);
		}
	}
	for(i = 0; i < entradas_cache; i++){
		if(cache[i].pid == pid){
			liberar_pagina_cacheada(cache, i);
		}
	}
	memoria->programas = list_filter(memoria->programas, (void *) !pertenece_al_programa(memoria->programas->head, pid));
}

t_entrada_cache * new_cache(int entradas_cache, int marco_size){
	t_entrada_cache * cache = malloc((2 * sizeof(int) + marco_size) * entradas_cache); //check
	if(cache == NULL){
		return NULL;
	}

	int i;
	for(i = 0; i < entradas_cache; i++){
		cache[i].pid = -1;
		cache[i].nro_pagina = -1;
		cache[i].contenido_pagina = NULL;
	}

	return cache;
}

//falta terminar
void reemplazo_pagina(t_pagina pagina, t_entrada_cache * cache, t_memoria * memoria){
//	int entrada = 0;
//	while(cache[entrada]->pid != -1){
//		entrada++;
//	}
//	if(cache[entrada]->pid == -1){
//		cache[entrada]->pid = pagina->pid;
//		cache[entrada]->nro_pagina = pagina->nro_pagina;
//		cache[entrada]->contenido_pagina = devolver_pagina(memoria, pagina->nro_pagina);
//	}
}

bool pertenece_al_programa(t_entrada_programa * entrada_programa, int pid){
	return entrada_programa->pid == pid;
}

t_entrada_cache * ordenar_cache(int posicion_reciente_uso, t_entrada_cache * cache, int marco_size, int entradas_cache){
	t_entrada_cache * cache_ordenada = malloc((2 * sizeof(int) + marco_size) * entradas_cache); //check
	if(cache_ordenada == NULL){
		return NULL;
	}
	cache_ordenada[0] = cache[posicion_reciente_uso];
	int i;
	for(i=0; i!=posicion_reciente_uso && i<entradas_cache; i++){

	}
	free(cache);
	return cache_ordenada;
}

void test(){
	t_memoria * memoria = new_memoria(1, 6);
	char * buffer = malloc(5);
	buffer = "hola";
	almacenar_bytes_en_una_pagina(memoria, 0, 0, sizeof(buffer), buffer);
	char * bufferSalida = solicitar_bytes_de_una_pagina(memoria, 0, 3, 5);
	puts(bufferSalida);
}

void test_cache(){
	t_entrada_cache * cache = new_cache(2, 5);
	cache[0].pid = 1;
	cache[0].nro_pagina = 0;
	cache[0].contenido_pagina = "hola";
	int i;
	for(i = 0; i < 2; i++){
		printf("PID: %d\n", cache[i].pid);
		printf("NRO PAGINA: %d\n", cache[i].nro_pagina);
		printf("CONTENIDO PAGINA: %s\n", cache[i].contenido_pagina);
	}
}
