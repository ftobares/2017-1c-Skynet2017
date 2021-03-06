/*
 * utils_protocolo.h
 *
 *  Created on: 24/4/2017
 *      Author: utnso
 */

#ifndef SRC_UTILS_PROTOCOLO_H_
#define SRC_UTILS_PROTOCOLO_H_

#include <stdio.h>
#include <stdint.h>

//Estructuras para cada mensaje
#define MSJ_HEADER 1
#define MSJ_HANDSHAKE 2
#define MSJ_PROGRAMA_ANSISOP 3
#define MSJ_PCB 4
#define MSJ_FINALIZAR_PROGRAMA 5

/* Estructuras genericas, modificar y agregar cuando definamos
 * el protocolo */

typedef struct{
	char* handshake;
} t_handshake;

typedef struct{
	uint32_t pid;
	char* contenido;
} t_programa_ansisop;

uint32_t calcular_tamanio_mensaje(void* p_mensaje, uint32_t tipo_mensaje);

#endif /* SRC_UTILS_PROTOCOLO_H_ */
