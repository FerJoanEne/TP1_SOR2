#include <stdio.h>

int main() {
    FILE *img;
    char buffer[1024];
    long int offset = 0x6A00; // posición de memoria del archivo dentro del archivo de imagen

    // Abrir el archivo de imagen en modo lectura binaria
    img = fopen("test.img", "rb");
    if (img == NULL) {
        printf("Error al abrir el archivo de imagen");
        return 1;
    }

    // Mover la posición actual del puntero de archivo a la posición de memoria del archivo
    fseek(img, offset, SEEK_SET);

    // Leer el contenido del archivo en un buffer en memoria
    fread(buffer, sizeof(char), sizeof(buffer), img);

    // Imprimir el contenido del archivo
    printf("Contenido del archivo:\n%s\n", buffer);

    // Cerrar el archivo de imagen
    fclose(img);

    return 0;
}