#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BLOCK_SIZE 512
#define FILE_SIGNATURE "texto" // Patrón de bytes a buscar

int main()
{
    FILE *fp;
    unsigned char block[BLOCK_SIZE];
    int bytesRead = 0;
    int bytesMatched = 0;
    int fileCount = 0;
    char filename[50];

    fp = fopen("test.img", "rb"); // Abrir imagen de disco

    if(fp == NULL) {
        printf("No se pudo abrir el archivo.\n");
        return 1;
    }

    while(fread(block, BLOCK_SIZE, 1, fp) == 1) { // Leer los bloques de la imagen
        bytesRead += BLOCK_SIZE;

        for(int i = 0; i < BLOCK_SIZE; i++) {
            if(block[i] == FILE_SIGNATURE[bytesMatched]) { // Comparar bytes
                bytesMatched++;
                if(bytesMatched == strlen(FILE_SIGNATURE)) { // Si encontramos el archivo
                    fileCount++;
                    sprintf(filename, "archivo_%d.txt", fileCount); // Generar un nombre de archivo para guardarlo
                    FILE *out = fopen(filename, "wb"); // Abrir archivo para escribir
                    fwrite(block + i - bytesMatched + 1, bytesMatched, 1, out); // Escribir el archivo
                    bytesMatched = 0;
                    fclose(out); // Cerrar archivo
                }
            } else {
                bytesMatched = 0;
            }
        }
    }

    fclose(fp); // Cerrar la imagen de disco

    printf("Se encontraron y recuperaron %d archivos.\n", fileCount);

    return 0;
}