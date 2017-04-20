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

#define PACKAGESIZE 1024
#define TRUE   1
#define TIPO_PROYECTO 2

//Variables Globales
t_cpu_config* config;
t_socket server_socket;

//Declaracion de funciones
void conectar_al_kernel();

int main(int argc, char* argv) {

	char* file_path;
	file_path = string_new();
	string_append(&file_path, "./src/cpu.config");
	config = cargar_configuracion(file_path, TIPO_PROYECTO);

	conectar_al_kernel();

	return 0;
}

void conectar_al_kernel() {
	int valorLectura;
	server_socket = cliente_crear_socket(config->ip_kernel, config->puerto_kernel);

//	struct addrinfo hints;
//	struct addrinfo *serverInfo;
//	memset(&hints, 0, sizeof(hints));
//	hints.ai_family = AF_UNSPEC; // Permite que la maquina se encargue de verificar si usamos IPv4 o IPv6
//	hints.ai_socktype = SOCK_STREAM; // Indica que usaremos el protocolo TCP
//	getaddrinfo(config->ip_kernel, config->puerto_kernel, &hints, &serverInfo); // Carga en serverInfo los datos de la conexion
//
//	int server_socket, valorLectura;
//	server_socket = socket(serverInfo->ai_family, serverInfo->ai_socktype,
//			serverInfo->ai_protocol);

	int c =connect(server_socket.socket, server_socket.socket_info->ai_addr, server_socket.socket_info->ai_addrlen);

	if(c < 0){
		printf("error en connect");
		exit(-1);
	}
	freeaddrinfo(server_socket.socket_info); // No lo necesitamos mas

	//int enviar = 1;
	char mensaje[PACKAGESIZE];
	printf("Conectado al servidor. Bienvenido al sistema!\n");
	while (TRUE) {
		valorLectura = recv(server_socket.socket, mensaje, PACKAGESIZE, 0);
		if(valorLectura > 0){
			mensaje[valorLectura] = '\0';
			fputs(mensaje, stdout);
			memset(mensaje, 0, PACKAGESIZE);
		}
		if(valorLectura < 0){
			printf("Error en recv() \n");
		}
	}

	close(server_socket.socket);
}
