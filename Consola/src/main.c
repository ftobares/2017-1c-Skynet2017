#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>
#include <commons/string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <src/utils_config.h>
#include <src/utils_socket.h>
#include <src/utils_protocolo.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>
#include <sys/stat.h>

#define PACKAGESIZE 1024
#define INPUTSIZE 100
#define TIPO_PROYECTO 1
#define BUFFERSIZE 40000

#define MSJ_HEADER 1
#define MSJ_HANDSHAKE 2
#define MSJ_PROGRAMA_ANSISOP 3
#define MSJ_PCB 4
#define MSJ_FINALIZAR_PROGRAMA 5

#define OPERACION_INICIAR 1
#define OPERACION_FINALIZAR 2
#define OPERACION_DESCONECTAR 3
#define OPERACION_LIMPIAR 4
#define OPERACION_SALIR 5

#define INICIAR "iniciar_programa"
#define FINALIZAR "finalizar_programa"
#define DESCONECTAR "desconectar_consola"
#define LIMPIAR "limpiar_mensajes"
#define SALIR "exit"
#define OK "1"
#define DEBUG "debug"

/* Variables Globales */
t_console_config* config;
t_socket server_socket;
sem_t sem_tipo_operacion;
sem_t sem_hay_instruccion;
sem_t sem_path_archivo;
sem_t sem_pid_proceso;
sem_t sem_lista_procesos;
int tipo_operacion = 0;
char* path_archivo_a_enviar;
int pid_proceso;
t_list* lista_procesos;

/* Declaracion de funciones */
void connect_to_kernel();
void disconnect_kernel();
void clean_destroy_close();
void hilo_lector_interfaz_usuario();
void hilo_manager_programa();
void hilo_procesar_operacion(void* tipo_operacion);
int hilo_escritor_interfaz_usuario();
int enviar_codigo();
bool es_igual_a(void* elemento);
t_programa_ansisop* crear_y_cargar_archivo(char* path);
void finalizar_proceso(int p_pid);

int main(int argc, char* argv) {

	printf("Inicio MAIN \n");

	/* Definir hilo interfaz usuario  */
	pthread_t thread_usuario_id;
	pthread_attr_t thread_usuario_attr;

	/* Definir hilo programa  */
	pthread_t thread_programa_id;
	pthread_attr_t thread_programa_attr;

	/* Inicializo semaforos  */
	sem_init(&sem_tipo_operacion, 0, 1);
	sem_init(&sem_hay_instruccion, 0, 0);

	/* Creo un hilo encargado de manejar los hilos que realizan las operaciones (Programa)  */
	pthread_attr_init(&thread_programa_attr);
	pthread_create(&thread_programa_id, &thread_programa_attr, &hilo_manager_programa, NULL);

	/* Creo un hilo encargado de la lectura de instrucciones (Interfaz Usuario)  */
	pthread_attr_init(&thread_usuario_attr);
	pthread_create(&thread_usuario_id, &thread_usuario_attr, &hilo_lector_interfaz_usuario, NULL);

	//Hace falta??
	pthread_join(thread_usuario_id, NULL);
	pthread_join(thread_programa_id, NULL);

	return 0;
}

/* Hilo encargado de la interfaz, captura comandos
 * y mediante semaforos activa los flags de instruccion y tipo de operacion */
