/*
 * utils_protocolo.c
 *
 *  Created on: 24/4/2017
 *      Author: utnso
 */

#include "utils_protocolo.h"
#include "utils_socket.h"

/**
 * @NAME: calcular_tamaño_mensaje
 * @DESC: Funcion para calcular el tamaño que tiene cada tipo de
 * @DESC: mensaje.*/
int calcular_tamanio_mensaje(void* p_mensaje, int tipo_mensaje) {

	switch(tipo_mensaje){
	case MSJ_HANDSHAKE:
		printf("Calcular tamaño Handshake\n");
		t_handshake* handshake = (struct t_handshake*) p_mensaje;
		int size_3 = sizeof(t_header)+strlen(handshake->handshake)+1;
		return size_3;
	case MSJ_PROGRAMA_ANSISOP:
		printf("Calcular tamaño Programa AnsiSOp\n");
		t_programa_ansisop* programa_ansisop = (struct t_programa_ansisop*) p_mensaje;
		int size_4 = sizeof(t_header)+sizeof(programa_ansisop->pid)+strlen(programa_ansisop->contenido)+1;
		return size_4;
	default:
		return 0;
	}
}
