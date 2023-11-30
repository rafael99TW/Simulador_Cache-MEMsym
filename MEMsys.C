#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define TAM_LINEA 16
#define NUM_FILAS 8

unsigned char Simul_RAM[4096];

int globaltime = 0;
int numfallos = 0;

typedef struct {
    unsigned char ETQ;
    unsigned char Data[TAM_LINEA];
} T_CACHE_LINE;

void LimpiarCACHE(T_CACHE_LINE tbl[NUM_FILAS]) {
    for (int i = 0; i < NUM_FILAS; ++i) {
        tbl[i].ETQ = 0xFF;
        for (int j = 0; j < TAM_LINEA; ++j) {
            tbl[i].Data[j] = 0x23;
        }
    }
}

void ImprimirCACHE(T_CACHE_LINE tbl[NUM_FILAS]) {
    for (int i = 0; i < NUM_FILAS; ++i) {
        printf("%02X\tDatos: ", tbl[i].ETQ);
        for (int j = 0; j < TAM_LINEA; ++j) {
            printf("%02X ", tbl[i].Data[j]);
        }
        printf("\n");
    }
}

void ParsearDireccion(unsigned int addr, int *ETQ, int *palabra, int*linea, int *bloque) {
    *palabra = addr & 0xF;
    *linea = (addr >> 4) & 0x7;
    *ETQ = (addr >> 7) & 0x1F;
    *bloque = *ETQ;
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

    FILE *contentRAM = fopen("CONTENTS_RAM.bin", "rb");
    FILE *accesosMEM = fopen("accesos_memoria.txt", "r");

    if(contentRAM == NULL || accesosMEM == NULL) {
        fprintf(stderr, "No se encontro alguno de los archivos necesarios.\n");
        return -1;
    }
    
    fread(Simul_RAM, sizeof(Simul_RAM), 1, contentRAM);

    fclose(contentRAM);

    char direccionMEM[TAM_LINEA];
    unsigned int addr;
    int ETQ, palabra, linea, bloque;

    while(fgets(direccionMEM, sizeof(direccionMEM), accesosMEM) != NULL) {
        sscanf(direccionMEM, "%X", &addr);

        ParsearDireccion(addr, &ETQ, &palabra, &linea, &bloque);

        if(cache[linea].ETQ != ETQ) {
            //Tratar fallo aqui
        }

    }
    

    printf("\n");

    return 0;
}
