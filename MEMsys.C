#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

//Definicion del tamaño de la linea y del numero de filas.
#define TAM_LINEA 16
#define NUM_FILAS 8

//Variables para simular la RAM y guardar los datos obtenidos durante la ejecucion.
unsigned char Simul_RAM[4096];
char texto[100];

//Variables globales de Tiempo y numero de fallos.
int globaltime = 0;
int numfallos = 0;

//Estructura de la cache.
typedef struct {
    unsigned char ETQ;
    unsigned char Data[TAM_LINEA];
} T_CACHE_LINE;

//Con este metodo, recorremos la cache que le enviamos 1 por 1 para modificar sus valores a los iniciales, donde la ETQ sera FF y los datos 23.
void LimpiarCACHE(T_CACHE_LINE tbl[NUM_FILAS]) {
    for (int i = 0; i < NUM_FILAS; ++i) {
        tbl[i].ETQ = 0xFF;
        for (int j = 0; j < TAM_LINEA; ++j) {
            tbl[i].Data[j] = 0x23;
        }
    }
}

//Con este metodo, solo mostraremos los datos que tenga la cache recorriendo cada linea y cada dato de esta misma.
void ImprimirCACHE(T_CACHE_LINE tbl[NUM_FILAS]) {
    printf("\n");

    for (int i = 0; i < NUM_FILAS; ++i) {
        printf("%02X\tDatos: ", tbl[i].ETQ);
        for (int j = 0; j < TAM_LINEA; ++j) {
            printf("%02X ", tbl[i].Data[j]);
        }
        printf("\n");
    }
}

//Con este metodo, volcamos y guardamos una vez finalizada la ejecucion de la cache, los datos guardados en esta misma en un fichero "CONTENTS_CACHE.bin".
void VolcarCACHE(T_CACHE_LINE *tlb) {
    //Inicializamos el archivo donde escribiremos los datos.
    FILE * contentsCACHE = fopen("CONTENTS_CACHE.bin", "wb");

    //Comprobamos si el archivo existe.
    if(contentsCACHE == NULL){
        fprintf(stderr, "Error al abrir el archivo CONTENTS_CACHE.bin para escritura\n");
        return;
    }

    //Volcamos los contenidos de la cache al archivo binario.
    fwrite(tlb, sizeof(T_CACHE_LINE), NUM_FILAS, contentsCACHE);

    //Cerramos el archivo.
    fclose(contentsCACHE);
}

//Con este metodo, Parseamos la direccion dada para diferenciar la ETIQUETA, LINEA, PALABRA y BLOQUE de cada direccion y separarlo en distintos datos.
void ParsearDireccion(unsigned int addr, int *ETQ, int *palabra, int*linea, int *bloque) {
    //Tamaño de la palabra (4 bits).
    *palabra = addr & 0xF;

    //Tamaño de la linea (3 bits).
    *linea = (addr >> 4) & 0x7;

    //Tamaño de la etiqueta (5 bits).
    *ETQ = (addr >> 7) & 0x1F;

    *bloque = *ETQ;
}

//Con este metodo, cada vez que la cache falle, modificamos los datos con los necesarios en la cache, y guardamos el dato que queremos obtener de la cache en texto[100].
void TratarFallo(T_CACHE_LINE *tlb, unsigned char *MRAM, int ETQ, int linea, int bloque) {
    //Actualizar la etiqueta y los datos de la cache con el bloque correspondiente
    tlb[linea].ETQ = ETQ;

    //Calcular la direccion de inicio del bloque en Simul_RAM.
    unsigned int inicioBloque = bloque << 7;

    //Copiar los datos del bloque en Simul_RAM a la cache.
    memcpy(tlb[linea].Data, &MRAM[inicioBloque], TAM_LINEA);

    //Agregar la letra correspondiente del Simul_RAM al final de texto[100].
    texto[strlen(texto)] = MRAM[inicioBloque + TAM_LINEA -1];
}

//Con este metodo, en caso de acertar, como guardamos el dato en texto[100] en el metodo TratarFallo, en caso de acierto directo, con este metodo tambien guardamos ese datos obtenido al acertar.
void TratarAcierto(T_CACHE_LINE *tlb, unsigned char *MRAM, int ETQ, int linea, int bloque) {
    //Calcular la direccion de inicio del bloque en Simul_RAM.
    unsigned int inicioBloque = bloque << 7;

    //Agregar la letra correspondiente del Simul_RAM al final de texto[100].
    texto[strlen(texto)] = MRAM[inicioBloque + TAM_LINEA -1];
}

