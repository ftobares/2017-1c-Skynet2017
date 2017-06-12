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

#define INICIAR "iniciar_programa"
#define FINALIZAR "finalizar_programa"
#define DESCONECTAR "desconectar_consola"
#define LIMPIAR "limpiar_mensajes"
#define SALIR "exit"
#define OK "1"

/* Variables Globales */
t_console_config* config;
t_socket server_socket;
sem_t sem_tipo_operacion;
sem_t sem_hay_instruccion;
sem_t sem_primera_ejecucion;
sem_t sem_path_archivo;
int tipo_operacion = 0;
char* path_archivo_a_enviar;
char* pid_proceso;
int primera_ejecucion = 1;

/* Declaracion de funciones */
void connect_to_kernel();
int hilo_lector_interfaz_usuario();
void hilo_manager_programa();
void hilo_procesar_operacion(void* tipo_operacion);
int hilo_escritor_interfaz_usuario();
int enviar_codigo();
t_programa_ansisop* crear_y_cargar_archivo(char* path);

int main(int argc, char* argv) {

	printf("Inicio MAIN \n");

	/* Definir hilo interfaz usuario  */
	pthread_t thread_usuario_id;
	pthread_attr_t thread_usuario_attr;

	/* Definir hilo programa  */
	pthread_t thread_programa_id;
	pthread_attr_t thread_programa_attr;

	/* Inicializo semaforos  */
//	sem_init(&sem_tipo_operacion, 0, 1);
//	sem_init(&sem_hay_instruccion, 0, 0);
//	sem_init(&sem_primera_ejecucion, 0, 1);

	connect_to_kernel();

	/* Creo un hilo encargado de la lectura de instrucciones (Interfaz Usuario)  */
//	pthread_attr_init(&thread_usuario_attr);
//	pthread_create(&thread_usuario_id, &thread_usuario_attr, &hilo_lector_interfaz_usuario, NULL);

	/* Creo un hilo encargado de manejar los hilos que realizan las operaciones (Programa)  */
//	pthread_attr_init(&thread_programa_attr);
//	pthread_create(&thread_programa_id, &thread_programa_attr, &hilo_manager_programa, NULL);

//	pthread_join(thread_usuario_id, NULL);
//	pthread_join(thread_programa_id, NULL);

//	sem_destroy(&sem_tipo_operacion);
//	sem_destroy(&sem_hay_instruccion);
//	sem_destroy(&sem_primera_ejecucion);

	return 0;
}

void connect_to_kernel(){

	/* leer archivo config */
	printf("Cargo archivo configuracion\n");
	char* file_path;
	file_path = string_new();
	string_append(&file_path, "./src/consola.config");
	config = cargar_configuracion(file_path, TIPO_PROYECTO);

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
		exit(-1);
	}

	connect(server_socket.socket, server_socket.socket_info->ai_addr,
			server_socket.socket_info->ai_addrlen);

	freeaddrinfo(server_socket.socket_info); // No lo necesitamos mas
	printf("CONECTADO AL KERNEL: %d", server_socket.socket);
}

int saludar(int sRemoto) {

	int aux;

	t_handshake handshake_message;
	handshake_message.handshake = string_new();
	string_append(&handshake_message.handshake, "C");
	int size_mensaje = calcular_tamanio_mensaje(&handshake_message, MSJ_HANDSHAKE);

	if(enviar_mensaje(&handshake_message, MSJ_HANDSHAKE, size_mensaje, sRemoto) == 1){
		perror("Fallo el envio del mensaje handshake con Kernel");
	}

	t_buffer buffer;
	buffer = recibir_mensaje(sRemoto);

	if(buffer.header.id_tipo == MSJ_HANDSHAKE){
		realloc(handshake_message.handshake, buffer.header.tamanio);
		memset(&handshake_message, 0, buffer.header.tamanio);
		handshake_message.handshake = deserializar_mensaje(buffer.data, MSJ_HANDSHAKE);
	}else{
		perror("Error al recibir OK del Kernel, tipo mensaje incorrecto");
	}

	if (!(string_starts_with(handshake_message.handshake, OK)))
	{
		printf("ERROR: HANDSHAKE NO FUE EXITOSO \n");
		aux = -1;
	}
	else
		aux = 0;

	free(handshake_message.handshake);
	free(buffer.data);

	return aux;
}

