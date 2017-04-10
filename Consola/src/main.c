#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>
#include <commons/string.h>

//Variables Globales
t_config *config;

//Declaracion de funciones
void cargarYMostrarConfiguracion();
bool validar_configuracion(t_config* config); //Poner en UtilsLibray

int main(int argc, char* argv) {

	cargarYMostrarConfiguracion();
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

	char* ipKernel = config_get_string_value(config,"IP_KERNEL");
	int puertoKernel = config_get_int_value(config,"PUERTO_KERNEL");

	printf("Imprimir archivo de configuración: \n");
	printf("IP_KERNEL es %s \n",ipKernel);
	printf("PUERTO_KERNEL es %d \n",puertoKernel);
}
