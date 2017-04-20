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
#define TIPO_PROYECTO 2

//Variables Globales
t_cpu_config* config;
t_socket socket_server;

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
	struct addrinfo hints;
	struct addrinfo *serverInfo;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC; // Permite que la maquina se encargue de verificar si usamos IPv4 o IPv6
	hints.ai_socktype = SOCK_STREAM; // Indica que usaremos el protocolo TCP
	getaddrinfo(config->ip_kernel, config->puerto_kernel, &hints, &serverInfo); // Carga en serverInfo los datos de la conexion
	int serverSocket;
	serverSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype,
			serverInfo->ai_protocol);

	connect(serverSocket, serverInfo->ai_addr, serverInfo->ai_addrlen);
	freeaddrinfo(serverInfo); // No lo necesitamos mas

	int enviar = 1;
	char message[PACKAGESIZE];
	printf(
			"Conectado al servidor. Bienvenido al sistema, ya puede enviar mensajes. Escriba 'exit' para salir\n");
	while (enviar) {
		fgets(message, PACKAGESIZE, stdin); // Lee una linea en el stdin (lo que escribimos en la consola) hasta encontrar un \n (y lo incluye) o llegar a PACKAGESIZE.
		if (!strcmp(message, "exit\n"))
			enviar = 0; // Chequeo que el usuario no quiera salir
		if (enviar)
			send(serverSocket, message, strlen(message) + 1, 0); // Solo envio si el usuario no quiere salir.
	}

	close(serverSocket);
}
