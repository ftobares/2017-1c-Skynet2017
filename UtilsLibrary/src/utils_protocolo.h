/*
 * utils_protocolo.h
 *
 *  Created on: 24/4/2017
 *      Author: utnso
 */

#ifndef SRC_UTILS_PROTOCOLO_H_
#define SRC_UTILS_PROTOCOLO_H_

//Estructuras para cada mensaje

/* Estructuras genericas, modificar y agregar cuando definamos
 * el protocolo */
typedef struct{
	int valor1;
	char* valor2;
} t_mensaje1;

typedef struct{
	int valor1;
	int valor2;
} t_mensaje2;

/**
 * Tipo mensaje = 3
 **/
typedef struct{
	char* handshake;
} t_handshake;

int calcular_tamanio_mensaje(void* p_mensaje, int tipo_mensaje);

#endif /* SRC_UTILS_PROTOCOLO_H_ */
