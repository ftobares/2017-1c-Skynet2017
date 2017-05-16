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
#define TIPO_PROYECTO 1
#define BUFFERSIZE 40000
#define HSKERNEL     2 //HANDSHAKE
#define TOCPUKERNEL  3 //TIPO
#define OK 	"1"

//Variables Globales
t_console_config* config;
t_socket server_socket;
int CONECTADOALKERNEL = 0;

//Declaracion de funciones
void conectar_al_kernel();

int main(int argc, char* argv) {

	char* file_path;
	file_path = string_new();
	string_append(&file_path, "./src/consola.config");
	config = cargar_configuracion(file_path, TIPO_PROYECTO);

	conectar_al_kernel();

	return 0;
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


void conectar_al_kernel() {

	server_socket = cliente_crear_socket(config->ip_kernel,
			config->puerto_kernel);

	int AUX_CONEC_KER = connect(server_socket.socket,
			server_socket.socket_info->ai_addr,
			server_socket.socket_info->ai_addrlen);

	if (AUX_CONEC_KER < 0) {
		printf("Error en connect CONSOLA -> KERNEL\n");
		exit(-1);
	}

	//HandShake CONSOLA -> KERNEL
	if (AUX_CONEC_KER == saludar(HSKERNEL, TOCPUKERNEL, server_socket.socket)) {
		CONECTADOALKERNEL = 1;
	}

	connect(server_socket.socket, server_socket.socket_info->ai_addr,
			server_socket.socket_info->ai_addrlen);

	freeaddrinfo(server_socket.socket_info); // No lo necesitamos mas
	printf("CONECTADO AL KERNEL: %d", CONECTADOALKERNEL);

	if (CONECTADOALKERNEL) {
		int enviar = 1;
		char message[PACKAGESIZE];
		printf(
				"Conectado al servidor. Ya puede enviar mensajes. Escriba 'exit' para salir\n");
		while (enviar) {
			fgets(message, PACKAGESIZE, stdin); // Lee una linea en el stdin (lo que escribimos en la consola) hasta encontrar un \n (y lo incluye) o llegar a PACKAGESIZE.
			if (!strcmp(message, "exit\n"))
				enviar = 0; // Chequeo que el usuario no quiera salir
			if (enviar)
				send(server_socket.socket, message, strlen(message) + 1, 0); // Solo envio si el usuario no quiere salir.
		}

	} else {

		close(server_socket.socket);
		printf("HANDSHAKE ERROR - No se pudo conectar al KERNEL");
		return;
	}
}
