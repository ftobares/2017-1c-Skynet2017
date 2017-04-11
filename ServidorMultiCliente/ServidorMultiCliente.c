#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h> //for threading , link with lpthread

#define PUERTO "6667"
#define BACKLOG 5            // Define cuantas conexiones vamos a mantener pendientes al mismo tiempo
#define PACKAGESIZE 1024    // Define cual va a ser el size maximo del paquete a enviar
#define TRUE   1

//the thread function
void *connection_handler(void *);

int main(){


    struct addrinfo hints;
    struct addrinfo *serverInfo;
    fd_set read_fds; // conjunto temporal de descriptores de fichero para select()
    int fdmax;       // número máximo de descriptores de fichero
    int actividad;
    int opt = TRUE;


    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;        // No importa si uso IPv4 o IPv6
    hints.ai_flags = AI_PASSIVE;        // Asigna el address del localhost: 127.0.0.1
    hints.ai_socktype = SOCK_STREAM;    // Indica que usaremos el protocolo TCP

    getaddrinfo(NULL, PUERTO, &hints, &serverInfo); // Notar que le pasamos NULL como IP, ya que le indicamos que use localhost en AI_PASSIVE

    /* Necesitamos un socket que escuche las conecciones entrantes */
    int listenningSocket;
    listenningSocket = socket(serverInfo->ai_family, serverInfo->ai_socktype, serverInfo->ai_protocol);

    setsockopt(listenningSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt));

    bind(listenningSocket,serverInfo->ai_addr, serverInfo->ai_addrlen);
    freeaddrinfo(serverInfo); // Ya no lo vamos a necesitar

    /*
     *     Ya tengo un medio de comunicacion (el socket) y le dije por que "telefono" tiene que esperar las llamadas.
     *
     *     Solo me queda decirle que vaya y escuche!
     *
     */
    while(1){
			listen(listenningSocket, BACKLOG);        // IMPORTANTE: listen() es una syscall BLOQUEANTE.

			puts("Server activo");

		    FD_ZERO(&read_fds);
			FD_SET(listenningSocket, &read_fds);
			fdmax = listenningSocket;
			actividad = select( fdmax + 1 , &read_fds , NULL , NULL , NULL);

			if (actividad < 0)
			{
				printf("select error");
			}

			if (FD_ISSET(listenningSocket, &read_fds))
			{

    		pthread_t sniffer_thread;
            struct sockaddr_in addr;            // Esta estructura contendra los datos de la conexion del cliente. IP, puerto, etc.
            socklen_t addrlen = sizeof(addr);


            int socketCliente = accept(listenningSocket, (struct sockaddr *) &addr, &addrlen);

                int *new_sock = malloc(1);
                *new_sock = socketCliente;


            if( pthread_create( &sniffer_thread , NULL ,  connection_handler , (void*) new_sock))
            {
            	puts("No se creo el hilo");
                perror("could not create thread");
                return 1;
            }

            //Now join the thread , so that we dont terminate before the thread
            pthread_join( sniffer_thread , NULL);
            //puts("Server activo");
			}
    }

    close(listenningSocket);

    return 0;
}

void *connection_handler(void *socketCliente)
{
//puts("Se creo el hilo");
    int sock = *(int*)socketCliente;

    char package[PACKAGESIZE];
    int  status = 1;

    puts("Cliente conectado. Esperando mensajes:\n");

    while(status){
	//fgets(package, PACKAGESIZE, stdin); //forma segura
	send(sock, package, strlen(package) + 1, 0);
	}

    close(sock);
}
