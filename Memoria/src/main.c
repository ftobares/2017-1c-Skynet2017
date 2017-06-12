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
#include <src/utils_memoria.h>
#include <commons/log.h>

#define BACKLOG 3			// Cantidad conexiones maximas
#define PACKAGESIZE 1024	// Size maximo del paquete a enviar
#define TIPO_PROYECTO 5

//Comandos Consola aceptados
#define COMANDO_CONSOLA_HELP						1
#define COMANDO_CONSOLA_RETARDO						2
#define COMANDO_CONSOLA_DUMP_CACHE					3
#define COMANDO_CONSOLA_DUMP_ESTRUCTURAS_MEMORIA 	4
#define COMANDO_CONSOLA_DUMP_CONTENIDO_MEMORIA		5
#define COMANDO_CONSOLA_FLUSH						6
#define COMANDO_CONSOLA_SIZE_MEMORY					7
#define COMANDO_CONSOLA_SIZE_PID					8

//Archivos
#define NOMBRE_ARCHIVO_DISCO	     "archivo_disco_memoria.txt"
#define NOMBRE_LOG			 		 "log_memoria.txt"

char CMD_HELP[5] = "help\0";
char CMD_RETARDO[8] = "retardo\0";
char CMD_DUMP_CACHE[11] = "dump_cache\0";
char CMD_DUMP_ESTRUCTURAS_MEMORIA[13] = "dump_structs\0";
char CMD_DUMP_CONTENIDO_MEMORIA[13] = "dump_content\0";
char CMD_FLUSH[6] = "flush\0";
char CMD_SIZE_MEMORY[12] = "size_memory\0";
char CMD_SIZE_PID[9] = "size_pid\0";

//Variables Globales
t_memoria_config* config;
t_master_socket socket_desc;
t_memoria * memoria;
t_entrada_cache * cache;
int so_ejecutando = 1;
FILE * archivo_disco_memoria;
t_log* log;

//Declaracion de funciones
void *connection_handler(void *);
int iniciar_servidor();