int main() {

    T_CACHE_LINE cache[NUM_FILAS];

    //Primera inicializacion de la cache y muestra de datos.
    printf("---- Limpieza de la cache y muestra ----");
    LimpiarCACHE(cache);
    ImprimirCACHE(cache);
    //Sumamos el primer tiempo por la limpieza de la cache.
    globaltime += 20;
    printf("\n");

    //Declaracion de los archivos a utilizar
    FILE *contentRAM = fopen("CONTENTS_RAM.bin", "rb");
    FILE *accesosMEM = fopen("accesos_memoria.txt", "r");

    //Comprobar si los archivos se han encontrado o no.
    if(contentRAM == NULL || accesosMEM == NULL) {
        fprintf(stderr, "No se encontro alguno de los archivos necesarios.\n");
        return -1;
    }
    
    //Guardamos el contenido de CONTENTS_RAM.bin dentro del array Simul_RAM.
    fread(Simul_RAM, sizeof(Simul_RAM), 1, contentRAM);

    //Cerramos el archivo CONTENTS_RAM.bin ya que lo hemos almacenado.
    fclose(contentRAM);

    //Declaracion de las variables a utilizar para la cache.
    char direccionMEM[TAM_LINEA];
    unsigned int addr;
    int ETQ, palabra, linea, bloque;

    //Bucle para recorrer cada direccion de memoria de accesosMEM.
    while(fgets(direccionMEM, sizeof(direccionMEM), accesosMEM) != NULL) {
        //Guardamos la direccion encontrada en la variable addr.
        sscanf(direccionMEM, "%X", &addr);

        //Diferenciamos la ETIQUETA, LINEA, PALABRA y BLOQUE de la direccion obtenida
        ParsearDireccion(addr, &ETQ, &palabra, &linea, &bloque);

        //Comprobamos si la ETIQUETA de la direccion esta contenida ya en la cache
        if(cache[linea].ETQ != ETQ) {
            //Sumamos un fallo de cache.
            numfallos++;
            //En caso de que no este contenida, indicamos el Fallo de CACHE con los datos obtenidos.
            printf("T: %d, Fallo de CACHE %d, ADDR %04X, Label %X, linea %02X, palabra %02X, bloque %02X\n", globaltime, numfallos, addr, ETQ, linea, palabra, bloque);
            //Tratamos el fallo para guardar esos datos en la cache y actualizarla.
            TratarFallo(cache, Simul_RAM, ETQ, linea, bloque);
            //Mostramos un mensaje de que se esta Cargando el bloque en la cache con el metodo TratarFallo.
            printf("Cargando el bloque %d en la linea %d\n", bloque, linea);
            //Agregamos 20 segundos al tiempo global por fallo.
            globaltime += 20;
        }else{
            //En caso de acertar, utilizamos el metodo TratarAcierto para poder guardar el dato obtenido de la RAM en la variable texto[100].
            TratarAcierto(cache, Simul_RAM, ETQ, linea, bloque);
        }

        //Ahora mostramos el acierto de cache una vez cargada, o si ya lo estaba, con los datos que hemos querido obtener de la cache.
        printf("T: %d, Acierto de CACHE, ADDR %04x, Label %X, linea %02X, palabra %02X, DATO %02X", globaltime, addr, ETQ, linea, palabra, cache[linea].Data[palabra]);

        //Mostramos los datos de la cache
        ImprimirCACHE(cache);
        printf("\n");
    
        //Sleep de un segundo entre cada direccion de memoria.
        sleep(1);

    }

    //Cerramos el archivo accesos_memoria.txt ya que hemos terminado con el.
    fclose(accesosMEM);
    
    //Volcaremos los datos de nuestra cache en un archivo CONTENTS_CACHE.bin
    VolcarCACHE(cache);

    //Mostramos el Tiempo de acceso, el numero de fallo obtenido y el tiempo medio del programa. Ademas de mostrar que datos ha recogido de la cache en texto[100].
    printf("Accesos totales: %d; fallos: %d; Tiempo medio: %d", globaltime, numfallos, (globaltime/numfallos));
    printf("\nTexto leido: %s\n", texto);


    return 0;
}
