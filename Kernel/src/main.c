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

#define BACKLOG 30			// Cantidad conexiones maximas
#define PACKAGESIZE 1024	// Size maximo del paquete a enviar
#define TRUE   1
#define FALSE  0
#define CantClientes 30
#define TIPO_PROYECTO 4

//Variables Globales
t_kernel_config* config;
t_socket socket_memoria;
t_socket socket_fs;
t_master_socket master_socket;

//Declaracion de funciones
int iniciar_servidor();

int main(int argc, char* argv) {

	char* file_path;
	file_path = string_new();
	string_append(&file_path, "./src/kernel.config");
	config = cargar_configuracion(file_path, TIPO_PROYECTO);

	int v_valor_retorno = iniciar_servidor();

	return v_valor_retorno;
}

int iniciar_servidor() {

	int opt = TRUE;
	int addrlen, new_socket, cliente_socket[CantClientes],
			max_clientes = CantClientes, actividad, i, valorLectura, sd /*, read_size*/;
	int max_sd;
//	struct sockaddr_in server;

//	char buffer[PACKAGESIZE];

	//set de socket descriptores
	fd_set readfds;

	//a message
	char mensaje[PACKAGESIZE];

	//Conectarse a la Memoria
	socket_memoria = conectar_a_otro_servidor(config->ipMemoria, config->puertoMemoria);

	//Conectarse al FileSystem
	socket_fs = conectar_a_otro_servidor(config->ipFileSystem, config->puertoFileSystem);

	//inicializar todos los cliente_socket[] a 0 (No chequeado)
	for (i = 0; i < max_clientes; i++) {
		cliente_socket[i] = 0;
	}

	master_socket = servidor_crear_socket_bind_and_listen(config->puertoProg, opt, max_clientes);

//	//crear un socket maestro
//	if ((master_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
//		perror("socket failed");
//		exit(EXIT_FAILURE);
//	}
//
//	puts("Socket maestro creado");
//
//	//setear socket maestro para que permita multiples conexiones (Buenas practicas)
//	if (setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *) &opt,
//			sizeof(opt)) < 0) {
//		perror("setsockopt failed");
//		exit(EXIT_FAILURE);
//	}
//
//	//Tipo de socket creado
//	server.sin_family = AF_INET;
//	server.sin_addr.s_addr = INADDR_ANY;
//	server.sin_port = htons(config->puertoProg);
//
//	//bind
//	if (bind(master_socket, (struct sockaddr *) &server, sizeof(server)) < 0) {
//		perror("bind failed");
//		exit(EXIT_FAILURE);
//	}
//	printf("Listener en puerto %d \n", config->puertoProg);
//
//	//Listen
//	if (listen(master_socket, BACKLOG) < 0) {
//		perror("listen failed");
//		exit(EXIT_FAILURE);
//	}

	//accept the incoming connection
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
			if ((new_socket = accept(master_socket.socket, (struct sockaddr *) &master_socket.socket_info,
					(socklen_t*) &addrlen)) < 0) {
				perror("accept");
				exit(EXIT_FAILURE);
			}

			//inform user of socket number - used in send and receive commands
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
					getpeername(sd, (struct sockaddr*) &master_socket.socket_info,
							(socklen_t*) &addrlen);
					printf("Host desconectado , ip %s , puerto %d \n",
							inet_ntoa(master_socket.socket_info.sin_addr), ntohs(master_socket.socket_info.sin_port));

					//Cerrar el socket y marcar como 0 en la lista para reusar
					close(sd);
					cliente_socket[i] = 0;
				}
				if(valorLectura > 0) {
					mensaje[valorLectura] = '\0';
					if( send(socket_memoria.socket, mensaje, strlen(mensaje), 0) != strlen(mensaje) )
					{
						perror("send memoria failed");
					}

					if( send(socket_fs.socket, mensaje, strlen(mensaje), 0) != strlen(mensaje) )
					{
						perror("send filesystem failed");
					}

					printf("valor lectura: %d\n",valorLectura);
					puts(mensaje);
					puts("mensajes a memoria y filesystem enviados correctamente");
				} else {
					printf("recv error");
				}
			}
		}
	}

	return 0;
}