void hilo_lector_interfaz_usuario(){
	printf("Inicio hilo_lector_interfaz_usuario \n");

	char mensaje[INPUTSIZE];
	lista_procesos = list_create();
	sem_init(&sem_path_archivo, 0, 1);
	sem_init(&sem_pid_proceso, 0, 1);
	sem_init(&sem_lista_procesos, 0, 1);

	while(true){
		printf("Consola iniciada ingrese la operación que desea realizar, exit para salir:\n");

		/* Espero por ingreso de instrucciones  */
		fgets(mensaje, 100, stdin);

		if(strncmp(mensaje, INICIAR, strlen(INICIAR)) == 0){

			/* Inicio un nuevo programa pasandole el path del codigo */
			sem_wait(&sem_tipo_operacion);
			tipo_operacion = OPERACION_INICIAR;
			sem_wait(&sem_path_archivo);
			char** str_separado = string_n_split(mensaje, 3, " ");
			path_archivo_a_enviar = str_separado[1];
			path_archivo_a_enviar[strlen(path_archivo_a_enviar)-1]=0;
			printf("El path es %s \n", path_archivo_a_enviar);
			free(str_separado);
			sem_post(&sem_hay_instruccion);
			sem_post(&sem_tipo_operacion);

		} else if (strncmp(mensaje, FINALIZAR, strlen(FINALIZAR)) == 0) {

			/* Finalizo un nuevo programa pasandole el PID del proceso */
			sem_wait(&sem_tipo_operacion);
			tipo_operacion = OPERACION_FINALIZAR;
			sem_wait(&sem_pid_proceso);
			char** str_separado = string_n_split(mensaje, 3, " ");
			char* temp_string = str_separado[1];
			temp_string[strlen(temp_string)-1]=0;
			pid_proceso = atoi(temp_string);
			printf("El PID del proceso a finalizar es %i \n", pid_proceso);
			free(str_separado);
			sem_post(&sem_hay_instruccion);
			sem_post(&sem_tipo_operacion);

		} else if (strncmp(mensaje, DESCONECTAR, strlen(DESCONECTAR)) == 0){

			/* Desconecto la consola del Kernel.
			 * Finalizo procesos que estan en ejecucion*/
			sem_wait(&sem_tipo_operacion);
			tipo_operacion = OPERACION_DESCONECTAR;
			printf("Desconecto consola del kernel \n");
			list_iterate(lista_procesos, finalizar_proceso);
			disconnect_kernel();
			sem_post(&sem_hay_instruccion);
			sem_post(&sem_tipo_operacion);

		} else if (strncmp(mensaje, LIMPIAR, strlen(LIMPIAR)) == 0) {

			/* Limpio la consola -NO ANDA */
			sem_wait(&sem_tipo_operacion);
			printf("Limpio la consola del kernel \n");
			tipo_operacion = OPERACION_LIMPIAR;
			sem_post(&sem_hay_instruccion);
			sem_post(&sem_tipo_operacion);

		} else if (strncmp(mensaje, SALIR, strlen(SALIR)) == 0) {

			/* Cierro la consola.
			 * Finalizo procesos en ejecucion.
			 * Desconecto del Kernel y libero memoria asignada */
			printf("Desconecto y cierro consola. \n");
			list_iterate(lista_procesos, finalizar_proceso);
			clean_destroy_close();
		} else if (strncmp(mensaje, DEBUG, strlen(DEBUG)) == 0) {
			/* Para debuggear, lo uso para imprimir
			 * por pantalla algun valor en especial.
			 * Ej: La lista de procesos en ejecucion */
			printf("Entrada para debug\n");
			void imprimir_elementos(void* elemento){
				int element = elemento;
				printf("PID: %d\n",element);
			}
			list_iterate(lista_procesos, imprimir_elementos);
		} else {
			printf("## ERROR ## Operacion no valida ## \n");
		}
	}
}

/* Hilo encargado de imprimir por pantalla
 * la informacion que mande el proceso asignado */
int hilo_escritor_interfaz_usuario(void* pid){

	printf("Inicio hilo_escritor_interfaz_usuario \n");

	int v_pid = (int)pid;
	char* v_s_pid = string_itoa(v_pid);

	/* SIMULO RECIBIR DATOS DEL PROCESO CREADO POR EL KERNEL */
	struct timespec tim, tim2;
	tim.tv_sec = 5;
	tim.tv_nsec = 500;

	int i = 0;
	for(i; i < 50;i+=1){

		printf("Recibiendo datos de proceso pid=%d iteracion n°%i\n",v_pid, i);

		if(nanosleep(&tim , &tim2) < 0 ) {
		      printf("Nano sleep system call failed \n");
		}

//		if(string_equals_ignore_case(v_s_pid,pid_proceso) == true){
//			printf("Finalizar proceso pid=%d\n",v_pid);
//			break;
//		}
	}

	printf("Fin hilo_escritor_interfaz_usuario PID %i\n", v_pid);

	return 0;
}

/* Hilo encargado de ejecutar las
 * operaciones indicadas por el hilo interfaz.
 * Segun la operacion, general threads nuevos
 * para no generar demoras */
void hilo_manager_programa() {
	printf("Inicio hilo_manager_programa \n");

	connect_to_kernel();

	while(true){

		sem_wait(&sem_hay_instruccion);

		switch(tipo_operacion){
		case OPERACION_INICIAR:
			printf("creo hilo iniciar programa \n");

			pthread_t th_operacion_id;
			pthread_attr_t th_operacion_attr;

			pthread_attr_init(&th_operacion_attr);
			pthread_create(&th_operacion_id, &th_operacion_attr, &hilo_procesar_operacion, tipo_operacion);

			break;
		case OPERACION_FINALIZAR:
			printf("creo hilo finalizar programa\n");

			pthread_t th_operacion_id_finalizar;
			pthread_attr_t th_operacion_attr_finalizar;

			pthread_attr_init(&th_operacion_attr_finalizar);
			pthread_create(&th_operacion_id_finalizar, &th_operacion_attr_finalizar, &hilo_procesar_operacion, tipo_operacion);

			break;
		case OPERACION_LIMPIAR:
			printf("Limpio mensajes de la consola\n");

			fflush(stdout);
			fflush(stdin);

			break;
		default:
			printf("operacion no valida\n");
		}
	}
}

