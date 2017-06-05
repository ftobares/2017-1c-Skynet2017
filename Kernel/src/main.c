#include <stdio.h>
#include <stdlib.h>
#include <time.h> //Solo para numero random BORRAR CUANDO NO SE USE!
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
#include <src/utils_protocolo.h>
#include <parser/metadata_program.h>

#define BACKLOG 30	// Cantidad conexiones maximas
#define PACKAGESIZE 1024	// Size maximo del paquete a enviar
#define TRUE 1
#define FALSE 0
#define CantClientes 30
#define TIPO_PROYECTO 4
#define MSJ_HANDSHAKE 3
#define MSJ_PROGRAMA_ANSISOP 4
#define MSJ_CONFIRMACION "1"
#define MSJ_NEGACION     "0"
#define HANDSHAKE_CPU 'H'
#define HANDSHAKE_CONSOLA 'C'
//Variables Globales
t_kernel_config* config;
t_socket socket_memoria;
t_socket socket_fs;
t_master_socket master_socket;
//Declaracion de funciones
int iniciar_servidor();

typedef struct
{
	int offsetInicio;
	int offsetFin;
} indiceCodigo;

typedef struct {
	t_size etiqueta_size; // Tamaño del mapa serializado de etiquetas
	char* etiqueta;  // La serializacion de las etiquetas
} indiceEtiquetas;

typedef struct
{
	int nroPagina;
	int offset;
	int size;
}	posicionDeMemoria;

typedef struct
{
	t_list* listaDeArgumentos;
	t_list* listaDeVariables;
	int retPos;
	posicionDeMemoria retVar;

} indiceStack;

typedef struct {
	uint32_t PID;
	uint32_t PC;
	uint32_t paginasCodigo;
	uint32_t paginasStack;
	uint32_t cantOperacIO;

	t_size tamanioIndiceDeCodigo;
	t_intructions* indiceDeCodigo;
	indiceEtiquetas indiceDeEtiquetas;
	uint32_t SP;
	t_size tamanioIndiceStack;
	t_list* indiceDeStack;
} t_PCB;


int main(int argc, char** argv) {

	char* file_path; //= malloc(sizeof(char) * 1024);
	file_path = string_new();
	string_append(&file_path, "./src/kernel.config");
	config = cargar_configuracion(file_path, TIPO_PROYECTO);
	int v_valor_retorno = iniciar_servidor();
	return v_valor_retorno;
}

void enviar_respuesta_handshake(int socket, void *buffer) {
	int bytecount;
	t_handshake temp_handshake;
	temp_handshake.handshake = string_new();
	string_append(temp_handshake.handshake, buffer);
	int size_mensaje = calcular_tamanio_mensaje(temp_handshake.handshake, MSJ_HANDSHAKE);

	if(enviar_mensaje(&temp_handshake, MSJ_HANDSHAKE, size_mensaje, socket) == 1){
		perror("No puedo enviar información al/los cliente/s");
	}
}

void inicializarPCB(int pID,char* codigoAnsisop) {
	t_PCB* pcb=malloc(sizeof(t_PCB));
			t_metadata_program* metaProg=metadata_desde_literal(codigoAnsisop);
			pcb->PID=(uint32_t)pID;
			pcb->PC=metaProg->instruccion_inicio;
			pcb->indiceDeCodigo=metaProg->instrucciones_serializado;
			pcb->tamanioIndiceDeCodigo=metaProg->instrucciones_size;
			pcb->indiceDeEtiquetas.etiqueta=metaProg->etiquetas;
			pcb->indiceDeEtiquetas.etiqueta_size=metaProg->etiquetas_size;
			pcb->paginasCodigo=(obtenerCantPags(codigoAnsisop));
			pcb->paginasStack=config->stackSize;
			pcb->cantOperacIO=0;
			pcb->SP=0;
			pcb->tamanioIndiceStack=0;
}


