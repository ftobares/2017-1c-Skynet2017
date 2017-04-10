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
	string_append(&configPath, "file_system.config");
	config = config_create(configPath);

	if (!validar_configuracion(config)) {
		printf("No se encontró el archivo de configuración.");
		free(config); //Libero la memoria de config
	}

	int puerto = config_get_int_value(config,"PUERTO");
	char* puntoMontaje = config_get_string_value(config,"PUNTO_MONTAJE");

	printf("Imprimir archivo de configuración: \n");
	printf("PUERTO es %d \n",puerto);
	printf("PUNTO_MONTAJE es %s \n",puntoMontaje);
}
