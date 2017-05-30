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
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

#define PACKAGESIZE 1024
#define TIPO_PROYECTO 1
#define BUFFERSIZE 40000
#define HSKERNEL     2 //HANDSHAKE
#define TOCPUKERNEL  3 //TIPO
#define OK 	"1"
#define INICIAR "iniciar_programa"
#define FINALIZAR "finalizar_programa"
#define DESCONECTAR "desconectar_consola"
#define LIMPIAR "limpiar_mensajes"
#define SALIR "exit"

//Variables Globales
t_console_config* config;
//t_socket server_socket;
//int CONECTADOALKERNEL = 0;
sem_t sem_tipo_operacion;
sem_t sem_hay_instruccion;
sem_t sem_buffer;
sem_t sem_primera_ejecucion;
int tipo_operacion = 0;
char* path_archivo_a_enviar;
int primera_ejecucion = 1;

//Declaracion de funciones
void conectar_al_kernel();
int hilo_lector_interfaz_usuario();
void hilo_manager_programa();
void hilo_procesar_operacion(void* tipo_operacion);
int hilo_escritor_interfaz_usuario();

int main(int argc, char* argv) {

	printf("Inicio MAIN \n");

	//Definir hilo interfaz usuario
	pthread_t thread_usuario_id;
	pthread_attr_t thread_usuario_attr;

	//Definir hilo programa
	pthread_t thread_programa_id;
	pthread_attr_t thread_programa_attr;

	sem_init(&sem_tipo_operacion, 0, 1);
	sem_init(&sem_hay_instruccion, 0, 0);
	sem_init(&sem_buffer, 0, 1);
	sem_init(&sem_primera_ejecucion, 0, 1);
	printf("###DEBUG# sem_tipo_operacion %d \n",sem_tipo_operacion.__align);
	printf("###DEBUG# sem_hay_instruccion %d \n",sem_hay_instruccion.__align);
	printf("###DEBUG# sem_buffer %d \n",sem_buffer.__align);

	//Creo un hilo encargado de la lectura de instrucciones (Interfaz Usuario)
	pthread_attr_init(&thread_usuario_attr);
	pthread_create(&thread_usuario_id, &thread_usuario_attr, &hilo_lector_interfaz_usuario, NULL);

	//Creo un hilo encargado de manejar los hilos que realizan las operaciones (Programa)
	pthread_attr_init(&thread_programa_attr);
	pthread_create(&thread_programa_id, &thread_programa_attr, &hilo_manager_programa, NULL);

	pthread_join(thread_usuario_id, NULL);
	pthread_join(thread_programa_id, NULL);

	printf("###DEBUG# sem_tipo_operacion %d \n",sem_tipo_operacion.__align);
	printf("###DEBUG# sem_hay_instruccion %d \n",sem_hay_instruccion.__align);
	printf("###DEBUG# sem_buffer %d \n",sem_buffer.__align);

	sem_destroy(&sem_tipo_operacion);
	sem_destroy(&sem_hay_instruccion);
	sem_destroy(&sem_buffer);
	sem_destroy(&sem_primera_ejecucion);

	return 0;
}

