/*
 * utils_for_config.c
 *
 *  Created on: 13/4/2017
 *      Author: ftobares
 */

#include <stdio.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include "utils_config.h"

t_config *config;
t_console_config* console_config;
t_cpu_config* cpu_config;
t_fs_config* fs_config;
t_kernel_config* kernel_config;
t_memoria_config* memoria_config;

bool validar_configuracion(t_config* config) {
	return (config_keys_amount(config) > 0);
}

t_list* get_config_list_de_string_array(char* key) {
	t_list* list = list_create();
	char** array = config_get_array_value(config, key);
	int i = 0;
	while (array[i] != NULL) {
		list_add(list, array[i]);
		i++;
	}
	return list;
}

void* cargar_configuracion(char* path_archivo, int /*t_tipo_de_proyecto*/ tipo_proyecto) {

	/** Leer archivo de configuracion */
	char* configPath;
	configPath = string_new();
	string_append(&configPath, path_archivo);
//	string_append(&configPath, "consola.config");
	config = config_create(configPath);

	if (!validar_configuracion(config)) {
		printf("ERROR: No se encontró el archivo de configuración.");
		free(config); //Libero la memoria de config
	}

	switch (tipo_proyecto) {
	case 1:
//	case CONSOLA:
		console_config = malloc(sizeof(t_console_config));
		console_config->ip_kernel = strdup(config_get_string_value(config, "IP_KERNEL"));
		console_config->puerto_kernel = strdup(config_get_string_value(config,"PUERTO_KERNEL"));
		config_destroy(config);
		return console_config;
	case 2:
//	case CPU:
		cpu_config = malloc(sizeof(t_cpu_config));
		cpu_config->ip_kernel = strdup(config_get_string_value(config, "IP_KERNEL"));
		cpu_config->puerto_kernel = strdup(config_get_string_value(config,"PUERTO_KERNEL"));
		config_destroy(config);
		return cpu_config;
	case 3:
//	case FILESYSTEM:
		fs_config = malloc(sizeof(t_fs_config));
		fs_config->puerto = config_get_int_value(config, "PUERTO");
		fs_config->punto_montaje = strdup(config_get_string_value(config,"PUNTO_MONTAJE"));
		config_destroy(config);
		return fs_config;
	case 4:
//	case KERNEL:
		kernel_config = malloc(sizeof(t_kernel_config));
		kernel_config->puertoProg = config_get_int_value(config, "PUERTO_PROG");
		kernel_config->puertoCPU = config_get_int_value(config, "PUERTO_CPU");
		kernel_config->ipMemoria = strdup(config_get_string_value(config, "IP_MEMORIA"));
		kernel_config->puertoMemoria = strdup(config_get_string_value(config, "PUERTO_MEMORIA"));
		kernel_config->ipFileSystem = strdup(config_get_string_value(config, "IP_FS"));
		kernel_config->puertoFileSystem = strdup(config_get_string_value(config, "PUERTO_FS"));
		kernel_config->quantum = config_get_int_value(config, "QUANTUM");
		kernel_config->quantumSleep = config_get_int_value(config, "QUANTUM_SLEEP");
		kernel_config->algoritmo = strdup(config_get_string_value(config, "ALGORITMO"));
		kernel_config->gradoMultiprog = config_get_int_value(config, "GRADO_MULTIPROG");
		kernel_config->semIds = get_config_list_de_string_array("SEM_IDS");
		kernel_config->semInit = get_config_list_de_string_array("SEM_INIT");
		kernel_config->sharedVars = get_config_list_de_string_array("SHARED_VARS");
		kernel_config->stackSize = config_get_int_value(config, "STACK_SIZE");
		config_destroy(config);
		return kernel_config;
	case 5:
//	case MEMEORIA:
		memoria_config = malloc(sizeof(t_memoria_config));
		memoria_config->puerto = config_get_int_value(config,"PUERTO");
		memoria_config->marcos = config_get_int_value(config,"MARCOS");
		memoria_config->marcosSize = config_get_int_value(config,"MARCO_SIZE");
		memoria_config->entradasCache = config_get_int_value(config,"ENTRADAS_CACHE");
		memoria_config->cacheXProc = config_get_int_value(config,"CACHE_X_PROC");
		memoria_config->reemplazoCache = config_get_string_value(config,"REEMPLAZO_CACHE");
		memoria_config->retardoMemoria = config_get_int_value(config,"RETARDO_MEMORIA");
		//config_destroy(config);
		return memoria_config;
	default:
		printf("ERROR: Tipo de proyecto no valido");
		config_destroy(config);
		return 1;
	}
}

