#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>
#include <commons/string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#define PACKAGESIZE 1024

//Variables Globales
t_config *config;
char* ipKernel;
char* puertoKernel;

//Declaracion de funciones
void cargarYMostrarConfiguracion();
bool validar_configuracion(t_config* config); //Poner en UtilsLibray
void conectarAlKernel();

int main(int argc, char* argv) {

	cargarYMostrarConfiguracion();
	conectarAlKernel();


	return 0;
}

bool validar_configuracion(t_config* config){
	return (config_keys_amount(config) > 0);
}

void cargarYMostrarConfiguracion(){

	/** Leer archivo de configuracion */
	char* configPath;
	configPath = string_new();
	string_append(&configPath, "./src/");
	string_append(&configPath, "consola.config");
	config = config_create(configPath);

	if (!validar_configuracion(config)) {
		printf("No se encontró el archivo de configuración.");
		free(config); //Libero la memoria de config
	}

	ipKernel = config_get_string_value(config,"IP_KERNEL");
	puertoKernel = config_get_string_value(config,"PUERTO_KERNEL");

	printf("Imprimir archivo de configuración: \n");
	printf("IP_KERNEL es %s \n",ipKernel);
	printf("PUERTO_KERNEL es %d \n",puertoKernel);
}

void conectarAlKernel(){
	struct addrinfo hints;
	struct addrinfo *serverInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo(ipKernel, puertoKernel, &hints, &serverInfo);

	int serverSocket;
	serverSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);

	connect(serverSocket, serverInfo->ai_addr, serverInfo->ai_addrlen);
	freeaddrinfo(serverInfo);

	char package[PACKAGESIZE];
	int status = 1;		// Estructura que manjea el status de los recieve.

	printf("Esperando mensajes:\n");

	while (status != 0){
		status = recv(serverSocket, (void*) package, PACKAGESIZE, 0);
		if (status != 0) printf("%s", package);

	}

	close(serverSocket);
}
