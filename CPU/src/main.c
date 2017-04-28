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
#define OK 	"1"
#define TIPO_PROYECTO 2
#define BUFFERSIZE 40000
#define HSKERNEL   1 //HANDSHAKE
#define TOCPUKERNEL  2 //TIPO

//Variables Globales
t_cpu_config* config;
t_socket server_socket;
int CONECTADOALKERNEL = 0;
int AUX_CONEC_KER = 0;

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

int Enviar(int sRemoto, char * buffer)
{
  int cantBytes;
  cantBytes = send(sRemoto, buffer, strlen(buffer), 0);
  if (cantBytes == -1)
    perror("ERROR ENVIO DATOS.\n");
  return cantBytes;
}

int Recibir(int sRemoto, char * buffer)
{
  int bytecount;
  memset(buffer, 0, BUFFERSIZE);
  if ((bytecount = recv(sRemoto, buffer, BUFFERSIZE, 0)) == -1)
    perror("ERROR RECIBO DATOS. \n");

  return bytecount;
}

int saludar(int handshake, int tipo, int sRemoto) {

	char *respuesta = malloc(BUFFERSIZE * sizeof(char));
	char *mensaje = string_new();
	string_append(&mensaje, string_itoa(handshake));
	string_append(&mensaje, string_itoa(tipo));
	int aux = 0;
	Enviar(sRemoto, mensaje);
	Recibir(sRemoto, respuesta);

	if (!(string_starts_with(respuesta, OK)))
		perror("ERROR - HANDSHAKE NO FUE EXITOSO \n");
	else
		aux = 1;

	if (mensaje != NULL)
		free(mensaje);
	if (respuesta != NULL)
		free(respuesta);

	return aux;
}

void conectar_al_kernel() {

	int valorLectura;
	server_socket = cliente_crear_socket(config->ip_kernel,
			config->puerto_kernel);

	int AUX_CONEC_KER = connect(server_socket.socket,
			server_socket.socket_info->ai_addr,
			server_socket.socket_info->ai_addrlen);

	if (AUX_CONEC_KER < 0) {
		printf("error en connect CPU -> KERNEL");
		exit(-1);
	}
	/*
	//HandShake CPU -> KERNEL
	if (AUX_CONEC_KER == saludar(HSKERNEL, TOCPUKERNEL, server_socket.socket)) {
		CONECTADOALKERNEL = 1;
	}*/

	freeaddrinfo(server_socket.socket_info); // No lo necesitamos mas

	//int enviar = 1;
	if (CONECTADOALKERNEL) {
		char mensaje[PACKAGESIZE];
		printf("Conectado al servidor. Bienvenido al sistema!\n");
		while (TRUE) {
			valorLectura = recv(server_socket.socket, mensaje, PACKAGESIZE, 0);
			if (valorLectura > 0) {
				mensaje[valorLectura] = '\0';
				fputs(mensaje, stdout);
				memset(mensaje, 0, PACKAGESIZE);
			}
			if (valorLectura < 0) {
				printf("Error en recv() \n");
			}
		}
	}

	close(server_socket.socket);
}
