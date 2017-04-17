/*
 * utils_socket.c
 *
 *  Created on: 16/4/2017
 *      Author: utnso
 */

#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>
#include <commons/string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include "utils_socket.h"

/*@NAME: crear_socket_cliente
 *@DESC: Recibe dos strings, uno para la IP y otro para el PUERTO.
 *		 Crea el socket y asgina el identificador. Rellana la estructura
 *		 addrinfo con AF_UNSPEC y SOCK_STREAM, utilizando getaddrinfo.
 */
t_socket cliente_crear_socket(char* ip, char* puerto) {
	t_socket un_socket;
	struct addrinfo hints;
	un_socket.socket = 0;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	getaddrinfo(ip, puerto, &hints, &(un_socket.socket_info));
	if ((un_socket.socket = socket(un_socket.socket_info->ai_family,
			un_socket.socket_info->ai_socktype,
			un_socket.socket_info->ai_protocol)) != 0) {
		return un_socket;
	} else {
		perror("Error al crear el socket cliente\n");
		return un_socket;
	}
}

/*@NAME: crear_socket_servidor (QUIZAS DEJE DE USARSE)
 *@DESC: Recibe un int con el puerto a escuchar.
 *		 Crea el socket y asigna el identificador. Rellena addrinfo
 *		 con AF_INET, INADDR_ANY y el PUERTO.
 */
t_socket servidor_crear_socket(int puerto) {
	t_socket un_socket;
	un_socket.socket = 0;
	if ((un_socket.socket = socket(AF_INET, SOCK_STREAM, 0)) != -1) {
		un_socket.socket_info->ai_family = AF_INET;
		un_socket.socket_info->ai_socktype = INADDR_ANY;
		un_socket.socket_info->ai_protocol = htons(puerto);
		return un_socket;
	} else {
		perror("Error al crear el socket servidor\n");
		return un_socket;
	}
}

/*@NAME: crear_socket_master_servidor
 *@DESC: Recibe un int con el puerto a escuchar.
 *		 Crea el socket y asigna el identificador. Rellena addrinfo
 *		 con AF_INET, INADDR_ANY y el PUERTO.
 */
t_master_socket servidor_crear_socket_master(int puerto) {
	t_master_socket un_socket;
	un_socket.socket = 0;
	if ((un_socket.socket = socket(AF_INET, SOCK_STREAM, 0)) != -1) {
		un_socket.socket_info->sin_family = AF_INET;
		un_socket.socket_info->sin_addr.s_addr = INADDR_ANY;
		un_socket.socket_info->sin_port = htons(puerto);
		return un_socket;
	} else {
		perror("Error al crear el socket servidor\n");
		return un_socket;
	}
}

/*@NAME: conectar_a_otro_servidor
 *@DESC: Funcion exclusiva para que un servidor, se conecte a otro
 *		 servidor.
 */
t_socket conectar_a_otro_servidor(char* ip, char* puerto) {
	t_socket socket_cliente = cliente_crear_socket(ip, puerto);
	connect(socket_cliente.socket, socket_cliente.socket_info->ai_addr,
			socket_cliente.socket_info->ai_addrlen);
	printf("Conectado a la ip=%s puerto=%s\n", ip, puerto);

	return socket_cliente;
}

/*@NAME: socket_server_bind_and_listen
 *@DESC: Funcion para configurar socket y dejar escuchando a traves
 *		 de un puerto.
 */
bool servidor_socket_bind_and_listen(t_master_socket un_socket, int opt, int conexiones_maximas) {

	//setear socket maestro para que permita multiples conexiones (Buenas practicas)
	if (setsockopt(un_socket.socket, SOL_SOCKET, SO_REUSEADDR, (char *) &opt,
			sizeof(opt)) < 0) {
		perror("ERROR. connect_bind_and_listen - Fallo setsockopt");
		exit(EXIT_FAILURE);
	}

	//bind
	if (bind(un_socket.socket, (struct sockaddr *) &(un_socket.socket_info), sizeof(un_socket.socket_info)) < 0) {
		perror("ERROR. Fallo socket bind");
		exit(EXIT_FAILURE);
	}
	printf("Listener en puerto %d \n", un_socket.socket_info->sin_port);

	//Listen
	if (listen(un_socket.socket, conexiones_maximas) < 0) {
		perror("ERROR. Fallo listen");
		exit(EXIT_FAILURE);
	}

	return true;
}
