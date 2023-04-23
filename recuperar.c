#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    FILE *fp1, *fp2;
    char ch;
    int flag = 0;

    // Chequeo de parámetros
    if (argc != 2) {
        printf("Uso: %s archivo\n", argv[0]);
        return 1;
    }

    // Abro el archivo original en modo lectura
    fp1 = fopen(argv[1], "r");
    if (fp1 == NULL) {
        printf("No se pudo abrir el archivo %s\n", argv[1]);
        return 1;
    }

    // Creo una copia del archivo original en modo escritura
    fp2 = fopen("copia.txt", "w");
    if (fp2 == NULL) {
        printf("No se pudo crear la copia del archivo\n");
        return 1;
    }

    // Recorro el archivo original en busca de caracteres
    while ((ch = fgetc(fp1)) != EOF) {
        fputc(ch, fp2);

        // Si encuentro algún caracter, cambio el flag a 1
        if (ch != ' ' && ch != '\n') {
            flag = 1;
        }
    }

    // Si el archivo se encontraba borrado, lo recupero
    if (flag == 0) {
        printf("El archivo estaba borrado, se ha recuperado.\n");
        remove(argv[1]); // Elimino el archivo original
        rename("copia.txt", argv[1]); // Renombro la copia al nombre original
    }

    // Cierro los archivos
    fclose(fp1);
    fclose(fp2);

    return 0;
}