/* FUNCIONES INTERFAZ */
int hilo_lector_interfaz_usuario(){
	printf("Inicio hilo_lector_interfaz_usuario \n");
//	printf("###DEBUG# sem_tipo_operacion %d \n",sem_tipo_operacion.__align);
//	printf("###DEBUG# sem_hay_instruccion %d \n",sem_hay_instruccion.__align);
//	printf("###DEBUG# sem_buffer %d \n",sem_buffer.__align);
	char mensaje[100];

	while(true){
		printf("Consola iniciada ingrese la operación que desea realizar, exit para salir:\n");

		//Espero por ingreso de instrucciones
		fgets(mensaje, 100, stdin);

		if(strncmp(mensaje, INICIAR, strlen(INICIAR)) == 0){

			printf("###DEBUG# sem_tipo_operacion %d \n",sem_tipo_operacion.__align);
			printf("###DEBUG# sem_hay_instruccion %d \n",sem_hay_instruccion.__align);
			printf("###DEBUG# sem_buffer %d \n",sem_buffer.__align);

			sem_wait(&sem_tipo_operacion);
			tipo_operacion = 1;
			printf("###DEBUG# sem_tipo_operacion %d \n",sem_tipo_operacion.__align);
			printf("###DEBUG# sem_hay_instruccion %d \n",sem_hay_instruccion.__align);
			printf("###DEBUG# sem_buffer %d \n",sem_buffer.__align);
			char** str_separado = string_n_split(mensaje, 3, " ");
			path_archivo_a_enviar = str_separado[1];
			printf("El path es %s \n", path_archivo_a_enviar);
			sem_post(&sem_hay_instruccion);
			sem_post(&sem_tipo_operacion);
			printf("###DEBUG# sem_tipo_operacion %d \n",sem_tipo_operacion.__align);
			printf("###DEBUG# sem_hay_instruccion %d \n",sem_hay_instruccion.__align);
			printf("###DEBUG# sem_buffer %d \n",sem_buffer.__align);

		} else if (strncmp(mensaje, FINALIZAR, strlen(FINALIZAR)) == 0) {
			tipo_operacion = 2;
		} else if (strncmp(mensaje, DESCONECTAR, strlen(DESCONECTAR)) == 0){
			tipo_operacion = 3;
		} else if (strncmp(mensaje, LIMPIAR, strlen(LIMPIAR)) == 0) {
			tipo_operacion = 4;
		} else if (strncmp(mensaje, SALIR, strlen(SALIR)) == 0) {
			return 0;
		} else {
			return 1;
		}
	}
	printf("Fin hilo_lector_interfaz_usuario \n");
	return 0;
}

int hilo_escritor_interfaz_usuario(){

	printf("Inicio hilo_escritor_interfaz_usuario \n");

	sem_wait(&sem_buffer);
	struct timespec tim, tim2;
	tim.tv_sec = 1;
	tim.tv_nsec = 500;

	int i = 0;
	for(i; i <= 50;i+=1){
		if(nanosleep(&tim , &tim2) < 0 ) {
		      printf("Nano sleep system call failed \n");
		}
//		printf("Recibiendo datos de proceso %i\n",i);
	}
	sem_post(&sem_buffer);

	printf("Fin hilo_escritor_interfaz_usuario \n");

	return 0;
}


/* FUNCIONES PROGRAMA */
void hilo_manager_programa() {
	printf("Inicio hilo_manager_programa \n");

	while(true){

		sem_wait(&sem_hay_instruccion);
		printf("###DEBUG# sem_hay_instruccion %d \n",sem_hay_instruccion.__align);

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
		t_socket server_socket;

		if(primera_ejecucion == 1){
			primera_ejecucion = 0; //Analizar si hace falta un semaforo (ya definido como sem_primera_ejecucion

			//leer archivo config
			printf("Cargo archivo configuracion\n");
			char* file_path;
			file_path = string_new();
			string_append(&file_path, "./src/consola.config");
			config = cargar_configuracion(file_path, TIPO_PROYECTO);
		}

		//handshake
		printf("Hacer handshake con Kernel\n");
		server_socket = cliente_crear_socket(config->ip_kernel,config->puerto_kernel);

		int AUX_CONEC_KER = connect(server_socket.socket,
				server_socket.socket_info->ai_addr,
				server_socket.socket_info->ai_addrlen);

		if (AUX_CONEC_KER < 0) {
			printf("Error en connect CONSOLA -> KERNEL\n");
			exit(-1);
		}

		//HandShake CONSOLA -> KERNEL
		if (AUX_CONEC_KER == saludar(HSKERNEL, TOCPUKERNEL, server_socket.socket)) {
			printf("Handshake Exitoso! \n");
		}

		//enviar mensaje
		printf("send mensaje a Kernel\n");
		connect(server_socket.socket, server_socket.socket_info->ai_addr,
				server_socket.socket_info->ai_addrlen);

		freeaddrinfo(server_socket.socket_info); // No lo necesitamos mas
//		printf("CONECTADO AL KERNEL: %d", CONECTADOALKERNEL);

//		if (CONECTADOALKERNEL) {
//			int enviar = 1;
			char message[23] = "hilo_procesar_operacion";
//			printf(
//					"Conectado al servidor. Ya puede enviar mensajes. Escriba 'exit' para salir\n");
//			while (enviar) {
//				fgets(message, PACKAGESIZE, stdin); // Lee una linea en el stdin (lo que escribimos en la consola) hasta encontrar un \n (y lo incluye) o llegar a PACKAGESIZE.
//				if (!strcmp(message, "exit\n"))
//					enviar = 0; // Chequeo que el usuario no quiera salir
//				if (enviar)
					send(server_socket.socket, message, strlen(message) + 1, 0); // Solo envio si el usuario no quiere salir.
//			}
//
//		} else {
//
//			close(server_socket.socket);
//			printf("HANDSHAKE ERROR - No se pudo conectar al KERNEL");
//			return;
//		}

		//recibir pid
		printf("recibir pid de proceso\n");

		//crear hilo de espera por mensajes proceso e imprimir por pantalla
		pthread_attr_init(&th_iniciar_proceso_attr);
		pthread_create(&th_iniciar_proceso_id, &th_iniciar_proceso_attr, &hilo_escritor_interfaz_usuario, NULL);
		break;
	case 2:
		printf("hilo_procesar_operacion tipo_operacion=%i\n",v_tipo_operacion);
		break;
	default:
		printf("hilo_procesar_operacion - operacion invalida\n");
	}
	printf("Fin hilo_procesar_operacion \n");
}

