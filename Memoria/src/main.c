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
#include <commons/log.h>
#include <math.h>

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

//Estructuras
typedef struct{
	int pid;
	int nro_pagina;
	void * contenido_pagina;
} t_entrada_cache;

typedef struct{
	int nro_marco;
	int pid; //proceso al que pertenece la pagina
	int nro_pagina; //orden de la pagina del proceso
} t_entrada_pagina;

typedef struct{
	int pid;
	int cantidad_paginas;
} t_entrada_programa;

//Variables Globales
t_memoria_config* config;
t_master_socket socket_desc;
void * memoria;
void * cache;
t_entrada_pagina * tabla_paginas;
int size_tabla_paginas;
t_entrada_programa * admin_programas;
int size_admin_programas;
int cant_programas;
t_entrada_cache * admin_cache;
t_list * cache_lru;
int ent_cache_aux;
int so_ejecutando = 1;
FILE * archivo_disco_memoria;
t_log* logger;

char CMD_HELP[5] = "help\0";
char CMD_RETARDO[8] = "retardo\0";
char CMD_DUMP_CACHE[11] = "dump_cache\0";
char CMD_DUMP_ESTRUCTURAS_MEMORIA[13] = "dump_structs\0";
char CMD_DUMP_CONTENIDO_MEMORIA[13] = "dump_content\0";
char CMD_FLUSH[6] = "flush\0";
char CMD_SIZE_MEMORY[12] = "size_memory\0";
char CMD_SIZE_PID[9] = "size_pid\0";

//Declaracion de funciones
void *connection_handler(void *);
int iniciar_servidor();
//Funciones Memoria Principal
void new_memoria();
void actualizar_tabla_paginas_en_memoria();
void actualizar_admin_programas_en_memoria();
int paginas_disponibles();
void inicializar_programa(int pid, int paginas_requeridas);
void asignar_entrada_a_pagina(int pid, int nro_pagina);
void * solicitar_bytes_de_una_pagina(int pid, int nro_pagina, int offset, int tamanio); //arreglar
void * devolver_pagina(int pid, int nro_pagina);
int get_nro_marco(int pid, int nro_pagina);
void almacenar_bytes_en_una_pagina(int pid, int nro_pagina, int offset, int tamanio, void * buffer);
void asignar_paginas_a_proceso(int pid, int paginas_requeridas);
void finalizar_programa(int pid);
void liberar_entrada_admin_programas(int pid);
int get_cantidad_paginas(int pid);
void liberar_proceso_memoria(int pid);
void liberar_entrada_tabla_paginas(int nro_entrada);
void sumar_pags_a_admin_programa(int pid, int cant_pags);
int get_entrada_programa_disponible();
void asignar_entrada_programa(int pid, int paginas_requeridas);
//Funciones Caché
void new_cache();
void reservar_cache();
void crear_admin_cache();
void reemplazo_entrada_cache(int pid, int nro_pagina, void * contenido_pagina);
void liberar_pagina_cacheada(int nro_entrada);
void liberar_proceso_cache(int pid);
int entrada_en_cache(int pid, int nro_pagina);
bool nro_es_igual_a(void * elemento);
void inicializar_cache_lru();
bool puede_estar_en_cache(int pid);
int * posiciones_entradas_cache_x_proceso(int pid);
int posicion_lru_cache_x_proceso(int pid);
//Funciones Consola
void consola();
int obtener_comando_consola(char * buffer);
void consola_comando_help();
void consola_comando_retardo();
void consola_comando_dump_cache();
void consola_comando_dump_estructuras_memoria();
void consola_comando_dump_contenido_memoria();
void consola_comando_flush();
void consola_comando_size_memory();
void consola_comando_size_pid();
//Otras
int funcion_hash(int pid, int nro_pagina);
void imprimir_y_grabar_en_archivo(const char* mensaje, ...);



