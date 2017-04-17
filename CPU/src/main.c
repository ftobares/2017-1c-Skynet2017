#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>
#include <commons/string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>

#define PACKAGESIZE 1024
#define TRUE   1

char* ipKernel;
char* puertoKernel;

//Variables Globales
t_config *config;

//Declaracion de funciones
void cargar_y_mostrar_configuracion();
bool validar_configuracion(t_config* config); //Poner en UtilsLibray
void conectar_al_kernel();

int main(int argc, char* argv) {

	cargar_y_mostrar_configuracion();

	conectar_al_kernel();

	return 0;
}

bool validar_configuracion(t_config* config) {
	return (config_keys_amount(config) > 0);
}

void cargar_y_mostrar_configuracion() {

	/** Leer archivo de configuracion */
	char* configPath;
	configPath = string_new();
	string_append(&configPath, "./src/");
	string_append(&configPath, "cpu.config");
	config = config_create(configPath);

	if (!validar_configuracion(config)) {
		printf("No se encontró el archivo de configuración.");
		free(config); //Libero la memoria de config
	}

	ipKernel = config_get_string_value(config, "IP_KERNEL");
	puertoKernel = config_get_string_value(config, "PUERTO_KERNEL");

	printf("Imprimir archivo de configuración: \n");
	printf("IP_KERNEL es %s \n", ipKernel);
	printf("PUERTO_KERNEL es %s \n", puertoKernel);
}

void conectar_al_kernel() {
	struct addrinfo hints;
	struct addrinfo *serverInfo;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC; // Permite que la maquina se encargue de verificar si usamos IPv4 o IPv6
	hints.ai_socktype = SOCK_STREAM; // Indica que usaremos el protocolo TCP
	getaddrinfo(ipKernel, puertoKernel, &hints, &serverInfo); // Carga en serverInfo los datos de la conexion
	int serverSocket, valorLectura;
	serverSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype,
			serverInfo->ai_protocol);

	int c =connect(serverSocket, serverInfo->ai_addr, serverInfo->ai_addrlen);

	if(c < 0){
		printf("error en connect");
		exit(-1);
	}
	freeaddrinfo(serverInfo); // No lo necesitamos mas

	//int enviar = 1;
	char mensaje[PACKAGESIZE];
	printf("Conectado al servidor. Bienvenido al sistema!\n");
	while (TRUE) {
		valorLectura = recv(serverSocket, mensaje, PACKAGESIZE, 0);
		if(valorLectura > 0){
			mensaje[valorLectura] = '\0';
			fputs(mensaje, stdout);
			memset(mensaje, 0, PACKAGESIZE);
		}
		if(valorLectura < 0){
			printf("Error en recv() \n");
		}
	}

	close(serverSocket);
}
