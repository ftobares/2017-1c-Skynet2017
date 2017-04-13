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
int puertoProg;
int puertoCPU;
char* ipMemoria;
char* puertoMemoria;
char* ipFileSystem;
char* puertoFileSystem;
int socket_memoria;
int socket_fs;

//Declaracion de funciones
void cargar_y_mostrar_configuracion();
bool validar_configuracion(t_config* config); //Poner en UtilsLibray
t_list* get_config_list_de_string_array(char* key); //Poner en UtilsLibray
void imprimirSemIds(char* config_param);
void imprimirSemInt(char* config_param);
void imprimirSharedVars(char* config_param);
int conectar_otro_server(char* ip, char* puerto);

void *connection_handler(void *);
int iniciar_servidor();

int main(int argc, char* argv) {

	cargar_y_mostrar_configuracion();

	int v_valor_retorno = iniciar_servidor();

	return v_valor_retorno;
}

bool validar_configuracion(t_config* config) {
	return (config_keys_amount(config) > 0);
}

t_list* get_config_list_de_string_array(char* key) {
	t_list* list = list_create();
	char** array = config_get_array_value(config, key);
	int i = 0;
	while (array[i] != NULL) {
		list_add(list, array[i]);
		i++;
	}
	return list;
}

void imprimirSemIds(char* config_param) {
	printf("SEM_IDS es %s \n", config_param);
}

void imprimirSemInt(char* config_param) {
	printf("SEM_INIT es %s \n", config_param);
}

void imprimirSharedVars(char* config_param) {
	printf("SHARED_VARS es %s \n", config_param);
}

void cargar_y_mostrar_configuracion() {

	/** Leer archivo de configuracion */
	char* configPath;
	configPath = string_new();
	string_append(&configPath, "./src/");
	string_append(&configPath, "kernel.config");
	config = config_create(configPath);

	if (!validar_configuracion(config)) {
		printf("No se encontró el archivo de configuración.");
		free(config); //Libero la memoria de config
	}

	puertoProg = config_get_int_value(config, "PUERTO_PROG");
	puertoCPU = config_get_int_value(config, "PUERTO_CPU");
	ipMemoria = config_get_string_value(config, "IP_MEMORIA");
	puertoMemoria = config_get_string_value(config, "PUERTO_MEMORIA");
	ipFileSystem = config_get_string_value(config, "IP_FS");
	puertoFileSystem = config_get_string_value(config, "PUERTO_FS");
	int quantum = config_get_int_value(config, "QUANTUM");
	int quantumSleep = config_get_int_value(config, "QUANTUM_SLEEP");
	char* algoritmo = config_get_string_value(config, "ALGORITMO");
	int gradoMultiprog = config_get_int_value(config, "GRADO_MULTIPROG");
	char* semIds = get_config_list_de_string_array("SEM_IDS");
	char* semInit = get_config_list_de_string_array("SEM_INIT");
	char* sharedVars = get_config_list_de_string_array("SHARED_VARS");
	int stackSize = config_get_int_value(config, "STACK_SIZE");

	printf("Imprimir archivo de configuración: \n");
	printf("PUERTO_PROG es %d \n", puertoProg);
	printf("PUERTO_CPU es %d \n", puertoCPU);
	printf("IP_MEMORIA es %s \n", ipMemoria);
	printf("PUERTO_MEMORIA es %s \n", puertoMemoria);
	printf("IP_FS es %s \n", ipFileSystem);
	printf("PUERTO_FS es %s \n", puertoFileSystem);
	printf("QUANTUM es %d \n", quantum);
	printf("QUANTUM_SLEEP es %d \n", quantumSleep);
	printf("ALGORITMO es %s \n", algoritmo);
	printf("GRADO_MULTIPROG es %d \n", gradoMultiprog);
	printf("STACK_SIZE es %d \n", stackSize);
//	list_iterate(semIds, imprimirSemIds);
//	list_iterate(semIds, imprimirSemInt);
//	list_iterate(semIds, imprimirSharedVars);
}

