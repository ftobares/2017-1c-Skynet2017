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
uint32_t calcular_tamanio_mensaje(void* p_mensaje, uint32_t tipo_mensaje) {
	uint32_t size;

	switch(tipo_mensaje){
	case MSJ_HANDSHAKE:
		printf("Calcular tamaño Handshake\n");
		t_handshake* handshake = (struct t_handshake*) p_mensaje;
		size = strlen(handshake->handshake)+1;
		return size;
	case MSJ_PROGRAMA_ANSISOP:
		printf("Calcular tamaño Programa AnsiSOp\n");
		t_programa_ansisop* programa_ansisop = (struct t_programa_ansisop*) p_mensaje;
		size = sizeof(programa_ansisop->pid)+strlen(programa_ansisop->contenido)+1;
		return size;
	case MSJ_FINALIZAR_PROGRAMA:
		printf("Calcular tamaño Programa AnsiSOp\n");
		t_programa_ansisop* programa_finalizar = (struct t_programa_ansisop*) p_mensaje;
		size = sizeof(programa_finalizar->pid);
		return size;
	default:
		return 0;
	}
}
