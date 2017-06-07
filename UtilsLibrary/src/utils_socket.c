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
#include "utils_protocolo.h"

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

/*@NAME: crear_socket_servidor (deprecated)
 *@DESC: Recibe un int con el puerto a escuchar.
 *		 Crea el socket y asigna el identificador. Rellena addrinfo
 *		 con AF_INET, INADDR_ANY y el PUERTO.
 */
t_socket servidor_crear_socket(uint32_t puerto) {
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
t_master_socket servidor_crear_socket_master(uint32_t puerto) {
	t_master_socket un_socket;
	un_socket.socket = 0;
	if ((un_socket.socket = socket(AF_INET, SOCK_STREAM, 0)) != -1) {
		un_socket.socket_info.sin_family = AF_INET;
		un_socket.socket_info.sin_addr.s_addr = INADDR_ANY;
		un_socket.socket_info.sin_port = htons(puerto);
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

/*@NAME: servidor_crear_socket_bind_and_listen
 *@DESC: Funcion para crear, configurar y dejar escuchando un socket
 *		 en un puerto. Uso exclusivo para socket select.
 */
t_master_socket servidor_crear_socket_bind_and_listen(uint32_t puerto, uint32_t opt,
		uint32_t conexiones_maximas) {

	t_master_socket un_socket;
	un_socket.socket = 0;
	if ((un_socket.socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Error al crear el socket servidor\n");
		exit(EXIT_FAILURE);
	}

	//setear socket maestro para que permita multiples conexiones (Buenas practicas)
	if (setsockopt(un_socket.socket, SOL_SOCKET, SO_REUSEADDR, (char *) &opt,
			sizeof(opt)) < 0) {
		perror("ERROR. connect_bind_and_listen - Fallo setsockopt");
		exit(EXIT_FAILURE);
	}

	un_socket.socket_info.sin_family = AF_INET;
	un_socket.socket_info.sin_addr.s_addr = INADDR_ANY; //inet_addr("127.0.0.1");
	un_socket.socket_info.sin_port = htons(puerto);

	//bind
	if (bind(un_socket.socket, (struct sockaddr*) &un_socket.socket_info,
			sizeof(un_socket.socket_info)) != 0) {
		perror("ERROR. Fallo socket bind");
		exit(EXIT_FAILURE);
	}
	printf("Listener en puerto %d \n", un_socket.socket_info.sin_port);

	//Listen
	if (listen(un_socket.socket, conexiones_maximas) < 0) {
		perror("ERROR. Fallo listen");
		exit(EXIT_FAILURE);
	}

	return un_socket;
}

/*@NAME: crear_buffer
 *@DESC: Crea un buffer a partir de los datos enviados
 */
t_buffer* crear_buffer(uint32_t tipo_mensaje, uint32_t size, uint32_t un_socket) {
	t_buffer* buffer_temp = malloc(sizeof(t_buffer));
	t_header header;
	header.id_tipo = tipo_mensaje;
	header.tamanio = size;
	buffer_temp->header = header;
	buffer_temp->socket = un_socket;
	buffer_temp->data = malloc(header.tamanio);
	return buffer_temp;
}

/*@NAME: destruir_buffer
 *@DESC: Libera un buffer
 */
void destruir_buffer(t_buffer* buffer) {
	free(buffer->data);
	free(buffer);
}

/*@NAME: recibir_mensaje
 *@DESC: Recibe datos y los guarda en el buffer
 *@DESC: tener en cuenta que hay que deserializar el buffer.data
 */
t_buffer recibir_mensaje(uint32_t un_socket) {

	t_buffer buffer;
	buffer.socket = un_socket;
	buffer.header.tamanio = 0;
	buffer.header.id_tipo = 1;
	buffer.data = malloc(sizeof(t_header));

	// Recibir datos y guardarlos en el buffer
	// Primero recibo el header para saber tipo de mensaje y tamaño
	int bytes_retorno = recv(buffer.socket, buffer.data, sizeof(t_header), MSG_WAITALL);
	if (bytes_retorno == -1) {
		buffer.header.id_tipo = -1;
		perror("Error al recibir header\n");
		return buffer;
	}else if(bytes_retorno == 0){
		buffer.header.tamanio = 0;
		buffer.header.id_tipo = 0;
		printf("Se desconecto el socket=%d\n",buffer.socket);
		return buffer;
	}
	t_header* header = deserializar_mensaje(buffer.data,buffer.header.id_tipo);
	buffer.header.id_tipo = header->id_tipo;
	buffer.header.tamanio = header->tamanio;

	// Segundo recervar memoria suficiente para el mensaje
	memset(buffer.data, 0, sizeof(t_header));
	buffer.data = realloc(buffer.data, buffer.header.tamanio);
	if (read(buffer.socket, buffer.data, buffer.header.tamanio) == -1) {
		buffer.header.id_tipo = 0;
		free(buffer.data);
		perror("Error al recibir el payload\n");
	}

	return buffer;
}

/*@NAME: enviar_mensaje
 *@DESC: Envia datos cargados en el buffer
 *@DESC: Datos ya deben venir serializados
 *@RETURN: Devuelve 1-fallo/false , 0-exito/true
 */
int enviar_mensaje(void* data, uint32_t tipo_mensaje, uint32_t size, uint32_t un_socket) {

	t_buffer* buffer = serializar_mensajes(data, tipo_mensaje, size, un_socket);

	if (send(buffer->socket, &buffer->data, sizeof(buffer->header.tamanio), 0) > 0) {
		return 0;
	}
	perror("Error al Enviar Datos\n");
	return 1;
}

/*@NAME: serializar_mensajes
 *@DESC: La funcion pone en el buffer todos los datos a ser enviados,
 *@DESC: header y mensaje. Luego se serializan segun el tipo de dato.
 *@DESC: Datos de ingreso:
 *@DESC: 	data-> struct del mensaje
 *@DESC: 	tipo_mensaje-> mensaje segun protocolo
 *@DESC: 	size-> tamaño del archivo
 *@DESC: 	un_socket-> socket de conexion
 */
t_buffer* serializar_mensajes(void* data, uint32_t tipo_mensaje, uint32_t size, uint32_t un_socket) {
	uint32_t offset = 0;
	t_buffer* buffer = crear_buffer(tipo_mensaje, size, un_socket);

	switch(buffer->header.id_tipo){
	case MSJ_HANDSHAKE:
		printf("Inicio serializacion handshake \n");
		t_handshake* handshake = (struct t_handshake*) data;

		//Host to Network
		buffer->header.id_tipo = htonl(buffer->header.id_tipo);
		buffer->header.tamanio = htonl(buffer->header.tamanio);

		memcpy(buffer->data + offset, &buffer->header, sizeof(buffer->header));
		offset += sizeof(buffer->header);

		memcpy(buffer->data + offset, handshake->handshake, strlen(handshake->handshake)+1);

		return buffer;
	case MSJ_PROGRAMA_ANSISOP:
		printf("Inicio serializacion programa ansisop \n");
		t_programa_ansisop* programa_ansisop = (struct t_programa_ansisop*) data;

		//Host to Network
		buffer->header.id_tipo = htonl(buffer->header.id_tipo);
		buffer->header.tamanio = htonl(buffer->header.tamanio);

		memcpy(buffer->data + offset, &buffer->header, sizeof(buffer->header));
		offset += sizeof(buffer->header);

		memcpy(buffer->data + offset, programa_ansisop->contenido, strlen(programa_ansisop->contenido)+1);

		return buffer;

	default:
		return buffer;
	}
}

/*@NAME: deserializar_mensaje (
 *@DESC: La funcion deserializa un stream de datos poniendolo
 *@DESC: en un struct segun el tipo de mensaje. Devuelve un
 *@DESC: puntero a ese struct cargado.
 */
void* deserializar_mensaje(char* stream_buffer, uint32_t tipo_mensaje) {
	uint32_t offset = 0;

	switch(tipo_mensaje){
	case MSJ_HEADER:
		printf("Inicio deserializacion header \n");
		printf("Alloco %d memoria \n",sizeof(t_header));
		t_header* header = malloc(sizeof(t_header));
		header->id_tipo = 0;
		header->tamanio = 0;

		printf("Copio en header->id_tipo <= %p \n", stream_buffer);
		memcpy(&header->id_tipo, stream_buffer, sizeof(header->id_tipo));
		offset += sizeof(header->id_tipo);

		printf("Copio en header->tamanio <= %p \n", stream_buffer + offset);
		memcpy(&header->tamanio, stream_buffer + offset, sizeof(header->tamanio));

		//Network_to_Host
		header->id_tipo = ntohl(header->id_tipo);
		header->tamanio = ntohl(header->tamanio);

		return header;
	case MSJ_HANDSHAKE:
		printf("Inicio deserializacion handshake 2\n");
		t_handshake* handshake = malloc(sizeof(t_handshake));

		offset += sizeof(t_header);

		handshake->handshake = strdup(stream_buffer + offset);

		return handshake;
	case MSJ_PROGRAMA_ANSISOP:
		printf("Inicio deserializacion programa ansisop 2\n");
		t_programa_ansisop* programa_ansisop = malloc(sizeof(t_programa_ansisop));

		offset += sizeof(t_header);

		memcpy(&programa_ansisop->pid, stream_buffer + offset, sizeof(programa_ansisop->pid));
		offset += sizeof(programa_ansisop->pid);

		//Network_to_Host
		programa_ansisop->pid = ntohl(programa_ansisop->pid);

		programa_ansisop->contenido = strdup(stream_buffer + offset);

		return programa_ansisop;

	default:
		return NULL;
	}
}

void clean_or_init_buffer(t_buffer* buffer){
	buffer->socket = 999;
	buffer->header.id_tipo = 0;
	buffer->header.tamanio = 0;
}
