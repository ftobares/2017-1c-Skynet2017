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

/*@NAME: crear_socket_cliente
 *@DESC: Recibe dos strings, uno para la IP y otro para el PUERTO.
 *		 Crea el socket y asgina el identificador. Rellana la estructura
 *		 addrinfo con AF_UNSPEC y SOCK_STREAM, utilizando getaddrinfo.
 */
t_socket cliente_crear_socket(char* ip, char* puerto);

/*@NAME: crear_socket_servidor (deprecated)
 *@DESC: Recibe un int con el puerto a escuchar.
 *		 Crea el socket y asigna el identificador. Rellena addrinfo
 *		 con AF_INET, INADDR_ANY y el PUERTO.
 */
t_socket servidor_crear_socket(int puerto);

/*@NAME: crear_socket_master_servidor
 *@DESC: Recibe un int con el puerto a escuchar.
 *		 Crea el socket y asigna el identificador. Rellena addrinfo
 *		 con AF_INET, INADDR_ANY y el PUERTO.
 */
t_master_socket servidor_crear_socket_master(int puerto);

/*@NAME: conectar_a_otro_servidor
 *@DESC: Funcion exclusiva para que un servidor, se conecte a otro
 *		 servidor.
 */
t_socket conectar_a_otro_servidor(char* ip, char* puerto);

/*@NAME: servidor_crear_socket_bind_and_listen
 *@DESC: Funcion para crear, configurar y dejar escuchando un socket
 *		 en un puerto. Uso exclusivo para socket select.
 */
t_master_socket servidor_crear_socket_bind_and_listen(int puerto, int opt, int conexiones_maximas);

/*@NAME: crear_buffer
 *@DESC: Crea un buffer a partir de los datos enviados
 */
t_buffer* crear_buffer(int32_t tipo_mensaje, int32_t size, int32_t un_socket);

/*@NAME: destruir_buffer
 *@DESC: Libera un buffer
 */
void destruir_buffer(t_buffer* buffer);

/*@NAME: recibir_mensaje
 *@DESC: Recibe datos y los guarda en el buffer
 *@DESC: tener en cuenta que hay que deserializar el buffer.data
 */
t_buffer recibir_mensaje(int32_t un_socket);

/*@NAME: enviar_mensaje
 *@DESC: Envia datos cargados en el buffer
 *@DESC: Datos ya deben venir serializados
 *@RETURN: Devuelve 1-fallo/false , 0-exito/true
 */
int enviar_mensaje(void* data, int tipo_mensaje, int size, int un_socket);

/*@NAME: serializar_mensajes
 *@DESC: La funcion pone en el buffer todos los datos a ser enviados,
 *@DESC: header y mensaje. Luego se serializan segun el tipo de dato.
 *@DESC: Datos de ingreso:
 *@DESC: 	data-> struct del mensaje
 *@DESC: 	buffer-> struct t_buffer creado anteriormente
 */
t_buffer* serializar_mensajes(void* data, int tipo_mensaje, int size, int un_socket);

/*@NAME: deserializar_mensaje (
 *@DESC: La funcion deserializa un stream de datos poniendolo
 *@DESC: en un struct segun el tipo de mensaje. Devuelve ese
 *@DESC: struct cargado.
 *@DESC: FIXME: Tiene un bug en el cual la deserializacion del int queda mal,
 *@DESC: y esto proboca que el mensaje tambien quede corrido 2 posiciones.
 */
void* deserializar_mensaje(char* stream_buffer, int tipo_mensaje);

/*@NAME: clean_or_init_buffer (
 *@DESC: Funcion para limpiar inicializar o limpiar basura del buffer
 */
void clean_or_init_buffer(t_buffer* buffer);

#endif /* SRC_UTILS_SOCKET_H_ */