int main(int argc, char** argv) {

	char* file_path;
	file_path = string_new();
	string_append(&file_path, "./src/memoria.config");
	config = cargar_configuracion(file_path, TIPO_PROYECTO);
//
	int v_valor_retorno = iniciar_servidor();
//
//	return v_valor_retorno;

//	archivo_disco_memoria = fopen(NOMBRE_ARCHIVO_DISCO, "wt");
//	log = log_create(NOMBRE_LOG, "MEMORIA", true, LOG_LEVEL_TRACE);
//
//	//HILOS
//	pthread_t hilo_conexiones, hilo_consola; //hacer funcion de conexiones
//
//	//instanciamos la memoria ppal y cache
//	memoria = new_memoria(config->marcos, config->marcosSize);
//	cache = new_cache(config->entradasCache, config->marcosSize);
//
//	//iniciamos los hilos
//	pthread_create(&hilo_conexiones, NULL, (void*) iniciar_servidor, NULL );
//	pthread_create(&hilo_consola, NULL, (void*) consola, NULL );
//
//	//finalizamos
//	free(config);
//	free(memoria);
//	free(cache);
//	//free(log); ??????
//	fclose(archivo_disco_memoria);
	return 0;
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
//	int c, cliente_socket;
//	struct sockaddr_in client;
//	socket_desc = servidor_crear_socket_master(config->puerto);

	// Bind
	if (bind(socket_desc, (struct sockaddr *) &(server), sizeof(server)) < 0) {
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

// Métodos consola
void consola() {

	while (so_ejecutando)
	{
		char comando[13];
		int comando_introducido;

		printf("\n--> Ingrese un comando. 'help' para más información.: ");
		scanf("%s", comando);
		comando_introducido = obtener_comando_consola(comando);

		switch (comando_introducido)
		{
			case COMANDO_CONSOLA_HELP:
				consola_comando_help();
				break;
			case COMANDO_CONSOLA_RETARDO:
				consola_comando_retardo();
				break;
			case COMANDO_CONSOLA_DUMP_CACHE:
				consola_comando_dump_cache();
				break;
			case COMANDO_CONSOLA_DUMP_ESTRUCTURAS_MEMORIA:
				consola_comando_dump_estructuras_memoria();
				break;
			case COMANDO_CONSOLA_DUMP_CONTENIDO_MEMORIA:
				consola_comando_dump_contenido_memoria();
				break;
			case COMANDO_CONSOLA_FLUSH:
				consola_comando_flush();
				break;
			case COMANDO_CONSOLA_SIZE_MEMORY:
				consola_comando_size_memory();
				break;
			case COMANDO_CONSOLA_SIZE_PID:
				consola_comando_size_pid();
				break;
			default:
				//error de comando no valido
				break;
		}
	}

}

int obtener_comando_consola(char * buffer) {
	if (strncmp(buffer, CMD_HELP, sizeof(CMD_HELP) - 1) == 0)
		return COMANDO_CONSOLA_HELP;

	if (strncmp(buffer, CMD_RETARDO, sizeof(CMD_RETARDO) - 1) == 0)
		return COMANDO_CONSOLA_RETARDO;

	if (strncmp(buffer, CMD_DUMP_CACHE, sizeof(CMD_DUMP_CACHE) - 1) == 0)
		return COMANDO_CONSOLA_DUMP_CACHE;

	if (strncmp(buffer, CMD_DUMP_ESTRUCTURAS_MEMORIA, sizeof(CMD_DUMP_ESTRUCTURAS_MEMORIA) - 1) == 0)
		return COMANDO_CONSOLA_DUMP_ESTRUCTURAS_MEMORIA;

	if (strncmp(buffer, CMD_DUMP_CONTENIDO_MEMORIA, sizeof(CMD_DUMP_CONTENIDO_MEMORIA) - 1) == 0)
		return COMANDO_CONSOLA_DUMP_CONTENIDO_MEMORIA;

	if (strncmp(buffer, CMD_FLUSH, sizeof(CMD_FLUSH) - 1) == 0)
		return COMANDO_CONSOLA_FLUSH;

	if (strncmp(buffer, CMD_SIZE_MEMORY, sizeof(CMD_SIZE_MEMORY) - 1) == 0)
		return COMANDO_CONSOLA_SIZE_MEMORY;

	if (strncmp(buffer, CMD_SIZE_PID, sizeof(CMD_SIZE_PID) - 1) == 0)
		return COMANDO_CONSOLA_SIZE_PID;

	return 0;
}

void consola_comando_help() {
	char * msj_help = string_new();

	string_append(&msj_help, "Comandos: \n");
	string_append_with_format(&msj_help, "%s: Retorna los comandos. \n", CMD_HELP);
	string_append_with_format(&msj_help, "%s: Modifica la cantidad de milisegundos que debe esperar el Proceso Memoria antes de responder una solicitud. \n", CMD_RETARDO);
	string_append_with_format(&msj_help, "%s: Genera un reporte en pantalla y en un archivo en disco del estado actual de la Memoria Caché. \n", CMD_DUMP_CACHE);
	string_append_with_format(&msj_help, "%s: Genera un reporte en pantalla y en un archivo en disco del estado actual de la Tabla de Páginas y de los Procesos Activos. \n", CMD_DUMP_ESTRUCTURAS_MEMORIA);
	string_append_with_format(&msj_help, "%s: Genera un reporte en pantalla y en un archivo en disco del estado actual de los datos almacenados en la memoria de todos los procesos o de un proceso en particular. \n", CMD_DUMP_CONTENIDO_MEMORIA);
	string_append_with_format(&msj_help, "%s: Limpia completamente el contenido de la Caché. \n", CMD_FLUSH);
	string_append_with_format(&msj_help, "%s: Indica el tamaño de la memoria en cantidad de frames, la cantidad de frames ocupados, y la cantidad de frames libres. \n", CMD_SIZE_MEMORY);
	string_append_with_format(&msj_help, "%s: Indica el tamaño total de un proceso. \n", CMD_SIZE_PID);

	printf("%s", msj_help);

	free(msj_help);
}

void consola_comando_retardo() {
	printf("--> Retardo actual [ms]: %d\n", config->retardoMemoria);
	printf("--> Retardo nuevo [ms]: ");
	scanf("%d", &config->retardoMemoria);
}

//falta lo de guardar en archivo de disco
void consola_comando_dump_cache() {
	int i;
	for(i=0; i<config->entradasCache; i++){
		printf("PID: %d\n", cache[i].pid);
		printf("NRO_PAGINA: %d\n", cache[i].nro_pagina);
		printf("CONTENIDO_PAGINA: %s\n\n", cache[i].contenido_pagina);
	}
}

//probar
//falta lo de guardar en archivo de disco
void consola_comando_dump_estructuras_memoria() {
	printf("Tabla de páginas: \n");
	printf("N° Marco \t PID\t N° Página\n");
	int i;
	for(i=0; i<memoria->cantidad_marcos; i++){
		printf("%d\t %d\t %d\t\n", memoria->paginas[i].nro_marco, memoria->paginas[i].pid, memoria->paginas[i].nro_pagina);
	}
}

//PROBAR
//falta lo de guardar en archivo de disco
void consola_comando_dump_contenido_memoria() {
	char opcion;
	printf("¿Todos los procesos?: S/N");
	scanf("%c", &opcion);
	if(opcion == 'N' || opcion == 'n'){
		int pid, i;
		printf("PID: ");
		scanf("%d", &pid);
		for(i=0; i<config->marcos; i++){
			if(pid == memoria->paginas[i].pid){
				//imprimir toda esa pag ?????????
				printf("%s", solicitar_bytes_de_una_pagina(memoria->memoria_principal, i, 0, config->marcosSize)); //ponele
			}
		}
	}
	if(opcion == 'S' || opcion == 's'){
		//imprimir toda la memoria!!!
		printf("%s", memoria->memoria_principal); //ponele
	} else {
		printf("Comando No Válido");
	}
}

void consola_comando_flush() {
	int i;
	for(i = 0; i < config->entradasCache; i++){
			cache[i].pid = -1;
			cache[i].nro_pagina = -1;
			cache[i].contenido_pagina = NULL;
	}
}

void consola_comando_size_memory() {
	printf("Tamaño de la memoria:\n");
	printf("\t- Cantidad de frames: %d\n", memoria->cantidad_marcos);
	printf("\t- Cantidad de frames ocupados: %d\n", memoria->cantidad_marcos - memoria->cantidad_paginas_disponibles);
	printf("\t- Cantidad de frames libres: %d\n", memoria->cantidad_paginas_disponibles);
}

//probar
void consola_comando_size_pid() {
	int pid;
	printf("PID: ");
	scanf("%d", &pid);
	t_entrada_programa * entrada_programa = malloc(sizeof(entrada_programa));
	entrada_programa = list_find(memoria->programas, (void *) es_igual_a_pid(pid, memoria->programas->head));
	printf("Tamaño total del proceso: %d páginas.", entrada_programa->cantidad_paginas);
	free(entrada_programa);
}

bool es_igual_a_pid(int pid, t_entrada_programa * entrada_programa){
	return pid == entrada_programa->pid;
}