/* Funcion encargada de decirle al Kernel
 * que finalize un proceso */
void finalizar_proceso(int p_pid){
	t_programa_ansisop* archivo = malloc(sizeof(t_programa_ansisop));
	archivo->pid = p_pid;
	int size_mensaje = calcular_tamanio_mensaje(archivo, MSJ_FINALIZAR_PROGRAMA);

	t_buffer* buffer_to_send = serializar_mensajes(archivo, MSJ_FINALIZAR_PROGRAMA, size_mensaje, server_socket.socket);

	/* Enviar el pid al Kernel, para que lo finalize */
	if(enviar_mensaje(buffer_to_send) == 1){
		perror("Fallo el envio de solicitud de finalizar programa al Kernel\n");
	}else{
		/* Quito de la lista el proceso finalizado */
		sem_wait(&sem_lista_procesos);
		bool es_proceso_finalizado(void* element){
			int elemento = element;
			if(elemento == p_pid){
				return true;
			}else{
				return false;
			}
		}
		int elemento_eliminado = list_remove_by_condition(lista_procesos, (void*)es_proceso_finalizado);
		sem_post(&sem_lista_procesos);
	}

	//Libero memoria
	free(archivo);
	free(buffer_to_send->data);
	free(buffer_to_send);
}

/* Hilo para procesar algunas operaciones que pueden
 * generar demoras */
void hilo_procesar_operacion(void* tipo_operacion){
	printf("Inicio hilo_procesar_operacion \n");

	int v_tipo_operacion = (int)tipo_operacion;
	switch(v_tipo_operacion){
	case OPERACION_INICIAR:
		printf("hilo_procesar_operacion tipo_operacion=%i\n",v_tipo_operacion);
		pthread_t th_iniciar_proceso_id;
		pthread_attr_t th_iniciar_proceso_attr;
		char* path = string_new();
		string_append(&path,path_archivo_a_enviar);
		sem_post(&sem_path_archivo);

		int v_pid = enviar_codigo(server_socket.socket, path);
		free(path);
		if(v_pid == -1){
			perror("Fallo el envio del codigo al Kernel");
			break;
		}

		/* Agrego el PID del proceso creado a la lista */
		sem_wait(&sem_lista_procesos);
		list_add(lista_procesos, v_pid);
		sem_post(&sem_lista_procesos);

		//crear hilo de espera por mensajes proceso e imprimir por pantalla
		pthread_attr_init(&th_iniciar_proceso_attr);
		pthread_create(&th_iniciar_proceso_id, &th_iniciar_proceso_attr, &hilo_escritor_interfaz_usuario, v_pid);
		break;
	case OPERACION_FINALIZAR:
		printf("hilo_procesar_operacion tipo_operacion=%i\n",v_tipo_operacion);
		int local_pid = pid_proceso;
		sem_post(&sem_pid_proceso);

		finalizar_proceso(local_pid);

		break;
	default:
		printf("hilo_procesar_operacion - operacion invalida\n");
	}
	printf("Fin hilo_procesar_operacion \n");
}

void clean_destroy_close(){
	disconnect_kernel();
	sem_destroy(&sem_path_archivo);
	sem_destroy(&sem_pid_proceso);
	sem_destroy(&sem_lista_procesos);
	sem_destroy(&sem_tipo_operacion);
	sem_destroy(&sem_hay_instruccion);
	free(path_archivo_a_enviar);
	list_destroy(lista_procesos);
	printf("Fin hilo_lector_interfaz_usuario \n");
	printf("Fin hilo_manager_programa \n");
	printf("Fin MAIN \n");
	exit(EXIT_SUCCESS);
}

void disconnect_kernel(){
	freeaddrinfo(server_socket.socket_info);
	close(server_socket.socket);
}

