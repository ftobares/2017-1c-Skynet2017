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
	case 1:
		printf("Calcular tamaño Mensaje 1\n");
		t_mensaje1* mensaje1 = (struct t_mensaje1*) p_mensaje;
		int size_1 = sizeof(t_header)+sizeof(mensaje1->valor1)+strlen(mensaje1->valor2)+1;
		return size_1;
	case 2:
		printf("Calcular tamaño Mensaje 2\n");
		t_mensaje2* mensaje2 = (struct t_mensaje2*) p_mensaje;
		int size_2 = sizeof(t_header)+sizeof(mensaje2->valor1)+sizeof(mensaje2->valor2);
		return size_2;
	case 3:
		printf("Calcular tamaño Handshake\n");
		t_handshake* handshake = (struct t_handshake*) p_mensaje;
		int size_3 = sizeof(t_header)+strlen(handshake->handshake)+1;
		return size_3;
	case 4:
		printf("Calcular tamaño Programa AnsiSOp\n");
		t_programa_ansisop* programa_ansisop = (struct t_programa_ansisop*) p_mensaje;
		int size_4 = sizeof(t_header)+sizeof(programa_ansisop->pid)+strlen(programa_ansisop->contenido)+1;
		return size_4;
	default:
		return 0;
	}
}
