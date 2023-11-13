#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define TAM_LINEA 16
#define NUM_FILAS 16

typedef struct {
    unsigned char ETQ;
    unsigned char Data[TAM_LINEA];
} T_CACHE_LINE;

unsigned char Simul_RAM[4096];

int globaltime = 0;
int numfallos = 0;

void LimpiarCACHE(T_CACHE_LINE tbl[NUM_FILAS]) {
    for (int i = 0; i < NUM_FILAS; ++i) {
        tbl[i].ETQ = 0xFF;
        for (int j = 0; j < TAM_LINEA; ++j) {
            tbl[i].Data[j] = 0x23;
        }
    }
}

void ImprimirCache(T_CACHE_LINE tbl[NUM_FILAS]) {
    for (int i = 0; i < NUM_FILAS; ++i) {
        printf("%02X\tDatos: ", tbl[i].ETQ);
        for (int j = 0; j < TAM_LINEA; ++j) {
            printf("%02X ", tbl[i].Data[j]);
        }
        printf("\n");
    }
}

int main() {
    // FILE *ramFile = fopen("/home/rafael/Escritorio/SSOO/Practica 1/Practica-MEMsym/CONTENTS_RAM.bin", "rb");
    // FILE *memFile = fopen("/home/rafael/Escritorio/SSOO/Practica 1/Practica-MEMsym/acceso_memoria.txt", "r");

    // if (ramFile == NULL || memFile == NULL) {
    //     printf("Error: No se pudo abrir uno o varios archivos.\n");
        
    // }

    T_CACHE_LINE Cache[NUM_FILAS];
    LimpiarCACHE(Cache);

    ImprimirCache(Cache);

    // fclose(ramFile);
    // fclose(memFile);

    printf("Número total de accesos: %d\n", globaltime / 20);
    printf("Número de fallos: %d\n", numfallos);

    printf("\n");
    return 0;
}
