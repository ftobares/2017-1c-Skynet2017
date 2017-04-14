/*
 * utils_for_config.c
 *
 *  Created on: 13/4/2017
 *      Author: ftobares
 */

#include "utilsForConfig.h"

t_config *config;
t_console_config* console_config;

bool validar_configuracion(t_config* config) {
	return (config_keys_amount(config) > 0);
}

t_console_config* cargar_configuracion_consola() {

	/** Leer archivo de configuracion */
	char* configPath;
	configPath = string_new();
	string_append(&configPath, "./src/");
	string_append(&configPath, "consola.config");
	config = config_create(configPath);

	if (!validar_configuracion(config)) {
		printf("ERROR: No se encontró el archivo de configuración.");
		free(config); //Libero la memoria de config
	}

	console_config = malloc(sizeof(t_console_config));
	console_config->ip_kernel = config_get_string_value(config,"IP_KERNEL");
	console_config->puerto_kernel = config_get_string_value(config,"PUERTO_KERNEL");

	imprimir_configuracion(CONSOLA);

	return console_config;
}

void imprimir_configuracion(t_tipo_de_proyecto tipo_proyecto){
	switch (tipo_proyecto) {
		case CONSOLA:
			printf("Imprimir archivo de configuración de Consola: \n");
			printf("IP_KERNEL es %s \n", console_config->ip_kernel);
			printf("PUERTO_KERNEL es %s \n", console_config->puerto_kernel);
			break;
		case CPU:
			break;
		case FILESYSTEM:
			break;
		case KERNEL:
			break;
		case MEMEORIA:
			break;
		default:
			printf("ERROR: Tipo de proyecto no valido");
		}
}

