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
	string_append(&configPath, "memoria.config");
	config = config_create(configPath);

	if (!validar_configuracion(config)) {
		printf("No se encontró el archivo de configuración.");
		free(config); //Libero la memoria de config
	}

	int puerto = config_get_int_value(config,"PUERTO");
	int marcos = config_get_int_value(config,"MARCOS");
	int marcosSize = config_get_int_value(config,"MARCO_SIZE");
	int entradasCache = config_get_int_value(config,"ENTRADAS_CACHE");
	int cacheXProc = config_get_int_value(config,"CACHE_X_PROC");
	char* reemplazoCache = config_get_string_value(config,"REEMPLAZO_CACHE");
	int retardoMemoria = config_get_int_value(config,"RETARDO_MEMORIA");

	printf("Imprimir archivo de configuración: \n");
	printf("PUERTO es %d \n",puerto);
	printf("MARCOS es %d \n",marcos);
	printf("MARCOS_SIZE es %d \n",marcosSize);
	printf("ENTRADAS_CACHE es %d \n",entradasCache);
	printf("CACHE_X_PROC es %d \n",cacheXProc);
	printf("REEMPLAZO_CACHE es %s \n",reemplazoCache);
	printf("RETARDO_MEMORIA es %d \n",retardoMemoria);
}
