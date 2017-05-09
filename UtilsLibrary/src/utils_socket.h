/*
 * utils_socket.h
 *
 *  Created on: 16/4/2017
 *      Author: utnso
 */

#ifndef SRC_UTILS_SOCKET_H_
#define SRC_UTILS_SOCKET_H_

#include <stdbool.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>

//Estructuras genericas
typedef struct {
	int socket;
	struct addrinfo* socket_info;
} t_socket;

typedef struct {
	int socket;
	struct sockaddr_in socket_info;
} t_master_socket;

typedef struct __attribute__((__packed__ )){
	int32_t id_tipo;
	int32_t tamanio;
} t_header;

typedef struct __attribute__((__packed__ )){
	t_header header;
	int32_t socket; /*Creo que es util tenerlo, pero despues vemos si es necesario*/
	char* data;
} t_buffer;

//Funciones
t_socket cliente_crear_socket(char* ip, char* puerto);

t_socket servidor_crear_socket(int puerto);

t_master_socket servidor_crear_socket_master(int puerto);

t_socket conectar_a_otro_servidor(char* ip, char* puerto);

t_master_socket servidor_crear_socket_bind_and_listen(int puerto, int opt, int conexiones_maximas);

t_buffer* crear_buffer(int32_t tipo_mensaje, int32_t size, int32_t un_socket);

void destruir_buffer(t_buffer* buffer);

t_buffer recibir_mensaje(int32_t un_socket);

int enviar_mensaje(void* data, int tipo_mensaje, int size, int un_socket);

t_buffer* serializar_mensajes(void* data, int tipo_mensaje, int size, int un_socket);

void* deserializar_mensaje(char* stream_buffer, int tipo_mensaje);

#endif /* SRC_UTILS_SOCKET_H_ */
