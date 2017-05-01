/*
 * utils_funciones.c
 *
 *  Created on: 24/4/2017
 *      Author: utnso
 */

#include "utils_funciones.h"
#include <string.h>

/*@NAME: validar_integridad_archivos (EN DESARROLLO)
 *@DESC: Funcion para validar mediante md5sum que el archivo recibido
 *		 sea igual al enviado. Es decir validar la integridad.
 */
int validar_integridad_archivos(char* ruta_y_nombre_archivo1, char* md5){

	char* comando_ejecutar1;
	comando_ejecutar1 = string_new();
	string_append(&comando_ejecutar1,"md5sum ");
	string_append(&comando_ejecutar1,ruta_y_nombre_archivo1);
	string_append(&comando_ejecutar1," | awk '{print $1}'");

	char* md5_archivo1 = system(comando_ejecutar1);

	int i;
	int tamanio_string = string_length(md5_archivo1);
	for(i=0; i<=tamanio_string;i=i+1){
		if(md5_archivo1[i] != md5[i]){
			return 1;
		}
	}

	return 0;
}

/*@NAME: convertir_lista_en_array (EN DESARROLLO)
 *@DESC: Funcion para convertir una lista en un array
 *		 Se utiliza principalmente en la serializacion

void* convertir_lista_en_array(t_list* lista){
	return "";
}*/
