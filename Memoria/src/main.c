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

#define BACKLOG 3			// Cantidad conexiones maximas
#define PACKAGESIZE 1024	// Size maximo del paquete a enviar

//Variables Globales
t_config *config;
int puerto;

//Declaracion de funciones
void cargar_y_mostrar_configuracion();
bool validar_configuracion(t_config* config); //Poner en UtilsLibray
void *connection_handler(void *);
int iniciar_servidor();

int main(int argc, char* argv) {

	cargar_y_mostrar_configuracion();

	int v_valor_retorno = iniciar_servidor();

	return v_valor_retorno;
}

bool validar_configuracion(t_config* config){
	return (config_keys_amount(config) > 0);
}

void cargar_y_mostrar_configuracion(){

	/** Leer archivo de configuracion */
	char* configPath;
	configPath = string_new();
	string_append(&configPath, "./src/");
	string_append(&configPath, "memoria.config");
	config = config_create(configPath);

	if (!validar_configuracion(config)) {
		printf("No se encontró el archivo de configuración.");
		free(config); //Libero la memoria de config
	}

	puerto = config_get_int_value(config,"PUERTO");
	int marcos = config_get_int_value(config,"MARCOS");
	int marcosSize = config_get_int_value(config,"MARCO_SIZE");
	int entradasCache = config_get_int_value(config,"ENTRADAS_CACHE");
	int cacheXProc = config_get_int_value(config,"CACHE_X_PROC");
	char* reemplazoCache = config_get_string_value(config,"REEMPLAZO_CACHE");
	int retardoMemoria = config_get_int_value(config,"RETARDO_MEMORIA");

	printf("Imprimir archivo de configuración: \n");
	printf("PUERTO es %d \n",puerto);
	printf("MARCOS es %d \n",marcos);
	printf("MARCOS_SIZE es %d \n",marcosSize);
	printf("ENTRADAS_CACHE es %d \n",entradasCache);
	printf("CACHE_X_PROC es %d \n",cacheXProc);
	printf("REEMPLAZO_CACHE es %s \n",reemplazoCache);
	printf("RETARDO_MEMORIA es %d \n",retardoMemoria);
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
	server.sin_port = htons(puerto);
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
