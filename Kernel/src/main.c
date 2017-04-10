#include <stdio.h>
#include <stdlib.h>
#include <commons/config.h>
#include <commons/string.h>
#include <commons/collections/list.h>

//Variables Globales
t_config *config;

//Declaracion de funciones
void cargarYMostrarConfiguracion();
bool validar_configuracion(t_config* config); //Poner en UtilsLibray
t_list* get_config_list_de_string_array(char* key); //Poner en UtilsLibray
void imprimirSemIds(char* config_param);
void imprimirSemInt(char* config_param);
void imprimirSharedVars(char* config_param);


int main(int argc, char* argv) {

	cargarYMostrarConfiguracion();
	return 0;
}

bool validar_configuracion(t_config* config){
	return (config_keys_amount(config) > 0);
}

t_list* get_config_list_de_string_array(char* key){
	t_list* list = list_create();
	char** array = config_get_array_value(config,key);
	int i = 0;
	while(array[i]!=NULL){
		list_add(list,array[i]);
		i++;
	}
	return list;
}

void imprimirSemIds(char* config_param){
	printf("SEM_IDS es %s \n",config_param);
}

void imprimirSemInt(char* config_param){
	printf("SEM_INIT es %s \n",config_param);
}

void imprimirSharedVars(char* config_param){
	printf("SHARED_VARS es %s \n",config_param);
}

void cargarYMostrarConfiguracion(){

	/** Leer archivo de configuracion */
	char* configPath;
	configPath = string_new();
	string_append(&configPath, "./src/");
	string_append(&configPath, "kernel.config");
	config = config_create(configPath);

	if (!validar_configuracion(config)) {
		printf("No se encontró el archivo de configuración.");
		free(config); //Libero la memoria de config
	}

	int puertoProg = config_get_int_value(config,"PUERTO_PROG");
	int puertoCPU = config_get_int_value(config,"PUERTO_CPU");
	char* ipMemoria = config_get_string_value(config,"IP_MEMORIA");
	int puertoMemoria = config_get_int_value(config,"PUERTO_MEMORIA");
	char* ipFileSystem = config_get_string_value(config,"IP_FS");
	int puertoFileSystem = config_get_int_value(config,"PUERTO_FS");
	int quantum = config_get_int_value(config,"QUANTUM");
	int quantumSleep = config_get_int_value(config,"QUANTUM_SLEEP");
	char* algoritmo = config_get_string_value(config,"ALGORITMO");
	int gradoMultiprog = config_get_int_value(config,"GRADO_MULTIPROG");
	char* semIds = get_config_list_de_string_array("SEM_IDS"); //config_get_array_value(config,"SEM_IDS");
	char* semInit = get_config_list_de_string_array("SEM_INIT"); //config_get_array_value(config,"SEM_INIT");
	char* sharedVars = get_config_list_de_string_array("SHARED_VARS"); //config_get_array_value(config,"SHARED_VARS");
	char* stackSize = config_get_int_value(config,"STACK_SIZE");

	printf("Imprimir archivo de configuración: \n");
	printf("PUERTO_PROG es %d \n",puertoProg);
	printf("PUERTO_CPU es %d \n",puertoCPU);
	printf("IP_MEMORIA es %s \n",ipMemoria);
	printf("PUERTO_MEMORIA es %d \n",puertoMemoria);
	printf("IP_FS es %s \n",ipFileSystem);
	printf("PUERTO_FS es %d \n",puertoFileSystem);
	printf("QUANTUM es %d \n",quantum);
	printf("QUANTUM_SLEEP es %d \n",quantumSleep);
	printf("ALGORITMO es %s \n",algoritmo);
	printf("GRADO_MULTIPROG es %d \n",gradoMultiprog);
	printf("STACK_SIZE es %d \n",stackSize);
	list_iterate(semIds,imprimirSemIds);
	list_iterate(semIds,imprimirSemInt);
	list_iterate(semIds,imprimirSharedVars);
}