int conectar_otro_server(char* ip, char* puerto) {
	struct addrinfo hints;
	struct addrinfo *serverInfo;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC; // Permite que la maquina se encargue de verificar si usamos IPv4 o IPv6
	hints.ai_socktype = SOCK_STREAM; // Indica que usaremos el protocolo TCP
	getaddrinfo(ip, puerto, &hints, &serverInfo); // Carga en serverInfo los datos de la conexion
	int serverSocket;
	serverSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype,
			serverInfo->ai_protocol);

	connect(serverSocket, serverInfo->ai_addr, serverInfo->ai_addrlen);
	freeaddrinfo(serverInfo); // No lo necesitamos mas

	printf("Conectado a la ip=%s puerto=%s ya se pueden enviar mensajes\n", ip,
			puerto);
	return serverSocket;
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
	server.sin_port = htons(puertoProg);
	// Bind
	if (bind(socket_desc, (struct sockaddr *) &server, sizeof(server)) < 0) {
		// print the error message
		perror("bind failed. Error");
		return 1;
	}
	puts("bind done");

	//Conectarse a la Memoria
	socket_memoria = conectar_otro_server(ipMemoria, puertoMemoria);

	//Conectarse al FileSystem
	socket_fs = conectar_otro_server(ipFileSystem, puertoFileSystem);

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

	if(pthread_join(thread_id, NULL) == 0){
		close(cliente_socket);
		close(socket_desc);
		close(socket_memoria);
		close(socket_fs);
		return 0;
	}else{
		perror("Error en Thread");
		return 1;
	}
}

void *connection_handler(void *socket_cliente) {

	int sock = *(int*) socket_cliente;
	int read_size;
	char cliente_mensaje[PACKAGESIZE];
	while ((read_size = recv(sock, cliente_mensaje, PACKAGESIZE, 0)) > 0) {
		cliente_mensaje[read_size] = '\0';
		fputs(cliente_mensaje, stdout);
		//Reenvio el mensaje a Memoria y FileSystem
		send(socket_memoria, cliente_mensaje, strlen(cliente_mensaje) + 1, 0);
		send(socket_fs, cliente_mensaje, strlen(cliente_mensaje) + 1, 0);
		memset(cliente_mensaje, 0, PACKAGESIZE);
	}
	fflush(stdout);
	return 0;
}


//Es lo anterior dejo por si sirve
//void conectar(){
//	struct addrinfo hints;
//	struct addrinfo *serverInfo;
//
//	memset(&hints, 0, sizeof(hints));
//	hints.ai_family = AF_UNSPEC;
//	hints.ai_flags = AI_PASSIVE;
//	hints.ai_socktype = SOCK_STREAM;
//
//	getaddrinfo(NULL, puertoProg, &hints, &serverInfo); // Notar que le pasamos NULL como IP, ya que le indicamos que use localhost en AI_PASSIVE
//
//	int listenningSocket;
//	listenningSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);
//
//	bind(listenningSocket,serverInfo->ai_addr, serverInfo->ai_addrlen);
//	freeaddrinfo(serverInfo);
//
//	while(1){
//		listen(listenningSocket, BACKLOG);		// IMPORTANTE: listen() es una syscall BLOQUEANTE.
//
//		puts("Connection accepted --- Esperando conexiones?");
//
//		pthread_t sniffer_thread;
//
//		struct sockaddr_in addr;
//		socklen_t addrlen = sizeof(addr);
//
//		int socketCliente = accept(listenningSocket, (struct sockaddr *) &addr,
//				&addrlen);
//
//		int *new_sock = malloc(1);
//		*new_sock = socketCliente;
//
//		if (pthread_create(&sniffer_thread, NULL, connection_handler,
//				(void*) new_sock)) {
//			printf("No se creo el hilo");
//			perror("could not create thread");
//			return;
//		}
//
//		//Now join the thread , so that we dont terminate before the thread
//		pthread_join(sniffer_thread, NULL);
//		puts("Handler assigned");
//	}
//
//	close(listenningSocket);
//}
//
//void *connection_handler(void *socketCliente)
//{
//	printf("Se creo el hilo");
//	int sock = *(int*)socketCliente;
//
//	printf("Cliente conectado.\n");
//
//	int enviar = 1;
//	char message[PACKAGESIZE];
//
//	printf("Conectado al servidor. Bienvenido al sistema, ya puede enviar mensajes. Escriba 'exit' para salir\n");
//
//	while(enviar){
//		fgets(message, PACKAGESIZE, stdin);			// Lee una linea en el stdin (lo que escribimos en la consola) hasta encontrar un \n (y lo incluye) o llegar a PACKAGESIZE.
//		if (!strcmp(message,"exit\n")) enviar = 0;			// Chequeo que el usuario no quiera salir
//		if (enviar) send(sock, message, strlen(message) + 1, 0); 	// Solo envio si el usuario no quiere salir.
//	}
//
//	close(sock);
//}
