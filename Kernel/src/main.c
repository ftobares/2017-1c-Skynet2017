#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h> //for threading , link with pthread (como las commons)

#define PUERTO "6667"		//este viene en el config????
#define BACKLOG 5			// Define cuantas conexiones vamos a mantener pendientes al mismo tiempo
#define PACKAGESIZE 1024	// Define cual va a ser el size maximo del paquete a enviar

//Variables Globales
t_config *config;

//Declaracion de funciones
void cargarYMostrarConfiguracion();
bool validar_configuracion(t_config* config); //Poner en UtilsLibray
t_list* get_config_list_de_string_array(char* key); //Poner en UtilsLibray
void imprimirSemIds(char* config_param);
void imprimirSemInt(char* config_param);
void imprimirSharedVars(char* config_param);
void conectar();
void *connection_handler(void *); //the thread function

int main(int argc, char* argv) {

	cargarYMostrarConfiguracion();
	return 0;
}

bool validar_configuracion(t_config* config){
	return (config_keys_amount(config) > 0);
}

t_list* get_config_list_de_string_array(char* key){
	t_list* list = list_create();
	char** array = config_get_array_value(config,key);
	int i = 0;
	while(array[i]!=NULL){
		list_add(list,array[i]);
		i++;
	}
	return list;
}

void imprimirSemIds(char* config_param){
	printf("SEM_IDS es %s \n",config_param);
}

void imprimirSemInt(char* config_param){
	printf("SEM_INIT es %s \n",config_param);
}

void imprimirSharedVars(char* config_param){
	printf("SHARED_VARS es %s \n",config_param);
}

void cargarYMostrarConfiguracion(){

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

	int puertoProg = config_get_int_value(config,"PUERTO_PROG");
	int puertoCPU = config_get_int_value(config,"PUERTO_CPU");
	char* ipMemoria = config_get_string_value(config,"IP_MEMORIA");
	int puertoMemoria = config_get_int_value(config,"PUERTO_MEMORIA");
	char* ipFileSystem = config_get_string_value(config,"IP_FS");
	int puertoFileSystem = config_get_int_value(config,"PUERTO_FS");
	int quantum = config_get_int_value(config,"QUANTUM");
	int quantumSleep = config_get_int_value(config,"QUANTUM_SLEEP");
	char* algoritmo = config_get_string_value(config,"ALGORITMO");
	int gradoMultiprog = config_get_int_value(config,"GRADO_MULTIPROG");
	char* semIds = get_config_list_de_string_array("SEM_IDS"); //config_get_array_value(config,"SEM_IDS");
	char* semInit = get_config_list_de_string_array("SEM_INIT"); //config_get_array_value(config,"SEM_INIT");
	char* sharedVars = get_config_list_de_string_array("SHARED_VARS"); //config_get_array_value(config,"SHARED_VARS");
	char* stackSize = config_get_int_value(config,"STACK_SIZE");

	printf("Imprimir archivo de configuración: \n");
	printf("PUERTO_PROG es %d \n",puertoProg);
	printf("PUERTO_CPU es %d \n",puertoCPU);
	printf("IP_MEMORIA es %s \n",ipMemoria);
	printf("PUERTO_MEMORIA es %d \n",puertoMemoria);
	printf("IP_FS es %s \n",ipFileSystem);
	printf("PUERTO_FS es %d \n",puertoFileSystem);
	printf("QUANTUM es %d \n",quantum);
	printf("QUANTUM_SLEEP es %d \n",quantumSleep);
	printf("ALGORITMO es %s \n",algoritmo);
	printf("GRADO_MULTIPROG es %d \n",gradoMultiprog);
	printf("STACK_SIZE es %d \n",stackSize);
	list_iterate(semIds,imprimirSemIds);
	list_iterate(semIds,imprimirSemInt);
	list_iterate(semIds,imprimirSharedVars);
}

void conectar(){
	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo(NULL, PUERTO, &hints, &serverInfo); // Notar que le pasamos NULL como IP, ya que le indicamos que use localhost en AI_PASSIVE

	int listenningSocket;
	listenningSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);

	bind(listenningSocket,serverInfo->ai_addr, serverInfo->ai_addrlen);
	freeaddrinfo(serverInfo);

	while(1){
		listen(listenningSocket, BACKLOG);		// IMPORTANTE: listen() es una syscall BLOQUEANTE.

		puts("Connection accepted --- Esperando conexiones?");

		pthread_t sniffer_thread;

		struct sockaddr_in addr;
		socklen_t addrlen = sizeof(addr);

		int socketCliente = accept(listenningSocket, (struct sockaddr *) &addr,
				&addrlen);

		int *new_sock = malloc(1);
		*new_sock = socketCliente;

		if (pthread_create(&sniffer_thread, NULL, connection_handler,
				(void*) new_sock)) {
			printf("No se creo el hilo");
			perror("could not create thread");
			return;
		}

		//Now join the thread , so that we dont terminate before the thread
		pthread_join(sniffer_thread, NULL);
		puts("Handler assigned");
	}

	close(listenningSocket);
}

void *connection_handler(void *socketCliente)
{
	printf("Se creo el hilo");
	int sock = *(int*)socketCliente;

	printf("Cliente conectado.\n");

	int enviar = 1;
	char message[PACKAGESIZE];

	printf("Conectado al servidor. Bienvenido al sistema, ya puede enviar mensajes. Escriba 'exit' para salir\n");

	while(enviar){
		fgets(message, PACKAGESIZE, stdin);			// Lee una linea en el stdin (lo que escribimos en la consola) hasta encontrar un \n (y lo incluye) o llegar a PACKAGESIZE.
		if (!strcmp(message,"exit\n")) enviar = 0;			// Chequeo que el usuario no quiera salir
		if (enviar) send(sock, message, strlen(message) + 1, 0); 	// Solo envio si el usuario no quiere salir.
	}

	close(sock);
}