void connect_to_kernel(){

	/* leer archivo config */
	printf("Cargo archivo configuracion\n");
	char* file_path;
	file_path = string_new();
	string_append(&file_path, "./src/consola.config");
	config = cargar_configuracion(file_path, TIPO_PROYECTO);
	free(file_path);

	/* Handshake */
	printf("Hacer handshake con Kernel\n");
	server_socket = cliente_crear_socket(config->ip_kernel,config->puerto_kernel);

	/* Conectar al Kernel  */
	int AUX_CONEC_KER = connect(server_socket.socket,
			server_socket.socket_info->ai_addr,
			server_socket.socket_info->ai_addrlen);

	if (AUX_CONEC_KER < 0) {
		printf("Error en connect CONSOLA -> KERNEL\n");
		exit(-1);
	}

	/* HandShake CONSOLA -> KERNEL  */
	if (AUX_CONEC_KER == saludar(server_socket.socket)) {
		printf("Handshake Exitoso! \n");
	}else{
		disconnect_kernel();
		exit(-1);
	}

	connect(server_socket.socket, server_socket.socket_info->ai_addr,
			server_socket.socket_info->ai_addrlen);

	printf("CONECTADO AL KERNEL: %d\n", server_socket.socket);
}

int saludar(int sRemoto) {

	printf("Inicio saludar\n");

	t_handshake mensaje_a_enviar;
	mensaje_a_enviar.handshake = string_new();
	string_append(&mensaje_a_enviar.handshake, "C");
	int size_mensaje = calcular_tamanio_mensaje(&mensaje_a_enviar, MSJ_HANDSHAKE);

	t_buffer* buffer_to_send = serializar_mensajes(&mensaje_a_enviar, MSJ_HANDSHAKE, size_mensaje, sRemoto);

	if(enviar_mensaje(buffer_to_send) == 1){
		perror("Fallo el envio del mensaje handshake con Kernel\n");
	}

	//destroy buffer
	free(mensaje_a_enviar.handshake);
	free(buffer_to_send->data);
	free(buffer_to_send);

	t_buffer buffer_to_rcv;
	t_handshake* mensaje_retorno;
	buffer_to_rcv = recibir_mensaje(sRemoto);

	if(buffer_to_rcv.header.id_tipo == MSJ_HANDSHAKE){
		mensaje_retorno = deserializar_mensaje(buffer_to_rcv.data, MSJ_HANDSHAKE);
	}else{
		perror("Error al recibir OK del Kernel, tipo mensaje incorrecto\n");
		return -1;
	}

	if (!(string_starts_with(mensaje_retorno->handshake, OK)))
	{
		printf("ERROR: HANDSHAKE NO FUE EXITOSO \n");
		return -1;
	}

	free(mensaje_retorno->handshake);
	free(mensaje_retorno);
	free(buffer_to_rcv.data);

	printf("Fin saludar\n");

	return 0;
}

t_programa_ansisop* crear_y_cargar_archivo(char* path){
	FILE* file = fopen(path, "r");

	if (file == NULL) {
		return NULL;
	}

	struct stat stat_file;
	stat(path, &stat_file);

	t_programa_ansisop* archivo = malloc(sizeof(t_programa_ansisop));

	archivo->contenido = calloc(1, stat_file.st_size + 1);

	fread(archivo->contenido, stat_file.st_size, 1, file);

	if (strstr(archivo->contenido, "\r\n")) {
		printf("\n\crear_y_cargar_archivo - WARNING: the file %s contains a \\r\\n sequence "
		 "- the Windows new line sequence. The \\r characters will remain as part "
		 "of the value, as Unix newlines consist of a single \\n. You can install "
		 "and use `dos2unix` program to convert your files to Unix style.\n\n", path);
	}

	fclose(file);

	return archivo;
}

int enviar_codigo(int socket, char* path){

	/* Crear el archivo y cargarlo con el programa*/
	t_programa_ansisop* archivo = crear_y_cargar_archivo(path);
	archivo->pid = 0;
	int size_mensaje = calcular_tamanio_mensaje(archivo, MSJ_PROGRAMA_ANSISOP);

	t_buffer* buffer_to_send = serializar_mensajes(archivo, MSJ_PROGRAMA_ANSISOP, size_mensaje, socket);

	/* Enviar el programa al Kernel, para crear el proceso*/
	if(enviar_mensaje(buffer_to_send) == 1){
		perror("Fallo envio del archivo al Kernel\n");
		return -1;
	}

	//Libero memoria
	free(archivo->contenido);
	free(archivo);
	free(buffer_to_send->data);
	free(buffer_to_send);

	/* Recibir el PID del proceso creado*/
	t_buffer buffer;
	buffer = recibir_mensaje(socket);

	/* Deserializo mensaje */
	if(buffer.header.id_tipo == MSJ_PROGRAMA_ANSISOP){
		archivo = deserializar_mensaje(buffer.data, MSJ_PROGRAMA_ANSISOP);
	}else{
		perror("Error al recibir PID del proceso creado por el Kernel, tipo mensaje incorrecto\n");
		return -1;
	}
	free(archivo->contenido);//Libero porque no lo uso... analizar si conviene dejarlo por si acaso y liberar luego.

	return archivo->pid;
}
