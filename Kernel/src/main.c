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

#define BACKLOG 30	// Cantidad conexiones maximas
#define PACKAGESIZE 1024	// Size maximo del paquete a enviar
#define TRUE 1
#define FALSE 0
#define CantClientes 30
#define TIPO_PROYECTO 4
#define MSJ_HANDSHAKE_CPU 'H'
#define MSJ_HANDSHAKE_CONSOLA 'C'
#define MSJ_CONFIRMACION "1"
#define MSJ_NEGACION     "0"
#define HANDSHAKE_CPU '1'
#define HANDSHAKE_CONSOLA '2'
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

typedef struct
{
 int etiqueta; //Verificar
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

typedef struct
{
	pid_t pid;
	int programCounter;
	int paginasCodigo;
	indiceCodigo indiceCodigo;
	indiceEtiquetas etiquetas;
	indiceStack stack;
	int exitCode;
} PCB;


int main(int argc, char** argv) {

	char* file_path; //= malloc(sizeof(char) * 1024);
	file_path = string_new();
	string_append(&file_path, "./src/kernel.config");
	config = cargar_configuracion(file_path, TIPO_PROYECTO);
	int v_valor_retorno = iniciar_servidor();
	return v_valor_retorno;
}

int EnviarDatos(int socket, void *buffer) {
	int bytecount;

	if ((bytecount = send(socket, buffer, strlen(buffer), 0)) == -1)
		perror("No puedo enviar información al/los cliente/s");

	return bytecount;
}

void inicializarPCB(PCB* auxPCB) {
	auxPCB->pid = 0;
	auxPCB->programCounter = 0;
	auxPCB->paginasCodigo = 0;
	auxPCB->indiceCodigo.offsetFin = 0;
	auxPCB->indiceCodigo.offsetInicio = 0;
	auxPCB->etiquetas.etiqueta = 0;
	auxPCB->stack.listaDeArgumentos = list_create();
	auxPCB->stack.listaDeVariables = list_create();
	auxPCB->exitCode = 0;
}


int iniciar_servidor() {
	int opt = TRUE;
	int addrlen, new_socket, cliente_socket[CantClientes], max_clientes =
	CantClientes, actividad, i, valorLectura, sd /*, read_size*/;
	int max_sd;

	fd_set readfds;

	char mensaje[PACKAGESIZE];
	char tipo_mensaje;
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
			if (FD_ISSET(sd, &readfds)) {
				valorLectura = recv(sd, mensaje, PACKAGESIZE, 0);

				//Verificar si fue por cierre, y tambien para leer un mensaje entrante
				if (valorLectura == 0) {
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
				}
				if (valorLectura > 0) {
					int j;
					mensaje[valorLectura] = '\0';
					tipo_mensaje = mensaje[0];
					printf("MENSAJE: %s\n", mensaje);
					switch (tipo_mensaje) {
					case MSJ_HANDSHAKE_CPU:
						if(mensaje[1] == HANDSHAKE_CPU)
							EnviarDatos(sd, MSJ_CONFIRMACION);
						else
							EnviarDatos(sd, MSJ_NEGACION);
						break;
					case MSJ_HANDSHAKE_CONSOLA:
						if(mensaje[1] == HANDSHAKE_CONSOLA)
						{
							puts("ENTRE CON LA CONSOLA");
							EnviarDatos(sd, MSJ_CONFIRMACION);
						}
						else
							EnviarDatos(sd, MSJ_NEGACION);
					break;

					default:
						/* send to everyone! */
						for (j = 0; j <= max_sd; j++) {
							// except the listener and ourselves
							if (cliente_socket[j] != sd
									&& cliente_socket[j] != 0) {
								if (send(cliente_socket[j], mensaje,
										strlen(mensaje), 0) == -1) {
									perror("send() error!");
								} else {
									printf(
											"Mensaje enviado con el socket %d \n",
											cliente_socket[j]);
								}
							}
						}
						if (send(socket_memoria.socket, mensaje,
								strlen(mensaje), 0) != strlen(mensaje)) {
							perror("send memoria failed");
						}
						if (send(socket_fs.socket, mensaje, strlen(mensaje), 0)
								!= strlen(mensaje)) {
							perror("send filesystem failed");
						}
						printf("valor lectura: %d\n", valorLectura);
						puts(
								"mensajes a memoria y filesystem enviados correctamente");
						break;
					}

				} else {
					printf("recv error \n");
				}
			}
		}
	}
	return 0;
}