/* FUNCIONES INTERFAZ */
int hilo_lector_interfaz_usuario(){
	printf("Inicio hilo_lector_interfaz_usuario \n");

	char mensaje[INPUTSIZE];
	sem_init(&sem_path_archivo, 0, 1);

	while(true){
		printf("Consola iniciada ingrese la operación que desea realizar, exit para salir:\n");

		/* Espero por ingreso de instrucciones  */
		fgets(mensaje, 100, stdin);

		if(strncmp(mensaje, INICIAR, strlen(INICIAR)) == 0){

			sem_wait(&sem_tipo_operacion);
			tipo_operacion = 1;
			sem_wait(&sem_path_archivo);
			char** str_separado = string_n_split(mensaje, 3, " ");
			path_archivo_a_enviar = str_separado[1];
			printf("El path es %s \n", path_archivo_a_enviar);
			sem_post(&sem_hay_instruccion);
			sem_post(&sem_tipo_operacion);

		} else if (strncmp(mensaje, FINALIZAR, strlen(FINALIZAR)) == 0) {

			sem_wait(&sem_tipo_operacion);
			tipo_operacion = 2;
			sem_wait(&sem_path_archivo);
			char** str_separado = string_n_split(mensaje, 3, " ");
			pid_proceso = str_separado[1];
			printf("El PID del proceso a finalizar es %s \n", pid_proceso);
			sem_post(&sem_hay_instruccion);
			sem_post(&sem_tipo_operacion);

		} else if (strncmp(mensaje, DESCONECTAR, strlen(DESCONECTAR)) == 0){

			sem_wait(&sem_tipo_operacion);
			tipo_operacion = 3;
			sem_wait(&sem_path_archivo);
			char** str_separado = string_n_split(mensaje, 3, " ");
			pid_proceso = str_separado[1];
			printf("El PID del proceso a finalizar es %s \n", pid_proceso);
			sem_post(&sem_hay_instruccion);
			sem_post(&sem_tipo_operacion);

		} else if (strncmp(mensaje, LIMPIAR, strlen(LIMPIAR)) == 0) {
			tipo_operacion = 4;
		} else if (strncmp(mensaje, SALIR, strlen(SALIR)) == 0) {
			return 0;
		} else {
			printf("## ERROR ## Operacion no valida ## \n");
		}
	}

	sem_destroy(&sem_path_archivo);
	printf("Fin hilo_lector_interfaz_usuario \n");
	return 0;
}

int hilo_escritor_interfaz_usuario(void* pid){

	printf("Inicio hilo_escritor_interfaz_usuario \n");

	int v_pid = (int)pid;

	struct timespec tim, tim2;
	tim.tv_sec = 1;
	tim.tv_nsec = 500;

	int i = 0;
	for(i; i <= 50;i+=1){
		if(nanosleep(&tim , &tim2) < 0 ) {
		      printf("Nano sleep system call failed \n");
		}

		printf("Recibiendo datos de proceso pid=%d iteracion n°%i\n",v_pid, i);
	}

	printf("Fin hilo_escritor_interfaz_usuario \n");

	return 0;
}


/* FUNCIONES PROGRAMA */
void hilo_manager_programa() {
	printf("Inicio hilo_manager_programa \n");

	while(true){

		sem_wait(&sem_hay_instruccion);

		switch(tipo_operacion){
		case 1:
			printf("creo hilo iniciar programa \n");

			pthread_t th_operacion_id;
			pthread_attr_t th_operacion_attr;

			pthread_attr_init(&th_operacion_attr);
			pthread_create(&th_operacion_id, &th_operacion_attr, &hilo_procesar_operacion, tipo_operacion);

			break;
		case 2:
			printf("creo hilo finalizar programa\n");

			pthread_attr_init(&th_operacion_attr);
			pthread_create(&th_operacion_id, &th_operacion_attr, &hilo_procesar_operacion, tipo_operacion);

			break;
		case 3:
			printf("creo hilo desconectar consola\n");

			pthread_attr_init(&th_operacion_attr);
			pthread_create(&th_operacion_id, &th_operacion_attr, &hilo_procesar_operacion, tipo_operacion);

			break;
		case 4:
			printf("creo hilo limpiar mensajes\n");

			pthread_attr_init(&th_operacion_attr);
			pthread_create(&th_operacion_id, &th_operacion_attr, &hilo_procesar_operacion, tipo_operacion);

			break;
		default:
			printf("operacion no valida\n");
		}
	}

	printf("Fin hilo_manager_programa \n");
}

void hilo_procesar_operacion(void* tipo_operacion){
	printf("Inicio hilo_procesar_operacion \n");

	int v_tipo_operacion = (int)tipo_operacion;
	switch(v_tipo_operacion){
	case 1:
		printf("hilo_procesar_operacion tipo_operacion=%i\n",v_tipo_operacion);
		pthread_t th_iniciar_proceso_id;
		pthread_attr_t th_iniciar_proceso_attr;
		char* path = path_archivo_a_enviar;
		sem_post(&sem_path_archivo);

		int v_pid = enviar_codigo(server_socket.socket, path);
		if(v_pid == -1){
			perror("Fallo el envio del codigo al Kernel");
		}

		//recibir pid
		printf("recibir pid de proceso\n");

		//crear hilo de espera por mensajes proceso e imprimir por pantalla
		pthread_attr_init(&th_iniciar_proceso_attr);
		pthread_create(&th_iniciar_proceso_id, &th_iniciar_proceso_attr, &hilo_escritor_interfaz_usuario, v_pid);
		break;
	case 2:
		printf("hilo_procesar_operacion tipo_operacion=%i\n",v_tipo_operacion);
		break;
	default:
		printf("hilo_procesar_operacion - operacion invalida\n");
	}
	printf("Fin hilo_procesar_operacion \n");
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

	/* Enviar el programa al Kernel, para crear el proceso*/
	if(enviar_mensaje(archivo, MSJ_PROGRAMA_ANSISOP, size_mensaje, socket) == 1){
		perror("Fallo envio del archivo al Kernel");
	}

	/* Recibir el PID del proceso creado*/
	t_buffer buffer;
	buffer = recibir_mensaje(socket);

	/* Deserializo mensaje */
	if(buffer.header.id_tipo == MSJ_PROGRAMA_ANSISOP){
		archivo->pid = deserializar_mensaje(buffer.data, MSJ_PROGRAMA_ANSISOP);
	}else{
		perror("Error al recibir PID del proceso creado por el Kernel, tipo mensaje incorrecto");
	}

	return archivo->pid;
}
