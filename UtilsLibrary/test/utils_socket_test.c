/*
 * utils_socket_test.c
 *
 *  Created on: 9/5/2017
 *      Author: utnso
 */

//#include "<src/utils_socket.h>";
//
//#include <sys/types.h>
//#include <sys/socket.h>
//#include <netdb.h>
//#include <unistd.h>
//#include <string.h>

//typedef struct __attribute__((__packed__ )){
//	int32_t id_tipo;
//	int32_t tamanio;
//} t_header;
//
//typedef struct __attribute__((__packed__ )){
//	t_header header;
//	uint32_t socket; /*Creo que es util tenerlo, pero despues vemos si es necesario*/
//	char* data;
//} t_buffer;
//
//typedef struct{
//	uint32_t valor1;
//	char* valor2;
//} t_mensaje1;
//
//typedef struct{
//	int valor1;
//	int valor2;
//} t_mensaje2;
//
//t_buffer* crear_buffer(int32_t tipo_mensaje, int32_t size, int32_t un_socket) {
//	t_buffer* buffer_temp = malloc(sizeof(t_buffer));
//	t_header header;
//	header.id_tipo = tipo_mensaje;
//	header.tamanio = size;
//	buffer_temp->header = header;
//	buffer_temp->socket = un_socket;
//	buffer_temp->data = malloc(header.tamanio);
//	return buffer_temp;
//}
//
//void destruir_buffer(t_buffer* buffer){
//	free(buffer->data);
//	free(buffer);
//}
//
//t_buffer* serializar_mensajes(void* data, int tipo_mensaje, int size, int un_socket){
//	int offset = 0;
//	t_buffer* buffer = crear_buffer(tipo_mensaje, size, un_socket);
//
//	switch(buffer->header.id_tipo){
//	case 1:
//		printf("Inicio serializacion \n");
//		//Casteo la data al tipo de struct del mensaje
//		t_mensaje1* mensaje = (struct t_mensaje1*) data;
//
//		//Agrego el encabezado en la estructura de datos
//		memcpy(buffer->data + offset, &buffer->header, sizeof(buffer->header));
//		offset += sizeof(buffer->header);
//
//		//Agrego el mensaje
////		int32_t host_to_network = htonl(mensaje->valor1);
//		memcpy(buffer->data + offset, &mensaje->valor1, sizeof(int32_t));
//		offset += sizeof(int32_t);
//
//		memcpy(buffer->data + offset, mensaje->valor2, strlen(mensaje->valor2)+1);
//
//		return buffer;
//	default:
//		return buffer;
//	}
//}
//
//void* deserializar_mensaje(char* buffer, int tipo_mensaje){
//	int offset = 0;
//
//	switch(tipo_mensaje){
//	case 0:
//		printf("Inicio deserializacion header \n");
//		printf("Alloco %d memoria \n",sizeof(t_header));
//		t_header* header = malloc(sizeof(t_header));
//		header->id_tipo = 0;
//		header->tamanio = 0;
//
//		printf("Copio en header->id_tipo <= %p \n", buffer);
//		memcpy(&header->id_tipo, buffer, sizeof(header->id_tipo));
//		offset += sizeof(header->id_tipo);
//
//		printf("Copio en header->tamanio <= %p \n", buffer + offset);
//		memcpy(&header->tamanio, buffer + offset, sizeof(header->tamanio));
//
//		return header;
//	case 1:
//		printf("Inicio deserializacion mensaje 1\n");
//		printf("Alloco %d memoria \n",sizeof(t_mensaje1));
//		t_mensaje1* mensaje = malloc(sizeof(t_mensaje1));
//		mensaje->valor1 = 0;
//
//		offset += sizeof(t_header);
//
//		printf("Copio en mensaje->valor1 <= %p \n", buffer + offset);
//		memcpy(&mensaje->valor1, buffer + offset, sizeof(mensaje->valor1));
////		uint32_t network_to_host = ntohl(mensaje->valor1);
////		mensaje->valor1 = network_to_host;
//		offset += sizeof(mensaje->valor1);
//		printf("Mensaje->valor1 <= %d \n",mensaje->valor1);
//
//		printf("Copio en mensaje->valor2 <= %p \n", buffer + offset);
//		char* valor2 = strdup(buffer + offset);
//		mensaje->valor2 = valor2;
//		printf("Mensaje->valor2 <= %s \n",mensaje->valor2);
//
//		return mensaje;
//	default:
//		return NULL;
//	}
//}

//int main(){
//
//	t_mensaje1 mensaje;
//	mensaje.valor1 = 10;
//	mensaje.valor2 = malloc(23);
//	strcpy(mensaje.valor2,"1234567890123456789012\0");
//	int tipo_mensaje = 1;
//	int size = sizeof(t_header)+sizeof(mensaje.valor1)+strlen(mensaje.valor2)+1; //Variable segun valor2
//	int un_socket = 1;
//
//	printf("Antes de serializar val1=%i y val2=%s \n",mensaje.valor1,mensaje.valor2);
//
//	t_buffer* buffer = serializar_mensajes(&mensaje, tipo_mensaje, size, un_socket);
//
//	printf("Ya serializé resultado:\n");
//	printf("buffer->header.id_tipo => %d \n",buffer->header.id_tipo);
//	printf("buffer->header.tamanio => %d \n",buffer->header.tamanio);
//	printf("buffer->socket => %d \n",buffer->socket);
//	printf("buffer->data => %p \n",buffer->data);
//
//	t_buffer buffer2;
//	buffer2.socket = 1;
//	buffer2.header.tamanio = 0;
//	buffer2.header.id_tipo = 0;
//	buffer2.data = malloc(sizeof(t_header));
//
//	char* data_a_enviar = buffer->data;
//
//	t_header* header = deserializar_mensaje(data_a_enviar, buffer2.header.id_tipo);
//	buffer2.header.id_tipo = header->id_tipo;
//	buffer2.header.tamanio = header->tamanio;
//
//	memset(buffer2.data, 0, sizeof(t_header));
//	buffer2.data = realloc(buffer2.data, buffer2.header.tamanio);
//	buffer2.data = data_a_enviar;
//
//	printf("buffer2.header.id_tipo => %d \n", buffer2.header.id_tipo);
//	printf("buffer2.header.tamanio => %d \n", buffer2.header.tamanio);
//
//	t_mensaje1* mensaje1_2 = deserializar_mensaje(buffer2.data, header->id_tipo);
//
//	printf("Luego de deserializar val1=%d y val2=%s \n",mensaje1_2->valor1,mensaje1_2->valor2);
//
//	free(mensaje.valor2);
//	destruir_buffer(buffer);
//	free(header);
//	free(mensaje1_2->valor2);
//	free(mensaje1_2);
//}
