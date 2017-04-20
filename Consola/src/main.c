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
#define tipo_proyecto 1

//Variables Globales
t_console_config* config;
t_socket server_socket;

//Declaracion de funciones
void conectar_al_kernel();

int main(int argc, char* argv) {

	char* file_path;
	file_path = string_new();
	string_append(&file_path, "./src/consola.config");
	config = cargar_configuracion(file_path, tipo_proyecto);

	conectar_al_kernel();

	return 0;
}

void conectar_al_kernel() {

	server_socket = cliente_crear_socket(config->ip_kernel, config->puerto_kernel);

	connect(server_socket.socket, server_socket.socket_info->ai_addr, server_socket.socket_info->ai_addrlen);

	freeaddrinfo(server_socket.socket_info); // No lo necesitamos mas

	int enviar = 1;
	char message[PACKAGESIZE];
	printf("Conectado al servidor. Ya puede enviar mensajes. Escriba 'exit' para salir\n");
	while (enviar) {
		fgets(message, PACKAGESIZE, stdin); // Lee una linea en el stdin (lo que escribimos en la consola) hasta encontrar un \n (y lo incluye) o llegar a PACKAGESIZE.
		if (!strcmp(message, "exit\n"))
			enviar = 0; // Chequeo que el usuario no quiera salir
		if (enviar)
			send(server_socket.socket, message, strlen(message) + 1, 0); // Solo envio si el usuario no quiere salir.
	}

	close(server_socket.socket);
}
