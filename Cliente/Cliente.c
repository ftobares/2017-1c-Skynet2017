#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#define IP "127.0.0.1"
#define PUERTO "6667"
#define PACKAGESIZE 1024	// Define cual va a ser el size maximo del paquete a enviar

int main(){

/*
*  ¿Quien soy? ¿Donde estoy? ¿Existo?
*
*  Estas y otras preguntas existenciales son resueltas getaddrinfo();
*
*  Obtiene los datos de la direccion de red y lo guarda en serverInfo.
*
*/
struct addrinfo hints;
struct addrinfo *serverInfo;

memset(&hints, 0, sizeof(hints));
hints.ai_family = AF_UNSPEC;	// Permite que la maquina se encargue de verificar si usamos IPv4 o IPv6
hints.ai_socktype = SOCK_STREAM;	// Indica que usaremos el protocolo TCP

getaddrinfo(IP, PUERTO, &hints, &serverInfo);	// Carga en serverInfo los datos de la conexion


/*
* Ya se quien y a donde me tengo que conectar... ¿Y ahora?
*	Tengo que encontrar una forma por la que conectarme al server... Ya se! Un socket!
*
* Obtiene un socket (un file descriptor -todo en linux es un archivo-), utilizando la estructura serverInfo que generamos antes.
*
*/
int serverSocket;
serverSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);

/*
* Perfecto, ya tengo el medio para conectarme (el archivo), y ya se lo pedi al sistema.
* Ahora me conecto!
*
*/
connect(serverSocket, serverInfo->ai_addr, serverInfo->ai_addrlen);
freeaddrinfo(serverInfo);	// No lo necesitamos mas

int recibir = 1;
char mensaje[PACKAGESIZE];

puts("Conectado al servidor. Bienvenido al sistema\n");

while (recibir){
recibir = recv(serverSocket, (void*) mensaje, PACKAGESIZE, 0);
if (!strcmp(mensaje,"exit\n")) recibir = 0;
if (recibir) puts(mensaje);

}

close(serverSocket);
return 0;

}
