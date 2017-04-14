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

t_console_config cargar_y_mostrar_configuracion();

bool validar_configuracion(t_config* config);

#endif /* SRC_UTILSFORCONFIG_H_ */
