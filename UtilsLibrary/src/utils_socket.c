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
#include "utils_protocolo.h";

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
t_master_socket servidor_crear_socket_bind_and_listen(int puerto, int opt,
		int conexiones_maximas) {

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
 *@DESC: (Faltaria agregar la serializacion - capaz cambie algo)
 */
t_buffer crear_buffer(t_header header, uint32_t un_socket) {
	t_buffer buffer_temp;
	buffer_temp.header = header;
	buffer_temp.socket = un_socket;
	return buffer_temp;
}

/*@NAME: recibir_mensaje
 *@DESC: Recibe datos y los guarda en el buffer
 *@DESC: (Falta la serializacion - seguro algo va a cambiar)
 */
t_buffer recibir_mensaje(int32_t un_socket) {

	t_buffer buffer;
	buffer.socket = un_socket;
	t_header header;
	header.tamanio = -1;

	// Recibir datos y guardarlos en el buffer
	// Primero recibo el header para saber tipo de mensaje y tamaño
	if (recv(buffer.socket, &header, sizeof(header), MSG_WAITALL) == -1) {
		buffer.header.id_tipo = 0;
		perror("Error al recibir header");
		return buffer;
	}

	// Segundo recervar memoria suficiente para el mensaje
	buffer.data = malloc(header.tamanio);
	if (read(buffer.socket, buffer.data, header.tamanio) == -1) {
		buffer.header.id_tipo = 0;
		free(buffer.data);
		perror("Error al recibir el payload");
	}

	return buffer;
}

/*@NAME: enviar_mensaje
 *@DESC: Envia datos cargados en el buffer
 *@DESC: (Falta la serializacion - seguro algo va a cambiar)
 *@RETURN: Devuelve 1-fallo/false , 0-exito/truel
 */
int enviar_mensaje(t_header header, t_buffer buffer) {
	if (send(buffer.socket, &header, sizeof(header), 0) > 0) {
		if (write(buffer.socket, buffer.data, header.tamanio) > 0) {
			return 0;
		}
	}
	perror("Error al Enviar Datos\n");
	return 1;
}

/*@NAME: serializar_mensajes
 *@DESC: La funcion pone en el buffer todos los datos a ser enviados,
 *@DESC: header y mensaje. Luego se serializan segun el tipo de dato.
 *@DESC: Datos de ingreso:
 *@DESC: 	data-> struct del mensaje
 *@DESC: 	buffer-> struct t_buffer creado anteriormente
 */
t_buffer serializar_mensajes(void* data, t_buffer buffer){

	switch(buffer.header.id_tipo){
	case 1:
		printf("Inicio serializacion");
//		buffer.data = malloc(buffer.header.tamanio);

		t_mensaje1* mensaje = (struct t_mensaje1*) data;

		//Agrego el encabezado en la estructura de datos
		memcpy(&buffer.data, &buffer.header, sizeof(buffer.header));

		//Serializar int
		uint32_t host_to_network = htonl(mensaje->valor1);
		memcpy(&buffer.data+sizeof(t_header), &host_to_network, sizeof(mensaje->valor1));

		//Serializo string
		memcpy(&buffer.data+sizeof(t_header)+sizeof(mensaje->valor1), &mensaje->valor2, sizeof(mensaje->valor2));

		return buffer;
	default:
		buffer.header.tamanio = -1;
		return buffer;
	}
}

/*@NAME: deserializar_mensaje (
 *@DESC: La funcion deserializa un stream de datos poniendolo
 *@DESC: en un struct segun el tipo de mensaje. Devuelve ese
 *@DESC: struct cargado.
 *@DESC: FIXME: Tiene un bug en el cual la deserializacion del int queda mal,
 *@DESC: y esto proboca que el mensaje tambien quede corrido 2 posiciones.
 */
void* deserializar_mensaje(t_buffer buffer){

	switch(buffer.header.id_tipo){
	case 1:
		printf("Inicio deserializacion");
		t_mensaje1* mensaje;
		mensaje = malloc(sizeof(t_mensaje1));

		memcpy(&mensaje->valor1, &buffer.data+sizeof(t_header), sizeof(mensaje->valor1));
		memcpy(&mensaje->valor2, &buffer.data+sizeof(t_header)+sizeof(mensaje->valor1), sizeof(mensaje->valor2));

		uint32_t network_to_host = ntohl(mensaje->valor1);
		mensaje->valor1 = network_to_host;

		return mensaje;
	default:
		return mensaje;
	}
}

//PARA TEST
//#include <sys/types.h>
//#include <sys/socket.h>
//#include <netdb.h>
//#include <unistd.h>
//
//typedef struct __attribute__((__packed__ )){
//	int32_t id_tipo;
//	int32_t tamanio;
//} t_header;
//
//typedef struct __attribute__((__packed__ )){
//	t_header header;
//	uint32_t socket; /*Creo que es util tenerlo, pero despues vemos si es necesario*/
//	void* data;
//} t_buffer;
//
//typedef struct{
//	uint32_t valor1;
//	char valor2[30];
//} t_mensaje1;
//
//typedef struct{
//	int valor1;
//	int valor2;
//} t_mensaje2;
//
//t_buffer serializar_mensajes(void* data, t_buffer buffer){
//
//	switch(buffer.header.id_tipo){
//	case 1:
//		printf("Inicio serializacion");
//		buffer.data = malloc(buffer.header.tamanio);
//
//		t_mensaje1* mensaje = (struct t_mensaje1*) data;
//
//		//Agrego el encabezado en la estructura de datos
//		memcpy(&buffer.data, &buffer.header, sizeof(buffer.header));
//
//		//Serializar int
//		uint32_t host_to_network = htonl(mensaje->valor1);
//		memcpy(&buffer.data+sizeof(t_header), &host_to_network, sizeof(mensaje->valor1));
//
//		//Serializo string
//		memcpy(&buffer.data+sizeof(t_header)+sizeof(mensaje->valor1), &mensaje->valor2, sizeof(mensaje->valor2));
//
//		return buffer;
//	default:
//		buffer.header.tamanio = -1;
//		return buffer;
//	}
//}
//
//void* deserializar_mensaje(t_buffer buffer){
//
//	switch(buffer.header.id_tipo){
//	case 1:
//		printf("Inicio deserializacion");
//		t_mensaje1* mensaje;
//		mensaje = malloc(sizeof(t_mensaje1));
//
//		memcpy(&mensaje->valor1, &buffer.data+sizeof(t_header), sizeof(mensaje->valor1));
//		memcpy(&mensaje->valor2, &buffer.data+sizeof(t_header)+sizeof(mensaje->valor1), sizeof(mensaje->valor2));
//
//		uint32_t network_to_host = ntohl(mensaje->valor1);
//		mensaje->valor1 = network_to_host;
//
//		return mensaje;
//	default:
//		return mensaje;
//	}
//}
//
//int main(){
//
//	t_buffer buffer;
//	buffer.header.id_tipo = 1;
//	buffer.header.tamanio = sizeof(t_header)+sizeof(t_mensaje1);
//	buffer.socket = 1;
////	buffer.data = malloc(buffer.header.tamanio);
//
//	t_mensaje1 mensaje;
//	mensaje.valor1 = 10;
//	char cadena[30] = "abcdefghijklmnopqrstuvwxyz1234\0";
//	strcpy(mensaje.valor2, cadena);
//
//	printf("Antes de serializar val1=%d y val2=%s\n",mensaje.valor1,mensaje.valor2);
//
//	buffer = serializar_mensajes(&mensaje, buffer);
//
//	printf("Ya serializé\n");
//
//	t_mensaje1* mensaje1_2;
//	mensaje1_2 = deserializar_mensaje(buffer);
//
//	printf("Luego de deserializar val1=%d y val2=%s\n",mensaje1_2->valor1,mensaje1_2->valor2);
//}
