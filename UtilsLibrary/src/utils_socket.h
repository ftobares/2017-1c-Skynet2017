/*
 * utils_socket.h
 *
 *  Created on: 16/4/2017
 *      Author: utnso
 */

#ifndef SRC_UTILS_SOCKET_H_
#define SRC_UTILS_SOCKET_H_

#include <stdbool.h>

typedef struct {
	int socket;
	struct addrinfo* socket_info;
} t_socket;

typedef struct {
	int socket;
	struct sockaddr_in* socket_info;
} t_master_socket;

t_socket cliente_crear_socket(char* ip, char* puerto);

t_socket servidor_crear_socket(int puerto);

t_master_socket servidor_crear_socket_master(int puerto);

t_socket conectar_a_otro_servidor(char* ip, char* puerto);

bool servidor_socket_bind_and_listen(t_master_socket un_socket, int opt, int conexiones_maximas);

#endif /* SRC_UTILS_SOCKET_H_ */
