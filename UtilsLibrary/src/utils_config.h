/*
 * utilsForConfig.h
 *
 *  Created on: 13/4/2017
 *      Author: ftobares
 */
#include <commons/config.h>

#ifndef SRC_UTILSFORCONFIG_H_
#define SRC_UTILSFORCONFIG_H_

typedef enum {
	CONSOLA,
	CPU,
	FILESYSTEM,
	KERNEL,
	MEMEORIA
} t_tipo_de_proyecto;

typedef struct {
	char* ip_kernel;
	char* puerto_kernel;
} t_console_config;

typedef struct {
	char* ip_kernel;
	char* puerto_kernel;
} t_cpu_config;

typedef struct {
	char* puerto;
	char* punto_montaje;
} t_fs_config;

typedef struct {
	int puertoProg;
	int puertoCPU;
	char* ipMemoria;
	char* puertoMemoria;
	char* ipFileSystem;
	char* puertoFileSystem;
	int quantum;
	int quantumSleep;
	char* algoritmo;
	int gradoMultiprog;
	char* semIds;
	char* semInit;
	char* sharedVars;
	int stackSize;
} t_kernel_config;

typedef struct {
	int puerto;
	int marcos;
	int marcosSize;
	int entradasCache;
	int cacheXProc;
	char* reemplazoCache;
	int retardoMemoria;
} t_memoria_config;

void* cargar_configuracion(char* path_archivo, t_tipo_de_proyecto tipo_proyecto);

bool validar_configuracion(t_config* config);

#endif /* SRC_UTILSFORCONFIG_H_ */
