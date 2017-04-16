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

#define BACKLOG 30			// Cantidad conexiones maximas
#define PACKAGESIZE 1024	// Size maximo del paquete a enviar
#define TRUE   1
#define FALSE  0
#define CantClientes 30

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
	/*char* semIds = get_config_list_de_string_array("SEM_IDS");
	char* semInit = get_config_list_de_string_array("SEM_INIT");
	char* sharedVars = get_config_list_de_string_array("SHARED_VARS");*/
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

	int opt = TRUE;
	int master_socket, addrlen, new_socket, cliente_socket[CantClientes],
			max_clientes = CantClientes, actividad, i, valorLectura, sd, read_size;
	int max_sd;
	struct sockaddr_in server;

	char buffer[PACKAGESIZE];

	//set de socket descriptores
	fd_set readfds;

	//a message
	char mensaje[PACKAGESIZE];

	//Conectarse a la Memoria
	socket_memoria = conectar_otro_server(ipMemoria, puertoMemoria);

	//Conectarse al FileSystem
	socket_fs = conectar_otro_server(ipFileSystem, puertoFileSystem);

	//inicializar todos los cliente_socket[] a 0 (No chequeado)
	for (i = 0; i < max_clientes; i++) {
		cliente_socket[i] = 0;
	}

	//crear un socket maestro
	if ((master_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	puts("Socket maestro creado");

	//setear socket maestro para que permita multiples conexiones (Buenas practicas)
	if (setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *) &opt,
			sizeof(opt)) < 0) {
		perror("setsockopt failed");
		exit(EXIT_FAILURE);
	}

	//Tipo de socket creado
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(puertoProg);

	//bind
	if (bind(master_socket, (struct sockaddr *) &server, sizeof(server)) < 0) {
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
	printf("Listener en puerto %d \n", puertoProg);

	//Listen
	if (listen(master_socket, BACKLOG) < 0) {
		perror("listen failed");
		exit(EXIT_FAILURE);
	}

	//accept the incoming connection
	addrlen = sizeof(server);
	puts("Esperando por conexiones entrantes...");

	while (TRUE) {
		//Limpiar el socket set
		FD_ZERO(&readfds);

		//Agregar el socket maestro al set
		FD_SET(master_socket, &readfds);
		max_sd = master_socket;

		//Agregar sockets hijos al set
		for (i = 0; i < max_clientes; i++) {
			//socket descriptor
			sd = cliente_socket[i];

			//si es un socket valido, se agrega a la lista de lectura
			if (sd > 0)
				FD_SET(sd, &readfds);

			//Numero de file descriptor mas alto, lo necesita para la función de seleccion
			if (sd > max_sd)
				max_sd = sd;
		}

		//espera por una actividad de uno de los sockets, el timeout es NULL (Se espera indefinidamente)
		actividad = select(max_sd + 1, &readfds, NULL, NULL, NULL);

		if (actividad < 0) {
			printf("select error");
		}

		//Si pasa algo en el socket maestro, entonces es una conexion entrante
		if (FD_ISSET(master_socket, &readfds)) {
			if ((new_socket = accept(master_socket, (struct sockaddr *) &server,
					(socklen_t*) &addrlen)) < 0) {
				perror("accept");
				exit(EXIT_FAILURE);
			}

			//inform user of socket number - used in send and receive commands
			printf(
					"Nueva conexion, socket fd es %d , ip es : %s , puerto : %d \n",
					new_socket, inet_ntoa(server.sin_addr),
					ntohs(server.sin_port));


			//Recibir y enviar mensaje a Memoria y FileSystem
			if ((read_size = recv(new_socket, mensaje, PACKAGESIZE, 0)) > 0){
				mensaje[read_size] = '\0';
			}

			if( send(socket_memoria, mensaje, strlen(mensaje), 0) != strlen(mensaje) )
			{
				perror("send memoria failed");
			}

			if( send(socket_fs, mensaje, strlen(mensaje), 0) != strlen(mensaje) )
			{
				perror("send filesystem failed");
			}

			printf("%d",read_size);
			puts(mensaje);
			puts("mensajes a memoria y filesystem enviados correctamente");

			//Agregar new socket al array de sockets
			for (i = 0; i < max_clientes; i++) {
				//Si la posicion es vacia
				if (cliente_socket[i] == 0) {
					cliente_socket[i] = new_socket;
					printf("Agregando a la lista de sockets como %d\n", i);

					break;
				}
			}
		}

		//Caso contrario, alguna operacion E/S en algun otro socket
		for (i = 0; i < max_clientes; i++) {
			sd = cliente_socket[i];

			if (FD_ISSET(sd, &readfds)) {
				//Verificar si fue por cierre, y tambien para leer un mensaje entrante
				if ((valorLectura = read(sd, buffer, PACKAGESIZE)) == 0) {
					//Alguien se desconecto, obtenemos los detalles e imprimimos
					getpeername(sd, (struct sockaddr*) &server,
							(socklen_t*) &addrlen);
					printf("Host desconectado , ip %s , puerto %d \n",
							inet_ntoa(server.sin_addr), ntohs(server.sin_port));

					//Cerrar el socket y marcar como 0 en la lista para reusar
					close(sd);
					cliente_socket[i] = 0;
				} else {
					buffer[valorLectura] = '\0';
				}
			}
		}
	}

	return 0;
}
