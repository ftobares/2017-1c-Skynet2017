#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h> //Agregar a C/C++ Build > GCC C Linker > Libraries > pthread (como las commons)
#include <string.h> // strlen
#include <arpa/inet.h> //inet_addr
#include <src/utils_config.h>
#include <src/utils_socket.h>

#define BACKLOG 3			// Cantidad conexiones maximas
#define PACKAGESIZE 1024	// Size maximo del paquete a enviar
#define TIPO_PROYECTO 3

//Variables Globales
t_fs_config* config;
t_master_socket socket_desc;

//Declaracion de funciones
void *connection_handler(void *);
int iniciar_servidor();

int main(int argc, char* argv) {

	char* file_path;
	file_path = string_new();
	string_append(&file_path, "./src/file_system.config");
	config = cargar_configuracion(file_path, TIPO_PROYECTO);

	int v_valor_retorno = iniciar_servidor();

	return v_valor_retorno;
}

int iniciar_servidor() {
	int socket_desc, cliente_socket, c;
	struct sockaddr_in server, client;
	// Create socket
	socket_desc = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_desc == -1) {
		printf("Could not create socket");
	}
	puts("Socket creado");
	// Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(config->puerto);
//	int cliente_socket, c;
//	struct sockaddr_in client;
//	socket_desc = servidor_crear_socket_master(config->puerto);

	// Bind
	if (bind(socket_desc, (struct sockaddr *) &server, sizeof(server)) < 0) {
		// print the error message
		perror("bind failed. Error");
		return 1;
	}
	puts("bind done");

	listen(socket_desc, BACKLOG);

	puts("Esperando por conexiones entrantes...");
	c = sizeof(struct sockaddr_in);
	pthread_t thread_id;
	while ((cliente_socket = accept(socket_desc, (struct sockaddr *) &client,
			(socklen_t*) &c))) {
		printf("Conexion aceptada, cliente: %d\n", cliente_socket);
		if (pthread_create(&thread_id, NULL, connection_handler,
				(void*) &cliente_socket) < 0) {
			perror("could not create thread");
			return 1;
		}
		puts("Handler assigned");
	}
	if (cliente_socket < 0) {
		perror("accept failed");
		return 1;
	}

	return 0;
}

void *connection_handler(void *socket_cliente) {

	int sock = *(int*) socket_cliente;
	int read_size;
	char cliente_mensaje[PACKAGESIZE];
	while ((read_size = recv(sock, cliente_mensaje, PACKAGESIZE, 0)) > 0) {
		cliente_mensaje[read_size] = '\0';
		fputs(cliente_mensaje, stdout);
		memset(cliente_mensaje, 0, PACKAGESIZE);
	}
	fflush(stdout);
	return 0;
}
