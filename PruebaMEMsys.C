#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

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

// Función para convertir un dígito hexadecimal a binario
void HexToBinary(char hexDigit) {
    char binary[5] = {'0'};
    int decimal = strtol(&hexDigit, NULL, 16);
    for (int i = 3; i >= 0; --i) {
        binary[i] = (decimal % 2) + '0';
        decimal /= 2;
    }
    printf("%s ", binary);
}

// Función para procesar cada línea del archivo
void ProcesarLinea(FILE *archivo) {
    char linea[4]; // Considerando que cada línea tiene 3 caracteres hexadecimales
    while (fscanf(archivo, "%s", linea) != EOF) {
        printf("Hexadecimal: %s\nBinario: ", linea);
        for (int i = 0; i < strlen(linea); ++i) {
            HexToBinary(linea[i]);
        }
        printf("\n");
    }
}

int main() {
    FILE *memFile = fopen("accesos_memoria.txt", "r");

    if (memFile == NULL) {
        printf("No se encuentra el archivo.");
    }

    ProcesarLinea(memFile);

    fclose(memFile);

    T_CACHE_LINE Cache[NUM_FILAS];
    LimpiarCACHE(Cache);

    ImprimirCache(Cache);

    

    printf("\n");
    return 0;
}