int Enviar(int sRemoto, char * buffer)
{
  int cantBytes;
  cantBytes = send(sRemoto, buffer, strlen(buffer), 0);
  if (cantBytes == -1)
    printf("ERROR ENVIO DATOS.\n");
  return cantBytes;
}

int Recibir(int sRemoto, char * buffer)
{
  int bytecount;
  memset(buffer, 0, BUFFERSIZE);
  if ((bytecount = recv(sRemoto, buffer, BUFFERSIZE, 0)) == -1)
	printf("ERROR RECIBO DATOS. \n");

  return bytecount;
}

int saludar(int handshake, int tipo, int sRemoto) {

	char *respuesta = malloc(BUFFERSIZE * sizeof(char));
	char *mensaje = string_new();
	string_append(&mensaje, "C");
	string_append(&mensaje, string_itoa(handshake));
	string_append(&mensaje, string_itoa(tipo));
	int aux;

	Enviar(sRemoto, mensaje);
	Recibir(sRemoto, respuesta);

	if (!(string_starts_with(respuesta, OK)))
	{
		printf("ERROR: HANDSHAKE NO FUE EXITOSO \n");
	}
	else
		aux = 0;

	if (mensaje != NULL)
		free(mensaje);
	if (respuesta != NULL)
		free(respuesta);

	return aux;
}

//void conectar_al_kernel() {
//
//	server_socket = cliente_crear_socket(config->ip_kernel,
//			config->puerto_kernel);
//
//	int AUX_CONEC_KER = connect(server_socket.socket,
//			server_socket.socket_info->ai_addr,
//			server_socket.socket_info->ai_addrlen);
//
//	if (AUX_CONEC_KER < 0) {
//		printf("Error en connect CONSOLA -> KERNEL\n");
//		exit(-1);
//	}
//
//	//HandShake CONSOLA -> KERNEL
//	if (AUX_CONEC_KER == saludar(HSKERNEL, TOCPUKERNEL, server_socket.socket)) {
//		CONECTADOALKERNEL = 1;
//	}
//
//	connect(server_socket.socket, server_socket.socket_info->ai_addr,
//			server_socket.socket_info->ai_addrlen);
//
//	freeaddrinfo(server_socket.socket_info); // No lo necesitamos mas
//	printf("CONECTADO AL KERNEL: %d", CONECTADOALKERNEL);
//
//	if (CONECTADOALKERNEL) {
//		int enviar = 1;
//		char message[PACKAGESIZE];
//		printf(
//				"Conectado al servidor. Ya puede enviar mensajes. Escriba 'exit' para salir\n");
//		while (enviar) {
//			fgets(message, PACKAGESIZE, stdin); // Lee una linea en el stdin (lo que escribimos en la consola) hasta encontrar un \n (y lo incluye) o llegar a PACKAGESIZE.
//			if (!strcmp(message, "exit\n"))
//				enviar = 0; // Chequeo que el usuario no quiera salir
//			if (enviar)
//				send(server_socket.socket, message, strlen(message) + 1, 0); // Solo envio si el usuario no quiere salir.
//		}
//
//	} else {
//
//		close(server_socket.socket);
//		printf("HANDSHAKE ERROR - No se pudo conectar al KERNEL");
//		return;
//	}
//}