int main(int argc, char** argv) {

	char* file_path;
	file_path = string_new();
	string_append(&file_path, "./src/memoria.config");
	config = cargar_configuracion(file_path, TIPO_PROYECTO);

	int v_valor_retorno = iniciar_servidor();

//	return v_valor_retorno;

//	archivo_disco_memoria = fopen(NOMBRE_ARCHIVO_DISCO, "wt");
//	logger = log_create(NOMBRE_LOG, "MEMORIA", true, LOG_LEVEL_TRACE);
//
//	//HILOS
//	pthread_t hilo_conexiones, hilo_consola; //hacer funcion de conexiones
//
//	//instanciamos la memoria ppal y cache
//	new_memoria();
//	new_cache();
//	//iniciamos los hilos
//	pthread_create(&hilo_conexiones, NULL, (void*) iniciar_servidor, NULL );
//	pthread_create(&hilo_consola, NULL, (void*) consola, NULL );
//
//	//finalizamos
//	free(config);
//	free(memoria);
//	free(cache);
//	free(logger);
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

//Funciones Memoria Principal
void new_memoria(){
	//MEMORIA
	int tamanio_memoria = config->marcos * config->marcosSize;
	memoria = malloc(tamanio_memoria);
	if(memoria == NULL) {
		//msj error
	} else{
		printf("1) Se creo la memoria\n");		//log
	}
	memset(memoria, '0', tamanio_memoria); //fijarse
	int marco_admin = 0;
	//TABLA DE PAGINAS
	size_tabla_paginas = sizeof(t_entrada_pagina) * config->marcos;
	printf("Size tabla de paginas: %d\n", size_tabla_paginas);
	tabla_paginas = malloc(size_tabla_paginas);
	if(tabla_paginas == NULL){
		free(memoria);
		//msj error
	} else{
		printf("2) Se creo la tabla de paginas\n");
		//log
	}
	int i;
	for(i=0; i<config->marcos; i++){
		tabla_paginas[i].pid = -1;
		tabla_paginas[i].nro_pagina = -1;
		tabla_paginas[i].nro_marco = i;
	}
	if(size_tabla_paginas <= config->marcosSize){
		tabla_paginas[marco_admin].pid = -2; //-2 para estructuras administrativas
		marco_admin++;
		actualizar_tabla_paginas_en_memoria();
		printf("3) Se guardo la tabla de paginas\n");
	} else{
		//ARREGLAR ESTO
		//idk
//		int i = 1;
//		while(size_tabla_paginas >= (config->marcosSize * i)){
//			i++;
//		}
//		int j;
//		for(j=marco_admin; ;){
//
//		}
	}

	//ADMIN PROGRAMAS
	int pag_admin = 2; //una para la tabla pag y otra para las entradas de programas
	cant_programas = config->marcos - pag_admin;
	size_admin_programas = sizeof(t_entrada_programa) * cant_programas;
	printf("Size admin programas: %d\n", size_admin_programas);
	admin_programas = malloc(size_admin_programas);
	if(admin_programas == NULL){
		free(tabla_paginas);
		free(memoria);
		//msj error
	} else{
		printf("4) Se creo admin programas\n");
		//log;
	}
	for(i=0; i<cant_programas; i++){
		admin_programas[i].pid = -1;
		admin_programas[i].cantidad_paginas = -1;
	}
	if(size_admin_programas <= config->marcosSize){
		tabla_paginas[1].pid = -2;
		actualizar_admin_programas_en_memoria();
		printf("5) Se guardo admin programas\n");
	} else{
		//idk
	}
	//pensar bien esto
	//Meter estructuras administrativas
}


void actualizar_tabla_paginas_en_memoria(){
	memcpy(memoria, tabla_paginas, size_tabla_paginas);
}

void actualizar_admin_programas_en_memoria(){
	//FIJARSE EN QUE PAGINA QUEDA ADMIN PROGRAMAS
	memcpy(memoria + config->marcosSize * 1, admin_programas, size_admin_programas);
}

int paginas_disponibles(){
	int i, pags_disponibles = 0;
	for(i=0; i<config->marcos; i++){
		if(tabla_paginas[i].pid == -1){
			pags_disponibles++;
		}
	}
	return pags_disponibles;
}

void inicializar_programa(int pid, int paginas_requeridas){
	if(paginas_requeridas > paginas_disponibles()){
		//avisar al kernel
		printf("No hay suficientes paginas en la memoria :/\n");
	} else {
		//asignar paginas
		asignar_entrada_programa(pid, paginas_requeridas);
		int i;
		for(i=0; i<paginas_requeridas; i++){
			asignar_entrada_a_pagina(pid, i);
		}
	}
}

void asignar_entrada_a_pagina(int pid, int nro_pagina){
	int nro_entrada = funcion_hash(pid, nro_pagina);
	while(tabla_paginas[nro_entrada].pid != -1){
		nro_entrada++;
	}
	tabla_paginas[nro_entrada].pid = pid;
	tabla_paginas[nro_entrada].nro_pagina = nro_pagina;
	actualizar_tabla_paginas_en_memoria();
}
//arreglar
void * solicitar_bytes_de_una_pagina(int pid, int nro_pagina, int offset, int tamanio){
	void * bytes = malloc(tamanio);
	int nro_entrada = entrada_en_cache(pid, nro_pagina);
	if(nro_entrada != -1){
		memcpy(bytes, admin_cache[nro_entrada].contenido_pagina + offset, tamanio);
	} else{
		sleep(config->retardoMemoria);
		//void * pagina = devolver_pagina(pid, nro_pagina);
		reemplazo_entrada_cache(pid, nro_pagina, devolver_pagina(pid, nro_pagina)); //actualiza cache
		memcpy(bytes, devolver_pagina(pid, nro_pagina) + offset, tamanio);
		//free(pagina);
	}
	return bytes;
}

void * devolver_pagina(int pid, int nro_pagina){
	int nro_marco = get_nro_marco(pid, nro_pagina);
	//void * pagina = malloc(config->marcosSize);
//	memcpy(pagina, memoria + nro_marco * config->marcosSize, config->marcosSize);
	return memoria + nro_marco * config->marcosSize;
	//return pagina;
}
//devuelve el numero de marco correspondiente a ese proceso y pagina
int get_nro_marco(int pid, int nro_pagina){
	int entrada = funcion_hash(pid, nro_pagina);
	while(tabla_paginas[entrada].pid != pid && tabla_paginas[entrada].nro_pagina != nro_pagina){
		entrada++;
	}
	return tabla_paginas[entrada].nro_marco;
}
//ver si no está en cache, si dsp hay que llevarla a cache
void almacenar_bytes_en_una_pagina(int pid, int nro_pagina, int offset, int tamanio, void * buffer){
	int entrada_cache = entrada_en_cache(pid, nro_pagina);
	void * pagina = devolver_pagina(pid, nro_pagina);
	if(entrada_cache != -1){
		memcpy(admin_cache[entrada_cache].contenido_pagina + offset, buffer, tamanio);
		memcpy(pagina + offset, buffer, tamanio);
	} else{
		sleep(config->retardoMemoria /1000);
		memcpy(pagina + offset, buffer, tamanio);
		printf("almacenó\n");
		reemplazo_entrada_cache(pid, nro_pagina, pagina);
	}
	//free(pagina);
}

//hay que ver si en la tabla de pagina van las estructuras admn. Si van, marco == nro_entrada
//HEAP--SOLUCIONAR
void asignar_paginas_a_proceso(int pid, int paginas_requeridas){
	if(paginas_requeridas > paginas_disponibles()){
		//avisar al kernel
	} else {
		//asignar paginas
		int cant_pag_anteriores = get_cantidad_paginas(pid);
		int cant_pag_nuevas = cant_pag_anteriores + paginas_requeridas;
		sumar_pags_a_admin_programa(pid, paginas_requeridas);
		actualizar_admin_programas_en_memoria();
		int i, nro_entrada;
		for(i=cant_pag_anteriores; i<cant_pag_nuevas; i++){
			asignar_entrada_a_pagina(pid, i);
		}
	}
}

void finalizar_programa(int pid){
	liberar_proceso_cache(pid);
	liberar_proceso_memoria(pid); //acá ya lo saca de tabla_paginas y admin_programas
	//fijarse otras estructuras
}

void liberar_entrada_admin_programas(int pid){
	int i;
	for(i=0; i<cant_programas; i++){
		if(admin_programas[i].pid == pid){
			admin_programas[i].pid = -1;
			admin_programas[i].cantidad_paginas = -1;
			return;
		}
	}
}

int get_cantidad_paginas(int pid){
	int i;
	for(i=0; i<cant_programas; i++){
		if(admin_programas[i].pid == pid)
			return admin_programas[i].cantidad_paginas;
	}
	return -1;
}

void liberar_proceso_memoria(int pid){
	int cantidad_paginas = get_cantidad_paginas(pid);
	int i;
	for(i=0; i<cantidad_paginas; i++){
		int marco = get_nro_marco(pid, i);
		liberar_entrada_tabla_paginas(marco); //marco es igual a entrada en la tabla???
	}
	liberar_entrada_admin_programas(pid);
}

void liberar_entrada_tabla_paginas(int nro_entrada){
	tabla_paginas[nro_entrada].pid = -1;
	tabla_paginas[nro_entrada].nro_pagina = -1;
	memset(memoria + tabla_paginas[nro_entrada].nro_marco * config->marcosSize, '0', config->marcosSize);
}

void sumar_pags_a_admin_programa(int pid, int cant_pags){
	int i;
	for(i=0; i<cant_programas; i++){
		if(admin_programas[i].pid == pid)
			admin_programas[i].cantidad_paginas += cant_pags;
	}
}

int get_entrada_programa_disponible(){
	int i;
	for(i=0; i<cant_programas; i++){
		if(admin_programas[i].pid == -1)
			return i;
	}
	return -1;
}

void asignar_entrada_programa(int pid, int paginas_requeridas){
	int nro_entrada = get_entrada_programa_disponible();
	admin_programas[nro_entrada].pid = pid;
	admin_programas[nro_entrada].cantidad_paginas = paginas_requeridas;
	actualizar_admin_programas_en_memoria();
}

//Funciones Caché
void new_cache(){
	reservar_cache();
	crear_admin_cache();
	inicializar_cache_lru();
}

void reservar_cache(){
	int tamanio = config->marcosSize * config->entradasCache;
	cache = malloc(tamanio);
	memset(cache, '0', tamanio); //fijarse
	if(cache == NULL){
		//mensaje de error
	} else
		log_trace(logger, "MEMORIA CACHE RESERVADA. Tamaño de la memoria caché: %d.", tamanio);
}

void crear_admin_cache(){
	admin_cache = malloc(sizeof(t_entrada_cache) * config->entradasCache);
	if(admin_cache == NULL){
		//mensaje de error
	} else
		log_trace(logger, "ESTRUCTURAS DE MEMORIA CACHE CREADAS.");
	int i;
	for(i = 0; i < config->entradasCache; i++){
		admin_cache[i].pid = -1;
		admin_cache[i].nro_pagina = -1;
		admin_cache[i].contenido_pagina = cache + i * config->marcosSize;
	}
}

void reemplazo_entrada_cache(int pid, int nro_pagina, void * contenido_pagina){
	int nro_entrada;
	if(puede_estar_en_cache(pid)){
		nro_entrada = list_remove(cache_lru, 0);
		list_add(cache_lru, nro_entrada);
	} else{
		nro_entrada = posicion_lru_cache_x_proceso(pid);
	}
	admin_cache[nro_entrada].pid = pid;
	admin_cache[nro_entrada].nro_pagina = nro_pagina;
	memcpy(admin_cache[nro_entrada].contenido_pagina, contenido_pagina, config->marcosSize);
}

void liberar_pagina_cacheada(int nro_entrada){
	admin_cache[nro_entrada].pid = -1;
	admin_cache[nro_entrada].nro_pagina = -1;
	memset(cache + nro_entrada * config->marcosSize, '0', config->marcosSize);

	//remover el elemento de cache_lru que coincida con nro_entrada
	//poner al principio de la lista esta entrada
	ent_cache_aux = nro_entrada;
	int i;
	for(i=0; i<config->entradasCache; i++){
		list_remove_by_condition(cache_lru, (void *) nro_es_igual_a);
	}
	list_add_in_index(cache_lru, 0, nro_entrada);
}

void liberar_proceso_cache(int pid){
	int i;
	for(i = 0; i < config->entradasCache; i++){
		if(admin_cache[i].pid == pid)
			liberar_pagina_cacheada(i);
	}
}

//Si la página de ese proceso está en caché, devuelve su posición. Si no está (caché miss), devuelve -1.
int entrada_en_cache(int pid, int nro_pagina){
	for(ent_cache_aux=0; ent_cache_aux<config->entradasCache; ent_cache_aux++){
		if(admin_cache[ent_cache_aux].pid == pid && admin_cache[ent_cache_aux].nro_pagina == nro_pagina){
			//actualiza ya el lru
			//remover de la lista el nro que sea igual a i
			//cache_lru es una lista de int
			list_remove_by_condition(cache_lru, (void *) nro_es_igual_a);

			list_add(cache_lru, ent_cache_aux);
			return ent_cache_aux;
		}
	}
	return -1;
}

bool nro_es_igual_a(void * elemento){
	int entrada = elemento;
	return entrada == ent_cache_aux;
}

void inicializar_cache_lru(){
	cache_lru = list_create();
	if(cache_lru == NULL){
			//mensaje de error
	}
	int i;
	for(i=0; i<config->entradasCache; i++){
		list_add(cache_lru, i);
	}
}

bool puede_estar_en_cache(int pid){
	int entradas = 0, i;
	for(i=0; i<config->entradasCache; i++){
		if(admin_cache[i].pid == pid)
			entradas++;
	}
	if(entradas < config->cacheXProc)
		return true;
	int lru = list_get(cache_lru,0);
	if(entradas==config->cacheXProc && pid==admin_cache[lru].pid)
		return true;
	return false;
}

int * posiciones_entradas_cache_x_proceso(int pid){
	int * posiciones =(int*) malloc(config->cacheXProc);
	int i, j=0;
	for(i=0; i<config->entradasCache || j<config->cacheXProc; i++){
			if(admin_cache[i].pid == pid){
				posiciones[j]=i;
				j++;
			}
	}
	return posiciones;
}

int posicion_lru_cache_x_proceso(int pid){
	int * posiciones = posiciones_entradas_cache_x_proceso(pid);
	int i,j,pos_lru;
	for(i=0; i<config->entradasCache; i++){
		pos_lru = list_get(cache_lru,i);
		for(j=0; j<config->cacheXProc; j++){
			if(posiciones[j]==pos_lru){
				//actualiza cache_lru
				list_remove(cache_lru, i);
				list_add(cache_lru, pos_lru);
				//////
				free(posiciones);
				return pos_lru;
			}
		}
	}
	free(posiciones);
	return -1;
}

//Funciones Consola
void consola(){

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

void consola_comando_help(){
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

void consola_comando_retardo(){
	printf("--> Retardo actual [ms]: %d\n", config->retardoMemoria);
	printf("--> Retardo nuevo [ms]: ");
	scanf("%d", &config->retardoMemoria);
}

void consola_comando_dump_cache(){
	int i;
	void * pag_aux = malloc(config->marcosSize);
	for(i=0; i<config->entradasCache; i++){
		imprimir_y_grabar_en_archivo("PID: %d\n", admin_cache[i].pid);
		imprimir_y_grabar_en_archivo("NRO_PAGINA: %d\n", admin_cache[i].nro_pagina);
		memcpy(pag_aux, admin_cache[i].contenido_pagina, config->marcosSize);
		imprimir_y_grabar_en_archivo("CONTENIDO_PAGINA: %s\n\n", pag_aux);
	}
	free(pag_aux);
}

void consola_comando_dump_estructuras_memoria(){
	imprimir_y_grabar_en_archivo("Tabla de páginas: \n");
	imprimir_y_grabar_en_archivo("N° Marco \t PID\t N° Página\n");
	int i;
	for(i=0; i<config->marcos; i++){
		imprimir_y_grabar_en_archivo("%d\t\t %d\t %d\t\n", tabla_paginas[i].nro_marco, tabla_paginas[i].pid, tabla_paginas[i].nro_pagina);
	}
	imprimir_y_grabar_en_archivo("PROGRAMAS:\n");
	imprimir_y_grabar_en_archivo("PID \t CANTIDAD PÁGINAS\n");
	for(i=0; i<cant_programas; i++){
		if(admin_programas[i].pid != -1){
			imprimir_y_grabar_en_archivo("%d\t %d\n", admin_programas[i].pid, admin_programas[i].cantidad_paginas);
		}
	}
}

//ARREGLAR
void consola_comando_dump_contenido_memoria(){
	char opcion;
	printf("¿Todos los procesos?: S/N");
	scanf("%c", &opcion);
	if(opcion == 'N' || opcion == 'n'){
		int pid, i;
		imprimir_y_grabar_en_archivo("PID: ");
		scanf("%d", &pid);
		imprimir_y_grabar_en_archivo("%d\n", pid);
		int cant_paginas = get_cantidad_paginas(pid);
		if(cant_paginas == -1){
			imprimir_y_grabar_en_archivo("Este proceso no se encuentra en memoria.\n");
		} else{
			for(i=0; i<cant_paginas; i++){
				imprimir_y_grabar_en_archivo("%s", solicitar_bytes_de_una_pagina(memoria, i, 0, config->marcosSize));
			}
		}
	}
	if(opcion == 'S' || opcion == 's'){
		imprimir_y_grabar_en_archivo("%s", memoria);
	} else {
		printf("Comando No Válido");
	}
}

void consola_comando_flush(){
	int i;
	for(i = 0; i < config->entradasCache; i++){
			admin_cache[i].pid = -1;
			admin_cache[i].nro_pagina = -1;
	}
	memset(cache, '0', config->entradasCache * config->marcosSize);
}

void consola_comando_size_memory(){
	int pags_disponibles = paginas_disponibles();
	printf("Tamaño de la memoria:\n");
	printf("\t- Cantidad de frames: %d\n", config->marcos);
	printf("\t- Cantidad de frames ocupados: %d\n", config->marcos - pags_disponibles);
	printf("\t- Cantidad de frames libres: %d\n", pags_disponibles);
}

void consola_comando_size_pid(){
	int pid, i=0;
	printf("PID: ");
	scanf("%d", &pid);
	while(admin_programas[i].pid != pid){
		i++;
	}
	printf("Tamaño total del proceso: %d páginas.", admin_programas[i].cantidad_paginas);
}

//hacer una con mas dispersion
//tener en cuenta el n de las estructuras administrativas
int funcion_hash(int pid, int nro_pagina){
	int frame, cuadrado, modulo;
	cuadrado = pow(pid+4, 2);
	modulo = cuadrado % config->marcos;
	if(modulo==0)
		frame = cuadrado + nro_pagina;
	else
		frame = modulo + nro_pagina;
	return frame;
}

void imprimir_y_grabar_en_archivo(const char* mensaje, ...){
	char * aux;
	va_list argumentos;
	va_start(argumentos, mensaje);
	aux = string_from_vformat(mensaje, argumentos);

	printf("%s", aux);

	fprintf(archivo_disco_memoria, "%s", aux);
	fflush(archivo_disco_memoria);

	va_end(argumentos);
	if(aux != NULL)
		free(aux);
}