int iniciar_servidor() {
	int opt = TRUE;
	int addrlen, new_socket, cliente_socket[CantClientes], max_clientes =
	CantClientes, actividad, i, /*valorLectura,*/ sd /*, read_size*/;
	t_buffer buffer;
	int max_sd;

	fd_set readfds;

	//char mensaje[PACKAGESIZE];
	//char tipo_mensaje;
	//Conectarse a la Memoria
	socket_memoria = conectar_a_otro_servidor(config->ipMemoria,
			config->puertoMemoria);

	//Conectarse al FileSystem
	socket_fs = conectar_a_otro_servidor(config->ipFileSystem,
			config->puertoFileSystem);
	//inicializar todos los cliente_socket[] a 0 (No chequeado)
	for (i = 0; i < max_clientes; i++) {
		cliente_socket[i] = 0;
	}
	master_socket = servidor_crear_socket_bind_and_listen(config->puertoProg,
			opt, max_clientes);

	addrlen = sizeof(master_socket.socket_info);
	puts("Esperando por conexiones entrantes...");
	while (TRUE) {
		//Limpiar el socket set
		FD_ZERO(&readfds);
		//Agregar el socket maestro al set
		FD_SET(master_socket.socket, &readfds);
		max_sd = master_socket.socket;
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
		if (FD_ISSET(master_socket.socket, &readfds)) {
			if ((new_socket = accept(master_socket.socket,
					(struct sockaddr *) &master_socket.socket_info,
					(socklen_t*) &addrlen)) < 0) {
				perror("accept");
				exit(EXIT_FAILURE);
			}
			//informar usuario de numero de socket
			printf(
					"Nueva conexion, socket fd es %d , ip es : %s , puerto : %d \n",
					new_socket, inet_ntoa(master_socket.socket_info.sin_addr),
					ntohs(master_socket.socket_info.sin_port));
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
			clean_or_init_buffer(&buffer);
			if (FD_ISSET(sd, &readfds)) {
//				valorLectura = recv(sd, mensaje, PACKAGESIZE, 0);
				buffer = recibir_mensaje(sd);

				//Verificar si fue por cierre, y tambien para leer un mensaje entrante
				if (buffer.header.tamanio == 0) {
					//Alguien se desconecto, obtenemos los detalles e imprimimos
					getpeername(sd,
							(struct sockaddr*) &master_socket.socket_info,
							(socklen_t*) &addrlen);
					printf("Host desconectado , ip %s , puerto %d \n",
							inet_ntoa(master_socket.socket_info.sin_addr),
							ntohs(master_socket.socket_info.sin_port));
					//Cerrar el socket y marcar como 0 en la lista para reusar
					close(sd);
					cliente_socket[i] = 0;
				} else if (buffer.header.tamanio > 0) {
					int j;
//					mensaje[valorLectura] = '\0';
//					tipo_mensaje = mensaje[0];
					//printf("MENSAJE: %s\n", mensaje);
					switch (buffer.header.id_tipo) {
					case MSJ_HANDSHAKE:
						printf("Ingreso mensaje handshake\n");
						t_handshake* v_handshake = deserializar_mensaje(buffer.data, buffer.header.id_tipo);
						if(string_contains(v_handshake->handshake,HANDSHAKE_CPU)){
							puts("ENTRE CON LA CPU\n");
							enviar_respuesta_handshake(sd, MSJ_CONFIRMACION);
						}else if(string_contains(v_handshake->handshake,HANDSHAKE_CONSOLA)){
							puts("ENTRE CON LA CONSOLA\n");
							enviar_respuesta_handshake(sd, MSJ_CONFIRMACION);
						}else{
							puts("NEGACION\n");
							enviar_respuesta_handshake(sd, MSJ_NEGACION);
						}
						free(v_handshake);
						break;
					case MSJ_PROGRAMA_ANSISOP:
						printf("Ingreso mensaje programa AnsiSOp\n",buffer.header.id_tipo);
						/* Aca se supone que leo el codigo que me enviaron y creo el proceso*/
						//Deserializo mensaje buffer.data
						//creo el proceso

						t_programa_ansisop programa_ansisop;

						/* Inicio simulacion de valor random para el PID a devolver */
						srand(time(NULL));
						int pid_random = rand();
						programa_ansisop.pid = pid_random;
						/* Fin simulacion */

						int size_mensaje = calcular_tamanio_mensaje(&programa_ansisop, MSJ_PROGRAMA_ANSISOP);

						if(enviar_mensaje(&programa_ansisop, MSJ_PROGRAMA_ANSISOP, size_mensaje, sd) == 1){
							perror("Fallo el envio del PID a la Consola\n");
						}

					break;

					default:
						/* send to everyone! */
//						for (j = 0; j <= max_sd; j++) {
//							// except the listener and ourselves
//							if (cliente_socket[j] != sd
//									&& cliente_socket[j] != 0) {
//								if (send(cliente_socket[j], mensaje,
//										strlen(mensaje), 0) == -1) {
//									perror("send() error!");
//								} else {
//									printf(
//											"Mensaje enviado con el socket %d \n",
//											cliente_socket[j]);
//								}
//							}
//						}
//						if (send(socket_memoria.socket, mensaje,
//								strlen(mensaje), 0) != strlen(mensaje)) {
//							perror("send memoria failed");
//						}
//						if (send(socket_fs.socket, mensaje, strlen(mensaje), 0)
//								!= strlen(mensaje)) {
//							perror("send filesystem failed");
//						}
						printf("valor lectura: %d\n", buffer.header.id_tipo);
						puts(
								"mensajes a memoria y filesystem enviados correctamente\n");
						break;
					}

				} else {
					printf("No hay datos - recv error \n");
				}
			}
		}
	}
	return 0;
}


