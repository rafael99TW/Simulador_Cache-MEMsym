#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#define TAM_LINEA 16
#define NUM_FILAS 8

unsigned char Simul_RAM[4096];
char texto[100];

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

void VolcarCACHE(T_CACHE_LINE *tlb){
    FILE * contentsCACHE = fopen("CONTENTS_CACHE.bin", "wb");
    if(contentsCACHE == NULL){
        fprintf(stderr, "Error al abrir el archivo CONTENTS_CACHE.bin para escritura\n");
        return;
    }
    fwrite(tlb, sizeof(T_CACHE_LINE), NUM_FILAS, contentsCACHE);
    fclose(contentsCACHE);
}

void ParsearDireccion(unsigned int addr, int *ETQ, int *palabra, int*linea, int *bloque) {
    *palabra = addr & 0xF;
    *linea = (addr >> 4) & 0x7;
    *ETQ = (addr >> 7) & 0x1F;
    *bloque = *ETQ;
}

void TratarFallo(T_CACHE_LINE *tlb, unsigned char *MRAM, int ETQ, int linea, int bloque) {
    tlb[linea].ETQ = ETQ;
    unsigned int inicioBloque = bloque << 7;
    memcpy(tlb[linea].Data, &MRAM[inicioBloque], TAM_LINEA);
    texto[strlen(texto)] = MRAM[inicioBloque + TAM_LINEA -1];
}

void TratarAcierto(T_CACHE_LINE *tlb, unsigned char *MRAM, int ETQ, int linea, int bloque) {
    unsigned int inicioBloque = bloque << 7;
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
            numfallos++;
            printf("T: %d, Fallo de CACHE %d, ADDR %04x, Label %X, linea %02X, palabra %02X, palabra %02X, bloque %02X\n", globaltime, numfallos, addr, ETQ, linea, palabra, bloque);
            TratarFallo(cache, Simul_RAM, ETQ, linea, bloque);
            printf("Cargando el bloque %d en la linea %d\n", bloque, linea);
            globaltime += 20;
        }else{
            TratarAcierto(cache, Simul_RAM, ETQ, linea, bloque);
        }

        printf("T: %d, Acierto de CACHE, ADDR %04x, Label %X, linea %02X, palabra %02X, DATO %02X", globaltime, addr, ETQ, linea, palabra, cache[linea].Data[palabra]);

        ImprimirCACHE(cache);
        printf("\n");
    

        sleep(1);

    }

    fclose(accesosMEM);
    
    VolcarCACHE(cache);

    printf("Accesos totales: %d; fallors: %d; tiempo medio: %d", globaltime, numfallos, (globaltime/numfallos));
    printf("\ntexto leido: %s\n", texto);

    printf("\n");

    return 0;
}
